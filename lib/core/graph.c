/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1996-2004 Grace Development Team
 * 
 * Maintained by Evgeny Stambulchik
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

#define ADVANCED_MEMORY_HANDLERS
#include "grace/coreP.h"

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

    g->type = GRAPH_XY;
    g->xinvert = FALSE;
    g->yinvert = FALSE;
    g->xyflip = FALSE;
    g->stacked = FALSE;
    g->bargap = 0.0;
    g->znorm  = 1.0;
    g->xscale = SCALE_NORMAL;
    g->yscale = SCALE_NORMAL;

    g->locator.type = GLOCATOR_TYPE_XY;
    g->locator.origin.x = g->locator.origin.y = 0.0;
    g->locator.pointset = FALSE;
    g->locator.fx.type = FORMAT_GENERAL;
    g->locator.fy.type = FORMAT_GENERAL;
    g->locator.fx.prec = 6;
    g->locator.fy.prec = 6;

    memcpy(&g->w, &d_w, sizeof(world));
}

graph *graph_data_new(AMem *amem)
{
    graph *g;
    
    g = amem_malloc(amem, sizeof(graph));
    if (g) {
        memset(g, 0, sizeof(graph));
        set_default_graph(g);
    }
    return g;
}

void graph_data_free(AMem *amem, graph *g)
{
    amem_free(amem, g);
}

graph *graph_data_copy(AMem *amem, graph *g)
{
    graph *g_new;
    
    if (!g) {
        return NULL;
    }
    
    g_new = amem_malloc(amem, sizeof(graph));
    if (!g_new) {
        return NULL;
    }
    
    memcpy(g_new, g, sizeof(graph));
    
    return g_new;
}

int graph_qf_register(QuarkFactory *qfactory)
{
    QuarkFlavor qf = {
        QFlavorGraph,
        (Quark_data_new) graph_data_new,
        (Quark_data_free) graph_data_free,
        (Quark_data_copy) graph_data_copy
    };

    return quark_flavor_add(qfactory, &qf);
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
        
        if (quark_is_active(q)) {
            Project *project = (Project *) udata;

            if (project->cg != q) {
                project->cg = q;
                return FALSE;
            }
        }
    } else
    if (q->fid == QFlavorFrame && !quark_is_active(q)) {
        closure->descend = FALSE;
    }

    return TRUE;
}

