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

#include <Xbae/Matrix.h>

#include "globals.h"
#include "graphs.h"
#include "utils.h"
#include "plotone.h"
#include "motifinc.h"
#include "protos.h"

#define cg get_cg()

static Widget but1[2];

static void dataset_aac_cb(Widget w, XtPointer client_data, XtPointer call_data);
static void do_drop_points_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_join_sets_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_split_sets_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_sort_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_reverse_sets_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void swap_aac_cb(Widget w, XtPointer client_data, XtPointer call_data);


typedef struct _Type_ui {
    Widget top;
    ListStructure *sel;
    Widget comment_item;
    Widget length_item;
    OptionStructure *datatype_item;
    Widget mw;
    char *rows[MAX_SET_COLS][6];
} Type_ui;

static Type_ui tui;

static void changetypeCB(Widget w, XtPointer client_data, XtPointer call_data)
{
    int i, j, ncols;
    double *datap;
    int imin, imax;
    double dmin, dmax, dmean, dsd;
    ListStructure *listp;
    SetChoiceData *sdata;
    int gno, setno;
    char buf[32];
    char **cells[MAX_SET_COLS];
    
    listp = (ListStructure *) client_data;
    if (listp == NULL) {
        return;
    }
    
    sdata = (SetChoiceData *) listp->anydata;
    gno = sdata->gno;
    
    if (GetSingleListChoice(listp, &setno) == GRACE_EXIT_SUCCESS &&
                is_set_active(gno, setno) == TRUE) {
	ncols = dataset_cols(gno, setno);
        xv_setstr(tui.comment_item, getcomment(gno, setno));
	sprintf(buf, "%d", getsetlength(gno, setno));
        xv_setstr(tui.length_item, buf);
        SetOptionChoice(tui.datatype_item, dataset_type(gno, setno));
    } else {
	ncols = 0;
    }
    for (i = 0; i < MAX_SET_COLS; i++) {
        datap = getcol(gno, setno, i);
	minmax(datap, getsetlength(gno, setno), &dmin, &dmax, &imin, &imax);
	stasum(datap, getsetlength(gno, setno), &dmean, &dsd);
        for (j = 0; j < 6; j++) {
            if (i < ncols) {
                switch (j) {
                case 0:
                    sprintf(buf, "%g", dmin);
                    break;
                case 1:
                    sprintf(buf, "%d", imin);
                    break;
                case 2:
                    sprintf(buf, "%g", dmax);
                    break;
                case 3:
                    sprintf(buf, "%d", imax);
                    break;
                case 4:
                    sprintf(buf, "%g", dmean);
                    break;
                case 5:
                    sprintf(buf, "%g", dsd);
                    break;
                default:
                    strcpy(buf, "");
                    break;
                }
                tui.rows[i][j] = copy_string(tui.rows[i][j], buf);
            } else {
                tui.rows[i][j] = copy_string(tui.rows[i][j], "");
            }
        }
        cells[i] = &tui.rows[i][0];
    }
    XtVaSetValues(tui.mw, XmNcells, cells, NULL);
}

void enterCB(Widget w, XtPointer client_data, XtPointer call_data)
{
    XbaeMatrixEnterCellCallbackStruct *cbs =
        (XbaeMatrixEnterCellCallbackStruct *) call_data;
    
    cbs->doit = False;
}


