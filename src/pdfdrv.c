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
 * Grace PDF driver
 */

#include <config.h>

#ifdef HAVE_LIBPDF

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

#include <pdflib.h>

#include "cmath.h"
#include "defines.h"
#include "utils.h"
#include "draw.h"
#include "devlist.h"
#include "pdfdrv.h"

#include "protos.h"

#ifndef NONE_GUI
#  include "motifinc.h"
#endif

static void pdf_error_handler(PDF *p, int type, const char* msg);

static unsigned long page_scale;
static float pixel_size;
static float page_scalef;

static int *pdf_font_ids;
static int *pdf_pattern_ids;

static int pdf_color;
static int pdf_pattern;
static double pdf_linew;
static int pdf_lines;
static int pdf_linecap;
static int pdf_linejoin;

static PDFCompatibility pdf_setup_compat = PDF_1_3;
static PDFColorSpace pdf_setup_colorspace = DEFAULT_COLORSPACE;
static int pdf_setup_compression = 4;
static int pdf_setup_fpprec = 4;

static PDF *phandle;

int register_pdf_drv(Canvas *canvas)
{
    Device_entry *d;
    
    PDF_boot();
    
    d = device_new("PDF", DEVICE_FILE, TRUE, NULL);
    if (!d) {
        return -1;
    }
    
    device_set_fext(d, "pdf");
    
    device_set_procs(d,
        pdf_initgraphics,
        pdf_leavegraphics,
        pdf_op_parser,
        pdf_gui_setup,
        NULL,
        pdf_drawpixel,
        pdf_drawpolyline,
        pdf_fillpolygon,
        pdf_drawarc,
        pdf_fillarc,
        pdf_putpixmap,
        pdf_puttext);
    
    return register_device(canvas, d);
}

int pdf_initgraphics(const Canvas *canvas, void *data, const CanvasStats *cstats)
{
    int i;
    Page_geometry *pg;
    char *s;
   
    pg = get_page_geometry(canvas);
    
    page_scale = MIN2(pg->height, pg->width);
    pixel_size = 1.0/page_scale;
    page_scalef = (float) page_scale*72.0/pg->dpi;

    /* undefine all graphics state parameters */
    pdf_color = -1;
    pdf_pattern = -1;
    pdf_linew = -1.0;
    pdf_lines = -1;
    pdf_linecap = -1;
    pdf_linejoin = -1;

    phandle = PDF_new2(pdf_error_handler, NULL, NULL, NULL, NULL);
    if (phandle == NULL) {
        return RETURN_FAILURE;
    }

    switch (pdf_setup_compat) {
    case PDF_1_2:
        s = "1.2";
        break;
    case PDF_1_3:
        s = "1.3";
        break;
    case PDF_1_4:
        s = "1.4";
        break;
    default:
        s = "1.3";
        break;
    }
    PDF_set_parameter(phandle, "compatibility", s);

    if (PDF_open_fp(phandle, canvas->prstream) == -1) {
        return RETURN_FAILURE;
    }
    
    PDF_set_value(phandle, "compress", (float) pdf_setup_compression);
    PDF_set_value(phandle, "floatdigits", (float) pdf_setup_fpprec);

    PDF_set_info(phandle, "Creator", bi_version_string());
    PDF_set_info(phandle, "Author", canvas_get_username(canvas));
    PDF_set_info(phandle, "Title", canvas_get_docname(canvas));
        
    pdf_font_ids = xmalloc(number_of_fonts(canvas)*SIZEOF_INT);
    for (i = 0; i < number_of_fonts(canvas); i++) {
        pdf_font_ids[i] = -1;
    }
    
    if (pdf_setup_compat >= PDF_1_3) {
        pdf_pattern_ids = xmalloc(number_of_patterns(canvas)*SIZEOF_INT);
        for (i = 0; i < cstats->npatterns; i++) {
            int patno = cstats->patterns[i];
            Pattern *pat = canvas_get_pattern(canvas, patno);
/* Unfortunately, there is no way to open a _masked_ image from memory */
#if 0
            int im;
            pdf_pattern_ids[i] = PDF_begin_pattern(phandle,
                pat->width, pat->height, pat->width, pat->height, 2);
            im = PDF_open_image(phandle, "raw", "memory",
                (const char *) pat_bits[i], pat->width*pat->height/8,
                pat->width, pat->height, 1, 1, "");
            PDF_place_image(phandle, im, 0.0, 0.0, 1.0);
            PDF_close_image(phandle, im);
            PDF_end_pattern(phandle);
#else
            int j, k, l;
            pdf_pattern_ids[patno] = PDF_begin_pattern(phandle,
                pat->width, pat->height, pat->width, pat->height, 2);
            for (j = 0; j < 256; j++) {
                k = j%16;
                l = 15 - j/16;
                if ((pat->bits[j/8] >> (j%8)) & 0x01) {
                    /* the bit is set */
                    PDF_rect(phandle, (float) k, (float) l, 1.0, 1.0);
                    PDF_fill(phandle);
                }
            }
            PDF_end_pattern(phandle);
#endif
        }
    }

    PDF_begin_page(phandle, pg->width*72.0/pg->dpi, pg->height*72.0/pg->dpi);
    
    if (pdf_setup_compat >= PDF_1_3) {
        s = get_project_description();

        if (!is_empty_string(s)) {
            PDF_set_border_style(phandle, "dashed", 3.0);
            PDF_set_border_dash(phandle, 5.0, 1.0);
            PDF_set_border_color(phandle, 1.0, 0.0, 0.0);

            PDF_add_note(phandle,
                20.0, 50.0, 320.0, 100.0, s, "Project description", "note", 0);
        }
    }
    
    PDF_scale(phandle, page_scalef, page_scalef);

    return RETURN_SUCCESS;
}

