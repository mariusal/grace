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
 * transformations, curve fitting, etc.
 *
 * formerly, this was all one big popup, now it is several.
 * All are created as needed
 *
 */

#include <config.h>
#include <cmath.h>

#include <stdio.h>
#include <stdlib.h>

#include <Xm/Xm.h>
#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/RowColumn.h>
#include <Xm/Text.h>

#include "globals.h"
#include "graphs.h"
#include "plotone.h"
#include "utils.h"
#include "motifinc.h"
#include "protos.h"

extern int nonlflag;		/* true if nonlinear curve fitting module is
				 * to be included */
static int pick_set = 0;	/* TODO: remove */

static Widget but1[3];
static Widget but2[3];

static void do_compute_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_compute_proc2(Widget w, XtPointer client_data, XtPointer call_data);
static void do_digfilter_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_linearc_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_xcor_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_spline_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_int_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_differ_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_seasonal_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_interp_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_regress_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_runavg_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_fourier_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_fft_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_window_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_histo_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_sample_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_prune_toggle(Widget w, XtPointer client_data, XtPointer call_data);
static void do_prune_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void set_regr_sensitivity(Widget , XtPointer , XtPointer );

typedef struct _Eval_ui {
    Widget top;
    SetChoiceItem sel;
    Widget formula_item;
    Widget *load_item;
    ListStructure *loadgraph_item;
    Widget *region_item;
    Widget rinvert_item;
} Eval_ui;

static Eval_ui eui;

void create_eval_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget dialog, rc;
    set_wait_cursor();
    if (eui.top == NULL) {
	char *label2[2];
	label2[0] = "Accept";
	label2[1] = "Close";
	eui.top = XmCreateDialogShell(app_shell, "Evaluate expression", NULL, 0);
	handle_close(eui.top);
	dialog = XmCreateRowColumn(eui.top, "dialog_rc", NULL, 0);

	eui.sel = CreateSetSelector(dialog, "Apply to set:",
				    SET_SELECT_ALL,
				    FILTER_SELECT_NONE,
				    GRAPH_SELECT_CURRENT,
				    SELECTION_TYPE_MULTIPLE);

	rc = XmCreateRowColumn(dialog, "rc", NULL, 0);
	eui.load_item = CreatePanelChoice(rc,
					  "Result to:", 3,
					  "Same set", "New set", NULL, 0);
	eui.loadgraph_item = CreateGraphChoice(rc, "In graph: ", LIST_TYPE_SINGLE);
	XtManageChild(rc);

	eui.formula_item = CreateScrollTextItem2(dialog, 30, 3,"Formula:");

	CreateSeparator(dialog);

	CreateCommandButtons(dialog, 2, but2, label2);
	XtAddCallback(but2[0], XmNactivateCallback, (XtCallbackProc) do_compute_proc, (XtPointer) & eui);
	XtAddCallback(but2[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) eui.top);

	XtManageChild(dialog);
    }
    XtRaise(eui.top);
    unset_wait_cursor();
}

/*
 * evaluate a formula
 */
static void do_compute_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int *selsets;
    int i, cnt;
    int setno, loadto, graphto, resno;
    char fstr[256];
    Eval_ui *ui = (Eval_ui *) client_data;
    if (w == NULL) {
	cnt = 1;
	selsets = (int *) malloc(sizeof(int));
	selsets[0] = pick_set;
    } else {
	cnt = GetSelectedSets(ui->sel, &selsets);
	if (cnt == SET_SELECT_ERROR) {
	    errwin("No sets selected");
	    return;
	}
    }
    loadto = GetChoice(ui->load_item);
    if (GetSingleListChoice(ui->loadgraph_item, &graphto) != GRACE_EXIT_SUCCESS) {
	errmsg("Please select single graph");
	return;
    }
    strcpy(fstr, xv_getstr(ui->formula_item));
    set_wait_cursor();
    for (i = 0; i < cnt; i++) {
	setno = selsets[i];
	resno = do_compute(setno, loadto, graphto, fstr);
	if (resno < 0) {
	    errwin("Error in  do_compute(), check expression");
	    break;
	}
    }
    update_set_lists(get_cg());
    free(selsets);
    unset_wait_cursor();
    drawgraph();
}

/* histograms */

typedef struct _Histo_ui {
    Widget top;
    SetChoiceItem sel;
    Widget binw_item;
    Widget hxmin_item;
    Widget hxmax_item;
    Widget *type_item;
    ListStructure *graph_item;
} Histo_ui;

static Histo_ui hui;

void create_histo_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget dialog;
    Widget rc;

    set_wait_cursor();
    if (hui.top == NULL) {
	char *label2[2];
	label2[0] = "Accept";
	label2[1] = "Close";
	hui.top = XmCreateDialogShell(app_shell, "Histograms", NULL, 0);
	handle_close(hui.top);
	dialog = XmCreateRowColumn(hui.top, "dialog_rc", NULL, 0);

	hui.sel = CreateSetSelector(dialog, "Apply to set:",
				    SET_SELECT_ALL,
				    FILTER_SELECT_NONE,
				    GRAPH_SELECT_CURRENT,
				    SELECTION_TYPE_MULTIPLE);
	rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, dialog,
			      XmNpacking, XmPACK_COLUMN,
			      XmNnumColumns, 4,
			      XmNorientation, XmHORIZONTAL,
			      XmNisAligned, True,
			      XmNadjustLast, False,
			      XmNentryAlignment, XmALIGNMENT_END,
			      NULL);

	XtVaCreateManagedWidget("Start value: ", xmLabelWidgetClass, rc, NULL);
	hui.hxmin_item = XtVaCreateManagedWidget("xmin", xmTextWidgetClass, rc, NULL);
	XtVaSetValues(hui.hxmin_item, XmNcolumns, 10, NULL);
	XtVaCreateManagedWidget("Ending value: ", xmLabelWidgetClass, rc, NULL);
	hui.hxmax_item = XtVaCreateManagedWidget("xmax", xmTextWidgetClass, rc, NULL);
	XtVaSetValues(hui.hxmax_item, XmNcolumns, 10, NULL);
	XtVaCreateManagedWidget("Bin width: ", xmLabelWidgetClass, rc, NULL);
	hui.binw_item = XtVaCreateManagedWidget("binwidth", xmTextWidgetClass, rc, NULL);
	XtVaSetValues(hui.binw_item, XmNcolumns, 10, NULL);
	XtManageChild(rc);

	CreateSeparator(dialog);
	hui.type_item = CreatePanelChoice(dialog, "Compute: ",
					  3,
					  "Histogram",
					  "Cumulative histogram",
					  0,
					  0);
	hui.graph_item = CreateGraphChoice(dialog, "Load result to graph:", LIST_TYPE_SINGLE);
	CreateSeparator(dialog);

	CreateCommandButtons(dialog, 2, but2, label2);
	XtAddCallback(but2[0], XmNactivateCallback, (XtCallbackProc) do_histo_proc, (XtPointer) & hui);
	XtAddCallback(but2[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) hui.top);

	XtManageChild(dialog);
    }
    XtRaise(hui.top);
    unset_wait_cursor();
}

/*
 * histograms
 */
static void do_histo_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int *selsets;
    int i, cnt;
    int fromset, toset, tograph, hist_type;
    double binw, xmin, xmax;
    Histo_ui *ui = (Histo_ui *) client_data;
    if (w == NULL) {
	cnt = 1;
	selsets = (int *) malloc(sizeof(int));
	selsets[0] = pick_set;
    } else {
	cnt = GetSelectedSets(ui->sel, &selsets);
	if (cnt == SET_SELECT_ERROR) {
	    errwin("No sets selected");
	    return;
	}
    }
    toset = SET_SELECT_NEXT;
    if (GetSingleListChoice(ui->graph_item, &tograph) != GRACE_EXIT_SUCCESS) {
	errmsg("Please select single graph");
	return;
    }
    if(xv_evalexpr(ui->binw_item, &binw) != GRACE_EXIT_SUCCESS ||
       xv_evalexpr(ui->hxmin_item, &xmin) != GRACE_EXIT_SUCCESS ||
       xv_evalexpr(ui->hxmax_item, &xmax) != GRACE_EXIT_SUCCESS ) {
        return;
    }
    hist_type = GetChoice(ui->type_item);
    set_wait_cursor();
    for (i = 0; i < cnt; i++) {
	fromset = selsets[i];
	do_histo(get_cg(), fromset, tograph, toset, binw, xmin, xmax, hist_type);
    }
    free(selsets);
    update_all();
    drawgraph();
    unset_wait_cursor();
}

/* DFTs */

typedef struct _Four_ui {
    Widget top;
    SetChoiceItem sel;
    Widget *load_item;
    Widget *window_item;
    Widget *loadx_item;
    Widget *inv_item;
    Widget *type_item;
    Widget *graph_item;
} Four_ui;

