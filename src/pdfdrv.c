/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-95 Paul J Turner, Portland, OR
 * Copyright (c) 1996-99 Grace Development Team
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
#include "utils.h"
#include "draw.h"
#include "device.h"
#include "devlist.h"
#include "patterns.h"
#include "pdfdrv.h"

#include "protos.h"

#include <pdflib.h>

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

extern FILE *prstream;

static PDF *phandle;

static Device_entry dev_pdf = {DEVICE_FILE,
          "PDF",
          pdfinitgraphics,
          NULL,
          NULL,
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
    char *s;
   
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
        return GRACE_EXIT_FAILURE;
    }
    if (PDF_open_fp(phandle, prstream) == -1) {
        return GRACE_EXIT_FAILURE;
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

    return GRACE_EXIT_SUCCESS;
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

    PDF_place_image(phandle,
        image, vp.x, vp.y - height*pixel_size, pixel_size);
    PDF_close_image(phandle, image);
    
    xfree(buf);
}

void pdf_puttext(VPoint start, VPoint end, double size, 
                                            CompositeString *cstring)
{
    int iglyph;
    int font;
    float angle;
    float vshift, hshift, fsize;
    float length, pdfstring_length;
    
    pdf_setpen();
    
    size /= page_scale;
    
    angle = (float) (180.0/M_PI) * atan2(end.y - start.y, end.x - start.x);
    length = (float) hypot (end.x - start.x, end.y - start.y);
    
    pdfstring_length = 0.0;
    
    iglyph = 0;
    while (cstring[iglyph].s != NULL) {
        hshift = (float) size*cstring[iglyph].hshift;
        fsize  = (float) size*cstring[iglyph].scale;
        font   = cstring[iglyph].font;
        if (pdf_font_ids[font] < 0) {
            char buf[GR_MAXPATHLEN];
            char *fontname, *encscheme;
            char *pdflibenc;
            fontname = get_fontalias(font);
            sprintf(buf, "%s=%s/fonts/type1/%s",
                fontname, get_grace_home(), get_afmfilename(font));
            PDF_set_parameter(phandle, "FontAFM", buf);
            sprintf(buf, "%s=%s/fonts/type1/%s",
                fontname, get_grace_home(), get_fontfilename(font));
            PDF_set_parameter(phandle, "FontOutline", buf);

            encscheme = get_encodingscheme(font);
            if (strcmp(encscheme, "ISOLatin1Encoding") == 0) {
                pdflibenc = "default";
            } else if (strcmp(encscheme, "FontSpecific") == 0) {
                pdflibenc = "builtin";
            } else {
                pdflibenc = "pdfdoc";
            }
            pdf_font_ids[font] = PDF_findfont(phandle, fontname, pdflibenc, 1);
        } 
        PDF_setfont(phandle, pdf_font_ids[font], fsize);
        pdfstring_length += hshift;
        pdfstring_length += PDF_stringwidth(phandle,
            cstring[iglyph].s, pdf_font_ids[font], fsize);
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
    
    iglyph = 0;
    while (cstring[iglyph].s != NULL) {
        vshift = (float) cstring[iglyph].vshift;
        hshift = (float) cstring[iglyph].hshift;
        fsize  = (float) cstring[iglyph].scale;
        font   = cstring[iglyph].font;
        PDF_setfont(phandle, pdf_font_ids[font], fsize);

        if (hshift != 0.0 || vshift != 0.0) {
            PDF_set_text_rise(phandle, vshift);
            PDF_show(phandle, cstring[iglyph].s);
            PDF_set_text_rise(phandle, 0.0);
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
