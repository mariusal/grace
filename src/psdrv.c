/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1996-2002 Grace Development Team
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

/*
 *  Grace PostScript driver
 */

#include <config.h>
#include <cmath.h>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "defines.h"
#include "utils.h"
#include "draw.h"
#include "devlist.h"
#include "psdrv.h"
#include "protos.h"

#ifndef NONE_GUI
#  include "motifinc.h"
#endif

typedef struct {
    int curformat;

    unsigned long page_scale;
    double pixel_size;
    float page_scalef;
    int page_orientation;

    int color;
    int pattern;
    double linew;
    int lines;
    int linecap;
    int linejoin;

    int level2;
    PSColorSpace colorspace;
    int docdata;
    int fonts;

    int offset_x;
    int offset_y;
    int feed;
    int hwres;

#ifndef NONE_GUI
    Widget frame;
    Widget level2_item;
    OptionStructure *docdata_item;
    OptionStructure *fonts_item;
    OptionStructure *colorspace_item;
    SpinStructure *offset_x_item;
    SpinStructure *offset_y_item;
    OptionStructure *feed_item;
    Widget hwres_item;
#endif
} PS_data;

static void put_string(PS_data *psdata, FILE *fp, const char *s, int len);


static Device_entry dev_ps = {
    DEVICE_PRINT,
    "PostScript",
    "ps",
    TRUE,
    FALSE,
    {3300, 2550, 300.0},

    TRUE,
    FALSE,

    ps_initgraphics,
    ps_op_parser,
    ps_gui_setup,
    NULL,
    ps_leavegraphics,
    ps_drawpixel,
    ps_drawpolyline,
    ps_fillpolygon,
    ps_drawarc,
    ps_fillarc,
    ps_putpixmap,
    ps_puttext,

    NULL
};

static Device_entry dev_eps = {
    DEVICE_FILE,
    "EPS",
    "eps",
    TRUE,
    FALSE,
    {2500, 2500, 300.0},

    TRUE,
    TRUE,

    ps_initgraphics,
    ps_op_parser,
    ps_gui_setup,
    NULL,
    ps_leavegraphics,
    ps_drawpixel,
    ps_drawpolyline,
    ps_fillpolygon,
    ps_drawarc,
    ps_fillarc,
    ps_putpixmap,
    ps_puttext,

    NULL
};

static PS_data *init_ps_data(const Canvas *canvas, int format)
{
    PS_data *data;

    /* we need to perform the allocations */
    data = (PS_data *) xmalloc(sizeof(PS_data));
    if (data == NULL) {
        return NULL;
    }

    memset(data, 0, sizeof(PS_data));

    data->curformat  = format;

    data->level2     = TRUE;
    data->colorspace = DEFAULT_COLORSPACE;
    data->docdata    = DOCDATA_8BIT;
    data->fonts      = FONT_EMBED_BUT35;

    data->offset_x   = 0;
    data->offset_y   = 0;
    data->feed       = MEDIA_FEED_AUTO;
    data->hwres      = FALSE;

    return data;
}

int register_ps_drv(Canvas *canvas)
{
    PS_data *data;
    
    data = init_ps_data(canvas, PS_FORMAT);
    if (!data) {
        return RETURN_FAILURE;
    }

    dev_ps.data = data;

    return register_device(canvas, &dev_ps);
}

int register_eps_drv(Canvas *canvas)
{
    PS_data *data;
    
    data = init_ps_data(canvas, EPS_FORMAT);
    if (!data) {
        return RETURN_FAILURE;
    }

    dev_eps.data = data;

    return register_device(canvas, &dev_eps);
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
    case FONT_EMBED_NONE:
        return FALSE;
        break;
    case FONT_EMBED_BUT13:
        for (i = 0; i < number_of_ps_standard_fonts13; i++) {
            if (strcmp(ps_standard_fonts13[i], fname) == 0) {
                return FALSE;
            }
        }
        return TRUE;
        break;
    case FONT_EMBED_BUT35:
        for (i = 0; i < number_of_ps_standard_fonts35; i++) {
            if (strcmp(ps_standard_fonts35[i], fname) == 0) {
                return FALSE;
            }
        }
        return TRUE;
        break;
    case FONT_EMBED_ALL:
    default:
        return TRUE;
        break;
    }
}

