/*
 * Grace - Graphics for Exploratory Data Analysis
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-95 Paul J Turner, Portland, OR
 * Copyright (c) 1996-98 GRACE Development Team
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
#include <cmath.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

#include "defines.h"
#include "globals.h"
#include "utils.h"
#include "draw.h"
#include "device.h"
#include "patterns.h"
#include "t1fonts.h"
#include "pdfdrv.h"

#include "patchlevel.h"

#include "protos.h"

/* uncomment this if the PDFlib was compiled w/ support for TIFF images */
/* #define USE_TIFF */
#include <pdf.h>

#ifndef NONE_GUI
#  include <Xm/Xm.h>
#  include <Xm/BulletinB.h>
#  include <Xm/RowColumn.h>

#  include "motifinc.h"
#endif

static void pdf_error_handler(int level, const char* fmt, va_list ap);

static unsigned long page_scale;
static float page_scalef;

static int pdf_setup_binary = TRUE;

extern FILE *prstream;

static PDF *phandle;

int pdfinitgraphics(void)
{
    int i, buflen;
    Page_geometry pg;
    PDF_info *info;
    static char buf_cr[64], buf_fp[GR_MAXPATHLEN], buf[64];
    
    /* device-dependent routines */
    devupdatecmap   = NULL;
    
    devdrawpolyline = pdf_drawpolyline;
    devfillpolygon  = pdf_fillpolygon;
    devdrawarc      = pdf_drawarc;
    devfillarc      = pdf_fillarc;
    devputpixmap    = pdf_putpixmap;
    devputtext      = pdf_puttext;
    
    devleavegraphics = pdf_leavegraphics;
    
    pg = get_page_geometry();
    
    page_scale = MIN2(pg.height, pg.width);
    page_scalef = (float) page_scale*72.0/pg.dpi_x;

    info = PDF_get_info();
    
    sprintf(buf_cr, "Grace v%d.%d.%d %s\n",
                                MAJOR_REV, MINOR_REV, PATCHLEVEL, BETA_VER);
    info->Creator = buf_cr;
    info->Author = getlogin();
    info->Title = docname;

    info->binary_mode = pdf_setup_binary;
    
    sprintf(buf_fp, "%s/fonts/type1", get_grace_home());
    info->fontpath = buf_fp;

    info->error_handler = pdf_error_handler;
           
    phandle = PDF_open(prstream, info);
    
    if (phandle == NULL) {
        return GRACE_EXIT_FAILURE;
    }
    
    for (i = 0; i < number_of_fonts(); i++) {
   	strcpy(buf, mybasename(get_fontfilename(i))); 
   	buflen = 0;
        while (buf[buflen] != '\0' && buf[buflen] != '.') {
            buflen++;
        }
        buf[buflen] = '\0';
        strcat(buf, ".afm");
        
        PDF_add_font_alias(phandle, get_fontalias(i), buf, get_fontfilename(i));
    }
    
    PDF_begin_page(phandle, pg.width*72.0/pg.dpi_x, pg.height*72.0/pg.dpi_y);
    PDF_scale(phandle, page_scalef, page_scalef);
    
    return GRACE_EXIT_SUCCESS;
}

void pdf_setpen(void)
{
    Pen pen;
    fRGB *frgb;
    
    pen = getpen();
    frgb = get_frgb(pen.color);
    PDF_setrgbcolor(phandle,
                    (float) frgb->red, (float) frgb->green,(float) frgb->blue);     
    /* TODO: patterns */
}

void pdf_setdrawbrush(void)
{
    int i;
    float lw;
    int ls;
    float *darray = NULL;
    
    ls = getlinestyle();
    lw = MAX2((float) getlinewidth(), 1.0/page_scale);
    PDF_setlinewidth(phandle, lw);

    if (ls == 0 || ls == 1) {
        PDF_setpolydash(phandle, darray, 0); /* length == 0,1 means solid line */
    } else {
        darray = (float *) malloc(dash_array_length[ls]*SIZEOF_FLOAT);
        for (i = 0; i < dash_array_length[ls]; i++) {
            darray[i] = lw*dash_array[ls][i];
        }
        PDF_setpolydash(phandle, darray, dash_array_length[ls]);
        free (darray);
    }
}