void pdf_setpen(const Canvas *canvas, const Pen *pen)
{
    if (pen->color != pdf_color || pen->pattern != pdf_pattern) {
        float c1, c2, c3, c4;
        char *cstype;
        switch (pdf_setup_colorspace) {
        case COLORSPACE_GRAYSCALE:
            {
                cstype = "gray";
                
                c1 = (float) get_colorintensity(canvas, pen->color);
                c2 = c3 = c4 = 0.0;
            }
            break;
        case COLORSPACE_CMYK:
            {
                fCMYK fcmyk;
                
                cstype = "cmyk";
                
                get_fcmyk(canvas, pen->color, &fcmyk);
                c1 = (float) fcmyk.cyan;
                c2 = (float) fcmyk.magenta;
                c3 = (float) fcmyk.yellow;
                c4 = (float) fcmyk.black;
            }
            break;
        case COLORSPACE_RGB:
        default:
            {
                fRGB frgb;
                
                cstype = "rgb";
                
                get_frgb(canvas, pen->color, &frgb);
                c1 = (float) frgb.red;
                c2 = (float) frgb.green;
                c3 = (float) frgb.blue;
                c4 = 0.0;
            }
            break;
        }

        PDF_setcolor(phandle, "both", cstype, c1, c2, c3, c4);     
        if (pdf_setup_compat >= PDF_1_3 &&
            pen->pattern > 1 && pdf_pattern_ids[pen->pattern] >= 0) {
            PDF_setcolor(phandle, "both", "pattern",
                (float) pdf_pattern_ids[pen->pattern], 0.0, 0.0, 0.0);     
        }
        pdf_color = pen->color;
        pdf_pattern = pen->pattern;
    }
}

void pdf_setdrawbrush(const Canvas *canvas)
{
    int i;
    float lw;
    int ls;
    float *darray;
    Pen pen;

    getpen(canvas, &pen);
    pdf_setpen(canvas, &pen);
    
    ls = getlinestyle(canvas);
    lw = MAX2(getlinewidth(canvas), pixel_size);

    if (ls != pdf_lines || lw != pdf_linew) {    
        PDF_setlinewidth(phandle, lw);

        if (ls == 0 || ls == 1) {
            PDF_setpolydash(phandle, NULL, 0); /* length == 0,1 means solid line */
        } else {
            LineStyle *linestyle = canvas_get_linestyle(canvas, ls);
            darray = xmalloc(linestyle->length*SIZEOF_FLOAT);
            for (i = 0; i < linestyle->length; i++) {
                darray[i] = lw*linestyle->array[i];
            }
            PDF_setpolydash(phandle, darray, linestyle->length);
            xfree(darray);
        }
        pdf_linew = lw;
        pdf_lines = ls;
    }
}

