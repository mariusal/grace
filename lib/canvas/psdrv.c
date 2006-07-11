/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1996-2005 Grace Development Team
 * 
 * Maintained by Evgeny Stambulchik
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

/*
 *  Grace PostScript driver
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "grace/baseP.h"
#define CANVAS_BACKEND_API
#include "grace/canvas.h"

typedef struct {
    int embed;
    char *name;
} PSFont;

#define PS_DEFAULT_COLORSPACE  PS_COLORSPACE_RGB

#define MAX_PS_LINELEN   70

static void put_string(PS_data *psdata, FILE *fp, const char *s, int len);

static PS_data *init_ps_data(int format)
{
    PS_data *data;

    /* we need to perform the allocations */
    data = (PS_data *) xmalloc(sizeof(PS_data));
    if (data == NULL) {
        return NULL;
    }

    memset(data, 0, sizeof(PS_data));

    data->format     = format;

    data->level2     = TRUE;
    data->colorspace = PS_DEFAULT_COLORSPACE;
    data->docdata    = PS_DOCDATA_8BIT;
    data->fonts      = PS_FONT_EMBED_BUT35;
    data->printable  = FALSE;

    data->offset_x   = 0;
    data->offset_y   = 0;
    data->feed       = PS_MEDIA_FEED_AUTO;
    data->hwres      = FALSE;

    return data;
}

static char *ps_standard_fonts13[] = 
{
    "Times-Roman",
    "Times-Italic",
    "Times-Bold",
    "Times-BoldItalic",
    
    "Helvetica",
    "Helvetica-Oblique",
    "Helvetica-Bold",
    "Helvetica-BoldOblique",
    
    "Courier",
    "Courier-Oblique",
    "Courier-Bold",
    "Courier-BoldOblique",
    
    "Symbol",
    
    "ZapfDingbats"
};

static int number_of_ps_standard_fonts13 =
    sizeof(ps_standard_fonts13)/SIZEOF_VOID_P;

static char *ps_standard_fonts35[] = 
{
    "Times-Roman",
    "Times-Italic",
    "Times-Bold",
    "Times-BoldItalic",
    
    "Helvetica",
    "Helvetica-Oblique",
    "Helvetica-Bold",
    "Helvetica-BoldOblique",
    
    "Courier",
    "Courier-Oblique",
    "Courier-Bold",
    "Courier-BoldOblique",
    
    "Bookman-Demi",
    "Bookman-DemiItalic",
    "Bookman-Light",
    "Bookman-LightItalic",

    "AvantGarde-Book",
    "AvantGarde-BookOblique",
    "AvantGarde-Demi",
    "AvantGarde-DemiOblique",

    "Helvetica-Narrow",
    "Helvetica-Narrow-Oblique",
    "Helvetica-Narrow-Bold",
    "Helvetica-Narrow-BoldOblique",

    "Palatino-Roman",
    "Palatino-Italic",
    "Palatino-Bold",
    "Palatino-BoldItalic",

    "NewCenturySchlbk-Roman",
    "NewCenturySchlbk-Italic",
    "NewCenturySchlbk-Bold",
    "NewCenturySchlbk-BoldItalic",
    
    "Symbol",

    "ZapfDingbats",

    "ZapfChancery-MediumItalic"
};

static int number_of_ps_standard_fonts35 =
    sizeof(ps_standard_fonts35)/SIZEOF_VOID_P;

static int ps_embedded_font(const char *fname, int embed_type)
{
    int i;
    switch (embed_type) {
    case PS_FONT_EMBED_NONE:
        return FALSE;
        break;
    case PS_FONT_EMBED_BUT13:
        for (i = 0; i < number_of_ps_standard_fonts13; i++) {
            if (strcmp(ps_standard_fonts13[i], fname) == 0) {
                return FALSE;
            }
        }
        return TRUE;
        break;
    case PS_FONT_EMBED_BUT35:
        for (i = 0; i < number_of_ps_standard_fonts35; i++) {
            if (strcmp(ps_standard_fonts35[i], fname) == 0) {
                return FALSE;
            }
        }
        return TRUE;
        break;
    case PS_FONT_EMBED_ALL:
    default:
        return TRUE;
        break;
    }
}

