/*
 * Grace - Graphics for Exploratory Data Analysis
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-95 Paul J Turner, Portland, OR
 * Copyright (c) 1996-98 GRACE Development Team
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

#include "defines.h"
#include "utils.h"
#include "device.h"
#include "draw.h"
#include "graphs.h"
#include "graphutils.h"

#include "protos.h"

/* graph definition */
graph *g = NULL;
static int cg = 0;			/* the current graph */
static int maxgraph = 0;

static void update_world_stack();


int is_valid_gno(int gno)
{
    if (gno >= 0 && gno < maxgraph) {
        return TRUE;
    } else {
        return FALSE;
    }
}

int is_valid_axis(axis)
{
    if (axis >= 0 || axis < MAXAXES) {
        return TRUE;
    } else {
        return FALSE;
    }
}

int number_of_graphs(void)
{
    return maxgraph;
}

int get_cg(void)
{
    return cg;
}

char *graph_types(int it)
{
    static char s[16];

    switch (it) {
    case GRAPH_XY:
	strcpy(s, "XY");
	break;
    case GRAPH_CHART:
	strcpy(s, "Chart");
	break;
    case GRAPH_POLAR:
	strcpy(s, "Polar");
	break;
    case GRAPH_SMITH:
	strcpy(s, "Smith");
	break;
    case GRAPH_FIXED:
	strcpy(s, "Fixed");
	break;
    default:
        strcpy(s, "Unknown");
	break;
   }
    return s;
}

/*
 * kill all sets in a graph
 */
int kill_all_sets(int gno)
{
    int i;
    
    if (is_valid_gno(gno) == TRUE) {
	for (i = 0; i < g[gno].maxplot; i++) {
	    killset(gno, i);
	}
        return GRACE_EXIT_SUCCESS;
    } else {
        return GRACE_EXIT_FAILURE;
    }
}

int kill_graph(int gno)
{
    if (is_valid_gno(gno) == TRUE) {
	kill_all_sets(gno);
        
	/* We want to have at least one graph anyway */
        if (gno == maxgraph - 1 && maxgraph > 1) {
            maxgraph--;
            g = xrealloc(g, maxgraph*sizeof(graph));
            if (cg == gno) {
                cg--;
            }
        } else {
            set_default_graph(gno);
        }
        
        set_dirtystate();
        return GRACE_EXIT_SUCCESS;
    } else {
        return GRACE_EXIT_FAILURE;
    }
}

void kill_all_graphs(void)
{
    int i;

    for (i = maxgraph - 1; i >= 0; i--) {
        kill_graph(i);
    }
}

int copy_graph(int from, int to)
{
    int i, j, a;
    tickmarks t;

    if (is_valid_gno(from) != TRUE || is_valid_gno(to) != TRUE) {
        return GRACE_EXIT_FAILURE;
    }
    
    kill_all_sets(to);

    memcpy(&g[to], &g[from], sizeof(graph));
    for (a = 0; a < MAXAXES; a++) {
        zero_ticklabels(&(g[from].t[a]));
        get_graph_tickmarks(from, &t, a);
        set_graph_tickmarks(to, &t, a);
        free_ticklabels(&t);
    }
    g[to].p = NULL;
    g[to].maxplot = 0;
    
    if (realloc_graph_plots(to, g[from].maxplot) != GRACE_EXIT_SUCCESS) {
        return GRACE_EXIT_FAILURE;
    }
    
    for (i = 0; i < g[from].maxplot; i++) {
	for (j = 0; j < MAX_SET_COLS; j++) {
	    g[to].p[i].data.ex[j] = NULL;
	}
	if (is_set_active(from, i)) {
	    do_copyset(from, i, to, i);
	}
    }

    g[to].labs.title = copy_plotstr(g[from].labs.title);
    g[to].labs.stitle = copy_plotstr(g[from].labs.stitle);

    for (j = 0; j < MAXAXES; j++) {
	g[to].t[j].label = copy_plotstr(g[from].t[j].label);
	for (i = 0; i < MAX_TICKS; i++) {
	    if (g[from].t[j].tloc[i].label != NULL) {
                g[to].t[j].tloc[i].label = copy_string(g[to].t[j].tloc[i].label,
                                                g[from].t[j].tloc[i].label);
	    }
	}
    }

    return GRACE_EXIT_SUCCESS;
}

