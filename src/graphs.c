/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2000 Grace Development Team
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
#include "device.h"
#include "draw.h"
#include "graphs.h"
#include "graphutils.h"
#include "storage.h"
#include "objutils.h"
#include "parser.h"

#include "protos.h"

/* FIXMEOBJ */
#define objects grace->project->objects

/* graph definition */
#define graphs grace->project->graphs

static int cg = -1;

char *graph_types(int it)
{
    char *s;
    
    switch (it) {
    case GRAPH_XY:
	s = "XY";
	break;
    case GRAPH_CHART:
	s = "Chart";
	break;
    case GRAPH_POLAR:
	s = "Polar";
	break;
    case GRAPH_SMITH:
	s = "Smith";
	break;
    case GRAPH_FIXED:
	s = "Fixed";
	break;
    case GRAPH_PIE:
	s = "Pie";
	break;
    default:
        s = "Unknown";
	break;
    }
    return s;
}

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


int get_graph_ids(int **ids)
{
    return storage_get_all_ids(graphs, ids);
}

int get_set_ids(int gno, int **ids)
{
    graph *g;
    
    g = graph_get(gno);
    
    if (g) {
        return storage_get_all_ids(g->sets, ids);
    } else {
        return 0;
    }
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
        
        xfree(g);
    }
}

int graph_next(void)
{
    graph *g;
    int gno;
    
    g = graph_new();
    if (g) {
        gno = storage_get_unique_id(graphs);
        if (storage_add(graphs, gno, g) == RETURN_SUCCESS) {
            set_dirtystate();
            return gno;
        } else {
            graph_free(g);
            return -1;
        }
    } else {
        return -1;
    }
}

void kill_all_graphs(void)
{
    storage_empty(graphs);
}

int duplicate_graph(int gno)
{
    int new_gno;
    
    new_gno = storage_duplicate(graphs, gno);
    
    if (new_gno < 0) {
        return RETURN_FAILURE;
    } else {
        return RETURN_SUCCESS;
    }
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
    g_new->labs.title.s = copy_string(NULL, g->labs.title.s);
    g_new->labs.stitle.s = copy_string(NULL, g->labs.stitle.s);
    for (j = 0; j < MAXAXES; j++) {
	g_new->t[j] = copy_graph_tickmarks(g->t[j]);
    }
    
    if (!g_new->sets) {
        graph_free(g_new);
        return NULL;
    } else {
        return g_new;
    }
}

int copy_graph(int from, int to)
{
    int res;
    
    if ((res = storage_data_copy(graphs, from, to, FALSE)) == RETURN_SUCCESS) {
        set_dirtystate();
    }
    return res;
}

int move_graph(int from, int to)
{
    int res;
    
    if ((res = storage_data_move(graphs, from, to, FALSE)) == RETURN_SUCCESS) {
        set_dirtystate();
    }
    return res;
}

