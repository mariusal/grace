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
 * Plot properties
 *
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>

#include <Xm/Xm.h>
#include <Xm/BulletinB.h>
#include <Xm/DialogS.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/RowColumn.h>
#include <Xm/Scale.h>
#include <Xm/Separator.h>

#include "globals.h"
#include "utils.h"
#include "plotone.h"
#include "motifinc.h"
#include "protos.h"

static Widget plot_frame;

/*
 * Panel item declarations
 */
static Widget *page_color_item;

static Widget timestamp_active_item;
static OptionStructure timestamp_font_item;
static Widget timestamp_size_item;
static Widget timestamp_rotate_item;
static Widget *timestamp_color_item;
Widget timestamp_x_item;
Widget timestamp_y_item;

static void plot_define_notify_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void update_plot_items(void);

void create_plot_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget panel;
    Widget buts[2];
    char *label1[2];
    set_wait_cursor();
    
    if (plot_frame == NULL) {
        label1[0] = "Accept";
        label1[1] = "Close";
    
	plot_frame = XmCreateDialogShell(app_shell, "Plot appearance", NULL, 0);
	handle_close(plot_frame);
	panel = XmCreateRowColumn(plot_frame, "plot_rc", NULL, 0);

	page_color_item = CreateColorChoice(panel, "Page color");

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, panel,
				NULL);

	timestamp_active_item = XtVaCreateManagedWidget("Display Time stamp",
					   xmToggleButtonWidgetClass, panel,
							NULL);

	timestamp_font_item = CreateFontChoice(panel, "Font:");
	timestamp_color_item = CreateColorChoice(panel, "Color:");

	XtVaCreateManagedWidget("Character size:", xmLabelWidgetClass, panel, NULL);
	timestamp_size_item = XtVaCreateManagedWidget("size", xmScaleWidgetClass, panel,
						      XmNminimum, 0,
						      XmNmaximum, 400,
						      XmNvalue, 100,
						      XmNshowValue, True,
				     XmNprocessingDirection, XmMAX_ON_RIGHT,
					       XmNorientation, XmHORIZONTAL,
						      NULL);

        XtVaCreateManagedWidget("Place at angle:", xmLabelWidgetClass, panel, NULL);
	timestamp_rotate_item = XtVaCreateManagedWidget("tstampangle", xmScaleWidgetClass, panel,
					  XmNminimum, 0,
					  XmNmaximum, 360,
					  XmNvalue, 100,
					  XmNshowValue, True,
					  XmNprocessingDirection, XmMAX_ON_RIGHT,
					  XmNorientation, XmHORIZONTAL,
					  NULL);

	timestamp_x_item = CreateTextItem2(panel, 10, "Timestamp X:");
	timestamp_y_item = CreateTextItem2(panel, 10, "Timestamp Y:");

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, panel,
				NULL);

	CreateCommandButtons(panel, 2, buts, label1);
	XtAddCallback(buts[0], XmNactivateCallback,
		   (XtCallbackProc) plot_define_notify_proc, (XtPointer) 0);
	XtAddCallback(buts[1], XmNactivateCallback,
		   (XtCallbackProc) destroy_dialog, (XtPointer) plot_frame);

	XtManageChild(panel);
    }
    XtRaise(plot_frame);
    update_plot_items();
    unset_wait_cursor();
}

static void update_plot_items(void)
{
    int iv;
    Arg a;
    char buf[32];

    if (plot_frame) {
	SetChoice(page_color_item, getbgcolor());

	XmToggleButtonSetState(timestamp_active_item, timestamp.active == TRUE, False);
	SetOptionChoice(timestamp_font_item, timestamp.font);
	SetChoice(timestamp_color_item, timestamp.color);

	iv = (int) (100 * timestamp.charsize);
	XtSetArg(a, XmNvalue, iv);
	XtSetValues(timestamp_size_item, &a, 1);

	iv = (int) (timestamp.rot % 360);
	XtSetArg(a, XmNvalue, iv);
	XtSetValues(timestamp_rotate_item, &a, 1);

	sprintf(buf, "%g", timestamp.x);
	xv_setstr(timestamp_x_item, buf);
	sprintf(buf, "%g", timestamp.y);
	xv_setstr(timestamp_y_item, buf);
    }
}

static void plot_define_notify_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int value;
    Arg a;

    setbgcolor (GetChoice(page_color_item));

    timestamp.active = XmToggleButtonGetState(timestamp_active_item) ? TRUE : FALSE;
    timestamp.font = GetOptionChoice(timestamp_font_item);
    timestamp.color = GetChoice(timestamp_color_item);
    
    XtSetArg(a, XmNvalue, &value);
    XtGetValues(timestamp_size_item, &a, 1);
    timestamp.charsize = value / 100.0;
    
    XtSetArg(a, XmNvalue, &value);
    XtGetValues(timestamp_rotate_item, &a, 1);
    timestamp.rot = value;
    
    xv_evalexpr(timestamp_x_item, &timestamp.x);
    xv_evalexpr(timestamp_y_item, &timestamp.y);
    set_dirtystate();
    drawgraph();
}