int move_graph(int from, int to)
{
    if (is_valid_gno(from) != TRUE || is_valid_gno(to) != TRUE) {
        return GRACE_EXIT_FAILURE;
    }
    
    if (copy_graph(from, to) != GRACE_EXIT_SUCCESS) {
        return GRACE_EXIT_FAILURE;
    } else {
        kill_graph(from);
        return GRACE_EXIT_SUCCESS;
    }
}

int duplicate_graph(int gno)
{
    int new_gno = maxgraph;
    
    if (is_valid_gno(gno) != TRUE) {
        return GRACE_EXIT_FAILURE;
    }

    if (set_graph_active(new_gno, TRUE) != GRACE_EXIT_SUCCESS) {
        return GRACE_EXIT_FAILURE;
    }
    
    if (copy_graph(gno, new_gno) != GRACE_EXIT_SUCCESS) {
        return GRACE_EXIT_FAILURE;
    } else {
        return GRACE_EXIT_SUCCESS;
    }
}

int swap_graph(int from, int to)
{
    graph gtmp;

    if (is_valid_gno(from) != TRUE || is_valid_gno(to) != TRUE) {
        return GRACE_EXIT_FAILURE;
    }

    memcpy(&gtmp, &g[from], sizeof(graph));
    memcpy(&g[from], &g[to], sizeof(graph));
    memcpy(&g[to], &gtmp, sizeof(graph));

    set_dirtystate();

    return GRACE_EXIT_SUCCESS;
}

int get_graph_framep(int gno, framep *f)
{
    if (is_valid_gno(gno) == TRUE) {
        memcpy(f, &g[gno].f, sizeof(framep));
        return GRACE_EXIT_SUCCESS;
    } else {
        return GRACE_EXIT_FAILURE;
    }
}

int get_graph_locator(int gno, GLocator *locator)
{
    if (is_valid_gno(gno) == TRUE) {
        memcpy(locator, &g[gno].locator, sizeof(GLocator));
        return GRACE_EXIT_SUCCESS;
    } else {
        return GRACE_EXIT_FAILURE;
    }
}

int get_graph_world(int gno, world *w)
{
    if (is_valid_gno(gno) == TRUE) {
        memcpy(w, &g[gno].w, sizeof(world));
        return GRACE_EXIT_SUCCESS;
    } else {
        return GRACE_EXIT_FAILURE;
    }
}

int get_graph_viewport(int gno, view *v)
{
    if (is_valid_gno(gno) == TRUE) {
        memcpy(v, &g[gno].v, sizeof(view));
        return GRACE_EXIT_SUCCESS;
    } else {
        return GRACE_EXIT_FAILURE;
    }
}

int get_graph_labels(int gno, labels *labs)
{
    if (is_valid_gno(gno) == TRUE) {
        memcpy(labs, &g[gno].labs, sizeof(labels));
        return GRACE_EXIT_SUCCESS;
    } else {
        return GRACE_EXIT_FAILURE;
    }
}

int get_graph_plotarr(int gno, int i, plotarr *p)
{
    if (is_valid_gno(gno) == TRUE) {
        memcpy(p, &g[gno].p[i], sizeof(plotarr));
        return GRACE_EXIT_SUCCESS;
    } else {
        return GRACE_EXIT_FAILURE;
    }
}

int get_graph_tickmarks(int gno, tickmarks *t, int a)
{
    int i;
    if (is_valid_gno(gno) == TRUE) {
        memcpy(t, &g[gno].t[a], sizeof(tickmarks));
        for (i = 0; i < MAX_TICKS; i++) {
            t->tloc[i].label = copy_string(NULL, g[gno].t[a].tloc[i].label);
        }
        return GRACE_EXIT_SUCCESS;
    } else {
        return GRACE_EXIT_FAILURE;
    }
}

int get_graph_legend(int gno, legend *leg)
{
    if (is_valid_gno(gno) == TRUE) {
        memcpy(leg, &g[gno].l, sizeof(legend));
        return GRACE_EXIT_SUCCESS;
    } else {
        return GRACE_EXIT_FAILURE;
    }
}


int set_graph_active(int gno, int flag)
{
    int retval;
    
    if (flag == TRUE) {
        if (gno >= maxgraph) {
            retval = realloc_graphs(gno + 1);
            if (retval == GRACE_EXIT_SUCCESS) {
                set_graph_hidden(gno, FALSE);
            }
            return retval;
        } else {
            return GRACE_EXIT_SUCCESS;
        }
    } else { 
        if (is_valid_gno(gno) == TRUE) {
            kill_graph(gno);
        } 
        return GRACE_EXIT_SUCCESS;
    }
}

