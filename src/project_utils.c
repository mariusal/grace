/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2001-2005 Grace Development Team
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

#include <config.h>

#include "core_utils.h"

typedef struct {
    int ngraphs;
    Quark **graphs;
} graph_hook_t;

static int graph_hook(Quark *q, void *udata, QTraverseClosure *closure)
{
    graph_hook_t *p = (graph_hook_t *) udata;
    
    if (quark_fid_get(q) == QFlavorGraph) {
        p->ngraphs++;
        p->graphs = xrealloc(p->graphs, p->ngraphs*SIZEOF_VOID_P);
        p->graphs[p->ngraphs - 1] = q;
    }
    
    return TRUE;
}

int project_get_graphs(Quark *q, Quark ***graphs)
{
    graph_hook_t p;
    
    p.ngraphs = 0;
    p.graphs  = NULL;
    
    quark_traverse(q, graph_hook, &p);
    
    *graphs = p.graphs;
    
    return p.ngraphs;
}

int project_get_viewport(const Quark *project, double *vx, double *vy)
{
    Project *pr = project_get_data(project);
    if (pr && pr->page_wpp > 0 && pr->page_hpp > 0) {
        if (pr->page_wpp < pr->page_hpp) {
            *vy = (double) pr->page_hpp/pr->page_wpp;
            *vx = 1.0;
        } else {
            *vx = (double) pr->page_wpp/pr->page_hpp;
            *vy = 1.0;
        }
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}
