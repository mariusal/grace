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

#include "globals.h"
#include "devlist.h"

#include "protos.h"

typedef struct {
    X11Stuff *xstuff;
    
    Visual *visual;
    int pixel_size;
    
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

    unsigned long pixels[MAXCOLORS];
} X11_data;

static X11_data *init_x11_data(void)
{
    X11Stuff *xstuff = grace->gui->xstuff;
    X11_data *data;
    XPixmapFormatValues *pmf;
    int i, n;

    /* we need to perform the allocations */
    data = xmalloc(sizeof(X11_data));
    if (data == NULL) {
        return NULL;
    }
    
    memset(data, 0, sizeof(X11_data));
    
    data->xstuff = xstuff;

    data->pixel_size = 0;
    pmf = XListPixmapFormats(xstuff->disp, &n);
    if (pmf) {
        for (i = 0; i < n; i++) {
            if (pmf[i].depth == xstuff->depth) {
                data->pixel_size = pmf[i].bits_per_pixel/8;
                break;
            }
        }
        XFree((char *) pmf);
    }
    if (data->pixel_size == 0) {
        data->monomode = TRUE;
    }

    data->visual = DefaultVisual(xstuff->disp, xstuff->screennumber);

    return data;
}

static void VPoint2XPoint(const VPoint *vp, XPoint *xp)
{
    x11_VPoint2dev(vp, &xp->x, &xp->y);
}

static void x11_initcmap(const Canvas *canvas, X11_data *x11data)
{
    unsigned int i;
    RGB rgb;
    long pixel;
    
    for (i = 0; i < number_of_colors(canvas); i++) {
        /* even in mono, b&w must be allocated */
        if (x11data->monomode == FALSE || i < 2) {
            if (get_rgb(canvas, i, &rgb) == RETURN_SUCCESS) {
                pixel = x11_allocate_color(grace->gui, &rgb);
                if (pixel >= 0) {
                    x11data->pixels[i] = pixel;
                } else {
                    x11data->pixels[i] = BlackPixel(x11data->xstuff->disp,
                        x11data->xstuff->screennumber);
                }
            }
        } else {
            x11data->pixels[i] = BlackPixel(x11data->xstuff->disp,
                x11data->xstuff->screennumber);
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
    int i, j;
    double step;
    XPoint xp;
    
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
    
    XSetForeground(x11data->xstuff->disp, x11data->xstuff->gc, x11data->pixels[0]);
    XSetFillStyle(x11data->xstuff->disp, x11data->xstuff->gc, FillSolid);
    XFillRectangle(x11data->xstuff->disp, x11data->xstuff->bufpixmap, x11data->xstuff->gc, 0, 0, x11data->xstuff->win_w, x11data->xstuff->win_h);
    XSetForeground(x11data->xstuff->disp, x11data->xstuff->gc, x11data->pixels[1]);
    
    step = (double) (x11data->xstuff->win_scale)/10;
    for (i = 0; i < x11data->xstuff->win_w/step; i++) {
        for (j = 0; j < x11data->xstuff->win_h/step; j++) {
            xp.x = rint(i*step);
            xp.y = x11data->xstuff->win_h - rint(j*step);
            XDrawPoint(x11data->xstuff->disp, x11data->xstuff->bufpixmap, x11data->xstuff->gc, xp.x, xp.y);
        }
    }
    
    XDrawRectangle(x11data->xstuff->disp, x11data->xstuff->bufpixmap, x11data->xstuff->gc, 0, 0, x11data->xstuff->win_w - 1, x11data->xstuff->win_h - 1);
    
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
        XSetForeground(x11data->xstuff->disp, x11data->xstuff->gc, x11data->pixels[fg]);
        XSetBackground(x11data->xstuff->disp, x11data->xstuff->gc, x11data->pixels[bg]);
        XSetFillStyle(x11data->xstuff->disp, x11data->xstuff->gc, FillSolid);
    } else {
        Pattern *pat = canvas_get_pattern(canvas, p);
        Pixmap ptmp = XCreatePixmapFromBitmapData(x11data->xstuff->disp, x11data->xstuff->root,
            (char *) pat->bits, pat->width, pat->height,
            x11data->pixels[fg], x11data->pixels[bg], x11data->xstuff->depth);
/*
 *      XSetFillStyle(x11data->xstuff->disp, x11data->xstuff->gc, FillStippled);
 *      XSetStipple(x11data->xstuff->disp, x11data->xstuff->gc, curstipple);
 */
        XSetFillStyle(x11data->xstuff->disp, x11data->xstuff->gc, FillTiled);
        XSetTile(x11data->xstuff->disp, x11data->xstuff->gc, ptmp);
        
        XFreePixmap(x11data->xstuff->disp, ptmp);
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
    
    iw = (unsigned int) rint(getlinewidth(canvas)*x11data->xstuff->win_scale);
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
            XSetLineAttributes(x11data->xstuff->disp, x11data->xstuff->gc, iw, LineOnOffDash, lc, lj);
            XSetDashes(x11data->xstuff->disp, x11data->xstuff->gc, 0, xdarr, darr_len);
            xfree(xdarr);
        } else if (style == 1) {
            XSetLineAttributes(x11data->xstuff->disp, x11data->xstuff->gc, iw, LineSolid, lc, lj);
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
    
    VPoint2XPoint(vp, &xp);
    x11_setpen(canvas, x11data);
    XDrawPoint(x11data->xstuff->disp, x11data->xstuff->bufpixmap, x11data->xstuff->gc, xp.x, xp.y);
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
        VPoint2XPoint(&vps[i], &p[i]);
    }
    if (mode == POLYLINE_CLOSED) {
        p[n] = p[0];
    }
    
    x11_setdrawbrush(canvas, x11data);
    
    XDrawLines(x11data->xstuff->disp, x11data->xstuff->bufpixmap, x11data->xstuff->gc, p, xn, CoordModeOrigin);
    
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
        VPoint2XPoint(&vps[i], &p[i]);
    }
    
    x11_setpen(canvas, x11data);

    if (getfillrule(canvas) != x11data->fillrule) {
        x11data->fillrule = getfillrule(canvas);
        if (getfillrule(canvas) == FILLRULE_WINDING) {
            XSetFillRule(x11data->xstuff->disp, x11data->xstuff->gc, WindingRule);
        } else {
            XSetFillRule(x11data->xstuff->disp, x11data->xstuff->gc, EvenOddRule);
        }
    }

    XFillPolygon(x11data->xstuff->disp, x11data->xstuff->bufpixmap, x11data->xstuff->gc, p, nc, Complex, CoordModeOrigin);
    
    xfree(p);
}

