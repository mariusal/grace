/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2000 Grace Development Team
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
#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/RowColumn.h>

#include "mbitmaps.h"

#include "globals.h"
#include "graphutils.h"
#include "plotone.h"
#include "device.h"
#include "utils.h"
#include "motifinc.h"
#include "protos.h"


static Widget overlay_frame;
static Widget overlay_panel;

/*
 * Panel item declarations
 */

static ListStructure *graph_overlay1_choice_item;
static ListStructure *graph_overlay2_choice_item;
static OptionStructure *graph_overlaytype_item;

static Widget but1[2];

static int define_arrange_proc(void *data);
static void define_overlay_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void define_autos_proc(Widget w, XtPointer client_data, XtPointer call_data);


typedef struct {
    int ncols;
    int nrows;
} GridData;

Widget CreateGrid(Widget parent, int ncols, int nrows)
{
    Widget w;
    int nfractions;
    GridData *gd;
    
    if (ncols <= 0 || nrows <= 0) {
        errmsg("Wrong call to CreateGrid()");
        ncols = 1;
        nrows = 1;
    }
    
    nfractions = 0;
    do {
        nfractions++;
    } while (nfractions % ncols || nfractions % nrows);
    
    gd = xmalloc(sizeof(GridData));
    gd->ncols = ncols;
    gd->nrows = nrows;
    
    w = XmCreateForm(parent, "grid_form", NULL, 0);
    XtVaSetValues(w,
        XmNfractionBase, nfractions,
        XmNuserData, gd,
        NULL);

    XtManageChild(w);
    return w;
}

void PlaceGridChild(Widget grid, Widget w, int col, int row)
{
    int nfractions, w1, h1;
    GridData *gd;
    
    XtVaGetValues(grid,
        XmNfractionBase, &nfractions,
        XmNuserData, &gd,
        NULL);
    
    if (gd == NULL) {
        errmsg("PlaceGridChild() called with a non-grid widget");
        return;
    }
    if (col < 0 || col >= gd->ncols) {
        errmsg("PlaceGridChild() called with wrong `col' argument");
        return;
    }
    if (row < 0 || row >= gd->nrows) {
        errmsg("PlaceGridChild() called with wrong `row' argument");
        return;
    }
    
    w1 = nfractions/gd->ncols;
    h1 = nfractions/gd->nrows;
    
    XtVaSetValues(w,
        XmNleftAttachment  , XmATTACH_POSITION,
        XmNleftPosition    , col*w1           ,
        XmNrightAttachment , XmATTACH_POSITION,
        XmNrightPosition   , (col + 1)*w1     ,
        XmNtopAttachment   , XmATTACH_POSITION,
        XmNtopPosition     , row*h1           ,
        XmNbottomAttachment, XmATTACH_POSITION,
        XmNbottomPosition  , (row + 1)*h1     ,
        NULL);
}


typedef struct _Arrange_ui {
    Widget top;
    ListStructure *graphs;
    SpinStructure *nrows;
    SpinStructure *ncols;
    OptionStructure *order;
    SpinStructure *toff;
    SpinStructure *loff;
    SpinStructure *roff;
    SpinStructure *boff;
    SpinStructure *hgap;
    SpinStructure *vgap;
    Widget hpack;
    Widget vpack;
    Widget add;
    Widget kill;
} Arrange_ui;


/*
 * Arrange graphs popup routines
 */
static int define_arrange_proc(void *data)
{
    Arrange_ui *ui = (Arrange_ui *) data;
    int ngraphs, *graphs;
    int nrows, ncols, order;
    int hpack, vpack, add, kill;
    double toff, loff, roff, boff, vgap, hgap;

    nrows = (int) GetSpinChoice(ui->nrows);
    ncols = (int) GetSpinChoice(ui->ncols);
    if (nrows < 1 || ncols < 1) {
	errmsg("# of rows and columns must be > 0");
	return RETURN_FAILURE;
    }
    
    ngraphs = GetListChoices(ui->graphs, &graphs);
    if (ngraphs == 0) {
        graphs = NULL;
    }
    
    order = GetOptionChoice(ui->order);
    
    toff = GetSpinChoice(ui->toff);
    loff = GetSpinChoice(ui->loff);
    roff = GetSpinChoice(ui->roff);
    boff = GetSpinChoice(ui->boff);

    hgap = GetSpinChoice(ui->hgap);
    vgap = GetSpinChoice(ui->vgap);
    
    add  = GetToggleButtonState(ui->add);
    kill = GetToggleButtonState(ui->kill);
    
    hpack = GetToggleButtonState(ui->hpack);
    vpack = GetToggleButtonState(ui->vpack);

    if (add && ngraphs < nrows*ncols) {
        int gno;
        graphs = xrealloc(graphs, nrows*ncols*SIZEOF_INT);
        for (gno = number_of_graphs(); ngraphs < nrows*ncols; ngraphs++, gno++) {
            graphs[ngraphs] = gno;
        }
    }
    
    if (kill && ngraphs > nrows*ncols) {
        for (; ngraphs > nrows*ncols; ngraphs--) {
            kill_graph(graphs[ngraphs - 1]);
        }
    }
    
    arrange_graphs(graphs, ngraphs,
        nrows, ncols, order,
        loff, roff, toff, boff, vgap, hgap,
        hpack, vpack);
    
    update_all();
    
    SelectListChoices(ui->graphs, ngraphs, graphs);
    xfree(graphs);
    
    xdrawgraph();
    
    return RETURN_SUCCESS;
}

