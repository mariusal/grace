/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 *
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 *
 * Copyright (c) 2011 Grace Development Team
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
 * EMF driver
 */

#include <string.h>
#include <math.h>

#define MIN2(a, b) (((a) < (b)) ? a : b)
#define MAX2(a, b) (((a) > (b)) ? a : b)

#define CANVAS_BACKEND_API
#include <grace/canvas.h>

//#include <QPainter>
//#include <QBitmap>

typedef struct {
    double x;
    double y;
} XPoint;

typedef struct {
//    QPainter *painter;
//    QImage *pixmap;

    unsigned int height, width, page_scale;

    int color;
    int bgcolor;
    int patno;
    int linewidth;
    int linestyle;
    int fillrule;
    int arcfillmode;
    int linecap;
    int linejoin;
} EMF_data;

static EMF_data *emf_data_new(void)
{
    EMF_data *data;

    /* we need to perform the allocations */
    data = xmalloc(sizeof(EMF_data));
    if (data) {
        memset(data, 0, sizeof(EMF_data));
    }

    return data;
}

static void emf_data_free(void *data)
{
    EMF_data *emfdata = (EMF_data *) data;

    if (emfdata) {
        xfree(emfdata);
    }
}

static void VPoint2XPoint(const EMF_data *emfdata, const VPoint *vp, XPoint *xp)
{
    xp->x = emfdata->page_scale*vp->x;
    xp->y = emfdata->height - emfdata->page_scale*vp->y;
}

//static QColor Color2QColor(const Canvas *canvas, const int color)
//{
//    RGB rgb;
//    get_rgb(canvas, color, &rgb);
//    return QColor(rgb.red, rgb.green, rgb.blue);
//}

// TODO: folowing is defined in xprotos.h
//typedef void *Pixmap;
//typedef struct {
//    Pixmap pixmap;
//} X11stream;

static int emf_initgraphics(const Canvas *canvas, void *data,
    const CanvasStats *cstats)
{
    EMF_data *emfdata = (EMF_data *) data;

    Page_geometry *pg = get_page_geometry(canvas);

    emfdata->width = pg->width;
    emfdata->height = pg->height;
    emfdata->page_scale = MIN2(pg->width, pg->height);

/*    X11stream *xstream = (X11stream *) canvas_get_prstream(canvas);
    emfdata->pixmap = (QImage *) xstream->pixmap;
    emfdata->painter = new QPainter(emfdata->pixmap);*/

    /* init settings specific to EMF driver */
    emfdata->color       = BAD_COLOR;
    emfdata->bgcolor     = BAD_COLOR;
    emfdata->patno       = -1;
    emfdata->linewidth   = -1;
    emfdata->linestyle   = -1;
    emfdata->fillrule    = -1;
    emfdata->arcfillmode = -1;
    emfdata->linecap     = -1;
    emfdata->linejoin    = -1;

    return RETURN_SUCCESS;
}

static void emf_setpen(const Canvas *canvas, EMF_data *emfdata)
{
/*    int fg;
    Pen pen;

    getpen(canvas, &pen);
    fg = pen.color;

    QPen qpen(Color2QColor(canvas, fg));
    qpen.setStyle(Qt::SolidLine);
    emfdata->painter->setPen(qpen);*/
}

static void emf_setdrawbrush(const Canvas *canvas, EMF_data *emfdata)
{
/*    unsigned int iw;
    int style;
    int lc, lj;
    int i, darr_len;

    emf_setpen(canvas, emfdata);

    iw = (unsigned int) rint(getlinewidth(canvas) * emfdata->page_scale);
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

    //    if (iw != emfdata->linewidth || style != emfdata->linestyle ||
    //        lc != emfdata->linecap   || lj    != emfdata->linejoin) {
    if (style > 1) {
        LineStyle *linestyle = canvas_get_linestyle(canvas, style);
        darr_len = linestyle->length;
        QVector<qreal> dashes(darr_len);
        for (i = 0; i < darr_len; i++) {
            dashes[i] = linestyle->array[i];
        }
        QPen pen = emfdata->painter->pen();

        pen.setWidth(iw);
        pen.setCapStyle((Qt::PenCapStyle) lc);
        pen.setJoinStyle((Qt::PenJoinStyle) lj);
        pen.setDashPattern(dashes);

        emfdata->painter->setPen(pen);
    } else if (style == 1) {
        QPen pen = emfdata->painter->pen();

        pen.setWidth(iw);
        pen.setStyle(Qt::SolidLine);
        pen.setCapStyle((Qt::PenCapStyle) lc);
        pen.setJoinStyle((Qt::PenJoinStyle) lj);

        emfdata->painter->setPen(pen);
    }

    //        emfdata->linestyle = style;
    //        emfdata->linewidth = iw;
    //        emfdata->linecap   = lc;
    //        emfdata->linejoin  = lj;
    //    }
    return;*/
}