int ps_initgraphics(const Canvas *canvas, void *data,
    const CanvasStats *cstats)
{
    PS_data *psdata = (PS_data *) data;
    int i, j, first;
    Page_geometry *pg;
    int width_pp, height_pp;
    PSFont *psfonts;
    int font_needed_any = FALSE, font_embed_any = FALSE;
    char **enc;
    view v;
    double llx, lly, urx, ury;
    
    time_t time_value;
    
    pg = get_page_geometry(canvas);
    
    psdata->page_scale = MIN2(pg->height, pg->width);
    psdata->pixel_size = 1.0/psdata->page_scale;
    psdata->page_scalef = (float) psdata->page_scale*72.0/pg->dpi;

    if (psdata->curformat == PS_FORMAT && pg->height < pg->width) {
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
    if (psdata->level2 == FALSE && psdata->colorspace == COLORSPACE_CMYK) {
        psdata->colorspace = COLORSPACE_RGB;
    }

    if (psdata->curformat == EPS_FORMAT) {
        fprintf(canvas->prstream, "%%!PS-Adobe-3.0 EPSF-3.0\n");
    } else {
        fprintf(canvas->prstream, "%%!PS-Adobe-3.0\n");
    }

    if (psdata->page_orientation == PAGE_ORIENT_LANDSCAPE) {
        width_pp  = (int) rint(72.0*pg->height/pg->dpi);
        height_pp = (int) rint(72.0*pg->width/pg->dpi);
    } else {
        width_pp  = (int) rint(72.0*pg->width/pg->dpi);
        height_pp = (int) rint(72.0*pg->height/pg->dpi);
    }

    v = cstats->bbox;
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
    fprintf(canvas->prstream, "%%%%BoundingBox: %d %d %d %d\n",
        (int) floor(llx), (int) floor(lly), (int) ceil(urx), (int) ceil(ury));
    fprintf(canvas->prstream, "%%%%HiResBoundingBox: %.2f %.2f %.2f %.2f\n",
        llx, lly, urx, ury);

    if (psdata->level2 == TRUE) {
        fprintf(canvas->prstream, "%%%%LanguageLevel: 2\n");
    } else {
        fprintf(canvas->prstream, "%%%%LanguageLevel: 1\n");
    }
    
    fprintf(canvas->prstream, "%%%%Creator: %s\n", bi_version_string());

    time(&time_value);
    fprintf(canvas->prstream, "%%%%CreationDate: %s", ctime(&time_value));
    switch (psdata->docdata) {
    case DOCDATA_7BIT:
        fprintf(canvas->prstream, "%%%%DocumentData: Clean7Bit\n");
        break;
    case DOCDATA_8BIT:
        fprintf(canvas->prstream, "%%%%DocumentData: Clean8Bit\n");
        break;
    default:
        fprintf(canvas->prstream, "%%%%DocumentData: Binary\n");
        break;
    }
    if (psdata->page_orientation == PAGE_ORIENT_LANDSCAPE) {
        fprintf(canvas->prstream, "%%%%Orientation: Landscape\n");
    } else {
        fprintf(canvas->prstream, "%%%%Orientation: Portrait\n");
    }
    
    if (psdata->curformat == PS_FORMAT) {
        fprintf(canvas->prstream, "%%%%Pages: 1\n");
        fprintf(canvas->prstream, "%%%%PageOrder: Ascend\n");
    }
    fprintf(canvas->prstream, "%%%%Title: %s\n", canvas_get_docname(canvas));
    fprintf(canvas->prstream, "%%%%For: %s\n", canvas_get_username(canvas));
    
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
                    fprintf(canvas->prstream, "%%%%DocumentNeededResources:");
                    first = FALSE;
                } else {
                    fprintf(canvas->prstream, "%%%%+");
                }
                fprintf(canvas->prstream, " font %s\n", psfonts[i].name);
            }
        }
    }

    if (font_embed_any) {
        first = TRUE;
        for (i = 0; i < cstats->nfonts; i++) {
            if (psfonts[i].embed) {
                if (first) {
                    fprintf(canvas->prstream, "%%%%DocumentSuppliedResources:");
                    first = FALSE;
                } else {
                    fprintf(canvas->prstream, "%%%%+");
                }
                fprintf(canvas->prstream, " font %s\n", psfonts[i].name);
            }
        }
    }

    fprintf(canvas->prstream, "%%%%EndComments\n");

    /* Definitions */
    fprintf(canvas->prstream, "%%%%BeginProlog\n");
    if (psdata->curformat == PS_FORMAT) {
        fprintf(canvas->prstream, "/PAGE_OFFSET_X %d def\n", psdata->offset_x);
        fprintf(canvas->prstream, "/PAGE_OFFSET_Y %d def\n", psdata->offset_y);
    }
    fprintf(canvas->prstream, "/m {moveto} def\n");
    fprintf(canvas->prstream, "/l {lineto} def\n");
    fprintf(canvas->prstream, "/s {stroke} def\n");
    fprintf(canvas->prstream, "/n {newpath} def\n");
    fprintf(canvas->prstream, "/c {closepath} def\n");
    fprintf(canvas->prstream, "/RL {rlineto} def\n");
    fprintf(canvas->prstream, "/SLW {setlinewidth} def\n");
    fprintf(canvas->prstream, "/GS {gsave} def\n");
    fprintf(canvas->prstream, "/GR {grestore} def\n");
    fprintf(canvas->prstream, "/SC {setcolor} def\n");
    fprintf(canvas->prstream, "/SGRY {setgray} def\n");
    fprintf(canvas->prstream, "/SRGB {setrgbcolor} def\n");
    if (psdata->colorspace == COLORSPACE_CMYK) {
        fprintf(canvas->prstream, "/SCMYK {setcmykcolor} def\n");
    }
    fprintf(canvas->prstream, "/SD {setdash} def\n");
    fprintf(canvas->prstream, "/SLC {setlinecap} def\n");
    fprintf(canvas->prstream, "/SLJ {setlinejoin} def\n");
    fprintf(canvas->prstream, "/SCS {setcolorspace} def\n");
    fprintf(canvas->prstream, "/FFSF {findfont setfont} def\n");
    fprintf(canvas->prstream, "/CC {concat} def\n");
    fprintf(canvas->prstream, "/PXL {n m 0 0 RL s} def\n");
    
    for (i = 0; i < cstats->ncolors; i++) {
        int cindex = cstats->colors[i];
        fprintf(canvas->prstream,"/Color%d {", cindex);
        switch (psdata->colorspace) {
        case COLORSPACE_GRAYSCALE:
            fprintf(canvas->prstream,"%.4f", get_colorintensity(canvas, cindex));
            break;
        case COLORSPACE_RGB:
            {
                fRGB frgb;
                if (get_frgb(canvas, cindex, &frgb) == RETURN_SUCCESS) {
                    fprintf(canvas->prstream, "%.4f %.4f %.4f",
                                      frgb.red, frgb.green, frgb.blue);
                }
            }
            break;
        case COLORSPACE_CMYK:
            {
                fCMYK fcmyk;
                if (get_fcmyk(canvas, cindex, &fcmyk) == RETURN_SUCCESS) {
                    fprintf(canvas->prstream, "%.4f %.4f %.4f %.4f",
                                      fcmyk.cyan, fcmyk.magenta,
                                      fcmyk.yellow, fcmyk.black);
                }
            }
            break;
        }
        fprintf(canvas->prstream,"} def\n");
    }

    for (i = 0; i < cstats->nfonts; i++) {
        if (psfonts[i].embed) {
            int font = cstats->fonts[i].font;
            char *fontdata;
            unsigned long datalen;
            
            fontdata = font_subset(canvas,
                font, cstats->fonts[i].chars_used, &datalen);
            if (fontdata) {
                fprintf(canvas->prstream, "%%%%BeginResource: font %s\n",
                    psfonts[i].name);
                fwrite(fontdata, 1, datalen, canvas->prstream);
                fprintf(canvas->prstream, "%%%%EndResource\n");
                free(fontdata);
            } else {
                errmsg("Font subsetting failed");
            }
        }
    }

    if (cstats->nfonts > 0) {
        /* Default encoding */
        enc = get_default_encoding(canvas);
        fprintf(canvas->prstream, "/DefEncoding [\n");
        for (i = 0; i < 256; i++) {
            fprintf(canvas->prstream, " /%s\n", enc[i]);
        }
        fprintf(canvas->prstream, "] def\n");
    }

    for (i = 0; i < cstats->nfonts; i++) {
        int font = cstats->fonts[i].font;
        char *encscheme = get_encodingscheme(canvas, font);

        fprintf(canvas->prstream, "/%s findfont\n", psfonts[i].name);
        if (strcmp(encscheme, "FontSpecific") != 0) {
            fprintf(canvas->prstream, "dup length dict begin\n");
            fprintf(canvas->prstream, " {1 index /FID ne {def} {pop pop} ifelse} forall\n");
            fprintf(canvas->prstream, " /Encoding DefEncoding def\n");
            fprintf(canvas->prstream, " currentdict\n");
            fprintf(canvas->prstream, "end\n");
        }
        fprintf(canvas->prstream, "/Font%d exch definefont pop\n", font);
    }
       
    if (psdata->level2 == TRUE && cstats->npatterns) {
        fprintf(canvas->prstream, "/PTRN {\n");
        fprintf(canvas->prstream, " /pat_bits exch def \n");
        fprintf(canvas->prstream, " /height exch def \n");
        fprintf(canvas->prstream, " /width exch def \n");
        fprintf(canvas->prstream, " <<\n");
        fprintf(canvas->prstream, "  /PaintType 2\n");
        fprintf(canvas->prstream, "  /PatternType 1 /TilingType 1\n");
        fprintf(canvas->prstream, "  /BBox[0 0 width height]\n");
        fprintf(canvas->prstream, "  /XStep width /YStep height\n");
        fprintf(canvas->prstream, "  /PaintProc {\n");
        fprintf(canvas->prstream, "   pop\n");
        fprintf(canvas->prstream, "   width height true [-1 0 0 -1 width height] pat_bits imagemask\n");
        fprintf(canvas->prstream, "  }\n");
        fprintf(canvas->prstream, " >>\n");
        fprintf(canvas->prstream, " [%.4f 0 0 %.4f 0 0]\n", 1.0/psdata->page_scalef, 1.0/psdata->page_scalef);
        fprintf(canvas->prstream, " makepattern\n");
        fprintf(canvas->prstream, "} def\n");
        for (i = 0; i < cstats->npatterns; i++) {
            int patno = cstats->patterns[i];
            Pattern *pat = canvas_get_pattern(canvas, patno);
            fprintf(canvas->prstream, "/Pattern%d {%d %d <",
                patno, pat->width, pat->height);
            for (j = 0; j < pat->width*pat->height/8; j++) {
                fprintf(canvas->prstream, "%02x", pat->bits[j]);
            }
            fprintf(canvas->prstream, "> PTRN} bind def\n");
        }
    }
    
    /* Elliptic arc */
    fprintf(canvas->prstream, "/ellipsedict 9 dict def\n");
    fprintf(canvas->prstream, "ellipsedict /mtrx matrix put\n");
    fprintf(canvas->prstream, "/EARC {\n");
    fprintf(canvas->prstream, " ellipsedict begin\n");
    fprintf(canvas->prstream, "  /pathangle exch def\n");
    fprintf(canvas->prstream, "  /startangle exch def\n");
    fprintf(canvas->prstream, "  /yrad exch def\n");
    fprintf(canvas->prstream, "  /xrad exch def\n");
    fprintf(canvas->prstream, "  /y exch def\n");
    fprintf(canvas->prstream, "  /x exch def\n");
    fprintf(canvas->prstream, "  /savematrix mtrx currentmatrix def\n");
    fprintf(canvas->prstream, "  /endangle startangle pathangle add def\n");
    fprintf(canvas->prstream, "  x y translate\n");
    fprintf(canvas->prstream, "  xrad yrad scale\n");
    fprintf(canvas->prstream, "  0 0 1 startangle endangle\n");
    fprintf(canvas->prstream, "  pathangle 0 lt {arcn} {arc} ifelse\n");
    fprintf(canvas->prstream, "  savematrix setmatrix\n");
    fprintf(canvas->prstream, " end\n");
    fprintf(canvas->prstream, "} def\n");

    /* Text under/overlining etc */
    fprintf(canvas->prstream, "/TL {\n");
    fprintf(canvas->prstream, "  /kcomp exch def\n");
    fprintf(canvas->prstream, "  /linewidth exch def\n");
    fprintf(canvas->prstream, "  /offset exch def\n");
    fprintf(canvas->prstream, "  GS\n");
    fprintf(canvas->prstream, "  0 offset rmoveto\n");
    fprintf(canvas->prstream, "  linewidth SLW\n");
    fprintf(canvas->prstream, "  dup stringwidth exch kcomp add exch RL s\n");
    fprintf(canvas->prstream, "  GR\n");
    fprintf(canvas->prstream, "} def\n");

    /* Kerning stuff */
    fprintf(canvas->prstream, "/KINIT\n");
    fprintf(canvas->prstream, "{\n");
    fprintf(canvas->prstream, " /kvector exch def\n");
    fprintf(canvas->prstream, " /kid 0 def\n");
    fprintf(canvas->prstream, "} def\n");
    fprintf(canvas->prstream, "/KPROC\n");
    fprintf(canvas->prstream, "{\n");
    fprintf(canvas->prstream, " pop pop\n");
    fprintf(canvas->prstream, " kvector kid get\n");
    fprintf(canvas->prstream, " 0 rmoveto\n");
    fprintf(canvas->prstream, " /kid 1 kid add def\n");
    fprintf(canvas->prstream, "} def\n");

    fprintf(canvas->prstream, "%%%%EndProlog\n");

    fprintf(canvas->prstream, "%%%%BeginSetup\n");
    if (psdata->level2 == TRUE && psdata->curformat == PS_FORMAT) {
        /* page size feed */
        switch (psdata->feed) {
        case MEDIA_FEED_AUTO:
            break;
        case MEDIA_FEED_MATCH:
            fprintf(canvas->prstream, "%%%%BeginFeature: *PageSize\n");
            fprintf(canvas->prstream,
                "<</PageSize [%d %d] /ImagingBBox null>> setpagedevice\n",
                width_pp, height_pp);
            fprintf(canvas->prstream, "%%%%EndFeature\n");
            break;
        case MEDIA_FEED_MANUAL:
            fprintf(canvas->prstream, "%%%%BeginFeature: *ManualFeed\n");
            fprintf(canvas->prstream, "<</ManualFeed true>> setpagedevice\n");
            fprintf(canvas->prstream, "%%%%EndFeature\n");
            break;
        }
        
        /* force HW resolution */
        if (psdata->hwres == TRUE) {
            fprintf(canvas->prstream, "%%%%BeginFeature: *HWResolution\n");
            fprintf(canvas->prstream, "<</HWResolution [%d %d]>> setpagedevice\n",
                (int) pg->dpi, (int) pg->dpi);
            fprintf(canvas->prstream, "%%%%EndFeature\n");
        }
    }
    
    /* compensate for printer page offsets */
    if (psdata->curformat == PS_FORMAT) {
        fprintf(canvas->prstream, "PAGE_OFFSET_X PAGE_OFFSET_Y translate\n");
    }
    fprintf(canvas->prstream, "%.2f %.2f scale\n", psdata->page_scalef, psdata->page_scalef);
    /* rotate to get landscape on hardcopy */
    if (psdata->page_orientation == PAGE_ORIENT_LANDSCAPE) {
        fprintf(canvas->prstream, "90 rotate\n");
        fprintf(canvas->prstream, "0.0 -1.0 translate\n");
    }
    fprintf(canvas->prstream, "%%%%EndSetup\n");

    if (psdata->curformat == PS_FORMAT) {
        fprintf(canvas->prstream, "%%%%Page: 1 1\n");
    }

    /* free the psfonts array */
    for (i = 0; i < cstats->nfonts; i++) {
        xfree(psfonts[i].name);
    }
    xfree(psfonts);

    return RETURN_SUCCESS;
}

