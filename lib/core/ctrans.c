/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1996-2004 Grace Development Team
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
 * ------------- coordinate transformation routines ------------
 */

#include <config.h>

#include <string.h>

#include "grace/core.h"

typedef struct {
    int xscale;
    int yscale;
    int coordinates;
    double xv_med;
    double yv_med;
    double xv_rc;
    double yv_rc;
    double fxg_med;
    double fyg_med;
} ctrans_data;

static const Quark *get_defining_graph(const Quark *q)
{
    if (q && q->fid == QFlavorGraph) {
        return q;
    } else {
        return get_parent_graph(q);
    }
}

static int get_ctrans_data(const Quark *q, ctrans_data *cd)
{
    graph *g = graph_get_data(get_defining_graph(q));
    
    if (g && cd) {
        if (g->type == GRAPH_POLAR) {
            cd->coordinates = COORDINATES_POLAR;
        } else {
            cd->coordinates = COORDINATES_XY;
        }
        cd->xscale  = g->xscale;
        cd->yscale  = g->yscale;
        cd->xv_med  = g->ccache.xv_med;
        cd->yv_med  = g->ccache.yv_med;
        cd->xv_rc   = g->ccache.xv_rc;
        cd->yv_rc   = g->ccache.yv_rc;
        cd->fxg_med = g->ccache.fxg_med;
        cd->fyg_med = g->ccache.fyg_med;
        
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int polar2xy(double phi, double rho, double *x, double *y)
{
    if (rho < 0.0) {
        return RETURN_FAILURE;
    } else {
        *x = rho*cos(phi);
        *y = rho*sin(phi);
        return RETURN_SUCCESS;
    }
}

void xy2polar(double x, double y, double *phi, double *rho)
{
    *phi = atan2(y, x);
    *rho = hypot(x, y);
}

/*
 * is_wpoint_inside() checks if point qp is inside of world rectangle w
 */
static int is_wpoint_inside(const WPoint *wp, const world *w)
{
    return ((wp->x >= w->xg1) && (wp->x <= w->xg2) &&
            (wp->y >= w->yg1) && (wp->y <= w->yg2));
}

/*
 * is_validWPoint() checks if a point is inside of (current) world rectangle
 */
int is_validWPoint(const Quark *q, const WPoint *wp)
{
    world w;
    if (graph_get_world(get_defining_graph(q), &w) != RETURN_SUCCESS) {
        return FALSE;
    }
    
    return is_wpoint_inside(wp, &w);
}

static int world2view(const Quark *q,
    double x, double y, double *xv, double *yv)
{
    ctrans_data cd;
    if (get_ctrans_data(q, &cd) != RETURN_SUCCESS) {
        return FALSE;
    }
    
    if (cd.coordinates == COORDINATES_POLAR) {
        if (polar2xy(cd.xv_rc*x, cd.yv_rc*y, xv, yv) != RETURN_SUCCESS) {
            return (RETURN_FAILURE);
        }
        *xv += cd.xv_med;
        *yv += cd.yv_med;
    } else {
        *xv = xy_xconv(q, x);
        *yv = xy_yconv(q, y);
    }
    return (RETURN_SUCCESS);
}

/*
 * map world co-ordinates to viewport
  */
double xy_xconv(const Quark *q, double wx)
{
    ctrans_data cd;
    if (get_ctrans_data(q, &cd) != RETURN_SUCCESS) {
        return FALSE;
    }
    
    if ((cd.xscale == SCALE_LOG && wx <= 0.0) ||
        (cd.xscale == SCALE_REC && wx == 0.0) ||
        (cd.xscale == SCALE_LOGIT && wx <= 0.0) ||
	(cd.xscale == SCALE_LOGIT && wx >= 1.0)){
        return 0.0;
    } else {
        return (cd.xv_med + cd.xv_rc*(fscale(wx, cd.xscale) - cd.fxg_med));
    }
}

double xy_yconv(const Quark *q, double wy)
{
    ctrans_data cd;
    if (get_ctrans_data(q, &cd) != RETURN_SUCCESS) {
        return FALSE;
    }
    
    if ((cd.yscale == SCALE_LOG && wy <= 0.0) ||
        (cd.yscale == SCALE_REC && wy == 0.0) ||
        (cd.yscale == SCALE_LOGIT && wy <= 0.0) ||
	(cd.yscale == SCALE_LOGIT && wy >= 1.0)) {
        return 0.0;
    } else {
        return (cd.yv_med + cd.yv_rc*(fscale(wy, cd.yscale) - cd.fyg_med));
    }
}


/*
 * Convert point's world coordinates to viewport
 */
int Wpoint2Vpoint(const Quark *q, const WPoint *wp, VPoint *vp)
{
    return world2view(q, wp->x, wp->y, &vp->x, &vp->y);
}

/*
 * Convert point's frame coordinates to viewport
 */
int Fpoint2Vpoint(const Quark *f, const FPoint *fp, VPoint *vp)
{
    view *v = frame_get_view(f);
    if (v) {
        vp->x = v->xv1 + (v->xv2 - v->xv1)*fp->x;
        vp->y = v->yv1 + (v->yv2 - v->yv1)*fp->y;
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

/*
 * axis scaling
 */
double fscale(double wc, int scale)
{
    switch (scale) {
    case SCALE_NORMAL:
        return (wc);
    case SCALE_LOG:
        return (log10(wc));
    case SCALE_REC:
        return (1.0/wc);
    case SCALE_LOGIT:
        return (log(wc/(1.0 - wc)));
    default:
        errmsg("internal error in fscale()");
        return (wc);
    }
}

/*
 * inverse of the above
 */
double ifscale(double vc, int scale)
{
    switch (scale) {
    case SCALE_NORMAL:
        return (vc);
    case SCALE_LOG:
        return (pow(10.0, vc));
    case SCALE_REC:
        return (1.0/vc);
    case SCALE_LOGIT:
        return (exp(vc)/(1.0 + exp(vc)));
    default:
        errmsg("internal error in ifscale()");
        return (vc);
    }
}


/*
 * Convert point's viewport coordinates to world ones 
 */
int Vpoint2Wpoint(const Quark *q, const VPoint *vp, WPoint *wp)
{
    ctrans_data cd;
    if (get_ctrans_data(q, &cd) != RETURN_SUCCESS) {
        return FALSE;
    }
    
    if (cd.coordinates == COORDINATES_POLAR) {
        xy2polar(vp->x - cd.xv_med, vp->y - cd.yv_med, &wp->x, &wp->y);
        wp->x /= cd.xv_rc;
        wp->y /= cd.yv_rc;
    } else {
        wp->x = ifscale(cd.fxg_med + (1.0/cd.xv_rc)*(vp->x - cd.xv_med),
            cd.xscale);
        wp->y = ifscale(cd.fyg_med + (1.0/cd.yv_rc)*(vp->y - cd.yv_med),
            cd.yscale);
    }
    
    return RETURN_SUCCESS;
}

/* updates coordinate transform cached values */
int update_graph_ccache(Quark *gr)
{
    graph *g = graph_get_data(gr);
    view *v = frame_get_view(get_parent_frame(gr));
    int ctrans_type, xyfixed;

    if (!g || !v) {
        return RETURN_FAILURE;
    }
    
    switch (g->type) {
    case GRAPH_POLAR:
        ctrans_type = COORDINATES_POLAR;
        xyfixed = FALSE;
        break;
    case GRAPH_FIXED:
        ctrans_type = COORDINATES_XY;
        xyfixed = TRUE;
        break;
    default: 
        ctrans_type = COORDINATES_XY;
        xyfixed = FALSE;
        break;
    }
    
    switch (ctrans_type) {
    case COORDINATES_POLAR:
        g->ccache.xv_med = (v->xv1 + v->xv2)/2;
        if (g->xinvert == FALSE) {
            g->ccache.xv_rc = +1.0;
        } else {
            g->ccache.xv_rc = -1.0;
        }

        g->ccache.yv_med = (v->yv1 + v->yv2)/2;
        g->ccache.yv_rc = (MIN2(v->xv2 - v->xv1, v->yv2 - v->yv1)/2.0)/g->w.yg2;
        break;
    case COORDINATES_XY:
        if (xyfixed) {
            g->ccache.xv_med = (v->xv1 + v->xv2)/2;
            g->ccache.fxg_med = (g->w.xg1 + g->w.xg2)/2;
            g->ccache.yv_med = (v->yv1 + v->yv2)/2;
            g->ccache.fyg_med = (g->w.yg1 + g->w.yg2)/2;

            g->ccache.xv_rc = MIN2((v->xv2 - v->xv1)/(g->w.xg2 - g->w.xg1),
                         (v->yv2 - v->yv1)/(g->w.yg2 - g->w.yg1));
            g->ccache.yv_rc = g->ccache.xv_rc;
            if (g->xinvert == TRUE) {
                g->ccache.xv_rc = -g->ccache.xv_rc;
            }
            if (g->yinvert == TRUE) {
                g->ccache.yv_rc = -g->ccache.yv_rc;
            }
        } else {
            g->ccache.xv_med = (v->xv1 + v->xv2)/2;
            g->ccache.fxg_med =
                (fscale(g->w.xg1, g->xscale) + fscale(g->w.xg2, g->xscale))/2;
            if (g->xinvert == FALSE) {
                g->ccache.xv_rc = (v->xv2 - v->xv1)/
                    (fscale(g->w.xg2, g->xscale) - fscale(g->w.xg1, g->xscale));
            } else {
                g->ccache.xv_rc = - (v->xv2 - v->xv1)/
                    (fscale(g->w.xg2, g->xscale) - fscale(g->w.xg1, g->xscale));
            }

            g->ccache.yv_med = (v->yv1 + v->yv2)/2;
            g->ccache.fyg_med =
                (fscale(g->w.yg1, g->yscale) + fscale(g->w.yg2, g->yscale))/2;
            if (g->yinvert == FALSE) {
                g->ccache.yv_rc = (v->yv2 - v->yv1)/
                    (fscale(g->w.yg2, g->yscale) - fscale(g->w.yg1, g->yscale));
            } else {
                g->ccache.yv_rc = - (v->yv2 - v->yv1)/
                    (fscale(g->w.yg2, g->yscale) - fscale(g->w.yg1, g->yscale));
            }
        }
        break;
    default:
        errmsg("internal error in update_graph_ccache()");
        break;
    }
    
    return RETURN_SUCCESS;
}
