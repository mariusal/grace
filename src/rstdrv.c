/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1996-99 Grace Development Team
 * Copyright (c) 1991-95 Paul J Turner, Portland, OR
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
 * GRACE generic raster format driver
 */

#include <config.h>
#include <cmath.h>

#include <stdio.h>

#include "defines.h"
#include "globals.h"
#include "utils.h"
#include "draw.h"
#include "device.h"
#include "patterns.h"
#include "rstdrv.h"
#include "patchlevel.h"
#include "protos.h"

#include <gd.h>

#ifndef NONE_GUI
#  include <Xm/Xm.h>
#  include <Xm/BulletinB.h>
#  include <Xm/RowColumn.h>

#  include "motifinc.h"
#endif

static void rstImagePnm(gdImagePtr ihandle, FILE *prstream);

extern FILE *prstream;

/* Declare the image */
gdImagePtr ihandle;

static int curformat = DEFAULT_RASTER_FORMAT;

static int rst_colors[MAXCOLORS];
static int rst_drawbrush, rst_fillbrush;

static Pen rstpen;
static int rstlines, rstlinew;

static int rst_dash_array_length;

static unsigned long page_scale;

static int gif_setup_interlaced = FALSE;
static int gif_setup_transparent = FALSE;

static void rst_updatecmap(void)
{
    int i, c;
    RGB *prgb;
    int red, green, blue;
    
    for (i = 0; i < number_of_colors(); i++) {
        prgb = get_rgb(i);
        if (prgb != NULL) {
            red = prgb->red >> (GRACE_BPP - 8);
            green = prgb->green >> (GRACE_BPP - 8);
            blue = prgb->blue >> (GRACE_BPP - 8);
            if ((c = gdImageColorExact(ihandle, red, green, blue))    == -1 &&
                (c = gdImageColorAllocate(ihandle, red, green, blue)) == -1 &&
                (c = gdImageColorClosest(ihandle, red, green, blue))  == -1) {
                c = rst_colors[0];
            }
            rst_colors[i] = c;
        }
    }
}

static gdPoint VPoint2gdPoint(VPoint vp)
{
    gdPoint gdp;
    
    gdp.x = (int) rint(page_scale * vp.x);
    gdp.y = (int) rint(page_height - page_scale * vp.y);
    
    return (gdp);
}

void rst_setdrawbrush(void)
{
    static gdImagePtr brush = NULL;
    int i, j, k;
    int *tmp_dash_array;
    RGB *prgb;
    int red, green, blue, bcolor;
    int scale;
    int on, off;

    rstpen = getpen();
    rstlinew = MAX2((int) rint(getlinewidth()*page_scale), 1);
    rstlines = getlinestyle();
    
    if (rstlines == 0 || rstpen.pattern == 0) {
        /* Should never come to here */
        rst_drawbrush = gdTransparent;
        return;
    }
    
    if (rstlinew > 1) {
        if (brush != NULL) {
            gdImageDestroy(brush);
        }
        brush = gdImageCreate(rstlinew, rstlinew);

        prgb = get_rgb(rstpen.color);
        red = prgb->red >> (GRACE_BPP - 8);
        green = prgb->green >> (GRACE_BPP - 8);
        blue = prgb->blue >> (GRACE_BPP - 8);
        bcolor = gdImageColorAllocate(brush, red, green, blue);

        gdImageFilledRectangle(brush, 0, 0, rstlinew, rstlinew, bcolor);

        gdImageSetBrush(ihandle, brush);
    }

    if (rstlines > 1) {
        rst_dash_array_length = 0;
        for (i = 0; i < dash_array_length[rstlines]; i++) {
            rst_dash_array_length += dash_array[rstlines][i];
        }
    
        if (rstlinew <= 1) {
            scale = 1;
            on = rstpen.color;
            off = gdTransparent;
            rst_drawbrush = gdStyled;
        } else {
            scale = rstlinew;
            on = 1;
            off = 0;
            rst_drawbrush = gdStyledBrushed;
        }
        
        tmp_dash_array = (int *) malloc((scale*rst_dash_array_length + 1)*SIZEOF_INT);
        if (tmp_dash_array == NULL) {
            return;
        }
        
        k = 0;
        for (i = 0; i < dash_array_length[rstlines]; i++) {
            if (i % 2 == 0) {
                /* black */
                for (j = 0; j < (dash_array[rstlines][i] - 1)*scale + 1; j++) {
                    tmp_dash_array[k++] = on;
                }
            } else {
                /* white */
                for (j = 0; j < (dash_array[rstlines][i] + 1)*scale - 1; j++) {
                    tmp_dash_array[k++] = off;
                }
            }
        }
        gdImageSetStyle(ihandle, tmp_dash_array, k);
        free(tmp_dash_array);
            
    } else {
        if (rstlinew <= 1) {
            rst_drawbrush = rst_colors[rstpen.color];
        } else {
            rst_drawbrush = gdBrushed;
        }
    }
}

