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
 * Contents:
 *     arrange graphs popup
 *     overlay graphs popup
 *     autoscaling popup
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
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#include <Xm/Text.h>

#include "globals.h"
#include "graphs.h"
#include "graphutils.h"
#include "plotone.h"
#include "device.h"
#include "utils.h"
#include "motifinc.h"
#include "protos.h"


static Widget arrange_frame;
static Widget arrange_panel;

static Widget overlay_frame;
static Widget overlay_panel;

/*
 * Panel item declarations
 */

static Widget *arrange_rows_item;
static Widget *arrange_cols_item;
static Widget arrange_vgap_item;
static Widget arrange_hgap_item;
static Widget arrange_startx_item;
static Widget arrange_starty_item;
static Widget arrange_widthx_item;
static Widget arrange_widthy_item;
static Widget *arrange_packed_item;

static ListStructure *graph_overlay1_choice_item;
static ListStructure *graph_overlay2_choice_item;
static Widget *graph_overlaytype_item;

static Widget but1[2];

int maxmajorticks = 500;
int maxminorticks = 1000;

static void define_arrange_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void define_overlay_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void define_autos_proc(Widget w, XtPointer client_data, XtPointer call_data);


/*
 * Arrange graphs popup routines
 */
static void define_arrange_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int nrows, ncols, pack;
    double vgap, hgap, sx, sy, wx, wy;

    nrows = GetChoice(arrange_rows_item) + 1;
    ncols = GetChoice(arrange_cols_item) + 1;
    if (nrows < 1 || ncols < 1) {
	return;
    }

    pack = GetChoice(arrange_packed_item);
	xv_evalexpr(arrange_vgap_item, &vgap);
	xv_evalexpr(arrange_hgap_item, &hgap);
	xv_evalexpr(arrange_startx_item, &sx);
	xv_evalexpr(arrange_starty_item, &sy);
	xv_evalexpr(arrange_widthx_item, &wx);
	xv_evalexpr(arrange_widthy_item, &wy);
    if (wx <= 0.0) {
	errwin("Graph width must be > 0.0");
	return;
    }
    if (wy <= 0.0) {
	errwin("Graph height must be > 0.0");
	return;
    }
    define_arrange(nrows, ncols, pack, vgap, hgap, sx, sy, wx, wy);
    
    drawgraph();
}

void row_arrange_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
    int nrow = (int)client_data,pack;
    double height, vgap, starty;
    double vx, vy;
    char buf[32];

    get_page_viewport(&vx, &vy);
    xv_evalexpr(arrange_starty_item, &starty);
    if ( (pack = GetChoice(arrange_packed_item)) == 2 || pack == 3 ) {
        vgap = 0.0;
        xv_setstr(arrange_vgap_item, "0.0");
    } else {
        xv_evalexpr(arrange_vgap_item, &vgap);
    }
    height = (vy - 2*starty - (nrow-1)*vgap)/nrow;
    sprintf( buf, "%g", height );
    xv_setstr(arrange_widthy_item, buf );
}

void col_arrange_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
    int ncol = (int)client_data, pack;
    double width, hgap, startx;
    double vx, vy;
    char buf[32];

    get_page_viewport(&vx, &vy);
    xv_evalexpr(arrange_startx_item, &startx);
    if ( (pack = GetChoice(arrange_packed_item)) == 1 || pack == 3 ) {
        hgap = 0.0;
        xv_setstr(arrange_hgap_item, "0.0");
    } else {
        xv_evalexpr(arrange_hgap_item, &hgap);
    }
    width = (vx - 2*startx - (ncol-1)*hgap)/ncol;
    sprintf( buf, "%g", width );
    xv_setstr(arrange_widthx_item, buf );
}


