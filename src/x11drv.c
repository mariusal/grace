/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
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
 *
 * driver for X11 for Grace
 *
 */

#include <config.h>
#include "defines.h"

#include <stdlib.h>

#include <X11/Xlib.h>

#include "globals.h"
#include "utils.h"
#include "devlist.h"
#include "grace/canvas.h"
#include "graphs.h"

#include "x11drv.h"

#include "protos.h"

extern Display *disp;
extern Window xwin;

Window root;
int screennumber;
GC gc, gcxor;
int depth;

static Visual *visual;
static int pixel_size;

int install_cmap = CMAP_INSTALL_AUTO;

static int private_cmap = FALSE;

unsigned long xvlibcolors[MAXCOLORS];
Colormap cmap;

static Pixmap displaybuff = (Pixmap) NULL;

static int xlibcolor;
static int xlibbgcolor;
static int xlibpatno;
static int xliblinewidth;
static int xliblinestyle;
static int xlibfillrule;
static int xlibarcfillmode;
static int xliblinecap;
static int xliblinejoin;

unsigned int win_h = 0, win_w = 0;
#define win_scale ((win_h < win_w) ? win_h:win_w)

Pixmap resize_bufpixmap(unsigned int w, unsigned int h);

int register_x11_drv(Canvas *canvas)
{
    long mrsize;
    int max_path_limit;
    Device_entry *d;
    float dpi;
    
    d = device_new("X11", DEVICE_TERM, FALSE, NULL);
    if (!d) {
        return -1;
    }
    
    /* XExtendedMaxRequestSize() appeared in X11R6 */
#if XlibSpecificationRelease > 5
    mrsize = XExtendedMaxRequestSize(disp);
#else
    mrsize = 0;
#endif
    if (mrsize <= 0) {
        mrsize = XMaxRequestSize(disp);
    }
    max_path_limit = (mrsize - 3)/2;
    if (max_path_limit < get_max_path_limit(canvas)) {
        char buf[128];
        sprintf(buf,
            "Setting max drawing path length to %d (limited by the X server)",
            max_path_limit);
        errmsg(buf);
        set_max_path_limit(canvas, max_path_limit);
    }
    
    dpi = (float) rint(MM_PER_INCH*DisplayWidth(disp, screennumber)/
        DisplayWidthMM(disp, screennumber));
    
    device_set_dpi(d, dpi, FALSE);
    
    device_set_procs(d,
        xlibinitgraphics,
        xlibleavegraphics,
        NULL,
        NULL,
        xlibupdatecmap,
        xlibdrawpixel,
        xlibdrawpolyline,
        xlibfillpolygon,
        xlibdrawarc,
        xlibfillarc,
        xlibputpixmap,
        NULL);

/*
 * disable font AA in mono mode
 */
    if (grace->gui->monomode == TRUE) {
        device_set_fontrast(d, FONT_RASTER_MONO);
    }
    
    return register_device(canvas, d);
}

int xlibinit(const Canvas *canvas)
{
    XGCValues gc_val;
    XPixmapFormatValues *pmf;
    int i, n;
    
    screennumber = DefaultScreen(disp);
    visual = DefaultVisual(disp, screennumber);
    root = RootWindow(disp, screennumber);
 
    gc = DefaultGC(disp, screennumber);
    
    depth = DisplayPlanes(disp, screennumber);

    pixel_size = 0;
    pmf = XListPixmapFormats (disp, &n);
    if (pmf) {
        for (i = 0; i < n; i++) {
            if (pmf[i].depth == depth) {
                pixel_size = pmf[i].bits_per_pixel/8;
                break;
            }
        }
        XFree ((char *) pmf);
    }
    if (pixel_size == 0) {
        grace->gui->monomode = TRUE;
    }

/*
 * init colormap
 */
    cmap = DefaultColormap(disp, screennumber);
    /* redefine colormap, if needed */
    if (install_cmap == CMAP_INSTALL_ALWAYS) {
        cmap = XCopyColormapAndFree(disp, cmap);
        private_cmap = TRUE;
    }
    xlibinitcmap(canvas);
    
/*
 * set GCs
 */
    gc_val.foreground = xvlibcolors[0];
    gc_val.background = xvlibcolors[1];
    if (grace->gui->invert) {
        gc_val.function = GXinvert;
    } else {
        gc_val.function = GXxor;
    }
    gcxor = XCreateGC(disp, root, GCFunction | GCForeground, &gc_val);

    displaybuff = resize_bufpixmap(win_w, win_h);
    
    return RETURN_SUCCESS;
}


int xconvxlib(double x)
{
    return ((int) rint(win_scale * x));
}

int yconvxlib(double y)
{
    return ((int) rint(win_h - win_scale * y));
}