void ps_setpen(const Canvas *canvas, const Pen *pen, PS_data *psdata)
{
    if (pen->color != psdata->color || pen->pattern != psdata->pattern) {
        if (psdata->level2 == TRUE) {
            if (pen->pattern == 1) {
                switch (psdata->colorspace) {
                case COLORSPACE_GRAYSCALE:
                    fprintf(canvas->prstream, "[/DeviceGray] SCS\n");
                    break;
                case COLORSPACE_RGB:
                    fprintf(canvas->prstream, "[/DeviceRGB] SCS\n");
                    break;
                case COLORSPACE_CMYK:
                    fprintf(canvas->prstream, "[/DeviceCMYK] SCS\n");
                    break;
                }
                fprintf(canvas->prstream, "Color%d SC\n", pen->color);
            } else {
                switch (psdata->colorspace) {
                case COLORSPACE_GRAYSCALE:
                    fprintf(canvas->prstream, "[/Pattern /DeviceGray] SCS\n");
                    break;
                case COLORSPACE_RGB:
                    fprintf(canvas->prstream, "[/Pattern /DeviceRGB] SCS\n");
                    break;
                case COLORSPACE_CMYK:
                    fprintf(canvas->prstream, "[/Pattern /DeviceCMYK] SCS\n");
                    break;
                }
                fprintf(canvas->prstream,
                    "Color%d Pattern%d SC\n", pen->color, pen->pattern);
            }
        } else {
            if (psdata->colorspace == COLORSPACE_GRAYSCALE) {
                fprintf(canvas->prstream, "Color%d SGRY\n", pen->color);
            } else {
                fprintf(canvas->prstream, "Color%d SRGB\n", pen->color);
            }
        }
        psdata->color = pen->color;
        psdata->pattern = pen->pattern;
    }
}

