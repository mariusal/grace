/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1996-2003 Grace Development Team
 * 
 * Maintained by Evgeny Stambulchik <fnevgeny@plasma-gate.weizmann.ac.il>
 * 
 * 
 *                           All Rights Reserved
 * 
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 * 
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 * 
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <config.h>

#include <string.h>

#include "grace/baseP.h"
#include "grace/canvasP.h"

#include <t1lib.h>
/* A hack - until there are T1_MAJORVERSION etc defined */
#ifndef T1ERR_SCAN_ENCODING
# define T1_CheckForFontID CheckForFontID
# define T1_GetNoFonts T1_Get_no_fonts
#endif

int init_t1(Canvas *canvas)
{
    /* Set log-level */
    T1_SetLogLevel(T1LOG_DEBUG);
    
    /* Initialize t1-library */
    if (T1_InitLib(T1LOGFILE|IGNORE_CONFIGFILE) == NULL) {
        return RETURN_FAILURE;
    }
    
    /* Rasterization parameters */
    T1_SetDeviceResolutions(72.0, 72.0);
    T1_AASetBitsPerPixel(GRACE_BPP);
    T1_SetBitmapPad(T1_DEFAULT_BITMAP_PAD);
    
    return RETURN_SUCCESS;
}

int canvas_set_encoding(Canvas *canvas, char *encfile)
{
    if (!encfile) {
        return RETURN_FAILURE;
    }
    
    canvas->DefEncoding = T1_LoadEncoding(encfile);
    if (canvas->DefEncoding) {
        T1_SetDefaultEncoding(canvas->DefEncoding);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int canvas_add_font(Canvas *canvas, char *ffile, const char *alias)
{
    void *p;
    FontDB *f;
    
    p = xrealloc(canvas->FontDBtable, (canvas->nfonts + 1)*sizeof(FontDB));
    if (!p) {
        return RETURN_FAILURE;
    }
    
    canvas->FontDBtable = p;
    f = &canvas->FontDBtable[canvas->nfonts];
    memset(f, 0, sizeof(FontDB));
    
    if (T1_AddFont(ffile) < 0 || T1_GetNoFonts() != canvas->nfonts + 1) {
        return RETURN_FAILURE;
    }
    
    f->alias = copy_string(NULL, alias);
    canvas->nfonts++;
    
    return RETURN_SUCCESS;
}

unsigned int number_of_fonts(const Canvas *canvas)
{
    return canvas->nfonts;
}

int canvas_get_font_by_name(const Canvas *canvas, const char *fname)
{
    unsigned int i;
    
    if (fname == NULL) {
        return BAD_FONT_ID;
    }
#if 0    
    /* check first the real font name */
    for (i = 0; i < canvas->nfonts; i++) {
        if (strcmp(get_fontname(canvas, i), fname) == 0) {
            return i;
        }
    }
#endif
    /* if failed, see if an alias fits */
    for (i = 0; i < canvas->nfonts; i++) {
        if (strcmp(get_fontalias(canvas, i), fname) == 0) {
            return i;
        }
    }

    return BAD_FONT_ID;
}

char *get_fontfilename(const Canvas *canvas, int font, int abspath)
{
    if (abspath) {
        return (T1_GetFontFilePath(font));
    } else {
        return (T1_GetFontFileName(font));
    }
}

char *get_afmfilename(const Canvas *canvas, int font, int abspath)
{
    char *s;

    if (abspath) {
        s = T1_GetAfmFilePath(font);
    } else {
        s = T1_GetAfmFileName(font);
    }
    
    if (s == NULL) {
        char *s1;
        static char buf[256];
        int len;
        
        s = get_fontfilename(canvas, font, abspath);
        len = strlen(s);
        s1 = s + (len - 1);
        while(s1 && *s1 != '.') {
            len--;
            s1--;
        }
        strncpy(buf, s, len);
        buf[len] = '\0';
        strcat(buf, "afm");
        return buf;
    } else {
        return s;
    }
}

char *get_fontname(const Canvas *canvas, int font)
{
    return (T1_GetFontName(font));
}

char *get_fontfullname(const Canvas *canvas, int font)
{
    return (T1_GetFullName(font));
}

char *get_fontfamilyname(const Canvas *canvas, int font)
{
    return (T1_GetFamilyName(font));
}

char *get_fontweight(const Canvas *canvas, int font)
{
    return (T1_GetWeight(font));
}

char *get_fontalias(const Canvas *canvas, int font)
{
    return (canvas->FontDBtable[font].alias);
}

char *get_encodingscheme(const Canvas *canvas, int font)
{
    return (T1_GetEncodingScheme(font));
}

char **get_default_encoding(const Canvas *canvas)
{
    return (canvas->DefEncoding);
}

double get_textline_width(const Canvas *canvas, int font)
{
    return (double) T1_GetUnderlineThickness(font)/1000.0;
}

double get_underline_pos(const Canvas *canvas, int font)
{
    return (double) T1_GetLinePosition(font, T1_UNDERLINE)/1000.0;
}

double get_overline_pos(const Canvas *canvas, int font)
{
    return (double) T1_GetLinePosition(font, T1_OVERLINE)/1000.0;
}

double get_italic_angle(const Canvas *canvas, int font)
{
    return (double) T1_GetItalicAngle(font);
}

double *get_kerning_vector(const Canvas *canvas,
    const char *str, int len, int font)
{
    if (len < 2 || T1_GetNoKernPairs(font) <= 0) {
        return NULL;
    } else {
        int i, k, ktot;
        double *kvector;
        
        kvector = xmalloc(len*SIZEOF_DOUBLE);
        for (i = 0, ktot = 0; i < len - 1; i++) {
            k = T1_GetKerning(font, str[i], str[i + 1]);
            ktot += k;
            kvector[i] = (double) k/1000;
        }
        if (ktot) {
            kvector[len - 1] = (double) ktot/1000;
        } else {
            XCFREE(kvector);
        }
        
        return kvector;
    }
}

char *font_subset(const Canvas *canvas,
    int font, char *mask, unsigned long *datalen)
{
    return T1_SubsetFont(font, mask, T1_SUBSET_DEFAULT, 64, 16384, datalen);
}

/* determinant */
static double tm_det(const TextMatrix *tm)
{
    return tm->cxx*tm->cyy - tm->cxy*tm->cyx;
}

int tm_product(TextMatrix *tm, const TextMatrix *p)
{
    TextMatrix tmp;
    
    if (tm_det(p) != 0.0) {
        tmp.cxx = p->cxx*tm->cxx + p->cxy*tm->cyx;
        tmp.cxy = p->cxx*tm->cxy + p->cxy*tm->cyy;
        tmp.cyx = p->cyx*tm->cxx + p->cyy*tm->cyx;
        tmp.cyy = p->cyx*tm->cxy + p->cyy*tm->cyy;
        *tm = tmp;
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

/* vertical size */
double tm_size(const TextMatrix *tm)
{
    return tm_det(tm)/sqrt(tm->cxx*tm->cxx + tm->cyx*tm->cyx);
}

int tm_scale(TextMatrix *tm, double s)
{
    if (s != 0.0) {
        tm->cxx *= s; tm->cxy *= s; tm->cyx *= s; tm->cyy *= s;
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int tm_rotate(TextMatrix *tm, double angle)
{
    if (angle != 0.0) {
        double co, si;
        TextMatrix tmp;

        si = sin(M_PI/180.0*angle);
        co = cos(M_PI/180.0*angle);
        tmp.cxx = co; tmp.cyy = co; tmp.cxy = -si; tmp.cyx = si;
        tm_product(tm, &tmp);
    }
    
    return RETURN_SUCCESS;
}

int tm_slant(TextMatrix *tm, double slant)
{
    if (slant != 0.0) {
        TextMatrix tmp;

        tmp.cxx = 1.0; tmp.cyy = 1.0; tmp.cxy = slant; tmp.cyx = 0.0;
        tm_product(tm, &tmp);
    }
    
    return RETURN_SUCCESS;
}

/* Set the colors for Anti-Aliasing */
static void set_aa_gray_values(Canvas *canvas,
    unsigned long fg, unsigned long bg, int t1aa)
{
    unsigned n;
    unsigned long *colors, last_bg, last_fg;
    int *colors_ok;
    
    switch (t1aa) {
    case T1_AA_LOW:
        n = T1_AALEVELS_LOW;
        colors = canvas->aacolors_low;
        colors_ok = &canvas->aacolors_low_ok;
        break;
    case T1_AA_HIGH:
        n = T1_AALEVELS_HIGH;
        colors = canvas->aacolors_high;
        colors_ok = &canvas->aacolors_high_ok;
        break;
    default:
        return;
        break;
    }
    
    last_bg = colors[0];
    last_fg = colors[n - 1];
    
    if (last_fg != fg || last_bg != bg || !(*colors_ok)) {
        make_color_scale(canvas, fg, bg, n, colors);

        if (t1aa == T1_AA_LOW) {
            T1_AASetGrayValues(colors[0],
    			       colors[1],
    			       colors[2],
    			       colors[3],
    			       colors[4]);
        } else {
    	    T1_AAHSetGrayValues(colors);
        }
        
        *colors_ok = TRUE;
    }

    T1_AASetLevel(t1aa);
}

static GLYPH *GetGlyphString(Canvas *canvas,
    CStringSegment *cs, double dpv, FontRaster fontrast)
{
    int len = cs->len;
    int FontID = cs->font;
    float Size;
    
    long Space = 0;
    
    GLYPH *glyph;
    
    int mono, t1aa;
    unsigned long fg, bg;

    int modflag;
    T1_TMATRIX matrix, *matrixP;

    if (cs->len == 0) {
        return NULL;
    }

    /* T1lib doesn't like negative sizes */
    Size = (float) fabs(tm_size(&cs->tm));
    if (Size == 0.0) {
        return NULL;
    }

    /* NB: T1lib uses counter-intuitive names for off-diagonal terms */
    matrix.cxx = (float) cs->tm.cxx/Size;
    matrix.cxy = (float) cs->tm.cyx/Size;
    matrix.cyx = (float) cs->tm.cxy/Size;
    matrix.cyy = (float) cs->tm.cyy/Size;

    Size *= dpv;

    modflag = T1_UNDERLINE * cs->underline |
              T1_OVERLINE  * cs->overline  |
              T1_KERNING   * cs->kerning;

    if (fabs(matrix.cxx - 1) < 0.01 && fabs(matrix.cyy - 1) < 0.01 &&
        fabs(matrix.cxy) < 0.01 && fabs(matrix.cyx) < 0.01) {
        matrixP = NULL;
    } else {
        matrixP = &matrix;
    }

    t1aa = T1_AA_LOW;
    mono = FALSE;
    switch (fontrast) {
    case FONT_RASTER_DEVICE:
        mono = TRUE;
        break;
    case FONT_RASTER_MONO:
        mono = TRUE;
        break;
    case FONT_RASTER_AA_HIGH:
        t1aa = T1_AA_HIGH;
        break;
    case FONT_RASTER_AA_SMART:
        if (Size < 18.0) {
            t1aa = T1_AA_HIGH;
        } else
        if (Size > 72.0) {
            mono = TRUE;
        }
        break;
    default:
        break;
    }

    /* set monomode for B/W color transformation */
    if (canvas->curdevice->color_trans == COLOR_TRANS_BW) {
        mono = TRUE;
    }
    
    if (mono != TRUE) {
        fg = cs->color;
    	bg = getbgcolor(canvas);

        set_aa_gray_values(canvas, fg, bg, t1aa);
        
    	glyph = T1_AASetString(FontID, cs->s, len,
    				   Space, modflag, Size, matrixP);
    } else {
    	glyph = T1_SetString(FontID, cs->s, len,
    				   Space, modflag, Size, matrixP);
    }
 
    return glyph;
}

static CompositeString *cstring_new(void)
{
    CompositeString *cstring;
    
    cstring = xmalloc(sizeof(CompositeString));
    if (cstring) {
        memset(cstring, 0, sizeof(CompositeString));
    }
    
    return cstring;
}

static void cstring_free(CompositeString *cs)
{
    unsigned int i = 0;
    
    for (i = 0; i < cs->nsegs; i++) {
	CStringSegment *seg = &cs->segs[i];
        xfree(seg->s);
	if (cs->cglyphs) {
            CSGlyphCache *cglyph = &cs->cglyphs[i];
            if (cglyph->glyph != NULL) {
                T1_FreeGlyph(cglyph->glyph);
            }
        }
    }
    xfree(cs->segs);
    xfree(cs->cglyphs);
    xfree(cs);
}

static const TextMatrix unit_tm = UNIT_TM;

CStringSegment *cstring_seg_new(CompositeString *cstring)
{
    CStringSegment *p, *cseg = NULL;
    
    p = xrealloc(cstring->segs, (cstring->nsegs + 1)*sizeof(CStringSegment));
    if (p) {
        cstring->segs  = p;
        cstring->nsegs++;
        
        cseg = &cstring->segs[cstring->nsegs - 1];
        memset(cseg, 0, sizeof(CStringSegment));
        
        cseg->tm       = unit_tm;
        cseg->color    = BAD_COLOR;
        cseg->font     = BAD_FONT_ID;
        cseg->setmark  = MARK_NONE;
        cseg->gotomark = MARK_NONE;
    }
    
    return cseg;
}

static CompositeString *String2Composite(Canvas *canvas, const char *s)
{
    CompositeString *cstring;
    
    cstring = cstring_new();
    
    if (cstring) {
        if (canvas->csparse_proc(canvas, s, cstring) != RETURN_SUCCESS) {
            cstring_free(cstring);
            cstring = NULL;
        } else {
            /* allocate the cache array */
            cstring->cglyphs = xcalloc(cstring->nsegs, sizeof(CSGlyphCache));
            if (!cstring->cglyphs) {
                cstring_free(cstring);
                cstring = NULL;
            }
        }
    }
    
    return cstring;
}

static void reverse_string(char *s, int len)
{
    char cbuf;
    int i;
    
    if (s == NULL) {
        return;
    }
    
    for (i = 0; i < len/2; i++) {
        cbuf = s[i];
        s[i] = s[len - i - 1];
        s[len - i - 1] = cbuf;
    }
}

static void process_ligatures(CStringSegment *cs)
{
    int j, k, l, m, none_found;
    char *ligtheString;
    char *succs, *ligs;
    char buf_char;

    ligtheString = xmalloc((cs->len + 1)*SIZEOF_CHAR);
    /* Loop through the characters */
    for (j = 0, m = 0; j < cs->len; j++, m++) {
        if ((k = T1_QueryLigs(cs->font, cs->s[j], &succs, &ligs)) > 0) {
            buf_char = cs->s[j];
            while (k > 0){
                none_found = 1;
                for (l = 0; l < k; l++) { /* Loop through the ligatures */
                    if (succs[l] == cs->s[j + 1]) {
                        buf_char = ligs[l];
                        j++;
                        none_found = 0;
                        break;
                    }
                }
                if (none_found) {
                    break;
                }
                k = T1_QueryLigs(cs->font, buf_char, &succs, &ligs);
            }
            ligtheString[m] = buf_char;
        } else { /* There are no ligatures */
            ligtheString[m] = cs->s[j];
        }
    }
    ligtheString[m] = 0;
    
    xfree(cs->s);
    cs->s = ligtheString;
    cs->len = m;
}

static int postprocess_cs(Canvas *canvas,
    CompositeString *cstring, double angle,
    double dpv, FontRaster fontrast,
    VPoint *rpoint, view *bbox)
{
    const VPoint vp_zero = {0, 0};
    VPoint cs_marks[MAX_MARKS];
    
    unsigned int iss;
    int def_font = getfont(canvas);
    int def_color = getcolor(canvas);
    int text_advancing;
    int gotomark, setmark;
 
    /* charsize (in VP units) */
    double charsize = getcharsize(canvas);

    if (charsize <= 0.0) {
        return RETURN_FAILURE;
    }

    /* zero marks */
    for (gotomark = 0; gotomark < MAX_MARKS; gotomark++) {
        cs_marks[gotomark] = vp_zero;
    }
    
    *rpoint  = vp_zero;
    memset(bbox, 0, sizeof(view));

    for (iss = 0; iss < cstring->nsegs; iss++) {
        GLYPH *glyph;
        VPoint vptmp;
	CStringSegment *cs = &cstring->segs[iss];
	CSGlyphCache *cglyph = &cstring->cglyphs[iss];
        
        /* Post-process the CS */
        if (cs->font == BAD_FONT_ID) {
            cs->font = def_font;
        }
        /* apply font mapping set by user */
        cs->font = canvas->fmap_proc(canvas, cs->font);
        
        if (cs->color == BAD_COLOR) {
            cs->color = def_color;
        }
        if (cs->ligatures == TRUE) {
            process_ligatures(cs);
        }
        if (cs->direction == STRING_DIRECTION_RL) {
            reverse_string(cs->s, cs->len);
        }
        tm_rotate(&cs->tm, angle);
        
        tm_scale(&cs->tm, charsize);
        cs->vshift *= charsize;
        cs->hshift *= charsize;
        
        text_advancing = cs->advancing;
        gotomark = cs->gotomark;
        setmark = cs->setmark;

        glyph = GetGlyphString(canvas, cs, dpv, fontrast);
        if (glyph != NULL) {
            VPoint hvpshift, vvpshift;

            if (text_advancing == TEXT_ADVANCING_RL) {
                glyph->metrics.leftSideBearing -= glyph->metrics.advanceX;
                glyph->metrics.rightSideBearing -= glyph->metrics.advanceX;
                glyph->metrics.advanceX *= -1;
                glyph->metrics.ascent  -= glyph->metrics.advanceY;
                glyph->metrics.descent -= glyph->metrics.advanceY;
                glyph->metrics.advanceY *= -1;
            }

            vvpshift.x = cs->tm.cxy*cs->vshift/tm_size(&cs->tm);
            vvpshift.y = cs->tm.cyy*cs->vshift/tm_size(&cs->tm);
            
            hvpshift.x = cs->tm.cxx*cs->hshift/tm_size(&cs->tm);
            hvpshift.y = cs->tm.cyx*cs->hshift/tm_size(&cs->tm);

            if (gotomark >= 0 && gotomark < MAX_MARKS) {
                *rpoint = cs_marks[gotomark];
            } else if (gotomark == MARK_CR) {
                /* carriage return */
                *rpoint = vp_zero;
            }

            rpoint->x += hvpshift.x;
            rpoint->y += hvpshift.y;
            
            cglyph->start = *rpoint;
            cglyph->start.x += vvpshift.x;
            cglyph->start.y += vvpshift.y;

            /* update bbox */
            vptmp.x = cglyph->start.x + (double) glyph->metrics.leftSideBearing/dpv;
            vptmp.y = cglyph->start.y + (double) glyph->metrics.descent/dpv;
            bbox->xv1 = MIN2(bbox->xv1, vptmp.x);
            bbox->yv1 = MIN2(bbox->yv1, vptmp.y);

            vptmp.x = cglyph->start.x + (double) glyph->metrics.rightSideBearing/dpv;
            vptmp.y = cglyph->start.y + (double) glyph->metrics.ascent/dpv;
            bbox->xv2 = MAX2(bbox->xv2, vptmp.x);
            bbox->yv2 = MAX2(bbox->yv2, vptmp.y);
            
            rpoint->x += (double) glyph->metrics.advanceX/dpv;
            rpoint->y += (double) glyph->metrics.advanceY/dpv;

            if (setmark >= 0 && setmark < MAX_MARKS) {
                cs_marks[setmark].x = rpoint->x;
                cs_marks[setmark].y = rpoint->y;
            }
            
            cglyph->stop = *rpoint;
            cglyph->stop.x += vvpshift.x;
            cglyph->stop.y += vvpshift.y;

            cglyph->glyph = T1_CopyGlyph(glyph);
        } else {
            cglyph->glyph = NULL;
        }
    }
    
    return RETURN_SUCCESS;
}

static int justify_cs(CompositeString *cstring, int just,
    view *bbox, const VPoint *baseline_advance)
{
    int hjust, vjust;
    double hfudge, vfudge;
    VPoint offset;
    unsigned int iss;

    hjust = just & 03;
    switch (hjust) {
    case JUST_LEFT:
        hfudge = 0.0;
        break;
    case JUST_RIGHT:
        hfudge = 1.0;
        break;
    case JUST_CENTER:
        hfudge = 0.5;
        break;
    default:
        errmsg("Wrong justification type of string");
        return RETURN_FAILURE;
    }

    vjust = just & 014;
    switch (vjust) {
    case JUST_BOTTOM:
        vfudge = 0.0;
        break;
    case JUST_TOP:
        vfudge = 1.0;
        break;
    case JUST_MIDDLE:
        vfudge = 0.5;
        break;
    case JUST_BLINE:
        /* Not used; to make compiler happy */
        vfudge = 0.0;
        break;
    default:
        /* This can't happen; to make compiler happy */
        errmsg("Internal error");
        return RETURN_FAILURE;
    }
    
    if (vjust == JUST_BLINE) {
        offset.x = hfudge*baseline_advance->x;
        offset.y = hfudge*baseline_advance->y;
    } else {
        offset.x = bbox->xv1 + hfudge*(bbox->xv2 - bbox->xv1);
        offset.y = bbox->yv1 + vfudge*(bbox->yv2 - bbox->yv1);
    }
    
    /* justification corrections */
    for (iss = 0; iss < cstring->nsegs; iss++) {
	CSGlyphCache *cglyph = &cstring->cglyphs[iss];
        if (cglyph->glyph == NULL) {
            continue;
        }
        cglyph->start.x -= offset.x;
        cglyph->start.y -= offset.y;
        cglyph->stop.x  -= offset.x;
        cglyph->stop.y  -= offset.y;
    }
        
    /* update BB */
    bbox->xv1 -= offset.x;
    bbox->yv1 -= offset.y;
    bbox->xv2 -= offset.x;
    bbox->yv2 -= offset.y;
    
    return RETURN_SUCCESS;
}

CompositeString *rasterize_string(Canvas *canvas,
    const VPoint *vp, double angle, int just, FontRaster fontrast, double dpv,
    const char *s, view *bbox)
{
    CompositeString *cstring;
 
    VPoint baseline_advance;
 
    bbox->xv1 = bbox->xv2 = vp->x;
    bbox->yv1 = bbox->yv2 = vp->y;
    
    if (is_empty_string(s)) {
	return NULL;
    }
    
    cstring = String2Composite(canvas, s);
    if (cstring == NULL) {
        return NULL;
    }
    
    if (postprocess_cs(canvas, cstring,
                       angle, dpv, fontrast,
                       &baseline_advance, bbox) != RETURN_SUCCESS ||
        justify_cs(cstring, just, bbox, &baseline_advance) != RETURN_SUCCESS) {
        cstring_free(cstring);
        return NULL;
    }
    
    /* update BB */
    bbox->xv1 += vp->x;
    bbox->yv1 += vp->y;
    bbox->xv2 += vp->x;
    bbox->yv2 += vp->y;

    return cstring;
}

void WriteString(Canvas *canvas,
    const VPoint *vp, double angle, int just, const char *theString)
{
    CompositeString *cstring;
    unsigned int iss;
 
    FontRaster fontrast;
    double ipv, dpv;
 
    view bbox;
 
    fontrast = (get_curdevice_props(canvas))->fontrast;

    /* inches per 1 unit of viewport */
    ipv = MIN2(page_width_in(canvas), page_height_in(canvas));
    /* dots per 1 unit of viewport */
    dpv = ipv*page_dpi(canvas);

    cstring = rasterize_string(canvas,
        vp, angle, just, fontrast, dpv, theString, &bbox);
    
    if (!cstring) {
        return;
    }
    
    update_bboxes_with_view(canvas, &bbox);
        
    for (iss = 0; iss < cstring->nsegs; iss++) {
        int pheight, pwidth;
        CStringSegment *cs = &cstring->segs[iss];
	CSGlyphCache *cglyph = &cstring->cglyphs[iss];
        GLYPH *glyph = cglyph->glyph;
    
        if (glyph == NULL) {
            continue;
        }
        
        pheight = glyph->metrics.ascent - glyph->metrics.descent;
        pwidth  = glyph->metrics.rightSideBearing -
                  glyph->metrics.leftSideBearing;
        if (pheight <= 0 || pwidth <= 0) {
            continue;
        }
        
        if (get_draw_mode(canvas) == TRUE) {
            VPoint vptmp;
            
            /* No patterned texts yet */
            setpattern(canvas, 1);
            setcolor(canvas, cs->color);

            vptmp = *vp;
            
            if (fontrast == FONT_RASTER_DEVICE) {
                if (cs->advancing == TEXT_ADVANCING_RL) {
                    vptmp.x += cglyph->stop.x;
                    vptmp.y += cglyph->stop.y;
                } else {
                    vptmp.x += cglyph->start.x;
                    vptmp.y += cglyph->start.y;
                }
                canvas_dev_puttext(canvas, &vptmp, cs->s, cs->len, cs->font,
                    &cs->tm, cs->underline, cs->overline, cs->kerning);
            } else {
                CPixmap pm;
                
                /* upper left corner of bitmap */
                vptmp.x += cglyph->start.x;
                vptmp.y += cglyph->start.y;
                vptmp.x += (double) glyph->metrics.leftSideBearing/dpv;
                vptmp.y += (double) glyph->metrics.ascent/dpv;

                pm.width  = pwidth;
                pm.height = pheight;
                pm.bits   = glyph->bits;
                pm.bpp    = glyph->bpp;
                pm.pad    = T1_GetBitmapPad();
                pm.type   = PIXMAP_TRANSPARENT;
                canvas_dev_putpixmap(canvas, &vptmp, &pm);
            }
        }
    }

    cstring_free(cstring);
}

int get_string_bbox(Canvas *canvas,
    const VPoint *vp, double angle, int just, const char *s, view *bbox)
{
    CompositeString *cstring;
 
    cstring = rasterize_string(canvas,
        vp, angle, just, FONT_RASTER_MONO, 612.0, s, bbox);
    
    if (cstring) {
        cstring_free(cstring);
        
        return RETURN_SUCCESS;
    } else {
	return RETURN_FAILURE;
    }
}