void create_arrange_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget rc;
    int i;
    
    set_wait_cursor();
    if (arrange_frame == NULL) {
	char *label1[2];
	label1[0] = "Accept";
	label1[1] = "Close";
	arrange_frame = XmCreateDialogShell(app_shell, "Arrange graphs", NULL, 0);
	handle_close(arrange_frame);
	arrange_panel = XmCreateRowColumn(arrange_frame, "arrange_rc", NULL, 0);

	rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, arrange_panel,
			      XmNpacking, XmPACK_COLUMN,
			      XmNnumColumns, 9,	/* nitems / 2 */
			      XmNorientation, XmHORIZONTAL,
			      XmNisAligned, True,
			      XmNadjustLast, False,
			      XmNentryAlignment, XmALIGNMENT_END,
			      NULL);

	XtVaCreateManagedWidget("Rows: ", xmLabelWidgetClass, rc, NULL);
	arrange_rows_item = CreatePanelChoice(rc, " ",
					      11,
			  "1", "2", "3", "4", "5", "6", "7", "8", "9", "10",
					      NULL, NULL);
	for( i=2; i<12; i++ )
	XtAddCallback(arrange_rows_item[i], XmNactivateCallback, 
			(XtCallbackProc) row_arrange_cb, (XtPointer) (i-1));
			
	XtVaCreateManagedWidget("Columns: ", xmLabelWidgetClass, rc, NULL);
	arrange_cols_item = CreatePanelChoice(rc, " ",
					      11,
			  "1", "2", "3", "4", "5", "6", "7", "8", "9", "10",
					      NULL, NULL);
	for( i=2; i<12; i++ )
	XtAddCallback(arrange_cols_item[i], XmNactivateCallback, 
			(XtCallbackProc) col_arrange_cb, (XtPointer) (i-1));

	XtVaCreateManagedWidget("Packing: ", xmLabelWidgetClass, rc, NULL);
	arrange_packed_item = CreatePanelChoice(rc, " ",
						5,
                                                "None",
                                                "Horizontal",
                                                "Vertical",
                                                "Both",
						NULL, NULL);

	arrange_vgap_item = CreateTextItem4(rc, 10, "Vertical gap:");
	arrange_hgap_item = CreateTextItem4(rc, 10, "Horizontal gap:");
	arrange_startx_item = CreateTextItem4(rc, 10, "Start at X =");
	arrange_starty_item = CreateTextItem4(rc, 10, "Start at Y =");
	arrange_widthx_item = CreateTextItem4(rc, 10, "Graph width:");
	arrange_widthy_item = CreateTextItem4(rc, 10, "Graph height:");

        xv_setstr(arrange_vgap_item, "0.07");
        xv_setstr(arrange_hgap_item, "0.07");
        xv_setstr(arrange_startx_item, "0.1");
        xv_setstr(arrange_starty_item, "0.1");
        xv_setstr(arrange_widthx_item, "0.8");
        xv_setstr(arrange_widthy_item, "0.8");
        
	XtManageChild(rc);

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, arrange_panel, NULL);

	CreateCommandButtons(arrange_panel, 2, but1, label1);
	XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc) define_arrange_proc, (XtPointer) NULL);
	XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) arrange_frame);

	XtManageChild(arrange_panel);
    }
/*
 *     update_arrange();
 */
    XtRaise(arrange_frame);
    unset_wait_cursor();
}

/*
 * Overlay graphs popup routines
 */
static void define_overlay_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int n, *values;
    int g1, g2;
    int type = GetChoice(graph_overlaytype_item);
    
    n = GetListChoices(graph_overlay1_choice_item, &values);
    if (n != 1) {
	errmsg("Please select a single graph");
	return;
    }
    g1 = values[0];
    free(values);
    
    n = GetListChoices(graph_overlay2_choice_item, &values);
    if (n != 1) {
	errmsg("Please select a single graph");
	return;
    }
    g2 = values[0];
    free(values);

    if (g1 == g2) {
	errmsg("Can't overlay a graph onto itself");
	return;
    }

    overlay_graphs(g1, g2, type);

    set_dirtystate();

    update_all();
    drawgraph();
}