void ps_setdrawbrush(const Canvas *canvas, PS_data *psdata)
{
    int i;
    int ls;
    double lw;
    Pen pen;
    
    getpen(canvas, &pen);
    ps_setpen(canvas, &pen, psdata);

    ls = getlinestyle(canvas);
    lw = MAX2(getlinewidth(canvas), psdata->pixel_size);
    
    if (ls != psdata->lines || lw != psdata->linew) {    
        fprintf(canvas->prstream, "[");
        if (ls > 1) {
            LineStyle *linestyle = canvas_get_linestyle(canvas, ls);
            for (i = 0; i < linestyle->length; i++) {
                fprintf(canvas->prstream, "%.4f ", lw*linestyle->array[i]);
            }
        }
        fprintf(canvas->prstream, "] 0 SD\n");
        fprintf(canvas->prstream, "%.4f SLW\n", lw);
        psdata->linew = lw;
        psdata->lines = ls;
    }
}

void ps_setlineprops(const Canvas *canvas, PS_data *psdata)
{
    int lc, lj;
    
    lc = getlinecap(canvas);
    lj = getlinejoin(canvas);
    
    if (lc != psdata->linecap) {
        switch (lc) {
        case LINECAP_BUTT:
            fprintf(canvas->prstream, "0 SLC\n");
            break;
        case LINECAP_ROUND:
            fprintf(canvas->prstream, "1 SLC\n");
            break;
        case LINECAP_PROJ:
            fprintf(canvas->prstream, "2 SLC\n");
            break;
        }
        psdata->linecap = lc;
    }

    if (lj != psdata->linejoin) {
        switch (lj) {
        case LINEJOIN_MITER:
            fprintf(canvas->prstream, "0 SLJ\n");
            break;
        case LINEJOIN_ROUND:
            fprintf(canvas->prstream, "1 SLJ\n");
            break;
        case LINEJOIN_BEVEL:
            fprintf(canvas->prstream, "2 SLJ\n");
            break;
        }
        psdata->linejoin = lj;
    }
}