void xlibVPoint2dev(const VPoint *vp, int *x, int *y)
{
    *x = xconvxlib(vp->x);
    *y = yconvxlib(vp->y);
}

void VPoint2XPoint(const VPoint *vp, XPoint *xp)
{
    xp->x = xconvxlib(vp->x);
    xp->y = yconvxlib(vp->y);
}

/*
 * xlibdev2VPoint - given (x,y) in screen coordinates, return the 
 * viewport coordinates
 */
void xlibdev2VPoint(int x, int y, VPoint *vp)
{
    if (win_scale == 0) {
        vp->x = (double) 0.0;
        vp->y = (double) 0.0;
    } else {
        vp->x = (double) x / win_scale;
        vp->y = (double) (win_h - y) / win_scale;
    }
}


void xlibupdatecmap(const Canvas *canvas, void *data)
{
    /* TODO: replace!!! */
    if (grace->gui->inwin) {
        xlibinitcmap(canvas);
    }
}


void xlibinitcmap(const Canvas *canvas)
{
    int i;
    RGB rgb;
    XColor xc[MAXCOLORS];
    
    for (i = 0; i < MAXCOLORS; i++) {
        xc[i].pixel = 0;
        xc[i].flags = DoRed | DoGreen | DoBlue;
    }
    
    for (i = 0; i < number_of_colors(canvas); i++) {
        /* even in mono, b&w must be allocated */
        if (grace->gui->monomode == FALSE || i < 2) { 
            if (get_rgb(canvas, i, &rgb) == RETURN_SUCCESS) {
                xc[i].red   = rgb.red << (16 - GRACE_BPP);
                xc[i].green = rgb.green << (16 - GRACE_BPP);
                xc[i].blue  = rgb.blue << (16 - GRACE_BPP);
                if (XAllocColor(disp, cmap, &xc[i])) {
                    xvlibcolors[i] = xc[i].pixel;
                } else {
                    if (install_cmap != CMAP_INSTALL_NEVER && 
                                        private_cmap == FALSE) {
                        cmap = XCopyColormapAndFree(disp, cmap);
                        private_cmap = TRUE;
                        /* will try to allocate the same color 
                         * in the private colormap
                         */
                        i--; 
                    } else {
                        /* really bad */
                        xvlibcolors[i] = xvlibcolors[1];
/*
 *                         errmsg("Can't allocate color");
 */
                    }
                }
            }
        } else {
            xvlibcolors[i] = xvlibcolors[1];
        }
    }
}

int xlibinitgraphics(const Canvas *canvas, void *data,
    const CanvasStats *cstats)
{
    int i, j;
    double step;
    XPoint xp;
    
    if (grace->gui->inwin == FALSE) {
        return RETURN_FAILURE;
    }

    xlibcolor = BAD_COLOR;
    xlibbgcolor = BAD_COLOR;
    xlibpatno = -1;
    xliblinewidth = -1;
    xliblinestyle = -1;
    xlibarcfillmode = -1;
    xlibarcfillmode = -1;
    xliblinecap   = -1;
    xliblinejoin  = -1;
    
    /* init settings specific to X11 driver */    
    
    if (get_pagelayout() == PAGE_FIXED) {
        sync_canvas_size(&win_w, &win_h, FALSE);
    } else {
        sync_canvas_size(&win_w, &win_h, TRUE);
    }
    
    displaybuff = resize_bufpixmap(win_w, win_h);
    
    xlibupdatecmap(canvas, data);
    
    XSetForeground(disp, gc, xvlibcolors[0]);
    XSetFillStyle(disp, gc, FillSolid);
    XFillRectangle(disp, displaybuff, gc, 0, 0, win_w, win_h);
    XSetForeground(disp, gc, xvlibcolors[1]);
    
    step = (double) win_scale/10;
    for (i = 0; i < win_w/step; i++) {
        for (j = 0; j < win_h/step; j++) {
            xp.x = rint(i*step);
            xp.y = win_h - rint(j*step);
            XDrawPoint(disp, displaybuff, gc, xp.x, xp.y);
        }
    }
    
    XDrawRectangle(disp, displaybuff, gc, 0, 0, win_w - 1, win_h - 1);
    
    return RETURN_SUCCESS;
}


