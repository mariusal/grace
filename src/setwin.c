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
 * setops - operations on sets
 *
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>

#include <Xm/Xm.h>
#include <Xm/BulletinB.h>
#include <Xm/DialogS.h>
#include <Xm/FileSB.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/RowColumn.h>
#include <Xm/Text.h>
#include <Xm/List.h>
#include <Xm/Separator.h>
#include <Xm/Protocols.h>

#include "globals.h"
#include "graphs.h"
#include "utils.h"
#include "plotone.h"
#include "motifinc.h"
#include "protos.h"

#define cg get_cg()

static Widget but1[2];

/*
 * char format[128] = "%16lg %16lg";
 */

static void do_setlength_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_changetype_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_drop_points_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_join_sets_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_split_sets_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_sort_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_reverse_sets_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_coalesce_sets_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_swap_proc(Widget w, XtPointer client_data, XtPointer call_data);


typedef struct _Type_ui {
    Widget top;
    SetChoiceItem sel;
    Widget comment_item;
    Widget *graph_item;
} Type_ui;

static Type_ui tui;

static void changetypeCB(Widget w, XtPointer clientd, XtPointer calld)
{
    Type_ui *ui = (Type_ui *) clientd;
    XmListCallbackStruct *cbs = (XmListCallbackStruct *) calld;
    char *s;
    XmStringGetLtoR(cbs->item, charset, &s);
    if (cbs->reason == XmCR_SINGLE_SELECT) {
	int setno = GetSelectedSet(ui->sel);
        if (setno == SET_SELECT_ERROR) {
            errwin("No set selected");
            return;
        }
/*
	int setno = GetSetFromString(s);
*/
	if (setno >= 0) {
	    xv_setstr(ui->comment_item, getcomment(cg, setno));
	}
    }
    XtFree(s);
}

void create_change_popup(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget dialog;

    set_wait_cursor();
    if (tui.top == NULL) {
	char *label1[2];
	label1[0] = "Accept";
	label1[1] = "Close";
	tui.top = XmCreateDialogShell(app_shell, "Set comments", NULL, 0);
	handle_close(tui.top);
	dialog = XmCreateRowColumn(tui.top, "dialog_rc", NULL, 0);

	tui.sel = CreateSetSelector(dialog, "Apply to set:",
				    SET_SELECT_ACTIVE,
				    FILTER_SELECT_NONE,
				    GRAPH_SELECT_CURRENT,
				    SELECTION_TYPE_MULTIPLE);

	XtVaSetValues(tui.sel.list,
		      XmNselectionPolicy, XmSINGLE_SELECT,
		      NULL);
	XtAddCallback(tui.sel.list, XmNdefaultActionCallback, changetypeCB, &tui);
	XtAddCallback(tui.sel.list, XmNsingleSelectionCallback, changetypeCB, &tui);
	tui.comment_item = CreateTextItem2(dialog, 20, "Comment:");
	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

	CreateCommandButtons(dialog, 2, but1, label1);
	XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc) do_changetype_proc, (XtPointer) & tui);
	XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) tui.top);

	XtManageChild(dialog);
    }
    XtRaise(tui.top);
    unset_wait_cursor();
}

typedef struct _Length_ui {
    Widget top;
    SetChoiceItem sel;
    Widget length_item;
    Widget *graph_item;
} Length_ui;

static Length_ui lui;

void create_setlength_popup(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget dialog;

    set_wait_cursor();
    if (lui.top == NULL) {
	char *label1[2];
	label1[0] = "Accept";
	label1[1] = "Close";
	lui.top = XmCreateDialogShell(app_shell, "Set length", NULL, 0);
	handle_close(lui.top);
	dialog = XmCreateRowColumn(lui.top, "dialog_rc", NULL, 0);

	lui.sel = CreateSetSelector(dialog, "Set length of set:",
				    SET_SELECT_ACTIVE,
				    FILTER_SELECT_NONE,
				    GRAPH_SELECT_CURRENT,
				    SELECTION_TYPE_MULTIPLE);
	lui.length_item = CreateTextItem2(dialog, 10, "Length:");

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

	CreateCommandButtons(dialog, 2, but1, label1);
	XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc) do_setlength_proc, (XtPointer) & lui);
	XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) lui.top);

	XtManageChild(dialog);
    }
    XtRaise(lui.top);
    unset_wait_cursor();
}

typedef struct _Drop_ui {
    Widget top;
    SetChoiceItem sel;
    Widget start_item;
    Widget stop_item;
} Drop_ui;