static int ps_initgraphics(const Canvas *canvas, void *data,
    const CanvasStats *cstats)
{
    PS_data *psdata = (PS_data *) data;
    unsigned int i, j, first;
    Page_geometry *pg;
    int width_pp, height_pp;
    double clip_width, clip_height;
    PSFont *psfonts;
    int font_needed_any = FALSE, font_embed_any = FALSE;
    char **enc;
    view v;
    double llx, lly, urx, ury;
    FILE *prstream = canvas_get_prstream(canvas);
    
    time_t time_value;
    
    pg = get_page_geometry(canvas);
    
    psdata->page_scale = MIN2(pg->height, pg->width);
    psdata->pixel_size = 1.0/psdata->page_scale;
    psdata->page_scalef = (float) psdata->page_scale*72.0/pg->dpi;

    if (psdata->format == PS_FORMAT && pg->height < pg->width) {
        psdata->page_orientation = PAGE_ORIENT_LANDSCAPE;
    } else {
        psdata->page_orientation = PAGE_ORIENT_PORTRAIT;
    }
    
    /* undefine all graphics state parameters */
    psdata->color = -1;
    psdata->pattern = -1;
    psdata->linew = -1.0;
    psdata->lines = -1;
    psdata->linecap = -1;
    psdata->linejoin = -1;
    
    /* CMYK is a PS2 feature */
    if (psdata->level2 == FALSE && psdata->colorspace == PS_COLORSPACE_CMYK) {
        psdata->colorspace = PS_COLORSPACE_RGB;
    }

    if (psdata->format == EPS_FORMAT) {
        fprintf(prstream, "%%!PS-Adobe-3.0 EPSF-3.0\n");
    } else {
        fprintf(prstream, "%%!PS-Adobe-3.0\n");
    }

    if (psdata->page_orientation == PAGE_ORIENT_LANDSCAPE) {
        width_pp  = (int) rint(72.0*pg->height/pg->dpi);
        height_pp = (int) rint(72.0*pg->width/pg->dpi);
    } else {
        width_pp  = (int) rint(72.0*pg->width/pg->dpi);
        height_pp = (int) rint(72.0*pg->height/pg->dpi);
    }

    if (pg->height < pg->width) {
        clip_width =  (double) pg->width/pg->height;
        clip_height = 1.0;
    } else {
        clip_width =  1.0;
        clip_height = (double) pg->height/pg->width;
    }
    
    v = cstats->bbox;
    v.xv1 = MAX2(0.0,         v.xv1);
    v.yv1 = MAX2(0.0,         v.yv1);
    v.xv2 = MIN2(clip_width,  v.xv2);
    v.yv2 = MIN2(clip_height, v.yv2);
    
    if (psdata->page_orientation == PAGE_ORIENT_LANDSCAPE) {
        llx = psdata->page_scalef*(1.0 - v.yv2);
        lly = psdata->page_scalef*v.xv1;
        urx = psdata->page_scalef*(1.0 - v.yv1);
        ury = psdata->page_scalef*v.xv2;
    } else {
        llx = psdata->page_scalef*v.xv1;
        lly = psdata->page_scalef*v.yv1;
        urx = psdata->page_scalef*v.xv2;
        ury = psdata->page_scalef*v.yv2;
    }
    fprintf(prstream, "%%%%BoundingBox: %d %d %d %d\n",
        (int) floor(llx), (int) floor(lly), (int) ceil(urx), (int) ceil(ury));
    fprintf(prstream, "%%%%HiResBoundingBox: %.2f %.2f %.2f %.2f\n",
        llx, lly, urx, ury);

    if (psdata->level2 == TRUE) {
        fprintf(prstream, "%%%%LanguageLevel: 2\n");
    } else {
        fprintf(prstream, "%%%%LanguageLevel: 1\n");
    }
    
    fprintf(prstream, "%%%%Creator: %s\n", "Grace/libcanvas");

    time(&time_value);
    fprintf(prstream, "%%%%CreationDate: %s", ctime(&time_value));
    switch (psdata->docdata) {
    case PS_DOCDATA_7BIT:
        fprintf(prstream, "%%%%DocumentData: Clean7Bit\n");
        break;
    case PS_DOCDATA_8BIT:
        fprintf(prstream, "%%%%DocumentData: Clean8Bit\n");
        break;
    default:
        fprintf(prstream, "%%%%DocumentData: Binary\n");
        break;
    }
    if (psdata->page_orientation == PAGE_ORIENT_LANDSCAPE) {
        fprintf(prstream, "%%%%Orientation: Landscape\n");
    } else {
        fprintf(prstream, "%%%%Orientation: Portrait\n");
    }
    
    if (psdata->format == PS_FORMAT) {
        fprintf(prstream, "%%%%Pages: 1\n");
        fprintf(prstream, "%%%%PageOrder: Ascend\n");
    }
    fprintf(prstream, "%%%%Title: %s\n", canvas_get_docname(canvas));
    fprintf(prstream, "%%%%For: %s\n", canvas_get_username(canvas));
    
    psfonts = xmalloc(cstats->nfonts*sizeof(PSFont));
    for (i = 0; i < cstats->nfonts; i++) {
        int font = cstats->fonts[i].font;
        char *fontalias = get_fontalias(canvas, font);
        if (ps_embedded_font(fontalias, psdata->fonts)) {
            char *fontname = get_fontname(canvas, font);
            psfonts[i].embed = TRUE;
            psfonts[i].name  = copy_string(NULL, fontname);
            font_embed_any = TRUE;
        } else {
            psfonts[i].embed = FALSE;
            psfonts[i].name  = copy_string(NULL, fontalias);
            font_needed_any = TRUE;
        }
    }
    
    if (font_needed_any) {
        first = TRUE;
        for (i = 0; i < cstats->nfonts; i++) {
            if (!psfonts[i].embed) {
                if (first) {
                    fprintf(prstream, "%%%%DocumentNeededResources:");
                    first = FALSE;
                } else {
                    fprintf(prstream, "%%%%+");
                }
                fprintf(prstream, " font %s\n", psfonts[i].name);
            }
        }
    }

    if (font_embed_any) {
        first = TRUE;
        for (i = 0; i < cstats->nfonts; i++) {
            if (psfonts[i].embed) {
                if (first) {
                    fprintf(prstream, "%%%%DocumentSuppliedResources:");
                    first = FALSE;
                } else {
                    fprintf(prstream, "%%%%+");
                }
                fprintf(prstream, " font %s\n", psfonts[i].name);
            }
        }
    }

    fprintf(prstream, "%%%%EndComments\n");

    /* Definitions */
    fprintf(prstream, "%%%%BeginProlog\n");
    if (psdata->format == PS_FORMAT) {
        fprintf(prstream, "/PAGE_OFFSET_X %d def\n", psdata->offset_x);
        fprintf(prstream, "/PAGE_OFFSET_Y %d def\n", psdata->offset_y);
    }
    fprintf(prstream, "/m {moveto} def\n");
    fprintf(prstream, "/l {lineto} def\n");
    fprintf(prstream, "/s {stroke} def\n");
    fprintf(prstream, "/n {newpath} def\n");
    fprintf(prstream, "/c {closepath} def\n");
    fprintf(prstream, "/RL {rlineto} def\n");
    fprintf(prstream, "/SLW {setlinewidth} def\n");
    fprintf(prstream, "/GS {gsave} def\n");
    fprintf(prstream, "/GR {grestore} def\n");
    fprintf(prstream, "/SC {setcolor} def\n");
    fprintf(prstream, "/SGRY {setgray} def\n");
    fprintf(prstream, "/SRGB {setrgbcolor} def\n");
    if (psdata->colorspace == PS_COLORSPACE_CMYK) {
        fprintf(prstream, "/SCMYK {setcmykcolor} def\n");
    }
    fprintf(prstream, "/SD {setdash} def\n");
    fprintf(prstream, "/SLC {setlinecap} def\n");
    fprintf(prstream, "/SLJ {setlinejoin} def\n");
    fprintf(prstream, "/SCS {setcolorspace} def\n");
    fprintf(prstream, "/FFSF {findfont setfont} def\n");
    fprintf(prstream, "/CC {concat} def\n");
    fprintf(prstream, "/PXL {n m 0 0 RL s} def\n");
    
    for (i = 0; i < cstats->ncolors; i++) {
        int cindex = cstats->colors[i];
        fprintf(prstream,"/Color%d {", cindex);
        switch (psdata->colorspace) {
        case PS_COLORSPACE_GRAYSCALE:
            fprintf(prstream,"%.4f", get_colorintensity(canvas, cindex));
            break;
        case PS_COLORSPACE_RGB:
            {
                fRGB frgb;
                if (get_frgb(canvas, cindex, &frgb) == RETURN_SUCCESS) {
                    fprintf(prstream, "%.4f %.4f %.4f",
                                      frgb.red, frgb.green, frgb.blue);
                }
            }
            break;
        case PS_COLORSPACE_CMYK:
            {
                fCMYK fcmyk;
                if (get_fcmyk(canvas, cindex, &fcmyk) == RETURN_SUCCESS) {
                    fprintf(prstream, "%.4f %.4f %.4f %.4f",
                                      fcmyk.cyan, fcmyk.magenta,
                                      fcmyk.yellow, fcmyk.black);
                }
            }
            break;
        }
        fprintf(prstream,"} def\n");
    }

    for (i = 0; i < cstats->nfonts; i++) {
        if (psfonts[i].embed) {
            int font = cstats->fonts[i].font;
            char *fontdata;
            unsigned long datalen;
            
            fontdata = font_subset(canvas,
                font, cstats->fonts[i].chars_used, &datalen);
            if (fontdata) {
                fprintf(prstream, "%%%%BeginResource: font %s\n",
                    psfonts[i].name);
                fwrite(fontdata, 1, datalen, prstream);
                fprintf(prstream, "%%%%EndResource\n");
                xfree(fontdata);
            } else {
                errmsg("Font subsetting failed");
            }
        }
    }

    if (cstats->nfonts > 0) {
        /* Default encoding */
        enc = get_default_encoding(canvas);
        fprintf(prstream, "/DefEncoding [\n");
        for (i = 0; i < 256; i++) {
            fprintf(prstream, " /%s\n", enc[i]);
        }
        fprintf(prstream, "] def\n");
    }

    for (i = 0; i < cstats->nfonts; i++) {
        int font = cstats->fonts[i].font;
        char *encscheme = get_encodingscheme(canvas, font);

        fprintf(prstream, "/%s findfont\n", psfonts[i].name);
        if (strcmp(encscheme, "FontSpecific") != 0) {
            fprintf(prstream, "dup length dict begin\n");
            fprintf(prstream, " {1 index /FID ne {def} {pop pop} ifelse} forall\n");
            fprintf(prstream, " /Encoding DefEncoding def\n");
            fprintf(prstream, " currentdict\n");
            fprintf(prstream, "end\n");
        }
        fprintf(prstream, "/Font%d exch definefont pop\n", font);
    }
       
    if (psdata->level2 == TRUE && cstats->npatterns) {
        fprintf(prstream, "/PTRN {\n");
        fprintf(prstream, " /pat_bits exch def \n");
        fprintf(prstream, " /height exch def \n");
        fprintf(prstream, " /width exch def \n");
        fprintf(prstream, " <<\n");
        fprintf(prstream, "  /PaintType 2\n");
        fprintf(prstream, "  /PatternType 1 /TilingType 1\n");
        fprintf(prstream, "  /BBox[0 0 width height]\n");
        fprintf(prstream, "  /XStep width /YStep height\n");
        fprintf(prstream, "  /PaintProc {\n");
        fprintf(prstream, "   pop\n");
        fprintf(prstream, "   width height true [-1 0 0 -1 width height] pat_bits imagemask\n");
        fprintf(prstream, "  }\n");
        fprintf(prstream, " >>\n");
        fprintf(prstream, " [%.4f 0 0 %.4f 0 0]\n", 1.0/psdata->page_scalef, 1.0/psdata->page_scalef);
        fprintf(prstream, " makepattern\n");
        fprintf(prstream, "} def\n");
        for (i = 0; i < cstats->npatterns; i++) {
            int patno = cstats->patterns[i];
            Pattern *pat = canvas_get_pattern(canvas, patno);
            fprintf(prstream, "/Pattern%d {%d %d <",
                patno, pat->width, pat->height);
            for (j = 0; j < pat->width*pat->height/8; j++) {
                fprintf(prstream, "%02x", pat->bits[j]);
            }
            fprintf(prstream, "> PTRN} bind def\n");
        }
    }
    
    /* Elliptic arc */
    fprintf(prstream, "/ellipsedict 9 dict def\n");
    fprintf(prstream, "ellipsedict /mtrx matrix put\n");
    fprintf(prstream, "/EARC {\n");
    fprintf(prstream, " ellipsedict begin\n");
    fprintf(prstream, "  /pathangle exch def\n");
    fprintf(prstream, "  /startangle exch def\n");
    fprintf(prstream, "  /yrad exch def\n");
    fprintf(prstream, "  /xrad exch def\n");
    fprintf(prstream, "  /y exch def\n");
    fprintf(prstream, "  /x exch def\n");
    fprintf(prstream, "  /savematrix mtrx currentmatrix def\n");
    fprintf(prstream, "  /endangle startangle pathangle add def\n");
    fprintf(prstream, "  x y translate\n");
    fprintf(prstream, "  xrad yrad scale\n");
    fprintf(prstream, "  0 0 1 startangle endangle\n");
    fprintf(prstream, "  pathangle 0 lt {arcn} {arc} ifelse\n");
    fprintf(prstream, "  savematrix setmatrix\n");
    fprintf(prstream, " end\n");
    fprintf(prstream, "} def\n");

    /* Text under/overlining etc */
    fprintf(prstream, "/TL {\n");
    fprintf(prstream, "  /kcomp exch def\n");
    fprintf(prstream, "  /linewidth exch def\n");
    fprintf(prstream, "  /offset exch def\n");
    fprintf(prstream, "  GS\n");
    fprintf(prstream, "  0 offset rmoveto\n");
    fprintf(prstream, "  linewidth SLW\n");
    fprintf(prstream, "  dup stringwidth exch kcomp add exch RL s\n");
    fprintf(prstream, "  GR\n");
    fprintf(prstream, "} def\n");

    /* Kerning stuff */
    fprintf(prstream, "/KINIT\n");
    fprintf(prstream, "{\n");
    fprintf(prstream, " /kvector exch def\n");
    fprintf(prstream, " /kid 0 def\n");
    fprintf(prstream, "} def\n");
    fprintf(prstream, "/KPROC\n");
    fprintf(prstream, "{\n");
    fprintf(prstream, " pop pop\n");
    fprintf(prstream, " kvector kid get\n");
    fprintf(prstream, " 0 rmoveto\n");
    fprintf(prstream, " /kid 1 kid add def\n");
    fprintf(prstream, "} def\n");

    fprintf(prstream, "%%%%EndProlog\n");

    fprintf(prstream, "%%%%BeginSetup\n");
    if (psdata->level2 == TRUE && psdata->format == PS_FORMAT) {
        /* page size feed */
        switch (psdata->feed) {
        case PS_MEDIA_FEED_AUTO:
            break;
        case PS_MEDIA_FEED_MATCH:
            fprintf(prstream, "%%%%BeginFeature: *PageSize\n");
            fprintf(prstream,
                "<</PageSize [%d %d] /ImagingBBox null>> setpagedevice\n",
                width_pp, height_pp);
            fprintf(prstream, "%%%%EndFeature\n");
            break;
        case PS_MEDIA_FEED_MANUAL:
            fprintf(prstream, "%%%%BeginFeature: *ManualFeed\n");
            fprintf(prstream, "<</ManualFeed true>> setpagedevice\n");
            fprintf(prstream, "%%%%EndFeature\n");
            break;
        }
        
        /* force HW resolution */
        if (psdata->hwres == TRUE) {
            fprintf(prstream, "%%%%BeginFeature: *HWResolution\n");
            fprintf(prstream, "<</HWResolution [%d %d]>> setpagedevice\n",
                (int) pg->dpi, (int) pg->dpi);
            fprintf(prstream, "%%%%EndFeature\n");
        }
    }
    fprintf(prstream, "%%%%EndSetup\n");

    if (psdata->format == PS_FORMAT) {
        fprintf(prstream, "%%%%Page: 1 1\n");
    }

    /* compensate for printer page offsets */
    if (psdata->format == PS_FORMAT) {
        fprintf(prstream, "PAGE_OFFSET_X PAGE_OFFSET_Y translate\n");
    }
    fprintf(prstream, "%.2f %.2f scale\n", psdata->page_scalef, psdata->page_scalef);
    /* rotate to get landscape on hardcopy */
    if (psdata->page_orientation == PAGE_ORIENT_LANDSCAPE) {
        fprintf(prstream, "90 rotate\n");
        fprintf(prstream, "0.0 -1.0 translate\n");
    }

    /* clip by the page dimensions */
    fprintf(prstream, "0 0 %.4f %.4f rectclip\n", clip_width, clip_height);

    /* free the psfonts array */
    for (i = 0; i < cstats->nfonts; i++) {
        xfree(psfonts[i].name);
    }
    xfree(psfonts);

    return RETURN_SUCCESS;
}

