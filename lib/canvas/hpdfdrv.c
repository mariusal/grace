/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1996-2011 Grace Development Team
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
 * Grace Haru-based PDF driver
 */

#include <config.h>

#ifdef HAVE_HARU

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

#include <hpdf.h>

#include "grace/baseP.h"
#define CANVAS_BACKEND_API
#include "grace/canvas.h"

#define HPDF_DEFAULT_COLORSPACE  HPDF_COLORSPACE_RGB

static void hpdf_error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no,
                              void *user_data);

static HPDF_data *init_hpdf_data(void)
{
    HPDF_data *data;

    /* we need to perform the allocations */
    data = xmalloc(sizeof(HPDF_data));
    if (data == NULL) {
        return NULL;
    }

    memset(data, 0, sizeof(HPDF_data));

    data->colorspace  = HPDF_DEFAULT_COLORSPACE;
    data->compression = TRUE;
    
    return data;
}

static void hpdf_data_free(void *data)
{
    HPDF_data *pdfdata = (HPDF_data *) data;
    
    if (pdfdata) {
        xfree(pdfdata->font_ids);
        xfree(pdfdata);
    }
}

static char *pdf_builtin_fonts[] = 
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

static int number_of_pdf_builtin_fonts = sizeof(pdf_builtin_fonts)/sizeof(char *);

static int hpdf_builtin_font(const char *fname)
{
    int i;
    for (i = 0; i < number_of_pdf_builtin_fonts; i++) {
        if (strcmp(pdf_builtin_fonts[i], fname) == 0) {
            return TRUE;
        }
    }
    return FALSE;
}

int hpdf_initgraphics(const Canvas *canvas, void *data, const CanvasStats *cstats)
{
    HPDF_data *pdfdata = (HPDF_data *) data;
    unsigned int i;
    Page_geometry *pg;
    char *s;
   
    pg = get_page_geometry(canvas);
    
    pdfdata->page_scale  = MIN2(pg->height, pg->width);
    pdfdata->pixel_size  = 1.0/pdfdata->page_scale;
    pdfdata->page_scalef = (float) pdfdata->page_scale*72.0/pg->dpi;

    /* undefine all graphics state parameters */
    pdfdata->color    = -1;
    pdfdata->pattern  = -1;
    pdfdata->linew    = -1.0;
    pdfdata->lines    = -1;
    pdfdata->linecap  = -1;
    pdfdata->linejoin = -1;

    pdfdata->pdf = HPDF_New(hpdf_error_handler, pdfdata);
    if (pdfdata->pdf == NULL) {
        return RETURN_FAILURE;
    }

    if (setjmp(pdfdata->jmpenv)) {
        HPDF_Free(pdfdata->pdf);
        return RETURN_FAILURE;
    }

    pdfdata->page = HPDF_AddPage(pdfdata->pdf);

    if (pdfdata->compression) {
        HPDF_SetCompressionMode(pdfdata->pdf, HPDF_COMP_ALL);
        HPDF_Page_SetFilter(pdfdata->page, HPDF_STREAM_FILTER_FLATE_DECODE);
    } else {
        HPDF_SetCompressionMode(pdfdata->pdf, HPDF_COMP_NONE);
        HPDF_Page_SetFilter(pdfdata->page, HPDF_STREAM_FILTER_NONE);
    }

    HPDF_SetInfoAttr(pdfdata->pdf,
        HPDF_INFO_CREATOR, "Grace/libcanvas");
    HPDF_SetInfoAttr(pdfdata->pdf,
        HPDF_INFO_AUTHOR, canvas_get_username(canvas));
    HPDF_SetInfoAttr(pdfdata->pdf,
        HPDF_INFO_TITLE, canvas_get_docname(canvas));


    /* HPDF_SetPassword(pdfdata->pdf, "aaa", "bbb"); */
    
    pdfdata->font_ids = xmalloc(number_of_fonts(canvas)*sizeof(HPDF_Font));
    for (i = 0; i < number_of_fonts(canvas); i++) {
        pdfdata->font_ids[i] = NULL;
    }
    for (i = 0; i < cstats->nfonts; i++) {
        int font;
        char *fontname, *encscheme;
        char *pdflibenc;
        HPDF_Font hfont;
        
        font = cstats->fonts[i].font;
        
        fontname = get_fontalias(canvas, font);
        
        encscheme = get_encodingscheme(canvas, font);
        if (strcmp(encscheme, "FontSpecific") == 0) {
            pdflibenc = "FontSpecific";
        } else {
            pdflibenc = "WinAnsiEncoding";
        }
        
        
        if (hpdf_builtin_font(fontname)) {
            hfont = HPDF_GetFont(pdfdata->pdf, fontname, pdflibenc);
        } else {
            const char *hpdf_fontname;
            hpdf_fontname = HPDF_LoadType1FontFromFile(pdfdata->pdf,
                get_afmfilename(canvas, font, TRUE),
                get_fontfilename(canvas, font, TRUE));
            
            hfont = HPDF_GetFont(pdfdata->pdf, hpdf_fontname, NULL);
        }

        pdfdata->font_ids[font] = hfont;
        
        if (!pdfdata->font_ids[font] < 0) {
            errmsg("...");
        }
    }
    
    HPDF_Page_SetWidth (pdfdata->page, pg->width*72.0/pg->dpi);
    HPDF_Page_SetHeight(pdfdata->page, pg->height*72.0/pg->dpi);
    
#if 0
    s = canvas_get_description(canvas);

    if (!string_is_empty(s)) {
        PDF_set_border_style(pdfdata->pdf, "dashed", 3.0);
        PDF_set_border_dash(pdfdata->pdf, 5.0, 1.0);
        PDF_set_border_color(pdfdata->pdf, 1.0, 0.0, 0.0);

        PDF_add_note(pdfdata->pdf,
            20.0, 50.0, 320.0, 100.0, s, "Project description", "note", 0);
    }
    
#endif
    HPDF_Page_Concat(pdfdata->page, pdfdata->page_scalef, 0, 0,
                                    pdfdata->page_scalef, 0, 0);

    return RETURN_SUCCESS;
}