void rst_setfillbrush(void)
{
    static gdImagePtr brush = NULL;
    int i, j, k;
    RGB *prgb;
    int red, green, blue, fgcolor, bgcolor;
    unsigned char p;
    
    rstpen = getpen();

    if (rstpen.pattern == 0) {
        /* Should never come to here */
        rst_fillbrush = gdTransparent;
    } else if (rstpen.pattern == 1) {
        rst_fillbrush = rst_colors[rstpen.color];
    } else {
        /* TODO */
        if (brush != NULL) {
            gdImageDestroy(brush);
        }
        brush = gdImageCreate(16, 16);
        
        prgb = get_rgb(rstpen.color);
        red = prgb->red >> (GRACE_BPP - 8);
        green = prgb->green >> (GRACE_BPP - 8);
        blue = prgb->blue >> (GRACE_BPP - 8);
        fgcolor = gdImageColorAllocate(brush, red, green, blue);
        
        prgb = get_rgb(getbgcolor());
        red = prgb->red >> (GRACE_BPP - 8);
        green = prgb->green >> (GRACE_BPP - 8);
        blue = prgb->blue >> (GRACE_BPP - 8);
        bgcolor = gdImageColorAllocate(brush, red, green, blue);
        
        for (k = 0; k < 16; k++) {
            for (j = 0; j < 2; j++) {
                for (i = 0; i < 8; i++) {
                    p = pat_bits[rstpen.pattern][k*2+j];
                    if ((p >> i) & 0x01) {
                        gdImageSetPixel(brush, 8*j + i, k, fgcolor);
                    } else {
                        gdImageSetPixel(brush, 8*j + i, k, bgcolor);
                    }
                }
            }
        }
        gdImageSetTile(ihandle, brush);
        
        rst_fillbrush = gdTiled;
    }
}

static int rst_initgraphics(int format)
{
    Page_geometry pg;
    
    curformat = format;
    
    /* device-dependent routines */
    devupdatecmap = rst_updatecmap;
    
    devdrawpixel = rst_drawpixel;
    devdrawpolyline = rst_drawpolyline;
    devfillpolygon = rst_fillpolygon;
    devdrawarc = rst_drawarc;
    devfillarc = rst_fillarc;
    devputpixmap = rst_putpixmap;
    
    devleavegraphics = rst_leavegraphics;
    
    pg = get_page_geometry();
    
    page_scale = MIN2(pg.height,pg.width);

    /* Allocate the image */
    ihandle = gdImageCreate(pg.width, pg.height);
    if (ihandle == NULL) {
        return GRACE_EXIT_FAILURE;
    }
    
    rst_updatecmap();
    
    return GRACE_EXIT_SUCCESS;
}

void rst_drawpixel(VPoint vp)
{
    gdPoint gdp;
    
    gdp = VPoint2gdPoint(vp);
    gdImageSetPixel(ihandle, gdp.x, gdp.y, rst_colors[getcolor()]);
}

void rst_drawpolyline(VPoint *vps, int n, int mode)
{
    int i;
    gdPointPtr gdps;
    
    gdps = (gdPointPtr) malloc(n*sizeof(gdPoint));
    if (gdps == NULL) {
        return;
    }
    
    for (i = 0; i < n; i++) {
        gdps[i] = VPoint2gdPoint(vps[i]);
    }
    
    rst_setdrawbrush();
    
    if (mode == POLYLINE_CLOSED) {
        gdImagePolygon(ihandle, gdps, n, rst_drawbrush);
    } else {
         for (i = 0; i < n - 1; i++) {
             gdImageLine(ihandle, gdps[i].x,     gdps[i].y, 
                                  gdps[i + 1].x, gdps[i + 1].y, 
                                  rst_drawbrush);
         }
    }
    
    free(gdps);
}

void rst_fillpolygon(VPoint *vps, int nc)
{
    int i;
    gdPointPtr gdps;
    
    gdps = (gdPointPtr) malloc(nc*sizeof(gdPoint));
    if (gdps == NULL) {
        return;
    }
    
    for (i = 0; i < nc; i++) {
        gdps[i] = VPoint2gdPoint(vps[i]);
    }
    
    rst_setfillbrush();
    gdImageFilledPolygon(ihandle, gdps, nc, rst_fillbrush);
    
    free(gdps);
}