static void ps_setpen(const Canvas *canvas, const Pen *pen, PS_data *psdata)
{
    if (pen->color != psdata->color || pen->pattern != psdata->pattern) {
        FILE *prstream = canvas_get_prstream(canvas);
        if (psdata->level2 == TRUE) {
            if (pen->pattern == 1) {
                switch (psdata->colorspace) {
                case PS_COLORSPACE_GRAYSCALE:
                    fprintf(prstream, "[/DeviceGray] SCS\n");
                    break;
                case PS_COLORSPACE_RGB:
                    fprintf(prstream, "[/DeviceRGB] SCS\n");
                    break;
                case PS_COLORSPACE_CMYK:
                    fprintf(prstream, "[/DeviceCMYK] SCS\n");
                    break;
                }
                fprintf(prstream, "Color%d SC\n", pen->color);
            } else {
                switch (psdata->colorspace) {
                case PS_COLORSPACE_GRAYSCALE:
                    fprintf(prstream, "[/Pattern /DeviceGray] SCS\n");
                    break;
                case PS_COLORSPACE_RGB:
                    fprintf(prstream, "[/Pattern /DeviceRGB] SCS\n");
                    break;
                case PS_COLORSPACE_CMYK:
                    fprintf(prstream, "[/Pattern /DeviceCMYK] SCS\n");
                    break;
                }
                fprintf(prstream,
                    "Color%d Pattern%d SC\n", pen->color, pen->pattern);
            }
        } else {
            if (psdata->colorspace == PS_COLORSPACE_GRAYSCALE) {
                fprintf(prstream, "Color%d SGRY\n", pen->color);
            } else {
                fprintf(prstream, "Color%d SRGB\n", pen->color);
            }
        }
        psdata->color = pen->color;
        psdata->pattern = pen->pattern;
    }
}

