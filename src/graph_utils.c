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
 *
 * utilities for graphs
 *
 */

#include <config.h>

#include <string.h>

#include "grace.h"
#include "parser.h"
#include "protos.h"

static int graph_count_hook(Quark *q, void *udata, QTraverseClosure *closure)
{
    int *ngraphs = (int *) udata;
    
    if (q->fid == QFlavorGraph) {
        (*ngraphs)++;
    }
    
    return TRUE;
}

int number_of_graphs(Quark *project)
{
    int ngraphs = 0;
    
    quark_traverse(project, graph_count_hook, &ngraphs);
    
    return ngraphs;
}

Quark *graph_get_current(const Quark *project)
{
    if (project) {
        Project *pr = project_get_data(project);
        return pr->cg;
    } else {
        return NULL;
    }
}

Quark *graph_next(Quark *project)
{
    Quark *f, *g;
    
    f = frame_new(project);
    g = graph_new(f);
    if (g && number_of_graphs(project) == 1) {
        Project *pr = project_get_data(project);
        pr->cg = g;
    }
    return g;
}

int select_graph(Quark *gr)
{
    if (graph_is_active(gr)) {
        Project *pr = project_get_data(get_parent_project(gr));
        if (pr) {
            set_parser_gno(gr);
            pr->cg = gr;

            return RETURN_SUCCESS;
        } else {
            return RETURN_FAILURE;
        }
    } else {
        return RETURN_FAILURE;
    }
}

static int set_count_hook(Quark *q, void *udata, QTraverseClosure *closure)
{
    int *nsets = (int *) udata;
    
    if (q->fid == QFlavorSet) {
        (*nsets)++;
    }
    
    return TRUE;
}

int number_of_sets(Quark *q)
{
    int nsets = 0;
    
    quark_traverse(q, set_count_hook, &nsets);
    
    return nsets;
}

typedef struct {
    int nsets;
    Quark **sets;
} set_hook_t;

static int set_hook(Quark *q, void *udata, QTraverseClosure *closure)
{
    set_hook_t *p = (set_hook_t *) udata;
    
    if (q->fid == QFlavorSet) {
        p->sets[p->nsets] = q;
        p->nsets++;
    }
    
    return TRUE;
}

int get_descendant_sets(Quark *q, Quark ***sets)
{
    set_hook_t p;
    
    if (q) {
        p.nsets = 0;
        p.sets  = xmalloc(number_of_sets(q)*SIZEOF_VOID_P);

        if (p.sets) {
            quark_traverse(q, set_hook, &p);
        }
        
        *sets = p.sets;

        return p.nsets;
    } else {
        return 0;
    }
}