static void emf_drawpixel(const Canvas *canvas, void *data,
    const VPoint *vp)
{
    EMF_data *emfdata = (EMF_data *) data;
/*    XPoint xp;

    VPoint2XPoint(emfdata, vp, &xp);
    emf_setpen(canvas, emfdata);
    emfdata->painter->drawPoint(QPointF(xp.x, xp.y));*/
}

static void emf_drawpolyline(const Canvas *canvas, void *data,
    const VPoint *vps, int n, int mode)
{
    EMF_data *emfdata = (EMF_data *) data;
/*    int i, xn = n;
    XPoint *p;

    if (mode == POLYLINE_CLOSED) {
        xn++;
    }

    p = (XPoint *) xmalloc(xn * sizeof(XPoint));
    if (p == NULL) {
        return;
    }

    QPointF points[xn];
    for (i = 0; i < n; i++) {
        VPoint2XPoint(emfdata, &vps[i], &p[i]);
        points[i] = QPointF(p[i].x, p[i].y);
    }
    if (mode == POLYLINE_CLOSED) {
        points[n] = QPointF(p[0].x, p[0].y);
    }

    emf_setdrawbrush(canvas, emfdata);

    emfdata->painter->drawPolyline(points, xn);

    xfree(p);*/
}

static void emf_setfillpen(const Canvas *canvas, EMF_data *emfdata)
{
/*    int fg, p;
    Pen pen;

    getpen(canvas, &pen);
    fg = pen.color;
    p = pen.pattern;

    if (p == 0) { *//* TODO: transparency !!!*/
/*        return;
    } else if (p == 1) {
        emfdata->painter->setPen(Qt::NoPen);
        emfdata->painter->setBrush(QBrush(Color2QColor(canvas, fg)));
    } else {
        emfdata->painter->setPen(Qt::NoPen);

        Pattern *pat = canvas_get_pattern(canvas, p);
        QBitmap bitmap = QBitmap::fromData(QSize(pat->width, pat->height),
                pat->bits, QImage::Format_MonoLSB);
        QBrush brush(Color2QColor(canvas, fg));
        brush.setTexture(bitmap);
        emfdata->painter->setBrush(brush);
    }*/
}

static void emf_fillpolygon(const Canvas *canvas, void *data,
    const VPoint *vps, int nc)
{
    EMF_data *emfdata = (EMF_data *) data;
/*    int i;
    XPoint *p;

    p = (XPoint *) xmalloc(nc * sizeof(XPoint));
    if (p == NULL) {
        return;
    }

    QPointF points[nc];
    for (i = 0; i < nc; i++) {
        VPoint2XPoint(emfdata, &vps[i], &p[i]);
        points[i] = QPointF(p[i].x, p[i].y);
    }

    emf_setfillpen(canvas, emfdata);

    Qt::FillRule rule;
    if (getfillrule(canvas) == FILLRULE_WINDING) {
        rule = Qt::WindingFill;
    } else {
        rule = Qt::OddEvenFill;
    }

    emfdata->painter->drawPolygon(points, nc, rule);

    xfree(p);*/
}

static void emf_drawarc(const Canvas *canvas, void *data,
    const VPoint *vp1, const VPoint *vp2, double a1, double a2)
{
    EMF_data *emfdata = (EMF_data *) data;
/*    XPoint xp1, xp2;

    VPoint2XPoint(emfdata, vp1, &xp1);
    VPoint2XPoint(emfdata, vp2, &xp2);

    emf_setdrawbrush(canvas, emfdata);

    if (xp1.x != xp2.x || xp1.y != xp2.y) {
        double x = MIN2(xp1.x, xp2.x);
        double y = MIN2(xp1.y, xp2.y);
        double width = fabs(xp2.x - xp1.x);
        double height = fabs(xp2.y - xp1.y);
        int angle1 = (int) rint(16 * a1);
        int angle2 = (int) rint(16 * a2);

        emfdata->painter->drawArc(QRectF(x, y, width, height), angle1, angle2);
    } else { *//* zero radius */
/*        emfdata->painter->drawPoint(QPointF(xp1.x, xp1.y));
    }*/
}