static void ps_setdrawbrush(const Canvas *canvas, PS_data *psdata)
{
    unsigned int i;
    int ls;
    double lw;
    Pen pen;
    
    getpen(canvas, &pen);
    ps_setpen(canvas, &pen, psdata);

    ls = getlinestyle(canvas);
    lw = MAX2(getlinewidth(canvas), psdata->pixel_size);
    
    if (ls != psdata->lines || lw != psdata->linew) {    
        FILE *prstream = canvas_get_prstream(canvas);
        fprintf(prstream, "[");
        if (ls > 1) {
            LineStyle *linestyle = canvas_get_linestyle(canvas, ls);
            for (i = 0; i < linestyle->length; i++) {
                fprintf(prstream, "%.4f ", lw*linestyle->array[i]);
            }
        }
        fprintf(prstream, "] 0 SD\n");
        fprintf(prstream, "%.4f SLW\n", lw);
        psdata->linew = lw;
        psdata->lines = ls;
    }
}

static void ps_setlineprops(const Canvas *canvas, PS_data *psdata)
{
    int lc, lj;
    FILE *prstream = canvas_get_prstream(canvas);
    
    lc = getlinecap(canvas);
    lj = getlinejoin(canvas);
    
    if (lc != psdata->linecap) {
        switch (lc) {
        case LINECAP_BUTT:
            fprintf(prstream, "0 SLC\n");
            break;
        case LINECAP_ROUND:
            fprintf(prstream, "1 SLC\n");
            break;
        case LINECAP_PROJ:
            fprintf(prstream, "2 SLC\n");
            break;
        }
        psdata->linecap = lc;
    }

    if (lj != psdata->linejoin) {
        switch (lj) {
        case LINEJOIN_MITER:
            fprintf(prstream, "0 SLJ\n");
            break;
        case LINEJOIN_ROUND:
            fprintf(prstream, "1 SLJ\n");
            break;
        case LINEJOIN_BEVEL:
            fprintf(prstream, "2 SLJ\n");
            break;
        }
        psdata->linejoin = lj;
    }
}

