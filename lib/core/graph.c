/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
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

#include <string.h>

#include "grace/core.h"

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

Quark *get_parent_graph(const Quark *child)
{
    Quark *p = (Quark *) child;
    
    while (p) {
        p = quark_parent_get(p);
        if (p && p->fid == QFlavorGraph) {
            return p;
        }
    }
    
    return NULL;
}

static int hook(Quark *q, void *udata, QTraverseClosure *closure)
{
    if (q->fid == QFlavorGraph) {
        closure->descend = FALSE;
        
        if (graph_is_active(q)) {
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

int graph_set_type(Quark *gr, int gtype)
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
            errmsg("Internal error in graph_set_type()");
            return RETURN_FAILURE;
        }
        g->type = gtype;
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

GLocator *graph_get_locator(const Quark *gr)
{
    graph *g = graph_get_data(gr);
    if (g) {
        return &g->locator;
    } else {
        return NULL;
    }
}

int graph_get_world(const Quark *gr, world *w)
{
    graph *g = graph_get_data(gr);
    if (g) {
        memcpy(w, &g->w, sizeof(world));
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int graph_set_locator(Quark *gr, const GLocator *locator)
{
    graph *g = graph_get_data(gr);
    if (g) {

        memcpy(&g->locator, locator, sizeof(GLocator));
        quark_dirtystate_set(gr, TRUE);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int graph_set_world(Quark *gr, const world *w)
{
    graph *g = graph_get_data(gr);
    if (g) {

        g->w = *w;
        quark_dirtystate_set(gr, TRUE);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int graph_set_xyflip(Quark *gr, int xyflip)
{
    graph *g = graph_get_data(gr);
    if (g) {

        g->xyflip = xyflip;

        quark_dirtystate_set(gr, TRUE);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int graph_get_xyflip(const Quark *gr)
{
    graph *g = graph_get_data(gr);
    if (g) {

        return g->xyflip;
    } else {
        return FALSE;
    }
}

int graph_is_active(const Quark *gr)
{
    graph *g = graph_get_data(gr);
    if (g) {
        return g->active;
    } else {
        return FALSE;
    }
}

int graph_set_active(Quark *gr, int flag)
{
    graph *g = graph_get_data(gr);
    if (g) {
        g->active = flag;
        quark_dirtystate_set(gr, TRUE);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int graph_set_stacked(Quark *gr, int flag)
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

int graph_is_stacked(const Quark *gr)
{
    graph *g = graph_get_data(gr);
    if (g) {
        return g->stacked;
    } else {
        return FALSE;
    }
}

double graph_get_bargap(const Quark *gr)
{
    graph *g = graph_get_data(gr);
    if (g) {
        return g->bargap;
    } else {
        return 0.0;
    }
}

int graph_set_bargap(Quark *gr, double bargap)
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

int graph_get_type(const Quark *gr)
{
    graph *g = graph_get_data(gr);
    if (g) {
        return g->type;
    } else {
        return -1;
    }
}

int graph_get_xscale(const Quark *gr)
{
    graph *g = graph_get_data(gr);
    if (g) {
        return g->xscale;
    } else {
        return -1;
    }
}

int graph_get_yscale(const Quark *gr)
{
    graph *g = graph_get_data(gr);
    if (g) {
        return g->yscale;
    } else {
        return -1;
    }
}

int graph_set_xscale(Quark *gr, int scale)
{
    graph *g = graph_get_data(gr);
    if (g) {
        if (g->xscale != scale) {
            g->xscale = scale;
#if 0
            for (naxis = 0; naxis < MAXAXES; naxis++) {
                if (axis_is_x(naxis)) {
                    tickmarks *t;
                    t = graph_get_tickmarks(gr, naxis);
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

int graph_set_yscale(Quark *gr, int scale)
{
    graph *g = graph_get_data(gr);
    if (g) {
        if (g->yscale != scale) {
            g->yscale = scale;
#if 0
            for (naxis = 0; naxis < MAXAXES; naxis++) {
                if (axis_is_y(naxis)) {
                    tickmarks *t;
                    t = graph_get_tickmarks(gr, naxis);
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

int graph_set_znorm(Quark *gr, double norm)
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

double graph_get_znorm(const Quark *gr)
{
    graph *g = graph_get_data(gr);
    if (g) {
        return g->znorm;
    } else {
        return 0.0;
    }
}

int graph_is_xinvert(const Quark *gr)
{
    graph *g = graph_get_data(gr);
    if (g) {
        return g->xinvert;
    } else {
        return FALSE;
    }
}

int graph_is_yinvert(const Quark *gr)
{
    graph *g = graph_get_data(gr);
    if (g) {
        return g->yinvert;
    } else {
        return FALSE;
    }
}

int graph_set_xinvert(Quark *gr, int flag)
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

int graph_set_yinvert(Quark *gr, int flag)
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
