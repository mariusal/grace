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
 * Graph stuff
 *
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defines.h"
#include "utils.h"
#include "grace/canvas.h"
#include "graphs.h"
#include "graphutils.h"
#include "objutils.h"
#include "parser.h"

#include "protos.h"

static int graph_count_hook(Quark *q,
    void *udata, QTraverseClosure *closure)
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

graph *graph_get_data(const Quark *q)
{
    if (q && q->fid == QFlavorGraph) {
        return (graph *) q->data;
    } else {
        return NULL;
    }
}

static void set_default_graph(graph *g)
{    
    static const world d_w = {0.0, 1.0, 0.0, 1.0};

    g->active = TRUE;
    g->type = GRAPH_XY;
    g->xinvert = FALSE;
    g->yinvert = FALSE;
    g->xyflip = FALSE;
    g->stacked = FALSE;
    g->bargap = 0.0;
    g->znorm  = 1.0;
    g->xscale = SCALE_NORMAL;
    g->yscale = SCALE_NORMAL;
    g->locator.dsx = g->locator.dsy = 0.0;      /* locator props */
    g->locator.pointset = FALSE;
    g->locator.pt_type = 0;
    g->locator.fx = FORMAT_GENERAL;
    g->locator.fy = FORMAT_GENERAL;
    g->locator.px = 6;
    g->locator.py = 6;
    memcpy(&g->w, &d_w, sizeof(world));
}

graph *graph_data_new(void)
{
    graph *g;
    
    g = xmalloc(sizeof(graph));
    if (g) {
        memset(g, 0, sizeof(graph));
        set_default_graph(g);
    }
    return g;
}

void graph_data_free(graph *g)
{
    xfree(g);
}

graph *graph_data_copy(graph *g)
{
    graph *g_new;
    
    if (!g) {
        return NULL;
    }
    
    g_new = xmalloc(sizeof(graph));
    if (!g_new) {
        return NULL;
    }
    
    memcpy(g_new, g, sizeof(graph));
    
    return g_new;
}

Quark *get_parent_project(const Quark *q)
{
    Quark *p = (Quark *) q;
    
    while (p) {
        if (p->fid == QFlavorProject) {
            return p;
        }
        p = quark_parent_get(p);
    }
    
    return NULL;
}

Quark *get_parent_graph(const Quark *child)
{
    Quark *p = (Quark *) child;
    
    while (p) {
        p = quark_parent_get(p);
        if (p->fid == QFlavorGraph) {
            return p;
        }
    }
    
    return NULL;
}

static int hook(Quark *q, void *udata, QTraverseClosure *closure)
{
    if (q->fid == QFlavorGraph) {
        closure->descend = FALSE;
        
        if (!is_graph_hidden(q)) {
            Project *project = (Project *) udata;

            if (project->cg != q) {
                project->cg = q;
                return FALSE;
            }
        }
    } else
    if (q->fid == QFlavorFrame && !frame_is_active(q)) {
        closure->descend = FALSE;
    }

    return TRUE;
}

static int graph_free_cb(Quark *gr, int etype, void *data)
{
    if (etype == QUARK_ETYPE_DELETE) {
        Quark *pr = get_parent_project(gr);
        Project *project = project_get_data(pr);
        if (project->cg == gr) {
            quark_traverse(pr, hook, project);
            if (project->cg == gr) {
                project->cg = NULL;
            }
        }
    }
    return RETURN_SUCCESS;
}

Quark *graph_new(Quark *q)
{
    Quark *g;
    
    if (q->fid != QFlavorFrame) {
        return NULL;
    }
    
    g = quark_new(q, QFlavorGraph);
    if (g) {
        Project *pr = project_get_data(get_parent_project(q));
        if (pr && pr->cg == NULL) {
            pr->cg = g;
        }
        quark_cb_set(g, graph_free_cb, NULL);
    }
    return g;
}

Quark *graph_next(Quark *project)
{
    Quark *f, *g;
    
    f = frame_new(project);
    g = graph_new(f);
    if (number_of_graphs(project) == 1) {
        Project *pr = project_get_data(project);
        pr->cg = g;
    }
    return g;
}

/**** Tickmarks ****/