void create_change_popup(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget panel, menubar, menupane, submenupane, cascade, dialog, rc, fr;
    int i, j;

    set_wait_cursor();
    if (tui.top == NULL) {
        char *rowlabels[MAX_SET_COLS];
        char *collabels[6] = {"Min", "at", "Max", "at", "Mean", "Stdev"};
        unsigned char column_alignments[6] = {
                                                XmALIGNMENT_CENTER,
                                                XmALIGNMENT_CENTER,
                                                XmALIGNMENT_CENTER,
                                                XmALIGNMENT_CENTER,
                                                XmALIGNMENT_CENTER,
                                                XmALIGNMENT_CENTER
                                             };
	tui.top = XmCreateDialogShell(app_shell, "Data set props", NULL, 0);
	handle_close(tui.top);
        panel = XtVaCreateWidget("dataSetPanel",
            xmFormWidgetClass, tui.top, NULL, 0);

        menubar = CreateMenuBar(panel, "dataSetMenuBar", NULL);
        
        XtManageChild(menubar);
        XtVaSetValues(menubar,
                      XmNtopAttachment, XmATTACH_FORM,
                      XmNleftAttachment, XmATTACH_FORM,
                      XmNrightAttachment, XmATTACH_FORM,
                      NULL);

	dialog = XmCreateRowColumn(panel, "dialog_rc", NULL, 0);

	tui.sel = CreateSetChoice(dialog,
            "Data sets:", LIST_TYPE_MULTIPLE, TRUE);
	AddListChoiceCB(tui.sel, changetypeCB);


        menupane = CreateMenu(menubar, "dataSetFileMenu", "File", 'F', NULL, NULL);
        CreateMenuButton(menupane, "close", "Close", 'C',
            (XtCallbackProc) dataset_aac_cb, (XtPointer) AAC_CLOSE, NULL);

        menupane = CreateMenu(menubar, "dataSetDataMenu", "Edit", 'E', NULL, NULL);

        CreateMenuButton(menupane, "duplicate", "Duplicate", 'D',
            duplicate_set_proc, (XtPointer) tui.sel, 0);
        CreateMenuButton(menupane, "killData", "Kill data", 'a',
            killd_set_proc, (XtPointer) tui.sel, 0);
        CreateMenuSeparator(menupane);
        submenupane = CreateMenu(menupane, "editData", "Edit data", 'E', NULL, NULL);
        CreateMenuButton(submenupane, "inShpreadsheet", "In spreadsheet", 's',
            editS_set_proc, (XtPointer) tui.sel, 0);
        CreateMenuButton(submenupane, "inEditor", "In text editor", 'e',
            editE_set_proc, (XtPointer) tui.sel, 0);
        submenupane = CreateMenu(menupane, "createNew", "Create new", 'n', NULL, NULL);
        CreateMenuButton(submenupane, "byFormula", "By formula", 'f',
            newF_set_proc, (XtPointer) tui.sel, 0);
        CreateMenuButton(submenupane, "inShpreadsheet", "In spreadsheet", 's',
            newS_set_proc, (XtPointer) tui.sel, 0);
        CreateMenuButton(submenupane, "inEditor", "In text editor", 'e',
            newE_set_proc, (XtPointer) tui.sel, 0);
        CreateMenuButton(submenupane, "fromBlockData", "From block data", 'b',
            newB_set_proc, (XtPointer) tui.sel, 0);
        CreateMenuSeparator(menupane);
        CreateMenuButton(menupane, "setAppearance", "Set appearance...", 'S',
            (XtCallbackProc) define_symbols_popup, (XtPointer) -1, 0);
        CreateMenuButton(menupane, "setOperations", "Set operations...", 'o',
                (XtCallbackProc) create_swap_popup, (XtPointer) NULL, 0);
 

        menupane = CreateMenu(menubar, "nonlHelpMenu", "Help", 'H', &cascade, NULL);
        XtVaSetValues(menubar, XmNmenuHelpWidget, cascade, NULL);
        CreateMenuButton(menupane, "onDataSets", "On data sets", 's',
            (XtCallbackProc) HelpCB, (XtPointer) NULL, 0);
        CreateMenuButton(menupane, "onContext", "On context", 'x',
            (XtCallbackProc) ContextHelpCB, (XtPointer) NULL, 0);

	rc = XmCreateRowColumn(dialog, "rc", NULL, 0);
        XtVaSetValues(rc, XmNorientation, XmHORIZONTAL, NULL);
	tui.datatype_item = CreateSetTypeChoice(rc, "Type:");
	tui.length_item = CreateTextItem2(rc, 6, "Length:");
        XtManageChild(rc);
	tui.comment_item = CreateTextItem2(dialog, 24, "Comment:");

        for (i = 0; i < MAX_SET_COLS; i++) {
            rowlabels[i] = copy_string(NULL, dataset_colname(i));
            for (j = 0; j < 6; j++) {
                tui.rows[i][j] = NULL;
            }
        }

	fr = CreateFrame(dialog, "Statistics");
        tui.mw = XtVaCreateManagedWidget("mw",
            xbaeMatrixWidgetClass, fr,
            XmNrows, MAX_SET_COLS,
            XmNcolumns, 6,
            XmNvisibleRows, MAX_SET_COLS,
            XmNvisibleColumns, 4,
            XmNcolumnLabels, collabels,
            XmNcolumnLabelAlignments, column_alignments,
            XmNrowLabels, rowlabels,
	    XmNrowLabelWidth, 3,
            XmNrowLabelAlignment, XmALIGNMENT_CENTER,
            XmNallowColumnResize, True,
            XmNgridType, XmGRID_COLUMN_SHADOW,
            XmNcellShadowType, XmSHADOW_OUT,
            XmNcellShadowThickness, 1,
            XmNaltRowCount, 1,
            XmNtraversalOn, False,
            NULL);

/*
 *         XtUninstallTranslations(tui.mw);
 */
        XtAddCallback(tui.mw, XmNenterCellCallback, enterCB, NULL);	

	XtManageChild(dialog);
        XtVaSetValues(dialog,
            XmNtopAttachment, XmATTACH_WIDGET,
            XmNtopWidget, menubar,
            XmNleftAttachment, XmATTACH_FORM,
            XmNrightAttachment, XmATTACH_FORM,
            NULL);

	fr = CreateFrame(panel, NULL);
        rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, fr, NULL);
        CreateAACButtons(rc, panel, dataset_aac_cb);
        XtManageChild(rc);
        XtVaSetValues(fr,
            XmNtopAttachment, XmATTACH_WIDGET,
            XmNtopWidget, dialog,
            XmNleftAttachment, XmATTACH_FORM,
            XmNrightAttachment, XmATTACH_FORM,
            XmNbottomAttachment, XmATTACH_FORM,
            NULL);

	XtManageChild(panel);
    }
    XtRaise(tui.top);
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