void ps_drawpixel(const Canvas *canvas, void *data, const VPoint *vp)
{
    PS_data *psdata = (PS_data *) data;
    Pen pen;
    getpen(canvas, &pen);
    ps_setpen(canvas, &pen, psdata);
    
    if (psdata->linew != psdata->pixel_size) {
        fprintf(canvas->prstream, "%.4f SLW\n", psdata->pixel_size);
        psdata->linew = psdata->pixel_size;
    }
    if (psdata->linecap != LINECAP_ROUND) {
        fprintf(canvas->prstream, "1 SLC\n");
        psdata->linecap = LINECAP_ROUND;
    }
    if (psdata->lines != 1) {
        fprintf(canvas->prstream, "[] 0 SD\n");
        psdata->lines = 1;
    }
    
    fprintf(canvas->prstream, "%.4f %.4f PXL\n", vp->x, vp->y);
}

void ps_drawpolyline(const Canvas *canvas, void *data,
    const VPoint *vps, int n, int mode)
{
    PS_data *psdata = (PS_data *) data;
    int i;
    
    ps_setdrawbrush(canvas, psdata);
    
    ps_setlineprops(canvas, psdata);
    
    fprintf(canvas->prstream, "n\n");
    fprintf(canvas->prstream, "%.4f %.4f m\n", vps[0].x, vps[0].y);
    for (i = 1; i < n; i++) {
        fprintf(canvas->prstream, "%.4f %.4f l\n", vps[i].x, vps[i].y);
    }
    if (mode == POLYLINE_CLOSED) {
        fprintf(canvas->prstream, "%.4f %.4f l\n", vps[0].x, vps[0].y);
        fprintf(canvas->prstream, "c\n");
    }
    fprintf(canvas->prstream, "s\n");
}

void ps_fillpolygon(const Canvas *canvas, void *data,
    const VPoint *vps, int nc)
{
    PS_data *psdata = (PS_data *) data;
    int i;
    Pen pen;
    getpen(canvas, &pen);
    
    if (pen.pattern == 0 || nc < 3) {
        return;
    }
    
    fprintf(canvas->prstream, "n\n");
    fprintf(canvas->prstream, "%.4f %.4f m\n", vps[0].x, vps[0].y);
    for (i = 1; i < nc; i++) {
        fprintf(canvas->prstream, "%.4f %.4f l\n", vps[i].x, vps[i].y);
    }
    fprintf(canvas->prstream, "c\n");

    /* fill bg first if the pattern != solid */
    if (pen.pattern != 1 && psdata->level2 == TRUE) {
        Pen bgpen;
        bgpen.color   = getbgcolor(canvas);
        bgpen.pattern = 1;
        fprintf(canvas->prstream, "GS\n");
        ps_setpen(canvas, &bgpen, psdata);
        fprintf(canvas->prstream, "fill\n");
        fprintf(canvas->prstream, "GR\n");
    }
    
    getpen(canvas, &pen);
    ps_setpen(canvas, &pen, psdata);
    if (getfillrule(canvas) == FILLRULE_WINDING) {
        fprintf(canvas->prstream, "fill\n");
    } else {
        fprintf(canvas->prstream, "eofill\n");
    }
}

void ps_drawarc(const Canvas *canvas, void *data,
    const VPoint *vp1, const VPoint *vp2, double a1, double a2)
{
    PS_data *psdata = (PS_data *) data;
    VPoint vpc;
    double rx, ry;
    
    ps_setdrawbrush(canvas, psdata);

    vpc.x = (vp1->x + vp2->x)/2;
    vpc.y = (vp1->y + vp2->y)/2;
    rx = fabs(vp2->x - vp1->x)/2;
    ry = fabs(vp2->y - vp1->y)/2;
    
    fprintf(canvas->prstream, "n %.4f %.4f %.4f %.4f %.4f %.4f EARC s\n",
                       vpc.x, vpc.y, rx, ry, a1, a2);
}