/*
 *  x11_drawarc
 */
static void x11_drawarc(const Canvas *canvas, void *data,
    const VPoint *vp1, const VPoint *vp2, double a1, double a2)
{
    X11_data *x11data = (X11_data *) data;
    short x1, y1, x2, y2;
    
    x11_VPoint2dev(vp1, &x1, &y2);
    x11_VPoint2dev(vp2, &x2, &y1);

    x11_setdrawbrush(canvas, x11data);
    
    if (x1 != x2 || y1 != y2) {
        XDrawArc(x11data->xstuff->disp, x11data->xstuff->bufpixmap, x11data->xstuff->gc, MIN2(x1, x2), MIN2(y1, y2),
              abs(x2 - x1), abs(y2 - y1), (int) rint(64*a1), (int) rint(64*a2));
    } else { /* zero radius */
        XDrawPoint(x11data->xstuff->disp, x11data->xstuff->bufpixmap, x11data->xstuff->gc, x1, y1);
    }
}

/*
 *  x11_fillarc
 */
static void x11_fillarc(const Canvas *canvas, void *data,
    const VPoint *vp1, const VPoint *vp2, double a1, double a2, int mode)
{
    X11_data *x11data = (X11_data *) data;
    short x1, y1, x2, y2;
    
    x11_VPoint2dev(vp1, &x1, &y2);
    x11_VPoint2dev(vp2, &x2, &y1);
    
    x11_setpen(canvas, x11data);
    if (x1 != x2 || y1 != y2) {
        if (x11data->arcfillmode != mode) {
            x11data->arcfillmode = mode;
            if (mode == ARCFILL_CHORD) {
                XSetArcMode(x11data->xstuff->disp, x11data->xstuff->gc, ArcChord);
            } else {
                XSetArcMode(x11data->xstuff->disp, x11data->xstuff->gc, ArcPieSlice);
            }
        }
        XFillArc(x11data->xstuff->disp, x11data->xstuff->bufpixmap, x11data->xstuff->gc, MIN2(x1, x2), MIN2(y1, y2),
           abs(x2 - x1), abs(y2 - y1), (int) rint(64*a1), (int) rint(64*a2));
    } else { /* zero radius */
        XDrawPoint(x11data->xstuff->disp, x11data->xstuff->bufpixmap, x11data->xstuff->gc, x1, y1);
    }
}


