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
 * Edit block data Panel
 *
 *
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <Xm/Xm.h>
#include <Xm/DialogS.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#include <Xm/Text.h>

#include "globals.h"
#include "graphs.h"
#include "plotone.h"
#include "utils.h"
#include "motifinc.h"
#include "protos.h"

Widget *CreateBlockChoice(Widget parent, char *labelstr, int ncols, int type);

static char ncolsbuf[128];

static int block_curtype = SET_XY;

static Widget eblock_frame;
static Widget eblock_panel;

/*
 * Panel item declarations
 */
static Widget eblock_ncols_item;
static Widget *eblock_type_choice_item;
static Widget *eblock_x_choice_item;
static Widget *eblock_y_choice_item;
static Widget *eblock_e1_choice_item;
static Widget *eblock_e2_choice_item;
static Widget *eblock_e3_choice_item;
static Widget *eblock_e4_choice_item;
static Widget *eblock_graph_choice_item;

/*
 * Event and Notify proc declarations
 */
static void eblock_type_notify_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void eblock_accept_notify_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void update_eblock(void);

/*
 * Create the files Frame and the files Panel
 */
void create_eblock_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    int i;
    Widget rc, buts[2];

    if (blockncols == 0) {
	errwin("Need to read block data first");
	return;
    }
    set_wait_cursor();
    if (eblock_frame == NULL) {
	char *label1[2];
	label1[0] = "Accept";
	label1[1] = "Close";
	eblock_frame = XmCreateDialogShell(app_shell, "Edit block data", NULL, 0);
	handle_close(eblock_frame);
	eblock_panel = XmCreateRowColumn(eblock_frame, "eblock_rc", NULL, 0);

	eblock_ncols_item = XtVaCreateManagedWidget("tmp", xmLabelWidgetClass, eblock_panel,
						    NULL);

        rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, eblock_panel,
                              XmNpacking, XmPACK_COLUMN,
                              XmNnumColumns, 9,
                              XmNorientation, XmHORIZONTAL,
                              XmNisAligned, True,
                              XmNadjustLast, False,
                              XmNentryAlignment, XmALIGNMENT_END,
                              NULL);

	XtVaCreateManagedWidget("Set type: ", xmLabelWidgetClass, rc, NULL);
	eblock_type_choice_item = CreatePanelChoice(rc,
						    " ",
						    12,
						    "XY",
						    "BAR",
						    "XY DX",
						    "XY DY",
						    "XY DX1 DX2",
						    "XY DY1 DY2",
						    "XY DX DY",
						    "XY Z",
						    "XY HILO",
						    "XY R",
						    "POLY",
						    NULL, 0);
	for (i = 0; i < 11; i++) {
	    XtAddCallback(eblock_type_choice_item[2 + i],
			  XmNactivateCallback, (XtCallbackProc) eblock_type_notify_proc, (XtPointer) i);
	}

	XtVaCreateManagedWidget("X from column:", xmLabelWidgetClass, rc, NULL);
	eblock_x_choice_item = CreateBlockChoice(rc, " ", maxblock, 1);
	XtVaCreateManagedWidget("Y from column:", xmLabelWidgetClass, rc, NULL);
	eblock_y_choice_item = CreateBlockChoice(rc, " ", maxblock, 0);
	XtVaCreateManagedWidget("E1 from column:", xmLabelWidgetClass, rc, NULL);
	eblock_e1_choice_item = CreateBlockChoice(rc, " ", maxblock, 0);
	XtVaCreateManagedWidget("E2 from column:", xmLabelWidgetClass, rc, NULL);
	eblock_e2_choice_item = CreateBlockChoice(rc, " ", maxblock, 0);
	XtVaCreateManagedWidget("E3 from column:", xmLabelWidgetClass, rc, NULL);
	eblock_e3_choice_item = CreateBlockChoice(rc, " ", maxblock, 0);
	XtVaCreateManagedWidget("E4 from column:", xmLabelWidgetClass, rc, NULL);
	eblock_e4_choice_item = CreateBlockChoice(rc, " ", maxblock, 0);

	XtVaCreateManagedWidget("Load to set in graph:", xmLabelWidgetClass, rc, NULL);
	eblock_graph_choice_item = CreateGraphChoice(rc, " ", number_of_graphs() , 1);

	XtManageChild(rc);

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, eblock_panel, NULL);

	CreateCommandButtons(eblock_panel, 2, buts, label1);
	XtAddCallback(buts[0], XmNactivateCallback,
		 (XtCallbackProc) eblock_accept_notify_proc, (XtPointer) 0);
	XtAddCallback(buts[1], XmNactivateCallback,
		 (XtCallbackProc) destroy_dialog, (XtPointer) eblock_frame);

	XtManageChild(eblock_panel);
    }
    XtRaise(eblock_frame);
    update_eblock();
    unset_wait_cursor();
}				/* end create_eblock_panel */

/*
 * Notify and event procs
 */

