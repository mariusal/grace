/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
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
 * ------------- coordinate transformation routines ------------
 */

#include <config.h>

#include <string.h>

#include "utils.h"
#include "ctrans.h"

static world worldwin;
static int coordinates;
static int scaletypex;
static int scaletypey;
static double xv_med;
static double yv_med;
static double xv_rc;
static double yv_rc;
static double fxg_med;
static double fyg_med;

/*
 * is_validWPoint() checks if a point is inside of (current) world rectangle
 */
int is_validWPoint(WPoint wp)
{
    if (coordinates == COORDINATES_POLAR) {
        if (wp.y >= 0.0 && wp.y <= worldwin.yg2) {
            return TRUE;
        } else {
            return FALSE;
        }
    } else {
        if (((wp.x >= worldwin.xg1 && wp.x <= worldwin.xg2) ||
             (wp.x >= worldwin.xg2 && wp.x <= worldwin.xg1)) &&
            ((wp.y >= worldwin.yg1 && wp.y <= worldwin.yg2) ||
             (wp.y >= worldwin.yg2 && wp.y <= worldwin.yg1))) {
            return TRUE;
        } else {
            return FALSE;
        }
    }
}

/*
 * Convert point's world coordinates to viewport
 */
VPoint Wpoint2Vpoint(WPoint wp)
{
    VPoint vp;
    world2view(wp.x, wp.y, &vp.x, &vp.y);
    return (vp);
}

/*
 * is_wpoint_inside() checks if point qp is inside of world rectangle w
 */
int is_wpoint_inside(WPoint *wp, world *w)
{
    return ((wp->x >= w->xg1) && (wp->x <= w->xg2) &&
            (wp->y >= w->yg1) && (wp->y <= w->yg2));
}

/*
 * Convert point's frame coordinates to viewport
 */
int Fpoint2Vpoint(Quark *f, const FPoint *fp, VPoint *vp)
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

char *scale_types(ScaleType it)
{
    char *s;

    switch (it) {
    case SCALE_NORMAL:
	s = "Normal";
	break;
    case SCALE_LOG:
	s = "Logarithmic";
	break;
    case SCALE_REC:
	s = "Reciprocal";
	break;
    case SCALE_LOGIT:
	s = "Logit";
	break; 	   
    default:
        s = "Unknown";
	break;
    }
    
    return s;
}

ScaleType get_scale_type_by_name(const char *name)
{
    int i;
    for (i = 0; i < NUMBER_OF_SCALETYPES; i++) {
        if (!strcmp(scale_types(i), name)) {
            return i;
        }
    }
    
    return SCALE_BAD;
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
        return (log(wc/(1.0-wc)));
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
        return (exp(vc)/(1+exp(vc)));
    default:
        errmsg("internal error in ifscale()");
        return (vc);
    }
}


/*
 * map world co-ordinates to viewport
  */
double xy_xconv(double wx)
{
    if ((scaletypex == SCALE_LOG && wx <= 0.0) ||
        (scaletypex == SCALE_REC && wx == 0.0) ||
        (scaletypex == SCALE_LOGIT && wx <= 0.0) ||
	(scaletypex == SCALE_LOGIT && wx >= 1.0)){
        return 0;
    } else {
        return (xv_med + xv_rc*(fscale(wx, scaletypex) - fxg_med));
    }
}

double xy_yconv(double wy)
{
    if ((scaletypey == SCALE_LOG && wy <= 0.0) ||
        (scaletypey == SCALE_REC && wy == 0.0) ||
        (scaletypey == SCALE_LOGIT && wy <= 0.0) ||
	(scaletypey == SCALE_LOGIT && wy >= 1.0)) {
        return 0;
    } else {
        return (yv_med + yv_rc*(fscale(wy, scaletypey) - fyg_med));
    }
}

int polar2xy(double phi, double rho, double *x, double *y)
{
    if (rho < 0.0) {
        return (RETURN_FAILURE);
    } else {
        *x = rho*cos(phi);
        *y = rho*sin(phi);
        return (RETURN_SUCCESS);
    }
}

void xy2polar(double x, double y, double *phi, double *rho)
{
    *phi = atan2(y, x);
    *rho = hypot(x, y);
}

int world2view(double x, double y, double *xv, double *yv)
{
    if (coordinates == COORDINATES_POLAR) {
        if (polar2xy(xv_rc*x, yv_rc*y, xv, yv) != RETURN_SUCCESS) {
            return (RETURN_FAILURE);
        }
        *xv += xv_med;
        *yv += yv_med;
    } else {
        *xv = xy_xconv(x);
        *yv = xy_yconv(y);
    }
    return (RETURN_SUCCESS);
}

/*
 * view2world - given (xv,yv) in viewport coordinates, return world coordinates
 *            in (xw,yw)
 */
void view2world(double xv, double yv, double *xw, double *yw)
{
    if (coordinates == COORDINATES_POLAR) {
        xy2polar(xv - xv_med, yv - yv_med, xw, yw);
        *xw /= xv_rc;
        *yw /= yv_rc;
    } else {
        *xw = ifscale(fxg_med + (1.0/xv_rc)*(xv - xv_med), scaletypex);
        *yw = ifscale(fyg_med + (1.0/yv_rc)*(yv - yv_med), scaletypey);
    }
}

/*
 * definewindow - defines the scaling
 *               of the plotting rectangle to be used for clipping
 */