static void emf_fillarc(const Canvas *canvas, void *data,
    const VPoint *vp1, const VPoint *vp2, double a1, double a2, int mode)
{
    EMF_data *emfdata = (EMF_data *) data;
/*    XPoint xp1, xp2;

    VPoint2XPoint(emfdata, vp1, &xp1);
    VPoint2XPoint(emfdata, vp2, &xp2);

    emf_setfillpen(canvas, emfdata);

    if (xp1.x != xp2.x || xp1.y != xp2.y) {
        double x = MIN2(xp1.x, xp2.x);
        double y = MIN2(xp1.y, xp2.y);
        double width = fabs(xp2.x - xp1.x);
        double height = fabs(xp2.y - xp1.y);
        int angle1 = (int) rint(16 * a1);
        int angle2 = (int) rint(16 * a2);

        if (mode == ARCCLOSURE_CHORD) {
            emfdata->painter->drawChord(QRectF(x, y, width, height), angle1, angle2);
        } else {
            emfdata->painter->drawPie(QRectF(x, y, width, height), angle1, angle2);
        }
    } else { *//* zero radius */
/*        emfdata->painter->drawPoint(QPointF(xp1.x, xp1.y));
    }*/
}

void emf_putpixmap(const Canvas *canvas, void *data,
    const VPoint *vp, const CPixmap *pm)
{
    EMF_data *emfdata = (EMF_data *) data;
/*    int cindex, bg;
    int color;

    int i, k, j;
    long paddedW;

    XPoint xp;
    double x, y;

    bg = getbgcolor(canvas);

    VPoint2XPoint(emfdata, vp, &xp);

    y = xp.y;
    if (pm->bpp == 1) {
        color = getcolor(canvas);
        paddedW = PADBITS(pm->width, pm->pad);
        for (k = 0; k < pm->height; k++) {
            x = xp.x;
            y++;
            for (j = 0; j < paddedW / pm->pad; j++) {
                for (i = 0; i < pm->pad && j * pm->pad + i < pm->width; i++) {
                    x++;
                    if (bin_dump(&(pm->bits)[k * paddedW / pm->pad + j], i, pm->pad)) {
                        QPen qpen(Color2QColor(canvas, color));
                        emfdata->painter->setPen(qpen);
                        emfdata->painter->drawPoint(QPointF(x, y));
                    } else {
                        if (pm->type == PIXMAP_OPAQUE) {
                            QPen qpen(Color2QColor(canvas, bg));
                            emfdata->painter->setPen(qpen);
                            emfdata->painter->drawPoint(QPointF(x, y));
                        }
                    }
                }
            }
        }
    } else {
        unsigned int *cptr = (unsigned int *) pm->bits;
        for (k = 0; k < pm->height; k++) {
            x = xp.x;
            y++;
            for (j = 0; j < pm->width; j++) {
                x++;
                cindex = cptr[k * pm->width + j];
                if (cindex != bg || pm->type == PIXMAP_OPAQUE) {
                    color = cindex;
                    QPen qpen(Color2QColor(canvas, color));
                    emfdata->painter->setPen(qpen);
                    emfdata->painter->drawPoint(QPointF(x, y));
                }
            }
        }
    }*/
}

static void emf_leavegraphics(const Canvas *canvas, void *data,
    const CanvasStats *cstats)
{
/*    EMF_data *emfdata = (EMF_data *) data;
    delete emfdata->painter;*/
}

int register_emf_drv(Canvas *canvas)
{
    Device_entry *d;
    EMF_data *data;

    data = emf_data_new();
    if (!data) {
        return -1;
    }

    d = device_new("EMF", DEVICE_FILE, FALSE, data, emf_data_free);
    if (!d) {
        xfree(data);
        return -1;
    }

    device_set_fext(d, "emf");

    device_set_procs(d,
        emf_initgraphics,
        emf_leavegraphics,
        NULL,
        NULL,
        emf_drawpixel,
        emf_drawpolyline,
        emf_fillpolygon,
        emf_drawarc,
        emf_fillarc,
        emf_putpixmap,
        NULL);

    return register_device(canvas, d);
}