static void ps_drawpixel(const Canvas *canvas, void *data, const VPoint *vp)
{
    PS_data *psdata = (PS_data *) data;
    Pen pen;
    FILE *prstream = canvas_get_prstream(canvas);
    
    getpen(canvas, &pen);
    ps_setpen(canvas, &pen, psdata);
    
    if (psdata->linew != psdata->pixel_size) {
        fprintf(prstream, "%.4f SLW\n", psdata->pixel_size);
        psdata->linew = psdata->pixel_size;
    }
    if (psdata->linecap != LINECAP_ROUND) {
        fprintf(prstream, "1 SLC\n");
        psdata->linecap = LINECAP_ROUND;
    }
    if (psdata->lines != 1) {
        fprintf(prstream, "[] 0 SD\n");
        psdata->lines = 1;
    }
    
    fprintf(prstream, "%.4f %.4f PXL\n", vp->x, vp->y);
}

static void ps_drawpolyline(const Canvas *canvas, void *data,
    const VPoint *vps, int n, int mode)
{
    PS_data *psdata = (PS_data *) data;
    int i;
    FILE *prstream = canvas_get_prstream(canvas);
    
    ps_setdrawbrush(canvas, psdata);
    
    ps_setlineprops(canvas, psdata);
    
    fprintf(prstream, "n\n");
    fprintf(prstream, "%.4f %.4f m\n", vps[0].x, vps[0].y);
    for (i = 1; i < n; i++) {
        fprintf(prstream, "%.4f %.4f l\n", vps[i].x, vps[i].y);
    }
    if (mode == POLYLINE_CLOSED) {
        fprintf(prstream, "%.4f %.4f l\n", vps[0].x, vps[0].y);
        fprintf(prstream, "c\n");
    }
    fprintf(prstream, "s\n");
}

static void ps_fillpolygon(const Canvas *canvas, void *data,
    const VPoint *vps, int nc)
{
    PS_data *psdata = (PS_data *) data;
    int i;
    Pen pen;
    FILE *prstream = canvas_get_prstream(canvas);

    getpen(canvas, &pen);
    
    if (pen.pattern == 0 || nc < 3) {
        return;
    }
    
    fprintf(prstream, "n\n");
    fprintf(prstream, "%.4f %.4f m\n", vps[0].x, vps[0].y);
    for (i = 1; i < nc; i++) {
        fprintf(prstream, "%.4f %.4f l\n", vps[i].x, vps[i].y);
    }
    fprintf(prstream, "c\n");

    /* fill bg first if the pattern != solid */
    if (pen.pattern != 1 && psdata->level2 == TRUE) {
        Pen bgpen;
        bgpen.color   = getbgcolor(canvas);
        bgpen.pattern = 1;
        fprintf(prstream, "GS\n");
        ps_setpen(canvas, &bgpen, psdata);
        fprintf(prstream, "fill\n");
        fprintf(prstream, "GR\n");
    }
    
    getpen(canvas, &pen);
    ps_setpen(canvas, &pen, psdata);
    if (getfillrule(canvas) == FILLRULE_WINDING) {
        fprintf(prstream, "fill\n");
    } else {
        fprintf(prstream, "eofill\n");
    }
}

