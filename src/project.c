/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2001-2003 Grace Development Team
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

#include <config.h>

#include <string.h>

#include "defines.h"
#include "utils.h"
#include "grace.h"
#include "objutils.h"
#include "graphs.h"
#include "protos.h"

static int project_free_cb(Quark *pr, int etype, void *data)
{
    if (etype == QUARK_ETYPE_DELETE) {
        Grace *grace = pr->grace;
        if (pr == grace->project) {
            grace->project = NULL;
        }
    }
#ifndef NONE_GUI
    clean_graph_selectors(pr, etype, data);
#endif
    return RETURN_SUCCESS;
}

Quark *project_new(Grace *grace)
{
    Quark *q;
    q = quark_root(grace, QFlavorProject);
#ifndef NONE_GUI
    if (q) {
        quark_cb_set(q, project_free_cb, NULL);
    }
#endif
    return q;
}

Project *project_data_new(void)
{
    Project *pr;
    
    pr = xmalloc(sizeof(Project));
    if (!pr) {
        return NULL;
    }
    memset(pr, 0, sizeof(Project));
    
    pr->version_id  = bi_version_id();
    pr->description = NULL;
    
    pr->sformat = copy_string(NULL, "%.8g");

    pr->timestamp = copy_string(NULL, "");

    /* FIXME: #defines */
    pr->page_wpp = 792;
    pr->page_hpp = 612;
    
    pr->bgcolor  = 0;
    pr->bgfill   = TRUE;
    
    pr->docname = copy_string(NULL, NONAME);

    return pr;
}

void project_data_free(Project *pr)
{
    int i;
    
    if (!pr) {
        return;
    }
    
    xfree(pr->description);
    
    xfree(pr->sformat);
    xfree(pr->docname);
    
    xfree(pr->timestamp);
    
    for (i = 0; i < pr->nfonts; i++) {
        Fontdef *f = &pr->fontmap[i];
        xfree(f->fontname);
        xfree(f->fallback);
    }
    xfree(pr->fontmap);
    pr->nfonts = 0;
    
    xfree(pr);
}


int project_get_version_id(const Quark *q)
{
    Project *pr = project_get_data(q);
    return pr->version_id;
}

int project_set_version_id(Quark *q, int version_id)
{
    Project *pr = project_get_data(q);
    int software_version_id = bi_version_id();
    if (version_id > software_version_id) {
        pr->version_id = software_version_id;
        return RETURN_FAILURE;
    } else {
        pr->version_id = version_id;
        return RETURN_SUCCESS;
    }
}

void project_reset_version(Quark *q)
{
    Project *pr = project_get_data(q);
    pr->version_id = bi_version_id();
}


void project_set_description(Quark *q, char *descr)
{
    Project *pr = project_get_data(q);
    pr->description = copy_string(pr->description, descr);
    set_dirtystate();
}

char *project_get_description(const Quark *q)
{
    Project *pr = project_get_data(q);
    return pr->description;
}

/*
 * dirtystate routines
 */
void project_set_dirtystate(Quark *q)
{
    update_timestamp(NULL);
    update_app_title(q);

/*
 * TODO:
 * 	if ( (dirtystate > SOME_LIMIT) || 
 *           (current_time - autosave_time > ANOTHER_LIMIT) ) {
 * 	    autosave();
 * 	}
 */
}

void project_clear_dirtystate(Quark *q)
{
    update_app_title(q);
}

typedef struct {
    int ngraphs;
    Quark **graphs;
} graph_hook_t;

static int graph_hook(Quark *q, void *udata, QTraverseClosure *closure)
{
    graph_hook_t *p = (graph_hook_t *) udata;
    
    if (q->fid == QFlavorGraph) {
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

char *project_get_sformat(const Quark *q)
{
    Project *pr = project_get_data(q);
    return pr->sformat;
}

void project_set_sformat(Quark *q, const char *s)
{
    Project *pr = project_get_data(q);
    pr->sformat = copy_string(pr->sformat, s);;
}

Project *project_get_data(const Quark *q)
{
    if (q && q->fid == QFlavorProject) {
        return (Project *) q->data;
    } else {
        return NULL;
    }
}