void xlib_setpen(const Canvas *canvas)
{
    int fg, bg, p;
    Pen pen;
    
    bg = getbgcolor(canvas);
    getpen(canvas, &pen);
    fg = pen.color;
    p = pen.pattern;
    
    if ((fg == xlibcolor) && (bg == xlibbgcolor) && (p == xlibpatno)) {
        return;
    }
        
    xlibcolor = fg;
    xlibbgcolor = bg;
    xlibpatno = p;
    
    if (p == 0) { /* TODO: transparency !!!*/
        return;
    } else if (p == 1) {
        /* To make X faster */
        XSetForeground(disp, gc, xvlibcolors[fg]);
        XSetBackground(disp, gc, xvlibcolors[bg]);
        XSetFillStyle(disp, gc, FillSolid);
    } else {
        Pattern *pat = canvas_get_pattern(canvas, p);
        Pixmap ptmp = XCreatePixmapFromBitmapData(disp, root,
            (char *) pat->bits, pat->width, pat->height,
            xvlibcolors[fg], xvlibcolors[bg], depth);
/*
 *      XSetFillStyle(disp, gc, FillStippled);
 *      XSetStipple(disp, gc, curstipple);
 */
        XSetFillStyle(disp, gc, FillTiled);
        XSetTile(disp, gc, ptmp);
        
        XFreePixmap(disp, ptmp);
    }
}

void xlib_setdrawbrush(const Canvas *canvas)
{
    unsigned int iw;
    int style;
    int lc, lj;
    int i, scale, darr_len;
    char *xdarr;

    xlib_setpen(canvas);
    
    iw = (unsigned int) rint(getlinewidth(canvas)*win_scale);
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
    
    if (iw != xliblinewidth || style != xliblinestyle ||
        lc != xliblinecap   || lj    != xliblinejoin) {
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
            XSetLineAttributes(disp, gc, iw, LineOnOffDash, lc, lj);
            XSetDashes(disp, gc, 0, xdarr, darr_len);
            xfree(xdarr);
        } else if (style == 1) {
            XSetLineAttributes(disp, gc, iw, LineSolid, lc, lj);
        }
 
        xliblinestyle = style;
        xliblinewidth = iw;
        xliblinecap   = lc;
        xliblinejoin  = lj;
    }

    return;
}

void xlibdrawpixel(const Canvas *canvas, void *data,
    const VPoint *vp)
{
    XPoint xp;
    
    VPoint2XPoint(vp, &xp);
    xlib_setpen(canvas);
    XDrawPoint(disp, displaybuff, gc, xp.x, xp.y);
}

void xlibdrawpolyline(const Canvas *canvas, void *data,
    const VPoint *vps, int n, int mode)
{
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
    
    xlib_setdrawbrush(canvas);
    
    XDrawLines(disp, displaybuff, gc, p, xn, CoordModeOrigin);
    
    xfree(p);
}


void xlibfillpolygon(const Canvas *canvas, void *data,
    const VPoint *vps, int nc)
{
    int i;
    XPoint *p;
    
    p = (XPoint *) xmalloc(nc*sizeof(XPoint));
    if (p == NULL) {
        return;
    }
    
    for (i = 0; i < nc; i++) {
        VPoint2XPoint(&vps[i], &p[i]);
    }
    
    xlib_setpen(canvas);

    if (getfillrule(canvas) != xlibfillrule) {
        xlibfillrule = getfillrule(canvas);
        if (getfillrule(canvas) == FILLRULE_WINDING) {
            XSetFillRule(disp, gc, WindingRule);
        } else {
            XSetFillRule(disp, gc, EvenOddRule);
        }
    }

    XFillPolygon(disp, displaybuff, gc, p, nc, Complex, CoordModeOrigin);
    
    xfree(p);
}

/*
 *  xlibdrawarc
 */
void xlibdrawarc(const Canvas *canvas, void *data,
    const VPoint *vp1, const VPoint *vp2, double a1, double a2)
{
    int x1, y1, x2, y2;
    
    xlibVPoint2dev(vp1, &x1, &y2);
    xlibVPoint2dev(vp2, &x2, &y1);

    xlib_setdrawbrush(canvas);
    
    if (x1 != x2 || y1 != y2) {
        XDrawArc(disp, displaybuff, gc, MIN2(x1, x2), MIN2(y1, y2),
              abs(x2 - x1), abs(y2 - y1), (int) rint(64*a1), (int) rint(64*a2));
    } else { /* zero radius */
        XDrawPoint(disp, displaybuff, gc, x1, y1);
    }
}

/*
 *  xlibfillarc
 */
void xlibfillarc(const Canvas *canvas, void *data,
    const VPoint *vp1, const VPoint *vp2, double a1, double a2, int mode)
{
    int x1, y1, x2, y2;
    
    xlibVPoint2dev(vp1, &x1, &y2);
    xlibVPoint2dev(vp2, &x2, &y1);
    
    xlib_setpen(canvas);
    if (x1 != x2 || y1 != y2) {
        if (xlibarcfillmode != mode) {
            xlibarcfillmode = mode;
            if (mode == ARCFILL_CHORD) {
                XSetArcMode(disp, gc, ArcChord);
            } else {
                XSetArcMode(disp, gc, ArcPieSlice);
            }
        }
        XFillArc(disp, displaybuff, gc, MIN2(x1, x2), MIN2(y1, y2),
           abs(x2 - x1), abs(y2 - y1), (int) rint(64*a1), (int) rint(64*a2));
    } else { /* zero radius */
        XDrawPoint(disp, displaybuff, gc, x1, y1);
    }
}