void rst_drawarc(VPoint vp1, VPoint vp2, int a1, int a2)
{
    gdPoint gdp1, gdp2, gdc;
    int w, h;
    
    gdp1 = VPoint2gdPoint(vp1);
    gdp2 = VPoint2gdPoint(vp2);
    gdc.x = (gdp1.x + gdp2.x)/2;
    gdc.y = (gdp1.y + gdp2.y)/2;
    w = (gdp2.x - gdp1.x);
    h = (gdp2.y - gdp1.y);
    
    rst_setdrawbrush();
    
    gdImageArc(ihandle, gdc.x, gdc.y, w, h, a1, a2, rst_drawbrush);
}

void rst_fillarc(VPoint vp1, VPoint vp2, int a1, int a2)
{
    gdPoint gdp1, gdp2, gdc;
    int w, h;
    
    int border;
    
    gdp1 = VPoint2gdPoint(vp1);
    gdp2 = VPoint2gdPoint(vp2);
    gdc.x = (gdp1.x + gdp2.x)/2;
    gdc.y = (gdp1.y + gdp2.y)/2;
    w = (gdp2.x - gdp1.x);
    h = (gdp2.y - gdp1.y);
    
    /* TODO: all this is a dirty trick... */
    border = rst_colors[rstpen.color];
    gdImageArc(ihandle, gdc.x, gdc.y, w, h, a1, a2, border);
    
    rst_setfillbrush();
    gdImageFillToBorder(ihandle, gdc.x + w/4, gdc.y, border, rst_fillbrush);
    gdImageFillToBorder(ihandle, gdc.x, gdc.y + h/4, border, rst_fillbrush);
    gdImageFillToBorder(ihandle, gdc.x - w/4, gdc.y, border, rst_fillbrush);
    gdImageFillToBorder(ihandle, gdc.x, gdc.y - h/4, border, rst_fillbrush);
}

void rst_putpixmap(VPoint vp, int width, int height, 
     char *databits, int pixmap_bpp, int bitmap_pad, int pixmap_type)
{
    int cindex, bg;
    int color, bgcolor;
    
    int	i, k, j;
    long paddedW;
    
    gdPoint gdp;
    int x, y;
    
    bg = getbgcolor();
    bgcolor = rst_colors[bg];
    
    gdp = VPoint2gdPoint(vp);
    
    y = gdp.y;
    if (pixmap_bpp == 1) {
        color = rstpen.color;
        paddedW = PAD(width, bitmap_pad);
        for (k = 0; k < height; k++) {
            x = gdp.x;
            y++;
            for (j = 0; j < paddedW/bitmap_pad; j++) {
                for (i = 0; i < bitmap_pad && j*bitmap_pad + i < width; i++) {
                    x++;
                    if (bin_dump(&(databits)[k*paddedW/bitmap_pad+j], i, bitmap_pad)) {
                        gdImageSetPixel(ihandle, x, y, color);
                    } else {
                        if (pixmap_type == PIXMAP_OPAQUE) {
                            gdImageSetPixel(ihandle, x, y, bgcolor);
                        }
                    }
                }
            }
        }
    } else {
        for (k = 0; k < height; k++) {
            x = gdp.x;
            y++;
            for (j = 0; j < width; j++) {
                x++;
                cindex = (databits)[k*width+j];
                if (cindex != bg || pixmap_type == PIXMAP_OPAQUE) {
                    color = rst_colors[cindex];
                    gdImageSetPixel(ihandle, x, y, color);
                }
            }
        }
    }
}
     
void rst_leavegraphics(void)
{
    /* Output the image to the disk file. */
    switch (curformat) {
    case RST_FORMAT_GD:
        gdImageGd(ihandle, prstream);
        break;   
    case RST_FORMAT_GIF:
        if (gif_setup_transparent == TRUE) {
            gdImageColorTransparent(ihandle, rst_colors[getbgcolor()]);
        }
        gdImageInterlace(ihandle, gif_setup_interlaced);
        gdImageGif(ihandle, prstream);
        break;
    case RST_FORMAT_PNM:
        rstImagePnm(ihandle, prstream);
        break;   
    default:
        errmsg("Invalid raster format");  
        break;
    }
    
    /* Destroy the image in memory. */
    gdImageDestroy(ihandle);
}