static Four_ui fui;

void create_fourier_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget dialog;
    Widget rc;
    Widget buts[4];

    set_wait_cursor();
    if (fui.top == NULL) {
	char *l[4];
	l[0] = "DFT";
	l[1] = "FFT";
	l[2] = "Window only";
	l[3] = "Close";
	fui.top = XmCreateDialogShell(app_shell, "Fourier transforms", NULL, 0);
	handle_close(fui.top);
	dialog = XmCreateRowColumn(fui.top, "dialog_rc", NULL, 0);

	fui.sel = CreateSetSelector(dialog, "Apply to set:",
				    SET_SELECT_ALL,
				    FILTER_SELECT_NONE,
				    GRAPH_SELECT_CURRENT,
				    SELECTION_TYPE_MULTIPLE);

	rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, dialog,
			      XmNpacking, XmPACK_COLUMN,
			      XmNnumColumns, 5,
			      XmNorientation, XmHORIZONTAL,
			      XmNisAligned, True,
			      XmNadjustLast, False,
			      XmNentryAlignment, XmALIGNMENT_END,
			      NULL);

	XtVaCreateManagedWidget("Data window: ", xmLabelWidgetClass, rc, NULL);
	fui.window_item = CreatePanelChoice(rc,
					    " ",
					    8,
					    "None (Rectangular)",
					    "Triangular",
					    "Hanning",
					    "Welch",
					    "Hamming",
					    "Blackman",
					    "Parzen",
					    NULL,
					    NULL);

	XtVaCreateManagedWidget("Load result as: ", xmLabelWidgetClass, rc, NULL);

	fui.load_item = CreatePanelChoice(rc,
					  " ",
					  4,
					  "Magnitude",
					  "Phase",
					  "Coefficients",
					  0,
					  0);

	XtVaCreateManagedWidget("Let result X = ", xmLabelWidgetClass, rc, NULL);
	fui.loadx_item = CreatePanelChoice(rc,
					   " ",
					   4,
					   "Index",
					   "Frequency",
					   "Period",
					   0,
					   0);

	XtVaCreateManagedWidget("Perform: ", xmLabelWidgetClass, rc, NULL);
	fui.inv_item = CreatePanelChoice(rc,
					 " ",
					 3,
					 "Transform",
					 "Inverse transform",
					 0,
					 0);

	XtVaCreateManagedWidget("Data is: ", xmLabelWidgetClass, rc, NULL);
	fui.type_item = CreatePanelChoice(rc,
					  " ",
					  3,
					  "Real",
					  "Complex",
					  0,
					  0);
	XtManageChild(rc);

	CreateSeparator(dialog);
	CreateCommandButtons(dialog, 4, buts, l);
	XtAddCallback(buts[0], XmNactivateCallback, (XtCallbackProc) do_fourier_proc, (XtPointer) & fui);
	XtAddCallback(buts[1], XmNactivateCallback, (XtCallbackProc) do_fft_proc, (XtPointer) & fui);
	XtAddCallback(buts[2], XmNactivateCallback, (XtCallbackProc) do_window_proc, (XtPointer) & fui);
	XtAddCallback(buts[3], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) fui.top);

	XtManageChild(dialog);
    }
    XtRaise(fui.top);
    unset_wait_cursor();
}

/*
 * DFT
 */
static void do_fourier_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int *selsets;
    int i, cnt;
    int setno, load, loadx, invflag, type, wind;
    Four_ui *ui = (Four_ui *) client_data;
    if (w == NULL) {
	cnt = 1;
	selsets = (int *) malloc(sizeof(int));
	selsets[0] = pick_set;
    } else {
	cnt = GetSelectedSets(ui->sel, &selsets);
	if (cnt == SET_SELECT_ERROR) {
	    errwin("No sets selected");
	    return;
	}
    }
    wind = GetChoice(ui->window_item);
    load = GetChoice(ui->load_item);
    loadx = GetChoice(ui->loadx_item);
    invflag = GetChoice(ui->inv_item);
    type = GetChoice(ui->type_item);
    set_wait_cursor();
    for (i = 0; i < cnt; i++) {
	setno = selsets[i];
	do_fourier(0, setno, load, loadx, invflag, type, wind);
    }
    update_set_lists(get_cg());
    free(selsets);
    unset_wait_cursor();
    drawgraph();
}

/*
 * DFT by FFT
 */
static void do_fft_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int *selsets;
    int i, cnt;
    int setno, load, loadx, invflag, type, wind;
    Four_ui *ui = (Four_ui *) client_data;
    cnt = GetSelectedSets(ui->sel, &selsets);
    if (cnt == SET_SELECT_ERROR) {
	errwin("No sets selected");
	return;
    }
    wind = GetChoice(ui->window_item);
    load = GetChoice(ui->load_item);
    loadx = GetChoice(ui->loadx_item);
    invflag = GetChoice(ui->inv_item);
    type = GetChoice(ui->type_item);
    set_wait_cursor();
    for (i = 0; i < cnt; i++) {
	setno = selsets[i];
	do_fourier(1, setno, load, loadx, invflag, type, wind);
    }
    update_set_lists(get_cg());
    free(selsets);
    unset_wait_cursor();
    drawgraph();
}

/*
 * Apply data window only
 */
static void do_window_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int *selsets;
    int i, cnt;
    int setno, type, wind;
    Four_ui *ui = (Four_ui *) client_data;
    if (w == NULL) {
	cnt = 1;
	selsets = (int *) malloc(sizeof(int));
	selsets[0] = pick_set;
    } else {
	cnt = GetSelectedSets(ui->sel, &selsets);
	if (cnt == SET_SELECT_ERROR) {
	    errwin("No sets selected");
	    return;
	}
    }
    wind = GetChoice(ui->window_item);
    type = GetChoice(ui->type_item);
    set_wait_cursor();
    for (i = 0; i < cnt; i++) {
	setno = selsets[i];
	do_window(setno, type, wind);
    }
    update_set_lists(get_cg());
    free(selsets);
    unset_wait_cursor();
    drawgraph();
}

/* running averages */

typedef struct _Run_ui {
    Widget top;
    SetChoiceItem sel;
    Widget len_item;
    Widget *type_item;
    Widget *region_item;
    Widget rinvert_item;
} Run_ui;

static Run_ui rui;

void create_run_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget dialog;
    Widget rc;

    set_wait_cursor();
    if (rui.top == NULL) {
	char *label2[2];
	label2[0] = "Accept";
	label2[1] = "Close";
	rui.top = XmCreateDialogShell(app_shell, "Running averages", NULL, 0);
	handle_close(rui.top);
	dialog = XmCreateRowColumn(rui.top, "dialog_rc", NULL, 0);

	rui.sel = CreateSetSelector(dialog, "Apply to set:",
				    SET_SELECT_ALL,
				    FILTER_SELECT_NONE,
				    GRAPH_SELECT_CURRENT,
				    SELECTION_TYPE_MULTIPLE);

	rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, dialog,
			      XmNpacking, XmPACK_COLUMN,
			      XmNnumColumns, 5,
			      XmNorientation, XmHORIZONTAL,
			      XmNisAligned, True,
			      XmNadjustLast, False,
			      XmNentryAlignment, XmALIGNMENT_END,
			      NULL);

	XtVaCreateManagedWidget("Running:", xmLabelWidgetClass, rc, NULL);
	rui.type_item = CreatePanelChoice(rc,
					  " ",
					  6,
					  "Average",
					  "Median",
					  "Minimum",
					  "Maximum",
					  "Std. dev.", 0,
					  0);
	rui.len_item = CreateTextItem4(rc, 10, "Length of average:");

	XtVaCreateManagedWidget("Restrictions:", xmLabelWidgetClass, rc, NULL);
	rui.region_item = CreatePanelChoice(rc,
					    " ",
					    9,
					    "None",
					    "Region 0",
					    "Region 1",
					    "Region 2",
					    "Region 3",
					    "Region 4",
					    "Inside graph",
					    "Outside graph",
					    0,
					    0);

	XtVaCreateManagedWidget("Invert region:", xmLabelWidgetClass, rc, NULL);
	rui.rinvert_item = XmCreateToggleButton(rc, " ", NULL, 0);
	XtManageChild(rui.rinvert_item);

	XtManageChild(rc);

	CreateSeparator(dialog);

	CreateCommandButtons(dialog, 2, but2, label2);
	XtAddCallback(but2[0], XmNactivateCallback, (XtCallbackProc) do_runavg_proc, (XtPointer) & rui);
	XtAddCallback(but2[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) rui.top);

	XtManageChild(dialog);
    }
    XtRaise(rui.top);
    unset_wait_cursor();
}

/*
 * running averages, medians, min, max, std. deviation
 */
