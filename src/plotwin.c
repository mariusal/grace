/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1996-2002 Grace Development Team
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
 * Plot properties
 *
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>

#include "globals.h"
#include "draw.h"
#include "utils.h"
#include "protos.h"
#include "motifinc.h"

static Widget plot_frame;

/*
 * Panel item declarations
 */
static Widget bgpen_item;

static Widget instantupdate_item;

static int plot_define_notify_proc(void *data);
static void update_plot_items(void);


void create_plot_frame_cb(void *data)
{
    create_plot_frame();
}

void create_plot_frame(void)
{
    set_wait_cursor();
    
    if (plot_frame == NULL) {
        Widget panel, fr, rc, menubar, menupane;
    
        plot_frame = CreateDialogForm(app_shell, "Plot appearance");

        menubar = CreateMenuBar(plot_frame);
        AddDialogFormChild(plot_frame, menubar);
        ManageChild(menubar);

        menupane = CreateMenu(menubar, "File", 'F', FALSE);
        CreateMenuCloseButton(menupane, plot_frame);

        menupane = CreateMenu(menubar, "Options", 'O', FALSE);
        instantupdate_item = CreateMenuToggle(menupane, "Instantaneous update",
                            'u', NULL, NULL);
        SetToggleButtonState(instantupdate_item, grace->gui->instant_update);

        menupane = CreateMenu(menubar, "Help", 'H', TRUE);
        CreateMenuHelpButton(menupane, "On plot appearance", 'p',
            plot_frame, "doc/UsersGuide.html#plot-appearance");

        panel = CreateVContainer(plot_frame);

	fr = CreateFrame(panel, "Page background");
        rc = CreateHContainer(fr);
	bgpen_item = CreatePenChoice(rc, "Fill");
        /* FIXME:!!! */
        /* AddPenChoiceCB(bgpen_item, tb_plot_cb, bgpen_item); */

	CreateAACDialog(plot_frame, panel, plot_define_notify_proc, NULL);
    }
    update_plot_items();
    
    RaiseWindow(GetParent(plot_frame));
    unset_wait_cursor();
}

static void update_plot_items(void)
{
    if (plot_frame) {
	Project *pr = (Project *) grace->project->data;
        SetPenChoice(bgpen_item, &pr->bgpen);
    }
}

static int plot_define_notify_proc(void *data)
{
    if (!GetToggleButtonState(instantupdate_item) && data != NULL) {
        return RETURN_SUCCESS;
    }
    
    if (data == bgpen_item || data == NULL) {
	Project *pr = (Project *) grace->project->data;
        GetPenChoice(bgpen_item, &pr->bgpen);
    }
    
    set_dirtystate();
    xdrawgraph();
    
    return RETURN_SUCCESS;
}