void xlibputpixmap(const Canvas *canvas, void *data,
    const VPoint *vp, int width, int height, char *databits,
    int pixmap_bpp, int bitmap_pad, int pixmap_type)
{
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
        if (grace->gui->monomode == TRUE) {
            /* TODO: dither pixmaps on mono displays */
            return;
        }
        pixmap_ptr = xcalloc(PAD(width, 8) * height, pixel_size);
        if (pixmap_ptr == NULL) {
            errmsg("xmalloc failed in xlibputpixmap()");
            return;
        }
 
        /* re-index pixmap */
        for (k = 0; k < height; k++) {
            for (j = 0; j < width; j++) {
                cindex = (unsigned char) (databits)[k*width+j];
                for (l = 0; l < pixel_size; l++) {
                    pixmap_ptr[pixel_size*(k*width+j) + l] =
                                        (char) (xvlibcolors[cindex] >> (8*l));
                }
            }
        }

        ximage=XCreateImage(disp, visual,
                           depth, ZPixmap, 0, pixmap_ptr, width, height,
                           bitmap_pad,  /* lines padded to bytes */
                           0 /* number of bytes per line */
                           );

        if (pixmap_type == PIXMAP_TRANSPARENT) {
            clipmask_ptr = xcalloc((PAD(width, 8)>>3)
                                              * height, SIZEOF_CHAR);
            if (clipmask_ptr == NULL) {
                errmsg("xmalloc failed in xlibputpixmap()");
                return;
            } else {
                /* Note: We pad the clipmask always to byte boundary */
                bg = getbgcolor(canvas);
                for (k = 0; k < height; k++) {
                    line_off = k*(PAD(width, 8) >> 3);
                    for (j = 0; j < width; j++) {
                        cindex = (unsigned char) (databits)[k*width+j];
                        if (cindex != bg) {
                            clipmask_ptr[line_off+(j>>3)] |= (0x01 << (j%8));
                        }
                    }
                }
        
                clipmask=XCreateBitmapFromData(disp, root, clipmask_ptr, 
                                                            width, height);
                xfree(clipmask_ptr);
            }
        }
    } else {
        pixmap_ptr = xcalloc((PAD(width, bitmap_pad)>>3) * height,
                                                        sizeof(unsigned char));
        if (pixmap_ptr == NULL) {
            errmsg("xmalloc failed in xlibputpixmap()");
            return;
        }
        memcpy(pixmap_ptr, databits, ((PAD(width, bitmap_pad)>>3) * height));

        fg = getcolor(canvas);
        if (fg != xlibcolor) {
            XSetForeground(disp, gc, xvlibcolors[fg]);
            xlibcolor = fg;
        }
        ximage=XCreateImage(disp, visual,
                            1, XYBitmap, 0, pixmap_ptr, width, height,
                            bitmap_pad, /* lines padded to bytes */
                            0 /* number of bytes per line */
                            );
        if (pixmap_type == PIXMAP_TRANSPARENT) {
            clipmask=XCreateBitmapFromData(disp, root, pixmap_ptr, 
                                              PAD(width, bitmap_pad), height);
        }
    }

    if (pixmap_type == PIXMAP_TRANSPARENT) {
        XSetClipMask(disp, gc, clipmask);
        XSetClipOrigin(disp, gc, xp.x, xp.y);
    }
        
    /* Force bit and byte order */
    ximage->bitmap_bit_order = LSBFirst;
    ximage->byte_order       = LSBFirst;
    
    XPutImage(disp, displaybuff, gc, ximage, 0, 0, xp.x, xp.y, width, height);
    
    /* XDestroyImage free's the image data - which is VERY wrong since we
       allocated (and hence, want to free) it ourselves. So, the trick is
       to set the image data to NULL to avoid the double free() */
    xfree(pixmap_ptr);
    ximage->data = NULL;
    XDestroyImage(ximage);
     
    if (pixmap_type == PIXMAP_TRANSPARENT) {
        XFreePixmap(disp, clipmask);
        clipmask = 0;
        XSetClipMask(disp, gc, None);
        XSetClipOrigin(disp, gc, 0, 0);
    }    
}

void xlibleavegraphics(const Canvas *canvas, void *data, 
    const CanvasStats *cstats)
{
    Quark *gr = graph_get_current(grace->project);
    
    if (is_graph_hidden(gr) == FALSE) {
        draw_focus(gr);
    }
    reset_crosshair(FALSE);
    xlibredraw(xwin, 0, 0, win_w, win_h);
    
    XFlush(disp);
}