void hpack_cb(int onoff, void *data)
{
    Arrange_ui *ui = (Arrange_ui *) data;
    SetSensitive(ui->hgap->rc, !onoff);
}
void vpack_cb(int onoff, void *data)
{
    Arrange_ui *ui = (Arrange_ui *) data;
    SetSensitive(ui->vgap->rc, !onoff);
}

void create_arrange_frame(void *data)
{
    static Arrange_ui *ui = NULL;
    set_wait_cursor();

    if (ui == NULL) {
        Widget arrange_panel, fr, gr, rc;
        BitmapOptionItem opitems[8] = {
            {0               | 0              | 0             , m_hv_lr_tb_bits},
            {0               | 0              | GA_ORDER_V_INV, m_hv_lr_bt_bits},
            {0               | GA_ORDER_H_INV | 0             , m_hv_rl_tb_bits},
            {0               | GA_ORDER_H_INV | GA_ORDER_V_INV, m_hv_rl_bt_bits},
            {GA_ORDER_HV_INV | 0              | 0             , m_vh_lr_tb_bits},
            {GA_ORDER_HV_INV | 0              | GA_ORDER_V_INV, m_vh_lr_bt_bits},
            {GA_ORDER_HV_INV | GA_ORDER_H_INV | 0             , m_vh_rl_tb_bits},
            {GA_ORDER_HV_INV | GA_ORDER_H_INV | GA_ORDER_V_INV, m_vh_rl_bt_bits}
        };
        
        ui = xmalloc(sizeof(Arrange_ui));
    
	ui->top = CreateDialogForm(app_shell, "Arrange graphs");

	arrange_panel = CreateVContainer(ui->top);
        
	fr = CreateFrame(arrange_panel, NULL);
        rc = CreateVContainer(fr);
        ui->graphs = CreateGraphChoice(rc,
            "Arrange graphs:", LIST_TYPE_MULTIPLE);
        ui->add = CreateToggleButton(rc,
            "Add graphs as needed to fill the matrix");
        ui->kill = CreateToggleButton(rc, "Kill extra graphs");

        fr = CreateFrame(arrange_panel, "Matrix");
        gr = CreateGrid(fr, 3, 1);
        ui->ncols = CreateSpinChoice(gr,
            "Cols:", 2, SPIN_TYPE_INT, (double) 1, (double) 99, (double) 1);
        PlaceGridChild(gr, ui->ncols->rc, 0, 0);
        ui->nrows = CreateSpinChoice(gr,
            "Rows:", 2, SPIN_TYPE_INT, (double) 1, (double) 99, (double) 1);
        PlaceGridChild(gr, ui->nrows->rc, 1, 0);
        ui->order = CreateBitmapOptionChoice(gr,
            "Order:", 2, 8, MBITMAP_WIDTH, MBITMAP_HEIGHT, opitems);
        PlaceGridChild(gr, ui->order->menu, 2, 0);

	fr = CreateFrame(arrange_panel, "Page offsets");
        gr = CreateGrid(fr, 3, 3);
        ui->toff = CreateSpinChoice(gr, "", 4, SPIN_TYPE_FLOAT, 0.0, 1.0, 0.05);
        PlaceGridChild(gr, ui->toff->rc, 1, 0);
        ui->loff = CreateSpinChoice(gr, "", 4, SPIN_TYPE_FLOAT, 0.0, 1.0, 0.05);
        PlaceGridChild(gr, ui->loff->rc, 0, 1);
        ui->roff = CreateSpinChoice(gr, "", 4, SPIN_TYPE_FLOAT, 0.0, 1.0, 0.05);
        PlaceGridChild(gr, ui->roff->rc, 2, 1);
        ui->boff = CreateSpinChoice(gr, "", 4, SPIN_TYPE_FLOAT, 0.0, 1.0, 0.05);
        PlaceGridChild(gr, ui->boff->rc, 1, 2);

	fr = CreateFrame(arrange_panel, "Spacing");
        gr = CreateGrid(fr, 2, 1);
        rc = CreateHContainer(gr);
        ui->hgap = CreateSpinChoice(rc,
            "Hgap/width", 3, SPIN_TYPE_FLOAT, 0.0, 9.0, 0.1);
        ui->hpack = CreateToggleButton(rc, "Pack");
        AddToggleButtonCB(ui->hpack, hpack_cb, ui);
        PlaceGridChild(gr, rc, 0, 0);
        rc = CreateHContainer(gr);
        ui->vgap = CreateSpinChoice(rc,
            "Vgap/height", 3, SPIN_TYPE_FLOAT, 0.0, 9.0, 0.1);
        ui->vpack = CreateToggleButton(rc, "Pack");
        AddToggleButtonCB(ui->vpack, vpack_cb, ui);
        PlaceGridChild(gr, rc, 1, 0);
        
	CreateAACDialog(ui->top, arrange_panel, define_arrange_proc, ui);
        
        SetSpinChoice(ui->nrows, (double) 1);
        SetSpinChoice(ui->ncols, (double) 1);
        
        SetSpinChoice(ui->toff, GA_OFFSET_DEFAULT);
        SetSpinChoice(ui->loff, GA_OFFSET_DEFAULT);
        SetSpinChoice(ui->roff, GA_OFFSET_DEFAULT);
        SetSpinChoice(ui->boff, GA_OFFSET_DEFAULT);

        SetSpinChoice(ui->hgap, GA_GAP_DEFAULT);
        SetSpinChoice(ui->vgap, GA_GAP_DEFAULT);
        
        SetToggleButtonState(ui->add, TRUE);
    }

    RaiseWindow(GetParent(ui->top));
    
    unset_wait_cursor();
}

