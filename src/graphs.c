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

#include "globals.h"
#include "defines.h"
#include "utils.h"
#include "grace/canvas.h"
#include "graphs.h"
#include "graphutils.h"
#include "storage.h"
#include "objutils.h"
#include "parser.h"
#include "dict3.h"

#include "protos.h"

int number_of_graphs(const Quark *project)
{
    Project *pr = (Project *) project->data;
    return storage_count(pr->graphs);
}

Quark *graph_get_current(const Quark *project)
{
    if (project) {
        Project *pr = (Project *) project->data;
        return pr->cg;
    } else {
        return NULL;
    }
}

graph *graph_get_data(Quark *gr)
{
    if (gr) {
        return (graph *) gr->data;
    } else {
        return NULL;
    }
}

int is_valid_gno(Quark *gr)
{
    if (gr) {
        return TRUE;
    } else {
        return FALSE;
    }
}

static void wrap_set_free(void *data)
{
    quark_free((Quark *) data);
}

static void *wrap_set_copy(void *data)
{
    return (void *) quark_copy((Quark *) data);
}

static void wrap_object_free(void *data)
{
    object_free((DObject *) data);
}

static void *wrap_object_copy(void *data)
{
    return (void *) object_copy((DObject *) data);
}

static void set_default_graph(graph *g)
{    
    int i;
    
    g->hidden = FALSE;
    g->type = GRAPH_XY;
    g->xinvert = FALSE;
    g->yinvert = FALSE;
    g->xyflip = FALSE;
    g->stacked = FALSE;
    g->bargap = 0.0;
    g->znorm  = 1.0;
    g->xscale = SCALE_NORMAL;
    g->yscale = SCALE_NORMAL;
    g->ws_top = 1;
    g->ws[0].w.xg1=g->ws[0].w.xg2=g->ws[0].w.yg1=g->ws[0].w.yg2=0;
        g->curw = 0;
    g->locator.dsx = g->locator.dsy = 0.0;      /* locator props */
    g->locator.pointset = FALSE;
    g->locator.pt_type = 0;
    g->locator.fx = FORMAT_GENERAL;
    g->locator.fy = FORMAT_GENERAL;
    g->locator.px = 6;
    g->locator.py = 6;
    for (i = 0; i < MAXAXES; i++) {
        g->t[i] = new_graph_tickmarks();
        switch (i) {
        case X_AXIS:
        case Y_AXIS:
            g->t[i]->active = TRUE;
            break;
        case ZX_AXIS:
        case ZY_AXIS:
            g->t[i]->active = FALSE;
            break;
        }
    }
    set_default_framep(&g->f);
    set_default_world(&g->w);
    set_default_view(&g->v);
    set_default_legend(&g->l);
    set_default_string(&g->labs.title);
    g->labs.title.charsize = 1.5;
    set_default_string(&g->labs.stitle);
    g->labs.stitle.charsize = 1.0;
    g->sets = storage_new(wrap_set_free, wrap_set_copy, NULL);
    g->dobjects = storage_new(wrap_object_free, wrap_object_copy, NULL);
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
    if (g) {
        int j;
        
        xfree(g->labs.title.s);
        xfree(g->labs.stitle.s);
        
        for (j = 0; j < MAXAXES; j++) {
            free_graph_tickmarks(g->t[j]);
        }

        storage_free(g->sets);
        storage_free(g->dobjects);
        
        xfree(g);
    }
}

graph *graph_data_copy(graph *g)
{
    graph *g_new;
    int j;
    
    if (!g) {
        return NULL;
    }
    
    g_new = xmalloc(sizeof(graph));
    if (!g_new) {
        return NULL;
    }
    
    memcpy(g_new, g, sizeof(graph));

    /* duplicate allocatable storage */
    g_new->sets = storage_copy(g->sets);

    g_new->dobjects = storage_copy(g->dobjects);
    
    g_new->labs.title.s = copy_string(NULL, g->labs.title.s);
    g_new->labs.stitle.s = copy_string(NULL, g->labs.stitle.s);
    
    for (j = 0; j < MAXAXES; j++) {
	g_new->t[j] = copy_graph_tickmarks(g->t[j]);
    }
    
    if (!g_new->sets || !g_new->dobjects) {
        graph_data_free(g_new);
        return NULL;
    } else {
        return g_new;
    }
}