int gifinitgraphics(void)
{
    int result;
    
    result = rst_initgraphics(RST_FORMAT_GIF);
    
    if (result == GRACE_EXIT_SUCCESS) {
        curformat = RST_FORMAT_GIF;
    }
    
    return (result);
}

int gdinitgraphics(void)
{
    int result;
    
    result = rst_initgraphics(RST_FORMAT_GD);
    
    if (result == GRACE_EXIT_SUCCESS) {
        curformat = RST_FORMAT_GD;
    }
    
    return (result);
}

int pnminitgraphics(void)
{
    int result;
    
    result = rst_initgraphics(RST_FORMAT_PNM);
    
    if (result == GRACE_EXIT_SUCCESS) {
        curformat = RST_FORMAT_PNM;
    }
    
    return (result);
}

static int pnm_setup_format = DEFAULT_PNM_FORMAT;
static int pnm_setup_rawbits = TRUE;

static void rstImagePnm(gdImagePtr ihandle, FILE *prstream)
{
    int w, h;
    int i, j, k;
    int c;
    int r, g, b;
    unsigned char y, pbm_buf;
    
    if (pnm_setup_rawbits == TRUE) {
        switch (pnm_setup_format) {
        case PNM_FORMAT_PBM:
            fprintf(prstream, "P4\n");
            break;
        case PNM_FORMAT_PGM:
            fprintf(prstream, "P5\n");
            break;
        case PNM_FORMAT_PPM:
            fprintf(prstream, "P6\n");
            break;
        }
    } else {
        switch (pnm_setup_format) {
        case PNM_FORMAT_PBM:
            fprintf(prstream, "P1\n");
            break;
        case PNM_FORMAT_PGM:
            fprintf(prstream, "P2\n");
            break;
        case PNM_FORMAT_PPM:
            fprintf(prstream, "P3\n");
            break;
        }
    }
    
    fprintf(prstream, "#Creator: Grace v%d.%d.%d %s\n",
                                MAJOR_REV, MINOR_REV, PATCHLEVEL, BETA_VER);
    
    w = gdImageSX(ihandle);
    h = gdImageSY(ihandle);
    fprintf(prstream, "%d %d\n", w, h);
    
    if (pnm_setup_format != PNM_FORMAT_PBM) {
        fprintf(prstream, "255\n");
    }
    
    k = 0;
    pbm_buf = 0;
    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            c = gdImageGetPixel(ihandle, j, i);
            r = gdImageRed(ihandle, c);
            g = gdImageGreen(ihandle, c);
            b = gdImageBlue(ihandle, c);
            if (pnm_setup_rawbits == TRUE) {
                switch (pnm_setup_format) {
                case PNM_FORMAT_PBM:
                    y = (r == 255 &&  g == 255 && b == 255 ? 0x00:0x01);
                    pbm_buf |= (y << (7 - k));
                    k++;
                    /* completed byte or padding line */
                    if (k == 8 || j == w - 1) {
                        fwrite(&pbm_buf, 1, 1, prstream);
                        k = 0;
                        pbm_buf = 0;
                    }
                    break;
                case PNM_FORMAT_PGM:
                    y = (int) (0.299*r + 0.587*g + 0.114*b);
                    fwrite(&y, 1, 1, prstream);
                    break;
                case PNM_FORMAT_PPM:
                    fwrite(&r, 1, 1, prstream);
                    fwrite(&g, 1, 1, prstream);
                    fwrite(&b, 1, 1, prstream);
                    break;
                }
            } else {
                switch (pnm_setup_format) {
                case PNM_FORMAT_PBM:
                    y = (r == 255 &&  g == 255 && b == 255 ? 0:1);
                    fprintf(prstream, "%1d\n", y);
                    break;
                case PNM_FORMAT_PGM:
                    y = (int) (0.299*r + 0.587*g + 0.114*b);
                    fprintf(prstream, "%3d\n", y);
                    break;
                case PNM_FORMAT_PPM:
                    fprintf(prstream, "%3d %3d %3d\n", r, g, b);
                    break;
                }
            }
        }
    }
}


#ifndef NONE_GUI

static void update_gif_setup_frame(void);
static void set_gif_setup_proc(Widget w, XtPointer client_data, 
                                                        XtPointer call_data);

static Widget gif_setup_frame;
static Widget gif_setup_interlaced_item;
static Widget gif_setup_transparent_item;

static void update_pnm_setup_frame(void);
static void set_pnm_setup_proc(Widget w, XtPointer client_data, 
                                                        XtPointer call_data);
static Widget pnm_setup_frame;
static Widget pnm_setup_rawbits_item;
static Widget *pnm_setup_format_item;