static int graph_cb(Quark *gr, int etype, void *data)
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
    } else
    if (etype == QUARK_ETYPE_REPARENT) {
        update_graph_ccache(gr);
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
    update_graph_ccache(g);
    if (g) {
        Project *pr = project_get_data(get_parent_project(q));
        if (pr && pr->cg == NULL) {
            pr->cg = g;
        }
        quark_cb_add(g, graph_cb, NULL);
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
        case GRAPH_PIE:
            break;
        case GRAPH_FIXED:
            /* Only linear axis scales are allowed in Fixed graphs */
            g->xscale = SCALE_NORMAL;
            g->yscale = SCALE_NORMAL;
            break;
        case GRAPH_POLAR:
            /* Only linear axis scales are allowed in Polar graphs */
            g->xscale = SCALE_NORMAL;
            g->yscale = SCALE_NORMAL;
            g->yinvert = FALSE;
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

        update_graph_ccache(gr);

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
        double dx, dy;
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
        if (g->type == GRAPH_POLAR && w->yg2 <= 0.0) {
            errmsg("World Rho-max <= 0.0");
            return RETURN_FAILURE;
        }
        if (g->xscale == SCALE_LOG && w->xg1 <= 0.0) {
            g->xscale = SCALE_NORMAL;
        }
        if (g->yscale == SCALE_LOG && w->yg1 <= 0.0) {
            g->yscale = SCALE_NORMAL;
        }
        if (g->xscale == SCALE_REC && sign(w->xg1) != sign(w->xg2)) {
            g->xscale = SCALE_NORMAL;
        }
        if (g->yscale == SCALE_REC && sign(w->yg1) != sign(w->yg2)) {
            g->yscale = SCALE_NORMAL;
        }
        if (g->xscale == SCALE_LOGIT && (w->xg1 <= 0.0 || w->xg2 >= 1.0)) {
            g->xscale = SCALE_NORMAL;
        }
        if (g->yscale == SCALE_LOGIT && (w->yg1 <= 0.0 || w->yg2 >= 1.0)) {
            g->yscale = SCALE_NORMAL;
        }

        g->w = *w;
        update_graph_ccache(gr);
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

typedef struct {
    int type;
    int scale;
} axis_hook_t;

static int axis_hook(Quark *q, void *udata, QTraverseClosure *closure)
{
    if (q->fid == QFlavorAGrid) {
        tickmarks *t = axisgrid_get_data(q);
        axis_hook_t *hdata = (axis_hook_t *) udata;
        closure->descend = FALSE;
        
        if (hdata && t && t->type == hdata->type) {
            if (hdata->scale == SCALE_LOG) {
                t->tmajor = 10.0;
                t->nminor = 9;
            } else {
                t->nminor = 1;
            }
        }
    }

    return TRUE;
}

int graph_set_xscale(Quark *gr, int scale)
{
    graph *g = graph_get_data(gr);
    if (g) {
        if (g->xscale != scale) {
            axis_hook_t hdata;
            
            if (g->type == GRAPH_FIXED && scale != SCALE_NORMAL) {
                errmsg("Only linear axis scales are allowed in Fixed graphs");
                return RETURN_FAILURE;
            }
            if (g->type == GRAPH_POLAR && scale != SCALE_NORMAL) {
                errmsg("Only linear axis scales are allowed in Polar plots");
                return RETURN_FAILURE;
            }
            if (scale == SCALE_LOG && g->w.xg2 <= 0.0) {
                errmsg("Can't set log scale when World X-max <= 0.0");
                return RETURN_FAILURE;
            }
            if (scale == SCALE_REC && sign(g->w.xg1) != sign(g->w.xg2)) {
                errmsg("Can't set Rec scale when X-axis contains 0");
                return RETURN_FAILURE;
            }
            if (scale == SCALE_LOGIT) {
                if (g->w.xg1 <= 0.0) {
                    errmsg("World X-min <= 0.0");
                    return RETURN_FAILURE;
                }
                if (g->w.xg2 >= 1.0) {
                    errmsg("World X-max >= 1.0");
                    return RETURN_FAILURE;
                }
	    }    

            if (scale == SCALE_LOG && g->w.xg1 <= 0.0) {
                g->w.xg1 = g->w.xg2/1.0e3;
            }
            g->xscale = scale;
            update_graph_ccache(gr);

            hdata.scale = scale;
            hdata.type = AXIS_TYPE_X;
            quark_traverse(gr, axis_hook, &hdata);

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
            axis_hook_t hdata;
            
            if (g->type == GRAPH_FIXED && scale != SCALE_NORMAL) {
                errmsg("Only linear axis scales are allowed in Fixed graphs");
                return RETURN_FAILURE;
            }
            if (g->type == GRAPH_POLAR && scale != SCALE_NORMAL) {
                errmsg("Only linear axis scales are allowed in Polar plots");
                return RETURN_FAILURE;
            }
            if (scale == SCALE_LOG && g->w.yg2 <= 0) {
                errmsg("Can't set log scale when World Y-max <= 0.0");
                return RETURN_FAILURE;
            }
            if (scale == SCALE_REC && sign(g->w.yg1) != sign(g->w.yg2)) {
                errmsg("Can't set Rec scale when Y-axis contains 0");
                return RETURN_FAILURE;
            }
            if (scale == SCALE_LOGIT) {
                if (g->w.yg1 <= 0) {
                    errmsg("World Y-min <= 0.0");
                    return RETURN_FAILURE;
                }
                if (g->w.yg2 >= 1) {
                    errmsg("World Y-max >= 1.0");
                    return RETURN_FAILURE;
                }
	    }    

            if (scale == SCALE_LOG && g->w.yg1 <= 0.0) {
                g->w.yg1 = g->w.yg2/1.0e3;
            }
            g->yscale = scale;
            update_graph_ccache(gr);

            hdata.scale = scale;
            hdata.type = AXIS_TYPE_Y;
            quark_traverse(gr, axis_hook, &hdata);

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
        update_graph_ccache(gr);
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
        if (g->type == GRAPH_POLAR && flag != FALSE) {
            errmsg("Can't set Y scale inverted in Polar plot");
            return RETURN_FAILURE;
        }
        g->yinvert = flag;
        update_graph_ccache(gr);
        quark_dirtystate_set(gr, TRUE);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}