void pdf_setlineprops(const Canvas *canvas)
{
    int lc, lj;
    
    lc = getlinecap(canvas);
    lj = getlinejoin(canvas);
    
    if (lc != pdf_linecap) {
        switch (lc) {
        case LINECAP_BUTT:
            PDF_setlinecap(phandle, 0);
            break;
        case LINECAP_ROUND:
            PDF_setlinecap(phandle, 1);
            break;
        case LINECAP_PROJ:
            PDF_setlinecap(phandle, 2);
            break;
        }
        pdf_linecap = lc;
    }

    if (lj != pdf_linejoin) {
        switch (lj) {
        case LINEJOIN_MITER:
            PDF_setlinejoin(phandle, 0);
            break;
        case LINEJOIN_ROUND:
            PDF_setlinejoin(phandle, 1);
            break;
        case LINEJOIN_BEVEL:
            PDF_setlinejoin(phandle, 2);
            break;
        }
        pdf_linejoin = lj;
    }
}

void pdf_drawpixel(const Canvas *canvas, void *data, const VPoint *vp)
{
    Pen pen;

    getpen(canvas, &pen);
    pdf_setpen(canvas, &pen);
    
    if (pdf_linew != pixel_size) {
        PDF_setlinewidth(phandle, pixel_size);
        pdf_linew = pixel_size;
    }
    if (pdf_linecap != LINECAP_ROUND) {
        PDF_setlinecap(phandle, 1);
        pdf_linecap = LINECAP_ROUND;
    }
    if (pdf_lines != 1) {
        PDF_setpolydash(phandle, NULL, 0);
        pdf_lines = 1;
    }

    PDF_moveto(phandle, (float) vp->x, (float) vp->y);
    PDF_lineto(phandle, (float) vp->x, (float) vp->y);
    PDF_stroke(phandle);
}

void pdf_poly_path(const VPoint *vps, int n)
{
    int i;
    
    PDF_moveto(phandle, (float) vps[0].x, (float) vps[0].y);
    for (i = 1; i < n; i++) {
        PDF_lineto(phandle, (float) vps[i].x, (float) vps[i].y);
    }
}

void pdf_drawpolyline(const Canvas *canvas, void *data,
    const VPoint *vps, int n, int mode)
{
    if (getlinestyle(canvas) == 0) {
        return;
    }
    
    pdf_setdrawbrush(canvas);
    pdf_setlineprops(canvas);
    
    pdf_poly_path(vps, n);
    
    if (mode == POLYLINE_CLOSED) {
        PDF_closepath_stroke(phandle);
    } else {
        PDF_stroke(phandle);
    }
}

void pdf_fillpolygon(const Canvas *canvas, void *data,
    const VPoint *vps, int nc)
{
    Pen pen;

    getpen(canvas, &pen);
    
    if (pen.pattern == 0) {
        return;
    }
    
    if (getfillrule(canvas) == FILLRULE_WINDING) {
        PDF_set_parameter(phandle, "fillrule", "winding");
    } else {
        PDF_set_parameter(phandle, "fillrule", "evenodd");
    }
    
    if (pdf_setup_compat >= PDF_1_3 && pen.pattern > 1) {
        Pen solid_pen;
        solid_pen.color = getbgcolor(canvas);
        solid_pen.pattern = 1;
        
        pdf_setpen(canvas, &solid_pen);
        pdf_poly_path(vps, nc);
        PDF_fill(phandle);
    }
    
    getpen(canvas, &pen);
    pdf_setpen(canvas, &pen);
    pdf_poly_path(vps, nc);
    PDF_fill(phandle);
}