static Drop_ui dui;

void create_drop_popup(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget dialog;

    set_wait_cursor();
    if (dui.top == NULL) {
	char *label1[2];
	label1[0] = "Accept";
	label1[1] = "Close";
	dui.top = XmCreateDialogShell(app_shell, "Drop points", NULL, 0);
	handle_close(dui.top);
	dialog = XmCreateRowColumn(dui.top, "dialog_rc", NULL, 0);

	dui.sel = CreateSetSelector(dialog, "Drop points from set:",
				    SET_SELECT_ALL,
				    FILTER_SELECT_NONE,
				    GRAPH_SELECT_CURRENT,
				    SELECTION_TYPE_MULTIPLE);
	dui.start_item = CreateTextItem2(dialog, 6, "Start drop at:");
	dui.stop_item = CreateTextItem2(dialog, 6, "End drop at:");

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

	CreateCommandButtons(dialog, 2, but1, label1);
	XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc) do_drop_points_proc, (XtPointer) & dui);
	XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) dui.top);

	XtManageChild(dialog);
    }
    XtRaise(dui.top);
    unset_wait_cursor();
}

typedef struct _Join_ui {
    Widget top;
    SetChoiceItem sel1;
    SetChoiceItem sel2;
    Widget *graph_item;
} Join_ui;

static Join_ui jui;

void create_join_popup(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget dialog;

    set_wait_cursor();
    if (jui.top == NULL) {
	char *label1[2];
	label1[0] = "Accept";
	label1[1] = "Close";
	jui.top = XmCreateDialogShell(app_shell, "Join sets", NULL, 0);
	handle_close(jui.top);
	dialog = XmCreateRowColumn(jui.top, "dialog_rc", NULL, 0);

	jui.sel1 = CreateSetSelector(dialog, "Join set:",
				     SET_SELECT_ACTIVE,
				     FILTER_SELECT_NONE,
				     GRAPH_SELECT_CURRENT,
				     SELECTION_TYPE_SINGLE);
	jui.sel2 = CreateSetSelector(dialog, "To the end of set:",
				     SET_SELECT_ACTIVE,
				     FILTER_SELECT_NONE,
				     GRAPH_SELECT_CURRENT,
				     SELECTION_TYPE_SINGLE);

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

	CreateCommandButtons(dialog, 2, but1, label1);
	XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc) do_join_sets_proc, (XtPointer) & jui);
	XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) jui.top);

	XtManageChild(dialog);
    }
    XtRaise(jui.top);
    unset_wait_cursor();
}

typedef struct _Split_ui {
    Widget top;
    SetChoiceItem sel;
    Widget len_item;
    Widget *graph_item;
} Split_ui;

static Split_ui sui;

void create_split_popup(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget dialog;

    set_wait_cursor();
    if (sui.top == NULL) {
	char *label1[2];
	label1[0] = "Accept";
	label1[1] = "Close";
	sui.top = XmCreateDialogShell(app_shell, "Split sets", NULL, 0);
	handle_close(sui.top);
	dialog = XmCreateRowColumn(sui.top, "dialog_rc", NULL, 0);

	sui.sel = CreateSetSelector(dialog, "Split set:",
				    SET_SELECT_ALL,
				    FILTER_SELECT_NONE,
				    GRAPH_SELECT_CURRENT,
				    SELECTION_TYPE_MULTIPLE);
	sui.len_item = CreateTextItem2(dialog, 10, "Length:");

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

	CreateCommandButtons(dialog, 2, but1, label1);
	XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc) do_split_sets_proc, (XtPointer) & sui);
	XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) sui.top);

	XtManageChild(dialog);
    }
    XtRaise(sui.top);
    unset_wait_cursor();
}

typedef struct _Sort_ui {
    Widget top;
    SetChoiceItem sel;
    Widget *xy_item;
    Widget *up_down_item;
    Widget *graph_item;
} Sort_ui;

static Sort_ui sortui;

