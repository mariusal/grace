/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2000 Grace Development Team
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
 * GRACE PDF driver
 */

#include <config.h>

#ifdef HAVE_LIBPDF

#include <cmath.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

#include "defines.h"
#include "utils.h"
#include "draw.h"
#include "device.h"
#include "devlist.h"
#include "patterns.h"
#include "pdfdrv.h"

#include "protos.h"

#include <pdflib.h>

#ifndef NONE_GUI
#  include <Xm/Xm.h>
#  include <Xm/Form.h>
#  include <Xm/RowColumn.h>
#  include <Xm/DialogS.h>

#  include "motifinc.h"
#endif

static void pdf_error_handler(PDF *p, int type, const char* msg);

static unsigned long page_scale;
static float pixel_size;
static float page_scalef;

static int *pdf_font_ids;

static int pdf_color;
static int pdf_pattern;
static double pdf_linew;
static int pdf_lines;
static int pdf_linecap;
static int pdf_linejoin;

static int pdf_setup_pdf1_3 = TRUE;
static int pdf_setup_compression = 4;

extern FILE *prstream;

static PDF *phandle;

static Device_entry dev_pdf = {DEVICE_FILE,
          "PDF",
          pdfinitgraphics,
          pdf_op_parser,
          pdf_gui_setup,
          "pdf",
          TRUE,
          FALSE,
          {612, 792, 72.0},
          NULL
         };

int register_pdf_drv(void)
{
    PDF_boot();
    return register_device(dev_pdf);
}

int pdfinitgraphics(void)
{
    int i;
    Page_geometry pg;
    char *s, buf[32];
   
    /* device-dependent routines */
    devupdatecmap   = NULL;
    
    devdrawpixel    = pdf_drawpixel;
    devdrawpolyline = pdf_drawpolyline;
    devfillpolygon  = pdf_fillpolygon;
    devdrawarc      = pdf_drawarc;
    devfillarc      = pdf_fillarc;
    devputpixmap    = pdf_putpixmap;
    devputtext      = pdf_puttext;
    
    devleavegraphics = pdf_leavegraphics;
    
    pg = get_page_geometry();
    
    page_scale = MIN2(pg.height, pg.width);
    pixel_size = 1.0/page_scale;
    page_scalef = (float) page_scale*72.0/pg.dpi;

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

    PDF_set_parameter(phandle, "prefix", get_grace_home());

    sprintf(buf, "%d", pdf_setup_compression);
    PDF_set_parameter(phandle, "compress", buf);

    if (pdf_setup_pdf1_3 == TRUE) {
        s = "1.3";
    } else {
        s = "1.2";
    }
    PDF_set_parameter(phandle, "compatibility", s);

    if (PDF_open_fp(phandle, prstream) == -1) {
        return RETURN_FAILURE;
    }
    
    PDF_set_info(phandle, "Creator", bi_version_string());
    PDF_set_info(phandle, "Author", get_username());
    PDF_set_info(phandle, "Title", get_docname());
        
    pdf_font_ids = xmalloc(number_of_fonts()*SIZEOF_INT);
    for (i = 0; i < number_of_fonts(); i++) {
        pdf_font_ids[i] = -1;
    }
    
    PDF_begin_page(phandle, pg.width*72.0/pg.dpi, pg.height*72.0/pg.dpi);

    if ((s = get_project_description())) {
        PDF_set_border_style(phandle, "dashed", 3.0);
        PDF_set_border_dash(phandle, 5.0, 1.0);
        PDF_set_border_color(phandle, 1.0, 0.0, 0.0);

        PDF_add_note(phandle,
            20.0, 50.0, 320.0, 100.0, s, "Project description", "note", 0);
    }
    
    PDF_scale(phandle, page_scalef, page_scalef);

    return RETURN_SUCCESS;
}

