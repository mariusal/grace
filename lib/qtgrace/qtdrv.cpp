/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1996-2008 Grace Development Team
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
 * Qt driver
 */

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
using namespace std;
#include <math.h>

#define MIN2(a, b) (((a) < (b)) ? a : b)
#define MAX2(a, b) (((a) > (b)) ? a : b)
 
#define CANVAS_BACKEND_API
extern "C" {
  #include "grace/canvas.h"
}

#include <QPicture>
#include <QPainter>

typedef struct {
    short x;
    short y;
} XPoint;

typedef struct {
    RGB rgb;
    QColor pixel;
    int allocated;
} QtColor;

typedef struct {
//    Screen *screen;
//    Pixmap pixmap;
    QPainter *painter;
    QPicture *pixmap;
    
//    int pixel_size;
    unsigned int height, width, page_scale;
        
//    int monomode;

    int color;
    int bgcolor;
    int patno;
    int linewidth;
    int linestyle;
    int fillrule;
    int arcfillmode;
    int linecap;
    int linejoin;

    QtColor *colors;
    unsigned int ncolors;
} Qt_data;

static Qt_data *qt_data_new(void)
{
    Qt_data *data = new Qt_data;

    if (data) {
        memset(data, 0, sizeof(Qt_data));
    }

    return data;
}

static void qt_data_free(void *data)
{
    Qt_data *qtdata = (Qt_data *) data;

    if (qtdata) {
	delete (QtColor *) qtdata->colors;
    
	delete (Qt_data *) data;
    }
}

static void VPoint2XPoint(const Qt_data *qtdata, const VPoint *vp, XPoint *xp)
{
    xp->x = (short) rint(qtdata->page_scale*vp->x);
    xp->y = (short) rint(qtdata->height - qtdata->page_scale*vp->y);
}

static void qt_initcmap(const Canvas *canvas, Qt_data *qtdata)
{
    errmsg("init cmap qt drv");
    unsigned int i;
    RGB rgb;
    unsigned int ncolors = number_of_colors(canvas);
    
    qtdata->colors = (QtColor *) xrealloc(qtdata->colors, sizeof(QtColor)*ncolors);
    if (!qtdata->colors) {
        return;
    }
    
    if (ncolors && ncolors > qtdata->ncolors) {
        memset(qtdata->colors + qtdata->ncolors, 0,
            sizeof(QtColor)*(ncolors - qtdata->ncolors));
    }
    qtdata->ncolors = ncolors;
    
    for (i = 0; i < ncolors; i++) {
        QtColor *qc = &qtdata->colors[i];
        /* even in mono, b&w must be allocated */
//        if (qtdata->monomode == FALSE || i < 2) {
            if (get_rgb(canvas, i, &rgb) == RETURN_SUCCESS) {
                if (!qc->allocated || !compare_rgb(&rgb, &qc->rgb)) {
                    qc->pixel.setRgb(rgb.red, rgb.green, rgb.blue);
                    qc->rgb = rgb;
                    qc->allocated = TRUE;
                }
            }
//        } else {
//            qc->pixel = BlackPixelOfScreen(qtdata->screen);
//        }
    }
}

static void qt_updatecmap(const Canvas *canvas, void *data)
{
    errmsg("update cmap qt drv");
    Qt_data *qtdata = (Qt_data *) data;
    /* TODO: replace!!! */
    qt_initcmap(canvas, qtdata);
}

static int qt_initgraphics(const Canvas *canvas, void *data,
    const CanvasStats *cstats)
{
    errmsg("init graphics qt drv");
    Qt_data *qtdata = (Qt_data *) data;

    Page_geometry *pg = get_page_geometry(canvas);

    cout << "page width: " << pg->width << " page height: " << pg->height << endl;
    qtdata->width = pg->width;
    qtdata->height = pg->height;
    qtdata->page_scale = MIN2(pg->width, pg->height);

    qtdata->pixmap = (QPicture *) canvas_get_prstream(canvas);
    qtdata->painter = new QPainter(qtdata->pixmap);
    
    //qtdata->pixel_size = x11_get_pixelsize(gapp->gui);

//    if (qtdata->pixel_size == 0) {
//        qtdata->monomode = TRUE;
//    }

    /* init settings specific to X11 driver */    
    qtdata->color       = BAD_COLOR;
    qtdata->bgcolor     = BAD_COLOR;
    qtdata->patno       = -1;
    qtdata->linewidth   = -1;
    qtdata->linestyle   = -1;
    qtdata->fillrule    = -1;
    qtdata->arcfillmode = -1;
    qtdata->linecap     = -1;
    qtdata->linejoin    = -1;

    qt_initcmap(canvas, qtdata);
    
    return RETURN_SUCCESS;
}

