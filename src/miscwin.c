/*
 * Grace - Graphics for Exploratory Data Analysis
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-95 Paul J Turner, Portland, OR
 * Copyright (c) 1996-98 GRACE Development Team
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
#include <cmath.h>

#include <stdio.h>
#include <stdlib.h>

#include <Xm/Xm.h>
#include <Xm/DialogS.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/RowColumn.h>
#include <Xm/Scale.h>

#include "globals.h"
#include "utils.h"
#include "graphs.h"
#include "plotone.h"
#include "motifinc.h"
#include "protos.h"

extern int cursortype;

static Widget props_frame;

/*
 * Panel item declarations
 */
#ifdef DEBUG
static Widget *debug_item;
#endif
static Widget noask_item;
static Widget dc_item;
static Widget *auto_item;

static Widget *graph_focus_choice_item;
static Widget graph_drawfocus_choice_item;

static Widget autoredraw_type_item;
static Widget cursor_type_item;
static Widget scrollper_item;
static Widget shexper_item;
static Widget linkscroll_item;

/*
 * Event and Notify proc declarations
 */
static void props_define_notify_proc(Widget w, XtPointer client_data, XtPointer call_data);

void create_props_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget panel;
    Widget buts[2];
    Widget wlabel;

    set_wait_cursor();
    if (props_frame == NULL) {
	char *label1[2];
	label1[0] = "Accept";
	label1[1] = "Close";
	props_frame = XmCreateDialogShell(app_shell, "Misc", NULL, 0);
	handle_close(props_frame);
	panel = XmCreateRowColumn(props_frame, "props_rc", NULL, 0);
#ifdef DEBUG
	debug_item = CreatePanelChoice0(panel,
					"Debug level:",
					3,
					10,
			      "Off", "1", "2", "3", "4", "5", "6", "7", "8",
					NULL,
					NULL);
#endif
	noask_item = XtVaCreateManagedWidget("Don't ask questions",
					   xmToggleButtonWidgetClass, panel,
					      NULL);
	dc_item = XtVaCreateManagedWidget("Allow double clicks on canvas",
					  xmToggleButtonWidgetClass, panel,
					  NULL);
	auto_item = CreatePanelChoice(panel, "Autoscale type:",
					 5,
				  	 "None",
				  	 "All X-axes",
				  	 "All Y-axes",
				  	 "All axes",
				  	 NULL,
				  	 NULL);

	CreateSeparator(panel);

	graph_focus_choice_item = CreatePanelChoice(panel, "Graph focus",
						    4,
						    "Button press",
						    "As set",
						    "Follows mouse",
						    NULL,
						    NULL);
	graph_drawfocus_choice_item =
                    XtVaCreateManagedWidget("Display focus markers",
					    xmToggleButtonWidgetClass, panel,
                                            NULL);

	CreateSeparator(panel);

	wlabel = XtVaCreateManagedWidget("Scroll %:", xmLabelWidgetClass, panel, NULL);
	scrollper_item = XtVaCreateManagedWidget("scroll", xmScaleWidgetClass, panel,
						 XmNwidth, 200,
						 XmNminimum, 0,
						 XmNmaximum, 200,
						 XmNvalue, 0,
						 XmNshowValue, True,
				     XmNprocessingDirection, XmMAX_ON_RIGHT,
					       XmNorientation, XmHORIZONTAL,
						 NULL);
	wlabel = XtVaCreateManagedWidget("Zoom %:", xmLabelWidgetClass, panel, NULL);
	shexper_item = XtVaCreateManagedWidget("shex", xmScaleWidgetClass, panel,
						 XmNwidth, 200,
						 XmNminimum, 0,
						 XmNmaximum, 200,
						 XmNvalue, 0,
						 XmNshowValue, True,
				     XmNprocessingDirection, XmMAX_ON_RIGHT,
					       XmNorientation, XmHORIZONTAL,
						 NULL);
	linkscroll_item = XtVaCreateManagedWidget("Linked scrolling",
				      xmToggleButtonWidgetClass, panel,
						  NULL);
	autoredraw_type_item = XtVaCreateManagedWidget("Auto redraw",
				      xmToggleButtonWidgetClass, panel,
						       NULL);
	cursor_type_item = XtVaCreateManagedWidget("Crosshair cursor",
				      xmToggleButtonWidgetClass, panel,
						   NULL);

	CreateSeparator(panel);

	CreateCommandButtons(panel, 2, buts, label1);
	XtAddCallback(buts[0], XmNactivateCallback,
		  (XtCallbackProc) props_define_notify_proc, (XtPointer) 0);
	XtAddCallback(buts[1], XmNactivateCallback,
		  (XtCallbackProc) destroy_dialog, (XtPointer) props_frame);

	XtManageChild(panel);
    }
    XtRaise(props_frame);
    update_props_items();
    unset_wait_cursor();
}

void update_props_items(void)
{
    int itest = 0;
    Arg a;
    int iv;
    
    if (props_frame) {
#ifdef DEBUG
	if (debuglevel > 8) {
	    errwin("Debug level > 8, resetting to 0");
	    debuglevel = 0;
	}
	SetChoice(debug_item, debuglevel);
#endif
	XmToggleButtonSetState(noask_item, noask, False);
	XmToggleButtonSetState(dc_item, allow_dc, False);
	SetChoice(auto_item, autoscale_onread);

	if (focus_policy == FOCUS_SET) {
	    itest = 1;
	} else if (focus_policy == FOCUS_CLICK) {
	    itest = 0;
	} else if (focus_policy == FOCUS_FOLLOWS) {
	    itest = 2;
	}
	SetChoice(graph_focus_choice_item, itest);
	XmToggleButtonSetState(graph_drawfocus_choice_item,
			       draw_focus_flag == TRUE ? True : False, False);

	XmToggleButtonSetState(linkscroll_item, scrolling_islinked == TRUE, False);
	XmToggleButtonSetState(autoredraw_type_item, auto_redraw == TRUE, False);
	XmToggleButtonSetState(cursor_type_item, cursortype == TRUE, False);
	iv = (int) rint(100 * scrollper);
	XtSetArg(a, XmNvalue, iv);
	XtSetValues(scrollper_item, &a, 1);
	iv = (int) rint(100 * shexper);
	XtSetArg(a, XmNvalue, iv);
	XtSetValues(shexper_item, &a, 1);
    }
}

static void props_define_notify_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    Arg a;
    int value;
    
#ifdef DEBUG
    debuglevel = GetChoice(debug_item);
#endif
    noask = XmToggleButtonGetState(noask_item);
    allow_dc = XmToggleButtonGetState(dc_item);
    autoscale_onread = GetChoice(auto_item);

    switch (GetChoice(graph_focus_choice_item)) {
    case 0:
	focus_policy = FOCUS_CLICK;
	break;
    case 1:
	focus_policy = FOCUS_SET;
	break;
    case 2:
	focus_policy = FOCUS_FOLLOWS;
	break;
    }
    draw_focus_flag = (int) XmToggleButtonGetState(graph_drawfocus_choice_item) ? TRUE : FALSE;

    scrolling_islinked = XmToggleButtonGetState(linkscroll_item);
    auto_redraw = XmToggleButtonGetState(autoredraw_type_item);
    cursortype = XmToggleButtonGetState(cursor_type_item);
    XtSetArg(a, XmNvalue, &value);
    XtGetValues(scrollper_item, &a, 1);
    scrollper = (double) value / 100.0;
    XtGetValues(shexper_item, &a, 1);
    shexper = (double) value / 100.0;
    
    drawgraph();
}