void pdf_setpen(void)
{
    Pen pen;
    fRGB *frgb;
    
    pen = getpen();
    if (pen.color != pdf_color || pen.pattern != pdf_pattern) {
        frgb = get_frgb(pen.color);
        PDF_setrgbcolor(phandle,
                    (float) frgb->red, (float) frgb->green,(float) frgb->blue);     
        /* TODO: patterns */
        pdf_color = pen.color;
        pdf_pattern = pen.pattern;
    }
}

void pdf_setdrawbrush(void)
{
    int i;
    float lw;
    int ls;
    float *darray;

    pdf_setpen();
    
    ls = getlinestyle();
    lw = MAX2(getlinewidth(), pixel_size);

    if (ls != pdf_lines || lw != pdf_linew) {    
        PDF_setlinewidth(phandle, lw);

        if (ls == 0 || ls == 1) {
            PDF_setpolydash(phandle, NULL, 0); /* length == 0,1 means solid line */
        } else {
            darray = xmalloc(dash_array_length[ls]*SIZEOF_FLOAT);
            for (i = 0; i < dash_array_length[ls]; i++) {
                darray[i] = lw*dash_array[ls][i];
            }
            PDF_setpolydash(phandle, darray, dash_array_length[ls]);
            xfree(darray);
        }
        pdf_linew = lw;
        pdf_lines = ls;
    }
}