/*
 * Overlay graphs popup routines
 */
static void define_overlay_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int g1, g2;
    int type = GetOptionChoice(graph_overlaytype_item);
    
    if (GetSingleListChoice(graph_overlay1_choice_item, &g1) != RETURN_SUCCESS) {
	errmsg("Please select a single graph");
	return;
    }
    
    if (GetSingleListChoice(graph_overlay2_choice_item, &g2) != RETURN_SUCCESS) {
	errmsg("Please select a single graph");
	return;
    }

    if (g1 == g2) {
	errmsg("Can't overlay a graph onto itself");
	return;
    }

    overlay_graphs(g1, g2, type);

    update_all();
    drawgraph();
}

void create_overlay_frame(void *data)
{
    char *label1[2];
    
    set_wait_cursor();
    if (overlay_frame == NULL) {
        OptionItem opitems[5];
	label1[0] = "Accept";
	label1[1] = "Close";
        
	overlay_frame = XmCreateDialogShell(app_shell, "Overlay graphs", NULL, 0);
	handle_close(overlay_frame);
	overlay_panel = XmCreateRowColumn(overlay_frame, "overlay_rc", NULL, 0);
	graph_overlay1_choice_item = CreateGraphChoice(overlay_panel,
            "Overlay graph:", LIST_TYPE_SINGLE);
	graph_overlay2_choice_item = CreateGraphChoice(overlay_panel,
            "Onto graph:", LIST_TYPE_SINGLE);
	
        opitems[0].value = GOVERLAY_SMART_AXES_DISABLED;
        opitems[0].label = "Disabled";
        opitems[1].value = GOVERLAY_SMART_AXES_NONE;
        opitems[1].label = "X and Y axes different";
        opitems[2].value = GOVERLAY_SMART_AXES_X;
        opitems[2].label = "Same X axis scaling";
        opitems[3].value = GOVERLAY_SMART_AXES_Y;
        opitems[3].label = "Same Y axis scaling";
        opitems[4].value = GOVERLAY_SMART_AXES_XY;
        opitems[4].label = "Same X and Y axis scaling";
        graph_overlaytype_item = CreateOptionChoice(overlay_panel,
            "Smart axis hints:", 0, 5, opitems);

	CreateSeparator(overlay_panel);

	CreateCommandButtons(overlay_panel, 2, but1, label1);
	XtAddCallback(but1[0], XmNactivateCallback, define_overlay_proc, NULL);
	XtAddCallback(but1[1], XmNactivateCallback, destroy_dialog, (XtPointer) overlay_frame);

	ManageChild(overlay_panel);
    }

    RaiseWindow(overlay_frame);
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

void create_autos_frame(void *data)
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

	CreateSeparator(panel);

	CreateCommandButtons(panel, 2, but1, label1);
	XtAddCallback(but1[0], XmNactivateCallback, 
	              define_autos_proc, (XtPointer) &aui);
	XtAddCallback(but1[1], XmNactivateCallback,
	              destroy_dialog, (XtPointer) aui.top);

	ManageChild(panel);
    }
    RaiseWindow(aui.top);
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
    update_ticks(cg);
    drawgraph();
}