static void set_default_ticks(Quark *q)
{
    int i;

    tickmarks *t = axis_get_data(q);
    defaults grdefaults;
    RunTime *rt = rt_from_quark(q);
    
    if (!t || !rt) {
        return;
    }
    
    grdefaults = rt->grdefaults;
    
    t->active = TRUE;
    t->type = AXIS_TYPE_X;
    t->zero = FALSE;
    t->tl_flag = TRUE;
    t->t_flag = TRUE;
    
    set_default_string(&t->label);
    t->label.offset.x = 0.0;
    t->label.offset.y = 0.08;
    
    t->tmajor = 0.5;
    t->nminor = 1;
    t->t_round = TRUE;
    t->offsx = 0.0;
    t->offsy = 0.0;
    t->label_layout = LAYOUT_PARALLEL;
    t->label_place = TYPE_AUTO;
    t->label_op = PLACEMENT_NORMAL;
    t->tl_format = FORMAT_GENERAL;
    t->tl_prec = 5;
    t->tl_formula = NULL;
    t->tl_angle = 0;
    t->tl_skip = 0;
    t->tl_staggered = 0;
    t->tl_starttype = TYPE_AUTO;
    t->tl_stoptype = TYPE_AUTO;
    t->tl_start = 0.0;
    t->tl_stop = 0.0;
    t->tl_op = PLACEMENT_NORMAL;
    t->tl_gaptype = TYPE_AUTO;
    t->tl_gap.x = 0.0;
    t->tl_gap.y = 0.01;
    t->tl_font = grdefaults.font;
    t->tl_charsize = 1.0;
    t->tl_color = grdefaults.line.pen.color;
    t->tl_appstr[0] = 0;
    t->tl_prestr[0] = 0;
    t->t_spec = TICKS_SPEC_NONE;
    t->t_autonum = 6;
    t->t_inout = TICKS_IN;
    t->t_op = PLACEMENT_BOTH;
    t->props.size = grdefaults.charsize;
    t->mprops.size = grdefaults.charsize / 2;
    t->t_drawbar = TRUE;
    t->t_drawbarcolor = grdefaults.line.pen.color;
    t->t_drawbarlines = grdefaults.line.style;
    t->t_drawbarlinew = grdefaults.line.width;
    t->props.gridflag = FALSE;
    t->mprops.gridflag = FALSE;
    t->props.color = grdefaults.line.pen.color;
    t->props.lines = grdefaults.line.style;
    t->props.linew = grdefaults.line.width;
    t->mprops.color = grdefaults.line.pen.color;
    t->mprops.lines = grdefaults.line.style;
    t->mprops.linew = grdefaults.line.width;
    t->nticks = 0;
    for (i = 0; i < MAX_TICKS; i++) {
        t->tloc[i].wtpos = 0.0;
        t->tloc[i].label = NULL;
    }
}

Quark *axis_new(Quark *q)
{
    Quark *a; 
    a = quark_new(q, QFlavorAxis);
    set_default_ticks(a);
    autotick_axis(a);
    return a;
}

tickmarks *axis_data_new(void)
{
    tickmarks *retval;
    
    retval = xmalloc(sizeof(tickmarks));
    if (retval != NULL) {
        memset(retval, 0, sizeof(tickmarks));
    }
    return retval;
}

tickmarks *axis_data_copy(tickmarks *t)
{
    tickmarks *retval;
    int i;
    
    if (t == NULL) {
        return NULL;
    } else {
        retval = axis_data_new();
        if (retval != NULL) {
            memcpy(retval, t, sizeof(tickmarks));
	    retval->label.s = copy_string(NULL, t->label.s);
	    retval->tl_formula = copy_string(NULL, t->tl_formula);
            for (i = 0; i < MAX_TICKS; i++) {
                retval->tloc[i].label = copy_string(NULL, t->tloc[i].label);
            }
        }
        return retval;
    }
}

void axis_data_free(tickmarks *t)
{
    if (t) {
        int i;

        xfree(t->label.s);
        xfree(t->tl_formula);
        
        for (i = 0; i < MAX_TICKS; i++) {
            xfree(t->tloc[i].label);
        }
        
        xfree(t);
    }
}

