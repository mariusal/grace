/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2000 Grace Development Team
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
#include "grace.h"
#include "objutils.h"
#include "utils.h"
#include "protos.h"

static defaults d_d =
{1, 0, 1, 1, 1, 1.0, 0, 1.0};

/* defaults layout
    int color;
    int bgcolor;
    int pattern;
    int lines;
    double linew;
    double charsize;
    int font;
    double symsize;
*/

static void wrap_object_free(void *data)
{
    object_free((DObject *) data);
}

Project *project_new(void)
{
    Project *pr;
    int i;
    
    pr = xmalloc(sizeof(Project));
    if (!pr) {
        return NULL;
    }
    
    pr->graphs  = NULL;
    
    pr->objects = storage_new(wrap_object_free, NULL);
    if (!pr->objects) {
        xfree(pr);
        return NULL;
    }

    pr->grdefaults = d_d;
        
    pr->nr = 0;
    for (i = 0; i < MAXREGION; i++) {
        set_region_defaults(&pr->rg[i]);
    }
    
    pr->scrolling_islinked = FALSE;
    pr->scrollper = 0.05;
    pr->shexper = 0.05;
    
    pr->sformat = copy_string(NULL, "%16.8g");

    set_default_string(&pr->timestamp);
    pr->timestamp.offset.x = 0.03;
    pr->timestamp.offset.y = 0.03;
    
    return pr;
}

void project_free(Project *pr)
{
    if (!pr) {
        return;
    }
    
    storage_free(pr->objects);
    
    xfree(pr->sformat);
    
    /* FIXME graphs */
    /* FIXME regions */
    /* FIXME timestamp */
    
    xfree(pr);
}


GUI *gui_new(void)
{
    GUI *gui;
    
    gui = xmalloc(sizeof(GUI));
    if (!gui) {
        return NULL;
    }

    gui->inwin           = FALSE;
    gui->invert          = TRUE;
    gui->auto_redraw     = TRUE;
    gui->allow_dc        = TRUE;
    gui->focus_policy    = FOCUS_CLICK;
    gui->draw_focus_flag = TRUE;
    gui->monomode        = FALSE;
    gui->noask           = FALSE;
    
    return gui;
}

void gui_free(GUI *gui)
{
    xfree(gui);
}


RunTime *runtime_new(void)
{
    RunTime *rt;
    
    rt = xmalloc(sizeof(RunTime));
    if (!rt) {
        return NULL;
    }
    
    rt->tdevice = 0;
    rt->hdevice = 0;
    
    rt->target_set.gno   = -1;
    rt->target_set.setno = -1;
    
    rt->nlfit = xmalloc(sizeof(NLFit));
    if (!rt->nlfit) {
        xfree(rt);
        return NULL;
    }
    rt->nlfit->title   = NULL;
    rt->nlfit->formula = NULL;
    reset_nonl(rt->nlfit);
    
    rt->timer_delay = 200;

    rt->autoscale_onread = AUTOSCALE_XY;

    /* FIXME curstuff */
    rt->curtype   = SET_XY;
    rt->cursource = SOURCE_DISK;

    rt->resfp = NULL;
    
    return rt;
}

void runtime_free(RunTime *rt)
{
    /* FIXME nonlfit */
    
    xfree(rt);
}

Grace *grace_new(void)
{
    Grace *grace;
    
    grace = xmalloc(sizeof(Grace));
    if (!grace) {
        return NULL;
    }
    
    grace->rt = runtime_new();
    if (!grace->rt) {
        xfree(grace);
        return NULL;
    }
    
    grace->gui = gui_new();
    if (!grace->gui) {
        runtime_free(grace->rt);
        xfree(grace);
        return NULL;
    }
    
    grace->project = project_new();
    if (!grace->project) {
        gui_free(grace->gui);
        runtime_free(grace->rt);
        xfree(grace);
        return NULL;
    }
    
    return grace;
}

void grace_free(Grace *grace)
{
    if (!grace) {
        return;
    }
    
    project_free(grace->project);
    gui_free(grace->gui);
    runtime_free(grace->rt);
    
    xfree(grace);
}
