/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2001,2002 Grace Development Team
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

#include "defines.h"
#include "utils.h"
#include "grace.h"
#include "objutils.h"
#include "graphs.h"
#include "protos.h"

static void wrap_graph_free(void *data)
{
    graph_free((graph *) data);
}

static void *wrap_graph_copy(void *data)
{
    return (void *) graph_copy((graph *) data);
}

Quark *project_new(Grace *grace)
{
    return quark_root(grace, QFlavorProject);
}

Project *project_data_new(void)
{
    Project *pr;
    int i;
    
    pr = xmalloc(sizeof(Project));
    if (!pr) {
        return NULL;
    }
    memset(pr, 0, sizeof(Project));
    
    pr->version_id  = bi_version_id();
    pr->description = NULL;
    
    pr->graphs  = storage_new(wrap_graph_free, wrap_graph_copy, NULL);
    if (!pr->graphs) {
        xfree(pr);
        return NULL;
    }
    
    pr->nr = 0;
    for (i = 0; i < MAXREGION; i++) {
        set_region_defaults(&pr->rg[i]);
    }
    
    pr->sformat = copy_string(NULL, "%.8g");

    pr->timestamp = copy_string(NULL, "");

    /* FIXME: #defines */
    pr->page_wpp = 792;
    pr->page_hpp = 612;
    
    pr->bgpen.color   = 0;
    pr->bgpen.pattern = 1;
    
    pr->docname = copy_string(NULL, NONAME);

    return pr;
}

void project_data_free(Project *pr)
{
    if (!pr) {
        return;
    }
    
    xfree(pr->description);
    
    storage_free(pr->graphs);
    
    xfree(pr->sformat);
    xfree(pr->docname);
    
    xfree(pr->timestamp);
    
    kill_all_regions();
    
    xfree(pr);
}


int project_get_version_id(const Quark *q)
{
    Project *pr = (Project *) q->data;
    return pr->version_id;
}

int project_set_version_id(Quark *q, int version_id)
{
    Project *pr = (Project *) q->data;
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
    Project *pr = (Project *) q->data;
    pr->version_id = bi_version_id();
}


void project_set_description(Quark *q, char *descr)
{
    Project *pr = (Project *) q->data;
    pr->description = copy_string(pr->description, descr);
    set_dirtystate();
}

char *project_get_description(const Quark *q)
{
    Project *pr = (Project *) q->data;
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

Storage *project_get_graphs(const Quark *q)
{
    Project *pr = (Project *) q->data;
    return pr->graphs;
}

char *project_get_sformat(const Quark *q)
{
    Project *pr = (Project *) q->data;
    return pr->sformat;
}

void project_set_sformat(Quark *q, const char *s)
{
    Project *pr = (Project *) q->data;
    pr->sformat = copy_string(pr->sformat, s);;
}
