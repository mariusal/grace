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
 * edit points, clip points to window, etc.
 *
 */

/* TODO:
   allow 'restrict to' option to operate only on a selected set
*/

#include <config.h>
#include <cmath.h>

#include <stdio.h>
#include <stdlib.h>

#include <Xm/Xm.h>
#include <Xm/DialogS.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/Text.h>
#include <Xm/ToggleB.h>

#include "globals.h"
#include "draw.h"
#include "plotone.h"
#include "graphs.h"
#include "x11drv.h"
#include "events.h"
#include "protos.h"
#include "motifinc.h"

static Widget but1[4];

extern int add_setno;
extern int add_at;
extern int move_dir;

static Widget points_frame;

static Widget locate_point_item;
static Widget locate_point_message;

static Widget goto_pointx_item;
static Widget goto_pointy_item;
static SetChoiceItem goto_set_item;
static Widget goto_index_item;

static SetChoiceItem addinset_item;
static Widget *addat_item;

static void do_ptsmove_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_del_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_track_proc(Widget w, XtPointer client_data, XtPointer call_data);

/*
 * set tracker
 */
static void do_track_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    set_action(0);
    set_action(TRACKER);
    SetLabel(locate_point_message, "Tracking -  Set, location, (X, Y):");
}

/*
 * activate the add point item in the canvas event proc
 */
static void do_add_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    set_action(0);
    set_action(ADD_POINT);
    add_setno = GetSelectedSet(addinset_item);
    if (add_setno == SET_SELECT_ERROR) {
        errwin("No set selected");
        return;
    }
    if (add_setno == SET_SELECT_NEXT) {
	if ((add_setno = nextset(get_cg())) != -1) {
	    activateset(get_cg(), add_setno);
	} else {
	    set_action(0);
	    return;
	}
    }
    add_at = GetChoice(addat_item);
    SetLabel(locate_point_message, "Adding points to set, location, (X, Y):");
}

/*
 * activate the delete point item in the canvas event proc
 */
static void do_del_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    set_action(0);
    set_action(DEL_POINT);
    SetLabel(locate_point_message, "Delete points - set, location, (X, Y):");
}

/*
 * move a point
 */
static void do_ptsmove_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    set_action(0);
    set_action(MOVE_POINT1ST);
    move_dir = 0;
    SetLabel(locate_point_message, "Move points - set, location, (X, Y):");
}

static void do_movey_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    set_action(0);
    set_action(MOVE_POINT1ST);
    move_dir = 2;
    SetLabel(locate_point_message, "Move points along y - set, location, (X, Y):");
}

static void do_movex_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    set_action(0);
    set_action(MOVE_POINT1ST);
    move_dir = 1;
    SetLabel(locate_point_message, "Move points along x - set, location, (X, Y):");
}

static void do_gotoxy_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    WPoint wp;
    VPoint vp;

    if (xv_evalexpr(goto_pointx_item, &wp.x) != GRACE_EXIT_SUCCESS ||
        xv_evalexpr(goto_pointy_item, &wp.y) != GRACE_EXIT_SUCCESS  ) {
	return;
    }

    vp = Wpoint2Vpoint(wp);
    setpointer(vp);
}

static void do_gotopt_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int setno, ind;
    double *x, *y;
    WPoint wp;
    VPoint vp;
    int cg = get_cg();

    setno = GetSelectedSet(goto_set_item);
    if (setno == SET_SELECT_ERROR) {
        errwin("No set selected");
        return;
    }
    xv_evalexpri(goto_index_item, &ind );
    if (is_set_active(cg, setno)) {
	if (ind <= getsetlength(cg, setno) && ind > 0) {
	    x = getx(cg, setno);
	    y = gety(cg, setno);
	    wp.x = x[ind - 1];
	    wp.y = y[ind - 1];
	    vp = Wpoint2Vpoint(wp);
            setpointer(vp);
	} else {
	    errwin("Point index out of range");
	}
    }
}

void create_goto_frame(void *data)
{
    static Widget top;
    Widget rc, fr, dialog, buts[3];
    set_wait_cursor();
    if (top == NULL) {
	char *label1[3];
	label1[0] = "Goto X, Y";
	label1[1] = "Goto point, set";
	label1[2] = "Close";
	top = XmCreateDialogShell(app_shell, "Goto", NULL, 0);
	handle_close(top);
	dialog = XmCreateRowColumn(top, "dialog_rc", NULL, 0);

	fr = CreateFrame(dialog, NULL);
	rc = XmCreateRowColumn(fr, "rc", NULL, 0);
	goto_pointx_item = CreateTextItem2(rc, 10, "X: ");
	goto_pointy_item = CreateTextItem2(rc, 10, "Y: ");
	XtManageChild(rc);

	fr = CreateFrame(dialog, NULL);
	rc = XmCreateRowColumn(fr, "rc", NULL, 0);
	goto_index_item = CreateTextItem2(rc, 10, "Goto point: ");
	goto_set_item = CreateSetSelector(dialog, "In set:",
					  SET_SELECT_ACTIVE,
					  FILTER_SELECT_NONE,
					  GRAPH_SELECT_CURRENT,
					  SELECTION_TYPE_MULTIPLE);
	XtManageChild(rc);

	CreateSeparator(dialog);

	CreateCommandButtons(dialog, 3, buts, label1);
	XtAddCallback(buts[0], XmNactivateCallback, (XtCallbackProc) do_gotoxy_proc, (XtPointer) NULL);
	XtAddCallback(buts[1], XmNactivateCallback, (XtCallbackProc) do_gotopt_proc, (XtPointer) NULL);
	XtAddCallback(buts[2], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) top);

	XtManageChild(dialog);
    }
    XtRaise(top);
    unset_wait_cursor();
}