void create_sort_popup(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget dialog;

    set_wait_cursor();
    if (sortui.top == NULL) {
	char *label1[2];
	label1[0] = "Accept";
	label1[1] = "Close";
	sortui.top = XmCreateDialogShell(app_shell, "Sort sets", NULL, 0);
	handle_close(sortui.top);
	dialog = XmCreateRowColumn(sortui.top, "dialog_rc", NULL, 0);

	sortui.sel = CreateSetSelector(dialog, "Sort set:",
				       SET_SELECT_ACTIVE,
				       FILTER_SELECT_NONE,
				       GRAPH_SELECT_CURRENT,
				       SELECTION_TYPE_MULTIPLE);
	sortui.xy_item = CreatePanelChoice(dialog,
					   "Sort on:",
					   7,
					   "X",
					   "Y",
					   "Y1",
					   "Y2",
					   "Y3",
					   "Y4",
					   0, 0);
	sortui.up_down_item = CreatePanelChoice(dialog,
						"Order:",
						3,
						"Ascending",
						"Descending", 0,
						0);
	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

	CreateCommandButtons(dialog, 2, but1, label1);
	XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc) do_sort_proc, (XtPointer) & sortui);
	XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) sortui.top);

	XtManageChild(dialog);
    }
    XtRaise(sortui.top);
    unset_wait_cursor();
}

typedef struct _Reverse_ui {
    Widget top;
    SetChoiceItem sel;
    Widget *graph_item;
} Reverse_ui;

static Reverse_ui rui;

void create_reverse_popup(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget dialog;

    set_wait_cursor();
    if (rui.top == NULL) {
	char *label1[2];
	label1[0] = "Accept";
	label1[1] = "Close";
	rui.top = XmCreateDialogShell(app_shell, "Reverse sets", NULL, 0);
	handle_close(rui.top);
	dialog = XmCreateRowColumn(rui.top, "dialog_rc", NULL, 0);

	rui.sel = CreateSetSelector(dialog, "Reverse set:",
				    SET_SELECT_ALL,
				    FILTER_SELECT_NONE,
				    GRAPH_SELECT_CURRENT,
				    SELECTION_TYPE_MULTIPLE);

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

	CreateCommandButtons(dialog, 2, but1, label1);
	XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc) do_reverse_sets_proc, (XtPointer) & rui);
	XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) rui.top);

	XtManageChild(dialog);
    }
    XtRaise(rui.top);
    unset_wait_cursor();
}

typedef struct _Coal_ui {
    Widget top;
    SetChoiceItem sel;
    Widget *graph_item;
} Coal_ui;

static Coal_ui coalui;

void create_coalesce_popup(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget dialog;

    set_wait_cursor();
    if (coalui.top == NULL) {
	char *label1[2];
	label1[0] = "Accept";
	label1[1] = "Close";
	coalui.top = XmCreateDialogShell(app_shell, "Coalesce sets", NULL, 0);
	handle_close(coalui.top);
	dialog = XmCreateRowColumn(coalui.top, "dialog_rc", NULL, 0);

	coalui.sel = CreateSetSelector(dialog, "Coalesce active sets to set:",
				       SET_SELECT_ALL,
				       FILTER_SELECT_NONE,
				       GRAPH_SELECT_CURRENT,
				       SELECTION_TYPE_MULTIPLE);

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

	CreateCommandButtons(dialog, 2, but1, label1);
	XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc) do_coalesce_sets_proc, (XtPointer) & coalui);
	XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) coalui.top);

	XtManageChild(dialog);
    }
    XtRaise(coalui.top);
    unset_wait_cursor();
}

typedef struct _Swap_ui {
    Widget top;
    SetChoiceItem sel1;
    SetChoiceItem sel2;
    Widget *graph1_item;
    Widget *graph2_item;
} Swap_ui;

static Swap_ui swapui;


void do_setsel_gr_update(Widget w, XtPointer client_data, XtPointer call_data)
{
	if( (int)client_data == 1 ){
		swapui.sel1.gno = (int)GetChoice(swapui.graph1_item)-1;
		if( swapui.sel1.gno == -1 ) {
			swapui.sel1.gno = GRAPH_SELECT_CURRENT;
			update_save_set_list( swapui.sel1, cg );
		} else
			update_save_set_list( swapui.sel1, swapui.sel1.gno );
	} else {
		swapui.sel2.gno = (int)GetChoice(swapui.graph2_item)-1;
		if( swapui.sel2.gno == -1 ) {
			swapui.sel2.gno = GRAPH_SELECT_CURRENT;
			update_save_set_list( swapui.sel2, cg );
		} else
			update_save_set_list( swapui.sel2, swapui.sel2.gno );
	}
}