static void do_runavg_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int *selsets;
    int i, cnt;
    int runlen, runtype, setno, rno, invr;
    Run_ui *ui = (Run_ui *) client_data;
    if (w == NULL) {
	cnt = 1;
	selsets = (int *) malloc(sizeof(int));
	selsets[0] = pick_set;
    } else {
	cnt = GetSelectedSets(ui->sel, &selsets);
	if (cnt == SET_SELECT_ERROR) {
	    errwin("No sets selected");
	    return;
	}
    }
    if (xv_evalexpri(ui->len_item, &runlen ) != GRACE_EXIT_SUCCESS) {
        return;
    }
    runtype = GetChoice(ui->type_item);
    rno = GetChoice(ui->region_item) - 1;
    invr = XmToggleButtonGetState(ui->rinvert_item);
    set_wait_cursor();
    for (i = 0; i < cnt; i++) {
	setno = selsets[i];
	do_runavg(setno, runlen, runtype, rno, invr);
    }
    update_set_lists(get_cg());
    unset_wait_cursor();
    free(selsets);
    drawgraph();
}

/* TODO finish this */
void do_eval_regress()
{
}

typedef struct _Reg_ui {
    Widget top;
    SetChoiceItem sel;
    Widget *degree_item;
    Widget zero_item;
    Widget *resid_item;
    Widget *region_item;
    Widget rinvert_item;
    Widget start_item;
    Widget stop_item;
    Widget step_item;
	Widget fload_rc;
    Widget method_item;
} Reg_ui;

static Reg_ui regui;


/*
 * set sensitivity of start, stop iand load buttons
 */
static void set_regr_sensitivity(Widget w, XtPointer client_data,
												XtPointer call_data)
{
	if( (int)client_data == 2 )
		XtSetSensitive( regui.fload_rc, True );
	else
		XtSetSensitive( regui.fload_rc, False );
}
	

void create_reg_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget dialog;
    Widget rc, rc2;
    Widget buts[2];
	int i;

    set_wait_cursor();
    if (regui.top == NULL) {
	char *label1[2];
	label1[0] = "Accept";
	label1[1] = "Close";
	regui.top = XmCreateDialogShell(app_shell, "Regression", NULL, 0);
	handle_close(regui.top);
	dialog = XmCreateRowColumn(regui.top, "dialog_rc", NULL, 0);

	regui.sel = CreateSetSelector(dialog, "Apply to set:",
				      SET_SELECT_ALL,
				      FILTER_SELECT_NONE,
				      GRAPH_SELECT_CURRENT,
				      SELECTION_TYPE_MULTIPLE);

	rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, dialog,
			      XmNorientation, XmVERTICAL,
 			      NULL);

	rc2 = XtVaCreateWidget("rc2", xmRowColumnWidgetClass, rc,
			      XmNorientation, XmHORIZONTAL,
 			      NULL);
			      
	XtVaCreateManagedWidget("Type of fit:", xmLabelWidgetClass, rc2, NULL);
	regui.degree_item = CreatePanelChoice(rc2,
					      " ",
					      16,
					      "Linear",
					      "Quadratic",
					      "Cubic",
					      "4th degree",
					      "5th degree",
					      "6th degree",
					      "7th degree",
					      "8th degree",
					      "9th degree",
					      "10th degree",
					      "1-10",
					      "Power y=A*x^B",
					      "Exponential y=A*exp(B*x)",
					      "Logarithmic y=A+B*ln(x)",
					      "Inverse y=1/(A+Bx)",
					      0,
					      0);
	XtManageChild(rc2);
	
	rc2 = XtVaCreateWidget("rc2", xmRowColumnWidgetClass, rc,
			      XmNorientation, XmHORIZONTAL,
 			      NULL);
			      
	XtVaCreateManagedWidget("Load:", xmLabelWidgetClass, rc2, NULL);
	regui.resid_item = CreatePanelChoice(rc2,
					     " ",
					     4,
					     "Fitted values",
					     "Residuals",
						 "Function",
					     0,
					     0);
    XtManageChild(rc2);
	for( i=2; i<5; i++ )
		XtAddCallback( regui.resid_item[i], XmNactivateCallback, 
					set_regr_sensitivity, (XtPointer)(i-2) );


	
	rc2 = XtVaCreateWidget("rc2", xmRowColumnWidgetClass, rc,
			      XmNorientation, XmHORIZONTAL,
 			      NULL);
	XtVaCreateManagedWidget("Restrictions:", xmLabelWidgetClass, rc2, NULL);
	regui.region_item = CreatePanelChoice(rc2,
					      " ",
					      9,
					      "None",
					      "Region 0",
					      "Region 1",
					      "Region 2",
					      "Region 3",
					      "Region 4",
					      "Inside graph",
					      "Outside graph",
					      0,
					      0);
	XtManageChild(rc2);
	
	rc2 = XtVaCreateWidget("rc2", xmRowColumnWidgetClass, rc,
			      XmNorientation, XmHORIZONTAL,
 			      NULL);
	XtVaCreateManagedWidget("Invert region:", xmLabelWidgetClass, rc2, NULL);
	regui.rinvert_item = XmCreateToggleButton(rc2, " ", NULL, 0);
	XtManageChild(regui.rinvert_item);
	XtManageChild(rc2);
	
	CreateSeparator(rc);

	regui.fload_rc = XmCreateRowColumn(rc, "nonl_fload_rc", NULL, 0);
	XtVaSetValues(regui.fload_rc, XmNorientation, XmHORIZONTAL, NULL);
	regui.start_item = CreateTextItem2(regui.fload_rc, 6, "Start load at:");
	regui.stop_item  = CreateTextItem2(regui.fload_rc, 6, "Stop load at:");
	regui.step_item  = CreateTextItem2(regui.fload_rc, 4, "# of points:");
	XtManageChild(regui.fload_rc);

	XtManageChild(rc);
	XtSetSensitive(regui.fload_rc, False);
	
	CreateSeparator(dialog);

	CreateCommandButtons(dialog, 2, buts, label1);
	XtAddCallback(buts[0], XmNactivateCallback, (XtCallbackProc) do_regress_proc, (XtPointer) & regui);
	XtAddCallback(buts[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) regui.top);

	XtManageChild(dialog);
    }
    XtRaise(regui.top);
    unset_wait_cursor();
}

/*
 * regression
 */
static void do_regress_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int *selsets;
    int cnt;
    Reg_ui *ui = (Reg_ui *) client_data;
    int setno, ideg, iresid, i, j, k;
    int rno = GetChoice(ui->region_item) - 1;
    int invr = XmToggleButtonGetState(ui->rinvert_item);
    int nstep = 0, rx, rset = 0;
    double xstart, xstop, stepsize = 0.0, *xr;

    if (w == NULL) {		/* called with "Pick" button */
		cnt = 1;
		selsets = (int *) malloc(sizeof(int));
		selsets[0] = pick_set;
    } else {
		cnt = GetSelectedSets(ui->sel, &selsets);
		if (cnt == SET_SELECT_ERROR) {
			errwin("No sets selected");
			return;
		}
    }
    ideg = (int) GetChoice(ui->degree_item) + 1;
	switch(rx=GetChoice(ui->resid_item) ){
		case 0:				/* evaluate fitted function at original x's */
			iresid = 0;
			rset = -1;
			break;
		case 1:				/* load residue at original x points */
			iresid = 1;
			rset = -1;
			break;
		case 2:		/* evaluate fitted function at new x points */
		    iresid = 0;
		    if(xv_evalexpri(ui->step_item, &nstep) != GRACE_EXIT_SUCCESS || nstep < 2 ) {
		            errwin("Number points < 2");
		            return;         
		    }
		    if(xv_evalexpr(ui->start_item, &xstart ) != GRACE_EXIT_SUCCESS) {
		            errwin("Specify starting value");
		            return;
		    }               
		    if(xv_evalexpr(ui->stop_item, &xstop) != GRACE_EXIT_SUCCESS) {
		            errwin("Specify stopping value");
		            return;
		    } else {
                        stepsize = (xstop - xstart)/(nstep-1);
                    }
		    break;
                default:
                    errwin("Internal error");
		    return;
	}	
    set_wait_cursor();
	for (i = (ideg==11?1:ideg); i <= (ideg==11?10:ideg); i++) {
    	for (j = 0; j < cnt; j++) {
			setno = selsets[j];
			if( rx == 2 ) {
				if( (rset = nextset( get_cg() )) == -1 ){
				     errwin("Not enough sets");
				     return;
				}
				activateset( get_cg(), rset );
				setlength( get_cg(), rset, nstep);
				xr = getx( get_cg(), rset );
				for( k=0; k<nstep; k++ )
					xr[k] = xstart+k*stepsize;
			}
			do_regress(setno, i, iresid, rno, invr, rset);
	    }
    }
    update_set_lists(get_cg());
    unset_wait_cursor();
    free(selsets);
    drawgraph();
}