static void x11_putpixmap(const Canvas *canvas, void *data,
    const VPoint *vp, int width, int height, char *databits,
    int pixmap_bpp, int bitmap_pad, int pixmap_type)
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
    
    VPoint2XPoint(vp, &xp);
      
    if (pixmap_bpp != 1) {
        if (x11data->monomode == TRUE) {
            /* TODO: dither pixmaps on mono displays */
            return;
        }
        pixmap_ptr = xcalloc(PADBITS(width, 8) * height, x11data->pixel_size);
        if (pixmap_ptr == NULL) {
            errmsg("xmalloc failed in x11_putpixmap()");
            return;
        }
 
        /* re-index pixmap */
        for (k = 0; k < height; k++) {
            for (j = 0; j < width; j++) {
                cindex = (unsigned char) (databits)[k*width+j];
                for (l = 0; l < x11data->pixel_size; l++) {
                    pixmap_ptr[x11data->pixel_size*(k*width+j) + l] =
                        (char) (x11data->pixels[cindex] >> (8*l));
                }
            }
        }

        ximage=XCreateImage(x11data->xstuff->disp, x11data->visual,
                           x11data->xstuff->depth, ZPixmap, 0, pixmap_ptr, width, height,
                           bitmap_pad,  /* lines padded to bytes */
                           0 /* number of bytes per line */
                           );

        if (pixmap_type == PIXMAP_TRANSPARENT) {
            clipmask_ptr = xcalloc((PADBITS(width, 8)>>3)
                                              * height, SIZEOF_CHAR);
            if (clipmask_ptr == NULL) {
                errmsg("xmalloc failed in x11_putpixmap()");
                return;
            } else {
                /* Note: We pad the clipmask always to byte boundary */
                bg = getbgcolor(canvas);
                for (k = 0; k < height; k++) {
                    line_off = k*(PADBITS(width, 8) >> 3);
                    for (j = 0; j < width; j++) {
                        cindex = (unsigned char) (databits)[k*width+j];
                        if (cindex != bg) {
                            clipmask_ptr[line_off+(j>>3)] |= (0x01 << (j%8));
                        }
                    }
                }
        
                clipmask = XCreateBitmapFromData(x11data->xstuff->disp,
                    x11data->xstuff->root, clipmask_ptr, width, height);
                xfree(clipmask_ptr);
            }
        }
    } else {
        pixmap_ptr = xcalloc((PADBITS(width, bitmap_pad)>>3) * height,
                                                        sizeof(unsigned char));
        if (pixmap_ptr == NULL) {
            errmsg("xmalloc failed in x11_putpixmap()");
            return;
        }
        memcpy(pixmap_ptr, databits, ((PADBITS(width, bitmap_pad)>>3) * height));

        fg = getcolor(canvas);
        if (fg != x11data->color) {
            XSetForeground(x11data->xstuff->disp, x11data->xstuff->gc, x11data->pixels[fg]);
            x11data->color = fg;
        }
        ximage = XCreateImage(x11data->xstuff->disp, x11data->visual,
                            1, XYBitmap, 0, pixmap_ptr, width, height,
                            bitmap_pad, /* lines padded to bytes */
                            0 /* number of bytes per line */
                            );
        if (pixmap_type == PIXMAP_TRANSPARENT) {
            clipmask = XCreateBitmapFromData(x11data->xstuff->disp,
                x11data->xstuff->root, pixmap_ptr, PADBITS(width, bitmap_pad), height);
        }
    }

    if (pixmap_type == PIXMAP_TRANSPARENT) {
        XSetClipMask(x11data->xstuff->disp, x11data->xstuff->gc, clipmask);
        XSetClipOrigin(x11data->xstuff->disp, x11data->xstuff->gc, xp.x, xp.y);
    }
        
    /* Force bit and byte order */
    ximage->bitmap_bit_order = LSBFirst;
    ximage->byte_order       = LSBFirst;
    
    XPutImage(x11data->xstuff->disp, x11data->xstuff->bufpixmap, x11data->xstuff->gc, ximage, 0, 0, xp.x, xp.y, width, height);
    
    /* XDestroyImage free's the image data - which is VERY wrong since we
       allocated (and hence, want to free) it ourselves. So, the trick is
       to set the image data to NULL to avoid the double free() */
    xfree(pixmap_ptr);
    ximage->data = NULL;
    XDestroyImage(ximage);
     
    if (pixmap_type == PIXMAP_TRANSPARENT) {
        XFreePixmap(x11data->xstuff->disp, clipmask);
        clipmask = 0;
        XSetClipMask(x11data->xstuff->disp, x11data->xstuff->gc, None);
        XSetClipOrigin(x11data->xstuff->disp, x11data->xstuff->gc, 0, 0);
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
    
    device_set_dpi(d, (float) data->xstuff->dpi, FALSE);
    
    /* disable font AA in mono mode */
    if (data->monomode == TRUE) {
        device_set_fontrast(d, FONT_RASTER_MONO);
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