void ps_fillarc(const Canvas *canvas, void *data,
    const VPoint *vp1, const VPoint *vp2, double a1, double a2, int mode)
{
    PS_data *psdata = (PS_data *) data;
    VPoint vpc;
    double rx, ry;
    Pen pen;
    getpen(canvas, &pen);
    
    if (pen.pattern == 0) {
        return;
    }

    vpc.x = (vp1->x + vp2->x)/2;
    vpc.y = (vp1->y + vp2->y)/2;
    rx = fabs(vp2->x - vp1->x)/2;
    ry = fabs(vp2->y - vp1->y)/2;
    
    fprintf(canvas->prstream, "n\n");
    
    if (mode == ARCFILL_PIESLICE) {
        fprintf(canvas->prstream, "%.4f %.4f m\n", vpc.x, vpc.y);
    }
    fprintf(canvas->prstream, "%.4f %.4f %.4f %.4f %.4f %.4f EARC c\n",
                       vpc.x, vpc.y, rx, ry, a1, a2);

    /* fill bg first if the pattern != solid */
    if (pen.pattern != 1 && psdata->level2 == TRUE) {
        Pen bgpen;
        bgpen.color   = getbgcolor(canvas);
        bgpen.pattern = 1;
        fprintf(canvas->prstream, "GS\n");
        ps_setpen(canvas, &bgpen, psdata);
        fprintf(canvas->prstream, "fill\n");
        fprintf(canvas->prstream, "GR\n");
    }

    getpen(canvas, &pen);
    ps_setpen(canvas, &pen, psdata);
    fprintf(canvas->prstream, "fill\n");
}

void ps_putpixmap(const Canvas *canvas, void *data,
    const VPoint *vp, int width, int height, char *databits,
    int pixmap_bpp, int bitmap_pad, int pixmap_type)
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

    getpen(canvas, &pen);
    ps_setpen(canvas, &pen, psdata);
    
    fprintf(canvas->prstream, "GS\n");
    fprintf(canvas->prstream, "%.4f %.4f translate\n", vp->x, vp->y);
    fprintf(canvas->prstream, "%.4f %.4f scale\n",
        (float) width/psdata->page_scale, (float) height/psdata->page_scale);    
    if (pixmap_bpp != 1) {
        int layers = 1, bpp = 8;
        if (pixmap_type == PIXMAP_TRANSPARENT) {
            /* TODO: mask */
        }
        switch (psdata->colorspace) {
        case COLORSPACE_GRAYSCALE:
            layers = 1;
            bpp = 8;
            break;
        case COLORSPACE_RGB:
            layers = 3;
            bpp = GRACE_BPP;
            break;
        case COLORSPACE_CMYK:
            layers = 4;
            bpp = GRACE_BPP;
            break;
        }
        fprintf(canvas->prstream, "/picstr %d string def\n", width*layers);
        fprintf(canvas->prstream, "%d %d %d\n", width, height, bpp);
        fprintf(canvas->prstream, "[%d 0 0 %d 0 0]\n", width, -height);
        fprintf(canvas->prstream, "{currentfile picstr readhexstring pop}\n");
        if (psdata->colorspace == COLORSPACE_GRAYSCALE || psdata->level2 == FALSE) {
            /* No color images in Level1 */
            fprintf(canvas->prstream, "image\n");
        } else {
            fprintf(canvas->prstream, "false %d\n", layers);
            fprintf(canvas->prstream, "colorimage\n");
        }
        for (k = 0; k < height; k++) {
            linelen = 0;
            for (j = 0; j < width; j++) {
                cindex = (databits)[k*width+j];
                if (psdata->colorspace == COLORSPACE_GRAYSCALE ||
                    psdata->level2 == FALSE) {
                    linelen += fprintf(canvas->prstream,"%02x",
                        (int) (255*get_colorintensity(canvas, cindex)));
                } else if (psdata->colorspace == COLORSPACE_CMYK) {
                    CMYK cmyk;
                    get_cmyk(canvas, cindex, &cmyk);
                    linelen += fprintf(canvas->prstream, "%02x%02x%02x%02x",
                                      cmyk.cyan, cmyk.magenta,
                                      cmyk.yellow, cmyk.black);
                } else {
                    RGB rgb;
                    get_rgb(canvas, cindex, &rgb);
                    linelen += fprintf(canvas->prstream, "%02x%02x%02x",
                                       rgb.red, rgb.green, rgb.blue);
                }
                if (linelen >= MAX_PS_LINELEN) {
                    fprintf(canvas->prstream, "\n");
                    linelen = 0;
                }
            }
            fprintf(canvas->prstream, "\n");
        }
    } else { /* monocolor bitmap */
        paddedW = PAD(width, bitmap_pad);
        if (pixmap_type == PIXMAP_OPAQUE) {
            cindex = getbgcolor(canvas);
            switch (psdata->colorspace) {
            case COLORSPACE_GRAYSCALE:
                fprintf(canvas->prstream,"%.4f SGRY\n",
                    get_colorintensity(canvas, cindex));
                break;
            case COLORSPACE_RGB:
                get_frgb(canvas, cindex, &frgb);
                fprintf(canvas->prstream,"%.4f %.4f %.4f SRGB\n",
                                  frgb.red, frgb.green, frgb.blue);
                break;
            case COLORSPACE_CMYK:
                get_fcmyk(canvas, cindex, &fcmyk);
                fprintf(canvas->prstream, "%.4f %.4f %.4f %.4f SCMYK\n",
                                  fcmyk.cyan, fcmyk.magenta,
                                  fcmyk.yellow, fcmyk.black);
                break;
            }
            fprintf(canvas->prstream, "0 0 1 -1 rectfill\n");
        }
        cindex = getcolor(canvas);
        switch (psdata->colorspace) {
        case COLORSPACE_GRAYSCALE:
            fprintf(canvas->prstream,"%.4f SGRY\n",
                get_colorintensity(canvas, cindex));
            break;
        case COLORSPACE_RGB:
            get_frgb(canvas, cindex, &frgb);
            fprintf(canvas->prstream,"%.4f %.4f %.4f SRGB\n",
                              frgb.red, frgb.green, frgb.blue);
            break;
        case COLORSPACE_CMYK:
            get_fcmyk(canvas, cindex, &fcmyk);
            fprintf(canvas->prstream, "%.4f %.4f %.4f %.4f SCMYK\n",
                              fcmyk.cyan, fcmyk.magenta,
                              fcmyk.yellow, fcmyk.black);
            break;
        }
        fprintf(canvas->prstream, "/picstr %d string def\n", paddedW/8);
        fprintf(canvas->prstream, "%d %d true\n", paddedW, height);
        fprintf(canvas->prstream, "[%d 0 0 %d 0 0]\n", paddedW, -height);
        fprintf(canvas->prstream, "{currentfile picstr readhexstring pop}\n");
        fprintf(canvas->prstream, "imagemask\n");
        for (k = 0; k < height; k++) {
            linelen = 0;
            for (j = 0; j < paddedW/bitmap_pad; j++) {
                tmpbyte = reversebits((unsigned char) (databits)[k*paddedW/bitmap_pad + j]);
                linelen += fprintf(canvas->prstream, "%02x", tmpbyte);
                if (linelen >= MAX_PS_LINELEN) {
                    fprintf(canvas->prstream, "\n");
                    linelen = 0;
                }
            }
            fprintf(canvas->prstream, "\n");
        }
    }
    fprintf(canvas->prstream, "GR\n");
}