/* finite differencing */

typedef struct _Diff_ui {
    Widget top;
    SetChoiceItem sel;
    Widget *type_item;
    Widget *region_item;
    Widget rinvert_item;
} Diff_ui;

static Diff_ui dui;

void create_diff_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget dialog;

    set_wait_cursor();
    if (dui.top == NULL) {
	char *label2[2];
	label2[0] = "Accept";
	label2[1] = "Close";
	dui.top = XmCreateDialogShell(app_shell, "Differences", NULL, 0);
	handle_close(dui.top);
	dialog = XmCreateRowColumn(dui.top, "dialog_rc", NULL, 0);

	dui.sel = CreateSetSelector(dialog, "Apply to set:",
				    SET_SELECT_ALL,
				    FILTER_SELECT_NONE,
				    GRAPH_SELECT_CURRENT,
				    SELECTION_TYPE_MULTIPLE);
	dui.type_item = CreatePanelChoice(dialog,
					  "Method:",
					  4,
					  "Forward difference",
					  "Backward difference",
					  "Centered difference",
					  0,
					  0);

	CreateSeparator(dialog);

	CreateCommandButtons(dialog, 2, but2, label2);
	XtAddCallback(but2[0], XmNactivateCallback, (XtCallbackProc) do_differ_proc, (XtPointer) & dui);
	XtAddCallback(but2[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) dui.top);

	XtManageChild(dialog);
    }
    XtRaise(dui.top);
    unset_wait_cursor();
}

/*
 * finite differences
 */
static void do_differ_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int *selsets;
    int i, cnt;
    int setno, itype;
    Diff_ui *ui = (Diff_ui *) client_data;
    if (w == NULL) {
	cnt = 1;
	selsets = (int *) malloc(sizeof(int));
	selsets[0] = pick_set;
    } else {
	cnt = GetSelectedSets(ui->sel, &selsets);
	if (cnt == SET_SELECT_ERROR) {
	    errwin("No sets selected");
	    return;
	}
    }
    itype = (int) GetChoice(ui->type_item);
    set_wait_cursor();
    for (i = 0; i < cnt; i++) {
	setno = selsets[i];
	do_differ(setno, itype);
    }
    update_set_lists(get_cg());
    unset_wait_cursor();
    free(selsets);
    drawgraph();
}

/* numerical integration */

typedef struct _Int_ui {
    Widget top;
    SetChoiceItem sel;
    Widget *type_item;
    Widget sum_item;
    Widget *region_item;
    Widget rinvert_item;
} Int_ui;

static Int_ui iui;

void create_int_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget dialog;

    set_wait_cursor();
    if (iui.top == NULL) {
	char *label2[2];
	label2[0] = "Accept";
	label2[1] = "Close";
	iui.top = XmCreateDialogShell(app_shell, "Integration", NULL, 0);
	handle_close(iui.top);
	dialog = XmCreateRowColumn(iui.top, "dialog_rc", NULL, 0);
	iui.sel = CreateSetSelector(dialog, "Apply to set:",
				    SET_SELECT_ALL,
				    FILTER_SELECT_NONE,
				    GRAPH_SELECT_CURRENT,
				    SELECTION_TYPE_MULTIPLE);

	iui.type_item = CreatePanelChoice(dialog,
					  "Load:",
					  3,
					  "Cumulative sum",
					  "Sum only",
					  0,
					  0);
	iui.sum_item = CreateTextItem2(dialog, 10, "Sum:");

	CreateSeparator(dialog);

	CreateCommandButtons(dialog, 2, but2, label2);
	XtAddCallback(but2[0], XmNactivateCallback, (XtCallbackProc) do_int_proc, (XtPointer) & iui);
	XtAddCallback(but2[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) iui.top);

	XtManageChild(dialog);
    }
    XtRaise(iui.top);
    unset_wait_cursor();
}

/*
 * numerical integration
 */
static void do_int_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int *selsets;
    int i, cnt;
    int setno, itype;
    double sum;
    Int_ui *ui = (Int_ui *) client_data;
    char buf[32];
    
    if (w == NULL) {
	cnt = 1;
	selsets = (int *) malloc(sizeof(int));
	selsets[0] = pick_set;
    } else {
	cnt = GetSelectedSets(ui->sel, &selsets);
	if (cnt == SET_SELECT_ERROR) {
	    errwin("No sets selected");
	    return;
	}
    }
    itype = GetChoice(ui->type_item);
    set_wait_cursor();
    for (i = 0; i < cnt; i++) {
	setno = selsets[i];
	sum = do_int(setno, itype);
	sprintf(buf, "%g", sum);
	xv_setstr(ui->sum_item, buf);
    }
    update_set_lists(get_cg());
    unset_wait_cursor();
    free(selsets);
    drawgraph();
}

/* seasonal differencing */

typedef struct _Seas_ui {
    Widget top;
    SetChoiceItem sel;
    Widget *type_item;
    Widget period_item;
    Widget *region_item;
    Widget rinvert_item;
} Seas_ui;

static Seas_ui sui;

void create_seasonal_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget dialog;

    set_wait_cursor();
    if (sui.top == NULL) {
	char *label2[2];
	label2[0] = "Accept";
	label2[1] = "Close";
	sui.top = XmCreateDialogShell(app_shell, "Seasonal differences", NULL, 0);
	handle_close(sui.top);
	dialog = XmCreateRowColumn(sui.top, "dialog_rc", NULL, 0);

	sui.sel = CreateSetSelector(dialog, "Apply to set:",
				    SET_SELECT_ALL,
				    FILTER_SELECT_NONE,
				    GRAPH_SELECT_CURRENT,
				    SELECTION_TYPE_MULTIPLE);
	sui.period_item = CreateTextItem2(dialog, 10, "Period:");

	CreateSeparator(dialog);

	CreateCommandButtons(dialog, 2, but2, label2);
	XtAddCallback(but2[0], XmNactivateCallback, (XtCallbackProc) do_seasonal_proc, (XtPointer) & sui);
	XtAddCallback(but2[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) sui.top);

	XtManageChild(dialog);
    }
    XtRaise(sui.top);
    unset_wait_cursor();
}

/*
 * seasonal differences
 */
static void do_seasonal_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int *selsets;
    int i, cnt;
    int setno, period;
    Seas_ui *ui = (Seas_ui *) client_data;
    cnt = GetSelectedSets(ui->sel, &selsets);
    if (w == NULL) {
	cnt = 1;
	selsets = (int *) malloc(sizeof(int));
	selsets[0] = pick_set;
    } else {
	cnt = GetSelectedSets(ui->sel, &selsets);
	if (cnt == SET_SELECT_ERROR) {
	    errwin("No sets selected");
	    return;
	}
    }
    if(xv_evalexpri(ui->period_item, &period ) != GRACE_EXIT_SUCCESS)
		return;
    set_wait_cursor();
    for (i = 0; i < cnt; i++) {
		setno = selsets[i];
		do_seasonal_diff(setno, period);
    }
    update_set_lists(get_cg());
    free(selsets);
    unset_wait_cursor();
    drawgraph();
}

/* interpolation */

typedef struct _Interp_ui {
    Widget top;
    SetChoiceItem sel1;
    SetChoiceItem sel2;
    Widget *type_item;
    Widget *region_item;
    Widget *meth_item;
    Widget rinvert_item;
} Interp_ui;

static Interp_ui interpui;

void create_interp_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget dialog;

    set_wait_cursor();
    if (interpui.top == NULL) {
	char *label2[3];
	label2[0] = "Accept";
	label2[1] = "Close";
	interpui.top = XmCreateDialogShell(app_shell, "Interpolation", NULL, 0);
	handle_close(interpui.top);
	dialog = XmCreateRowColumn(interpui.top, "dialog_rc", NULL, 0);

	interpui.sel1 = CreateSetSelector(dialog, "Interpolate on set:",
					  SET_SELECT_ACTIVE,
					  FILTER_SELECT_NONE,
					  GRAPH_SELECT_CURRENT,
					  SELECTION_TYPE_SINGLE);
	interpui.sel2 = CreateSetSelector(dialog, "At points from set:",
					  SET_SELECT_ACTIVE,
					  FILTER_SELECT_NONE,
					  GRAPH_SELECT_CURRENT,
					  SELECTION_TYPE_SINGLE);

	interpui.meth_item = CreatePanelChoice(dialog,
					  "Method:",
					  4,
					  "Linear",
					  "Spline",
					  "Akima",
					  NULL, 0);
					  
	CreateSeparator(dialog);

	CreateCommandButtons(dialog, 2, but2, label2);
	XtAddCallback(but2[0], XmNactivateCallback, (XtCallbackProc) do_interp_proc, (XtPointer) & interpui);
	XtAddCallback(but2[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) interpui.top);

	XtManageChild(dialog);
    }
    XtRaise(interpui.top);
    unset_wait_cursor();
}