void hpdf_setpen(const Canvas *canvas, const Pen *pen, HPDF_data *pdfdata)
{
    if (pen->color != pdfdata->color || pen->pattern != pdfdata->pattern) {
        float c1, c2, c3, c4;
        switch (pdfdata->colorspace) {
        case HPDF_COLORSPACE_GRAYSCALE:
            {
                c1 = (float) get_colorintensity(canvas, pen->color);
                HPDF_Page_SetGrayFill  (pdfdata->page, c1);
                HPDF_Page_SetGrayStroke(pdfdata->page, c1);
            }
            break;
        case HPDF_COLORSPACE_CMYK:
            {
                fCMYK fcmyk;
                
                get_fcmyk(canvas, pen->color, &fcmyk);
                c1 = (float) fcmyk.cyan;
                c2 = (float) fcmyk.magenta;
                c3 = (float) fcmyk.yellow;
                c4 = (float) fcmyk.black;

                HPDF_Page_SetCMYKFill  (pdfdata->page, c1, c2, c3, c4);
                HPDF_Page_SetCMYKStroke(pdfdata->page, c1, c2, c3, c4);
            }
            break;
        case HPDF_COLORSPACE_RGB:
        default:
            {
                fRGB frgb;
                
                get_frgb(canvas, pen->color, &frgb);
                c1 = (float) frgb.red;
                c2 = (float) frgb.green;
                c3 = (float) frgb.blue;

                HPDF_Page_SetRGBFill  (pdfdata->page, c1, c2, c3);
                HPDF_Page_SetRGBStroke(pdfdata->page, c1, c2, c3);
            }
            break;
        }

        pdfdata->color   = pen->color;
        pdfdata->pattern = pen->pattern;
    }
}

void hpdf_setdrawbrush(const Canvas *canvas, HPDF_data *pdfdata)
{
    unsigned int i;
    float lw;
    int ls;
    HPDF_REAL *darray;
    Pen pen;

    getpen(canvas, &pen);
    hpdf_setpen(canvas, &pen, pdfdata);
    
    ls = getlinestyle(canvas);
    lw = MAX2(getlinewidth(canvas), pdfdata->pixel_size);

    if (ls != pdfdata->lines || lw != pdfdata->linew) {    
        HPDF_Page_SetLineWidth(pdfdata->page, lw);

        if (ls == 0 || ls == 1) {
            HPDF_Page_SetDash(pdfdata->page, NULL, 0, 0);
        } else {
            LineStyle *linestyle = canvas_get_linestyle(canvas, ls);
            darray = xmalloc(linestyle->length*sizeof(HPDF_REAL));
            for (i = 0; i < linestyle->length; i++) {
                darray[i] = lw*linestyle->array[i];
            }
            HPDF_Page_SetDash(pdfdata->page, darray, linestyle->length, 0);
            xfree(darray);
        }
        pdfdata->linew = lw;
        pdfdata->lines = ls;
    }
}