static void ps_drawarc(const Canvas *canvas, void *data,
    const VPoint *vp1, const VPoint *vp2, double a1, double a2)
{
    PS_data *psdata = (PS_data *) data;
    VPoint vpc;
    double rx, ry;
    FILE *prstream = canvas_get_prstream(canvas);
    
    ps_setdrawbrush(canvas, psdata);

    vpc.x = (vp1->x + vp2->x)/2;
    vpc.y = (vp1->y + vp2->y)/2;
    rx = fabs(vp2->x - vp1->x)/2;
    ry = fabs(vp2->y - vp1->y)/2;
    
    fprintf(prstream, "n %.4f %.4f %.4f %.4f %.4f %.4f EARC s\n",
                       vpc.x, vpc.y, rx, ry, a1, a2);
}

static void ps_fillarc(const Canvas *canvas, void *data,
    const VPoint *vp1, const VPoint *vp2, double a1, double a2, int mode)
{
    PS_data *psdata = (PS_data *) data;
    VPoint vpc;
    double rx, ry;
    Pen pen;
    FILE *prstream = canvas_get_prstream(canvas);

    getpen(canvas, &pen);
    
    if (pen.pattern == 0) {
        return;
    }

    vpc.x = (vp1->x + vp2->x)/2;
    vpc.y = (vp1->y + vp2->y)/2;
    rx = fabs(vp2->x - vp1->x)/2;
    ry = fabs(vp2->y - vp1->y)/2;
    
    fprintf(prstream, "n\n");
    
    if (mode == ARCCLOSURE_PIESLICE) {
        fprintf(prstream, "%.4f %.4f m\n", vpc.x, vpc.y);
    }
    fprintf(prstream, "%.4f %.4f %.4f %.4f %.4f %.4f EARC c\n",
                       vpc.x, vpc.y, rx, ry, a1, a2);

    /* fill bg first if the pattern != solid */
    if (pen.pattern != 1 && psdata->level2 == TRUE) {
        Pen bgpen;
        bgpen.color   = getbgcolor(canvas);
        bgpen.pattern = 1;
        fprintf(prstream, "GS\n");
        ps_setpen(canvas, &bgpen, psdata);
        fprintf(prstream, "fill\n");
        fprintf(prstream, "GR\n");
    }

    getpen(canvas, &pen);
    ps_setpen(canvas, &pen, psdata);
    fprintf(prstream, "fill\n");
}

static void ps_putpixmap(const Canvas *canvas, void *data,
    const VPoint *vp, const CPixmap *pm)
{
    PS_data *psdata = (PS_data *) data;
    int j, k;
    int cindex;
    int paddedW;
    fRGB frgb;
    fCMYK fcmyk;
    unsigned char tmpbyte;
    Pen pen;
    int linelen;
    FILE *prstream = canvas_get_prstream(canvas);

    getpen(canvas, &pen);
    ps_setpen(canvas, &pen, psdata);
    
    fprintf(prstream, "GS\n");
    fprintf(prstream, "%.4f %.4f translate\n", vp->x, vp->y);
    fprintf(prstream, "%.4f %.4f scale\n",
        (float) pm->width/psdata->page_scale,
        (float) pm->height/psdata->page_scale);    
    if (pm->bpp != 1) {
        int layers = 1, bpp = 8;
        if (pm->type == PIXMAP_TRANSPARENT) {
            /* TODO: mask */
        }
        switch (psdata->colorspace) {
        case PS_COLORSPACE_GRAYSCALE:
            layers = 1;
            bpp = 8;
            break;
        case PS_COLORSPACE_RGB:
            layers = 3;
            bpp = CANVAS_BPCC;
            break;
        case PS_COLORSPACE_CMYK:
            layers = 4;
            bpp = CANVAS_BPCC;
            break;
        }
        fprintf(prstream, "/picstr %d string def\n", pm->width*layers);
        fprintf(prstream, "%d %d %d\n", pm->width, pm->height, bpp);
        fprintf(prstream, "[%d 0 0 %d 0 0]\n", pm->width, -pm->height);
        fprintf(prstream, "{currentfile picstr readhexstring pop}\n");
        if (psdata->colorspace == PS_COLORSPACE_GRAYSCALE || psdata->level2 == FALSE) {
            /* No color images in Level1 */
            fprintf(prstream, "image\n");
        } else {
            fprintf(prstream, "false %d\n", layers);
            fprintf(prstream, "colorimage\n");
        }
        for (k = 0; k < pm->height; k++) {
            linelen = 0;
            for (j = 0; j < pm->width; j++) {
                cindex = (pm->bits)[k*pm->width+j];
                if (psdata->colorspace == PS_COLORSPACE_GRAYSCALE ||
                    psdata->level2 == FALSE) {
                    linelen += fprintf(prstream,"%02x",
                        (int) (255*get_colorintensity(canvas, cindex)));
                } else if (psdata->colorspace == PS_COLORSPACE_CMYK) {
                    CMYK cmyk;
                    get_cmyk(canvas, cindex, &cmyk);
                    linelen += fprintf(prstream, "%02x%02x%02x%02x",
                                      cmyk.cyan, cmyk.magenta,
                                      cmyk.yellow, cmyk.black);
                } else {
                    RGB rgb;
                    get_rgb(canvas, cindex, &rgb);
                    linelen += fprintf(prstream, "%02x%02x%02x",
                                       rgb.red, rgb.green, rgb.blue);
                }
                if (linelen >= MAX_PS_LINELEN) {
                    fprintf(prstream, "\n");
                    linelen = 0;
                }
            }
            fprintf(prstream, "\n");
        }
    } else { /* monocolor bitmap */
        paddedW = PADBITS(pm->width, pm->pad);
        if (pm->type == PIXMAP_OPAQUE) {
            cindex = getbgcolor(canvas);
            switch (psdata->colorspace) {
            case PS_COLORSPACE_GRAYSCALE:
                fprintf(prstream,"%.4f SGRY\n",
                    get_colorintensity(canvas, cindex));
                break;
            case PS_COLORSPACE_RGB:
                get_frgb(canvas, cindex, &frgb);
                fprintf(prstream,"%.4f %.4f %.4f SRGB\n",
                                  frgb.red, frgb.green, frgb.blue);
                break;
            case PS_COLORSPACE_CMYK:
                get_fcmyk(canvas, cindex, &fcmyk);
                fprintf(prstream, "%.4f %.4f %.4f %.4f SCMYK\n",
                                  fcmyk.cyan, fcmyk.magenta,
                                  fcmyk.yellow, fcmyk.black);
                break;
            }
            fprintf(prstream, "0 0 1 -1 rectfill\n");
        }
        cindex = getcolor(canvas);
        switch (psdata->colorspace) {
        case PS_COLORSPACE_GRAYSCALE:
            fprintf(prstream,"%.4f SGRY\n",
                get_colorintensity(canvas, cindex));
            break;
        case PS_COLORSPACE_RGB:
            get_frgb(canvas, cindex, &frgb);
            fprintf(prstream,"%.4f %.4f %.4f SRGB\n",
                              frgb.red, frgb.green, frgb.blue);
            break;
        case PS_COLORSPACE_CMYK:
            get_fcmyk(canvas, cindex, &fcmyk);
            fprintf(prstream, "%.4f %.4f %.4f %.4f SCMYK\n",
                              fcmyk.cyan, fcmyk.magenta,
                              fcmyk.yellow, fcmyk.black);
            break;
        }
        fprintf(prstream, "/picstr %d string def\n", paddedW/8);
        fprintf(prstream, "%d %d true\n", paddedW, pm->height);
        fprintf(prstream, "[%d 0 0 %d 0 0]\n", paddedW, -pm->height);
        fprintf(prstream, "{currentfile picstr readhexstring pop}\n");
        fprintf(prstream, "imagemask\n");
        for (k = 0; k < pm->height; k++) {
            linelen = 0;
            for (j = 0; j < paddedW/pm->pad; j++) {
                tmpbyte = reversebits((unsigned char) (pm->bits)[k*paddedW/pm->pad + j]);
                linelen += fprintf(prstream, "%02x", tmpbyte);
                if (linelen >= MAX_PS_LINELEN) {
                    fprintf(prstream, "\n");
                    linelen = 0;
                }
            }
            fprintf(prstream, "\n");
        }
    }
    fprintf(prstream, "GR\n");
}