static void update_eblock(void)
{
    XmString string;
    Arg al;
    if (!eblock_frame) {
	return;
    }
    if (blockncols == 0) {
	errwin("Need to read block data first");
	return;
    }
    sprintf(ncolsbuf, "%d columns of length %d", blockncols, blocklen);
    string = XmStringCreateLtoR(ncolsbuf, charset);
    XtSetArg(al, XmNlabelString, string);
    XtSetValues(eblock_ncols_item, &al, 1);
    XmStringFree(string);
    switch (block_curtype) {
    case SET_XY:
    case SET_BAR:
	XtSetSensitive(eblock_e1_choice_item[0], False);
	XtSetSensitive(eblock_e2_choice_item[0], False);
	XtSetSensitive(eblock_e3_choice_item[0], False);
	XtSetSensitive(eblock_e4_choice_item[0], False);
	break;
    case SET_XYR:
    case SET_XYDX:
    case SET_XYDY:
    case SET_XYZ:
    case SET_BARDY:
	XtSetSensitive(eblock_e1_choice_item[0], True);
	XtSetSensitive(eblock_e2_choice_item[0], False);
	XtSetSensitive(eblock_e3_choice_item[0], False);
	XtSetSensitive(eblock_e4_choice_item[0], False);
	break;
    case SET_XYDXDX:
    case SET_XYDYDY:
    case SET_XYDXDY:
    case SET_BARDYDY:
	XtSetSensitive(eblock_e1_choice_item[0], True);
	XtSetSensitive(eblock_e2_choice_item[0], True);
	XtSetSensitive(eblock_e3_choice_item[0], False);
	XtSetSensitive(eblock_e4_choice_item[0], False);
	break;
    case SET_XYHILO:
	XtSetSensitive(eblock_e1_choice_item[0], True);
	XtSetSensitive(eblock_e2_choice_item[0], True);
	XtSetSensitive(eblock_e3_choice_item[0], True);
	XtSetSensitive(eblock_e4_choice_item[0], False);
	break;
    }
}

static void eblock_type_notify_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int cd = (int) client_data;

    block_curtype = cd;

    update_eblock();
}

static void eblock_accept_notify_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int i = 0;
    char buf[256];
    int setno, graphno;
    int d1, cx, cy, c1 = 0, c2 = 0, c3 = 0;
    double *tx, *ty, *t2, *t3, *t4;

    d1 = (int) GetChoice(eblock_type_choice_item);
    cx = (int) GetChoice(eblock_x_choice_item) - 1;
    cy = (int) GetChoice(eblock_y_choice_item);
    if (cx >= 0 && cx >= blockncols) {
	errwin("Column for X exceeds the number of columns in block data");
	return;
    }
    if (cy >= blockncols) {
	errwin("Column for Y exceeds the number of columns in block data");
	return;
    }
    switch (block_curtype) {
    case SET_XY:
    case SET_BAR:
	break;
    case SET_XYR:
    case SET_XYDX:
    case SET_XYDY:
    case SET_XYZ:
    case SET_BARDY:
	c1 = (int) GetChoice(eblock_e1_choice_item);
	if (c1 >= blockncols) {
	    errwin("Column for E1 exceeds the number of columns in block data");
	    return;
	}
	break;
    case SET_XYDXDX:
    case SET_XYDYDY:
    case SET_XYDXDY:
    case SET_BARDYDY:
	c1 = (int) GetChoice(eblock_e1_choice_item);
	c2 = (int) GetChoice(eblock_e2_choice_item);
	if (c1 >= blockncols) {
	    errwin("Column for E1 exceeds the number of columns in block data");
	    return;
	}
	if (c2 >= blockncols) {
	    errwin("Column for E2 exceeds the number of columns in block data");
	    return;
	}
	break;
    case SET_XYHILO:
	c1 = (int) GetChoice(eblock_e1_choice_item);
	c2 = (int) GetChoice(eblock_e2_choice_item);
	c3 = (int) GetChoice(eblock_e3_choice_item);
	if (c1 >= blockncols) {
	    errwin("Column for E1 exceeds the number of columns in block data");
	    return;
	}
	if (c2 >= blockncols) {
	    errwin("Column for E2 exceeds the number of columns in block data");
	    return;
	}
	if (c3 >= blockncols) {
	    errwin("Column for E3 exceeds the number of columns in block data");
	    return;
	}
	break;
    }
    setno = -1;
    graphno = (int) GetChoice(eblock_graph_choice_item) - 1;

    if (graphno == -1) {
	graphno = get_cg();
    }
    if (setno == -1) {
	setno = nextset(graphno);
    }
    if (setno == -1) {
	return;
    }
    if (!is_graph_active(graphno)) {
	set_graph_active(graphno, TRUE);
    }
    activateset(graphno, setno);
    settype(graphno, setno, block_curtype);

    tx = (double *) calloc(blocklen, sizeof(double));
    if (tx == NULL) {
	errwin("Can't allocate memory for X");
	return;
    }
    ty = (double *) calloc(blocklen, sizeof(double));
    if (ty == NULL) {
	free(tx);
	errwin("Can't allocate memory for Y");
	return;
    }
    for (i = 0; i < blocklen; i++) {
	if (cx == -1) {
	    tx[i] = i + 1;
	}
	else {
	    tx[i] = blockdata[cx][i];
	}
	ty[i] = blockdata[cy][i];
    }
    setcol(graphno, tx, setno, blocklen, 0);
    setcol(graphno, ty, setno, blocklen, 1);

    switch (block_curtype) {
    case SET_XY:
    case SET_BAR:
	sprintf(buf, "Cols %d %d", cx + 1, cy + 1);
	break;
    case SET_XYR:
    case SET_XYDX:
    case SET_XYDY:
    case SET_XYZ:
    case SET_BARDY:
	sprintf(buf, "Cols %d %d %d", cx + 1, cy + 1, c1 + 1);
	t2 = (double *) calloc(blocklen, sizeof(double));
	for (i = 0; i < blocklen; i++) {
	    t2[i] = blockdata[c1][i];
	}
	setcol(graphno, t2, setno, blocklen, 2);
	break;
    case SET_XYDXDX:
    case SET_XYDYDY:
    case SET_XYDXDY:
    case SET_BARDYDY:
	sprintf(buf, "Cols %d %d %d %d", cx + 1, cy + 1, c1 + 1, c2 + 1);
	t2 = (double *) calloc(blocklen, sizeof(double));
	t3 = (double *) calloc(blocklen, sizeof(double));
	for (i = 0; i < blocklen; i++) {
	    t2[i] = blockdata[c1][i];
	    t3[i] = blockdata[c2][i];
	}
	setcol(graphno, t2, setno, blocklen, 2);
	setcol(graphno, t3, setno, blocklen, 3);
	break;
    case SET_XYHILO:
	sprintf(buf, "Cols %d %d %d %d %d", cx + 1, cy + 1, c1 + 1, c2 + 1, c3 + 1);
	t2 = (double *) calloc(blocklen, sizeof(double));
	t3 = (double *) calloc(blocklen, sizeof(double));
	t4 = (double *) calloc(blocklen, sizeof(double));
	for (i = 0; i < blocklen; i++) {
	    t2[i] = blockdata[c1][i];
	    t3[i] = blockdata[c2][i];
	    t4[i] = blockdata[c3][i];
	}
	setcol(graphno, t2, setno, blocklen, 2);
	setcol(graphno, t3, setno, blocklen, 3);
	setcol(graphno, t4, setno, blocklen, 4);
	break;
    }

    setcomment(graphno, setno, buf);

    update_status_popup(NULL, NULL, NULL);
    drawgraph();
}