static void qt_setpen(const Canvas *canvas, Qt_data *qtdata)
{
    int fg, bg, p;
    Pen pen;
    
    bg = getbgcolor(canvas);
    getpen(canvas, &pen);
    fg = pen.color;
    p = pen.pattern;
    
    if ((fg == qtdata->color) && (bg == qtdata->bgcolor) && (p == qtdata->patno)) {
        return;
    }
        
    qtdata->color = fg;
    qtdata->bgcolor = bg;
    qtdata->patno = p;
    
    if (p == 0) { /* TODO: transparency !!!*/
        return;
    } else if (p == 1) {
        /* To make X faster */
	qtdata->painter->setPen(QPen(qtdata->colors[fg].pixel));
	qtdata->painter->setBrush(QBrush(qtdata->colors[bg].pixel));
/*        XSetForeground(DisplayOfScreen(qtdata->screen),
            DefaultGCOfScreen(qtdata->screen), qtdata->colors[fg].pixel);
        XSetBackground(DisplayOfScreen(qtdata->screen),
            DefaultGCOfScreen(qtdata->screen), qtdata->colors[bg].pixel);
        XSetFillStyle(DisplayOfScreen(qtdata->screen),
            DefaultGCOfScreen(qtdata->screen), FillSolid);*/
    } else {
        /*Pattern *pat = canvas_get_pattern(canvas, p);
        Pixmap ptmp = XCreatePixmapFromBitmapData(DisplayOfScreen(qtdata->screen), RootWindowOfScreen(qtdata->screen),
            (char *) pat->bits, pat->width, pat->height,
            qtdata->colors[fg].pixel, qtdata->colors[bg].pixel,
                PlanesOfScreen(qtdata->screen));
        XSetFillStyle(DisplayOfScreen(qtdata->screen), DefaultGCOfScreen(qtdata->screen), FillTiled);
        XSetTile(DisplayOfScreen(qtdata->screen), DefaultGCOfScreen(qtdata->screen), ptmp);
        
        XFreePixmap(DisplayOfScreen(qtdata->screen), ptmp);*/
    }
}

static void qt_setdrawbrush(const Canvas *canvas, Qt_data *qtdata)
{
    unsigned int iw;
    int style;
    int lc, lj;
    int i, scale, darr_len;
    char *xdarr;

    qt_setpen(canvas, qtdata);
    
    iw = (unsigned int) rint(getlinewidth(canvas)*qtdata->page_scale);
    if (iw == 1) {
        iw = 0;
    }
    style = getlinestyle(canvas);
    lc = getlinecap(canvas);
    lj = getlinejoin(canvas);
    
    switch (lc) {
    case LINECAP_BUTT:
        lc = (int) Qt::FlatCap;
        break;
    case LINECAP_ROUND:
        lc = (int) Qt::RoundCap;
        break;
    case LINECAP_PROJ:
        lc = (int) Qt::SquareCap;
        break;
    }

    switch (lj) {
    case LINEJOIN_MITER:
        lj = (int) Qt::MiterJoin;
        break;
    case LINEJOIN_ROUND:
        lj = (int) Qt::RoundJoin;
        break;
    case LINEJOIN_BEVEL:
        lj = (int) Qt::BevelJoin;
        break;
    }
    
    if (iw != qtdata->linewidth || style != qtdata->linestyle ||
        lc != qtdata->linecap   || lj    != qtdata->linejoin) {
        if (style > 1) {
            LineStyle *linestyle = canvas_get_linestyle(canvas, style);
            darr_len = linestyle->length;
            xdarr = (char *) xmalloc(darr_len*SIZEOF_CHAR);
            if (xdarr == NULL) {
                return;
            }
            scale = MAX2(1, iw);
            for (i = 0; i < darr_len; i++) {
                xdarr[i] = scale*linestyle->array[i];
            }
            //XSetLineAttributes(DisplayOfScreen(qtdata->screen), DefaultGCOfScreen(qtdata->screen), iw, LineOnOffDash, lc, lj);
            //XSetDashes(DisplayOfScreen(qtdata->screen), DefaultGCOfScreen(qtdata->screen), 0, xdarr, darr_len);
            xfree(xdarr);
        } else if (style == 1) {
	    QPen pen = qtdata->painter->pen();

	    pen.setWidth(iw);
	    pen.setStyle(Qt::SolidLine);
	    pen.setCapStyle((Qt::PenCapStyle) lc);
	    pen.setJoinStyle((Qt::PenJoinStyle) lj);

	    qtdata->painter->setPen(pen);
        }
 
        qtdata->linestyle = style;
        qtdata->linewidth = iw;
        qtdata->linecap   = lc;
        qtdata->linejoin  = lj;
    }
    return;
}

static void qt_drawpixel(const Canvas *canvas, void *data,
    const VPoint *vp)
{
    errmsg("draw pixel qt drv");
    Qt_data *qtdata = (Qt_data *) data;
    XPoint xp;
    
    VPoint2XPoint(qtdata, vp, &xp);
    qt_setpen(canvas, qtdata);
    cout << "x: " << vp->x << " y: " << vp->y << endl;
    cout << "xp: " << xp.x << " yp: " << xp.y << endl;
    qtdata->painter->drawPoint(xp.x, xp.y);
}