/* FIXME: remove the xyfixed argument */
int definewindow(const world *w, const view *v, int ctrans_type, int xyfixed,
    int xscale, int yscale, int invx, int invy)
{
    double dx, dy;
    
    /* Safety checks */
    if (!isvalid_viewport(v)) {
        errmsg("Invalid viewport");
        return RETURN_FAILURE;
    }
    
    dx = w->xg2 - w->xg1;
    if (dx <= 0.0) {
        errmsg("World DX <= 0.0");
        return RETURN_FAILURE;
    }
    dy = w->yg2 - w->yg1;
    if (dy <= 0.0) {
        errmsg("World DY <= 0.0");
        return RETURN_FAILURE;
    }

    switch (ctrans_type) {
    case COORDINATES_POLAR:
        if (w->yg2 <= 0.0) {
            errmsg("World Rho-max <= 0.0");
            return RETURN_FAILURE;
        } else
        if ((xscale != SCALE_NORMAL) ||
            (yscale != SCALE_NORMAL)) {
            errmsg("Only linear scales are supported in Polar plots");
            return RETURN_FAILURE;
        } else
        if (invy == TRUE) {
            errmsg("Can't set Y scale inverted in Polar plot");
            return RETURN_FAILURE;
        } else {
            xv_med = (v->xv1 + v->xv2)/2;
            if (invx == FALSE) {
                xv_rc = +1.0;
            } else {
                xv_rc = -1.0;
            }

            yv_med = (v->yv1 + v->yv2)/2;
            yv_rc = (MIN2(v->xv2 - v->xv1, v->yv2 - v->yv1)/2.0)/w->yg2;
        }
        break;
    default:
        /* FIXME: set_graph_type() should worry that for GRAPH_FIXED,  */
        /*        the scalings, world window etc are set appropriately */
        if (xyfixed) {
            if ((xscale != SCALE_NORMAL) ||
                (yscale != SCALE_NORMAL)) {
                errmsg("Only linear axis scale is allowed in Fixed graphs");
                return RETURN_FAILURE;
            } else {

                xv_med = (v->xv1 + v->xv2)/2;
                fxg_med = (w->xg1 + w->xg2)/2;
                yv_med = (v->yv1 + v->yv2)/2;
                fyg_med = (w->yg1 + w->yg2)/2;

                xv_rc = MIN2((v->xv2 - v->xv1)/(w->xg2 - w->xg1),
                             (v->yv2 - v->yv1)/(w->yg2 - w->yg1));
                yv_rc = xv_rc;
                if (invx == TRUE) {
                    xv_rc = -xv_rc;
                }
                if (invy == TRUE) {
                    yv_rc = -yv_rc;
                }
            }
        } else {
            if (xscale == SCALE_LOG) {
                if (w->xg1 <= 0) {
                    errmsg("World X-min <= 0.0");
                    return RETURN_FAILURE;
                }
                if (w->xg2 <= 0) {
                    errmsg("World X-max <= 0.0");
                    return RETURN_FAILURE;
                }
            } else if (xscale == SCALE_REC) {
                if (sign(w->xg1) != sign(w->xg2)) {
                    errmsg("X-axis contains 0");
                    return RETURN_FAILURE;
                }

            }
            if (xscale == SCALE_LOGIT) {
                if (w->xg1 <= 0) {
                    errmsg("World X-min <= 0.0");
                    return RETURN_FAILURE;
                }
                if (w->xg2 >= 1) {
                    errmsg("World X-max >= 1.0");
                    return RETURN_FAILURE;
                }
	    }    

            if (yscale == SCALE_LOG) {
                if (w->yg1 <= 0.0) {
                    errmsg("World Y-min <= 0.0");
                    return RETURN_FAILURE;
                }
                if (w->yg2 <= 0.0) {
                    errmsg("World Y-max <= 0.0");
                    return RETURN_FAILURE;
                }
            } else if (yscale == SCALE_REC) {
                if (sign(w->yg1) != sign(w->yg2)) {
                    errmsg("Y-axis contains 0");
                    return RETURN_FAILURE;
                }
            }
	    if (yscale == SCALE_LOGIT) {
                if (w->yg1 <= 0) {
                    errmsg("World Y-min <= 0.0");
                    return RETURN_FAILURE;
                }
                if (w->yg2 >= 1) {
                    errmsg("World Y-max >= 1.0");
                    return RETURN_FAILURE;
                }
	    }    

            xv_med = (v->xv1 + v->xv2)/2;
            fxg_med = (fscale(w->xg1, xscale) + fscale(w->xg2, xscale))/2;
            if (invx == FALSE) {
                xv_rc = (v->xv2 - v->xv1)/(fscale(w->xg2, xscale) - fscale(w->xg1, xscale));
            } else {
                xv_rc = - (v->xv2 - v->xv1)/(fscale(w->xg2, xscale) - fscale(w->xg1, xscale));
            }

            yv_med = (v->yv1 + v->yv2)/2;
            fyg_med = (fscale(w->yg1, yscale) + fscale(w->yg2, yscale))/2;
            if (invy == FALSE) {
                yv_rc = (v->yv2 - v->yv1)/(fscale(w->yg2, yscale) - fscale(w->yg1, yscale));
            } else {
                yv_rc = - (v->yv2 - v->yv1)/(fscale(w->yg2, yscale) - fscale(w->yg1, yscale));
            }
        }
 
        break;
    }
    
    worldwin = *w;
    coordinates = ctrans_type;
    scaletypex = xscale;
    scaletypey = yscale;
    
    return RETURN_SUCCESS;
}
