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
 *
 * driver for X11 for Grace
 *
 */

#include <config.h>

#include <stdlib.h>

#include <X11/Xlib.h>

#define CANVAS_BACKEND_API
#include "grace/canvas.h"

#include "devlist.h"

#include "globals.h"
#include "protos.h"

typedef struct {
    RGB rgb;
    unsigned long pixel;
    int allocated;
} x11color;

typedef struct {
    Screen *screen;
    Pixmap pixmap;
    
    int pixel_size;
    unsigned int height, width, page_scale;
        
    int monomode;

    int color;
    int bgcolor;
    int patno;
    int linewidth;
    int linestyle;
    int fillrule;
    int arcfillmode;
    int linecap;
    int linejoin;

    x11color colors[MAXCOLORS];
} X11_data;

static X11_data *init_x11_data(void)
{
    X11_data *data;

    /* we need to perform the allocations */
    data = xmalloc(sizeof(X11_data));
    if (data) {
        memset(data, 0, sizeof(X11_data));
    }
    
    return data;
}

static void VPoint2XPoint(const X11_data *x11data, const VPoint *vp, XPoint *xp)
{
    xp->x = (short) rint(x11data->page_scale*vp->x);
    xp->y = (short) rint(x11data->height - x11data->page_scale*vp->y);
}

static void x11_initcmap(const Canvas *canvas, X11_data *x11data)
{
    unsigned int i;
    RGB rgb;
    long pixel;
    
    for (i = 0; i < number_of_colors(canvas); i++) {
        x11color *xc = &x11data->colors[i];
        /* even in mono, b&w must be allocated */
        if (x11data->monomode == FALSE || i < 2) {
            if (get_rgb(canvas, i, &rgb) == RETURN_SUCCESS) {
                if (!xc->allocated || !compare_rgb(&rgb, &xc->rgb)) {
                    pixel = x11_allocate_color(grace->gui, &rgb);
                    if (pixel >= 0) {
                        xc->pixel = pixel;
                    } else {
                        xc->pixel = BlackPixelOfScreen(x11data->screen);
                    }
                    xc->rgb = rgb;
                    xc->allocated = TRUE;
                }
            }
        } else {
            xc->pixel = BlackPixelOfScreen(x11data->screen);
        }
    }
}

static void x11_updatecmap(const Canvas *canvas, void *data)
{
    X11_data *x11data = (X11_data *) data;
    /* TODO: replace!!! */
    x11_initcmap(canvas, x11data);
}

static int x11_initgraphics(const Canvas *canvas, void *data,
    const CanvasStats *cstats)
{
    X11_data *x11data = (X11_data *) data;
    X11stream *xstream;

    Page_geometry *pg = get_page_geometry(canvas);

    x11data->width = pg->width;
    x11data->height = pg->height;
    x11data->page_scale = MIN2(pg->width, pg->height);

    xstream = (X11stream *) canvas_get_prstream(canvas);
    
    x11data->screen = xstream->screen;
    x11data->pixmap = xstream->pixmap;
    
    x11data->pixel_size = x11_get_pixelsize(grace->gui);

    if (x11data->pixel_size == 0) {
        x11data->monomode = TRUE;
    }

    /* init settings specific to X11 driver */    
    x11data->color       = BAD_COLOR;
    x11data->bgcolor     = BAD_COLOR;
    x11data->patno       = -1;
    x11data->linewidth   = -1;
    x11data->linestyle   = -1;
    x11data->fillrule    = -1;
    x11data->arcfillmode = -1;
    x11data->linecap     = -1;
    x11data->linejoin    = -1;

    x11_initcmap(canvas, x11data);
    
    return RETURN_SUCCESS;
}