void set_graph_framep(int gno, framep * f)
{
    if (is_valid_gno(gno) != TRUE) {
        return;
    }
    
    memcpy(&g[gno].f, f, sizeof(framep));
    
    set_dirtystate();
}

void set_graph_locator(int gno, GLocator *locator)
{
    if (is_valid_gno(gno) != TRUE) {
        return;
    }

    memcpy(&g[gno].locator, locator, sizeof(GLocator));
}

void set_graph_world(int gno, world w)
{
    if (is_valid_gno(gno) != TRUE) {
        return;
    }

    g[gno].w = w;
    
    set_dirtystate();
}

void set_graph_viewport(int gno, view v)
{
    if (is_valid_gno(gno) != TRUE) {
        return;
    }

    g[gno].v = v;
    
    set_dirtystate();
}

void set_graph_labels(int gno, labels * labs)
{
    if (is_valid_gno(gno) != TRUE) {
        return;
    }

    memcpy(&g[gno].labs, labs, sizeof(labels));
    
    set_dirtystate();
}

void set_graph_plotarr(int gno, int i, plotarr * p)
{
    if (is_valid_gno(gno) != TRUE) {
        return;
    }

    memcpy(&g[gno].p[i], p, sizeof(plotarr));
    
    set_dirtystate();
}

void zero_ticklabels(tickmarks *t)
{
    int i;
    
    for (i = 0; i < MAX_TICKS; i++) {
        t->tloc[i].label = NULL;
    }
}

void free_ticklabels(tickmarks *t)
{
    int i;
    
    for (i = 0; i < MAX_TICKS; i++) {
        cxfree(t->tloc[i].label);
    }
}

int set_graph_tickmarks(int gno, tickmarks *t, int a)
{
    int i;

    if (is_valid_gno(gno) == TRUE) {
        free_ticklabels(&(g[gno].t[a]));
        memcpy(&g[gno].t[a], t, sizeof(tickmarks));
        zero_ticklabels(&(g[gno].t[a]));
        for (i = 0; i < MAX_TICKS; i++) {
            g[gno].t[a].tloc[i].label =
                       copy_string(g[gno].t[a].tloc[i].label, t->tloc[i].label);
        }
        return GRACE_EXIT_SUCCESS;
    } else {
        return GRACE_EXIT_FAILURE;
    }
}

void set_graph_legend(int gno, legend *leg)
{
    if (is_valid_gno(gno) != TRUE) {
        return;
    }

    memcpy(&g[gno].l, leg, sizeof(legend));

    set_dirtystate();
}

void set_graph_legend_active(int gno, int flag)
{
    if (is_valid_gno(gno) != TRUE) {
        return;
    }

    g[gno].l.active = flag;

    set_dirtystate();
}


/*
 * Count the number of active sets in graph gno
 */
int nactive(int gno)
{
    int i, cnt = 0;

    for (i = 0; i < number_of_sets(gno); i++) {
	if (is_set_active(gno, i)) {
	    cnt++;
	}
    }

    return cnt;
}



int select_graph(int gno)
{
    int retval;

    if (is_valid_gno(gno) == TRUE) {
        cg = gno;
        retval = definewindow(g[gno].w, g[gno].v, g[gno].type,
                              g[gno].xscale, g[gno].yscale,
                              g[gno].xinvert, g[gno].yinvert);
    } else {
        retval = GRACE_EXIT_FAILURE;
    }
    
    return retval;
}

int realloc_graphs(int n)
{
    int j;
    graph *gtmp;

    if (n <= 0) {
        return GRACE_EXIT_FAILURE;
    }
    gtmp = xrealloc(g, n*sizeof(graph));
    if (gtmp == NULL) {
        return GRACE_EXIT_FAILURE;
    } else {
        g = gtmp;
        for (j = maxgraph; j < n; j++) {
            set_default_graph(j);
        }
        maxgraph = n;
        return GRACE_EXIT_SUCCESS;
    }
}