static void qt_drawpolyline(const Canvas *canvas, void *data,
    const VPoint *vps, int n, int mode)
{
//    errmsg("draw polyline qt drv");
    Qt_data *qtdata = (Qt_data *) data;
    int i, xn = n;
    XPoint *p;

    if (mode == POLYLINE_CLOSED) {
        xn++;
    }

    p = (XPoint *) xmalloc(xn*sizeof(XPoint));
    if (p == NULL) {
        return;
    }

    QPoint points[xn];
    for (i = 0; i < n; i++) {
        VPoint2XPoint(qtdata, &vps[i], &p[i]);
	points[i] = QPoint(p[i].x, p[i].y);
    }
    if (mode == POLYLINE_CLOSED) {
	points[n] = QPoint(p[0].x, p[0].y);
    }

    qt_setdrawbrush(canvas, qtdata);

    qtdata->painter->drawPolyline(points, xn);
}

static void qt_fillpolygon(const Canvas *canvas, void *data,
    const VPoint *vps, int nc)
{
    errmsg("fill polygon qt drv");
}

static void qt_drawarc(const Canvas *canvas, void *data,
    const VPoint *vp1, const VPoint *vp2, double a1, double a2)
{
  //  errmsg("draw arc qt drv");
    Qt_data *qtdata = (Qt_data *) data;
    XPoint xp;
    short x1, y1, x2, y2;

    VPoint2XPoint(qtdata, vp1, &xp);
    x1 = xp.x;
    y2 = xp.y;
    VPoint2XPoint(qtdata, vp2, &xp);
    x2 = xp.x;
    y1 = xp.y;

    qt_setdrawbrush(canvas, qtdata);

    if (x1 != x2 || y1 != y2) {
        int a1_64 = (int) rint(64*a1), a2_64 = (int) rint(64*a2);
        a1_64 %= 360*64;
	qtdata->painter->drawArc(QRectF (MIN2(x1, x2), MIN2(y1, y2), abs(x2 - x1), abs(y2 - y1)), a1_64, a2_64);
    } else { /* zero radius */
	qtdata->painter->drawPoint(x1, y1);
    }
}

static void qt_fillarc(const Canvas *canvas, void *data,
    const VPoint *vp1, const VPoint *vp2, double a1, double a2, int mode)
{
    errmsg("fill arc qt drv");
    Qt_data *qtdata = (Qt_data *) data;
    XPoint xp;
    short x1, y1, x2, y2;

    VPoint2XPoint(qtdata, vp1, &xp);
    x1 = xp.x;
    y2 = xp.y;
    VPoint2XPoint(qtdata, vp2, &xp);
    x2 = xp.x;
    y1 = xp.y;

    qt_setpen(canvas, qtdata);
    if (x1 != x2 || y1 != y2) {
        int a1_64 = (int) rint(64*a1), a2_64 = (int) rint(64*a2);
        a1_64 %= 360*64;
        if (qtdata->arcfillmode != mode) {
            qtdata->arcfillmode = mode;
            if (mode == ARCCLOSURE_CHORD) {
		qtdata->painter->drawChord(QRectF (MIN2(x1, x2), MIN2(y1, y2), abs(x2 - x1), abs(y2 - y1)), a1_64, a2_64);
                //XSetArcMode(DisplayOfScreen(qtdata->screen), DefaultGCOfScreen(qtdata->screen), ArcChord);
            } else {
		qtdata->painter->drawPie(QRectF (MIN2(x1, x2), MIN2(y1, y2), abs(x2 - x1), abs(y2 - y1)), a1_64, a2_64);
                //XSetArcMode(DisplayOfScreen(qtdata->screen), DefaultGCOfScreen(qtdata->screen), ArcPieSlice);
            }
        }
        //XFillArc(DisplayOfScreen(qtdata->screen), qtdata->pixmap, DefaultGCOfScreen(qtdata->screen), MIN2(x1, x2), MIN2(y1, y2),
           //abs(x2 - x1), abs(y2 - y1), a1_64, a2_64);
    } else { /* zero radius */
	qtdata->painter->drawPoint(x1, y1);
    }
}

void qt_putpixmap(const Canvas *canvas, void *data,
    const VPoint *vp, const CPixmap *pm)
{
    errmsg("put pixmap qt drv");
}

static void qt_leavegraphics(const Canvas *canvas, void *data,
    const CanvasStats *cstats)
{
    errmsg("leave graphics qt drv");
    Qt_data *qtdata = (Qt_data *) data;
    delete qtdata->painter;
}

int register_qt_drv(Canvas *canvas)
{
    Device_entry *d;
    Qt_data *data;

    data = qt_data_new();
    if (!data) {
        return -1;
    }

    d = device_new("Qt", DEVICE_TERM, FALSE, data, qt_data_free);
    if (!d) {
        return -1;
    }
    
    device_set_procs(d,
        qt_initgraphics,
        qt_leavegraphics,
        NULL,
        qt_updatecmap,
        qt_drawpixel,
        qt_drawpolyline,
        qt_fillpolygon,
        qt_drawarc,
        qt_fillarc,
        qt_putpixmap,
        NULL);
    
    errmsg("register qt drv");
    return register_device(canvas, d);
}