static void x11_setpen(const Canvas *canvas, X11_data *x11data)
{
    int fg, bg, p;
    Pen pen;
    
    bg = getbgcolor(canvas);
    getpen(canvas, &pen);
    fg = pen.color;
    p = pen.pattern;
    
    if ((fg == x11data->color) && (bg == x11data->bgcolor) && (p == x11data->patno)) {
        return;
    }
        
    x11data->color = fg;
    x11data->bgcolor = bg;
    x11data->patno = p;
    
    if (p == 0) { /* TODO: transparency !!!*/
        return;
    } else if (p == 1) {
        /* To make X faster */
        XSetForeground(DisplayOfScreen(x11data->screen),
            DefaultGCOfScreen(x11data->screen), x11data->colors[fg].pixel);
        XSetBackground(DisplayOfScreen(x11data->screen),
            DefaultGCOfScreen(x11data->screen), x11data->colors[bg].pixel);
        XSetFillStyle(DisplayOfScreen(x11data->screen),
            DefaultGCOfScreen(x11data->screen), FillSolid);
    } else {
        Pattern *pat = canvas_get_pattern(canvas, p);
        Pixmap ptmp = XCreatePixmapFromBitmapData(DisplayOfScreen(x11data->screen), RootWindowOfScreen(x11data->screen),
            (char *) pat->bits, pat->width, pat->height,
            x11data->colors[fg].pixel, x11data->colors[bg].pixel,
                PlanesOfScreen(x11data->screen));
/*
 *      XSetFillStyle(DisplayOfScreen(x11data->screen), DefaultGCOfScreen(x11data->screen), FillStippled);
 *      XSetStipple(DisplayOfScreen(x11data->screen), DefaultGCOfScreen(x11data->screen), curstipple);
 */
        XSetFillStyle(DisplayOfScreen(x11data->screen), DefaultGCOfScreen(x11data->screen), FillTiled);
        XSetTile(DisplayOfScreen(x11data->screen), DefaultGCOfScreen(x11data->screen), ptmp);
        
        XFreePixmap(DisplayOfScreen(x11data->screen), ptmp);
    }
}

static void x11_setdrawbrush(const Canvas *canvas, X11_data *x11data)
{
    unsigned int iw;
    int style;
    int lc, lj;
    int i, scale, darr_len;
    char *xdarr;

    x11_setpen(canvas, x11data);
    
    iw = (unsigned int) rint(getlinewidth(canvas)*x11data->page_scale);
    if (iw == 1) {
        iw = 0;
    }
    style = getlinestyle(canvas);
    lc = getlinecap(canvas);
    lj = getlinejoin(canvas);
    
    switch (lc) {
    case LINECAP_BUTT:
        lc = CapButt;
        break;
    case LINECAP_ROUND:
        lc = CapRound;
        break;
    case LINECAP_PROJ:
        lc = CapProjecting;
        break;
    }

    switch (lj) {
    case LINEJOIN_MITER:
        lj = JoinMiter;
        break;
    case LINEJOIN_ROUND:
        lj = JoinRound;
        break;
    case LINEJOIN_BEVEL:
        lj = JoinBevel;
        break;
    }
    
    if (iw != x11data->linewidth || style != x11data->linestyle ||
        lc != x11data->linecap   || lj    != x11data->linejoin) {
        if (style > 1) {
            LineStyle *linestyle = canvas_get_linestyle(canvas, style);
            darr_len = linestyle->length;
            xdarr = xmalloc(darr_len*SIZEOF_CHAR);
            if (xdarr == NULL) {
                return;
            }
            scale = MAX2(1, iw);
            for (i = 0; i < darr_len; i++) {
                xdarr[i] = scale*linestyle->array[i];
            }
            XSetLineAttributes(DisplayOfScreen(x11data->screen), DefaultGCOfScreen(x11data->screen), iw, LineOnOffDash, lc, lj);
            XSetDashes(DisplayOfScreen(x11data->screen), DefaultGCOfScreen(x11data->screen), 0, xdarr, darr_len);
            xfree(xdarr);
        } else if (style == 1) {
            XSetLineAttributes(DisplayOfScreen(x11data->screen), DefaultGCOfScreen(x11data->screen), iw, LineSolid, lc, lj);
        }
 
        x11data->linestyle = style;
        x11data->linewidth = iw;
        x11data->linecap   = lc;
        x11data->linejoin  = lj;
    }

    return;
}