Widget *CreateBlockChoice(Widget parent, char *labelstr, int nsets, int type)
{
    int nmal, i = 0;
    XmString str;
    char *name = "setchoice";
    char buf[10];
    Widget *retval;

    switch (type) {
    case 0:
	nmal = nsets + 2;
	retval = (Widget *) XtMalloc(nmal * sizeof(Widget));
	retval[1] = XmCreatePulldownMenu(parent, name, NULL, 0);
	XtVaSetValues(retval[1],
		      XmNorientation, XmVERTICAL,
		      XmNpacking, XmPACK_COLUMN,
		      XmNnumColumns, nsets / 10,
		      NULL);
	i = 0;
	for (i = 0; i < nsets; i++) {
	    sprintf(buf, "%d", i + 1);
	    retval[i + 2] = XmCreatePushButton(retval[1], buf, NULL, 0);
	}
	XtManageChildren(retval + 2, nsets);

	str = XmStringCreate(labelstr, charset);

	retval[0] = XmCreateOptionMenu(parent, name, NULL, 0);
	XtVaSetValues(retval[0],
		      XmNlabelString, str,
		      XmNsubMenuId, retval[1],
		      XmNentryBorder, 2,
		      XmNwhichButton, 1,
		      NULL);
	XtManageChild(retval[0]);
	break;
    case 1:
	nmal = nsets + 3;
	retval = (Widget *) XtMalloc(nmal * sizeof(Widget));
	retval[1] = XmCreatePulldownMenu(parent, name, NULL, 0);
	XtVaSetValues(retval[1],
		      XmNorientation, XmVERTICAL,
		      XmNpacking, XmPACK_COLUMN,
		      XmNnumColumns, nsets / 10,
		      NULL);
	i = 0;
	retval[2] = XmCreatePushButton(retval[1], "Index", NULL, 0);
	for (i = 1; i < nsets + 1; i++) {
	    sprintf(buf, "%d", i);
	    retval[i + 2] = XmCreatePushButton(retval[1], buf, NULL, 0);
	}
	XtManageChildren(retval + 2, nsets + 1);

	str = XmStringCreate(labelstr, charset);

	retval[0] = XmCreateOptionMenu(parent, name, NULL, 0);
	XtVaSetValues(retval[0],
		      XmNlabelString, str,
		      XmNsubMenuId, retval[1],
		      XmNentryBorder, 2,
		      XmNwhichButton, 1,
		      NULL);
	XtManageChild(retval[0]);
	break;
    default:
    	/* error */
    	retval = (Widget *) NULL;
    	errmsg("Internal error, CreateBlockChoice called with wrong argument");
    }
    return retval;
}