int swap_graph(int from, int to)
{
    int res;
    
    if ((res = storage_data_swap(graphs, from, to, FALSE)) == RETURN_SUCCESS) {
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
    int retval;
    graph *g;

    g = graph_get(gno);
    if (g && set_parser_gno(gno) == RETURN_SUCCESS) {
        cg = gno;
        retval = definewindow(g->w, g->v, g->type,
                              g->xscale,  g->yscale,
                              g->xinvert, g->yinvert);
    } else {
        retval = RETURN_FAILURE;
    }
    
    return retval;
}

int set_graph_type(int gno, int gtype)
{
    graph *g;
    
    g = graph_get(gno);
    if (g) {
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
        return -1;
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


int get_graph_legend(int gno, legend *leg)
{
    graph *g = graph_get(gno);
    if (g) {
        memcpy(leg, &g->l, sizeof(legend));
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}


int set_graph_active(int gno, int flag)
{
    graph *g = graph_get(gno);
    if (g) {
        if (flag) {
            g->hidden = FALSE;
            return RETURN_SUCCESS;
        } else {
            return kill_graph(gno);
        }
    } else {
        if (flag) {
            g = graph_new();
            if (g) {
                if (storage_add(graphs, gno, g) == RETURN_SUCCESS) {
                    return RETURN_SUCCESS;
                } else {
                    graph_free(g);
                    return RETURN_FAILURE;
                }
            } else {
                return RETURN_FAILURE;
            }
        } else {
            return RETURN_SUCCESS;
        }
    }
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

    memcpy(&g->l, leg, sizeof(legend));

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
        g->xscale = scale;
        set_dirtystate();
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int set_graph_yscale(int gno, int scale)
{
    graph *g = graph_get(gno);
    if (g) {
        g->yscale = scale;
        set_dirtystate();
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

int set_set_colors(int gno, int setno, int color)
{
    set *s;
    
    s = set_get(gno, setno);
    
    if (s && color < number_of_colors() && color >= 0) {
        s->linepen.color    = color;
        s->sympen.color     = color;
        s->symfillpen.color = color;
        s->errbar.pen.color = color;

        set_dirtystate();
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

static int project_version;

int get_project_version(void)
{
    return project_version;
}

int set_project_version(int version)
{
    if (version > bi_version_id()) {
        project_version = bi_version_id();
        return RETURN_FAILURE;
    } else {
        project_version = version;
        return RETURN_SUCCESS;
    }
}

void reset_project_version(void)
{
    project_version = bi_version_id();
}

static char *project_description = NULL;

void set_project_description(char *descr)
{
    project_description = copy_string(project_description, descr);
    set_dirtystate();
}

char *get_project_description(void)
{
    return project_description;
}

void postprocess_project(int version)
{
    int ngraphs, gno, setno, naxis;
    double ext_x, ext_y;
    
    if (version >= bi_version_id()) {
        return;
    }

    if (version < 40005) {
        set_page_dimensions(792, 612, FALSE);
    }

    if (get_project_version() < 50002) {
        setbgfill(TRUE);
    }

    if (get_project_version() < 50003) {
        allow_two_digits_years(TRUE);
        set_wrap_year(1900);
    }
    
    if (version <= 40102) {
#ifndef NONE_GUI
        set_pagelayout(PAGE_FIXED);
#endif
        get_page_viewport(&ext_x, &ext_y);
        rescale_viewport(ext_x, ext_y);
    }
    
    storage_rewind(graphs);
    ngraphs = storage_count(graphs);
    for (gno = 0; gno < ngraphs; gno++) {
        graph *g;
        Storage *sets;
        int nsets;
        
        if (storage_get_data(graphs, (void **) &g) != RETURN_SUCCESS) {
            break;
        }
	if (version <= 40102) {
            g->l.vgap -= 1;
        }
        
        sets = g->sets;
        storage_rewind(sets);
        nsets = storage_count(sets);
	for (setno = 0; setno < nsets; setno++) {
            set *s;
            if (storage_get_data(sets, (void **) &s) != RETURN_SUCCESS) {
                break;
            }
            
            if (version < 50000) {
                switch (s->sym) {
                case SYM_NONE:
                    break;
                case SYM_DOT_OBS:
                    s->sym = SYM_CIRCLE;
                    s->symsize = 0.0;
                    s->symlines = 0;
                    s->symfillpen.pattern = 1;
                    break;
                default:
                    s->sym--;
                    break;
                }
            }
            if ((version < 40004 && g->type != GRAPH_CHART) ||
                s->sympen.color == -1) {
                s->sympen.color = s->linepen.color;
            }
            if (version < 40200 || s->symfillpen.color == -1) {
                s->symfillpen.color = s->sympen.color;
            }
            
	    if (version <= 40102 && g->type == GRAPH_CHART) {
                s->type     = SET_BAR;
                s->sympen   = s->linepen;
                s->symlines = s->lines;
                s->symlinew = s->linew;
                s->lines    = 0;
                
                s->symfillpen = s->setfillpen;
                s->setfillpen.pattern = 0;
            }
	    if (version <= 40102 && s->type == SET_XYHILO) {
                s->symlinew = s->linew;
            }
	    if (version < 50100 && s->type == SET_BOXPLOT) {
                s->symlinew = s->linew;
                s->symlines = s->lines;
                s->symsize = 2.0;
                s->errbar.riser_linew = s->linew;
                s->errbar.riser_lines = s->lines;
                s->lines = 0;
                s->errbar.barsize = 0.0;
            }
            if (version < 50003) {
                s->errbar.active = TRUE;
                s->errbar.pen.color = s->sympen.color;
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
            if (version < 50002) {
                s->errbar.barsize *= 2;
            }
        }
        for (naxis = 0; naxis < MAXAXES; naxis++) {
	    if (version <= 40102) {
                if ((is_xaxis(naxis) && g->xscale == SCALE_LOG) ||
                    (is_yaxis(naxis) && g->yscale == SCALE_LOG)) {
                    g->t[naxis]->tmajor = pow(10.0, g->t[naxis]->tmajor);
                }
                
                /* TODO : world/view translation */
                g->t[naxis]->offsx = 0.0;
                g->t[naxis]->offsy = 0.0;
            }
	    if (version < 50000) {
	        /* There was no label_op in Xmgr */
                g->t[naxis]->label_op = g->t[naxis]->tl_op;
	        
                /* in xmgr, axis label placement was in x,y coordinates */
	        /* in Grace, it's parallel/perpendicular */
	        if (is_yaxis(naxis)) {
	            fswap(&g->t[naxis]->label.offset.x,
                          &g->t[naxis]->label.offset.y);
	        }
	        g->t[naxis]->label.offset.y *= -1;
	    }
        }
        
        
        storage_next(graphs);
    }

    if (version >= 40200 && version <= 50005) {
        int i, n;
        DObject *o;
        /* BBox type justification was erroneously set */
        storage_rewind(objects);
        n = storage_count(objects);
        for (i = 0; i < n; i++) {
            if (storage_get_data(objects, (void **) &o) == RETURN_SUCCESS) {
                if (o->type == DO_STRING) {
                    DOStringData *s = (DOStringData *) o->odata;
                    s->just |= JUST_MIDDLE;
                }
            } else {
                break;
            }
            storage_next(objects);
        }
    }
    if (version <= 50101) {
        int i, n, gsave;
        DObject *o;
        
        gsave = get_cg();
        
        storage_rewind(objects);
        n = storage_count(objects);
        for (i = 0; i < n; i++) {
            if (storage_get_data(objects, (void **) &o) == RETURN_SUCCESS) {
                if (o->loctype == COORD_WORLD && is_valid_gno(o->gno)) {
                    WPoint wp;
                    VPoint vp1, vp2;
                    
                    select_graph(o->gno);
                    
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
                            wp.x = o->ap.x + l->length*cos(o->angle);
                            wp.y = o->ap.y + l->length*sin(o->angle);
                            vp2 = Wpoint2Vpoint(wp);
                            
                            l->length = hypot(vp2.x - vp1.x, vp2.y - vp1.y);
                            o->angle  = atan2(vp2.y - vp1.y, vp2.x - vp1.x);
                        }
                        break;
                    case DO_STRING:
                        break;
                    }
                }
            } else {
                break;
            }
            storage_next(objects);
        }
        select_graph(gsave);
    }
}