int graph_get_viewport(Quark *gr, view *v)
{
    if (gr) {
        Quark *fr = get_parent_frame(gr);
        
        frame_get_view(fr, v);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int is_refpoint_active(Quark *gr)
{
    graph *g = graph_get_data(gr);
    if (g) {
        return g->locator.pointset;
    } else {
        return FALSE;
    }
}


int is_axis_active(tickmarks *t)
{
    if (t) {
        return t->active;
    } else {
        return FALSE;
    }
}

int is_zero_axis(tickmarks *t)
{
    if (t) {
        return t->zero;
    } else {
        return FALSE;
    }
}


int is_log_axis(const Quark *q)
{
    Quark *gr = get_parent_graph(q);
    if ((axis_is_x(q) && islogx(gr)) ||
        (axis_is_y(q) && islogy(gr))) {
        return TRUE;
    } else {
        return FALSE;
    }
}

int is_logit_axis(const Quark *q)
{
    Quark *gr = get_parent_graph(q);
    if ((axis_is_x(q) && islogitx(gr)) ||
        (axis_is_y(q) && islogity(gr))) {
        return TRUE;
    } else {
        return FALSE;
    }
}

int islogx(Quark *gr)
{
    if (graph_get_xscale(gr) == SCALE_LOG) {
        return TRUE;
    } else {
        return FALSE;
    }
}

int islogy(Quark *gr)
{
    if (graph_get_yscale(gr) == SCALE_LOG) {
        return TRUE;
    } else {
        return FALSE;
    }
}

int islogitx(Quark *gr)
{
    if (graph_get_xscale(gr) == SCALE_LOGIT) {
        return TRUE;
    } else {
        return FALSE;
    }
}

int islogity(Quark *gr)
{
    if (graph_get_yscale(gr) == SCALE_LOGIT) {
        return TRUE;
    } else {
        return FALSE;
    }
}


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


/* The following routines determine default axis range and tickmarks */

static void autorange_bysets(Quark **sets, int nsets, int autos_type);

static int autotick_hook(Quark *q, void *udata, QTraverseClosure *closure)
{
    int *amask = (int *) udata;
    
    switch (q->fid) {
    case QFlavorAxis:
        closure->descend = FALSE;
        if (((*amask & AXIS_MASK_X) && axis_is_x(q)) ||
            ((*amask & AXIS_MASK_Y) && axis_is_y(q))) {
            axis_autotick(q);
        }
        break;
    }
    
    return TRUE;
}

void autotick_graph_axes(Quark *q, int amask)
{
    quark_traverse(q, autotick_hook, &amask);
}

void autoscale_bysets(Quark **sets, int nsets, int autos_type)
{
    Quark *gr;
    
    if (nsets <= 0) {
        return;
    }
    
    gr = get_parent_graph(sets[0]);
    
    autorange_bysets(sets, nsets, autos_type);
    autotick_graph_axes(gr, autos_type);
}

int autoscale_graph(Quark *gr, int autos_type)
{
    int nsets;
    Quark **sets;
    nsets = get_descendant_sets(gr, &sets);
    if (nsets) {
        autoscale_bysets(sets, nsets, autos_type);
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

static void autorange_bysets(Quark **sets, int nsets, int autos_type)
{
    Quark *gr;
    world w;
    double xmax, xmin, ymax, ymin;
    int scale;

    if (autos_type == AUTOSCALE_NONE || nsets <= 0) {
        return;
    }
    
    gr = get_parent_graph(sets[0]);
    
    graph_get_world(gr, &w);
    
    if (graph_get_type(gr) == GRAPH_SMITH) {
        if (autos_type == AUTOSCALE_X || autos_type == AUTOSCALE_XY) {
            w.xg1 = -1.0;
            w.yg1 = -1.0;
        }
        if (autos_type == AUTOSCALE_Y || autos_type == AUTOSCALE_XY) {
            w.xg2 = 1.0;
            w.yg2 = 1.0;
	}
        graph_set_world(gr, &w);
        return;
    }

    xmin = w.xg1;
    xmax = w.xg2;
    ymin = w.yg1;
    ymax = w.yg2;
    if (autos_type == AUTOSCALE_XY) {
        getsetminmax(sets, nsets, &xmin, &xmax, &ymin, &ymax);
    } else if (autos_type == AUTOSCALE_X) {
        getsetminmax_c(sets, nsets, &xmin, &xmax, &ymin, &ymax, 2);
    } else if (autos_type == AUTOSCALE_Y) {
        getsetminmax_c(sets, nsets, &xmin, &xmax, &ymin, &ymax, 1);
    }

    if (autos_type == AUTOSCALE_X || autos_type == AUTOSCALE_XY) {
        scale = graph_get_xscale(gr);
        round_axis_limits(&xmin, &xmax, scale);
        w.xg1 = xmin;
        w.xg2 = xmax;
    }

    if (autos_type == AUTOSCALE_Y || autos_type == AUTOSCALE_XY) {
        scale = graph_get_yscale(gr);
        round_axis_limits(&ymin, &ymax, scale);
        w.yg1 = ymin;
        w.yg2 = ymax;
    }

    graph_set_world(gr, &w);
}

/*
 * pan through world coordinates
 */
int graph_scroll(Quark *gr, int type)
{
    RunTime *rt = rt_from_quark(gr);
    world w;
    double xmax, xmin, ymax, ymin;
    double dwc;

    if (graph_get_world(gr, &w) == RETURN_SUCCESS) {
	if (islogx(gr) == TRUE) {
	    xmin = log10(w.xg1);
	    xmax = log10(w.xg2);
	} else {
	    xmin = w.xg1;
	    xmax = w.xg2;
	}

	if (islogy(gr) == TRUE) {
	    ymin = log10(w.yg1);
	    ymax = log10(w.yg2);
	} else {
	    ymin = w.yg1;
	    ymax = w.yg2;
	}

	dwc = 1.0;
        switch (type) {
        case GSCROLL_LEFT:
	    dwc = -1.0;
	case GSCROLL_RIGHT:    
            dwc *= rt->scrollper * (xmax - xmin);
            xmin += dwc;
            xmax += dwc;
            break;
        case GSCROLL_DOWN:
	    dwc = -1.0;
	case GSCROLL_UP:    
            dwc *= rt->scrollper * (ymax - ymin);
            ymin += dwc;
            ymax += dwc;
            break;
        }

	if (islogx(gr) == TRUE) {
	    w.xg1 = pow(10.0, xmin);
	    w.xg2 = pow(10.0, xmax);
	} else {
	    w.xg1 = xmin;
	    w.xg2 = xmax;
	}

	if (islogy(gr) == TRUE) {
	    w.yg1 = pow(10.0, ymin);
	    w.yg2 = pow(10.0, ymax);
	} else {
	    w.yg1 = ymin;
	    w.yg2 = ymax;
	}
       
        graph_set_world(gr, &w);
        
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int graph_zoom(Quark *gr, int type)
{
    RunTime *rt = rt_from_quark(gr);
    double dx, dy;
    double xmax, xmin, ymax, ymin;
    world w;

    if (graph_get_world(gr, &w) == RETURN_SUCCESS) {

	if (islogx(gr) == TRUE) {
	    xmin = log10(w.xg1);
	    xmax = log10(w.xg2);
	} else {
	    xmin = w.xg1;
	    xmax = w.xg2;
	}

	if (islogy(gr) == TRUE) {
	    ymin = log10(w.yg1);
	    ymax = log10(w.yg2);
	} else {
	    ymin = w.yg1;
	    ymax = w.yg2;
	}

	dx = rt->shexper * (xmax - xmin);
	dy = rt->shexper * (ymax - ymin);
	if (type == GZOOM_SHRINK) {
	    dx *= -1;
	    dy *= -1;
	}

	xmin -= dx;
	xmax += dx;
	ymin -= dy;
	ymax += dy;

	if (islogx(gr) == TRUE) {
	    w.xg1 = pow(10.0, xmin);
	    w.xg2 = pow(10.0, xmax);
	} else {
	    w.xg1 = xmin;
	    w.xg2 = xmax;
	}

	if (islogy(gr) == TRUE) {
	    w.yg1 = pow(10.0, ymin);
	    w.yg2 = pow(10.0, ymax);
	} else {
	    w.yg1 = ymin;
	    w.yg2 = ymax;
	}

        graph_set_world(gr, &w);
        
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

/*
 *  Arrange graphs
 */
int arrange_graphs(Quark **graphs, int ngraphs,
                   int nrows, int ncols, int order, int snake,
                   double loff, double roff, double toff, double boff,
                   double vgap, double hgap,
                   int hpack, int vpack)
{
    int i, imax, j, jmax, iw, ih, ng;
    double pw, ph, w, h;
    view v;
    Quark *gr;
    RunTime *rt;

    if (!graphs) {
        return RETURN_FAILURE;
    }
    gr = graphs[0];
    
    rt = rt_from_quark(gr);
    if (!rt) {
        return RETURN_FAILURE;
    }
    
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
    
    get_page_viewport(rt->canvas, &pw, &ph);
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
            gr = graphs[ng];
            
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
#if 0
            graph_set_viewport(gr, &v);
            
            if (hpack) {
	        tickmarks *t = graph_get_tickmarks(gr, Y_AXIS);
                if (iw == 0) {
	            if (!t) {
                        continue;
                    }
                    t->active = TRUE;
	            t->label_op = PLACEMENT_NORMAL;
	            t->t_op = PLACEMENT_NORMAL;
	            t->tl_op = PLACEMENT_NORMAL;
                } else {
                    activate_tick_labels(t, FALSE);
                }
            }
            if (vpack) {
	        tickmarks *t = graph_get_tickmarks(gr, X_AXIS);
                if (ih == 0) {
	            if (!t) {
                        continue;
                    }
	            t->active = TRUE;
	            t->label_op = PLACEMENT_NORMAL;
	            t->t_op = PLACEMENT_NORMAL;
	            t->tl_op = PLACEMENT_NORMAL;
                } else {
                    activate_tick_labels(t, FALSE);
                }
            }
#endif            
            ng++;
        }
    }
    return RETURN_SUCCESS;
}

void move_legend(Quark *fr, const VVector *shift)
{
    legend *l = frame_get_legend(fr);
    if (l) {
        switch (l->acorner) {
        case CORNER_LL:
            l->offset.x += shift->x;
            l->offset.y += shift->y;
            break;
        case CORNER_UL:
            l->offset.x += shift->x;
            l->offset.y -= shift->y;
            break;
        case CORNER_UR:
        default:
            l->offset.x -= shift->x;
            l->offset.y -= shift->y;
            break;
        case CORNER_LR:
            l->offset.x -= shift->x;
            l->offset.y += shift->y;
            break;
        }

        quark_dirtystate_set(fr, TRUE);
    }
}

typedef struct {
    double x, y;
} ext_xy_t;

static int hook(Quark *q, void *udata, QTraverseClosure *closure)
{
    frame *f;
    view v;
    DObject *o;
    ext_xy_t *ext_xy = (ext_xy_t *) udata;
    
    switch (q->fid) {
    case QFlavorFrame:
        f = frame_get_data(q);
        frame_get_view(q, &v);
        v.xv1 *= ext_xy->x;
        v.xv2 *= ext_xy->x;
        v.yv1 *= ext_xy->y;
        v.yv2 *= ext_xy->y;
        frame_set_view(q, &v);
        
        f->l.offset.x *= ext_xy->x;
        f->l.offset.y *= ext_xy->y;
        
        /* TODO: tickmark offsets */
        quark_dirtystate_set(q, TRUE);
        break;
    case QFlavorDObject:
        o = object_get_data(q);
        if (object_get_loctype(q) == COORD_VIEW) {
            o->ap.x     *= ext_xy->x;
            o->ap.y     *= ext_xy->y;
            o->offset.x *= ext_xy->x;
            o->offset.y *= ext_xy->y;
            
            quark_dirtystate_set(q, TRUE);
        }
        break;
    }
    
    return TRUE;
}

void rescale_viewport(Quark *project, double ext_x, double ext_y)
{
    ext_xy_t ext_xy;
    ext_xy.x = ext_x;
    ext_xy.y = ext_y;

    quark_traverse(project, hook, &ext_xy);
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
