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

#include <stdlib.h>
#include <unistd.h>

#include "defines.h"
#include "grace.h"
#include "utils.h"
#include "graphs.h"
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
    char *s;
    
    rt = xmalloc(sizeof(RunTime));
    if (!rt) {
        return NULL;
    }

    /* allocatables */
    rt->grace_home   = NULL;
    rt->print_cmd    = NULL;
    rt->grace_editor = NULL;
    rt->help_viewer  = NULL;
    rt->workingdir   = NULL;
    rt->username     = NULL;
    rt->userhome     = NULL;
    
    rt->nlfit        = NULL;
    
    /* grace home directory */
    if ((s = getenv("GRACE_HOME")) == NULL) {
	s = bi_home();
    }
    rt->grace_home = copy_string(NULL, s);

    /* print command */
    if ((s = getenv("GRACE_PRINT_CMD")) == NULL) {
	s = bi_print_cmd();
    }
    rt->print_cmd = copy_string(NULL, s);
    /* if no print command defined, print to file by default */
    if (is_empty_string(rt->print_cmd)) {
        rt->ptofile = TRUE;
    } else {
        rt->ptofile = FALSE;
    }

    /* editor */
    if ((s = getenv("GRACE_EDITOR")) == NULL) {
	s = bi_editor();
    }
    rt->grace_editor = copy_string(NULL, s);

    /* html viewer */
    if ((s = getenv("GRACE_HELPVIEWER")) == NULL) {
	s = bi_helpviewer();
    }
    rt->help_viewer = copy_string(NULL, s);

    /* working directory */
    rt->workingdir = xmalloc(GR_MAXPATHLEN);
    getcwd(rt->workingdir, GR_MAXPATHLEN - 1);
    if (rt->workingdir[strlen(rt->workingdir)-1] != '/') {
        rt->workingdir = concat_strings(rt->workingdir, "/");
    }

    /* username */
    s = getenv("LOGNAME");
    if (is_empty_string(s)) {
        s = getlogin();
        if (is_empty_string(s)) {
            s = "a user";
        }
    }
    rt->username = copy_string(NULL, s);

    /* userhome */
    if ((s = getenv("HOME")) == NULL) {
        s = "/";
    }
    rt->userhome = copy_string(NULL, s);
    if (rt->userhome[strlen(rt->userhome) - 1] != '/') {
        rt->userhome = concat_strings(rt->userhome, "/");
    }

    rt->nlfit = xmalloc(sizeof(NLFit));

    if (!rt->grace_home   ||
        !rt->print_cmd    ||
        !rt->grace_editor ||
        !rt->help_viewer  ||
        !rt->workingdir   ||
        !rt->username     ||
        !rt->userhome     ||
        !rt->nlfit) {
        runtime_free(rt);
        return NULL;
    }
    
    rt->tdevice = 0;
    rt->hdevice = 0;
    
    rt->target_set.gno   = -1;
    rt->target_set.setno = -1;
    
    rt->nlfit->title   = NULL;
    rt->nlfit->formula = NULL;
    reset_nonl(rt->nlfit);
    
    rt->timer_delay = 200;

    rt->autoscale_onread = AUTOSCALE_XY;

    /* FIXME curstuff */
    rt->curtype   = SET_XY;
    rt->cursource = SOURCE_DISK;

    rt->grdefaults = d_d;
        
    rt->scrolling_islinked = FALSE;
    rt->scrollper = 0.05;
    rt->shexper = 0.05;
    
    rt->resfp = NULL;

    rt->emergency_save = FALSE;
    rt->interrupts = 0;

#ifdef DEBUG
    rt->debuglevel = 0;
#endif

    return rt;
}

void runtime_free(RunTime *rt)
{
    xfree(rt->grace_home);
    xfree(rt->print_cmd);
    xfree(rt->grace_editor);
    xfree(rt->help_viewer);
    xfree(rt->workingdir);
    xfree(rt->username);
    xfree(rt->userhome);
    
    /* FIXME nonlfit */
    xfree(rt->nlfit);
    
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