static void x11_drawpixel(const Canvas *canvas, void *data,
    const VPoint *vp)
{
    X11_data *x11data = (X11_data *) data;
    XPoint xp;
    
    VPoint2XPoint(x11data, vp, &xp);
    x11_setpen(canvas, x11data);
    XDrawPoint(DisplayOfScreen(x11data->screen), x11data->pixmap, DefaultGCOfScreen(x11data->screen), xp.x, xp.y);
}

static void x11_drawpolyline(const Canvas *canvas, void *data,
    const VPoint *vps, int n, int mode)
{
    X11_data *x11data = (X11_data *) data;
    int i, xn = n;
    XPoint *p;
    
    if (mode == POLYLINE_CLOSED) {
        xn++;
    }
    
    p = xmalloc(xn*sizeof(XPoint));
    if (p == NULL) {
        return;
    }
    
    for (i = 0; i < n; i++) {
        VPoint2XPoint(x11data, &vps[i], &p[i]);
    }
    if (mode == POLYLINE_CLOSED) {
        p[n] = p[0];
    }
    
    x11_setdrawbrush(canvas, x11data);
    
    XDrawLines(DisplayOfScreen(x11data->screen), x11data->pixmap, DefaultGCOfScreen(x11data->screen), p, xn, CoordModeOrigin);
    
    xfree(p);
}


static void x11_fillpolygon(const Canvas *canvas, void *data,
    const VPoint *vps, int nc)
{
    X11_data *x11data = (X11_data *) data;
    int i;
    XPoint *p;
    
    p = (XPoint *) xmalloc(nc*sizeof(XPoint));
    if (p == NULL) {
        return;
    }
    
    for (i = 0; i < nc; i++) {
        VPoint2XPoint(x11data, &vps[i], &p[i]);
    }
    
    x11_setpen(canvas, x11data);

    if (getfillrule(canvas) != x11data->fillrule) {
        x11data->fillrule = getfillrule(canvas);
        if (getfillrule(canvas) == FILLRULE_WINDING) {
            XSetFillRule(DisplayOfScreen(x11data->screen), DefaultGCOfScreen(x11data->screen), WindingRule);
        } else {
            XSetFillRule(DisplayOfScreen(x11data->screen), DefaultGCOfScreen(x11data->screen), EvenOddRule);
        }
    }

    XFillPolygon(DisplayOfScreen(x11data->screen), x11data->pixmap, DefaultGCOfScreen(x11data->screen), p, nc, Complex, CoordModeOrigin);
    
    xfree(p);
}

/*
 *  x11_drawarc
 */
static void x11_drawarc(const Canvas *canvas, void *data,
    const VPoint *vp1, const VPoint *vp2, double a1, double a2)
{
    X11_data *x11data = (X11_data *) data;
    XPoint xp;
    short x1, y1, x2, y2;
    
    VPoint2XPoint(x11data, vp1, &xp);
    x1 = xp.x;
    y2 = xp.y;
    VPoint2XPoint(x11data, vp2, &xp);
    x2 = xp.x;
    y1 = xp.y;

    x11_setdrawbrush(canvas, x11data);
    
    if (x1 != x2 || y1 != y2) {
        XDrawArc(DisplayOfScreen(x11data->screen), x11data->pixmap, DefaultGCOfScreen(x11data->screen), MIN2(x1, x2), MIN2(y1, y2),
              abs(x2 - x1), abs(y2 - y1), (int) rint(64*a1), (int) rint(64*a2));
    } else { /* zero radius */
        XDrawPoint(DisplayOfScreen(x11data->screen), x11data->pixmap, DefaultGCOfScreen(x11data->screen), x1, y1);
    }
}

