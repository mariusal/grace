/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
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
 * Misc properties
 *
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>

#include "globals.h"
#include "grace/canvas.h"
#include "utils.h"
#include "graphs.h"
#include "motifinc.h"
#include "protos.h"

extern int cursortype;

#if defined WITH_XMHTML || defined WITH_LIBHELP
extern int force_external_viewer;
#endif

static Widget props_frame;

/*
 * Panel item declarations
 */
#ifdef DEBUG
static SpinStructure *debug_item;
#endif
static Widget noask_item;
static Widget dc_item;

static OptionStructure *graph_focus_choice_item;
static Widget graph_drawfocus_choice_item;

static Widget autoredraw_type_item;
static Widget cursor_type_item;
static SpinStructure *max_path_item;
static Widget safe_mode_item;
static Widget scrollper_item;
static Widget shexper_item;

#if defined WITH_XMHTML || defined WITH_LIBHELP
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

	props_frame = CreateDialogForm(app_shell, "Preferences");

	fr = CreateFrame(props_frame, "Responsiveness");
        AddDialogFormChild(props_frame, fr);
        rc1 = CreateVContainer(fr);

#ifdef DEBUG
	debug_item = CreateSpinChoice(rc1,
            "Debug level:", 1, SPIN_TYPE_INT, 0.0, 8.0, 1.0);
#endif
	noask_item = CreateToggleButton(rc1, "Don't ask questions");
	dc_item = CreateToggleButton(rc1, "Allow double clicks on canvas");

	graph_focus_choice_item = CreatePanelChoice(rc1,
            "Graph focus switch",
	    4,
	    "Button press",
	    "As set",
	    "Follows mouse",
	    NULL);

        graph_drawfocus_choice_item =
            CreateToggleButton(rc1, "Display focus markers");
	autoredraw_type_item = CreateToggleButton(rc1, "Auto redraw");
	cursor_type_item = CreateToggleButton(rc1, "Crosshair cursor");
#if defined WITH_XMHTML || defined WITH_LIBHELP
	force_external_viewer_item = CreateToggleButton(rc1,
            "Use external help viewer for local documents");
#endif        
	fr = CreateFrame(props_frame, "Restrictions");
        AddDialogFormChild(props_frame, fr);
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
    
    RaiseWindow(GetParent(props_frame));
    unset_wait_cursor();
}

void update_props_items(void)
{
    int itest = 0;
    int iv;
    
    if (props_frame) {
        GUI *gui = grace->gui;
#ifdef DEBUG
	if (get_debuglevel() > 8) {
	    errwin("Debug level > 8, resetting to 0");
	    set_debuglevel(0);
	}
	SetSpinChoice(debug_item, (double) get_debuglevel());
#endif
	SetToggleButtonState(noask_item, gui->noask);
	SetToggleButtonState(dc_item, gui->allow_dc);

	if (gui->focus_policy == FOCUS_SET) {
	    itest = 1;
	} else if (gui->focus_policy == FOCUS_CLICK) {
	    itest = 0;
	} else if (gui->focus_policy == FOCUS_FOLLOWS) {
	    itest = 2;
	}
	SetOptionChoice(graph_focus_choice_item, itest);
	SetToggleButtonState(graph_drawfocus_choice_item, gui->draw_focus_flag);

	SetToggleButtonState(autoredraw_type_item, gui->auto_redraw);
	SetToggleButtonState(cursor_type_item, cursortype);
#if defined WITH_XMHTML || defined WITH_LIBHELP
	SetToggleButtonState(force_external_viewer_item, force_external_viewer);
#endif
	SetSpinChoice(max_path_item,
            (double) get_max_path_limit(grace->rt->canvas));
	SetToggleButtonState(safe_mode_item, grace->rt->safe_mode);
	iv = (int) rint(100*grace->rt->scrollper);
	SetScaleValue(scrollper_item, iv);
	iv = (int) rint(100*grace->rt->shexper);
	SetScaleValue(shexper_item, iv);
    }
}

static int props_define_notify_proc(void *data)
{
    GUI *gui = grace->gui;
    
#ifdef DEBUG
    set_debuglevel((int) GetSpinChoice(debug_item));
#endif
    gui->noask = GetToggleButtonState(noask_item);
    gui->allow_dc = GetToggleButtonState(dc_item);

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
    gui->draw_focus_flag = GetToggleButtonState(graph_drawfocus_choice_item);

    gui->auto_redraw = GetToggleButtonState(autoredraw_type_item);
    cursortype = GetToggleButtonState(cursor_type_item);
#if defined WITH_XMHTML || defined WITH_LIBHELP
    force_external_viewer = GetToggleButtonState(force_external_viewer_item);
#endif
    set_max_path_limit(grace->rt->canvas, (int) GetSpinChoice(max_path_item));
    grace->rt->safe_mode = GetToggleButtonState(safe_mode_item);
    grace->rt->scrollper = (double) GetScaleValue(scrollper_item)/100.0;
    grace->rt->shexper   = (double) GetScaleValue(shexper_item)/100.0;
    
    xdrawgraph();
    
    return RETURN_SUCCESS;
}