void hpdf_setlineprops(const Canvas *canvas, HPDF_data *pdfdata)
{
    int lc, lj;
    
    lc = getlinecap(canvas);
    lj = getlinejoin(canvas);
    
    if (lc != pdfdata->linecap) {
        switch (lc) {
        case LINECAP_BUTT:
            HPDF_Page_SetLineCap(pdfdata->page, HPDF_BUTT_END);
            break;
        case LINECAP_ROUND:
            HPDF_Page_SetLineCap(pdfdata->page, HPDF_ROUND_END);
            break;
        case LINECAP_PROJ:
            HPDF_Page_SetLineCap(pdfdata->page, HPDF_PROJECTING_SCUARE_END);
            break;
        }
        pdfdata->linecap = lc;
    }

    if (lj != pdfdata->linejoin) {
        switch (lj) {
        case LINEJOIN_MITER:
            HPDF_Page_SetLineJoin(pdfdata->page, HPDF_MITER_JOIN);
            break;
        case LINEJOIN_ROUND:
            HPDF_Page_SetLineJoin(pdfdata->page, HPDF_ROUND_JOIN);
            break;
        case LINEJOIN_BEVEL:
            HPDF_Page_SetLineJoin(pdfdata->page, HPDF_BEVEL_JOIN);
            break;
        }
        pdfdata->linejoin = lj;
    }
}

void hpdf_drawpixel(const Canvas *canvas, void *data, const VPoint *vp)
{
    HPDF_data *pdfdata = (HPDF_data *) data;
    Pen pen;

    getpen(canvas, &pen);
    hpdf_setpen(canvas, &pen, pdfdata);
    
    if (pdfdata->linew != pdfdata->pixel_size) {
        HPDF_Page_SetLineWidth(pdfdata->page, pdfdata->pixel_size);
        pdfdata->linew = pdfdata->pixel_size;
    }
    if (pdfdata->linecap != LINECAP_ROUND) {
        HPDF_Page_SetLineCap(pdfdata->page, HPDF_ROUND_END);
        pdfdata->linecap = LINECAP_ROUND;
    }
    if (pdfdata->lines != 1) {
        HPDF_Page_SetDash(pdfdata->page, NULL, 0, 0);
        pdfdata->lines = 1;
    }

    HPDF_Page_MoveTo(pdfdata->page, (float) vp->x, (float) vp->y);
    HPDF_Page_LineTo(pdfdata->page, (float) vp->x, (float) vp->y);
    HPDF_Page_Stroke(pdfdata->page);
}

void hpdf_poly_path(const VPoint *vps, int n, HPDF_data *pdfdata)
{
    int i;
    
    HPDF_Page_MoveTo(pdfdata->page, (float) vps[0].x, (float) vps[0].y);
    for (i = 1; i < n; i++) {
        HPDF_Page_LineTo(pdfdata->page, (float) vps[i].x, (float) vps[i].y);
    }
}

void hpdf_drawpolyline(const Canvas *canvas, void *data,
    const VPoint *vps, int n, int mode)
{
    HPDF_data *pdfdata = (HPDF_data *) data;
    if (getlinestyle(canvas) == 0) {
        return;
    }
    
    hpdf_setdrawbrush(canvas, pdfdata);
    hpdf_setlineprops(canvas, pdfdata);
    
    hpdf_poly_path(vps, n, pdfdata);
    
    if (mode == POLYLINE_CLOSED) {
        HPDF_Page_ClosePathStroke(pdfdata->page);
    } else {
        HPDF_Page_Stroke(pdfdata->page);
    }
}

void hpdf_fillpolygon(const Canvas *canvas, void *data,
    const VPoint *vps, int nc)
{
    HPDF_data *pdfdata = (HPDF_data *) data;
    Pen pen;

    getpen(canvas, &pen);
    
    if (pen.pattern == 0) {
        return;
    }
    
#if 0
    if (getfillrule(canvas) == FILLRULE_WINDING) {
        PDF_set_parameter(pdfdata->pdf, "fillrule", "winding");
    } else {
        PDF_set_parameter(pdfdata->pdf, "fillrule", "evenodd");
    }
#endif
    
    if (pen.pattern > 1) {
        Pen solid_pen;
        solid_pen.color = getbgcolor(canvas);
        solid_pen.pattern = 1;
        
        hpdf_setpen(canvas, &solid_pen, pdfdata);
        hpdf_poly_path(vps, nc, pdfdata);
        HPDF_Page_Fill(pdfdata->page);
    }
    
    getpen(canvas, &pen);
    hpdf_setpen(canvas, &pen, pdfdata);
    hpdf_poly_path(vps, nc, pdfdata);
    HPDF_Page_Fill(pdfdata->page);
}