int realloc_graph_plots(int gno, int n)
{
    int oldmaxplot, j;
    plotarr *ptmp;
    int c, bg;
    
    if (is_valid_gno(gno) != TRUE) {
        return GRACE_EXIT_FAILURE;
    }
    if (n <= 0) {
        return GRACE_EXIT_FAILURE;
    }
    ptmp = xrealloc(g[gno].p, n * sizeof(plotarr));
    if (ptmp == NULL) {
        return GRACE_EXIT_FAILURE;
    } else {
        oldmaxplot = g[gno].maxplot;
        g[gno].p = ptmp;
        g[gno].maxplot = n;
        bg = getbgcolor();
        c = oldmaxplot + 1;
        for (j = oldmaxplot; j < n; j++) {
            set_default_plotarr(&g[gno].p[j]);
            while (c == bg || get_colortype(c) != COLOR_MAIN) {
                c++;
                c %= number_of_colors();
            }
            set_set_colors(gno, j, c);
            c++;
        }
        return GRACE_EXIT_SUCCESS;
    }
}


int set_graph_type(int gno, int gtype)
{
    if (is_valid_gno(gno) == TRUE) {
        switch (gtype) {
        case GRAPH_XY:
        case GRAPH_CHART:
        case GRAPH_FIXED:
            break;
        case GRAPH_POLAR:
	    g[gno].w.xg1 = 0.0;
	    g[gno].w.xg2 = 2*M_PI;
	    g[gno].w.yg1 = 0.0;
	    g[gno].w.yg2 = 1.0;
            break;
        case GRAPH_SMITH:
	    g[gno].w.xg1 = -1.0;
	    g[gno].w.xg2 =  1.0;
	    g[gno].w.yg1 = -1.0;
	    g[gno].w.yg2 =  1.0;
            break;
        default:
            errmsg("Internal error in set_graph_type()");
            return GRACE_EXIT_FAILURE;
        }
        g[gno].type = gtype;
        return GRACE_EXIT_SUCCESS;
    } else {
        return GRACE_EXIT_FAILURE;
    }
}

int is_graph_hidden(int gno)
{
    if (is_valid_gno(gno) == TRUE) {
        return g[gno].hidden;
    } else {
        return TRUE;
    }
}

int set_graph_hidden(int gno, int flag)
{
    if (is_valid_gno(gno) == TRUE) {
        g[gno].hidden = flag;
        set_dirtystate();
        return GRACE_EXIT_SUCCESS;
    } else {
        return GRACE_EXIT_FAILURE;
    }
}

int set_graph_stacked(int gno, int flag)
{
    if (is_valid_gno(gno) == TRUE) {
        g[gno].stacked = flag;
        set_dirtystate();
        return GRACE_EXIT_SUCCESS;
    } else {
        return GRACE_EXIT_FAILURE;
    }
}

int is_graph_stacked(int gno)
{
    if (is_valid_gno(gno) == TRUE) {
        return g[gno].stacked;
    } else {
        return FALSE;
    }
}

double get_graph_bargap(int gno)
{
    if (is_valid_gno(gno) == TRUE) {
        return g[gno].bargap;
    } else {
        return 0.0;
    }
}

int set_graph_bargap(int gno, double bargap)
{
    if (is_valid_gno(gno) == TRUE) {
        g[gno].bargap = bargap;
        set_dirtystate();
        return GRACE_EXIT_SUCCESS;
    } else {
        return GRACE_EXIT_FAILURE;
    }
}

int get_graph_type(int gno)
{
    if (is_valid_gno(gno) == TRUE) {
        return g[gno].type;
    } else {
        return -1;
    }
}

int is_refpoint_active(int gno)
{
    if (is_valid_gno(gno) == TRUE) {
        return g[gno].locator.pointset;
    } else {
        return FALSE;
    }
}

int get_graph_xscale(int gno)
{
    if (is_valid_gno(gno) == TRUE) {
        return g[gno].xscale;
    } else {
        return -1;
    }
}

int get_graph_yscale(int gno)
{
    if (is_valid_gno(gno) == TRUE) {
        return g[gno].yscale;
    } else {
        return -1;
    }
}

int set_graph_xscale(int gno, int scale)
{
    if (is_valid_gno(gno) == TRUE) {
        g[gno].xscale = scale;
        set_dirtystate();
        return GRACE_EXIT_SUCCESS;
    } else {
        return GRACE_EXIT_FAILURE;
    }
}

int set_graph_yscale(int gno, int scale)
{
    if (is_valid_gno(gno) == TRUE) {
        g[gno].yscale = scale;
        set_dirtystate();
        return GRACE_EXIT_SUCCESS;
    } else {
        return GRACE_EXIT_FAILURE;
    }
}

int is_graph_xinvert(int gno)
{
    if (is_valid_gno(gno) == TRUE) {
        return g[gno].xinvert;
    } else {
        return FALSE;
    }
}