void create_swap_popup(Widget w, XtPointer client_data, XtPointer call_data)
{
    int i;
    Widget dialog;

    set_wait_cursor();
    if (swapui.top == NULL) {
	char *label1[2];
	label1[0] = "Accept";
	label1[1] = "Close";
	swapui.top = XmCreateDialogShell(app_shell, "Swap sets", NULL, 0);
	handle_close(swapui.top);
	dialog = XmCreateRowColumn(swapui.top, "dialog_rc", NULL, 0);

	swapui.sel1 = CreateSetSelector(dialog, "Swap set:",
					SET_SELECT_ACTIVE,
					FILTER_SELECT_NONE,
					GRAPH_SELECT_CURRENT,
					SELECTION_TYPE_SINGLE);
	swapui.graph1_item = CreateGraphChoice(dialog, "In graph:", number_of_graphs(), 1);
	for(i=0; i<number_of_graphs(); i++ )
	    XtAddCallback(swapui.graph1_item[2 + i], XmNactivateCallback,
			(XtCallbackProc) do_setsel_gr_update, (XtPointer) 1);

	swapui.sel2 = CreateSetSelector(dialog, "With set:",
					SET_SELECT_ACTIVE,
					FILTER_SELECT_ALL,
					GRAPH_SELECT_CURRENT,
					SELECTION_TYPE_SINGLE);
	DefineSetSelectorFilter(&swapui.sel2);
	swapui.graph2_item = CreateGraphChoice(dialog, "In graph:", number_of_graphs(), 1);
	for(i=0; i<number_of_graphs(); i++ )
	    XtAddCallback(swapui.graph2_item[2 + i], XmNactivateCallback,
			(XtCallbackProc) do_setsel_gr_update, (XtPointer) 2);

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

	CreateCommandButtons(dialog, 2, but1, label1);
	XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc) do_swap_proc, (XtPointer) & swapui);
	XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) swapui.top);

	XtManageChild(dialog);
    }
    XtRaise(swapui.top);
    unset_wait_cursor();
}

/*
 * setops - combine, copy sets - callbacks
*/


/*
 * change the type of a set
 */
void do_changetype(int setno, int type)
{
    settype(cg, setno, type);
    setlength(cg, setno, getsetlength(cg, setno));

    update_set_status(cg, setno);
}

/*
 * change the type of a set
 */
static void do_changetype_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int setno;
    Type_ui *ui = (Type_ui *) client_data;
    setno = GetSelectedSet(ui->sel);
    if (setno == SET_SELECT_ERROR) {
        errwin("No set selected");
        return;
    }
    setcomment(cg, setno, (char *) xv_getstr(ui->comment_item));
    set_wait_cursor();
    set_work_pending(TRUE);
    set_work_pending(FALSE);
    update_set_lists(cg);
    unset_wait_cursor();
}


/*
 * set the length of an active set - contents are destroyed
 */
void do_setlength(int setno, int len)
{
    char buf[64];
    
    if (!is_set_active(cg, setno)) {
	sprintf(buf, "Set %d not active", setno);
	errmsg(buf);
	return;
    }
    if (len <= 0) {
	sprintf(buf, "Improper set length = %d", len);
	errmsg(buf);
	return;
    }
    setlength(cg, setno, len);

    update_set_status(cg, setno);
}

/*
 * set the length of an active set - contents are destroyed
 */
static void do_setlength_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int *selsets;
    int i, cnt;
    int setno, len;
    Length_ui *ui = (Length_ui *) client_data;
    cnt = GetSelectedSets(ui->sel, &selsets);
    if (cnt == SET_SELECT_ERROR) {
	errwin("No sets selected");
	return;
    }
    xv_evalexpri(ui->length_item, &len);
    set_wait_cursor();
    set_work_pending(TRUE);
    for (i = 0; i < cnt; i++) {
	setno = selsets[i];
	do_setlength(setno, len);
    }
    set_work_pending(FALSE);
    update_set_lists(cg);
    unset_wait_cursor();
    free(selsets);
    drawgraph();
}


/*
 * swap a set with another set
 */
static void do_swap_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int j1, j2, gto, gfrom;

    Swap_ui *ui = (Swap_ui *) client_data;
    j1 = GetSelectedSet(ui->sel1);
    j2 = GetSelectedSet(ui->sel2);
    if (j1 == SET_SELECT_ERROR || j2 == SET_SELECT_ERROR) {
	errwin("Select 2 sets");
	return;
    }
    gfrom = (int) GetChoice(ui->graph1_item);
    gto = (int) GetChoice(ui->graph2_item);
    set_wait_cursor();
    set_work_pending(TRUE);
    do_swap(j1, gfrom, j2, gto);
    set_work_pending(FALSE);
    update_set_lists(cg);
    unset_wait_cursor();
    drawgraph();
}

/*
 * drop points from an active set
 */