void create_add_frame(void *data)
{
    static Widget top, dialog;

    set_wait_cursor();
    if (top == NULL) {
	char *label1[2];
	label1[0] = "Accept";
	label1[1] = "Close";
	top = XmCreateDialogShell(app_shell, "Add points", NULL, 0);
	handle_close(top);
	dialog = XmCreateRowColumn(top, "dialog_rc", NULL, 0);

	addinset_item = CreateSetSelector(dialog, "Add to set:",
					  SET_SELECT_NEXT,
					  FILTER_SELECT_NONE,
					  GRAPH_SELECT_CURRENT,
					  SELECTION_TYPE_SINGLE);
	addat_item = CreatePanelChoice(dialog,
				       "Add:", 4,
				       "To end of set",
				       "To beginning of set",
				       "Between nearest points in set",
				       NULL, 0);
	CreateSeparator(dialog);

	CreateCommandButtons(dialog, 2, but1, label1);
	XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc) do_add_proc, (XtPointer) NULL);
	XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) top);

	XtManageChild(dialog);
    }
    XtRaise(top);
    unset_wait_cursor();
}

static void points_done_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    set_action(0);
    XtUnmanageChild((Widget) client_data);
}

void create_points_frame(void *data)
{
    Widget dialog, wbut, rc;
    XmString str;

    set_wait_cursor();
    if (points_frame == NULL) {
	char *label0[1];
	label0[0] = "Close";
	points_frame = XmCreateDialogShell(app_shell, "Points", NULL, 0);
	handle_close(points_frame);
	dialog = XmCreateRowColumn(points_frame, "dialog_rc", NULL, 0);

	rc = XmCreateRowColumn(dialog, "rc", NULL, 0);
	str = XmStringCreateLocalized("Set, location, (X, Y): ");
	locate_point_message = XtVaCreateManagedWidget("pointslabel",
            xmLabelWidgetClass, rc, XmNlabelString, str, NULL);
        XmStringFree(str);

	locate_point_item = XtVaCreateManagedWidget("locator", xmTextWidgetClass, rc,
						    XmNcolumns, 50,
						    XmNeditable, False,
                                                    NULL);
	XtManageChild(rc);

	CreateSeparator(dialog);

	rc = XmCreateRowColumn(dialog, "rc", NULL, 0);
	XtVaSetValues(rc, XmNorientation, XmHORIZONTAL, NULL);

	wbut = XtVaCreateManagedWidget("Track", xmPushButtonWidgetClass, rc,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) do_track_proc, (XtPointer) 0);

	wbut = XtVaCreateManagedWidget("Move", xmPushButtonWidgetClass, rc,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) do_ptsmove_proc, (XtPointer) 0);

	wbut = XtVaCreateManagedWidget("Move X", xmPushButtonWidgetClass, rc,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) do_movex_proc, (XtPointer) 0);

	wbut = XtVaCreateManagedWidget("Move Y", xmPushButtonWidgetClass, rc,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) do_movey_proc, (XtPointer) 0);

	wbut = XtVaCreateManagedWidget("Goto...", xmPushButtonWidgetClass, rc,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) create_goto_frame, (XtPointer) 0);

	wbut = XtVaCreateManagedWidget("Add...", xmPushButtonWidgetClass, rc,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) create_add_frame, (XtPointer) 0);

	wbut = XtVaCreateManagedWidget("Delete", xmPushButtonWidgetClass, rc,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) do_del_proc, (XtPointer) 0);


	wbut = XtVaCreateManagedWidget("Close", xmPushButtonWidgetClass, rc,
				       (XtPointer) NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) points_done_proc, (XtPointer) points_frame);
	XtManageChild(rc);

	XtManageChild(dialog);
    }
    XtRaise(points_frame);
    unset_wait_cursor();
}

void update_point_locator(int gno, int setno, int loc)
{
    WPoint wp;
    char buf[64];
    
    if (get_point(gno, setno, loc, &wp) == GRACE_EXIT_SUCCESS) {
        sprintf(buf, "G%d.S%d, loc %d, (%g, %g)", gno, setno, loc, wp.x, wp.y);
        xv_setstr(locate_point_item, buf);
    } else {
        xv_setstr(locate_point_item, "");
    }
}