int is_graph_yinvert(int gno)
{
    if (is_valid_gno(gno) == TRUE) {
        return g[gno].yinvert;
    } else {
        return FALSE;
    }
}

int set_graph_xinvert(int gno, int flag)
{
    if (is_valid_gno(gno) == TRUE) {
        g[gno].xinvert = flag;
        set_dirtystate();
        return GRACE_EXIT_SUCCESS;
    } else {
        return GRACE_EXIT_FAILURE;
    }
}

int set_graph_yinvert(int gno, int flag)
{
    if (is_valid_gno(gno) == TRUE) {
        g[gno].yinvert = flag;
        set_dirtystate();
        return GRACE_EXIT_SUCCESS;
    } else {
        return GRACE_EXIT_FAILURE;
    }
}

int is_axis_active(int gno, int axis)
{
    if (is_valid_gno(gno) == TRUE && is_valid_axis(axis) == TRUE) {
        return g[gno].t[axis].active;
    } else {
        return FALSE;
    }
}

int is_zero_axis(int gno, int axis)
{
    if (is_valid_gno(gno) == TRUE && is_valid_axis(axis) == TRUE) {
        return g[gno].t[axis].zero;
    } else {
        return FALSE;
    }
}

int islogx(int gno)
{
    if (is_valid_gno(gno) == TRUE) {
        return (g[gno].xscale == SCALE_LOG);
    } else {
        return FALSE;
    }
}

int islogy(int gno)
{
    if (is_valid_gno(gno) == TRUE) {
        return (g[gno].yscale == SCALE_LOG);
    } else {
        return FALSE;
    }
}


/* 
 * Stack manipulation functions
 */
 
void clear_world_stack(void)
{
    g[cg].ws_top = 1;
    g[cg].curw = 0;
    g[cg].ws[0].w.xg1 = 0.0;
    g[cg].ws[0].w.xg2 = 0.0;
    g[cg].ws[0].w.yg1 = 0.0;
    g[cg].ws[0].w.yg2 = 0.0;
}

static void update_world_stack()
{
    g[cg].ws[g[cg].curw].w = g[cg].w;
}

/* Add a world window to the stack
 * If there are other windows, simply add this one to the bottom of the stack
 * Otherwise, replace the first window with the new window 
 */
void add_world(int gno, double x1, double x2, double y1, double y2)
{
    /* see if another entry has been stacked */
    if( g[gno].ws[0].w.xg1 == 0.0 &&
	g[gno].ws[0].w.xg2 == 0.0 &&
	g[gno].ws[0].w.yg1 == 0.0 &&
 	g[gno].ws[0].w.yg2 == 0.0 ) {	    
	    g[gno].ws_top = 0;
	}
		
    if (g[gno].ws_top < MAX_ZOOM_STACK) {
	g[gno].ws[g[gno].ws_top].w.xg1 = x1;
	g[gno].ws[g[gno].ws_top].w.xg2 = x2;
	g[gno].ws[g[gno].ws_top].w.yg1 = y1;
	g[gno].ws[g[gno].ws_top].w.yg2 = y2;

	g[gno].ws_top++;
    } else {
	errmsg("World stack full");
    }
}

void cycle_world_stack(void)
{
    int neww;
    
    if (g[cg].ws_top < 1) {
	errmsg("World stack empty");
    } else {
	update_world_stack();
	neww = (g[cg].curw + 1) % g[cg].ws_top;
 	show_world_stack(neww);
    }
}

void show_world_stack(int n)
{
    if (g[cg].ws_top < 1) {
	errmsg("World stack empty");
    } else {
	if (n >= g[cg].ws_top) {
	    errmsg("Selected view greater than stack depth");
	} else if (n < 0) {
	    errmsg("Selected view less than zero");
	} else {
	    g[cg].curw = n;
	    g[cg].w = g[cg].ws[n].w;
	}
    }
}

void push_world(void)
{
    int i;

    if (g[cg].ws_top < MAX_ZOOM_STACK) {
        update_world_stack();
        for( i=g[cg].ws_top; i>g[cg].curw; i-- ) {
               g[cg].ws[i] = g[cg].ws[i-1];
        }
	g[cg].ws_top++;
    } else {
	errmsg("World stack full");
    }
}

