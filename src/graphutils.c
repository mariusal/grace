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
 * utilities for graphs
 *
 */

#include <config.h>
#include <cmath.h>

#include <stdio.h>

#include "globals.h"
#include "utils.h"
#include "draw.h"
#include "graphs.h"
#include "objutils.h"
#include "graphutils.h"
#include "protos.h"

extern char print_file[];

static void auto_ticks(int gno, int axis);

char *get_format_types(FormatType f)
{
    char *s;

    s = "decimal";
    switch (f) {
    case FORMAT_DECIMAL:
	s = "decimal";
	break;
    case FORMAT_EXPONENTIAL:
	s = "exponential";
	break;
    case FORMAT_GENERAL:
	s = "general";
	break;
    case FORMAT_POWER:
	s = "power";
	break;
    case FORMAT_SCIENTIFIC:
	s = "scientific";
	break;
    case FORMAT_ENGINEERING:
	s = "engineering";
	break;
    case FORMAT_DDMMYY:
	s = "ddmmyy";
	break;
    case FORMAT_MMDDYY:
	s = "mmddyy";
	break;
    case FORMAT_YYMMDD:
	s = "yymmdd";
	break;
    case FORMAT_MMYY:
	s = "mmyy";
	break;
    case FORMAT_MMDD:
	s = "mmdd";
	break;
    case FORMAT_MONTHDAY:
	s = "monthday";
	break;
    case FORMAT_DAYMONTH:
	s = "daymonth";
	break;
    case FORMAT_MONTHS:
	s = "months";
	break;
    case FORMAT_MONTHSY:
	s = "monthsy";
	break;
    case FORMAT_MONTHL:
	s = "monthl";
	break;
    case FORMAT_DAYOFWEEKS:
	s = "dayofweeks";
	break;
    case FORMAT_DAYOFWEEKL:
	s = "dayofweekl";
	break;
    case FORMAT_DAYOFYEAR:
	s = "dayofyear";
	break;
    case FORMAT_HMS:
	s = "hms";
	break;
    case FORMAT_MMDDHMS:
	s = "mmddhms";
	break;
    case FORMAT_MMDDYYHMS:
	s = "mmddyyhms";
	break;
    case FORMAT_YYMMDDHMS:
	s = "yymmddhms";
	break;
    case FORMAT_DEGREESLON:
	s = "degreeslon";
	break;
    case FORMAT_DEGREESMMLON:
	s = "degreesmmlon";
	break;
    case FORMAT_DEGREESMMSSLON:
	s = "degreesmmsslon";
	break;
    case FORMAT_MMSSLON:
	s = "mmsslon";
	break;
    case FORMAT_DEGREESLAT:
	s = "degreeslat";
	break;
    case FORMAT_DEGREESMMLAT:
	s = "degreesmmlat";
	break;
    case FORMAT_DEGREESMMSSLAT:
	s = "degreesmmsslat";
	break;
    case FORMAT_MMSSLAT:
	s = "mmsslat";
	break;
    default:
	s = "unknown";
        errmsg("Internal error in get_format_types()");
	break;
    }
    return s;
}

FormatType get_format_type_by_name(const char *name)
{
    FormatType i;
    for (i = 0; i < NUMBER_OF_FORMATTYPES; i++) {
        if (!strcmp(get_format_types(i), name)) {
            return i;
        }
    }
    
    return FORMAT_BAD;
}


int wipeout(void)
{
    quark_free(grace->project);
    grace->project = project_new(grace);
    
    map_fonts(grace->rt->canvas, FONT_MAP_DEFAULT);
    print_file[0] = '\0';
    /* a hack! the global "curtype" (as well as all others) should be removed */
    grace->rt->curtype = SET_XY;
    
    return 0;
}


/* The following routines determine default axis range and tickmarks */

static void autorange_bysets(int gno, int *sets, int nsets, int autos_type);
static double nicenum(double x, int nrange, int round);

#define NICE_FLOOR   0
#define NICE_CEIL    1
#define NICE_ROUND   2