void pdf_arc_path(const VPoint *vp1, const VPoint *vp2,
    double a1, double a2, int mode)
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
    
    PDF_scale(phandle, 1.0, ry/rx);
    PDF_moveto(phandle, (float) vpc.x + rx*cos(a1*M_PI/180.0), 
                        (float) rx/ry*vpc.y + rx*sin(a1*M_PI/180.0));
    if (a2 < 0) {
        PDF_arcn(phandle, (float) vpc.x, (float) rx/ry*vpc.y, rx, 
                                            (float) a1, (float) (a1 + a2));
    } else {
        PDF_arc(phandle, (float) vpc.x, (float) rx/ry*vpc.y, rx, 
                                            (float) a1, (float) (a1 + a2));
    }

    if (mode == ARCFILL_PIESLICE) {
        PDF_lineto(phandle, (float) vpc.x, (float) rx/ry*vpc.y);
    }
}

void pdf_drawarc(const Canvas *canvas, void *data,
    const VPoint *vp1, const VPoint *vp2, double a1, double a2)
{
    if (getlinestyle(canvas) == 0) {
        return;
    }
    
    pdf_setdrawbrush(canvas);
    PDF_save(phandle);
    pdf_arc_path(vp1, vp2, a1, a2, ARCFILL_CHORD);
    PDF_stroke(phandle);
    PDF_restore(phandle);
}

void pdf_fillarc(const Canvas *canvas, void *data,
    const VPoint *vp1, const VPoint *vp2, double a1, double a2, int mode)
{
    Pen pen;

    getpen(canvas, &pen);
    
    if (pen.pattern == 0) {
        return;
    }
    
    if (pdf_setup_compat >= PDF_1_3 && pen.pattern > 1) {
        Pen solid_pen;
        solid_pen.color = getbgcolor(canvas);
        solid_pen.pattern = 1;
        
        PDF_save(phandle);
        pdf_setpen(canvas, &solid_pen);
        pdf_arc_path(vp1, vp2, a1, a2, mode);
        PDF_fill(phandle);
        PDF_restore(phandle);
    }

    PDF_save(phandle);
    pdf_setpen(canvas, &pen);
    pdf_arc_path(vp1, vp2, a1, a2, mode);
    PDF_fill(phandle);
    PDF_restore(phandle);
}