void ps_puttext(const Canvas *canvas, void *data,
    const VPoint *vp, const char *s, int len, int font, const TextMatrix *tm,
    int underline, int overline, int kerning)
{
    PS_data *psdata = (PS_data *) data;
    double *kvector;
    int i;
    Pen pen;
    int linelen;
    
    fprintf(canvas->prstream, "/Font%d FFSF\n", font);

    getpen(canvas, &pen);
    ps_setpen(canvas, &pen, psdata);
    
    fprintf(canvas->prstream, "%.4f %.4f m\n", vp->x, vp->y);
    fprintf(canvas->prstream, "GS\n");
    fprintf(canvas->prstream, "[%.4f %.4f %.4f %.4f 0 0] CC\n",
                        tm->cxx, tm->cyx, tm->cxy, tm->cyy);
    
    if (kerning) {
        kvector = get_kerning_vector(canvas, s, len, font);
    } else {
        kvector = NULL;
    }
    
    if (kvector) {
        linelen = 0;
        linelen += fprintf(canvas->prstream, "[");
        for (i = 0; i < len - 1; i++) {
            linelen += fprintf(canvas->prstream, "%.4f ", kvector[i]);
            if (linelen >= MAX_PS_LINELEN) {
                fprintf(canvas->prstream, "\n");
                linelen = 0;
            }
        }
        fprintf(canvas->prstream, "] KINIT\n");
        fprintf(canvas->prstream, "{KPROC}\n");
    }
    
    put_string(psdata, canvas->prstream, s, len);

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
            fprintf(canvas->prstream, " %.4f %.4f %.4f TL", pos, w, kcomp);
        }
        if (overline) {
            pos = get_overline_pos(canvas, font);
            fprintf(canvas->prstream, " %.4f %.4f %.4f TL", pos, w, kcomp);
        }
    }
    
    if (kvector) {
        fprintf(canvas->prstream, " kshow\n");
        xfree(kvector);
    } else {
        fprintf(canvas->prstream, " show\n");
    }
    
    fprintf(canvas->prstream, "GR\n");
}


void ps_leavegraphics(const Canvas *canvas, void *data,
    const CanvasStats *cstats)
{
    PS_data *psdata = (PS_data *) data;
    if (psdata->curformat == PS_FORMAT) {
        fprintf(canvas->prstream, "showpage\n");
        fprintf(canvas->prstream, "%%%%PageTrailer\n");
    }
    fprintf(canvas->prstream, "%%%%Trailer\n");
    
    fprintf(canvas->prstream, "%%%%EOF\n");
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
        if ((psdata->docdata == DOCDATA_7BIT && !is7bit(uc)) ||
            (psdata->docdata == DOCDATA_8BIT && !is8bit(uc))) {
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

int ps_op_parser(const Canvas *canvas, void *data, const char *opstring)
{
    PS_data *psdata = (PS_data *) data;
    
    if (!strcmp(opstring, "level2")) {
        psdata->level2 = TRUE;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "level1")) {
        psdata->level2 = FALSE;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "colorspace:grayscale")) {
        psdata->colorspace = COLORSPACE_GRAYSCALE;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "colorspace:rgb")) {
        psdata->colorspace = COLORSPACE_RGB;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "colorspace:cmyk")) {
        psdata->colorspace = COLORSPACE_CMYK;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "docdata:7bit")) {
        psdata->docdata = DOCDATA_7BIT;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "docdata:8bit")) {
        psdata->docdata = DOCDATA_8BIT;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "docdata:binary")) {
        psdata->docdata = DOCDATA_BINARY;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "embedfonts:none")) {
        psdata->fonts = FONT_EMBED_NONE;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "embedfonts:but13")) {
        psdata->fonts = FONT_EMBED_BUT13;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "embedfonts:but35")) {
        psdata->fonts = FONT_EMBED_BUT35;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "embedfonts:all")) {
        psdata->fonts = FONT_EMBED_ALL;
        return RETURN_SUCCESS;
    } else if (psdata->curformat == PS_FORMAT) {
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
            psdata->feed = MEDIA_FEED_AUTO;
            return RETURN_SUCCESS;
        } else if (!strcmp(opstring, "mediafeed:match")) {
            psdata->feed = MEDIA_FEED_MATCH;
            return RETURN_SUCCESS;
        } else if (!strcmp(opstring, "mediafeed:manual")) {
            psdata->feed = MEDIA_FEED_MANUAL;
            return RETURN_SUCCESS;
        } else {
            return RETURN_FAILURE;
        }
    } else {
        return RETURN_FAILURE;
    }
}