static void ps_puttext(const Canvas *canvas, void *data,
    const VPoint *vp, const char *s, int len, int font, const TextMatrix *tm,
    int underline, int overline, int kerning)
{
    PS_data *psdata = (PS_data *) data;
    double *kvector;
    int i;
    Pen pen;
    int linelen;
    FILE *prstream = canvas_get_prstream(canvas);
    
    fprintf(prstream, "/Font%d FFSF\n", font);

    getpen(canvas, &pen);
    ps_setpen(canvas, &pen, psdata);
    
    fprintf(prstream, "%.4f %.4f m\n", vp->x, vp->y);
    fprintf(prstream, "GS\n");
    fprintf(prstream, "[%.4f %.4f %.4f %.4f 0 0] CC\n",
                        tm->cxx, tm->cyx, tm->cxy, tm->cyy);
    
    if (kerning) {
        kvector = get_kerning_vector(canvas, s, len, font);
    } else {
        kvector = NULL;
    }
    
    if (kvector) {
        linelen = 0;
        linelen += fprintf(prstream, "[");
        for (i = 0; i < len - 1; i++) {
            linelen += fprintf(prstream, "%.4f ", kvector[i]);
            if (linelen >= MAX_PS_LINELEN) {
                fprintf(prstream, "\n");
                linelen = 0;
            }
        }
        fprintf(prstream, "] KINIT\n");
        fprintf(prstream, "{KPROC}\n");
    }
    
    put_string(psdata, prstream, s, len);

    if (underline | overline) {
        double w, pos, kcomp;
        
        if (kvector) {
            kcomp = kvector[len - 1];
        } else {
            kcomp = 0.0;
        }
        w = get_textline_width(canvas, font);
        if (underline) {
            pos = get_underline_pos(canvas, font);
            fprintf(prstream, " %.4f %.4f %.4f TL", pos, w, kcomp);
        }
        if (overline) {
            pos = get_overline_pos(canvas, font);
            fprintf(prstream, " %.4f %.4f %.4f TL", pos, w, kcomp);
        }
    }
    
    if (kvector) {
        fprintf(prstream, " kshow\n");
        xfree(kvector);
    } else {
        fprintf(prstream, " show\n");
    }
    
    fprintf(prstream, "GR\n");
}


static void ps_leavegraphics(const Canvas *canvas, void *data,
    const CanvasStats *cstats)
{
    PS_data *psdata = (PS_data *) data;
    FILE *prstream = canvas_get_prstream(canvas);
    
    if (psdata->format == PS_FORMAT || psdata->printable) {
        fprintf(prstream, "showpage\n");
    }
    
    if (psdata->format == PS_FORMAT) {
        fprintf(prstream, "%%%%PageTrailer\n");
    }
    
    fprintf(prstream, "%%%%Trailer\n");
    fprintf(prstream, "%%%%EOF\n");
}

static int is7bit(unsigned char uc)
{
    if (uc >= 0x1b && uc <= 0x7e) {
        return TRUE;
    } else {
        return FALSE;
    }
}

