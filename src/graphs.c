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
 * Graph stuff
 *
 */

#include <config.h>
#include <cmath.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "globals.h"
#include "defines.h"
#include "utils.h"
#include "draw.h"
#include "graphs.h"
#include "graphutils.h"
#include "storage.h"
#include "objutils.h"
#include "parser.h"
#include "dict3.h"

#include "protos.h"

/* graph definition */
#define graphs project_get_graphs(grace->project)

static int cg = -1;

int is_valid_gno(int gno)
{
    return storage_id_exists(graphs, gno);
}

int number_of_graphs(void)
{
    return storage_count(graphs);
}

int get_cg(void)
{
    return cg;
}

graph *graph_get(int gno)
{
    graph *g;
    
    if (storage_get_data_by_id(graphs, gno, (void **) &g) == RETURN_SUCCESS) {
        return g;
    } else {
        return NULL;
    }
}

graph *graph_get_current(void)
{
    return graph_get(cg);
}

/*
 * kill all sets in a graph
 */
int kill_all_sets(graph *g)
{
    if (g) {
	storage_empty(g->sets);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int kill_graph(int gno)
{
    set_dirtystate();
    return storage_delete_by_id(graphs, gno);
}

static void wrap_set_free(void *data)
{
    set_free((set *) data);
}

static void *wrap_set_copy(void *data)
{
    return (set *) set_copy((set *) data);
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
    
    g->hidden = TRUE;
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

graph *graph_new(void)
{
    graph *g;
    
    g = xmalloc(sizeof(graph));
    if (g) {
        set_default_graph(g);
    }
    return g;
}

void graph_free(graph *g)
{
    if (g) {
        int j;
        
	kill_all_sets(g);
        
        XCFREE(g->labs.title.s);
        XCFREE(g->labs.stitle.s);
        
        for (j = 0; j < MAXAXES; j++) {
            free_graph_tickmarks(g->t[j]);
        }

        storage_free(g->dobjects);
        
        xfree(g);
    }
}

graph *graph_next(void)
{
    graph *g;
    
    g = graph_new();
    if (g) {
        if (storage_add(graphs, g) == RETURN_SUCCESS) {
            if (storage_count(graphs) == 1) {
                cg = 0;
            }
            set_dirtystate();
            return g;
        } else {
            graph_free(g);
            return NULL;
        }
    } else {
        return NULL;
    }
}

void kill_all_graphs(void)
{
    storage_empty(graphs);
}

graph *duplicate_graph(int gno)
{
    return storage_duplicate(graphs, gno);
}

graph *graph_copy(graph *g)
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
        graph_free(g_new);
        return NULL;
    } else {
        return g_new;
    }
}

int copy_graph(int from, int to)
{
    int res;
    
    if ((res = storage_data_copy_by_id(graphs, from, to)) == RETURN_SUCCESS) {
        set_dirtystate();
    }
    return res;
}

int move_graph(int from, int to)
{
    int res;
    
    if ((res = storage_data_move_by_id(graphs, from, to)) == RETURN_SUCCESS) {
        set_dirtystate();
    }
    return res;
}

int swap_graph(int from, int to)
{
    int res;
    
    if ((res = storage_data_swap(graphs, from, to)) == RETURN_SUCCESS) {
        set_dirtystate();
    }
    return res;
}


/**** Tickmarks ****/

int is_valid_axis(graph *g, int axis)
{
    if (g && axis >= 0 && axis < MAXAXES) {
        return TRUE;
    } else {
        return FALSE;
    }
}

tickmarks *get_graph_tickmarks(int gno, int axis)
{
    graph *g;
    
    g = graph_get(gno);
    
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
    int i;
    
    if (t == NULL) {
        return;
    }
    XCFREE(t->label.s);
    XCFREE(t->tl_formula);
    for (i = 0; i < MAX_TICKS; i++) {
        XCFREE(t->tloc[i].label);
    }
    XCFREE(t);
}

int set_graph_tickmarks(int gno, int a, tickmarks *t)
{
    graph *g;
    g = graph_get(gno);
    if (is_valid_axis(g, a) == TRUE) {
        free_graph_tickmarks(g->t[a]);
        g->t[a] = copy_graph_tickmarks(t);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int activate_tick_labels(int gno, int axis, int flag)
{
    tickmarks *t;
    t = get_graph_tickmarks(gno, axis);
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


int select_graph(int gno)
{
    graph *g = graph_get(gno);
    
    if (g && set_parser_gno(gno) == RETURN_SUCCESS &&
        isvalid_viewport(&g->v) == TRUE &&
        definewindow(&g->w, &g->v, g->type,
            g->xscale,  g->yscale,
            g->xinvert, g->yinvert) == RETURN_SUCCESS) {

        cg = gno;

        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int set_graph_type(int gno, int gtype)
{
    graph *g;
    
    g = graph_get(gno);
    if (g) {
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

int is_axis_active(int gno, int axis)
{
    tickmarks *t = get_graph_tickmarks(gno, axis);
    if (t) {
        return t->active;
    } else {
        return FALSE;
    }
}

int is_zero_axis(int gno, int axis)
{
    tickmarks *t = get_graph_tickmarks(gno, axis);
    if (t) {
        return t->zero;
    } else {
        return FALSE;
    }
}

/* 
 * Stack manipulation functions
 */
 
void clear_world_stack(void)
{
    graph *g;
    
    g = graph_get_current();
    
    if (g) {
        g->ws_top = 1;
        g->curw = 0;
        g->ws[0].w.xg1 = 0.0;
        g->ws[0].w.xg2 = 0.0;
        g->ws[0].w.yg1 = 0.0;
        g->ws[0].w.yg2 = 0.0;
    }
}

static void update_world_stack()
{
    graph *g;
    
    g = graph_get_current();
    
    if (g) {
        g->ws[g->curw].w = g->w;
    }
}

/* Add a world window to the stack
 * If there are other windows, simply add this one to the bottom of the stack
 * Otherwise, replace the first window with the new window 
 */
void add_world(int gno, double x1, double x2, double y1, double y2)
{
    graph *g;
    
    g = graph_get(gno);
    
    if (!g) {
        return;
    }

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

void cycle_world_stack(void)
{
    int neww;
    graph *g;
    
    g = graph_get_current();
    
    if (!g) {
        return;
    }
    
    if (g->ws_top < 1) {
	errmsg("World stack empty");
    } else {
	update_world_stack();
	neww = (g->curw + 1) % g->ws_top;
 	show_world_stack(neww);
    }
}

void show_world_stack(int n)
{
    graph *g;
    
    g = graph_get_current();
    
    if (!g) {
        return;
    }

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

void push_world(void)
{
    int i;
    graph *g;
    
    g = graph_get_current();
    
    if (!g) {
        return;
    }
    
    if (g->ws_top < MAX_ZOOM_STACK) {
        update_world_stack();
        for (i = g->ws_top; i > g->curw; i--) {
            g->ws[i] = g->ws[i - 1];
        }
	g->ws_top++;
    } else {
	errmsg("World stack full");
    }
}

/* modified to actually pop the current world view off the stack */
void pop_world(void)
{
    int i, neww;
    graph *g;
    
    g = graph_get_current();
    
    if (!g) {
        return;
    }

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
        show_world_stack(neww);
    }
}


int is_valid_setno(int gno, int setno)
{
    graph *g;
    
    g = graph_get(gno);
    
    if (g && storage_id_exists(g->sets, setno)) {
        return TRUE;
    } else {
        return FALSE;
    }
}

set *set_get(int gno, int setno)
{
    graph *g;
    set *p;
    
    g = graph_get(gno);
    
    if (g && storage_get_data_by_id(g->sets, setno, (void **) &p) ==
        RETURN_SUCCESS) {
        return p;
    } else {
        return NULL;
    }
}

int is_set_hidden(int gno, int setno)
{
    set *p;
    
    p = set_get(gno, setno);
    
    if (p) {
        return p->hidden;
    } else {
        return FALSE;
    }
}

int set_set_hidden(int gno, int setno, int flag)
{
    set *p;
    
    p = set_get(gno, setno);
    
    if (p) {
        p->hidden = flag;
        set_dirtystate();
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int number_of_sets(int gno)
{
    graph *g;
    
    g = graph_get(gno);
    
    if (g) {
        return storage_count(g->sets);
    } else {
        return 0;
    }
}

int graph_world_stack_size(int gno)
{
    graph *g;
    
    g = graph_get(gno);
    
    if (g) {
        return g->ws_top;
    } else {
        return -1;
    }
}

int get_world_stack_current(int gno)
{
    graph *g;
    
    g = graph_get(gno);
    
    if (g) {
        return g->curw;
    } else {
        return -1;
    }
}

int get_world_stack_entry(int gno, int n, world_stack *ws)
{
    graph *g;
    
    g = graph_get(gno);
    
    if (g) {
        memcpy(ws, &g->ws[n], sizeof(world_stack));
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}



int get_graph_framep(int gno, framep *f)
{
    graph *g = graph_get(gno);
    if (g) {
        memcpy(f, &g->f, sizeof(framep));
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int get_graph_locator(int gno, GLocator *locator)
{
    graph *g = graph_get(gno);
    if (g) {
        memcpy(locator, &g->locator, sizeof(GLocator));
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int get_graph_world(int gno, world *w)
{
    graph *g = graph_get(gno);
    if (g) {
        memcpy(w, &g->w, sizeof(world));
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int get_graph_viewport(int gno, view *v)
{
    graph *g = graph_get(gno);
    if (g) {
        memcpy(v, &g->v, sizeof(view));
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int get_graph_labels(int gno, labels *labs)
{
    graph *g = graph_get(gno);
    if (g) {
        memcpy(labs, &g->labs, sizeof(labels));
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}


legend *get_graph_legend(int gno)
{
    graph *g = graph_get(gno);
    if (g) {
        return &g->l;
    } else {
        return NULL;
    }
}


int set_graph_active(int gno, int show)
{
    graph *g;
    while ((g = graph_get(gno)) == NULL) {
        if (!graph_next()) {
            return RETURN_FAILURE;
        }
    }
    if (show) {
        g->hidden = FALSE;
    }
    return RETURN_SUCCESS;
}

void set_graph_framep(int gno, framep * f)
{
    graph *g = graph_get(gno);
    if (!g) {
        return;
    }
    
    memcpy(&g->f, f, sizeof(framep));
    
    set_dirtystate();
}

void set_graph_locator(int gno, GLocator *locator)
{
    graph *g = graph_get(gno);
    if (!g) {
        return;
    }

    memcpy(&g->locator, locator, sizeof(GLocator));

    set_dirtystate();
}

void set_graph_world(int gno, world w)
{
    graph *g = graph_get(gno);
    if (!g) {
        return;
    }

    g->w = w;
    
    set_dirtystate();
}

void set_graph_viewport(int gno, view v)
{
    graph *g = graph_get(gno);
    if (!g) {
        return;
    }

    g->v = v;
    
    set_dirtystate();
}

void set_graph_labels(int gno, labels *labs)
{
    graph *g = graph_get(gno);
    if (!g) {
        return;
    }

    xfree(g->labs.title.s);
    xfree(g->labs.stitle.s);
    memcpy(&g->labs, labs, sizeof(labels));
    g->labs.title.s = copy_string(NULL, labs->title.s);
    g->labs.stitle.s = copy_string(NULL, labs->stitle.s);
    
    set_dirtystate();
}

void set_graph_legend(int gno, legend *leg)
{
    graph *g = graph_get(gno);
    if (!g) {
        return;
    }

    if (&g->l != leg) {
        memcpy(&g->l, leg, sizeof(legend));
    }

    set_dirtystate();
}

void set_graph_legend_active(int gno, int flag)
{
    graph *g = graph_get(gno);
    if (!g) {
        return;
    }

    g->l.active = flag;

    set_dirtystate();
}


int is_graph_hidden(int gno)
{
    graph *g = graph_get(gno);
    if (g) {
        return g->hidden;
    } else {
        return TRUE;
    }
}

int set_graph_hidden(int gno, int flag)
{
    graph *g = graph_get(gno);
    if (g) {
        g->hidden = flag;
        set_dirtystate();
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int set_graph_stacked(int gno, int flag)
{
    graph *g = graph_get(gno);
    if (g) {
        g->stacked = flag;
        set_dirtystate();
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int is_graph_stacked(int gno)
{
    graph *g = graph_get(gno);
    if (g) {
        return g->stacked;
    } else {
        return FALSE;
    }
}

double get_graph_bargap(int gno)
{
    graph *g = graph_get(gno);
    if (g) {
        return g->bargap;
    } else {
        return 0.0;
    }
}

int set_graph_bargap(int gno, double bargap)
{
    graph *g = graph_get(gno);
    if (g) {
        g->bargap = bargap;
        set_dirtystate();
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int get_graph_type(int gno)
{
    graph *g = graph_get(gno);
    if (g) {
        return g->type;
    } else {
        return -1;
    }
}

int is_refpoint_active(int gno)
{
    graph *g = graph_get(gno);
    if (g) {
        return g->locator.pointset;
    } else {
        return FALSE;
    }
}

int get_graph_xscale(int gno)
{
    graph *g = graph_get(gno);
    if (g) {
        return g->xscale;
    } else {
        return -1;
    }
}

int get_graph_yscale(int gno)
{
    graph *g = graph_get(gno);
    if (g) {
        return g->yscale;
    } else {
        return -1;
    }
}

int set_graph_xscale(int gno, int scale)
{
    graph *g = graph_get(gno);
    if (g) {
        if (g->xscale != scale) {
            int naxis;
            g->xscale = scale;
            for (naxis = 0; naxis < MAXAXES; naxis++) {
                if (is_xaxis(naxis)) {
                    tickmarks *t;
                    t = get_graph_tickmarks(gno, naxis);
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

int set_graph_yscale(int gno, int scale)
{
    graph *g = graph_get(gno);
    if (g) {
        if (g->yscale != scale) {
            int naxis;
            g->yscale = scale;
            for (naxis = 0; naxis < MAXAXES; naxis++) {
                if (is_yaxis(naxis)) {
                    tickmarks *t;
                    t = get_graph_tickmarks(gno, naxis);
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

int set_graph_znorm(int gno, double norm)
{
    graph *g = graph_get(gno);
    if (g) {
        g->znorm = norm;
        set_dirtystate();
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

double get_graph_znorm(int gno)
{
    graph *g = graph_get(gno);
    if (g) {
        return g->znorm;
    } else {
        return 0.0;
    }
}

int is_graph_xinvert(int gno)
{
    graph *g = graph_get(gno);
    if (g) {
        return g->xinvert;
    } else {
        return FALSE;
    }
}

int is_graph_yinvert(int gno)
{
    graph *g = graph_get(gno);
    if (g) {
        return g->yinvert;
    } else {
        return FALSE;
    }
}

int set_graph_xinvert(int gno, int flag)
{
    graph *g = graph_get(gno);
    if (g) {
        g->xinvert = flag;
        set_dirtystate();
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int set_graph_yinvert(int gno, int flag)
{
    graph *g = graph_get(gno);
    if (g) {
        g->yinvert = flag;
        set_dirtystate();
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int islogx(int gno)
{
    graph *g = graph_get(gno);
    if (g) {
        return (g->xscale == SCALE_LOG);
    } else {
        return FALSE;
    }
}

int islogy(int gno)
{
    graph *g = graph_get(gno);
    if (g) {
        return (g->yscale == SCALE_LOG);
    } else {
        return FALSE;
    }
}

int islogitx(int gno)
{
    graph *g = graph_get(gno);
    if (g) {
        return (g->xscale == SCALE_LOGIT);
    } else {
        return FALSE;
    }
}

int islogity(int gno)
{
    graph *g = graph_get(gno);
    if (g) {
        return (g->yscale == SCALE_LOGIT);
    } else {
        return FALSE;
    }
}

int set_set_colors(set *p, int color)
{
    if (p && color < number_of_colors(grace->rt->canvas) && color >= 0) {
        p->line.pen.color    = color;
        p->symline.pen.color     = color;
        p->symfillpen.color = color;
        p->errbar.pen.color = color;

        set_dirtystate();
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

char *get_project_description(void)
{
    return project_get_description(grace->project);
}

#undef graphs

void project_postprocess(Quark *q)
{
    Project *pr = (Project *) q->data;
    int ngraphs, gsave, gno, setno, naxis;
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
        rescale_viewport(pr, ext_x, ext_y);
    }
    
    gsave = get_cg();
    
    storage_rewind(pr->graphs);
    ngraphs = storage_count(pr->graphs);
    for (gno = 0; gno < ngraphs; gno++) {
        graph *g;
        Storage *sets;
        int nsets;
        
        select_graph(gno);
        
        if (storage_get_data(pr->graphs, (void **) &g) != RETURN_SUCCESS) {
            break;
        }

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
            set *s;
            if (storage_get_data(sets, (void **) &s) != RETURN_SUCCESS) {
                break;
            }
            
            if (pr->version_id < 50000) {
                switch (s->sym) {
                case SYM_NONE:
                    break;
                case SYM_DOT_OBS:
                    s->sym = SYM_CIRCLE;
                    s->symsize = 0.0;
                    s->symline.style = 0;
                    s->symfillpen.pattern = 1;
                    break;
                default:
                    s->sym--;
                    break;
                }
            }
            if ((pr->version_id < 40004 && g->type != GRAPH_CHART) ||
                s->symline.pen.color == -1) {
                s->symline.pen.color = s->line.pen.color;
            }
            if (pr->version_id < 40200 || s->symfillpen.color == -1) {
                s->symfillpen.color = s->symline.pen.color;
            }
            
	    if (pr->version_id <= 40102 && g->type == GRAPH_CHART) {
                s->type       = SET_BAR;
                s->symline    = s->line;
                s->line.style = 0;
                
                s->symfillpen = s->setfillpen;
                s->setfillpen.pattern = 0;
            }
	    if (pr->version_id <= 40102 && s->type == SET_XYHILO) {
                s->symline.width = s->line.width;
            }
	    if (pr->version_id < 50100 && s->type == SET_BOXPLOT) {
                s->symline.width = s->line.width;
                s->symline.style = s->line.style;
                s->symsize = 2.0;
                s->errbar.riser_linew = s->line.width;
                s->errbar.riser_lines = s->line.style;
                s->line.style = 0;
                s->errbar.barsize = 0.0;
            }
            if (pr->version_id < 50003) {
                s->errbar.active = TRUE;
                s->errbar.pen.color = s->symline.pen.color;
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
                if (get_graph_type(gno) == GRAPH_PIE) {
                    world w;
                    get_graph_world(gno, &w);
                    w.xg1 = 0.0;
                    w.xg2 = 2*M_PI;
                    set_graph_world(gno, w);
                    set_graph_xinvert(gno, FALSE);
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