/*
 * interpolation
 */
static void do_interp_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
/* TODO
    int *selsets;
    int i, cnt;
    int setno;
    if (w == NULL) {
	cnt = 1;
	selsets = (int *) malloc(sizeof(int));
	selsets[0] = pick_set;
    } else {
	cnt = GetSelectedSets(ui->sel, &selsets);
	if (cnt == SET_SELECT_ERROR) {
	    errwin("No sets selected");
	    return;
	}
    }
    set_wait_cursor();
    for (i = 0; i < cnt; i++) {
	setno = selsets[i];
    }
    update_set_lists(get_cg());
    unset_wait_cursor();
    free(selsets);
    drawgraph();
*/
    int set1, set2, method;
    Interp_ui *ui = (Interp_ui *) client_data;
    set1 = GetSelectedSet(ui->sel1);
    set2 = GetSelectedSet(ui->sel2);
    if (set1 == SET_SELECT_ERROR || set2 == SET_SELECT_ERROR) {
		errwin("Select 2 sets");
		return;
    }
    method = (int) GetChoice(ui->meth_item);
    set_wait_cursor();
    do_interp(set1, set2, method);
    update_set_lists(get_cg());
    unset_wait_cursor();
}

/* cross correlation */

typedef struct _Cross_ui {
    Widget top;
    SetChoiceItem sel1;
    SetChoiceItem sel2;
    Widget lag_item;
    Widget *region_item;
    Widget rinvert_item;
} Cross_ui;

static Cross_ui crossui;

void create_xcor_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget dialog;

    set_wait_cursor();
    if (crossui.top == NULL) {
	char *label2[3];
	label2[0] = "Accept";
	label2[1] = "Close";
	crossui.top = XmCreateDialogShell(app_shell, "X-correlation", NULL, 0);
	handle_close(crossui.top);
	dialog = XmCreateRowColumn(crossui.top, "dialog_rc", NULL, 0);

	crossui.sel1 = CreateSetSelector(dialog, "Select set:",
					 SET_SELECT_ACTIVE,
					 FILTER_SELECT_NONE,
					 GRAPH_SELECT_CURRENT,
					 SELECTION_TYPE_SINGLE);
	crossui.sel2 = CreateSetSelector(dialog, "Select set:",
					 SET_SELECT_ACTIVE,
					 FILTER_SELECT_NONE,
					 GRAPH_SELECT_CURRENT,
					 SELECTION_TYPE_SINGLE);
	crossui.lag_item = CreateTextItem2(dialog, 10, "Maximum lag:");

	CreateSeparator(dialog);

	CreateCommandButtons(dialog, 2, but2, label2);
	XtAddCallback(but2[0], XmNactivateCallback, (XtCallbackProc) do_xcor_proc, (XtPointer) & crossui);
	XtAddCallback(but2[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) crossui.top);

	XtManageChild(dialog);
    }
    XtRaise(crossui.top);
    unset_wait_cursor();
}

/*
 * cross correlation
 */
static void do_xcor_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int set1, set2, maxlag;
    Cross_ui *ui = (Cross_ui *) client_data;
    set1 = GetSelectedSet(ui->sel1);
    set2 = GetSelectedSet(ui->sel2);
    if (set1 == SET_SELECT_ERROR || set2 == SET_SELECT_ERROR) {
	errwin("Select 2 sets");
	return;
    }
    if(xv_evalexpri(ui->lag_item, &maxlag) != GRACE_EXIT_SUCCESS) { 
        return;
    }
    set_wait_cursor();
    do_xcor(set1, set2, maxlag);
    update_set_lists(get_cg());
    drawgraph();
    unset_wait_cursor();
}

/* splines */

typedef struct _Spline_ui {
    Widget top;
    SetChoiceItem sel;
    Widget *type_item;
    Widget start_item;
    Widget stop_item;
    Widget step_item;
    Widget *region_item;
    Widget rinvert_item;
} Spline_ui;

static Spline_ui splineui;

void create_spline_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    static Widget dialog;

    set_wait_cursor();
    if (splineui.top == NULL) {
	char *label2[2];
	label2[0] = "Accept";
	label2[1] = "Close";
	splineui.top = XmCreateDialogShell(app_shell, "Splines", NULL, 0);
	handle_close(splineui.top);
	dialog = XmCreateRowColumn(splineui.top, "dialog_rc", NULL, 0);

	splineui.sel = CreateSetSelector(dialog, "Apply to set:",
					 SET_SELECT_ALL,
					 FILTER_SELECT_NONE,
					 GRAPH_SELECT_CURRENT,
					 SELECTION_TYPE_MULTIPLE);


	splineui.start_item = CreateTextItem2(dialog, 10, "Start:");
	splineui.stop_item = CreateTextItem2(dialog, 10, "Stop:");
	splineui.step_item = CreateTextItem2(dialog, 6, "Number of points:");
	splineui.type_item = CreatePanelChoice(dialog,
					  "Spline type:",
					  3,
					  "Cubic",
					  "Akima",
					  0, 0);

	CreateSeparator(dialog);

	CreateCommandButtons(dialog, 2, but2, label2);
	XtAddCallback(but2[0], XmNactivateCallback, (XtCallbackProc) do_spline_proc, (XtPointer) & splineui);
	XtAddCallback(but2[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) splineui.top);

	XtManageChild(dialog);
    }
    XtRaise(splineui.top);
    unset_wait_cursor();
}

/*
 * splines
 */
static void do_spline_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int *selsets;
    int i, cnt;
    int setno, n;
    int stype;
    double start, stop;
    Spline_ui *ui = (Spline_ui *) client_data;
    if (w == NULL) {
	cnt = 1;
	selsets = (int *) malloc(sizeof(int));
	selsets[0] = pick_set;
    } else {
	cnt = GetSelectedSets(ui->sel, &selsets);
	if (cnt == SET_SELECT_ERROR) {
	    errwin("No sets selected");
	    return;
	}
    }
    if(xv_evalexpr(ui->start_item, &start) != GRACE_EXIT_SUCCESS ||
       xv_evalexpr(ui->stop_item,  &stop)  != GRACE_EXIT_SUCCESS ||
       xv_evalexpri(ui->step_item, &n)     != GRACE_EXIT_SUCCESS )
		return;

    stype = GetChoice(ui->type_item);
    
    set_wait_cursor();
    for (i = 0; i < cnt; i++) {
	setno = selsets[i];
	do_spline(setno, start, stop, n, stype+1);
    }
    update_set_lists(get_cg());
    unset_wait_cursor();

    free(selsets);
    drawgraph();
}

/* sample a set */

typedef struct _Samp_ui {
    Widget top;
    SetChoiceItem sel;
    Widget *type_item;
    Widget start_item;
    Widget step_item;
    Widget expr_item;
    Widget *region_item;
    Widget rinvert_item;
} Samp_ui;

static Samp_ui sampui;

void create_samp_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    static Widget dialog;
    Widget rc;

    set_wait_cursor();
    if (sampui.top == NULL) {
	char *label2[2];
	label2[0] = "Accept";
	label2[1] = "Close";
	sampui.top = XmCreateDialogShell(app_shell, "Sample points", NULL, 0);
	handle_close(sampui.top);
	dialog = XmCreateRowColumn(sampui.top, "dialog_rc", NULL, 0);

	sampui.sel = CreateSetSelector(dialog, "Apply to set:",
				       SET_SELECT_ALL,
				       FILTER_SELECT_NONE,
				       GRAPH_SELECT_CURRENT,
				       SELECTION_TYPE_MULTIPLE);

	rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, dialog,
			      XmNpacking, XmPACK_COLUMN,
			      XmNnumColumns, 5,
			      XmNorientation, XmHORIZONTAL,
			      XmNisAligned, True,
			      XmNadjustLast, False,
			      XmNentryAlignment, XmALIGNMENT_END,
			      NULL);

	XtVaCreateManagedWidget("Sample type:", xmLabelWidgetClass, rc, NULL);
	sampui.type_item = CreatePanelChoice(rc,
					     " ",
					     3,
					     "Start/step",
					     "Expression",
					     0,
					     0);
	sampui.start_item = CreateTextItem4(rc, 10, "Start:");
	sampui.step_item = CreateTextItem4(rc, 10, "Step:");
	sampui.expr_item = CreateTextItem4(rc, 10, "Logical expression:");
	XtManageChild(rc);

	CreateSeparator(dialog);

	CreateCommandButtons(dialog, 2, but2, label2);
	XtAddCallback(but2[0], XmNactivateCallback, (XtCallbackProc) do_sample_proc, (XtPointer) & sampui);
	XtAddCallback(but2[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) sampui.top);

	XtManageChild(dialog);
    }
    XtRaise(sampui.top);
    unset_wait_cursor();
}