tickmarks *axis_get_data(const Quark *q)
{
    if (q && q->fid == QFlavorAxis) {
        return (tickmarks *) q->data;
    } else {
        return NULL;
    }
}

int axis_set_type(Quark *q, int type)
{
    tickmarks *t = axis_get_data(q);
    if (t) {
        if (t->type != type) {
            t->type = type;
            quark_dirtystate_set(q, TRUE);
        }
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int axis_set_active(Quark *q, int flag)
{
    tickmarks *t = axis_get_data(q);
    if (t) {
        if (t->active != flag) {
            t->active = flag;
            quark_dirtystate_set(q, TRUE);
        }
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int axis_is_x(const Quark *q)
{
    tickmarks *t = axis_get_data(q);
    return (t && t->type == AXIS_TYPE_X);
}

int axis_is_y(const Quark *q)
{
    tickmarks *t = axis_get_data(q);
    return (t && t->type == AXIS_TYPE_Y);
}


int select_graph(Quark *gr)
{
    graph *g = graph_get_data(gr);
    int ctrans_type, xyfixed;
    view v;
    
    if (!g) {
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
    
    get_graph_viewport(gr, &v);
    if (definewindow(&g->w, &v, ctrans_type, xyfixed,
            g->xscale,  g->yscale,
            g->xinvert, g->yinvert) == RETURN_SUCCESS) {
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

int set_graph_type(Quark *gr, int gtype)
{
    if (gr) {
        graph *g = graph_get_data(gr);
        if (g->type == gtype) {
            return RETURN_SUCCESS;
        }
        
        switch (gtype) {
        case GRAPH_XY:
        case GRAPH_CHART:
        case GRAPH_FIXED:
        case GRAPH_PIE:
            break;
        case GRAPH_POLAR:
	    g->w.xg1 = 0.0;
	    g->w.xg2 = 2*M_PI;
	    g->w.yg1 = 0.0;
	    g->w.yg2 = 1.0;
            break;
        case GRAPH_SMITH:
	    g->w.xg1 = -1.0;
	    g->w.xg2 =  1.0;
	    g->w.yg1 = -1.0;
	    g->w.yg2 =  1.0;
            break;
        default:
            errmsg("Internal error in set_graph_type()");
            return RETURN_FAILURE;
        }
        g->type = gtype;
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
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

Symbol *symbol_new()
{
    Symbol *retval;
    retval = xmalloc(sizeof(Symbol));
    if (retval) {
        memset(retval, 0, sizeof(Symbol));
    }
    return retval;
}

void symbol_free(Symbol *sym)
{
    xfree(sym);
}

SetLine *setline_new()
{
    SetLine *retval;
    retval = xmalloc(sizeof(SetLine));
    if (retval) {
        memset(retval, 0, sizeof(SetLine));
    }
    return retval;
}

void setline_free(SetLine *sl)
{
    xfree(sl);
}

BarLine *barline_new(void)
{
    BarLine *retval;
    retval = xmalloc(sizeof(BarLine));
    if (retval) {
        memset(retval, 0, sizeof(BarLine));
    }
    return retval;
}

RiserLine *riserline_new(void)
{
    RiserLine *retval;
    retval = xmalloc(sizeof(RiserLine));
    if (retval) {
        memset(retval, 0, sizeof(RiserLine));
    }
    return retval;
}

int is_set_hidden(Quark *pset)
{
    if (pset) {
        set *p = set_get_data(pset);
        return !(p->active);
    } else {
        return FALSE;
    }
}

int set_set_hidden(Quark *pset, int flag)
{
    if (pset) {
        set *p = set_get_data(pset);
        p->active = !flag;
        quark_dirtystate_set(pset, TRUE);
        return RETURN_SUCCESS;
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

Quark *get_parent_frame(Quark *q)
{
    Quark *p = (Quark *) q;
    
    while (p) {
        p = quark_parent_get(p);
        if (p->fid == QFlavorFrame) {
            return p;
        }
    }
    
    return NULL;
}

GLocator *get_graph_locator(Quark *gr)
{
    graph *g = graph_get_data(gr);
    if (g) {
        return &g->locator;
    } else {
        return NULL;
    }
}

int get_graph_world(Quark *gr, world *w)
{
    graph *g = graph_get_data(gr);
    if (g) {
        memcpy(w, &g->w, sizeof(world));
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int get_graph_viewport(Quark *gr, view *v)
{
    if (gr) {
        view *vv;
        Quark *fr = get_parent_frame(gr);
        
        vv = frame_get_view(fr);
        memcpy(v, vv, sizeof(view));
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

void set_graph_locator(Quark *gr, const GLocator *locator)
{
    graph *g = graph_get_data(gr);
    if (g) {

        memcpy(&g->locator, locator, sizeof(GLocator));
        quark_dirtystate_set(gr, TRUE);
    }
}

void set_graph_world(Quark *gr, const world *w)
{
    graph *g = graph_get_data(gr);
    if (g) {

        g->w = *w;
        quark_dirtystate_set(gr, TRUE);
    }
}

void set_graph_xyflip(Quark *gr, int xyflip)
{
    graph *g = graph_get_data(gr);
    if (g) {

        g->xyflip = xyflip;

        quark_dirtystate_set(gr, TRUE);
    }
}

int get_graph_xyflip(Quark *gr)
{
    graph *g = graph_get_data(gr);
    if (g) {

        return g->xyflip;
    } else {
        return FALSE;
    }
}

int is_graph_hidden(Quark *gr)
{
    graph *g = graph_get_data(gr);
    if (g) {
        return !(g->active);
    } else {
        return TRUE;
    }
}

int set_graph_hidden(Quark *gr, int flag)
{
    graph *g = graph_get_data(gr);
    if (g) {
        g->active = !flag;
        quark_dirtystate_set(gr, TRUE);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int set_graph_stacked(Quark *gr, int flag)
{
    graph *g = graph_get_data(gr);
    if (g) {
        g->stacked = flag;
        quark_dirtystate_set(gr, TRUE);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int is_graph_stacked(Quark *gr)
{
    graph *g = graph_get_data(gr);
    if (g) {
        return g->stacked;
    } else {
        return FALSE;
    }
}

double get_graph_bargap(Quark *gr)
{
    graph *g = graph_get_data(gr);
    if (g) {
        return g->bargap;
    } else {
        return 0.0;
    }
}

int set_graph_bargap(Quark *gr, double bargap)
{
    graph *g = graph_get_data(gr);
    if (g) {
        g->bargap = bargap;
        quark_dirtystate_set(gr, TRUE);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int get_graph_type(Quark *gr)
{
    graph *g = graph_get_data(gr);
    if (g) {
        return g->type;
    } else {
        return -1;
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

int get_graph_xscale(Quark *gr)
{
    graph *g = graph_get_data(gr);
    if (g) {
        return g->xscale;
    } else {
        return -1;
    }
}

int get_graph_yscale(Quark *gr)
{
    graph *g = graph_get_data(gr);
    if (g) {
        return g->yscale;
    } else {
        return -1;
    }
}

int set_graph_xscale(Quark *gr, int scale)
{
    graph *g = graph_get_data(gr);
    if (g) {
        if (g->xscale != scale) {
            g->xscale = scale;
#if 0
            for (naxis = 0; naxis < MAXAXES; naxis++) {
                if (axis_is_x(naxis)) {
                    tickmarks *t;
                    t = get_graph_tickmarks(gr, naxis);
                    if (t) {
                        if (scale == SCALE_LOG) {
                            if (g->w.xg2 <= 0.0) {
                                g->w.xg2 = 10.0;
                            }
                            if (g->w.xg1 <= 0.0) {
                                g->w.xg1 = g->w.xg2/1.0e3;
                            }
                            t->tmajor = 10.0;
                            t->nminor = 9;
                        } else {
                            t->nminor = 1;
                        }
                    }
                }
            }
#endif
            quark_dirtystate_set(gr, TRUE);
        }
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int set_graph_yscale(Quark *gr, int scale)
{
    graph *g = graph_get_data(gr);
    if (g) {
        if (g->yscale != scale) {
            g->yscale = scale;
#if 0
            for (naxis = 0; naxis < MAXAXES; naxis++) {
                if (axis_is_y(naxis)) {
                    tickmarks *t;
                    t = get_graph_tickmarks(gr, naxis);
                    if (t) {
                        if (scale == SCALE_LOG) {
                            if (g->w.yg2 <= 0.0) {
                                g->w.yg2 = 10.0;
                            }
                            if (g->w.yg1 <= 0.0) {
                                g->w.yg1 = g->w.yg2/1.0e3;
                            }
                            t->tmajor = 10.0;
                            t->nminor = 9;
                        } else {
                            t->nminor = 1;
                        }
                    }
                }
            }
#endif
            quark_dirtystate_set(gr, TRUE);
        }
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int set_graph_znorm(Quark *gr, double norm)
{
    graph *g = graph_get_data(gr);
    if (g) {
        g->znorm = norm;
        quark_dirtystate_set(gr, TRUE);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

double get_graph_znorm(Quark *gr)
{
    graph *g = graph_get_data(gr);
    if (g) {
        return g->znorm;
    } else {
        return 0.0;
    }
}

int is_graph_xinvert(Quark *gr)
{
    graph *g = graph_get_data(gr);
    if (g) {
        return g->xinvert;
    } else {
        return FALSE;
    }
}

int is_graph_yinvert(Quark *gr)
{
    graph *g = graph_get_data(gr);
    if (g) {
        return g->yinvert;
    } else {
        return FALSE;
    }
}

int set_graph_xinvert(Quark *gr, int flag)
{
    graph *g = graph_get_data(gr);
    if (g) {
        g->xinvert = flag;
        quark_dirtystate_set(gr, TRUE);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int set_graph_yinvert(Quark *gr, int flag)
{
    graph *g = graph_get_data(gr);
    if (g) {
        g->yinvert = flag;
        quark_dirtystate_set(gr, TRUE);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int islogx(Quark *gr)
{
    graph *g = graph_get_data(gr);
    if (g) {
        return (g->xscale == SCALE_LOG);
    } else {
        return FALSE;
    }
}

int islogy(Quark *gr)
{
    graph *g = graph_get_data(gr);
    if (g) {
        return (g->yscale == SCALE_LOG);
    } else {
        return FALSE;
    }
}

int islogitx(Quark *gr)
{
    graph *g = graph_get_data(gr);
    if (g) {
        return (g->xscale == SCALE_LOGIT);
    } else {
        return FALSE;
    }
}

int islogity(Quark *gr)
{
    graph *g = graph_get_data(gr);
    if (g) {
        return (g->yscale == SCALE_LOGIT);
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

static int fcomp(const Quark *q1, const Quark *q2, void *udata)
{
    if (q1->fid == QFlavorDObject && q2->fid == QFlavorDObject) {
        DObject *o1 = object_get_data(q1), *o2 = object_get_data(q2);
        if (o1->type != o2->type) {
            return (o1->type - o2->type);
        } else {
            return strcmp(QIDSTR(q1), QIDSTR(q2));
        }
    } else
    if (q1->fid == QFlavorDObject) {
        return 1;
    } else
    if (q2->fid == QFlavorDObject) {
        return -1;
    } else
    if (q1->fid == QFlavorAxis) {
        tickmarks *t = axis_get_data(q1);
        if (t->props.gridflag || t->mprops.gridflag) {
            return -1;
        } else {
            return 1;
        }
    } else
    if (q2->fid == QFlavorAxis) {
        tickmarks *t = axis_get_data(q2);
        if (t->props.gridflag || t->mprops.gridflag) {
            return 1;
        } else {
            return -1;
        }
    } else
    if (q1->fid == q2->fid) {
        return strcmp(QIDSTR(q1), QIDSTR(q2));
    } else {
        return 0;
    }
}

static int project_postprocess_hook(Quark *q,
    void *udata, QTraverseClosure *closure)
{
    int version_id = *((int *) udata);
    RunTime *rt = rt_from_quark(q);
    Project *pr;
    frame *f;
    graph *g;
    tickmarks *t;
    set *s;
    int gtype;
    
    switch (q->fid) {
    case QFlavorProject:
        pr = project_get_data(q);
        
        if (version_id < 50200) {
            quark_sort_children(q, fcomp, NULL);
        }

        if (version_id < 40005) {
            set_page_dimensions(grace_from_quark(q), 792, 612, FALSE);
        }

        if (version_id < 50002) {
            pr->bgfill = TRUE;
        }

        if (version_id < 50003) {
            pr->two_digits_years = TRUE;
            pr->wrap_year = 1900;
        }

        if (version_id <= 40102) {
            double ext_x, ext_y;
#ifndef NONE_GUI
            set_pagelayout(PAGE_FIXED);
#endif
            get_page_viewport(rt->canvas, &ext_x, &ext_y);
            rescale_viewport(q, ext_x, ext_y);
        }
        break;
    case QFlavorFrame:
        f = frame_get_data(q);
        
	if (version_id <= 40102) {
            f->l.vgap -= 0.01;
        }
	if (version_id < 50200) {
            f->l.acorner = CORNER_UL;
        }

        break;
    case QFlavorGraph:
        if (version_id < 50200) {
            quark_sort_children(q, fcomp, NULL);
        }
        select_graph(q);

        break;
    case QFlavorAxis:
	t = axis_get_data(q);
        g = graph_get_data(get_parent_graph(q));

        if (version_id <= 40102) {
            if ((axis_is_x(q) && g->xscale == SCALE_LOG) ||
                (axis_is_y(q) && g->yscale == SCALE_LOG)) {
                t->tmajor = pow(10.0, t->tmajor);
            }

            /* TODO : world/view translation */
            t->offsx = 0.0;
            t->offsy = 0.0;
        }
	if (version_id < 50000) {
	    /* There was no label_op in Xmgr */
            t->label_op = t->tl_op;

            /* in xmgr, axis label placement was in x,y coordinates */
	    /* in Grace, it's parallel/perpendicular */
	    if (axis_is_y(q)) {
	        fswap(&t->label.offset.x,
                      &t->label.offset.y);
	    }
	    t->label.offset.y *= -1;
	}
	if (version_id >= 50000 && version_id < 50103) {
	    /* Autoplacement of axis labels wasn't implemented 
               in early versions of Grace */
            if (t->label_place == TYPE_AUTO) {
                t->label.offset.x = 0.0;
                t->label.offset.y = 0.08;
                t->label_place = TYPE_SPEC;
            }
        }
        if (version_id < 50105) {
            /* Starting with 5.1.5, X axis min & inverting is honored
               in pie charts */
            if (get_graph_type(q) == GRAPH_PIE) {
                world w;
                get_graph_world(q, &w);
                w.xg1 = 0.0;
                w.xg2 = 2*M_PI;
                set_graph_world(q, &w);
                set_graph_xinvert(q, FALSE);
            }
        }

        break;
    case QFlavorSet:
        s = set_get_data(q);
        gtype = get_graph_type(get_parent_graph(q));

        if (version_id < 50000) {
            switch (s->sym.type) {
            case SYM_NONE:
                break;
            case SYM_DOT_OBS:
                s->sym.type = SYM_CIRCLE;
                s->sym.size = 0.0;
                s->sym.line.style = 0;
                s->sym.fillpen.pattern = 1;
                break;
            default:
                s->sym.type--;
                break;
            }
        }
        if ((version_id < 40004 && gtype != GRAPH_CHART) ||
            s->sym.line.pen.color == -1) {
            s->sym.line.pen.color = s->line.line.pen.color;
        }
        if (version_id < 40200 || s->sym.fillpen.color == -1) {
            s->sym.fillpen.color = s->sym.line.pen.color;
        }

	if (version_id <= 40102 && gtype == GRAPH_CHART) {
            s->type       = SET_BAR;
            s->sym.line    = s->line.line;
            s->line.line.style = 0;

            s->sym.fillpen = s->line.fillpen;
            s->line.fillpen.pattern = 0;
        }
	if (version_id <= 40102 && s->type == SET_XYHILO) {
            s->sym.line.width = s->line.line.width;
        }
	if (version_id <= 50112 && s->type == SET_XYHILO) {
            s->avalue.active = FALSE;
        }
	if (version_id < 50100 && s->type == SET_BOXPLOT) {
            s->sym.line.width = s->line.line.width;
            s->sym.line.style = s->line.line.style;
            s->sym.size = 2.0;
            s->errbar.riser_linew = s->line.line.width;
            s->errbar.riser_lines = s->line.line.style;
            s->line.line.style = 0;
            s->errbar.barsize = 0.0;
        }
        if (version_id < 50003) {
            s->errbar.active = TRUE;
            s->errbar.pen.color = s->sym.line.pen.color;
            s->errbar.pen.pattern = 1;
            switch (s->errbar.ptype) {
            case PLACEMENT_NORMAL:
                s->errbar.ptype = PLACEMENT_OPPOSITE;
                break;
            case PLACEMENT_OPPOSITE:
                s->errbar.ptype = PLACEMENT_NORMAL;
                break;
            case PLACEMENT_BOTH:
                switch (s->type) {
                case SET_XYDXDX:
                case SET_XYDYDY:
                case SET_BARDYDY:
                    s->errbar.ptype = PLACEMENT_NORMAL;
                    break;
                }
                break;
            }
        }
        if (version_id < 50002) {
            s->errbar.barsize *= 2;
        }

        if (version_id < 50107) {
            /* Starting with 5.1.7, symskip is honored for all set types */
            switch (s->type) {
            case SET_BAR:
            case SET_BARDY:
            case SET_BARDYDY:
            case SET_XYHILO:
            case SET_XYR:
            case SET_XYVMAP:
            case SET_BOXPLOT:
                s->symskip = 0;
                break;
            }
        }
        
        break;
    case QFlavorDObject:
        if (version_id >= 40200 && version_id <= 50005 &&
            !compare_strings(q->idstr, "timestamp")) {
            /* BBox type justification was erroneously set */
            DObject *o = object_get_data(q);
            if (o->type == DO_STRING) {
                DOStringData *s = (DOStringData *) o->odata;
                s->just |= JUST_MIDDLE;
            }
        }

        if (version_id < 50200) {
            DObject *o = object_get_data(q);
            if (object_get_loctype(q) == COORD_WORLD) {
                WPoint wp;
                VPoint vp1, vp2;

                switch (o->type) {
                case DO_BOX:
                    {
                        DOBoxData *b = (DOBoxData *) o->odata;
                        wp.x = o->ap.x - b->width/2;
                        wp.y = o->ap.y - b->height/2;
                        vp1 = Wpoint2Vpoint(wp);
                        wp.x = o->ap.x + b->width/2;
                        wp.y = o->ap.y + b->height/2;
                        vp2 = Wpoint2Vpoint(wp);

                        b->width  = fabs(vp2.x - vp1.x);
                        b->height = fabs(vp2.y - vp1.y);
                    }
                    break;
                case DO_ARC:
                    {
                        DOArcData *a = (DOArcData *) o->odata;
                        wp.x = o->ap.x - a->width/2;
                        wp.y = o->ap.y - a->height/2;
                        vp1 = Wpoint2Vpoint(wp);
                        wp.x = o->ap.x + a->width/2;
                        wp.y = o->ap.y + a->height/2;
                        vp2 = Wpoint2Vpoint(wp);

                        a->width  = fabs(vp2.x - vp1.x);
                        a->height = fabs(vp2.y - vp1.y);
                    }
                    break;
                case DO_LINE:
                    {
                        DOLineData *l = (DOLineData *) o->odata;
                        wp.x = o->ap.x;
                        wp.y = o->ap.y;
                        vp1 = Wpoint2Vpoint(wp);
                        wp.x = o->ap.x +
                            l->length*cos(M_PI/180.0*o->angle);
                        wp.y = o->ap.y +
                            l->length*sin(M_PI/180.0*o->angle);
                        vp2 = Wpoint2Vpoint(wp);

                        l->length = hypot(vp2.x - vp1.x, vp2.y - vp1.y);
                        o->angle  = 180.0/M_PI*atan2(vp2.y - vp1.y,
                                                     vp2.x - vp1.x);
                    }
                    break;
                case DO_STRING:
                    break;
                case DO_NONE:
                    break;
                }
            }
        }

        break;
    }
    
    return TRUE;
}

void project_postprocess(Quark *project)
{
    Quark *gsave;
    int version_id = project_get_version_id(project);
    
    if (version_id >= bi_version_id()) {
        return;
    }
    
    gsave = graph_get_current(project);
    
    quark_traverse(project, project_postprocess_hook, &version_id);

    select_graph(gsave);
}