typedef struct _Swap_ui {
    Widget top;
    ListStructure *sel1;
    ListStructure *sel2;
    ListStructure *graph1_item;
    ListStructure *graph2_item;
    OptionStructure *optype_item;
} Swap_ui;

static Swap_ui swapui;

void source_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
    ListStructure *listp;
    int gno;
    
    listp = (ListStructure *) client_data;
    if (listp == NULL) {
        return;
    }
    
    if (GetSingleListChoice(listp, &gno) == GRACE_EXIT_SUCCESS) {
        UpdateSetChoice(swapui.sel1, gno);
    } else {
        UpdateSetChoice(swapui.sel1, -1);
    }
}

void target_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
    ListStructure *listp;
    int gno;
    
    listp = (ListStructure *) client_data;
    if (listp == NULL) {
        return;
    }
    
    if (GetSingleListChoice(listp, &gno) == GRACE_EXIT_SUCCESS) {
        UpdateSetChoice(swapui.sel2, gno);
    } else {
        UpdateSetChoice(swapui.sel2, -1);
    }
}

#define OPTYPE_COPY 0
#define OPTYPE_MOVE 1
#define OPTYPE_SWAP 2

void create_swap_popup(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget panel, rc, rc2, fr;
    OptionItem opitems[3];

    set_wait_cursor();
    if (swapui.top == NULL) {
	swapui.top = XmCreateDialogShell(app_shell, "SwapSets", NULL, 0);
	handle_close(swapui.top);
        panel = XtVaCreateWidget("panel", xmFormWidgetClass, 
                                          swapui.top, NULL, 0);
	rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, panel,
			      XmNorientation, XmHORIZONTAL,
			      NULL);

	fr = CreateFrame(rc, "Source");
        rc2 = XtVaCreateWidget("rc2", xmRowColumnWidgetClass, fr, NULL);
	swapui.graph1_item = CreateGraphChoice(rc2, "Graph:", LIST_TYPE_SINGLE);
	swapui.sel1 = CreateSetChoice(rc2, "Set:", LIST_TYPE_MULTIPLE, FALSE);
        AddListChoiceCB(swapui.graph1_item, source_cb);
        UpdateSetChoice(swapui.sel1, cg);
        XtManageChild(rc2);

	fr = CreateFrame(rc, "Destination");
        rc2 = XtVaCreateWidget("rc2", xmRowColumnWidgetClass, fr, NULL);
	swapui.graph2_item = CreateGraphChoice(rc2, "Graph:", LIST_TYPE_SINGLE);
	swapui.sel2 = CreateSetChoice(rc2, "Set:", LIST_TYPE_MULTIPLE, FALSE);
        AddListChoiceCB(swapui.graph2_item, target_cb);
        UpdateSetChoice(swapui.sel2, cg);
        XtManageChild(rc2);

        XtManageChild(rc);
        XtVaSetValues(rc,
            XmNtopAttachment, XmATTACH_FORM,
            XmNleftAttachment, XmATTACH_FORM,
            XmNrightAttachment, XmATTACH_FORM,
            NULL);
        
        opitems[0].value = OPTYPE_COPY;
        opitems[0].label = "Copy";
        opitems[1].value = OPTYPE_MOVE;
        opitems[1].label = "Move";
        opitems[2].value = OPTYPE_SWAP;
        opitems[2].label = "Swap";
        swapui.optype_item = CreateOptionChoice(panel,
            "Type of operation:", 0, 3, opitems);
        XtVaSetValues(swapui.optype_item->rc,
            XmNtopAttachment, XmATTACH_WIDGET,
            XmNtopWidget, rc,
            XmNleftAttachment, XmATTACH_FORM,
            XmNrightAttachment, XmATTACH_FORM,
            NULL);
        
	fr = CreateFrame(panel, NULL);
        XtVaSetValues(fr,
            XmNtopAttachment, XmATTACH_WIDGET,
            XmNtopWidget, swapui.optype_item->rc,
            XmNleftAttachment, XmATTACH_FORM,
            XmNrightAttachment, XmATTACH_FORM,
            XmNbottomAttachment, XmATTACH_FORM,
            NULL);
        rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, fr, NULL);
        CreateAACButtons(rc, panel, swap_aac_cb);
        XtManageChild(rc);

	XtManageChild(panel);
    }
    XtRaise(swapui.top);
    unset_wait_cursor();
}