/*
 * sample a set, by start/step or logical expression
 */
static void do_sample_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int *selsets;
    int i, cnt;
    int setno, typeno;
    char exprstr[256];
    int startno, stepno;
    Samp_ui *ui = (Samp_ui *) client_data;
    if (w == NULL) {
	cnt = 1;
	selsets = (int *) malloc(sizeof(int));
	selsets[0] = pick_set;
    } else {
	cnt = GetSelectedSets(ui->sel, &selsets);
	if (cnt == SET_SELECT_ERROR) {
	    errwin("No sets selected");
	    return;
	}
    }
    typeno = (int) GetChoice(ui->type_item);
	
	if(xv_evalexpri(ui->start_item, &startno) != GRACE_EXIT_SUCCESS||
	   xv_evalexpri(ui->step_item, &stepno) != GRACE_EXIT_SUCCESS)
		return;
    set_wait_cursor();
    for (i = 0; i < cnt; i++) {
		setno = selsets[i];
/* exprstr gets clobbered */
		strcpy(exprstr, (char *) xv_getstr(ui->expr_item));
		do_sample(setno, typeno, exprstr, startno, stepno);
    }
    update_set_lists(get_cg());
    unset_wait_cursor();
    free(selsets);
    drawgraph();
}

/* Prune data */

typedef struct _Prune_ui {
    Widget top;
    SetChoiceItem sel;
    Widget *type_item;
    Widget *dxtype_item;
    Widget *dytype_item;
    Widget *deltatype_item;
    Widget dx_rc;
    Widget dy_rc;
    Widget dx_item;
    Widget dy_item;
} Prune_ui;

static Prune_ui pruneui;

void create_prune_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    int i;
    static Widget dialog;

    set_wait_cursor();
    if (pruneui.top == NULL) {
	char *label2[2];
	label2[0] = "Accept";
	label2[1] = "Close";
	pruneui.top = XmCreateDialogShell(app_shell, "Prune data", NULL, 0);
	handle_close(pruneui.top);
	dialog = XmCreateRowColumn(pruneui.top, "dialog_rc", NULL, 0);

	pruneui.sel = CreateSetSelector(dialog, "Apply to set:",
	    SET_SELECT_ALL, FILTER_SELECT_NONE, GRAPH_SELECT_CURRENT,
	    SELECTION_TYPE_MULTIPLE);

	pruneui.type_item = CreatePanelChoice(dialog,
	    "Prune type: ", 5,
	    "Interpolation", "Circle", "Ellipse", "Rectangle",
	    NULL, 0);

	pruneui.dx_rc = XtVaCreateWidget("dx_rc",
            xmRowColumnWidgetClass, dialog,
            XmNorientation, XmHORIZONTAL,
	    NULL);
	pruneui.dx_item = CreateTextItem4(pruneui.dx_rc, 17, "Delta X:");
        XtManageChild(pruneui.dx_rc);

	pruneui.dy_rc = XtVaCreateWidget("dy_rc",
            xmRowColumnWidgetClass, dialog,
            XmNorientation, XmHORIZONTAL,
	    NULL);
	pruneui.dy_item = CreateTextItem4(pruneui.dy_rc, 17, "Delta Y:");
        XtManageChild(pruneui.dy_rc);

	CreateSeparator(dialog);

        pruneui.deltatype_item = CreatePanelChoice(dialog,
	    "Type of Delta coordinates:", 3, "Viewport", "World", NULL, 0);
	
	pruneui.dxtype_item = CreatePanelChoice(dialog,
            "Scaling of Delta X:", 3, "Linear", "Logarithmic", NULL, 0);
	
	pruneui.dytype_item = CreatePanelChoice(dialog,
            "Scaling of Delta Y:", 3, "Linear", "Logarithmic", NULL, 0);

        update_prune_frame();

        for (i = 0; i <= 3; i++) {
            XtAddCallback(pruneui.type_item[2 + i], XmNactivateCallback,
                (XtCallbackProc) do_prune_toggle, (XtPointer) &pruneui);
        }
	for (i = 0; i <= 1; i++) {
            XtAddCallback(pruneui.deltatype_item[2 + i], XmNactivateCallback,
                (XtCallbackProc) do_prune_toggle, (XtPointer) &pruneui);
        }
        do_prune_toggle ((Widget) NULL, (XtPointer) &pruneui, 0);

	CreateSeparator(dialog);

	CreateCommandButtons(dialog, 2, but2, label2);
	XtAddCallback(but2[0], XmNactivateCallback, (XtCallbackProc) do_prune_proc, (XtPointer) & pruneui);
	XtAddCallback(but2[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) pruneui.top);

	XtManageChild(dialog);
    }
    XtRaise(pruneui.top);
    unset_wait_cursor();
}

void update_prune_frame(void)
{
    if (pruneui.top != NULL) {
        SetChoice(pruneui.dxtype_item,
            (get_graph_xscale(get_cg()) == SCALE_LOG) ? 1 : 0);
        SetChoice(pruneui.dytype_item,
            (get_graph_yscale(get_cg()) == SCALE_LOG) ? 1 : 0);
    }
}

/*
 * Toggle prune type
 */
static void do_prune_toggle(Widget w, XtPointer client_data, XtPointer call_data)
{
    Prune_ui *ui = (Prune_ui *) client_data;
    int typeno = (int) GetChoice(ui->type_item);
    int deltatypeno = (int) GetChoice(ui->deltatype_item);

    switch (typeno) {
        case PRUNE_CIRCLE:
	    XtSetSensitive(pruneui.dx_rc, TRUE);
	    XtSetSensitive(pruneui.dy_rc, FALSE);
	    switch (deltatypeno) {
		case PRUNE_VIEWPORT:
		    XtSetSensitive(*pruneui.dxtype_item, FALSE);
		    XtSetSensitive(*pruneui.dytype_item, FALSE);
		    break;
		case PRUNE_WORLD:
		    XtSetSensitive(*pruneui.dxtype_item, TRUE);
		    XtSetSensitive(*pruneui.dytype_item, FALSE);
		    break;
	    }
	    break;
        case PRUNE_ELLIPSE:
        case PRUNE_RECTANGLE:
	    XtSetSensitive(pruneui.dx_rc, TRUE);
	    XtSetSensitive(pruneui.dy_rc, TRUE);
	    switch (deltatypeno) {
		case PRUNE_VIEWPORT:
		    XtSetSensitive(*pruneui.dxtype_item, FALSE);
		    XtSetSensitive(*pruneui.dytype_item, FALSE);
		    break;
		case PRUNE_WORLD:
		    XtSetSensitive(*pruneui.dxtype_item, TRUE);
		    XtSetSensitive(*pruneui.dytype_item, TRUE);
		    break;
	    }
	    break;
        case PRUNE_INTERPOLATION:
	    XtSetSensitive(pruneui.dx_rc, FALSE);
	    XtSetSensitive(pruneui.dy_rc, TRUE);
	    switch (deltatypeno) {
		case PRUNE_VIEWPORT:
		    XtSetSensitive(*pruneui.dxtype_item, FALSE);
		    XtSetSensitive(*pruneui.dytype_item, FALSE);
		    break;
		case PRUNE_WORLD:
		    XtSetSensitive(*pruneui.dxtype_item, FALSE);
		    XtSetSensitive(*pruneui.dytype_item, TRUE);
		    break;
	    }
	    break;
    }
}

/*
 * Prune data
 */
static void do_prune_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int *selsets;
    int i, cnt;
    int setno, typeno, deltatypeno;
    int dxtype, dytype;
    double deltax, deltay;

    Prune_ui *ui = (Prune_ui *) client_data;
    if (w == NULL) {
	cnt = 1;
	selsets = (int *) malloc(sizeof(int));
	selsets[0] = pick_set;
    } else {
	cnt = GetSelectedSets(ui->sel, &selsets);
	if (cnt == SET_SELECT_ERROR) {
	    errwin("No sets selected");
	    return;
	}
    }
    typeno = (int) GetChoice(ui->type_item);
    deltatypeno = (int) GetChoice(ui->deltatype_item);
    dxtype = (int) GetChoice(ui->dxtype_item);
    dytype = (int) GetChoice(ui->dytype_item);

	if( XtIsSensitive(ui->dx_rc)== True ){
		if(xv_evalexpr(ui->dx_item, &deltax) != GRACE_EXIT_SUCCESS)
			return;
	} else
		deltax = 0;
	if( XtIsSensitive(ui->dy_rc)== True ){
		if(xv_evalexpr(ui->dy_item, &deltay) != GRACE_EXIT_SUCCESS )
			return;
	} else
		deltay = 0;	
	
    set_wait_cursor();
    for (i = 0; i < cnt; i++) {
	setno = selsets[i];

	do_prune(setno, typeno, deltatypeno, deltax, deltay, dxtype, dytype);
    }
    update_set_lists(get_cg());
    unset_wait_cursor();
    free(selsets);
    drawgraph();
}