/* modified to actually pop the current world view off the stack */
void pop_world(void)
{
    int i, neww;

    if (g[cg].ws_top <= 1) {
	errmsg("World stack empty");
    } else {
    	if (g[cg].curw != g[cg].ws_top - 1) {
    	    for (i = g[cg].curw; i < g[cg].ws_top; i++) {
                g[cg].ws[i] = g[cg].ws[i + 1];
            }
            neww = g[cg].curw;
    	} else {
            neww = g[cg].curw - 1;
        }
        g[cg].ws_top--;
        show_world_stack(neww);
    }
}


void set_default_graph(int gno)
{    
    g[gno].hidden = TRUE;
    g[gno].type = GRAPH_XY;
    g[gno].xinvert = FALSE;
    g[gno].yinvert = FALSE;
    g[gno].xyflip = FALSE;
    g[gno].stacked = FALSE;
    g[gno].bargap = 0.0;
    g[gno].xscale = SCALE_NORMAL;
    g[gno].yscale = SCALE_NORMAL;
    g[gno].ws_top = 1;
    g[gno].ws[0].w.xg1=g[gno].ws[0].w.xg2=g[gno].ws[0].w.yg1=g[gno].ws[0].w.yg2=0;
        g[gno].curw = 0;
    g[gno].locator.dsx = g[gno].locator.dsy = 0.0;      /* locator props */
    g[gno].locator.pointset = FALSE;
    g[gno].locator.pt_type = 0;
    g[gno].locator.fx = FORMAT_GENERAL;
    g[gno].locator.fy = FORMAT_GENERAL;
    g[gno].locator.px = 6;
    g[gno].locator.py = 6;
    set_default_ticks(&g[gno].t[0], X_AXIS);
    set_default_ticks(&g[gno].t[1], Y_AXIS);
    set_default_ticks(&g[gno].t[2], ZX_AXIS);
    set_default_ticks(&g[gno].t[3], ZY_AXIS);
    set_default_framep(&g[gno].f);
    set_default_world(&g[gno].w);
    set_default_view(&g[gno].v);
    set_default_legend(gno, &g[gno].l);
    /* TODO: memory leak! */
    set_default_string(&g[gno].labs.title);
    g[gno].labs.title.charsize = 1.5;
    /* TODO: memory leak! */
    set_default_string(&g[gno].labs.stitle);
    g[gno].labs.stitle.charsize = 1.0;
    g[gno].maxplot = 0;
    g[gno].p = NULL;
}

int overlay_graphs(int g1, int g2, int type)
{
    int i;
    
    if (g1 == g2) {
        return GRACE_EXIT_FAILURE;
    }
    if (is_valid_gno(g1) == FALSE || is_valid_gno(g2) == FALSE) {
        return GRACE_EXIT_FAILURE;
    }
    
    /* set identical viewports g2 in the controlling graph */
    g[g1].v = g[g2].v;
    switch (type) {
    case 0:
	g[g1].w = g[g2].w;
	for (i = 0; i < MAXAXES; i++) {
	    g[g1].t[i].active = FALSE;
	    g[g2].t[i].active = TRUE;
	}
	g[g1].t[1].tl_op = PLACEMENT_NORMAL;
	g[g2].t[1].tl_op = PLACEMENT_NORMAL;
	g[g1].t[1].t_op = PLACEMENT_BOTH;
	g[g2].t[1].t_op = PLACEMENT_BOTH;

	g[g1].t[0].tl_op = PLACEMENT_NORMAL;
	g[g2].t[0].tl_op = PLACEMENT_NORMAL;
	g[g1].t[0].t_op = PLACEMENT_BOTH;
	g[g2].t[0].t_op = PLACEMENT_BOTH;
	break;
    case 1:
	g[g1].w.xg1 = g[g2].w.xg1;
	g[g1].w.xg2 = g[g2].w.xg2;
	for (i = 0; i < MAXAXES; i++) {
	    if (i % 2 == 0) {
		g[g1].t[i].active = FALSE;
	    } else {
		g[g1].t[i].active = TRUE;
	    }
	}
	g[g2].t[1].tl_op = PLACEMENT_NORMAL;
	g[g1].t[1].tl_op = PLACEMENT_OPPOSITE;
	g[g2].t[1].t_op = PLACEMENT_NORMAL;
	g[g1].t[1].t_op = PLACEMENT_OPPOSITE;

	g[g2].t[0].tl_op = PLACEMENT_NORMAL;
	g[g1].t[0].tl_op = PLACEMENT_NORMAL;
	g[g2].t[0].t_op = PLACEMENT_BOTH;
	g[g1].t[0].t_op = PLACEMENT_BOTH;

	break;
    case 2:
	g[g1].w.yg1 = g[g2].w.yg1;
	g[g1].w.yg2 = g[g2].w.yg2;
	for (i = 0; i < MAXAXES; i++) {
	    if (i % 2 == 1) {
		g[g1].t[i].active = FALSE;
	    } else {
		g[g1].t[i].active = TRUE;
	    }
	}
	g[g2].t[0].tl_op = PLACEMENT_NORMAL;
	g[g1].t[0].tl_op = PLACEMENT_OPPOSITE;
	g[g2].t[0].t_op = PLACEMENT_NORMAL;
	g[g1].t[0].t_op = PLACEMENT_OPPOSITE;

	g[g2].t[1].tl_op = PLACEMENT_NORMAL;
	g[g1].t[1].tl_op = PLACEMENT_NORMAL;
	g[g2].t[1].t_op = PLACEMENT_BOTH;
	g[g1].t[1].t_op = PLACEMENT_BOTH;
	break;
    case 3:
	for (i = 0; i < MAXAXES; i++) {
	    g[g1].t[i].active = TRUE;
	    g[g2].t[i].active = TRUE;
	}
	g[g2].t[1].tl_op = PLACEMENT_NORMAL;
	g[g1].t[1].tl_op = PLACEMENT_OPPOSITE;
	g[g2].t[0].tl_op = PLACEMENT_NORMAL;
	g[g1].t[0].tl_op = PLACEMENT_OPPOSITE;
	g[g2].t[1].t_op = PLACEMENT_NORMAL;
	g[g1].t[1].t_op = PLACEMENT_OPPOSITE;
	g[g2].t[0].t_op = PLACEMENT_NORMAL;
	g[g1].t[0].t_op = PLACEMENT_OPPOSITE;
	break;
    }
    
    set_dirtystate();
    return GRACE_EXIT_SUCCESS;
}

