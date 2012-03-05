/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2002 Grace Development Team
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
 * Misc properties
 *
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>

#include "globals.h"
#include "utils.h"
#include "core_utils.h"
#include "motifinc.h"
#include "xprotos.h"

static Widget props_frame;

/*
 * Panel item declarations
 */
static Widget noask_item;

static OptionStructure *graph_focus_choice_item;
static Widget graph_drawfocus_choice_item;

static Widget cursor_type_item;
static SpinStructure *max_path_item;
static Widget safe_mode_item;
static Widget scrollper_item;
static Widget shexper_item;

#if defined WITH_XMHTML
static Widget force_external_viewer_item;
#endif

/*
 * Event and Notify proc declarations
 */
static int props_define_notify_proc(void *data);

void create_props_frame(Widget but, void *data)
{
    set_wait_cursor();

    if (props_frame == NULL) {
        Widget fr, rc1;

	props_frame = CreateDialog(app_shell, "Preferences");

	fr = CreateFrame(props_frame, "Responsiveness");
	FormAddVChild(props_frame, fr);
        rc1 = CreateVContainer(fr);

	noask_item = CreateToggleButton(rc1, "Don't ask questions");

	graph_focus_choice_item = CreateOptionChoiceVA(rc1,
            "Graph focus switch",
	    "Button press",  FOCUS_CLICK,
	    "As set",        FOCUS_SET,
	    "Follows mouse", FOCUS_FOLLOWS,
	    NULL);

        graph_drawfocus_choice_item =
            CreateToggleButton(rc1, "Display focus markers");
	cursor_type_item = CreateToggleButton(rc1, "Crosshair cursor");
#if defined WITH_XMHTML
	force_external_viewer_item = CreateToggleButton(rc1,
            "Use external help viewer for local documents");
#endif        
	fr = CreateFrame(props_frame, "Restrictions");
	FormAddVChild(props_frame, fr);
        rc1 = CreateVContainer(fr);
	max_path_item = CreateSpinChoice(rc1,
            "Max drawing path length:", 6, SPIN_TYPE_INT, 0.0, 1.0e6, 1000);
	safe_mode_item = CreateToggleButton(rc1, "Run in safe mode");
        
	fr = CreateFrame(props_frame, "Scroll/zoom");
        rc1 = CreateVContainer(fr);
	scrollper_item = CreateScale(rc1, "Scroll %", 0, 200, 20);
	shexper_item   = CreateScale(rc1, "Zoom %",   0, 200, 20);

	CreateAACDialog(props_frame, fr, props_define_notify_proc, NULL);
    }
    
    update_props_items();
    
    DialogRaise(props_frame);
    unset_wait_cursor();
}

void update_props_items(void)
{
    int itest = 0;
    int iv;
    
    if (props_frame) {
        GUI *gui = gapp->gui;
	ToggleButtonSetState(noask_item, gui->noask);

	if (gui->focus_policy == FOCUS_SET) {
	    itest = 1;
	} else if (gui->focus_policy == FOCUS_CLICK) {
	    itest = 0;
	} else if (gui->focus_policy == FOCUS_FOLLOWS) {
	    itest = 2;
	}
	SetOptionChoice(graph_focus_choice_item, itest);
	ToggleButtonSetState(graph_drawfocus_choice_item, gui->draw_focus_flag);

	ToggleButtonSetState(cursor_type_item, gui->crosshair_cursor);
#if defined WITH_XMHTML
	ToggleButtonSetState(force_external_viewer_item, gui->force_external_viewer);
#endif
	SpinChoiceSetValue(max_path_item,
            (double) get_max_path_limit(grace_get_canvas(gapp->grace)));
	ToggleButtonSetState(safe_mode_item, gapp->rt->safe_mode);
	iv = (int) rint(100*gapp->rt->scrollper);
	ScaleSetValue(scrollper_item, iv);
	iv = (int) rint(100*gapp->rt->shexper);
	ScaleSetValue(shexper_item, iv);
    }
}

static int props_define_notify_proc(void *data)
{
    GUI *gui = gapp->gui;
    
    gui->noask = ToggleButtonGetState(noask_item);

    switch (GetOptionChoice(graph_focus_choice_item)) {
    case 0:
	gui->focus_policy = FOCUS_CLICK;
	break;
    case 1:
	gui->focus_policy = FOCUS_SET;
	break;
    case 2:
	gui->focus_policy = FOCUS_FOLLOWS;
	break;
    }
    gui->draw_focus_flag = ToggleButtonGetState(graph_drawfocus_choice_item);

    gui->crosshair_cursor = ToggleButtonGetState(cursor_type_item);
#if defined WITH_XMHTML
    gui->force_external_viewer = ToggleButtonGetState(force_external_viewer_item);
#endif
    set_max_path_limit(grace_get_canvas(gapp->grace), (int) SpinChoiceGetValue(max_path_item));
    gapp->rt->safe_mode = ToggleButtonGetState(safe_mode_item);
    gapp->rt->scrollper = (double) ScaleGetValue(scrollper_item)/100.0;
    gapp->rt->shexper   = (double) ScaleGetValue(shexper_item)/100.0;
    
    xdrawgraph(gapp->gp);
    
    return RETURN_SUCCESS;
}