/*
 * change the type of a set
 */
static void dataset_aac_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
    int aac_mode, error = FALSE;
    int *selset, nsets, i, len, setno, type;
    char *s;
    
    aac_mode = (int) client_data;
    if (aac_mode == AAC_CLOSE) {
        XtUnmanageChild(tui.top);
        return;
    }

    nsets = GetListChoices(tui.sel, &selset);
    if (nsets < 1) {
        errmsg("No set selected");
        return;
    } else {
        set_wait_cursor();
        set_work_pending(TRUE);
 
        type = GetOptionChoice(tui.datatype_item);
        xv_evalexpri(tui.length_item, &len);
        if (len < 0) {
            errmsg("Negative set length!");
            error = TRUE;
        }
        s = xv_getstr(tui.comment_item);
        
 
        if (error == FALSE) {
            for (i = 0; i < nsets; i++) {
                setno = selset[i];
                set_dataset_type(cg, setno, type);
                setlength(cg, setno, len);
                setcomment(cg, setno, s);
            }
        }
 
        if (aac_mode == AAC_ACCEPT && error == FALSE) {
            XtUnmanageChild(tui.top);
        }
        
        free(selset);

        set_work_pending(FALSE);
        update_set_lists(cg);
        unset_wait_cursor();
        drawgraph();
    }
}


/*
 * swap a set with another set
 */
static void swap_aac_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
    int aac_mode, optype, error;
    int i, g1_ok, g2_ok, ns1, ns2, *svalues1, *svalues2, gno1, gno2, setno2;

    aac_mode = (int) client_data;
    
    if (aac_mode == AAC_CLOSE) {
        XtUnmanageChild(swapui.top);
        return;
    }

    set_wait_cursor();
    set_work_pending(TRUE);
    
    optype = GetOptionChoice(swapui.optype_item);
    
    g1_ok = GetSingleListChoice(swapui.graph1_item, &gno1);
    g2_ok = GetSingleListChoice(swapui.graph2_item, &gno2);
    ns1 = GetListChoices(swapui.sel1, &svalues1);
    ns2 = GetListChoices(swapui.sel2, &svalues2);
    
    error = FALSE;
    if (g1_ok == GRACE_EXIT_FAILURE || g2_ok == GRACE_EXIT_FAILURE) {
        error = TRUE;
        errmsg("Please select single source and destination graphs");
    } else if (ns1 == 0) {
        error = TRUE;
        errmsg("No source sets selected");
    } else if (ns2 == 0 && optype == OPTYPE_SWAP) {
        error = TRUE;
        errmsg("No destination sets selected");
    } else if (ns1 != ns2 && (optype == OPTYPE_SWAP || ns2 != 0)) {
        error = TRUE;
        errmsg("Different number of source and destination sets");
    } else if (gno1 == gno2 && ns2 == 0 && optype == OPTYPE_MOVE) {
        error = TRUE;
        errmsg("Can't move a set to itself");
    } else {
        for (i = 0; i < ns1; i++) {
            switch (optype) {
            case OPTYPE_SWAP:
                if (do_swapset(gno1, svalues1[i], gno2, svalues2[i])
                                                != GRACE_EXIT_SUCCESS) {
                    error = TRUE;
                }
                break;
            case OPTYPE_COPY:
                if (ns2 == 0) {
                    setno2 = nextset(gno2);
                } else {
                    setno2 = svalues2[i];
                }
                if (do_copyset(gno1, svalues1[i], gno2, setno2)
                                                != GRACE_EXIT_SUCCESS) {
                    error = TRUE;
                }
                break;
            case OPTYPE_MOVE:
                if (ns2 == 0) {
                    setno2 = nextset(gno2);
                } else {
                    setno2 = svalues2[i];
                }
                if (do_moveset(gno1, svalues1[i], gno2, setno2)
                                                != GRACE_EXIT_SUCCESS) {
                    error = TRUE;
                }
                break;
            }
        }
    }
    
    if (aac_mode == AAC_ACCEPT && error == FALSE) {
        XtUnmanageChild(swapui.top);
    }

    if (ns1 > 0) {
        free(svalues1);
    }
    if (ns2 > 0) {
        free(svalues2);
    }
    set_work_pending(FALSE);
    if (error == FALSE) {
        update_all();
        drawgraph();
    }
    unset_wait_cursor();
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
	reverse_set(get_cg(), setno);
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