static int hook(unsigned int step, void *data, void *udata)
{
    Project *project = (Project *) udata;
    Quark *gr = (Quark *) data;
    
    Quark *cg = project->cg;
    
    if (cg != gr) {
        project->cg = gr;
        return FALSE;
    } else {
        return TRUE;
    }
}

static int graph_free_cb(Quark *gr, int etype, void *data)
{
    if (etype == QUARK_ETYPE_DELETE) {
        Quark *pr = gr->parent;
        Project *project = (Project *) pr->data;
        if (project->cg == gr) {
            storage_traverse(project->graphs, hook, project);
        }
        if (project->cg == gr) {
            project->cg = NULL;
        }
    }
    return RETURN_SUCCESS;
}

Quark *graph_new(Quark *project)
{
    Quark *g; 
    g = quark_new(project, QFlavorGraph);
    if (g) {
        Project *pr = (Project *) project->data;
        if (storage_add(pr->graphs, g) != RETURN_SUCCESS) {
            quark_free(g);
            return NULL;
        }
        quark_cb_set(g, graph_free_cb, NULL);
    }
    return g;
}

Quark *graph_next(Quark *project)
{
    Quark *g;
    
    g = graph_new(project);
    if (number_of_graphs(project) == 1) {
    Project *pr = (Project *) project->data;
        pr->cg = g;
    }
    return g;
}

void kill_all_graphs(Quark *project)
{
    Project *pr = (Project *) project->data;
    storage_empty(pr->graphs);
}

Quark *duplicate_graph(Quark *graph)
{
    Quark *project = graph->parent;
    Project *pr = (Project *) project->data;
    
    storage_scroll_to_data(pr->graphs, graph);
    return storage_duplicate(pr->graphs);
}

/**** Tickmarks ****/

tickmarks *get_graph_tickmarks(const Quark *gr, int axis)
{
    graph *g = (graph *) gr->data;
    
    if (g && axis >= 0 && axis < MAXAXES) {
        return g->t[axis];
    } else {
        return NULL;
    }
}

tickmarks *new_graph_tickmarks(void)
{
    tickmarks *retval;
    
    retval = xmalloc(sizeof(tickmarks));
    if (retval != NULL) {
        set_default_ticks(retval);
    }
    return retval;
}