static int is8bit(unsigned char uc)
{
    if (is7bit(uc) || uc >= 0x80) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/*
 * Put a NOT NULL-terminated string escaping parentheses and backslashes
 */
static void put_string(PS_data *psdata, FILE *fp, const char *s, int len)
{
    int i, linelen = 0;
    
    fputc('(', fp);
    linelen++;
    for (i = 0; i < len; i++) {
        char c = s[i];
        unsigned char uc = (unsigned char) c;
        if (c == '(' || c == ')' || c == '\\') {
            fputc('\\', fp);
            linelen++;
        }
        if ((psdata->docdata == PS_DOCDATA_7BIT && !is7bit(uc)) ||
            (psdata->docdata == PS_DOCDATA_8BIT && !is8bit(uc))) {
            linelen += fprintf(fp, "\\%03o", uc);
        } else {
            fputc(c, fp);
            linelen++;
        }
        if (linelen >= MAX_PS_LINELEN) {
            fprintf(fp, "\\\n");
            linelen = 0;
        }
    }
    fputc(')', fp);
}

static int ps_op_parser(const Canvas *canvas, void *data, const char *opstring)
{
    PS_data *psdata = (PS_data *) data;
    
    if (!strcmp(opstring, "level2")) {
        psdata->level2 = TRUE;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "level1")) {
        psdata->level2 = FALSE;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "colorspace:grayscale")) {
        psdata->colorspace = PS_COLORSPACE_GRAYSCALE;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "colorspace:rgb")) {
        psdata->colorspace = PS_COLORSPACE_RGB;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "colorspace:cmyk")) {
        psdata->colorspace = PS_COLORSPACE_CMYK;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "docdata:7bit")) {
        psdata->docdata = PS_DOCDATA_7BIT;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "docdata:8bit")) {
        psdata->docdata = PS_DOCDATA_8BIT;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "docdata:binary")) {
        psdata->docdata = PS_DOCDATA_BINARY;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "embedfonts:none")) {
        psdata->fonts = PS_FONT_EMBED_NONE;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "embedfonts:but13")) {
        psdata->fonts = PS_FONT_EMBED_BUT13;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "embedfonts:but35")) {
        psdata->fonts = PS_FONT_EMBED_BUT35;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "embedfonts:all")) {
        psdata->fonts = PS_FONT_EMBED_ALL;
        return RETURN_SUCCESS;
    } else if (psdata->format == PS_FORMAT) {
        if (!strncmp(opstring, "xoffset:", 8)) {
            psdata->offset_x = atoi(opstring + 8);
            return RETURN_SUCCESS;
        } else if (!strncmp(opstring, "yoffset:", 8)) {
            psdata->offset_y = atoi(opstring + 8);
            return RETURN_SUCCESS;
        } else if (!strcmp(opstring, "hwresolution:on")) {
            psdata->hwres = TRUE;
            return RETURN_SUCCESS;
        } else if (!strcmp(opstring, "hwresolution:off")) {
            psdata->hwres = FALSE;
            return RETURN_SUCCESS;
        } else if (!strcmp(opstring, "mediafeed:auto")) {
            psdata->feed = PS_MEDIA_FEED_AUTO;
            return RETURN_SUCCESS;
        } else if (!strcmp(opstring, "mediafeed:match")) {
            psdata->feed = PS_MEDIA_FEED_MATCH;
            return RETURN_SUCCESS;
        } else if (!strcmp(opstring, "mediafeed:manual")) {
            psdata->feed = PS_MEDIA_FEED_MANUAL;
            return RETURN_SUCCESS;
        } else {
            return RETURN_FAILURE;
        }
    } else if (psdata->format == EPS_FORMAT) {
        if (!strcmp(opstring, "printable:on")) {
            psdata->printable = TRUE;
            return RETURN_SUCCESS;
        } else if (!strcmp(opstring, "printable:off")) {
            psdata->printable = FALSE;
            return RETURN_SUCCESS;
        } else {
            return RETURN_FAILURE;
        }
    } else {
        return RETURN_FAILURE;
    }
}

int register_ps_drv(Canvas *canvas)
{
    Device_entry *d;
    PS_data *data;
    
    data = init_ps_data(PS_FORMAT);
    if (!data) {
        return -1;
    }

    d = device_new("PostScript", DEVICE_PRINT, TRUE, data, xfree);
    if (!d) {
        xfree(data);
        return -1;
    }
    
    device_set_fext(d, "ps");
    
    device_set_dpi(d, 300.0);
    
    device_set_procs(d,
        ps_initgraphics,
        ps_leavegraphics,
        ps_op_parser,
        NULL,
        ps_drawpixel,
        ps_drawpolyline,
        ps_fillpolygon,
        ps_drawarc,
        ps_fillarc,
        ps_putpixmap,
        ps_puttext);
    
    return register_device(canvas, d);
    
}

int register_eps_drv(Canvas *canvas)
{
    Device_entry *d;
    PS_data *data;
    
    data = init_ps_data(EPS_FORMAT);
    if (!data) {
        return -1;
    }

    d = device_new("EPS", DEVICE_FILE, TRUE, (void *) data, xfree);
    if (!d) {
        xfree(data);
        return -1;
    }
    
    device_set_fext(d, "eps");
    
    device_set_dpi(d, 300.0);
    
    device_set_autocrop(d, TRUE);
    
    device_set_procs(d,
        ps_initgraphics,
        ps_leavegraphics,
        ps_op_parser,
        NULL,
        ps_drawpixel,
        ps_drawpolyline,
        ps_fillpolygon,
        ps_drawarc,
        ps_fillarc,
        ps_putpixmap,
        ps_puttext);
    
    return register_device(canvas, d);
}