void pdf_setlineprops(void)
{
    int lc, lj;
    
    lc = getlinecap();
    lj = getlinejoin();
    
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

void pdf_drawpixel(VPoint vp)
{
    pdf_setpen();
    
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

    PDF_moveto(phandle, (float) vp.x, (float) vp.y);
    PDF_lineto(phandle, (float) vp.x, (float) vp.y);
    PDF_stroke(phandle);
}

void pdf_drawpolyline(VPoint *vps, int n, int mode)
{
    int i;
    
    if (getlinestyle() == 0) {
        return;
    }
    
    pdf_setdrawbrush();
    pdf_setlineprops();
    
    PDF_moveto(phandle, (float) vps[0].x, (float) vps[0].y);
    for (i = 1; i < n; i++) {
        PDF_lineto(phandle, (float) vps[i].x, (float) vps[i].y);
    }
    if (mode == POLYLINE_CLOSED) {
        PDF_closepath_stroke(phandle);
    } else {
        PDF_stroke(phandle);
    }
}

void pdf_fillpolygon(VPoint *vps, int nc)
{
    int i;
    
    if (getpattern() == 0) {
        return;
    }
    
    pdf_setpen();
    
    PDF_moveto(phandle, (float) vps[0].x, (float) vps[0].y);
    for (i = 1; i < nc; i++) {
        PDF_lineto(phandle, (float) vps[i].x, (float) vps[i].y);
    }
    if (getfillrule() == FILLRULE_WINDING) {
        PDF_set_fillrule(phandle, "winding");
    } else {
        PDF_set_fillrule(phandle, "evenodd");
    }
    PDF_fill(phandle);
}

void pdf_drawarc(VPoint vp1, VPoint vp2, int a1, int a2)
{
    VPoint vpc;
    double rx, ry;
    
    if (getlinestyle() == 0) {
        return;
    }
    
    pdf_setdrawbrush();
    
    vpc.x = (vp1.x + vp2.x)/2;
    vpc.y = (vp1.y + vp2.y)/2;
    rx = fabs(vp2.x - vp1.x)/2;
    ry = fabs(vp2.y - vp1.y)/2;
    
    if (rx == 0.0 || ry == 0.0) {
        return;
    }
    
    PDF_save(phandle);
    PDF_scale(phandle, 1.0, ry/rx);
    PDF_moveto(phandle, (float) vpc.x + rx*cos(a1*M_PI/180.0), 
                        (float) rx/ry*vpc.y + rx*sin(a1*M_PI/180.0));
    PDF_arc(phandle, (float) vpc.x, (float) rx/ry*vpc.y, rx, 
                                        (float) a1, (float) a2);
    PDF_stroke(phandle);
    PDF_restore(phandle);
}

void pdf_fillarc(VPoint vp1, VPoint vp2, int a1, int a2, int mode)
{
    VPoint vpc;
    double rx, ry;
    
    if (getpattern() == 0) {
        return;
    }
    
    pdf_setpen();
    
    vpc.x = (vp1.x + vp2.x)/2;
    vpc.y = (vp1.y + vp2.y)/2;
    rx = fabs(vp2.x - vp1.x)/2;
    ry = fabs(vp2.y - vp1.y)/2;
    
    if (rx == 0.0 || ry == 0.0) {
        return;
    }
    
    PDF_save(phandle);
    PDF_scale(phandle, 1.0, ry/rx);
    PDF_moveto(phandle, (float) vpc.x + rx*cos(a1*M_PI/180.0), 
                        (float) rx/ry*vpc.y + rx*sin(a1*M_PI/180.0));
    PDF_arc(phandle, (float) vpc.x, (float) rx/ry*vpc.y, rx, 
                                        (float) a1, (float) a2);
    if (mode == ARCFILL_PIESLICE) {
        PDF_lineto(phandle, (float) vpc.x, (float) rx/ry*vpc.y);
    }
    PDF_fill(phandle);
    PDF_restore(phandle);
}

/* TODO: transparent pixmaps */
void pdf_putpixmap(VPoint vp, int width, int height, char *databits, 
                             int pixmap_bpp, int bitmap_pad, int pixmap_type)
{
    char *buf, *bp;
    int image;
    int cindex;
    RGB *fg, *bg;
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
        fg = get_rgb(getcolor());
        bg = get_rgb(getbgcolor());
        for (k = 0; k < height; k++) {
            for (j = 0; j < paddedW/bitmap_pad; j++) {
                for (i = 0; i < bitmap_pad && j*bitmap_pad + i < width; i++) {
                    if (bin_dump(&(databits)[k*paddedW/bitmap_pad + j], i, bitmap_pad)) {
                        *bp++ = (char) fg->red;
                        *bp++ = (char) fg->green;
                        *bp++ = (char) fg->blue;
                    } else {
                        *bp++ = (char) bg->red;
                        *bp++ = (char) bg->green;
                        *bp++ = (char) bg->blue;
                    }
                }
            }
        }
    } else {
        for (k = 0; k < height; k++) {
            for (j = 0; j < width; j++) {
                cindex = (databits)[k*width + j];
                fg = get_rgb(cindex);
                *bp++ = (char) fg->red;
                *bp++ = (char) fg->green;
                *bp++ = (char) fg->blue;
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

    PDF_place_image(phandle, image, vp.x, vp.y - height*pixel_size, pixel_size);
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

void pdf_puttext(VPoint vp, char *s, int len, int font,
     TextMatrix *tm, int underline, int overline, int kerning)
{
    pdf_setpen();
    
    if (pdf_font_ids[font] < 0) {
        char buf[GR_MAXPATHLEN];
        char *fontname, *encscheme;
        char *pdflibenc;
        int embed;
        
        fontname = get_fontalias(font);
        
        if (pdf_builtin_font(fontname)) {
            embed = 0;
        } else {
            sprintf(buf, "%s=fonts/type1/%s",
                fontname, get_afmfilename(font));
            PDF_set_parameter(phandle, "FontAFM", buf);
            sprintf(buf, "%s=fonts/type1/%s",
                fontname, get_fontfilename(font));
            PDF_set_parameter(phandle, "FontOutline", buf);

            embed = 1;
        }

        encscheme = get_encodingscheme(font);
        if (strcmp(encscheme, "FontSpecific") == 0) {
            pdflibenc = "builtin";
        } else {
            pdflibenc = "winansi";
        }
        
        pdf_font_ids[font] = PDF_findfont(phandle, fontname, pdflibenc, embed);
    } 
    PDF_setfont(phandle, pdf_font_ids[font], 1.0);

    PDF_set_parameter(phandle, "underline", true_or_false(underline));
    PDF_set_parameter(phandle, "overline",  true_or_false(overline));
    
    PDF_set_text_matrix(phandle, (float) tm->cxx, (float) tm->cyx,
                                 (float) tm->cxy, (float) tm->cyy,
                                 vp.x, vp.y);

    PDF_show2(phandle, s, len);

}

void pdf_leavegraphics(void)
{
    PDF_end_page(phandle);
    PDF_close(phandle);
    PDF_delete(phandle);
    xfree(pdf_font_ids);
}

static void pdf_error_handler(PDF *p, int type, const char* msg)
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

int pdf_op_parser(char *opstring)
{
    if (!strcmp(opstring, "PDF1.3")) {
        pdf_setup_pdf1_3 = TRUE;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "PDF1.2")) {
        pdf_setup_pdf1_3 = FALSE;
        return RETURN_SUCCESS;
    } else if (!strncmp(opstring, "compression:", 12)) {
        char *bufp;
        bufp = strchr(opstring, ':');
        bufp++;
        if (bufp != NULL && *bufp != '\0') {
            pdf_setup_compression = atoi(bufp);
            return RETURN_SUCCESS;
        } else {
            return RETURN_FAILURE;
        }
    } else {
        return RETURN_FAILURE;
    }
}

#ifndef NONE_GUI

static void update_pdf_setup_frame(void);
static void set_pdf_setup_proc(void *data);

static Widget pdf_setup_frame;
static Widget pdf_setup_pdf1_3_item;
static SpinStructure *pdf_setup_compression_item;

void pdf_gui_setup(void)
{
    Widget pdf_setup_panel, pdf_setup_rc, fr, rc;
    
    set_wait_cursor();
    if (pdf_setup_frame == NULL) {
	pdf_setup_frame = XmCreateDialogShell(app_shell, "PDF options", NULL, 0);
	handle_close(pdf_setup_frame);
        pdf_setup_panel = XtVaCreateWidget("device_panel", xmFormWidgetClass, 
                                        pdf_setup_frame, NULL, 0);
        pdf_setup_rc = XmCreateRowColumn(pdf_setup_panel, "rc", NULL, 0);

	fr = CreateFrame(pdf_setup_rc, "PDF options");
        rc = XmCreateRowColumn(fr, "rc", NULL, 0);
	pdf_setup_pdf1_3_item = CreateToggleButton(rc, "PDF-1.3");
	pdf_setup_compression_item = CreateSpinChoice(rc,
            "Compression:", 1, SPIN_TYPE_INT, 0.0, 9.0, 1.0);

	XtManageChild(rc);

	CreateSeparator(pdf_setup_rc);

	CreateAACButtons(pdf_setup_rc, pdf_setup_panel, set_pdf_setup_proc);
        
	XtManageChild(pdf_setup_rc);
	XtManageChild(pdf_setup_panel);
    }
    XtRaise(pdf_setup_frame);
    update_pdf_setup_frame();
    unset_wait_cursor();
}

static void update_pdf_setup_frame(void)
{
    if (pdf_setup_frame) {
        SetToggleButtonState(pdf_setup_pdf1_3_item, pdf_setup_pdf1_3);
        SetSpinChoice(pdf_setup_compression_item, (double) pdf_setup_compression);
    }
}

static void set_pdf_setup_proc(void *data)
{
    int aac_mode;
    aac_mode = (int) data;
    
    if (aac_mode == AAC_CLOSE) {
        XtUnmanageChild(pdf_setup_frame);
        return;
    }
    
    pdf_setup_pdf1_3 = GetToggleButtonState(pdf_setup_pdf1_3_item);
    pdf_setup_compression = (int) GetSpinChoice(pdf_setup_compression_item);
    
    if (aac_mode == AAC_ACCEPT) {
        XtUnmanageChild(pdf_setup_frame);
    }
}

#endif

#else /* No PDFlib */
void _pdfdrv_c_dummy_func(void) {}
#endif