void create_overlay_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    char *label1[2];
    
    set_wait_cursor();
    if (overlay_frame == NULL) {
	label1[0] = "Accept";
	label1[1] = "Close";
	overlay_frame = XmCreateDialogShell(app_shell, "Overlay graphs", NULL, 0);
	handle_close(overlay_frame);
	overlay_panel = XmCreateRowColumn(overlay_frame, "overlay_rc", NULL, 0);
	graph_overlay1_choice_item = CreateGraphChoice(overlay_panel,
            "Overlay graph:", LIST_TYPE_SINGLE);
	graph_overlay2_choice_item = CreateGraphChoice(overlay_panel,
            "Onto graph:", LIST_TYPE_SINGLE);
	graph_overlaytype_item = CreatePanelChoice(overlay_panel,
            "Overlay type:",
	    5,
	    "Same axes scaling along X and Y",
	    "X-axes same, Y-axes different:",
	    "Y-axes same, X-axes different:",
	    "X and Y axes different:",
	    NULL, NULL);

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, overlay_panel, NULL);

	CreateCommandButtons(overlay_panel, 2, but1, label1);
	XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc) define_overlay_proc, (XtPointer) NULL);
	XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) overlay_frame);

	XtManageChild(overlay_panel);
    }

    XtRaise(overlay_frame);
    unset_wait_cursor();
}

/*
 * autoscale popup
 */
typedef struct _Auto_ui {
    Widget top;
    SetChoiceItem sel;
    Widget *on_item;
    Widget *applyto_item;
} Auto_ui;

static Auto_ui aui;

static void define_autos_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int aon, au, ap;
    Auto_ui *ui = (Auto_ui *) client_data;
    aon = GetChoice(ui->on_item);
    au = GetSelectedSet(ui->sel);
    if (au == SET_SELECT_ERROR) {
        errwin("No set selected");
        return;
    }
    if (au == SET_SELECT_ALL) {
	au = -1;
    } 
    ap = GetChoice(ui->applyto_item);
    define_autos(aon, au, ap);
}

void create_autos_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget panel;

    set_wait_cursor();
    if (aui.top == NULL) {
	char *label1[2];
	label1[0] = "Accept";
	label1[1] = "Close";
	aui.top = XmCreateDialogShell(app_shell, "Autoscale graphs", NULL, 0);
	handle_close(aui.top);
	panel = XmCreateRowColumn(aui.top, "autos_rc", NULL, 0);

	aui.on_item = CreatePanelChoice(panel, "Autoscale:",
					 5,
				  	 "None",
				  	 "All X-axes",
				  	 "All Y-axes",
				  	 "All axes",
				  	 NULL,
				  	 NULL);

	aui.applyto_item = CreatePanelChoice(panel, "Apply to:",
					       3,
					       "Current graph",
					       "All active graphs",
					       NULL,
					       NULL);

	aui.sel = CreateSetSelector(panel, "Use set:",
                                    SET_SELECT_ALL,
                                    FILTER_SELECT_NONE,
                                    GRAPH_SELECT_CURRENT,
                                    SELECTION_TYPE_MULTIPLE);

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, panel, NULL);

	CreateCommandButtons(panel, 2, but1, label1);
	XtAddCallback(but1[0], XmNactivateCallback, 
	              (XtCallbackProc) define_autos_proc, (XtPointer) &aui);
	XtAddCallback(but1[1], XmNactivateCallback,
	              (XtCallbackProc) destroy_dialog, (XtPointer) aui.top);

	XtManageChild(panel);
    }
    XtRaise(aui.top);
    unset_wait_cursor();
}

void define_autos(int aon, int au, int ap)
{
    int i, ming, maxg;
    int cg = get_cg();

    if (au >= 0 && !is_set_active(cg, au)) {
	errmsg("Set not active");
	return;
    }
    if (ap) {
	ming = 0;
	maxg = number_of_graphs() - 1;
    } else {
	ming = cg;
	maxg = cg;
    }
    if (ming == cg && maxg == cg) {
	if (!is_graph_active(cg)) {
	    errmsg("Current graph is not active!");
	    return;
	}
    }
    for (i = ming; i <= maxg; i++) {
	if (is_graph_active(i)) {
	    autoscale_byset(i, au, aon);
	}
    }
    drawgraph();
}