/* TODO: transparent pixmaps */
void pdf_putpixmap(const Canvas *canvas, void *data,
    const VPoint *vp, int width, int height, char *databits,
    int pixmap_bpp, int bitmap_pad, int pixmap_type)
{
    char *buf, *bp;
    int image;
    int cindex;
    RGB fg, bg;
    int	i, k, j;
    long paddedW;

    int components    = 3;
        
    buf = xmalloc(width*height*components);
    if (buf == NULL) {
        errmsg("xmalloc failed in pdf_putpixmap()");
        return;
    }
    
    bp = buf;
    if (pixmap_bpp == 1) {
        paddedW = PAD(width, bitmap_pad);
        get_rgb(canvas, getcolor(canvas), &fg);
        get_rgb(canvas, getbgcolor(canvas), &bg);
        for (k = 0; k < height; k++) {
            for (j = 0; j < paddedW/bitmap_pad; j++) {
                for (i = 0; i < bitmap_pad && j*bitmap_pad + i < width; i++) {
                    if (bin_dump(&(databits)[k*paddedW/bitmap_pad + j], i, bitmap_pad)) {
                        *bp++ = (char) fg.red;
                        *bp++ = (char) fg.green;
                        *bp++ = (char) fg.blue;
                    } else {
                        *bp++ = (char) bg.red;
                        *bp++ = (char) bg.green;
                        *bp++ = (char) bg.blue;
                    }
                }
            }
        }
    } else {
        for (k = 0; k < height; k++) {
            for (j = 0; j < width; j++) {
                cindex = (databits)[k*width + j];
                get_rgb(canvas, cindex, &fg);
                *bp++ = (char) fg.red;
                *bp++ = (char) fg.green;
                *bp++ = (char) fg.blue;
            }
        }
    }
    
    image = PDF_open_image(phandle, "raw", "memory",
        buf, width*height*components,
        width, height, components, GRACE_BPP, "");
    if (image == -1) {
        errmsg("Not enough memory for image!");
        xfree(buf);
        return;
    }

    PDF_place_image(phandle,
        image, vp->x, vp->y - height*pixel_size, pixel_size);
    PDF_close_image(phandle, image);
    
    xfree(buf);
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

static int pdf_builtin_font(const char *fname)
{
    int i;
    for (i = 0; i < number_of_pdf_builtin_fonts; i++) {
        if (strcmp(pdf_builtin_fonts[i], fname) == 0) {
            return TRUE;
        }
    }
    return FALSE;
}

void pdf_puttext(const Canvas *canvas, void *data,
    const VPoint *vp, const char *s, int len, int font, const TextMatrix *tm,
    int underline, int overline, int kerning)
{
    Pen pen;

    getpen(canvas, &pen);
    pdf_setpen(canvas, &pen);
    
    if (pdf_font_ids[font] < 0) {
        char buf[GR_MAXPATHLEN];
        char *fontname, *encscheme;
        char *pdflibenc;
        int embed;
        
        fontname = get_fontalias(canvas, font);
        
        if (pdf_builtin_font(fontname)) {
            embed = 0;
        } else {
            sprintf(buf, "%s==%s",
                fontname, get_afmfilename(canvas, font, TRUE));
            PDF_set_parameter(phandle, "FontAFM", buf);
            sprintf(buf, "%s==%s",
                fontname, get_fontfilename(canvas, font, TRUE));
            PDF_set_parameter(phandle, "FontOutline", buf);

            embed = 1;
        }

        encscheme = get_encodingscheme(canvas, font);
        if (strcmp(encscheme, "FontSpecific") == 0) {
            pdflibenc = "builtin";
        } else {
            pdflibenc = "winansi";
        }
        
        pdf_font_ids[font] = PDF_findfont(phandle, fontname, pdflibenc, embed);
    } 
    
    PDF_save(phandle);
    
    PDF_setfont(phandle, pdf_font_ids[font], 1.0);

    PDF_set_parameter(phandle, "underline", true_or_false(underline));
    PDF_set_parameter(phandle, "overline",  true_or_false(overline));
    PDF_concat(phandle, (float) tm->cxx, (float) tm->cyx,
                        (float) tm->cxy, (float) tm->cyy,
                        vp->x, vp->y);

    PDF_show2(phandle, s, len);

    PDF_restore(phandle);
}

void pdf_leavegraphics(const Canvas *canvas, void *data,
    const CanvasStats *cstats)
{
    view v;
    v = cstats->bbox;

    PDF_set_value(phandle, "CropBox/llx", page_scalef*v.xv1);
    PDF_set_value(phandle, "CropBox/lly", page_scalef*v.yv1);
    PDF_set_value(phandle, "CropBox/urx", page_scalef*v.xv2);
    PDF_set_value(phandle, "CropBox/ury", page_scalef*v.yv2);
    
    PDF_end_page(phandle);
    PDF_close(phandle);
    PDF_delete(phandle);
    xfree(pdf_font_ids);
    XCFREE(pdf_pattern_ids);
}

static void pdf_error_handler(PDF *p, int type, const char *msg)
{
    char buf[MAX_STRING_LENGTH];

    switch (type) {
    case PDF_NonfatalError:
        /* continue on a non-fatal error */
        sprintf(buf, "PDFlib: %s", msg);
        errmsg(buf);
        break;
    default:
        /* give up in all other cases */
        sprintf(buf, "PDFlib: %s", msg);
        errmsg(buf);
        return;
    }
}

int pdf_op_parser(const Canvas *canvas, void *data, const char *opstring)
{
    if (!strcmp(opstring, "compatibility:PDF-1.2")) {
        pdf_setup_compat = PDF_1_2;
        return RETURN_SUCCESS;
    } else
    if (!strcmp(opstring, "compatibility:PDF-1.3")) {
        pdf_setup_compat = PDF_1_3;
        return RETURN_SUCCESS;
    } else
    if (!strcmp(opstring, "compatibility:PDF-1.4")) {
        pdf_setup_compat = PDF_1_4;
        return RETURN_SUCCESS;
    } else
    if (!strncmp(opstring, "compression:", 12)) {
        char *bufp;
        bufp = strchr(opstring, ':');
        bufp++;
        if (!is_empty_string(bufp)) {
            pdf_setup_compression = atoi(bufp);
            return RETURN_SUCCESS;
        } else {
            return RETURN_FAILURE;
        }
    } else
    if (!strncmp(opstring, "fpprecision:", 12)) {
        char *bufp;
        bufp = strchr(opstring, ':');
        bufp++;
        if (!is_empty_string(bufp)) {
            pdf_setup_fpprec = atoi(bufp);
            return RETURN_SUCCESS;
        } else {
            return RETURN_FAILURE;
        }
    } else
    if (!strcmp(opstring, "colorspace:grayscale")) {
        pdf_setup_colorspace = COLORSPACE_GRAYSCALE;
        return RETURN_SUCCESS;
    } else
    if (!strcmp(opstring, "colorspace:rgb")) {
        pdf_setup_colorspace = COLORSPACE_RGB;
        return RETURN_SUCCESS;
    } else
    if (!strcmp(opstring, "colorspace:cmyk")) {
        pdf_setup_colorspace = COLORSPACE_CMYK;
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

#ifndef NONE_GUI

static void update_pdf_setup_frame(void);
static int set_pdf_setup_proc(void *data);

static Widget pdf_setup_frame;
static OptionStructure *pdf_setup_compat_item;
static SpinStructure *pdf_setup_compression_item;
static SpinStructure *pdf_setup_fpprec_item;
static OptionStructure *pdf_setup_colorspace_item;

void pdf_gui_setup(const Canvas *canvas, void *data)
{
    set_wait_cursor();
    
    if (pdf_setup_frame == NULL) {
        Widget fr, rc;
        OptionItem compat_op_items[3] = {
            {PDF_1_2, "PDF-1.2"},
            {PDF_1_3, "PDF-1.3"},
            {PDF_1_4, "PDF-1.4"}
        };
        OptionItem colorspace_op_items[3] = {
            {COLORSPACE_GRAYSCALE, "Grayscale"},
            {COLORSPACE_RGB,       "RGB"      },
            {COLORSPACE_CMYK,      "CMYK"     }
        };
    
	pdf_setup_frame = CreateDialogForm(app_shell, "PDF options");

	fr = CreateFrame(pdf_setup_frame, "PDF options");
        rc = CreateVContainer(fr);
	pdf_setup_compat_item =
            CreateOptionChoice(rc, "Compatibility:", 1, 3, compat_op_items);
        pdf_setup_colorspace_item =
            CreateOptionChoice(rc, "Colorspace:", 1, 3, colorspace_op_items);
	pdf_setup_compression_item = CreateSpinChoice(rc,
            "Compression:", 1, SPIN_TYPE_INT, 0.0, 9.0, 1.0);
	pdf_setup_fpprec_item = CreateSpinChoice(rc,
            "FP precision:", 1, SPIN_TYPE_INT, 4.0, 6.0, 1.0);

	CreateAACDialog(pdf_setup_frame, fr, set_pdf_setup_proc, NULL);
    }
    update_pdf_setup_frame();
    RaiseWindow(GetParent(pdf_setup_frame));
    unset_wait_cursor();
}

static void update_pdf_setup_frame(void)
{
    if (pdf_setup_frame) {
        SetOptionChoice(pdf_setup_compat_item, pdf_setup_compat);
        SetOptionChoice(pdf_setup_colorspace_item, pdf_setup_colorspace);
        SetSpinChoice(pdf_setup_compression_item, (double) pdf_setup_compression);
        SetSpinChoice(pdf_setup_fpprec_item, (double) pdf_setup_fpprec);
    }
}

static int set_pdf_setup_proc(void *data)
{
    pdf_setup_compat      = GetOptionChoice(pdf_setup_compat_item);
    pdf_setup_colorspace  = GetOptionChoice(pdf_setup_colorspace_item);
    pdf_setup_compression = (int) GetSpinChoice(pdf_setup_compression_item);
    pdf_setup_fpprec      = (int) GetSpinChoice(pdf_setup_fpprec_item);
    
    return RETURN_SUCCESS;
}

#endif

#else /* No PDFlib */
void _pdfdrv_c_dummy_func(void) {}
#endif