/* apply a digital filter in set 2 to set 1 */

typedef struct _Digf_ui {
    Widget top;
    SetChoiceItem sel1;
    SetChoiceItem sel2;
    Widget *type_item;
    Widget *region_item;
    Widget rinvert_item;
} Digf_ui;

static Digf_ui digfui;

void create_digf_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget dialog;

    set_wait_cursor();
    if (digfui.top == NULL) {
	char *label1[2];
	label1[0] = "Accept";
	label1[1] = "Close";
	digfui.top = XmCreateDialogShell(app_shell, "Digital filter", NULL, 0);
	handle_close(digfui.top);
	dialog = XmCreateRowColumn(digfui.top, "dialog_rc", NULL, 0);

	digfui.sel1 = CreateSetSelector(dialog, "Filter set:",
					SET_SELECT_ACTIVE,
					FILTER_SELECT_NONE,
					GRAPH_SELECT_CURRENT,
					SELECTION_TYPE_SINGLE);
	digfui.sel2 = CreateSetSelector(dialog, "With weights from set:",
					SET_SELECT_ACTIVE,
					FILTER_SELECT_NONE,
					GRAPH_SELECT_CURRENT,
					SELECTION_TYPE_SINGLE);

	CreateSeparator(dialog);

	CreateCommandButtons(dialog, 2, but1, label1);
	XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc) do_digfilter_proc, (XtPointer) & digfui);
	XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) digfui.top);

	XtManageChild(dialog);
    }
    XtRaise(digfui.top);
    unset_wait_cursor();
}

/*
 * apply a digital filter
 */
static void do_digfilter_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int set1, set2;
    Digf_ui *ui = (Digf_ui *) client_data;
    set1 = GetSelectedSet(ui->sel1);
    set2 = GetSelectedSet(ui->sel2);
    if (set1 == SET_SELECT_ERROR || set2 == SET_SELECT_ERROR) {
	errwin("Select 2 sets");
	return;
    }
    set_wait_cursor();
    do_digfilter(set1, set2);
    update_set_lists(get_cg());
    unset_wait_cursor();
}

/* linear convolution */

typedef struct _Lconv_ui {
    Widget top;
    SetChoiceItem sel1;
    SetChoiceItem sel2;
    Widget *type_item;
    Widget lag_item;
    Widget *region_item;
    Widget rinvert_item;
} Lconv_ui;

static Lconv_ui lconvui;

void create_lconv_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget dialog;

    set_wait_cursor();
    if (lconvui.top == NULL) {
	char *label1[2];
	label1[0] = "Accept";
	label1[1] = "Close";
	lconvui.top = XmCreateDialogShell(app_shell, "Linear convolution", NULL, 0);
	handle_close(lconvui.top);
	dialog = XmCreateRowColumn(lconvui.top, "dialog_rc", NULL, 0);

	lconvui.sel1 = CreateSetSelector(dialog, "Convolve set:",
					 SET_SELECT_ACTIVE,
					 FILTER_SELECT_NONE,
					 GRAPH_SELECT_CURRENT,
					 SELECTION_TYPE_SINGLE);
	lconvui.sel2 = CreateSetSelector(dialog, "With set:",
					 SET_SELECT_ACTIVE,
					 FILTER_SELECT_NONE,
					 GRAPH_SELECT_CURRENT,
					 SELECTION_TYPE_SINGLE);

	CreateSeparator(dialog);

	CreateCommandButtons(dialog, 2, but1, label1);
	XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc) do_linearc_proc, (XtPointer) & lconvui);
	XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) lconvui.top);

	XtManageChild(dialog);
    }
    XtRaise(lconvui.top);
    unset_wait_cursor();
}

/*
 * linear convolution
 */
static void do_linearc_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int set1, set2;
    Lconv_ui *ui = (Lconv_ui *) client_data;
    set1 = GetSelectedSet(ui->sel1);
    set2 = GetSelectedSet(ui->sel2);
    if (set1 == SET_SELECT_ERROR || set2 == SET_SELECT_ERROR) {
	errwin("Select 2 sets");
	return;
    }
    set_wait_cursor();
    do_linearc(set1, set2);
    update_set_lists(get_cg());
    drawgraph();
    unset_wait_cursor();
}

/* evaluate a formula - load the next set */

typedef struct _Leval_ui {
    Widget top;
    SetChoiceItem sel1;
    SetChoiceItem sel2;
    Widget *load_item;
    Widget x_item;
    Widget y_item;
    Widget start_item;
    Widget stop_item;
    Widget npts_item;
    Widget *region_item;
    Widget rinvert_item;
    int gno;
} Leval_ui;

static Leval_ui levalui;

void create_leval_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget dialog;
    Widget rc;
    int gno = (int) client_data;

    set_wait_cursor();
    if (is_valid_gno(gno)) {
        levalui.gno = gno;
    } else {
        levalui.gno = get_cg();
    }
    if (levalui.top == NULL) {
	char *label1[2];
	label1[0] = "Accept";
	label1[1] = "Close";
	levalui.top = XmCreateDialogShell(app_shell, "Load & evaluate", NULL, 0);
	handle_close(levalui.top);
	dialog = XmCreateRowColumn(levalui.top, "dialog_rc", NULL, 0);

	rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, dialog,
			      XmNpacking, XmPACK_COLUMN,
			      XmNnumColumns, 6,
			      XmNorientation, XmHORIZONTAL,
			      XmNisAligned, True,
			      XmNadjustLast, False,
			      XmNentryAlignment, XmALIGNMENT_END,
			      NULL);
 
	levalui.x_item = CreateScrollTextItem2(dialog, 40, 3, "X = ");
	levalui.y_item = CreateScrollTextItem2(dialog, 40, 3, "Y = ");

	XtVaCreateManagedWidget("Load:", xmLabelWidgetClass, rc, NULL);
	levalui.load_item = CreatePanelChoice(rc,
					      "",
					      7,
					      "Set X",
					      "Set Y",
					      "Scratch A",
					      "Scratch B",
					      "Scratch C",
					      "Scratch D", 0,
					      0);
	levalui.start_item = CreateTextItem4(rc, 10, "Start load at:");
	levalui.stop_item = CreateTextItem4(rc, 10, "Stop load at:");
	levalui.npts_item = CreateTextItem4(rc, 10, "# of points:");
	XtManageChild(rc);
	CreateSeparator(dialog);

	CreateCommandButtons(dialog, 2, but1, label1);
	XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc) do_compute_proc2, (XtPointer) & levalui);
	XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) levalui.top);

	XtManageChild(dialog);
    }
    XtRaise(levalui.top);
    unset_wait_cursor();
}





void loadset(int gno, int selset, int toval, double startno, double stepno)
{
    int i, lenset;
    double *ltmp;
    double *xtmp, *ytmp;

    if ((lenset = getsetlength(gno, selset)) <= 0) {
	char stmp[60];

	sprintf(stmp, "Length of set %d <= 0", selset);
	errmsg(stmp);
	return;
    }
    xtmp = getx(gno, selset);
    ytmp = gety(gno, selset);
    switch (toval) {
    case 1:
	ltmp = xtmp;
	break;
    case 2:
	ltmp = ytmp;
	break;
    case 3:
	init_scratch_arrays(lenset);
        ltmp = get_scratch(0);
	break;
    case 4:
	init_scratch_arrays(lenset);
	ltmp = get_scratch(1);
	break;
    case 5:
	init_scratch_arrays(lenset);
	ltmp = get_scratch(2);
	break;
    case 6:
	init_scratch_arrays(lenset);
	ltmp = get_scratch(3);
	break;
    default:
	return;
    }
    for (i = 0; i < lenset; i++) {
	*ltmp++ = startno + i * stepno;
    }

    set_dirtystate();
}



/*
 * evaluate a formula loading the next set
 */
void do_compute2(int gno, char *fstrx, char *fstry, double start, double stop, int npts, int toval)
{
    int setno;
    double step;
    char buf[MAX_STRING_LENGTH];

    if (npts < 2) {
	errmsg("Number of points < 2");
	return;
    }

    setno = nextset(gno);
    if (setno < 0) {
	return;
    }
    activateset(gno, setno);
    setlength(gno, setno, npts);
    if (strlen(fstrx) == 0) {
	errmsg("Undefined expression for X");
	return;
    }
    if (strlen(fstry) == 0) {
	errmsg("Undefined expression for Y");
	return;
    }

    step = (stop - start) / (npts - 1);
    loadset(gno, setno, toval, start, step);

    sprintf(buf, "X=%s", fstrx);
    formula(gno, setno, buf);

    sprintf(buf, "Y=%s", fstry);
    formula(gno, setno, buf);

    sprintf(buf, "X=%s, Y=%s", fstrx, fstry);
    setcomment(gno, setno, buf);
}


