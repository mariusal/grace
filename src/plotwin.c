/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-95 Paul J Turner, Portland, OR
 * Copyright (c) 1996-99 Grace Development Team
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
#include <Xm/DialogS.h>
#include <Xm/RowColumn.h>

#include "globals.h"
#include "utils.h"
#include "plotone.h"
#include "motifinc.h"
#include "protos.h"

static Widget plot_frame;

/*
 * Panel item declarations
 */
static OptionStructure *bg_color_item;
static Widget bg_fill_item;

static Widget timestamp_active_item;
static OptionStructure *timestamp_font_item;
static Widget timestamp_size_item;
static Widget timestamp_rotate_item;
static OptionStructure *timestamp_color_item;
Widget timestamp_x_item;
Widget timestamp_y_item;

static void plot_define_notify_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void update_plot_items(void);

void create_plot_frame_cb(void *data)
{
    create_plot_frame();
}

void create_plot_frame(void)
{
    Widget panel, fr, rc;
    Widget buts[2];
    char *label1[2];
    set_wait_cursor();
    
    if (plot_frame == NULL) {
        label1[0] = "Accept";
        label1[1] = "Close";
    
	plot_frame = XmCreateDialogShell(app_shell, "Plot appearance", NULL, 0);
	handle_close(plot_frame);
	panel = XmCreateRowColumn(plot_frame, "plot_rc", NULL, 0);

	fr = CreateFrame(panel, "Page background");
        rc = XmCreateRowColumn(fr, "bg_rc", NULL, 0);
        XtVaSetValues(rc, XmNorientation, XmHORIZONTAL, NULL);
        bg_color_item = CreateColorChoice(rc, "Color:");
	bg_fill_item = CreateToggleButton(rc, "Fill");
        XtManageChild(rc);

	fr = CreateFrame(panel, "Time stamp");
        rc = XmCreateRowColumn(fr, "bg_rc", NULL, 0);

	timestamp_active_item = CreateToggleButton(rc, "Enable");
	timestamp_font_item = CreateFontChoice(rc, "Font:");
	timestamp_color_item = CreateColorChoice(rc, "Color:");
	timestamp_size_item = CreateCharSizeChoice(rc, "Character size");
	timestamp_rotate_item = CreateAngleChoice(rc, "Angle");
	timestamp_x_item = CreateTextItem2(rc, 10, "Timestamp X:");
	timestamp_y_item = CreateTextItem2(rc, 10, "Timestamp Y:");

        XtManageChild(rc);

	CreateSeparator(panel);

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
    char buf[32];

    if (plot_frame) {
	SetOptionChoice(bg_color_item, getbgcolor());
	SetToggleButtonState(bg_fill_item, getbgfill());

	SetToggleButtonState(timestamp_active_item, timestamp.active);
	SetOptionChoice(timestamp_font_item, timestamp.font);
	SetOptionChoice(timestamp_color_item, timestamp.color);

	SetCharSizeChoice(timestamp_size_item, timestamp.charsize);

	SetAngleChoice(timestamp_rotate_item, timestamp.rot);

	sprintf(buf, "%g", timestamp.x);
	xv_setstr(timestamp_x_item, buf);
	sprintf(buf, "%g", timestamp.y);
	xv_setstr(timestamp_y_item, buf);
    }
}

static void plot_define_notify_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    setbgcolor(GetOptionChoice(bg_color_item));
    setbgfill(GetToggleButtonState(bg_fill_item));

    timestamp.active = GetToggleButtonState(timestamp_active_item);
    timestamp.font = GetOptionChoice(timestamp_font_item);
    timestamp.color = GetOptionChoice(timestamp_color_item);
    
    timestamp.charsize = GetCharSizeChoice(timestamp_size_item);
    
    timestamp.rot = GetAngleChoice(timestamp_rotate_item);
    
    xv_evalexpr(timestamp_x_item, &timestamp.x);
    xv_evalexpr(timestamp_y_item, &timestamp.y);
    set_dirtystate();
    drawgraph();
}