int is_valid_setno(int gno, int setno)
{
    if (is_valid_gno(gno) == TRUE && setno >= 0 && setno < g[gno].maxplot) {
        return TRUE;
    } else {
        return FALSE;
    }
}

int is_set_hidden(int gno, int setno)
{
    if (is_valid_setno(gno, setno) == TRUE) {
        return g[gno].p[setno].hidden;
    } else {
        return FALSE;
    }
}

int set_set_hidden(int gno, int setno, int flag)
{
    if (is_valid_setno(gno, setno) == TRUE) {
        g[gno].p[setno].hidden = flag;
        set_dirtystate();
        return GRACE_EXIT_SUCCESS;
    } else {
        return GRACE_EXIT_FAILURE;
    }
}

int number_of_sets(int gno)
{
    if (is_valid_gno(gno) == TRUE) {
        return g[gno].maxplot;
    } else {
        return -1;
    }
}

int graph_world_stack_size(int gno)
{
    if (is_valid_gno(gno) == TRUE) {
        return g[gno].ws_top;
    } else {
        return -1;
    }
}

int get_world_stack_current(int gno)
{
    if (is_valid_gno(gno) == TRUE) {
        return g[gno].curw;
    } else {
        return -1;
    }
}

int get_world_stack_entry(int gno, int n, world_stack *ws)
{
    if (is_valid_gno(gno) == TRUE) {
        memcpy(ws, &g[gno].ws[n], sizeof(world_stack));
        return GRACE_EXIT_SUCCESS;
    } else {
        return GRACE_EXIT_FAILURE;
    }
}

int activate_tick_labels(int gno, int axis, int flag)
{
    if (is_valid_gno(gno) == TRUE && is_valid_axis(axis) == TRUE) {
        g[gno].t[axis].tl_flag = flag;
        set_dirtystate();
        return GRACE_EXIT_SUCCESS;
    } else {
        return GRACE_EXIT_FAILURE;
    }
}

int set_set_colors(int gno, int setno, int color)
{
    if (is_valid_setno(gno, setno) != TRUE) {
        return GRACE_EXIT_FAILURE;
    }
    if (color >= number_of_colors() || color < 0) {
        return GRACE_EXIT_FAILURE;
    }
    
    g[gno].p[setno].linepen.color = color;
    g[gno].p[setno].sympen.color = color;
    g[gno].p[setno].symfillpen.color = color;
    
    set_dirtystate();
    return GRACE_EXIT_SUCCESS;
}

static int project_version;

int get_project_version(void)
{
    return project_version;
}