void hpdf_arc_path(const VPoint *vp1, const VPoint *vp2,
    double a1, double a2, int mode, HPDF_data *pdfdata)
{
    VPoint vpc;
    double rx, ry;

    vpc.x = (vp1->x + vp2->x)/2;
    vpc.y = (vp1->y + vp2->y)/2;
    rx = fabs(vp2->x - vp1->x)/2;
    ry = fabs(vp2->y - vp1->y)/2;
    
    if (rx == 0.0 || ry == 0.0) {
        return;
    }
    
    if (a2 < 360.0 - 1.0e-3) {
        double ha1, ha2;
        
        HPDF_Page_Concat(pdfdata->page, 1.0, 0, 0, ry/rx, 0, 0);
        HPDF_Page_MoveTo(pdfdata->page, (float) vpc.x + rx*cos(a1*M_PI/180.0), 
                                     (float) rx/ry*vpc.y + rx*sin(a1*M_PI/180.0));

        /* hPDF counts from pi/2 in clockwise direction! */
        ha1 = 90 - (a1 + a2);
        ha2 = 90 - a1;
        if (ha1 > ha2) {
            fswap(&ha1, &ha2);
        }

        HPDF_Page_Arc(pdfdata->page, vpc.x, rx/ry*vpc.y, rx, ha1, ha2);

        if (mode == ARCCLOSURE_PIESLICE) {
            HPDF_Page_LineTo(pdfdata->page, (HPDF_REAL) vpc.x, (HPDF_REAL) rx/ry*vpc.y);
        }
    } else {
        HPDF_Page_Ellipse(pdfdata->page, vpc.x, vpc.y, rx, ry);
    }
}

void hpdf_drawarc(const Canvas *canvas, void *data,
    const VPoint *vp1, const VPoint *vp2, double a1, double a2)
{
    HPDF_data *pdfdata = (HPDF_data *) data;

    if (getlinestyle(canvas) == 0) {
        return;
    }
    
    hpdf_setdrawbrush(canvas, pdfdata);
    HPDF_Page_GSave(pdfdata->page);
    hpdf_arc_path(vp1, vp2, a1, a2, ARCCLOSURE_CHORD, pdfdata);
    HPDF_Page_Stroke(pdfdata->page);
    HPDF_Page_GRestore(pdfdata->page);
}

void hpdf_fillarc(const Canvas *canvas, void *data,
    const VPoint *vp1, const VPoint *vp2, double a1, double a2, int mode)
{
    HPDF_data *pdfdata = (HPDF_data *) data;
    Pen pen;

    getpen(canvas, &pen);
    
    if (pen.pattern == 0) {
        return;
    }
    
    if (pen.pattern > 1) {
        Pen solid_pen;
        solid_pen.color = getbgcolor(canvas);
        solid_pen.pattern = 1;
        
        hpdf_setpen(canvas, &solid_pen, pdfdata);
        HPDF_Page_GSave(pdfdata->page);
        hpdf_arc_path(vp1, vp2, a1, a2, mode, pdfdata);
        HPDF_Page_Fill(pdfdata->page);
        HPDF_Page_GRestore(pdfdata->page);
    }

    hpdf_setpen(canvas, &pen, pdfdata);
    HPDF_Page_GSave(pdfdata->page);
    hpdf_arc_path(vp1, vp2, a1, a2, mode, pdfdata);
    HPDF_Page_Fill(pdfdata->page);
    HPDF_Page_GRestore(pdfdata->page);
}

/* TODO */
void hpdf_putpixmap(const Canvas *canvas, void *data,
    const VPoint *vp, const CPixmap *pm)
{
    return;
}