/*
 *  x11_fillarc
 */
static void x11_fillarc(const Canvas *canvas, void *data,
    const VPoint *vp1, const VPoint *vp2, double a1, double a2, int mode)
{
    X11_data *x11data = (X11_data *) data;
    XPoint xp;
    short x1, y1, x2, y2;
    
    VPoint2XPoint(x11data, vp1, &xp);
    x1 = xp.x;
    y2 = xp.y;
    VPoint2XPoint(x11data, vp2, &xp);
    x2 = xp.x;
    y1 = xp.y;
    
    x11_setpen(canvas, x11data);
    if (x1 != x2 || y1 != y2) {
        if (x11data->arcfillmode != mode) {
            x11data->arcfillmode = mode;
            if (mode == ARCFILL_CHORD) {
                XSetArcMode(DisplayOfScreen(x11data->screen), DefaultGCOfScreen(x11data->screen), ArcChord);
            } else {
                XSetArcMode(DisplayOfScreen(x11data->screen), DefaultGCOfScreen(x11data->screen), ArcPieSlice);
            }
        }
        XFillArc(DisplayOfScreen(x11data->screen), x11data->pixmap, DefaultGCOfScreen(x11data->screen), MIN2(x1, x2), MIN2(y1, y2),
           abs(x2 - x1), abs(y2 - y1), (int) rint(64*a1), (int) rint(64*a2));
    } else { /* zero radius */
        XDrawPoint(DisplayOfScreen(x11data->screen), x11data->pixmap, DefaultGCOfScreen(x11data->screen), x1, y1);
    }
}