tickmarks *copy_graph_tickmarks(tickmarks *t)
{
    tickmarks *retval;
    int i;
    
    if (t == NULL) {
        return NULL;
    } else {
        retval = new_graph_tickmarks();
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

void free_graph_tickmarks(tickmarks *t)
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

int set_graph_tickmarks(Quark *gr, int a, tickmarks *t)
{
    if (gr && a >= 0 && a < MAXAXES) {
        graph *g = (graph *) gr->data;
        free_graph_tickmarks(g->t[a]);
        g->t[a] = copy_graph_tickmarks(t);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int activate_tick_labels(tickmarks *t, int flag)
{
    if (t) {
        if (t->tl_flag != flag) {
            t->tl_flag = flag;
            set_dirtystate();
        }
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int select_graph(Quark *gr)
{
    graph *g = (graph *) gr->data;
    int ctrans_type, xyfixed;
    
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
    
    if (definewindow(&g->w, &g->v, ctrans_type, xyfixed,
            g->xscale,  g->yscale,
            g->xinvert, g->yinvert) == RETURN_SUCCESS) {
        Project *pr = (Project *) gr->parent->data;

        set_parser_gno(gr);
        pr->cg = gr;

        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int set_graph_type(Quark *gr, int gtype)
{
    if (gr) {
        graph *g = (graph *) gr->data;
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

/* 
 * Stack manipulation functions
 */
 
void clear_world_stack(Quark *pr)
{
    Quark *q;
    
    q = graph_get_current(pr);
    
    if (q) {
        graph *g = (graph *) q->data;
        g->ws_top = 1;
        g->curw = 0;
        g->ws[0].w.xg1 = 0.0;
        g->ws[0].w.xg2 = 0.0;
        g->ws[0].w.yg1 = 0.0;
        g->ws[0].w.yg2 = 0.0;
    }
}

static void update_world_stack(Quark *pr)
{
    Quark *q;
    
    q = graph_get_current(pr);
    
    if (q) {
        graph *g = (graph *) q->data;
        g->ws[g->curw].w = g->w;
    }
}

/* Add a world window to the stack
 * If there are other windows, simply add this one to the bottom of the stack
 * Otherwise, replace the first window with the new window 
 */
void add_world(Quark *gr, double x1, double x2, double y1, double y2)
{
    if (gr) {
        graph *g = (graph *) gr->data;
        
        /* see if another entry has been stacked */
        if (g->ws[0].w.xg1 == 0.0 &&
	    g->ws[0].w.xg2 == 0.0 &&
	    g->ws[0].w.yg1 == 0.0 &&
 	    g->ws[0].w.yg2 == 0.0) {	    
	        g->ws_top = 0;
	    }

        if (g->ws_top < MAX_ZOOM_STACK) {
	    g->ws[g->ws_top].w.xg1 = x1;
	    g->ws[g->ws_top].w.xg2 = x2;
	    g->ws[g->ws_top].w.yg1 = y1;
	    g->ws[g->ws_top].w.yg2 = y2;

	    g->ws_top++;
        } else {
	    errmsg("World stack full");
        }
    }
}

void cycle_world_stack(Quark *pr)
{
    int neww;
    Quark *gr;
    
    gr = graph_get_current(pr);
    
    if (gr) {
        graph *g = (graph *) gr->data;
        
        if (g->ws_top < 1) {
	    errmsg("World stack empty");
        } else {
	    update_world_stack(pr);
	    neww = (g->curw + 1) % g->ws_top;
 	    show_world_stack(pr, neww);
        }
    }
    
}

void show_world_stack(Quark *pr, int n)
{
    Quark *gr;
    
    gr = graph_get_current(pr);
    
    if (gr) {
        graph *g = (graph *) gr->data;

        if (g->ws_top < 1) {
	    errmsg("World stack empty");
        } else {
	    if (n >= g->ws_top) {
	        errmsg("Selected view greater than stack depth");
	    } else if (n < 0) {
	        errmsg("Selected view less than zero");
	    } else {
	        g->curw = n;
	        g->w = g->ws[n].w;
	    }
        }
    }

}

void push_world(Quark *pr)
{
    int i;
    Quark *gr;
    
    gr = graph_get_current(pr);
    
    if (gr) {
        graph *g = (graph *) gr->data;
    
        if (g->ws_top < MAX_ZOOM_STACK) {
            update_world_stack(pr);
            for (i = g->ws_top; i > g->curw; i--) {
                g->ws[i] = g->ws[i - 1];
            }
	    g->ws_top++;
        } else {
	    errmsg("World stack full");
        }
    }
}

void pop_world(Quark *pr)
{
    int i, neww;
    Quark *gr;
    
    gr = graph_get_current(pr);
    
    if (gr) {
        graph *g = (graph *) gr->data;

        if (g->ws_top <= 1) {
	    errmsg("World stack empty");
        } else {
    	    if (g->curw != g->ws_top - 1) {
    	        for (i = g->curw; i < g->ws_top; i++) {
                    g->ws[i] = g->ws[i + 1];
                }
                neww = g->curw;
    	    } else {
                neww = g->curw - 1;
            }
            g->ws_top--;
            show_world_stack(pr, neww);
        }
    }
}

int graph_world_stack_size(Quark *gr)
{
    if (gr) {
        graph *g = (graph *) gr->data;
        return g->ws_top;
    } else {
        return -1;
    }
}

int get_world_stack_current(Quark *gr)
{
    if (gr) {
        graph *g = (graph *) gr->data;
        return g->curw;
    } else {
        return -1;
    }
}

int get_world_stack_entry(Quark *gr, int n, world_stack *ws)
{
    if (gr) {
        graph *g = (graph *) gr->data;
        memcpy(ws, &g->ws[n], sizeof(world_stack));
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
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


Quark *set_get(Quark *gr, int setid)
{
    Quark *p = NULL;
    
    if (gr) {
        graph *g = (graph *) gr->data;
    
        if (storage_get_data_by_id(g->sets, setid, (void **) &p) !=
            RETURN_SUCCESS) {
            p = NULL;
        }
    }
    
    return p;
}

int is_set_hidden(Quark *pset)
{
    if (pset) {
        set *p = (set *) pset->data;
        return p->hidden;
    } else {
        return FALSE;
    }
}

int set_set_hidden(Quark *pset, int flag)
{
    if (pset) {
        set *p = (set *) pset->data;
        p->hidden = flag;
        set_dirtystate();
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int number_of_sets(Quark *gr)
{
    if (gr) {
        graph *g = (graph *) gr->data;
        return storage_count(g->sets);
    } else {
        return 0;
    }
}


framep *get_graph_frame(Quark *gr)
{
    if (gr) {
        graph *g = (graph *) gr->data;
        return &g->f;
    } else {
        return NULL;
    }
}

GLocator *get_graph_locator(Quark *gr)
{
    if (gr) {
        graph *g = (graph *) gr->data;
        return &g->locator;
    } else {
        return NULL;
    }
}

int get_graph_world(Quark *gr, world *w)
{
    if (gr) {
        graph *g = (graph *) gr->data;
        memcpy(w, &g->w, sizeof(world));
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int get_graph_viewport(Quark *gr, view *v)
{
    if (gr) {
        graph *g = (graph *) gr->data;
        memcpy(v, &g->v, sizeof(view));
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

labels *get_graph_labels(Quark *gr)
{
    if (gr) {
        graph *g = (graph *) gr->data;
        return &g->labs;
    } else {
        return NULL;
    }
}

legend *get_graph_legend(Quark *gr)
{
    if (gr) {
        graph *g = (graph *) gr->data;
        return &g->l;
    } else {
        return NULL;
    }
}

void set_graph_frame(Quark *gr, const framep *f)
{
    if (gr) {
        graph *g = (graph *) gr->data;
    
        memcpy(&g->f, f, sizeof(framep));
        set_dirtystate();
    }
}

void set_graph_locator(Quark *gr, const GLocator *locator)
{
    if (gr) {
        graph *g = (graph *) gr->data;

        memcpy(&g->locator, locator, sizeof(GLocator));
        set_dirtystate();
    }
}

void set_graph_world(Quark *gr, const world *w)
{
    if (gr) {
        graph *g = (graph *) gr->data;

        g->w = *w;
        set_dirtystate();
    }
}

void set_graph_viewport(Quark *gr, const view *v)
{
    if (gr) {
        graph *g = (graph *) gr->data;

        g->v = *v;
        set_dirtystate();
    }
}

void set_graph_title(Quark *gr, const plotstr *s)
{
    if (gr) {
        graph *g = (graph *) gr->data;

        xfree(g->labs.title.s);
        memcpy(&g->labs.title, s, sizeof(plotstr));
        g->labs.title.s = copy_string(NULL, s->s);

        set_dirtystate();
    }
}

void set_graph_stitle(Quark *gr, const plotstr *s)
{
    if (gr) {
        graph *g = (graph *) gr->data;

        xfree(g->labs.stitle.s);
        memcpy(&g->labs.stitle, s, sizeof(plotstr));
        g->labs.stitle.s = copy_string(NULL, s->s);

        set_dirtystate();
    }
}

void set_graph_xyflip(Quark *gr, int xyflip)
{
    if (gr) {
        graph *g = (graph *) gr->data;

        g->xyflip = xyflip;

        set_dirtystate();
    }
}

int get_graph_xyflip(Quark *gr)
{
    if (gr) {
        graph *g = (graph *) gr->data;

        return g->xyflip;
    } else {
        return FALSE;
    }
}

void set_graph_labels(Quark *gr, const labels *labs)
{
    if (gr) {
        graph *g = (graph *) gr->data;

        xfree(g->labs.title.s);
        xfree(g->labs.stitle.s);
        memcpy(&g->labs, labs, sizeof(labels));
        g->labs.title.s = copy_string(NULL, labs->title.s);
        g->labs.stitle.s = copy_string(NULL, labs->stitle.s);

        set_dirtystate();
    }
}

void set_graph_legend(Quark *gr, const legend *leg)
{
    if (gr) {
        graph *g = (graph *) gr->data;

        if (&g->l != leg) {
            memcpy(&g->l, leg, sizeof(legend));
        }

        set_dirtystate();
    }
}

void set_graph_legend_active(Quark *gr, int flag)
{
    if (gr) {
        graph *g = (graph *) gr->data;
        
        g->l.active = flag;

        set_dirtystate();
    }
}


int is_graph_hidden(Quark *gr)
{
    if (gr) {
        graph *g = (graph *) gr->data;
        return g->hidden;
    } else {
        return TRUE;
    }
}

int set_graph_hidden(Quark *gr, int flag)
{
    if (gr) {
        graph *g = (graph *) gr->data;
        g->hidden = flag;
        set_dirtystate();
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int set_graph_stacked(Quark *gr, int flag)
{
    if (gr) {
        graph *g = (graph *) gr->data;
        g->stacked = flag;
        set_dirtystate();
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int is_graph_stacked(Quark *gr)
{
    if (gr) {
        graph *g = (graph *) gr->data;
        return g->stacked;
    } else {
        return FALSE;
    }
}

double get_graph_bargap(Quark *gr)
{
    if (gr) {
        graph *g = (graph *) gr->data;
        return g->bargap;
    } else {
        return 0.0;
    }
}

int set_graph_bargap(Quark *gr, double bargap)
{
    if (gr) {
        graph *g = (graph *) gr->data;
        g->bargap = bargap;
        set_dirtystate();
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int get_graph_type(Quark *gr)
{
    if (gr) {
        graph *g = (graph *) gr->data;
        return g->type;
    } else {
        return -1;
    }
}

int is_refpoint_active(Quark *gr)
{
    if (gr) {
        graph *g = (graph *) gr->data;
        return g->locator.pointset;
    } else {
        return FALSE;
    }
}

int get_graph_xscale(Quark *gr)
{
    if (gr) {
        graph *g = (graph *) gr->data;
        return g->xscale;
    } else {
        return -1;
    }
}

int get_graph_yscale(Quark *gr)
{
    if (gr) {
        graph *g = (graph *) gr->data;
        return g->yscale;
    } else {
        return -1;
    }
}

int set_graph_xscale(Quark *gr, int scale)
{
    if (gr) {
        graph *g = (graph *) gr->data;
        if (g->xscale != scale) {
            int naxis;
            g->xscale = scale;
            for (naxis = 0; naxis < MAXAXES; naxis++) {
                if (is_xaxis(naxis)) {
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
            set_dirtystate();
        }
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int set_graph_yscale(Quark *gr, int scale)
{
    if (gr) {
        graph *g = (graph *) gr->data;
        if (g->yscale != scale) {
            int naxis;
            g->yscale = scale;
            for (naxis = 0; naxis < MAXAXES; naxis++) {
                if (is_yaxis(naxis)) {
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
            set_dirtystate();
        }
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int set_graph_znorm(Quark *gr, double norm)
{
    if (gr) {
        graph *g = (graph *) gr->data;
        g->znorm = norm;
        set_dirtystate();
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

double get_graph_znorm(Quark *gr)
{
    if (gr) {
        graph *g = (graph *) gr->data;
        return g->znorm;
    } else {
        return 0.0;
    }
}

int is_graph_xinvert(Quark *gr)
{
    if (gr) {
        graph *g = (graph *) gr->data;
        return g->xinvert;
    } else {
        return FALSE;
    }
}

int is_graph_yinvert(Quark *gr)
{
    if (gr) {
        graph *g = (graph *) gr->data;
        return g->yinvert;
    } else {
        return FALSE;
    }
}

int set_graph_xinvert(Quark *gr, int flag)
{
    if (gr) {
        graph *g = (graph *) gr->data;
        g->xinvert = flag;
        set_dirtystate();
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int set_graph_yinvert(Quark *gr, int flag)
{
    if (gr) {
        graph *g = (graph *) gr->data;
        g->yinvert = flag;
        set_dirtystate();
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int islogx(Quark *gr)
{
    if (gr) {
        graph *g = (graph *) gr->data;
        return (g->xscale == SCALE_LOG);
    } else {
        return FALSE;
    }
}

int islogy(Quark *gr)
{
    if (gr) {
        graph *g = (graph *) gr->data;
        return (g->yscale == SCALE_LOG);
    } else {
        return FALSE;
    }
}

int islogitx(Quark *gr)
{
    if (gr) {
        graph *g = (graph *) gr->data;
        return (g->xscale == SCALE_LOGIT);
    } else {
        return FALSE;
    }
}

int islogity(Quark *gr)
{
    if (gr) {
        graph *g = (graph *) gr->data;
        return (g->yscale == SCALE_LOGIT);
    } else {
        return FALSE;
    }
}

int is_log_axis(Quark *gr, int axis)
{
    if ((is_xaxis(axis) && islogx(gr)) ||
        (is_yaxis(axis) && islogy(gr))) {
        return TRUE;
    } else {
        return FALSE;
    }
}

int is_logit_axis(Quark *gr, int axis)
{
    if ((is_xaxis(axis) && islogitx(gr)) ||
        (is_yaxis(axis) && islogity(gr))) {
        return TRUE;
    } else {
        return FALSE;
    }
}

void project_postprocess(Quark *project)
{
    Project *pr = (Project *) project->data;
    Quark *gsave;
    int ngraphs, gno, setno, naxis;
    double ext_x, ext_y;
    
    if (pr->version_id >= bi_version_id()) {
        return;
    }

    if (pr->version_id < 40005) {
        set_page_dimensions(grace, 792, 612, FALSE);
    }

    if (pr->version_id < 50002) {
        pr->bgpen.pattern = 1;
    }

    if (pr->version_id < 50003) {
        allow_two_digits_years(TRUE);
        set_wrap_year(1900);
    }
    
    if (pr->version_id <= 40102) {
#ifndef NONE_GUI
        set_pagelayout(PAGE_FIXED);
#endif
        get_page_viewport(grace->rt->canvas, &ext_x, &ext_y);
        rescale_viewport(project, ext_x, ext_y);
    }
    
    gsave = pr->cg;
    
    storage_rewind(pr->graphs);
    ngraphs = storage_count(pr->graphs);
    for (gno = 0; gno < ngraphs; gno++) {
        Quark *gr;
        graph *g;
        Storage *sets;
        int nsets;
        
        if (storage_get_data(pr->graphs, (void **) &gr) != RETURN_SUCCESS) {
            break;
        }
        select_graph(gr);

        g = (graph *) gr->data;
        
	if (pr->version_id <= 40102) {
            g->l.vgap -= 0.01;
        }
	if (pr->version_id < 50200) {
            g->l.acorner = CORNER_UL;
        }
        
        sets = g->sets;
        storage_rewind(sets);
        nsets = storage_count(sets);
	for (setno = 0; setno < nsets; setno++) {
            Quark *pset;
            set *s;
            if (storage_get_data(sets, (void **) &pset) != RETURN_SUCCESS) {
                break;
            }
            
            s = (set *) pset->data;
            
            if (pr->version_id < 50000) {
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
            if ((pr->version_id < 40004 && g->type != GRAPH_CHART) ||
                s->sym.line.pen.color == -1) {
                s->sym.line.pen.color = s->line.line.pen.color;
            }
            if (pr->version_id < 40200 || s->sym.fillpen.color == -1) {
                s->sym.fillpen.color = s->sym.line.pen.color;
            }
            
	    if (pr->version_id <= 40102 && g->type == GRAPH_CHART) {
                s->type       = SET_BAR;
                s->sym.line    = s->line.line;
                s->line.line.style = 0;
                
                s->sym.fillpen = s->line.fillpen;
                s->line.fillpen.pattern = 0;
            }
	    if (pr->version_id <= 40102 && s->type == SET_XYHILO) {
                s->sym.line.width = s->line.line.width;
            }
	    if (pr->version_id < 50100 && s->type == SET_BOXPLOT) {
                s->sym.line.width = s->line.line.width;
                s->sym.line.style = s->line.line.style;
                s->sym.size = 2.0;
                s->errbar.riser_linew = s->line.line.width;
                s->errbar.riser_lines = s->line.line.style;
                s->line.line.style = 0;
                s->errbar.barsize = 0.0;
            }
            if (pr->version_id < 50003) {
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
            if (pr->version_id < 50002) {
                s->errbar.barsize *= 2;
            }

            if (pr->version_id < 50107) {
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

            storage_next(sets);
        }
        for (naxis = 0; naxis < MAXAXES; naxis++) {
	    tickmarks *t = g->t[naxis];
            if (!t) {
                continue;
            }
            
            if (pr->version_id <= 40102) {
                if ((is_xaxis(naxis) && g->xscale == SCALE_LOG) ||
                    (is_yaxis(naxis) && g->yscale == SCALE_LOG)) {
                    t->tmajor = pow(10.0, t->tmajor);
                }
                
                /* TODO : world/view translation */
                t->offsx = 0.0;
                t->offsy = 0.0;
            }
	    if (pr->version_id < 50000) {
	        /* There was no label_op in Xmgr */
                t->label_op = t->tl_op;
	        
                /* in xmgr, axis label placement was in x,y coordinates */
	        /* in Grace, it's parallel/perpendicular */
	        if (is_yaxis(naxis)) {
	            fswap(&t->label.offset.x,
                          &t->label.offset.y);
	        }
	        t->label.offset.y *= -1;
	    }
	    if (pr->version_id >= 50000 && pr->version_id < 50103) {
	        /* Autoplacement of axis labels wasn't implemented 
                   in early versions of Grace */
                if (t->label_place == TYPE_AUTO) {
                    t->label.offset.x = 0.0;
                    t->label.offset.y = 0.08;
                    t->label_place = TYPE_SPEC;
                }
            }
            if (pr->version_id < 50105) {
                /* Starting with 5.1.5, X axis min & inverting is honored
                   in pie charts */
                if (get_graph_type(gr) == GRAPH_PIE) {
                    world w;
                    get_graph_world(gr, &w);
                    w.xg1 = 0.0;
                    w.xg2 = 2*M_PI;
                    set_graph_world(gr, &w);
                    set_graph_xinvert(gr, FALSE);
                }
            }
        }
        
        if (pr->version_id >= 40200 && pr->version_id <= 50005) {
            int i, n;
            DObject *o;
            /* BBox type justification was erroneously set */
            storage_rewind(g->dobjects);
            n = storage_count(g->dobjects);
            for (i = 0; i < n; i++) {
                if (storage_get_data(g->dobjects, (void **) &o) ==
                    RETURN_SUCCESS) {
                    if (o->type == DO_STRING) {
                        DOStringData *s = (DOStringData *) o->odata;
                        s->just |= JUST_MIDDLE;
                    }
                } else {
                    break;
                }
                storage_next(g->dobjects);
            }
        }

        if (pr->version_id < 50200) {
            int i, n;
            DObject *o;

            storage_rewind(g->dobjects);
            n = storage_count(g->dobjects);
            for (i = 0; i < n; i++) {
                if (storage_get_data(g->dobjects, (void **) &o) ==
                    RETURN_SUCCESS) {
                    if (o->loctype == COORD_WORLD) {
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
                } else {
                    break;
                }
                storage_next(g->dobjects);
            }
        }
        storage_next(pr->graphs);
    }
    
    select_graph(gsave);
}