static void do_drop_points_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int i, *selsets;
    int cnt;
    int startno, endno, setno;
    Drop_ui *ui = (Drop_ui *) client_data;
    xv_evalexpri(ui->start_item, &startno);
    xv_evalexpri(ui->stop_item, &endno);
	startno -= 1;
	endno -= 1;
    cnt = GetSelectedSets(ui->sel, &selsets);
    if (cnt == SET_SELECT_ERROR) {
		errwin("No sets selected");
		return;
    }
    set_wait_cursor();
    set_work_pending(TRUE);
    for (i = 0; i < cnt; i++) {
		setno = selsets[i];
		do_drop_points(setno, startno, endno);
    }
    set_work_pending(FALSE);
    update_set_lists(cg);
    unset_wait_cursor();
    free(selsets);
    drawgraph();
}

/*
 * append one set to another
 */
static void do_join_sets_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int j1, j2;
    Join_ui *ui = (Join_ui *) client_data;
    j1 = GetSelectedSet(ui->sel1);
    j2 = GetSelectedSet(ui->sel2);
    if (j1 == SET_SELECT_ERROR || j2 == SET_SELECT_ERROR) {
	errwin("Select 2 sets");
	return;
    }
    set_wait_cursor();
    set_work_pending(TRUE);
    do_join_sets(cg, j1, cg, j2);
    set_work_pending(FALSE);
    update_set_lists(cg);
    unset_wait_cursor();
    drawgraph();
}

/*
 * reverse the order of a set
 */
static void do_reverse_sets_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int setno;
    int cnt, i, *selsets;
    Reverse_ui *ui = (Reverse_ui *) client_data;
    cnt = GetSelectedSets(ui->sel, &selsets);
    if (cnt == SET_SELECT_ERROR) {
	errwin("No sets selected");
	return;
    }
    set_wait_cursor();
    set_work_pending(TRUE);
    for (i = 0; i < cnt; i++) {
	setno = selsets[i];
	do_reverse_sets(setno);
    }
    set_work_pending(FALSE);
    update_set_lists(cg);
    unset_wait_cursor();
    free(selsets);
    drawgraph();
}

/*
 * coalesce sets
 */
static void do_coalesce_sets_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int *selsets;
    int i, cnt;
    int setno;
    Coal_ui *ui = (Coal_ui *) client_data;
    cnt = GetSelectedSets(ui->sel, &selsets);
    if (cnt == SET_SELECT_ERROR) {
	errwin("No sets selected");
	return;
    }
    set_wait_cursor();
    set_work_pending(TRUE);
    for (i = 0; i < cnt; i++) {
	setno = selsets[i];
	do_coalesce_sets(setno);
    }
    set_work_pending(FALSE);
    update_set_lists(cg);
    unset_wait_cursor();
    free(selsets);
    drawgraph();
}

/*
 sort sets
*/
static void do_sort_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int *selsets;
    int i, cnt;
    int setno, sorton, stype;
    Sort_ui *ui = (Sort_ui *) client_data;
    static int son[MAX_SET_COLS] = {DATA_X, DATA_Y, DATA_Y1, DATA_Y2, DATA_Y3, DATA_Y4};

    cnt = GetSelectedSets(ui->sel, &selsets);
    if (cnt == SET_SELECT_ERROR) {
	errwin("No sets selected");
	return;
    }
    sorton = son[(int) GetChoice(ui->xy_item)];
    stype = (int) GetChoice(ui->up_down_item);

    set_wait_cursor();
    set_work_pending(TRUE);
    for (i = 0; i < cnt; i++) {
	setno = selsets[i];
	do_sort(setno, sorton, stype);
    }
    set_work_pending(FALSE);
    update_set_lists(cg);
    unset_wait_cursor();
    free(selsets);
    drawgraph();
}


/*
 * split sets split by itmp, remainder in last set.
 */
static void do_split_sets_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int *selsets;
    int i, cnt;
    int setno, lpart;
    Split_ui *ui = (Split_ui *) client_data;
    cnt = GetSelectedSets(ui->sel, &selsets);
    if (cnt == SET_SELECT_ERROR) {
		errwin("No sets selected");
		return;
    }
    xv_evalexpri(ui->len_item, &lpart);
    set_wait_cursor();
    set_work_pending(TRUE);
    for (i = 0; i < cnt; i++) {
		setno = selsets[i];
		do_splitsets(cg, setno, lpart);
    }
    set_work_pending(FALSE);
    update_set_lists(cg);
    unset_wait_cursor();
    free(selsets);
    drawgraph();
}