static void x11_putpixmap(const Canvas *canvas, void *data,
    const VPoint *vp, const CPixmap *pm)
{
    X11_data *x11data = (X11_data *) data;
    int j, k, l;
    
    XPoint xp;

    XImage *ximage;
 
    Pixmap clipmask = 0;
    char *pixmap_ptr;
    char *clipmask_ptr = NULL;
    
    int line_off;

    int cindex, fg, bg;
    
    VPoint2XPoint(x11data, vp, &xp);
      
    if (pm->bpp != 1) {
        if (x11data->monomode == TRUE) {
            /* TODO: dither pixmaps on mono displays */
            return;
        }
        pixmap_ptr = xcalloc(PADBITS(pm->width, 8) * pm->height, x11data->pixel_size);
        if (pixmap_ptr == NULL) {
            errmsg("xmalloc failed in x11_putpixmap()");
            return;
        }
 
        /* re-index pixmap */
        for (k = 0; k < pm->height; k++) {
            for (j = 0; j < pm->width; j++) {
                cindex = (unsigned char) (pm->bits)[k*pm->width+j];
                for (l = 0; l < x11data->pixel_size; l++) {
                    pixmap_ptr[x11data->pixel_size*(k*pm->width+j) + l] =
                        (char) (x11data->colors[cindex].pixel >> (8*l));
                }
            }
        }

        ximage=XCreateImage(DisplayOfScreen(x11data->screen),
            DefaultVisualOfScreen(x11data->screen),
            PlanesOfScreen(x11data->screen), ZPixmap, 0,
            pixmap_ptr, pm->width, pm->height,
            pm->pad, 0);

        if (pm->type == PIXMAP_TRANSPARENT) {
            clipmask_ptr = xcalloc((PADBITS(pm->width, 8)>>3)
                                              * pm->height, SIZEOF_CHAR);
            if (clipmask_ptr == NULL) {
                errmsg("xmalloc failed in x11_putpixmap()");
                return;
            } else {
                /* Note: We pad the clipmask always to byte boundary */
                bg = getbgcolor(canvas);
                for (k = 0; k < pm->height; k++) {
                    line_off = k*(PADBITS(pm->width, 8) >> 3);
                    for (j = 0; j < pm->width; j++) {
                        cindex = (unsigned char) (pm->bits)[k*pm->width+j];
                        if (cindex != bg) {
                            clipmask_ptr[line_off+(j>>3)] |= (0x01 << (j%8));
                        }
                    }
                }
        
                clipmask = XCreateBitmapFromData(DisplayOfScreen(x11data->screen),
                    RootWindowOfScreen(x11data->screen), clipmask_ptr, pm->width, pm->height);
                xfree(clipmask_ptr);
            }
        }
    } else {
        pixmap_ptr = xcalloc((PADBITS(pm->width, pm->pad)>>3) * pm->height,
                                                        sizeof(unsigned char));
        if (pixmap_ptr == NULL) {
            errmsg("xmalloc failed in x11_putpixmap()");
            return;
        }
        memcpy(pixmap_ptr, pm->bits, ((PADBITS(pm->width, pm->pad)>>3) * pm->height));

        fg = getcolor(canvas);
        if (fg != x11data->color) {
            XSetForeground(DisplayOfScreen(x11data->screen),
                DefaultGCOfScreen(x11data->screen), x11data->colors[fg].pixel);
            x11data->color = fg;
        }
        ximage = XCreateImage(DisplayOfScreen(x11data->screen),
            DefaultVisualOfScreen(x11data->screen),
            1, XYBitmap, 0, pixmap_ptr, pm->width, pm->height,
            pm->pad, 0);
        if (pm->type == PIXMAP_TRANSPARENT) {
            clipmask = XCreateBitmapFromData(DisplayOfScreen(x11data->screen),
                RootWindowOfScreen(x11data->screen), pixmap_ptr,
                PADBITS(pm->width, pm->pad), pm->height);
        }
    }

    if (pm->type == PIXMAP_TRANSPARENT) {
        XSetClipMask(DisplayOfScreen(x11data->screen), DefaultGCOfScreen(x11data->screen), clipmask);
        XSetClipOrigin(DisplayOfScreen(x11data->screen), DefaultGCOfScreen(x11data->screen), xp.x, xp.y);
    }
        
    /* Force bit and byte order */
    ximage->bitmap_bit_order = LSBFirst;
    ximage->byte_order       = LSBFirst;
    
    XPutImage(DisplayOfScreen(x11data->screen), x11data->pixmap,
        DefaultGCOfScreen(x11data->screen), ximage, 0, 0, xp.x, xp.y, pm->width, pm->height);
    
    /* XDestroyImage free's the image data - which is VERY wrong since we
       allocated (and hence, want to free) it ourselves. So, the trick is
       to set the image data to NULL to avoid the double free() */
    xfree(pixmap_ptr);
    ximage->data = NULL;
    XDestroyImage(ximage);
     
    if (pm->type == PIXMAP_TRANSPARENT) {
        XFreePixmap(DisplayOfScreen(x11data->screen), clipmask);
        clipmask = 0;
        XSetClipMask(DisplayOfScreen(x11data->screen), DefaultGCOfScreen(x11data->screen), None);
        XSetClipOrigin(DisplayOfScreen(x11data->screen), DefaultGCOfScreen(x11data->screen), 0, 0);
    }    
}

static void x11_leavegraphics(const Canvas *canvas, void *data, 
    const CanvasStats *cstats)
{
}

int register_x11_drv(Canvas *canvas)
{
    Device_entry *d;
    X11_data *data;

    data = init_x11_data();
    if (!data) {
        return -1;
    }
    
    d = device_new("X11", DEVICE_TERM, FALSE, data, xfree);
    if (!d) {
        return -1;
    }
    
    device_set_procs(d,
        x11_initgraphics,
        x11_leavegraphics,
        NULL,
        NULL,
        x11_updatecmap,
        x11_drawpixel,
        x11_drawpolyline,
        x11_fillpolygon,
        x11_drawarc,
        x11_fillarc,
        x11_putpixmap,
        NULL);

    return register_device(canvas, d);
}