void hpdf_puttext(const Canvas *canvas, void *data,
    const VPoint *vp, const char *s, int len, int font, const TextMatrix *tm,
    int underline, int overline, int kerning)
{
    HPDF_data *pdfdata = (HPDF_data *) data;
    Pen pen;
    char *buf;

    if (!pdfdata->font_ids[font]) {
        return;
    } 
    
    getpen(canvas, &pen);
    hpdf_setpen(canvas, &pen, pdfdata);
    
    HPDF_Page_GSave(pdfdata->page);
    
    HPDF_Page_Concat(pdfdata->page, (float) tm->cxx, (float) tm->cyx,
                                    (float) tm->cxy, (float) tm->cyy,
                                    vp->x, vp->y);
    
    HPDF_Page_BeginText(pdfdata->page);
    
    HPDF_Page_SetFontAndSize(pdfdata->page, pdfdata->font_ids[font], 1.0);
#if 0
    PDF_set_parameter(pdfdata->pdf, "underline", true_or_false(underline));
    PDF_set_parameter(pdfdata->pdf, "overline",  true_or_false(overline));
    if (pdfdata->kerning_supported) {
        PDF_set_parameter(pdfdata->pdf,
            "kerning", kerning ? "true":"false");
    }
#endif

    buf = xmalloc(len + 1);
    strncpy(buf, s, len);
    buf[len] = '\0';
    HPDF_Page_ShowText(pdfdata->page, buf);
    xfree(buf);

    HPDF_Page_EndText(pdfdata->page);
    HPDF_Page_GRestore(pdfdata->page);
}

void hpdf_leavegraphics(const Canvas *canvas, void *data,
    const CanvasStats *cstats)
{
    HPDF_data *pdfdata = (HPDF_data *) data;
    view v;
    v = cstats->bbox;

#if 0
    PDF_set_value(pdfdata->pdf, "CropBox/llx", pdfdata->page_scalef*v.xv1);
    PDF_set_value(pdfdata->pdf, "CropBox/lly", pdfdata->page_scalef*v.yv1);
    PDF_set_value(pdfdata->pdf, "CropBox/urx", pdfdata->page_scalef*v.xv2);
    PDF_set_value(pdfdata->pdf, "CropBox/ury", pdfdata->page_scalef*v.yv2);
#endif

    HPDF_SaveToStream(pdfdata->pdf);
    
    while (1) {
        HPDF_BYTE   buf[4096];
        HPDF_UINT32 size = 4096;
        HPDF_STATUS hstatus = HPDF_ReadFromStream(pdfdata->pdf, buf, &size);
        
        if (size > 0) {
            fwrite(buf, size, 1, canvas_get_prstream(canvas));
        }
        
        if (hstatus != HPDF_OK ) {
            break;
        }
    }
    
    HPDF_Free(pdfdata->pdf);

    XCFREE(pdfdata->font_ids);
}

static void hpdf_error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no,
                              void *user_data)
{
    HPDF_data *pdfdata = (HPDF_data *) user_data;
    
    printf ("ERROR: error_no=%04X, detail_no=%u\n", (HPDF_UINT) error_no,
                (HPDF_UINT) detail_no);
    
    longjmp(pdfdata->jmpenv, 1);
}

int hpdf_op_parser(const Canvas *canvas, void *data, const char *opstring)
{
    HPDF_data *pdfdata = (HPDF_data *) data;

    if (!strcmp(opstring, "compression:on")) {
        pdfdata->compression = TRUE;
        return RETURN_SUCCESS;
    } else
    if (!strcmp(opstring, "compression:off")) {
        pdfdata->compression = FALSE;
        return RETURN_SUCCESS;
    } else
    if (!strcmp(opstring, "colorspace:grayscale")) {
        pdfdata->colorspace = HPDF_COLORSPACE_GRAYSCALE;
        return RETURN_SUCCESS;
    } else
    if (!strcmp(opstring, "colorspace:rgb")) {
        pdfdata->colorspace = HPDF_COLORSPACE_RGB;
        return RETURN_SUCCESS;
    } else
    if (!strcmp(opstring, "colorspace:cmyk")) {
        pdfdata->colorspace = HPDF_COLORSPACE_CMYK;
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int register_hpdf_drv(Canvas *canvas)
{
    Device_entry *d;
    HPDF_data *data;
    
    data = init_hpdf_data();
    if (!data) {
        return -1;
    }

    d = device_new("hPDF", DEVICE_FILE, TRUE, data, hpdf_data_free);
    if (!d) {
        xfree(data);
        return -1;
    }
    
    device_set_fext(d, "pdf");

    device_set_dpi(d, 300.0);
    
    device_set_procs(d,
        hpdf_initgraphics,
        hpdf_leavegraphics,
        hpdf_op_parser,
        NULL,
        hpdf_drawpixel,
        hpdf_drawpolyline,
        hpdf_fillpolygon,
        hpdf_drawarc,
        hpdf_fillarc,
        hpdf_putpixmap,
        hpdf_puttext);
    
    return register_device(canvas, d);
}

#else /* No libHaru */
void _hpdfdrv_c_dummy_func(void) {}
#endif