/*
 * evaluate a formula loading the next set
 */
static void do_compute_proc2(Widget w, XtPointer client_data, XtPointer call_data)
{
    int npts, toval;
    double start, stop;
    char fstrx[256], fstry[256];
    Leval_ui *ui = (Leval_ui *) client_data;

    if (xv_evalexpri(ui->npts_item, &npts) != GRACE_EXIT_SUCCESS) {
	errmsg("Number of points undefined");
        return;
    }

    if (xv_evalexpr(ui->start_item, &start) != GRACE_EXIT_SUCCESS) {
	errmsg("Start item undefined");
        return;
    }

    if (xv_evalexpr(ui->stop_item, &stop) != GRACE_EXIT_SUCCESS) {
	errmsg("Stop item undefined");
        return;
    }

    strcpy(fstrx, xv_getstr(ui->x_item));
    strcpy(fstry, xv_getstr(ui->y_item));

    toval = GetChoice(ui->load_item) + 1;

    set_wait_cursor();
    do_compute2(ui->gno, fstrx, fstry, start, stop, npts, toval);
    update_all();
    drawgraph();
    unset_wait_cursor();
}


/*
 * Rotate, scale, translate
 */

typedef struct _Geom_ui {
    Widget top;
    SetChoiceItem sel;
    SetChoiceItem sel2;
    Widget *order_item;
    Widget degrees_item;
    Widget rotx_item;
    Widget roty_item;
    Widget scalex_item;
    Widget scaley_item;
    Widget transx_item;
    Widget transy_item;
    Widget *region_item;
    Widget rinvert_item;
} Geom_ui;

static Geom_ui gui;

static void do_geom_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void reset_geom_proc(Widget, XtPointer, XtPointer);

void create_geom_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget dialog;
    Widget rc;

    set_wait_cursor();
    if (gui.top == NULL) {
	char *label1[3];
	label1[0] = "Accept";
	label1[1] = "Reset";
	label1[2] = "Close";
	gui.top = XmCreateDialogShell(app_shell, "Geometric transformations", NULL, 0);
	handle_close(gui.top);
	dialog = XmCreateRowColumn(gui.top, "dialog_rc", NULL, 0);

	gui.sel = CreateSetSelector(dialog, "Apply to set:",
				    SET_SELECT_ALL,
				    FILTER_SELECT_NONE,
				    GRAPH_SELECT_CURRENT,
				    SELECTION_TYPE_MULTIPLE);

	rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, dialog,
			      XmNpacking, XmPACK_COLUMN,
			      XmNnumColumns, 8,
			      XmNorientation, XmHORIZONTAL,
			      XmNisAligned, True,
			      XmNadjustLast, False,
			      XmNentryAlignment, XmALIGNMENT_END,
			      NULL);

	gui.order_item = CreatePanelChoice(dialog,
					   "Apply in order:",
					   7,
					   "Rotate, translate, scale",
					   "Rotate, scale, translate",
					   "Translate, scale, rotate",
					   "Translate, rotate, scale",
					   "Scale, translate, rotate",
					   "Scale, rotate, translate",
					   0,
					   0);

	gui.degrees_item = CreateTextItem4(rc, 10, "Rotation (degrees):");
	gui.rotx_item = CreateTextItem4(rc, 10, "Rotate about X = :");
	gui.roty_item = CreateTextItem4(rc, 10, "Rotate about Y = :");
	gui.scalex_item = CreateTextItem4(rc, 10, "Scale X:");
	gui.scaley_item = CreateTextItem4(rc, 10, "Scale Y:");
	gui.transx_item = CreateTextItem4(rc, 10, "Translate X:");
	gui.transy_item = CreateTextItem4(rc, 10, "Translate Y:");
	XtManageChild(rc);

	CreateSeparator(dialog);

	CreateCommandButtons(dialog, 3, but1, label1);
	XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc) do_geom_proc, (XtPointer) & gui);
	XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc) reset_geom_proc, (XtPointer)
	& gui.top);
	XtAddCallback(but1[2], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) gui.top);

	XtManageChild(dialog);
	xv_setstr(gui.degrees_item, "0.0");
	xv_setstr(gui.rotx_item, "0.0");
	xv_setstr(gui.roty_item, "0.0");
	xv_setstr(gui.scalex_item, "1.0");
	xv_setstr(gui.scaley_item, "1.0");
	xv_setstr(gui.transx_item, "0.0");
	xv_setstr(gui.transy_item, "0.0");
    }
    XtRaise(gui.top);
    unset_wait_cursor();
}

/*
 * compute geom
 */
static void do_geom_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int i, j, k, cnt, order[3], setno, ord;
    int *selsets;
    double degrees, sx, sy, rotx, roty, tx, ty, xtmp, ytmp, *x, *y;
    double cosd, sind;
    Geom_ui *ui = (Geom_ui *) client_data;
    
    if (w == NULL) {
	cnt = 1;
	selsets = (int *) malloc(sizeof(int));
	selsets[0] = pick_set;
    } else {
	cnt = GetSelectedSets(ui->sel, &selsets);
	if (cnt == SET_SELECT_ERROR) {
	    errwin("No sets selected");
	    return;
	}
    }
    ord = (int) GetChoice(ui->order_item);
    switch (ord) {
    case 0:
	order[0] = 0;		/* rotate */
	order[1] = 1;		/* translate */
	order[2] = 2;		/* scale */
	break;
    case 1:
	order[0] = 0;
	order[1] = 2;
	order[2] = 1;
    case 2:
	order[0] = 1;
	order[1] = 2;
	order[2] = 0;
	break;
    case 3:
	order[0] = 1;
	order[1] = 0;
	order[2] = 2;
	break;
    case 4:
	order[0] = 2;
	order[1] = 1;
	order[2] = 0;
	break;
    case 5:
	order[0] = 2;
	order[1] = 0;
	order[2] = 1;
	break;
    }
	/* check input fields */
    if (xv_evalexpr(ui->degrees_item, &degrees) != GRACE_EXIT_SUCCESS ||
        xv_evalexpr(ui->rotx_item, &rotx)       != GRACE_EXIT_SUCCESS ||
        xv_evalexpr(ui->roty_item, &roty)       != GRACE_EXIT_SUCCESS ||
        xv_evalexpr(ui->transx_item, &tx)       != GRACE_EXIT_SUCCESS ||
        xv_evalexpr(ui->transy_item, &ty)       != GRACE_EXIT_SUCCESS ||
        xv_evalexpr(ui->scalex_item, &sx)       != GRACE_EXIT_SUCCESS ||
        xv_evalexpr(ui->scaley_item, &sy)       != GRACE_EXIT_SUCCESS )
		return;
   	
	degrees = M_PI / 180.0 * degrees;
	cosd = cos(degrees);
	sind = sin(degrees);
	
    set_wait_cursor();
    for (k = 0; k < cnt; k++) {
	setno = selsets[k];
	if (is_set_active(get_cg(), setno)) {
	    x = getx(get_cg(), setno);
	    y = gety(get_cg(), setno);
	    for (j = 0; j < 3; j++) {
		switch (order[j]) {
		case 0:			/* rotate */
		    if (degrees == 0.0) {
				break;
		    }
		    for (i = 0; i < getsetlength(get_cg(), setno); i++) {
				xtmp = x[i] - rotx;
				ytmp = y[i] - roty;
				x[i] = rotx + cosd * xtmp - sind * ytmp;
				y[i] = roty + sind * xtmp + cosd * ytmp;
		    }
		    break;
		case 1:			/* translate */
		    for (i = 0; i < getsetlength(get_cg(), setno); i++) {
			x[i] += tx;
			y[i] += ty;
		    }
		    break;
		case 2:					/* scale */
		    for (i = 0; i < getsetlength(get_cg(), setno); i++) {
				x[i] *= sx;
				y[i] *= sy;
		    }
		    break;
		}		/* end case */
	    }			/* end for j */

	    update_set_lists(get_cg());
	}			/* end if */
    }				/* end for k */
    update_set_lists(get_cg());
    free(selsets);
    set_dirtystate();
    unset_wait_cursor();
    drawgraph();
}

static void reset_geom_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    Geom_ui *tui = (Geom_ui *) client_data;
	xv_setstr(tui->degrees_item, "0.0");
	xv_setstr(tui->rotx_item, "0.0");
	xv_setstr(tui->roty_item, "0.0");
	xv_setstr(tui->scalex_item, "1.0");
	xv_setstr(tui->scaley_item, "1.0");
	xv_setstr(tui->transx_item, "0.0");
	xv_setstr(tui->transy_item, "0.0");
}