void pdf_drawpolyline(VPoint *vps, int n, int mode)
{
    int i;
    
    if (getlinestyle() == 0) {
        return;
    }
    
    pdf_setpen();
    pdf_setdrawbrush();
    
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
        PDF_setfillrule(phandle, Winding);
    } else {
        PDF_setfillrule(phandle, EvenOdd);
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
    
    pdf_setpen();
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

void pdf_fillarc(VPoint vp1, VPoint vp2, int a1, int a2)
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
    PDF_fill(phandle);
    PDF_restore(phandle);
}

/* TODO: transparent pixmaps */
void pdf_putpixmap(VPoint vp, int width, int height, char *databits, 
                             int pixmap_bpp, int bitmap_pad, int pixmap_type)
{
    byte *buf, *bp;
    long buflen;
    PDF_image image;
    int cindex;
    fRGB *frgb, *ffg, *fbg;
    int	i, k, j;
    long paddedW;

    image.width		= width;
    image.height	= height;
    image.bpc		= 8;
    image.components    = 3;
    image.colorspace    = DeviceRGB;
    
/*
 *     if (pixmap_bpp == 1) {
 *         image.components	= 1;
 *         image.colorspace	= DeviceGray;
 *     } else {
 *         image.components	= 3;
 *         image.colorspace	= DeviceRGB;
 *     }
 */

    buflen = image.width * image.height * image.components;
    
    buf = (byte *) malloc(buflen);
    if (buf == NULL) {
        errmsg("malloc failed in pdf_putpixmap()");
        return;
    }
    
    bp = buf;
    if (pixmap_bpp == 1) {
        paddedW = PAD(width, bitmap_pad);
        ffg = get_frgb(getcolor());
        fbg = get_frgb(getbgcolor());
        for (k = 0; k < image.height; k++) {
            for (j = 0; j < paddedW/bitmap_pad; j++) {
                for (i = 0; i < bitmap_pad && j*bitmap_pad + i < image.width; i++) {
                    if (bin_dump(&(databits)[k*paddedW/bitmap_pad+j], i, bitmap_pad)) {
                        *bp++ = (byte) (255 * ffg->red);
                        *bp++ = (byte) (255 * ffg->green);
                        *bp++ = (byte) (255 * ffg->blue);
                    } else {
                        *bp++ = (byte) (255 * fbg->red);
                        *bp++ = (byte) (255 * fbg->green);
                        *bp++ = (byte) (255 * fbg->blue);
                    }
                }
            }
        }
    } else {
        for (k = 0; k < image.height; k++) {
            for (j = 0; j < image.width; j++) {
                cindex = (databits)[k*image.width+j];
                frgb = get_frgb(cindex);
                *bp++ = (byte) (255 * frgb->red);
                *bp++ = (byte) (255 * frgb->green);
                *bp++ = (byte) (255 * frgb->blue);
            }
        }
    }
    
    PDF_save(phandle);

    PDF_translate(phandle, vp.x, vp.y);
    PDF_scale(phandle, 1.0/page_scale, 1.0/page_scale);
    PDF_translate(phandle, 0.0, - (float) image.height);
     
    PDF_data_source_from_buf(phandle, &image.src, buf, buflen);
    PDF_place_inline_image(phandle, &image, 0.0, 0.0, 1.0);
    
    PDF_restore(phandle);

    free(buf);
}