int set_project_version(int version)
{
    if (version  > bi_version_id()) {
        project_version = bi_version_id();
        return GRACE_EXIT_FAILURE;
    } else {
        project_version = version;
        return GRACE_EXIT_SUCCESS;
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
    int gno, setno, naxis;
    double ext_x, ext_y;
    Page_geometry pg;
    
    if (version >= bi_version_id()) {
        return;
    }

    if (version < 40005) {
        pg.width  = 792;
        pg.height = 612;
        pg.dpi_x = 72.0;
        pg.dpi_y = 72.0;
        set_page_geometry(pg);
#ifndef NONE_GUI
        set_pagelayout(PAGE_FIXED);
#endif
    }

    if (get_project_version() < 50002) {
        setbgfill(TRUE);
    }
    
    if (version <= 40102 && get_pagelayout() == PAGE_FIXED) {
        get_page_viewport(&ext_x, &ext_y);
        rescale_viewport(ext_x, ext_y);
    }

    for (gno = 0; gno < number_of_graphs(); gno++) {
	if (version <= 40102) {
            g[gno].l.vgap -= 1;
        }
	for (setno = 0; setno < number_of_sets(gno); setno++) {
            if (version < 50003) {
                g[gno].p[setno].errbar.active = TRUE;
                switch (g[gno].p[setno].errbar.ptype) {
                case PLACEMENT_NORMAL:
                    g[gno].p[setno].errbar.ptype = PLACEMENT_OPPOSITE;
                    break;
                case PLACEMENT_OPPOSITE:
                    g[gno].p[setno].errbar.ptype = PLACEMENT_NORMAL;
                    break;
                default:
                    break;
                }
            }
            if (version < 50002) {
                g[gno].p[setno].errbar.length *= 2;
            }
            if (version < 50000) {
                switch (g[gno].p[setno].sym) {
                case SYM_NONE:
                    break;
                case SYM_DOT_OBS:
                    g[gno].p[setno].sym = SYM_CIRCLE;
                    g[gno].p[setno].symsize = 0.0;
                    g[gno].p[setno].symlines = 0;
                    g[gno].p[setno].symfillpen.pattern = 1;
                    break;
                default:
                    g[gno].p[setno].sym--;
                    break;
                }
            }
            if ((version < 40004 && g[gno].type != GRAPH_CHART) ||
                g[gno].p[setno].sympen.color == -1) {
                g[gno].p[setno].sympen.color = g[gno].p[setno].linepen.color;
            }
            if (version < 40200 || g[gno].p[setno].symfillpen.color == -1) {
                g[gno].p[setno].symfillpen.color = g[gno].p[setno].sympen.color;
            }
            
	    if (version <= 40102 && g[gno].type == GRAPH_CHART) {
                set_dataset_type(gno, setno, SET_BAR);
                g[gno].p[setno].sympen = g[gno].p[setno].linepen;
                g[gno].p[setno].symlines = g[gno].p[setno].lines;
                g[gno].p[setno].symlinew = g[gno].p[setno].linew;
                g[gno].p[setno].lines = 0;
                
                g[gno].p[setno].symfillpen = g[gno].p[setno].setfillpen;
                g[gno].p[setno].setfillpen.pattern = 0;
            }
	    if (version <= 40102 && g[gno].p[setno].type == SET_XYHILO) {
                g[gno].p[setno].symlinew = g[gno].p[setno].linew;
            }
        }
        for (naxis = 0; naxis < MAXAXES; naxis++) {
	    if (version <= 40102) {
                if ( (is_xaxis(naxis) && g[gno].xscale == SCALE_LOG) ||
                     (!is_xaxis(naxis) && g[gno].yscale == SCALE_LOG) ) {
                    g[gno].t[naxis].tmajor = pow(10.0, g[gno].t[naxis].tmajor);
                }
                
                /* TODO : world/view translation */
                g[gno].t[naxis].offsx = 0.0;
                g[gno].t[naxis].offsy = 0.0;
            }
	    if (version < 50000) {
	        /* There was no label_op in Xmgr */
                g[gno].t[naxis].label_op = g[gno].t[naxis].tl_op;
	        
                /* in xmgr, axis label placement was in x,y coordinates */
	        /* in Grace, it's parallel/perpendicular */
	        if(!is_xaxis(naxis)) {
	            fswap(&g[gno].t[naxis].label.x, &g[gno].t[naxis].label.y);
	        }
	        g[gno].t[naxis].label.y *= -1;
	    }
        }
    }
}