void autotick_axis(int gno, int axis)
{
    switch (axis) {
    case ALL_AXES:
        auto_ticks(gno, X_AXIS);
        auto_ticks(gno, ZX_AXIS);
        auto_ticks(gno, Y_AXIS);
        auto_ticks(gno, ZY_AXIS);
        break;
    case ALL_X_AXES:
        auto_ticks(gno, X_AXIS);
        auto_ticks(gno, ZX_AXIS);
        break;
    case ALL_Y_AXES:
        auto_ticks(gno, Y_AXIS);
        auto_ticks(gno, ZY_AXIS);
        break;
    default:
        auto_ticks(gno, axis);
        break;
    }
}

void autoscale_bysets(int gno, int *sets, int nsets, int autos_type)
{
    autorange_bysets(gno, sets, nsets, autos_type);
    switch (autos_type) {
    case AUTOSCALE_X:
        autotick_axis(gno, ALL_X_AXES);
        break;
    case AUTOSCALE_Y:
        autotick_axis(gno, ALL_Y_AXES);
        break;
    case AUTOSCALE_XY:
        autotick_axis(gno, ALL_AXES);
        break;
    }
}

int autoscale_graph(int gno, int autos_type)
{
    int nsets, *sets;
    nsets = number_of_sets(gno);
    if (nsets) {
        int i;
        sets = xmalloc(nsets*SIZEOF_INT);
        if (!sets) {
            return RETURN_FAILURE;
        }
        for (i = 0; i < nsets; i++) {
            sets[i] = i;
        }
        autoscale_bysets(gno, sets, nsets, autos_type);
        xfree(sets);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

static void round_axis_limits(double *amin, double *amax, int scale)
{
    double smin, smax;
    int nrange;
    
    if (*amin == *amax) {
        switch (sign(*amin)) {
        case 0:
            *amin = -1.0;
            *amax = +1.0;
            break;
        case 1:
            *amin /= 2.0;
            *amax *= 2.0;
            break;
        case -1:
            *amin *= 2.0;
            *amax /= 2.0;
            break;
        }
    } 
    
    if (scale == SCALE_LOG) {
        if (*amax <= 0.0) {
            errmsg("Can't autoscale a log axis by non-positive data");
            *amax = 10.0;
            *amin = 1.0;
            return;
        } else if (*amin <= 0.0) {
            errmsg("Data have non-positive values");
            *amin = *amax/1.0e3;
        }
        smin = log10(*amin);
        smax = log10(*amax);
    } else if (scale == SCALE_LOGIT) {
	if (*amax <= 0.0) {
            errmsg("Can't autoscale a logit axis by non-positive data");
            *amax = 0.9;
            *amin = 0.1;
            return;
        } else if (*amin <= 0.0) {
            errmsg("Data have non-positive values");
            *amin = 0.1;
        }
        smin = log(*amin/(1-*amin));
        smax = log(*amax/(1-*amax));	
    } else {
        smin = *amin;
        smax = *amax;
    }

    if (sign(smin) == sign(smax)) {
        nrange = -rint(log10(fabs(2*(smax - smin)/(smax + smin))));
        nrange = MAX2(0, nrange);
    } else {
        nrange = 0;
    }
    smin = nicenum(smin, nrange, NICE_FLOOR);
    smax = nicenum(smax, nrange, NICE_CEIL);
    if (sign(smin) == sign(smax)) {
        if (smax/smin > 5.0) {
            smin = 0.0;
        } else if (smin/smax > 5.0) {
            smax = 0.0;
        }
    }
    
    if (scale == SCALE_LOG) {
        *amin = pow(10.0, smin);
        *amax = pow(10.0, smax);
    } else if (scale == SCALE_LOGIT) {
	*amin = exp(smin)/(1.0 + exp(smin));
        *amax = exp(smax)/(1.0 + exp(smax));	
    } else {
        *amin = smin;
        *amax = smax;
    }
}

static void autorange_bysets(int gno, int *sets, int nsets, int autos_type)
{
    world w;
    double xmax, xmin, ymax, ymin;
    int scale;

    if (autos_type == AUTOSCALE_NONE) {
        return;
    }
    
    get_graph_world(gno, &w);
    
    if (get_graph_type(gno) == GRAPH_SMITH) {
        if (autos_type == AUTOSCALE_X || autos_type == AUTOSCALE_XY) {
            w.xg1 = -1.0;
            w.yg1 = -1.0;
        }
        if (autos_type == AUTOSCALE_Y || autos_type == AUTOSCALE_XY) {
            w.xg2 = 1.0;
            w.yg2 = 1.0;
	}
        set_graph_world(gno, w);
        return;
    }

    xmin=w.xg1;
    xmax=w.xg2;
    ymin=w.yg1;
    ymax=w.yg2;
    if (autos_type == AUTOSCALE_XY) {
        getsetminmax(gno, sets, nsets, &xmin, &xmax, &ymin, &ymax);
    } else if (autos_type == AUTOSCALE_X) {
        getsetminmax_c(gno, sets, nsets, &xmin, &xmax, &ymin, &ymax, 2);
    } else if (autos_type == AUTOSCALE_Y) {
        getsetminmax_c(gno, sets, nsets, &xmin, &xmax, &ymin, &ymax, 1);
    }

    if (autos_type == AUTOSCALE_X || autos_type == AUTOSCALE_XY) {
        scale = get_graph_xscale(gno);
        round_axis_limits(&xmin, &xmax, scale);
        w.xg1 = xmin;
        w.xg2 = xmax;
    }

    if (autos_type == AUTOSCALE_Y || autos_type == AUTOSCALE_XY) {
        scale = get_graph_yscale(gno);
        round_axis_limits(&ymin, &ymax, scale);
        w.yg1 = ymin;
        w.yg2 = ymax;
    }

    set_graph_world(gno, w);
}

static void auto_ticks(int gno, int axis)
{
    tickmarks *t;
    world w;
    double range, d, tmpmax, tmpmin;
    int axis_scale;

    t = get_graph_tickmarks(gno, axis);
    if (t == NULL) {
        return;
    }
    get_graph_world(gno, &w);

    if (is_xaxis(axis)) {
        tmpmin = w.xg1;
        tmpmax = w.xg2;
        axis_scale = get_graph_xscale(gno);
    } else {
        tmpmin = w.yg1;
        tmpmax = w.yg2;
        axis_scale = get_graph_yscale(gno);
    }

    if (axis_scale == SCALE_LOG) {
	if (t->tmajor <= 1.0) {
            t->tmajor = 10.0;
        }
        tmpmax = log10(tmpmax)/log10(t->tmajor);
	tmpmin = log10(tmpmin)/log10(t->tmajor);
    } else if (axis_scale == SCALE_LOGIT) {
    	if (t->tmajor >= 0.5) {
            t->tmajor = 0.4;
        }
        tmpmax = log(tmpmax/(1-tmpmax))/log(t->tmajor/(1-t->tmajor));
	tmpmin = log(tmpmin/(1-tmpmin))/log(t->tmajor/(1-t->tmajor)); 
    } else if (t->tmajor <= 0.0) {
        t->tmajor = 1.0;
    }
    
    range = tmpmax - tmpmin;
    if (axis_scale == SCALE_LOG) {
	d = ceil(range/(t->t_autonum - 1));
	t->tmajor = pow(t->tmajor, d);
    } 
    else if (axis_scale == SCALE_LOGIT ){
        d = ceil(range/(t->t_autonum - 1));
	t->tmajor = exp(d)/(1.0 + exp(d));
    } 
    else {
	d = nicenum(range/(t->t_autonum - 1), 0, NICE_ROUND);
	t->tmajor = d;
    }

    /* alter # of minor ticks only if the current value is anomalous */
    if (t->nminor < 0 || t->nminor > 10) {
        if (axis_scale != SCALE_LOG) {
	    t->nminor = 1;
        } else {
            t->nminor = 8;
        }
    }
    
    set_dirtystate();
}

/*
 * nicenum: find a "nice" number approximately equal to x
 */

static double nicenum(double x, int nrange, int round)
{
    int xsign;
    double f, y, fexp, rx, sx;
    
    if (x == 0.0) {
        return(0.0);
    }

    xsign = sign(x);
    x = fabs(x);

    fexp = floor(log10(x)) - nrange;
    sx = x/pow(10.0, fexp)/10.0;            /* scaled x */
    rx = floor(sx);                         /* rounded x */
    f = 10*(sx - rx);                       /* fraction between 0 and 10 */

    if ((round == NICE_FLOOR && xsign == +1) ||
        (round == NICE_CEIL  && xsign == -1)) {
        y = (int) floor(f);
    } else if ((round == NICE_FLOOR && xsign == -1) ||
               (round == NICE_CEIL  && xsign == +1)) {
	y = (int) ceil(f);
    } else {    /* round == NICE_ROUND */
	if (f < 1.5)
	    y = 1;
	else if (f < 3.)
	    y = 2;
	else if (f < 7.)
	    y = 5;
	else
	    y = 10;
    }
    
    sx = rx + (double) y/10.0;
    
    return (xsign*sx*10.0*pow(10.0, fexp));
}

/*
 * set scroll amount
 */
void scroll_proc(int value)
{
    grace->rt->scrollper = value / 100.0;
}

void scrollinout_proc(int value)
{
    grace->rt->shexper = value / 100.0;
}

/*
 * pan through world coordinates
 */
int graph_scroll(int type)
{
    world w;
    double dwc = 0.0;
    int gno, cg = get_cg(), gmin, gmax;

    if (grace->rt->scrolling_islinked) {
        gmin = 0;
        gmax = number_of_graphs() - 1;
    } else {
        gmin = cg;
        gmax = cg;
    }
    
    for (gno = gmin; gno <= gmax; gno++) {
        if (get_graph_world(gno, &w) == RETURN_SUCCESS) {
            switch (type) {
            case GSCROLL_LEFT:    
            case GSCROLL_RIGHT:    
                if (islogx(gno) == TRUE) {
                    errmsg("Scrolling of LOG axes is not implemented");
                    return RETURN_FAILURE;
                }
                dwc = grace->rt->scrollper * (w.xg2 - w.xg1);
                break;
            case GSCROLL_DOWN:    
            case GSCROLL_UP:    
                if (islogy(gno) == TRUE) {
                    errmsg("Scrolling of LOG axes is not implemented");
                    return RETURN_FAILURE;
                }
                dwc = grace->rt->scrollper * (w.yg2 - w.yg1);
                break;
            }
            
            switch (type) {
            case GSCROLL_LEFT:    
                w.xg1 -= dwc;
                w.xg2 -= dwc;
                break;
            case GSCROLL_RIGHT:    
                w.xg1 += dwc;
                w.xg2 += dwc;
                break;
            case GSCROLL_DOWN:    
                w.yg1 -= dwc;
                w.yg2 -= dwc;
                break;
            case GSCROLL_UP:    
                w.yg1 += dwc;
                w.yg2 += dwc;
                break;
            }
            set_graph_world(gno, w);
        }
    }
    
    return RETURN_SUCCESS;
}

int graph_zoom(int type)
{
    double dx, dy;
    world w;
    int gno, cg = get_cg(), gmin, gmax;

    if (grace->rt->scrolling_islinked) {
        gmin = 0;
        gmax = number_of_graphs() - 1;
    } else {
        gmin = cg;
        gmax = cg;
    }
    
    for (gno = gmin; gno <= gmax; gno++) {
        if (!islogx(gno) && !islogy(gno)) {
            if (get_graph_world(gno, &w) == RETURN_SUCCESS) {
                dx = grace->rt->shexper * (w.xg2 - w.xg1);
                dy = grace->rt->shexper * (w.yg2 - w.yg1);
                if (type == GZOOM_SHRINK) {
                    dx *= -1;
                    dy *= -1;
                }
 
                w.xg1 -= dx;
                w.xg2 += dx;
                w.yg1 -= dy;
                w.yg2 += dy;
 
                set_graph_world(gno, w);
            }
        } else {
            errmsg("Zooming is not implemented for LOG plots");
            return RETURN_FAILURE;
        }
    }
    
    return RETURN_SUCCESS;
}

/*
 *  Arrange graphs
 */
int arrange_graphs(int *graphs, int ngraphs,
                   int nrows, int ncols, int order, int snake,
                   double loff, double roff, double toff, double boff,
                   double vgap, double hgap,
                   int hpack, int vpack)
{
    int i, imax, j, jmax, iw, ih, ng, gno;
    double pw, ph, w, h;
    view v;

    if (hpack) {
        hgap = 0.0;
    }
    if (vpack) {
        vgap = 0.0;
    }
    if (ncols < 1 || nrows < 1) {
	errmsg("# of rows and columns must be > 0");
        return RETURN_FAILURE;
    }
    if (hgap < 0.0 || vgap < 0.0) {
	errmsg("hgap and vgap must be >= 0");
        return RETURN_FAILURE;
    }
    
    get_page_viewport(grace->rt->canvas, &pw, &ph);
    w = (pw - loff - roff)/(ncols + (ncols - 1)*hgap);
    h = (ph - toff - boff)/(nrows + (nrows - 1)*vgap);
    if (h <= 0.0 || w <= 0.0) {
	errmsg("Page offsets are too large");
        return RETURN_FAILURE;
    }
    
    ng = 0;
    if (order & GA_ORDER_HV_INV) {
        imax = ncols;
        jmax = nrows;
    } else {
        imax = nrows;
        jmax = ncols;
    }
    for (i = 0; i < imax && ng < ngraphs; i++) {
        for (j = 0; j < jmax && ng < ngraphs; j++) {
            gno = graphs[ng];
            set_graph_active(gno, TRUE);
            
            if (order & GA_ORDER_HV_INV) {
                iw = i;
                ih = j;
                if (snake && (iw % 2)) {
                    ih = nrows - ih - 1;
                }
            } else {
                iw = j;
                ih = i;
                if (snake && (ih % 2)) {
                    iw = ncols - iw - 1;
                }
            }
            if (order & GA_ORDER_H_INV) {
                iw = ncols - iw - 1;
            }
            /* viewport y coord goes bottom -> top ! */
            if (!(order & GA_ORDER_V_INV)) {
                ih = nrows - ih - 1;
            }
            
            v.xv1 = loff + iw*w*(1.0 + hgap);
            v.xv2 = v.xv1 + w;
            v.yv1 = boff + ih*h*(1.0 + vgap);
            v.yv2 = v.yv1 + h;
            set_graph_viewport(gno, v);
            
            if (hpack) {
                if (iw == 0) {
	            tickmarks *t = get_graph_tickmarks(gno, Y_AXIS);
	            if (!t) {
                        continue;
                    }
                    t->active = TRUE;
	            t->label_op = PLACEMENT_NORMAL;
	            t->t_op = PLACEMENT_NORMAL;
	            t->tl_op = PLACEMENT_NORMAL;
                } else {
                    activate_tick_labels(gno, Y_AXIS, FALSE);
                }
            }
            if (vpack) {
                if (ih == 0) {
	            tickmarks *t = get_graph_tickmarks(gno, X_AXIS);
	            if (!t) {
                        continue;
                    }
	            t->active = TRUE;
	            t->label_op = PLACEMENT_NORMAL;
	            t->t_op = PLACEMENT_NORMAL;
	            t->tl_op = PLACEMENT_NORMAL;
                } else {
                    activate_tick_labels(gno, X_AXIS, FALSE);
                }
            }
            
            ng++;
        }
    }
    return RETURN_SUCCESS;
}

int arrange_graphs_simple(int nrows, int ncols,
    int order, int snake, double offset, double hgap, double vgap)
{
    int *graphs, i, gno, ngraphs, ngraphs_old, retval;
    
    ngraphs = nrows*ncols;
    graphs = xmalloc(ngraphs*SIZEOF_INT);
    if (graphs == NULL) {
        return RETURN_FAILURE;
    }
    
    for (i = 0; i < ngraphs; i++) {
        graphs[i] = i;
    }
    
    ngraphs_old = number_of_graphs();
    for (gno = 0; gno < ngraphs_old; gno++) {
        if (gno >= ngraphs) {
            kill_graph(gno);
        }
    }
    
    retval = arrange_graphs(graphs, ngraphs, nrows, ncols, order, snake,
        offset, offset, offset, offset, vgap, hgap, FALSE, FALSE);
    
    xfree(graphs);
    
    return retval;
}

void move_legend(int gno, VVector shift)
{
    legend *l = get_graph_legend(gno);
    if (l) {
        switch (l->acorner) {
        case CORNER_LL:
            l->offset.x += shift.x;
            l->offset.y += shift.y;
            break;
        case CORNER_UL:
            l->offset.x += shift.x;
            l->offset.y -= shift.y;
            break;
        case CORNER_UR:
        default:
            l->offset.x -= shift.x;
            l->offset.y -= shift.y;
            break;
        case CORNER_LR:
            l->offset.x -= shift.x;
            l->offset.y += shift.y;
            break;
        }

        set_dirtystate();
    }
}

void rescale_viewport(Project *pr, double ext_x, double ext_y)
{
    graph *g;
    DObject *o;

    storage_rewind(pr->graphs);
    while (storage_get_data(pr->graphs, (void **) &g) == RETURN_SUCCESS) {
        g->v.xv1 *= ext_x;
        g->v.xv2 *= ext_x;
        g->v.yv1 *= ext_y;
        g->v.yv2 *= ext_y;
        
        g->l.offset.x *= ext_x;
        g->l.offset.y *= ext_y;
        
        /* TODO: tickmark offsets */
        
        storage_rewind(g->dobjects);
        while (storage_get_data(g->dobjects, (void **) &o) == RETURN_SUCCESS) {
            if (o->loctype == COORD_VIEW) {
                o->ap.x     *= ext_x;
                o->ap.y     *= ext_y;
                o->offset.x *= ext_x;
                o->offset.y *= ext_y;
            }
            if (storage_next(g->dobjects) != RETURN_SUCCESS) {
                break;
            }
        }
        
        if (storage_next(pr->graphs) != RETURN_SUCCESS) {
            break;
        }
    }
    
    set_dirtystate();
}

int set_page_dimensions(Grace *grace, int wpp, int hpp, int rescale)
{
    int i;
    Canvas *canvas = grace->rt->canvas;
    
    if (wpp <= 0 || hpp <= 0) {
        return RETURN_FAILURE;
    } else {
        int wpp_old, hpp_old;
        Project *pr = (Project *) grace->project->data;
	wpp_old = pr->page_wpp;
	hpp_old = pr->page_hpp;
        
        pr->page_wpp = wpp;
	pr->page_hpp = hpp;
        if (rescale) {
            if (hpp*wpp_old - wpp*hpp_old != 0) {
                /* aspect ratio changed */
                double ext_x, ext_y;
                double old_aspectr, new_aspectr;
                
                old_aspectr = (double) wpp_old/hpp_old;
                new_aspectr = (double) wpp/hpp;
                if (old_aspectr >= 1.0 && new_aspectr >= 1.0) {
                    ext_x = new_aspectr/old_aspectr;
                    ext_y = 1.0;
                } else if (old_aspectr <= 1.0 && new_aspectr <= 1.0) {
                    ext_x = 1.0;
                    ext_y = old_aspectr/new_aspectr;
                } else if (old_aspectr >= 1.0 && new_aspectr <= 1.0) {
                    ext_x = 1.0/old_aspectr;
                    ext_y = 1.0/new_aspectr;
                } else {
                    ext_x = new_aspectr;
                    ext_y = old_aspectr;
                }

                rescale_viewport(pr, ext_x, ext_y);
            } 
        }
        for (i = 0; i < canvas->ndevices; i++) {
            canvas->device_table[i]->pg.width =
                (unsigned long) (wpp*(canvas->device_table[i]->pg.dpi/72));
            canvas->device_table[i]->pg.height =
                (unsigned long) (hpp*(canvas->device_table[i]->pg.dpi/72));
        }
        return RETURN_SUCCESS;
    }
}


int overlay_graphs(int gsec, int gpri, int type)
{
    int i;
    tickmarks *tsec, *tpri;
    world wpri, wsec;
    view v;
    
    if (gsec == gpri) {
        return RETURN_FAILURE;
    }
    if (is_valid_gno(gpri) == FALSE || is_valid_gno(gsec) == FALSE) {
        return RETURN_FAILURE;
    }
    
    get_graph_viewport(gpri, &v);
    get_graph_world(gpri, &wpri);
    get_graph_world(gsec, &wsec);

    switch (type) {
    case GOVERLAY_SMART_AXES_XY:
        wsec = wpri;
	for (i = 0; i < MAXAXES; i++) {
	    tpri = get_graph_tickmarks(gpri, i);
	    tsec = get_graph_tickmarks(gsec, i);
            switch(i) {
            case X_AXIS:
            case Y_AXIS:
                tpri->active = TRUE;
	        tpri->label_op = PLACEMENT_NORMAL;
	        tpri->t_op = PLACEMENT_BOTH;
	        tpri->tl_op = PLACEMENT_NORMAL;

	        tsec->active = FALSE;
                break;
            default:
                /* don't touch alternative axes */
                break;
            }
	}
	break;
    case GOVERLAY_SMART_AXES_X:
        wsec.xg1 = wpri.xg1;
        wsec.xg2 = wpri.xg2;
	for (i = 0; i < MAXAXES; i++) {
	    tpri = get_graph_tickmarks(gpri, i);
	    tsec = get_graph_tickmarks(gsec, i);
	    switch(i) {
            case X_AXIS:
                tpri->active = TRUE;
	        tpri->label_op = PLACEMENT_NORMAL;
	        tpri->t_op = PLACEMENT_BOTH;
	        tpri->tl_op = PLACEMENT_NORMAL;

	        tsec->active = FALSE;
                break;
            case Y_AXIS:
	        tpri->active = TRUE;
	        tpri->label_op = PLACEMENT_NORMAL;
	        tpri->t_op = PLACEMENT_NORMAL;
	        tpri->tl_op = PLACEMENT_NORMAL;

                tsec->active = TRUE;
	        tsec->label_op = PLACEMENT_OPPOSITE;
	        tsec->t_op = PLACEMENT_OPPOSITE;
	        tsec->tl_op = PLACEMENT_OPPOSITE;
                break;
            default:
                /* don't touch alternative axes */
                break;
            }
	}
	break;
    case GOVERLAY_SMART_AXES_Y:
        wsec.yg1 = wpri.yg1;
        wsec.yg2 = wpri.yg2;
	for (i = 0; i < MAXAXES; i++) {
	    tpri = get_graph_tickmarks(gpri, i);
	    tsec = get_graph_tickmarks(gsec, i);
	    switch(i) {
            case X_AXIS:
	        tpri->active = TRUE;
	        tpri->label_op = PLACEMENT_NORMAL;
	        tpri->t_op = PLACEMENT_NORMAL;
	        tpri->tl_op = PLACEMENT_NORMAL;

                tsec->active = TRUE;
	        tsec->label_op = PLACEMENT_OPPOSITE;
	        tsec->t_op = PLACEMENT_OPPOSITE;
	        tsec->tl_op = PLACEMENT_OPPOSITE;
                break;
            case Y_AXIS:
                tpri->active = TRUE;
	        tpri->label_op = PLACEMENT_NORMAL;
	        tpri->t_op = PLACEMENT_BOTH;
	        tpri->tl_op = PLACEMENT_NORMAL;

	        tsec->active = FALSE;
                break;
            default:
                /* don't touch alternative axes */
                break;
            }
	}
	break;
    case GOVERLAY_SMART_AXES_NONE:
	for (i = 0; i < MAXAXES; i++) {
	    tpri = get_graph_tickmarks(gpri, i);
	    tsec = get_graph_tickmarks(gsec, i);
	    switch(i) {
            case X_AXIS:
            case Y_AXIS:
	        tpri->active = TRUE;
	        tpri->label_op = PLACEMENT_NORMAL;
	        tpri->t_op = PLACEMENT_NORMAL;
	        tpri->tl_op = PLACEMENT_NORMAL;

                tsec->active = TRUE;
	        tsec->label_op = PLACEMENT_OPPOSITE;
	        tsec->t_op = PLACEMENT_OPPOSITE;
	        tsec->tl_op = PLACEMENT_OPPOSITE;
                break;
            default:
                /* don't touch alternative axes */
                break;
            }
	}
	break;
    default:
        break;
    }
    
    /* set identical viewports */
    set_graph_viewport(gsec, v);
    
    /* update world coords */
    set_graph_world(gsec, wsec);

    return RETURN_SUCCESS;
}