void pdf_puttext(VPoint start, VPoint end, double size, 
                                            CompositeString *cstring)
{
    int iglyph;
    float angle;
    float length, pdfstring_length;
    char *fontname, *encscheme;
    int pdflibenc;
    
    size /= page_scalef;
    
    angle = (float) (180.0/M_PI) * atan2(end.y - start.y, end.x - start.x);
    length = (float) hypot (end.x - start.x, end.y - start.y);
    
    pdfstring_length = 0.0;
    
    iglyph = 0;
    while (cstring[iglyph].s != NULL) {
        fontname = get_fontalias(cstring[iglyph].font);
        encscheme = get_encodingscheme(cstring[iglyph].font);
        if (strcmp(encscheme, "ISOLatin1Encoding") == 0) {
            pdflibenc = winansi;
        } else if (strcmp(encscheme, "FontSpecific") == 0) {
            pdflibenc = builtin;
        } else {
            pdflibenc = pdfdoc;
        }
        PDF_set_font(phandle, fontname, (float) size*cstring[iglyph].scale, pdflibenc);
        pdfstring_length += (float) size*cstring[iglyph].hshift;
        pdfstring_length += PDF_stringwidth(phandle, (unsigned char*) cstring[iglyph].s);
        iglyph++;
    }

    PDF_save(phandle);
    PDF_translate(phandle, (float) start.x, (float) start.y);
    PDF_rotate(phandle, angle);
    /*
     * Compensate for diffs between PDFlib & T1lib 
     * (should Y be scaled the same??)
     */
    PDF_scale(phandle, (float) size*length/pdfstring_length, (float) size);

    PDF_set_text_pos(phandle, 0.0, 0.0);
    
    pdf_setpen();
    
    iglyph = 0;
    while (cstring[iglyph].s != NULL) {
        fontname = get_fontalias(cstring[iglyph].font);
        encscheme = get_encodingscheme(cstring[iglyph].font);
        if (strcmp(encscheme, "ISOLatin1Encoding") == 0) {
            pdflibenc = winansi;
        } else if (strcmp(encscheme, "FontSpecific") == 0) {
            pdflibenc = builtin;
        } else {
            pdflibenc = pdfdoc;
        }
        PDF_set_font(phandle, fontname, (float) cstring[iglyph].scale, pdflibenc);
        if (cstring[iglyph].vshift != 0.0 || cstring[iglyph].hshift != 0.0) {
            PDF_translate(phandle, (float) cstring[iglyph].hshift,
                                   (float) cstring[iglyph].vshift);
            PDF_show(phandle, cstring[iglyph].s);
            PDF_translate(phandle, 0.0, - (float) cstring[iglyph].vshift);
        } else {
            PDF_show(phandle, cstring[iglyph].s);
        }
        
        if (cstring[iglyph].underline == TRUE) {
            /* TODO */
        }
        
        if (cstring[iglyph].overline == TRUE) {
            /* TODO */
        }
        
        iglyph++;
    }

    PDF_restore(phandle);
}

void pdf_leavegraphics(void)
{
    PDF_end_page(phandle);
    PDF_close(phandle);
}

static void pdf_error_handler(int level, const char* fmt, va_list ap)
{
    char buf[MAX_STRING_LENGTH], buf_tail[MAX_STRING_LENGTH];
    
    switch (level) {
        case PDF_INFO:
            strcpy(buf, "PDFlib note: ");
            break;

        case PDF_WARN:
            strcpy(buf, "PDFlib warning: ");
            break;

        case PDF_INTERNAL:
            strcpy(buf, "PDFlib internal error: ");
            break;

        case PDF_FATAL:
        default:
            strcpy(buf, "PDFlib fatal error: ");
            break;
    }
    vsprintf(buf_tail, fmt, ap);
    strcat(buf, buf_tail);
    
    errmsg(buf);
}

#ifndef NONE_GUI

static void update_pdf_setup_frame(void);
static void set_pdf_setup_proc(Widget w, XtPointer client_data, 
                                                        XtPointer call_data);

static Widget pdf_setup_frame;
static Widget pdf_setup_binary_item;
static Widget pdf_setup_compress_item;

void pdf_gui_setup(void)
{
    Widget pdf_setup_panel, pdf_setup_rc, fr, rc;
    
    set_wait_cursor();
    if (pdf_setup_frame == NULL) {
	pdf_setup_frame = XmCreateDialogShell(app_shell, "PDF options", NULL, 0);
	handle_close(pdf_setup_frame);
        pdf_setup_panel = XtVaCreateWidget("device_panel", xmFormWidgetClass, 
                                        pdf_setup_frame, NULL, 0);
        pdf_setup_rc = XmCreateRowColumn(pdf_setup_panel, "psetup_rc", NULL, 0);

	fr = CreateFrame(pdf_setup_rc, "PDF options");
        rc = XmCreateRowColumn(fr, "rc", NULL, 0);
	pdf_setup_binary_item = CreateToggleButton(rc, "Binary output");
	pdf_setup_compress_item = CreateToggleButton(rc, "Compression (N/I)");
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
        SetToggleButtonState(pdf_setup_binary_item, pdf_setup_binary);
    }
}

static void set_pdf_setup_proc(Widget w, XtPointer client_data, 
                                                        XtPointer call_data)
{
    int aac_mode;
    aac_mode = (int) client_data;
    
    if (aac_mode == AAC_CLOSE) {
        XtUnmanageChild(pdf_setup_frame);
        return;
    }
    
    pdf_setup_binary = GetToggleButtonState(pdf_setup_binary_item);
    
    if (aac_mode == AAC_ACCEPT) {
        XtUnmanageChild(pdf_setup_frame);
    }
}

#endif