void gif_gui_setup(void)
{
    Widget gif_setup_panel, gif_setup_rc, fr, rc;
    
    set_wait_cursor();
    if (gif_setup_frame == NULL) {
	gif_setup_frame = XmCreateDialogShell(app_shell, "GIF options", NULL, 0);
	handle_close(gif_setup_frame);
        gif_setup_panel = XtVaCreateWidget("device_panel", xmFormWidgetClass, 
                                        gif_setup_frame, NULL, 0);
        gif_setup_rc = XmCreateRowColumn(gif_setup_panel, "psetup_rc", NULL, 0);

	fr = CreateFrame(gif_setup_rc, "GIF options");
        rc = XmCreateRowColumn(fr, "rc", NULL, 0);
	gif_setup_interlaced_item = CreateToggleButton(rc, "Interlaced");
	gif_setup_transparent_item = CreateToggleButton(rc, "Transparent");
	XtManageChild(rc);

	CreateSeparator(gif_setup_rc);

	CreateAACButtons(gif_setup_rc, gif_setup_panel, set_gif_setup_proc);
        
	XtManageChild(gif_setup_rc);
	XtManageChild(gif_setup_panel);
    }
    XtRaise(gif_setup_frame);
    update_gif_setup_frame();
    unset_wait_cursor();
}

static void update_gif_setup_frame(void)
{
    if (gif_setup_frame) {
        SetToggleButtonState(gif_setup_interlaced_item, gif_setup_interlaced);
        SetToggleButtonState(gif_setup_transparent_item, gif_setup_transparent);
    }
}

static void set_gif_setup_proc(Widget w, XtPointer client_data, 
                                                        XtPointer call_data)
{
    int aac_mode;
    aac_mode = (int) client_data;
    
    if (aac_mode == AAC_CLOSE) {
        XtUnmanageChild(gif_setup_frame);
        return;
    }
    
    gif_setup_interlaced = GetToggleButtonState(gif_setup_interlaced_item);
    gif_setup_transparent = GetToggleButtonState(gif_setup_transparent_item);
    
    if (aac_mode == AAC_ACCEPT) {
        XtUnmanageChild(gif_setup_frame);
    }
}

void pnm_gui_setup(void)
{
    Widget pnm_setup_panel, pnm_setup_rc, fr, rc;
    
    set_wait_cursor();
    if (pnm_setup_frame == NULL) {
	pnm_setup_frame = XmCreateDialogShell(app_shell, "PNM options", NULL, 0);
	handle_close(pnm_setup_frame);
        pnm_setup_panel = XtVaCreateWidget("device_panel", xmFormWidgetClass, 
                                        pnm_setup_frame, NULL, 0);
        pnm_setup_rc = XmCreateRowColumn(pnm_setup_panel, "psetup_rc", NULL, 0);

	fr = CreateFrame(pnm_setup_rc, "PNM options");
        rc = XmCreateRowColumn(fr, "rc", NULL, 0);
	pnm_setup_format_item = CreatePanelChoice(rc, "Format: ",
					 4,
					 "1-bit mono (PBM)",
					 "8-bit grayscale (PGM)",
					 "8-bit color (PPM)",
                                         0, 0);
	pnm_setup_rawbits_item = CreateToggleButton(rc, "\"Rawbits\"");
	XtManageChild(rc);

	CreateSeparator(pnm_setup_rc);

	CreateAACButtons(pnm_setup_rc, pnm_setup_panel, set_pnm_setup_proc);
        
	XtManageChild(pnm_setup_rc);
	XtManageChild(pnm_setup_panel);
    }
    XtRaise(pnm_setup_frame);
    update_pnm_setup_frame();
    unset_wait_cursor();
}

static void update_pnm_setup_frame(void)
{
    if (pnm_setup_frame) {
        SetChoice(pnm_setup_format_item, pnm_setup_format);
        SetToggleButtonState(pnm_setup_rawbits_item, pnm_setup_rawbits);
    }
}

static void set_pnm_setup_proc(Widget w, XtPointer client_data, 
                                                        XtPointer call_data)
{
    int aac_mode;
    aac_mode = (int) client_data;
    
    if (aac_mode == AAC_CLOSE) {
        XtUnmanageChild(pnm_setup_frame);
        return;
    }
    
    pnm_setup_format = GetChoice(pnm_setup_format_item);
    pnm_setup_rawbits = GetToggleButtonState(pnm_setup_rawbits_item);
    
    if (aac_mode == AAC_ACCEPT) {
        XtUnmanageChild(pnm_setup_frame);
    }
}

#endif
