/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2001 Grace Development Team
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

Project *project_new(void)
{
    Project *pr;
    int i;
    
    pr = xmalloc(sizeof(Project));
    if (!pr) {
        return NULL;
    }
    
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

    set_default_string(&pr->timestamp);
    pr->timestamp.offset.x = 0.03;
    pr->timestamp.offset.y = 0.03;

    /* FIXME: #defines */
    pr->page_wpp = 792;
    pr->page_hpp = 612;
    
    pr->docname = copy_string(NULL, NONAME);

    pr->dirtystate = 0;
    pr->dirtystate_lock = FALSE;
    
    return pr;
}

void project_free(Project *pr)
{
    if (!pr) {
        return;
    }
    
    xfree(pr->description);
    
    storage_free(pr->graphs);
    
    xfree(pr->sformat);
    xfree(pr->docname);
    
    /* FIXME regions */
    /* FIXME timestamp */
    
    xfree(pr);
}


int project_get_version_id(Project *pr)
{
    return pr->version_id;
}

int project_set_version_id(Project *pr, int version_id)
{
    int software_version_id = bi_version_id();
    if (version_id > software_version_id) {
        pr->version_id = software_version_id;
        return RETURN_FAILURE;
    } else {
        pr->version_id = version_id;
        return RETURN_SUCCESS;
    }
}

void project_reset_version(Project *pr)
{
    pr->version_id = bi_version_id();
}


void project_set_description(Project *pr, char *descr)
{
    pr->description = copy_string(pr->description, descr);
    set_dirtystate();
}

char *project_get_description(Project *pr)
{
    return pr->description;
}

/*
 * dirtystate routines
 */
void project_set_dirtystate(Project *pr)
{
    if (pr->dirtystate_lock == FALSE) {
        pr->dirtystate++;
        update_timestamp();
        update_app_title();

/*
 * TODO:
 * 	if ( (dirtystate > SOME_LIMIT) || 
 *           (current_time - autosave_time > ANOTHER_LIMIT) ) {
 * 	    autosave();
 * 	}
 */
    }
}

void project_clear_dirtystate(Project *pr)
{
    pr->dirtystate = 0;
    pr->dirtystate_lock = FALSE;
    update_app_title();
}

void project_lock_dirtystate(Project *pr, int flag)
{
    pr->dirtystate_lock = flag;
}

int project_is_dirtystate(Project *pr)
{
    return (pr->dirtystate ? TRUE:FALSE);
}

Storage *project_get_graphs(Project *pr)
{
    return pr->graphs;
}