#ifndef NONE_GUI

static void update_ps_setup_frame(PS_data *psdata);
static int set_ps_setup_proc(void *data);

static void colorspace_cb(int onoff, void *data)
{
    OptionStructure *opt = (OptionStructure *) data;
    
    OptionItem colorspace_op_items[3] = {
        {COLORSPACE_GRAYSCALE, "Grayscale"},
        {COLORSPACE_RGB,       "RGB"      },
        {COLORSPACE_CMYK,      "CMYK"     }
    };
    
    if (onoff) {
        UpdateOptionChoice(opt, 3, colorspace_op_items);
    } else {
        UpdateOptionChoice(opt, 2, colorspace_op_items);
    }
}

void ps_gui_setup(const Canvas *canvas, void *data)
{
    PS_data *psdata = (PS_data *) data;
    
    set_wait_cursor();
    
    if (psdata->frame == NULL) {
        char *title;
        Widget ps_setup_rc, fr, rc;
        OptionItem colorspace_op_items[3] = {
            {COLORSPACE_GRAYSCALE, "Grayscale"},
            {COLORSPACE_RGB,       "RGB"      },
            {COLORSPACE_CMYK,      "CMYK"     }
        };
        OptionItem docdata_op_items[3] = {
            {DOCDATA_7BIT,   "7bit"  },
            {DOCDATA_8BIT,   "8bit"  },
            {DOCDATA_BINARY, "Binary"}
        };
        OptionItem op_items[3] = {
            {MEDIA_FEED_AUTO,   "Automatic" },
            {MEDIA_FEED_MATCH,  "Match size"},
            {MEDIA_FEED_MANUAL, "Manual"    }
        };
        OptionItem font_op_items[4] = {
            {FONT_EMBED_NONE,  "None"               },
            {FONT_EMBED_BUT13, "All but 13 standard"},
            {FONT_EMBED_BUT35, "All but 35 standard"},
            {FONT_EMBED_ALL,   "All"                }
        };
        
        if (psdata->curformat == PS_FORMAT) {
            title = "PS options";
        } else {
            title = "EPS options";
        }
	psdata->frame = CreateDialogForm(app_shell, title);

        ps_setup_rc = CreateVContainer(psdata->frame);

	fr = CreateFrame(ps_setup_rc, "PS options");
        rc = CreateVContainer(fr);
	psdata->level2_item = CreateToggleButton(rc, "PS Level 2");
        psdata->colorspace_item =
            CreateOptionChoice(rc, "Colorspace:", 1, 3, colorspace_op_items);
	AddToggleButtonCB(psdata->level2_item,
            colorspace_cb, psdata->colorspace_item);
	psdata->docdata_item =
            CreateOptionChoice(rc, "Document data:", 1, 3, docdata_op_items);
	psdata->fonts_item =
            CreateOptionChoice(rc, "Embed fonts:", 1, 4, font_op_items);

        if (psdata->curformat == PS_FORMAT) {
	    fr = CreateFrame(ps_setup_rc, "Page offsets (pt)");
            rc = CreateHContainer(fr);
	    psdata->offset_x_item = CreateSpinChoice(rc,
                "X: ", 4, SPIN_TYPE_INT, -999.0, 999.0, 10.0);
	    psdata->offset_y_item = CreateSpinChoice(rc,
                "Y: ", 4, SPIN_TYPE_INT, -999.0, 999.0, 10.0);

	    fr = CreateFrame(ps_setup_rc, "Hardware");
            rc = CreateVContainer(fr);
	    psdata->feed_item = CreateOptionChoice(rc, "Media feed:", 1, 3, op_items);
	    psdata->hwres_item = CreateToggleButton(rc, "Set hardware resolution");
        }

	CreateAACDialog(psdata->frame, ps_setup_rc, set_ps_setup_proc, psdata);
    }
    update_ps_setup_frame(psdata);
    
    RaiseWindow(GetParent(psdata->frame));
    unset_wait_cursor();
}

static void update_ps_setup_frame(PS_data *psdata)
{
    if (psdata->frame) {
        SetToggleButtonState(psdata->level2_item, psdata->level2);
        SetOptionChoice(psdata->colorspace_item, psdata->colorspace);
        colorspace_cb(psdata->level2, psdata->colorspace_item);
        SetOptionChoice(psdata->fonts_item, psdata->fonts);
        SetOptionChoice(psdata->docdata_item, psdata->docdata);
        if (psdata->curformat == PS_FORMAT) {
            SetSpinChoice(psdata->offset_x_item, (double) psdata->offset_x);
            SetSpinChoice(psdata->offset_y_item, (double) psdata->offset_y);
            SetOptionChoice(psdata->feed_item, psdata->feed);
            SetToggleButtonState(psdata->hwres_item, psdata->hwres);
        }
    }
}

static int set_ps_setup_proc(void *data)
{
    PS_data *psdata = (PS_data *) data;

    psdata->level2     = GetToggleButtonState(psdata->level2_item);
    psdata->docdata    = GetOptionChoice(psdata->docdata_item);
    psdata->colorspace = GetOptionChoice(psdata->colorspace_item);
    psdata->fonts      = GetOptionChoice(psdata->fonts_item);
    if (psdata->curformat == PS_FORMAT) {
        psdata->offset_x   = (int) GetSpinChoice(psdata->offset_x_item);
        psdata->offset_y   = (int) GetSpinChoice(psdata->offset_y_item);
        psdata->feed       = GetOptionChoice(psdata->feed_item);
        psdata->hwres      = GetToggleButtonState(psdata->hwres_item);
    }
    
    return RETURN_SUCCESS;
}

#endif
