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
 * spreadsheet-like editing of data points
 *
 */

#include <config.h>
#include <cmath.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "globals.h"
#include "graphs.h"
#include "utils.h"
#include "files.h"
#include "plotone.h"
#include "protos.h"


#ifdef HAVE_LIBXBAE

#include <Xm/Xm.h>
#include <Xm/DialogS.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/Protocols.h>
#include <Xbae/Matrix.h>
#include "motifinc.h"


typedef struct _EditPoints {
    struct _EditPoints *next;
    int gno;
    int setno;
    int ncols;
    int nrows;
    Widget top;
    Widget mw;
    int cformat[MAX_SET_COLS];
    int cprec[MAX_SET_COLS];
    short cwidth[MAX_SET_COLS];
} EditPoints;

void update_cells(EditPoints *ep);
void do_update_cells(Widget w, XtPointer client_data, XtPointer call_data);

/* default cell value precision */
#define CELL_PREC 5

/* default cell value format (0 - Decimal; 1 - General; 2 - Exponential) */
#define CELL_FORMAT 1

/* default cell width */
#define CELL_WIDTH 10

char *scformat[3] =
{"%.*lf", "%.*lg", "%.*le"};

typedef enum {
    NoSelection,
    CellSelection,
    RowSelection,
    ColumnSelection
} SelectionType;

typedef enum {
    AddMode,
    ExclusiveMode
} SelectionMode;

typedef struct _SelectionStruct {
    int row, column;
    SelectionType type;
    SelectionMode mode;
    Boolean selected;
    Widget matrix;
} SelectionStruct, *SelectionPtr;

/*
 * delete the selected row
 */
void del_point_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
    int i,j;
    EditPoints *ep = (EditPoints *) client_data;

    XbaeMatrixGetCurrentCell(ep->mw, &i, &j);
    if(i >= ep->nrows || j >= ep->ncols) {
        errwin("Selected cell out of range");
        return;
    }
    del_point( ep->gno, ep->setno, i+1 );
    update_set_status(ep->gno, ep->setno);
    if(is_set_active(ep->gno, ep->setno)) {
        update_cells(ep);
    }
}


/*
 * add a point to a set by copying the selected cell and placing it after it
 */
void add_pt_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
    int i,j, k;
    double vals[MAX_SET_COLS];
    EditPoints *ep = (EditPoints *)client_data;
    int gno = ep->gno, setno = ep->setno;

    XbaeMatrixGetCurrentCell( ep->mw, &i, &j );
    if( i>=ep->nrows || j>=ep->ncols || i<0 || j<0 ){
            errwin( "Selected cell out of range" );
            return;
    }
    for( k=0; k<ep->ncols; k++ )
            vals[k] = *(getcol( gno, setno, k )+i );
    for( ;k<MAX_SET_COLS; k++ )
            vals[k] = 0.;
    add_point_at(gno, setno, i, 1, 
        vals[0], vals[1], vals[2], vals[3], dataset_type(gno, setno));
    update_set_status(gno, setno);
    if(is_set_active(gno, setno)) {
        update_cells(ep);
    }
}

static Widget *editp_col_item;
static Widget *editp_format_item;
static Widget *editp_precision_item;
static Widget *editp_width_item;

static void update_props(EditPoints *ep)
{
    int col;

    col = GetChoice(editp_col_item);
    if (col >= MAX_SET_COLS) {
    	col = 0;
    }

    SetChoice(editp_format_item, ep->cformat[col]); 

    SetChoice(editp_precision_item, ep->cprec[col]);
    SetChoice(editp_width_item, ep->cwidth[col] - 1);
}

static void do_accept_props(Widget w, XtPointer client_data, XtPointer call_data)
{
    int i, col, cformat, cprec, cwidth;
    EditPoints *ep = (EditPoints *) client_data;

    col = GetChoice(editp_col_item);
    cformat = GetChoice(editp_format_item);
    cprec = GetChoice(editp_precision_item);
    cwidth = GetChoice(editp_width_item) + 1;
    
    if (col < MAX_SET_COLS) {
        ep->cformat[col] = cformat;
        ep->cprec[col] = cprec;
        ep->cwidth[col] = cwidth;
    } else {	    /* do it for all columns */
    	for (i = 0; i < MAX_SET_COLS; i++) {
    	    ep->cformat[i] = cformat;
    	    ep->cprec[i] = cprec;
    	    ep->cwidth[i] = cwidth;
        }
    } 	
    XtVaSetValues(ep->mw, XmNcolumnWidths, ep->cwidth, NULL);
}

void do_update_cells(Widget w, XtPointer client_data, XtPointer call_data)
{
    update_cells((EditPoints *) client_data);
}

/*
 * redo frame since number of data points or set type, etc.,  may change 
 */
void update_cells(EditPoints *ep)
{
    int i, nr, nc;
    short widths[MAX_SET_COLS] =
       {CELL_WIDTH, CELL_WIDTH, CELL_WIDTH, CELL_WIDTH, CELL_WIDTH, CELL_WIDTH};
    short width;
    char buf[32];
    char **rowlabels;
	
    ep->nrows = getsetlength(ep->gno, ep->setno);
    ep->ncols = dataset_cols(ep->gno, ep->setno);
    /* get current size of widget and update rows/columns as needed */
    XtVaGetValues(ep->mw,
        XmNcolumns, &nc,
        XmNrows, &nr,
        NULL);
    if (ep->nrows > nr) {
        XbaeMatrixAddRows(ep->mw, 0, NULL, NULL, NULL, ep->nrows - nr);
    } else if (ep->nrows < nr) {
        XbaeMatrixDeleteRows(ep->mw, 0, nr - ep->nrows);
    }
    if (ep->ncols > nc) {
        XbaeMatrixAddColumns(ep->mw, 0, NULL, NULL, widths, NULL, 
            NULL, NULL, NULL, ep->ncols-nc);
    } else if (ep->ncols < nc) {
        XbaeMatrixDeleteColumns(ep->mw, 0, nc - ep->ncols);
    }
		
    rowlabels = malloc(ep->nrows*sizeof(char *));
    for (i = 0; i < ep->nrows; i++) {
    	sprintf(buf, "%d", i);
    	rowlabels[i] = malloc((sizeof(buf) + 1)*SIZEOF_CHAR);
    	strcpy(rowlabels[i], buf);
    }
    width = (short) ceil(log10(i)) + 2;	/* increase row label width by 1 */

    XtVaSetValues(ep->mw,
        XmNrowLabels, rowlabels,
	XmNrowLabelWidth, width,
        XmNcolumnWidths, ep->cwidth,
	NULL);

    /* free memory used to hold strings */
    for (i = 0; i < ep->nrows; i++) {
	free(rowlabels[i]);
    }
    free(rowlabels);
}

void do_props_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    static Widget top, dialog;
    EditPoints *ep;
    static Widget but1[2];

    set_wait_cursor();
    ep = (EditPoints *) client_data;
    if (top == NULL) {
        char *label1[2];
        label1[0] = "Accept";
        label1[1] = "Close";
	top = XmCreateDialogShell(app_shell, "Edit set props", NULL, 0);
	handle_close(top);
	dialog = XmCreateRowColumn(top, "dialog_rc", NULL, 0);

	editp_col_item = CreatePanelChoice(dialog, "Apply to column:",
				    8, "1", "2", "3", "4", "5", "6", "All",
					   NULL, 0);

	editp_format_item = CreatePanelChoice(dialog, "Format:",
					      4,
					      "Decimal",
					      "General",
					      "Exponential",
					      NULL, 0);

	editp_precision_item = CreatePanelChoice(dialog, "Precision:",
						 16,
						 "0", "1", "2", "3", "4",
						 "5", "6", "7", "8", "9",
						 "10", "11", "12", "13", "14",
						 NULL, 0);

	editp_width_item = CreatePanelChoice0(dialog, "Width:",
					5, 21,
				"1", "2", "3", "4", "5",
				"6", "7", "8", "9", "10", 
				"11", "12", "13", "14", "15",
				"16", "17", "18", "19", "20",
					NULL, 0);

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

	CreateCommandButtons(dialog, 2, but1, label1);
	XtAddCallback(but1[1], XmNactivateCallback,
	    	(XtCallbackProc) destroy_dialog, (XtPointer) top);
	XtManageChild(dialog);
    }
    /* TODO: remove the dirty stuff */
    XtRemoveAllCallbacks(but1[0], XmNactivateCallback);
    XtAddCallback(but1[0], XmNactivateCallback,
    	    (XtCallbackProc) do_accept_props, (XtPointer) ep);
    update_props(ep);
    XtRaise(top);
    unset_wait_cursor();
}


void leaveCB(Widget w, XtPointer client_data, XtPointer calld)
{
    double *datap;
    char buf[128];
    EditPoints *ep = (EditPoints *) client_data;
    XbaeMatrixLeaveCellCallbackStruct *cs =
    	    (XbaeMatrixLeaveCellCallbackStruct *) calld;

    datap = getcol(ep->gno, ep->setno, cs->column);
    sprintf(buf, scformat[(ep->cformat[cs->column])], ep->cprec[cs->column],
    	    datap[cs->row]);
    if (strcmp(buf, cs->value) != 0) {
	datap[cs->row] = atof(cs->value);

	update_set_status(ep->gno, ep->setno);
	drawgraph();
    }
}


void drawcellCB(Widget w, XtPointer client_data, XtPointer calld)
{
    int i, j;
    double *datap;
    EditPoints *ep = (EditPoints *) client_data;
    static char buf[128];
    XbaeMatrixDrawCellCallbackStruct *cs =
    	    (XbaeMatrixDrawCellCallbackStruct *) calld;

    i = cs->row;
    j = cs->column;
    datap = getcol(ep->gno, ep->setno, j);
    sprintf(buf, scformat[(ep->cformat[j])], ep->cprec[j], datap[i]);
    cs->type = XbaeString;
    cs->string = XtNewString(buf);
}

void selectCB(Widget w, XtPointer client_data, XtPointer call_data)
{
    XbaeMatrixSelectCellCallbackStruct *sc =
        (XbaeMatrixSelectCellCallbackStruct *) call_data;

    XbaeMatrixSelectCell(w, sc->row, sc->column);
}


void writeCB(Widget w, XtPointer client_data, XtPointer call_data)
{
}


static EditPoints *ep_start = NULL;

void delete_ep(EditPoints *ep)
{
    EditPoints *ep_tmp = ep_start;
    
    if (ep == NULL) {
        return;
    }
    
    if (ep == ep_start) {
        ep_start = ep_start->next;
        cxfree(ep);
        return;
    }
    
    while (ep_tmp != NULL) {
        if (ep_tmp->next == ep) {
            ep_tmp->next = ep->next;
            cxfree(ep);
            return;
        }
        ep_tmp = ep_tmp->next;
    }
}

EditPoints *get_ep(int gno, int setno)
{
    EditPoints *ep_tmp = ep_start;

    while (ep_tmp != NULL) {
        if (ep_tmp->gno == gno && ep_tmp->setno == setno) {
            break;
        }
        ep_tmp = ep_tmp->next;
    }
    return ep_tmp;
}

void destroy_ep(Widget w, XtPointer client_data, XtPointer call_data)
{
    EditPoints *ep;
    
    ep = (EditPoints *) client_data;
    deletewidget(ep->top);
    delete_ep(ep);
}

void create_ss_frame(int gno, int setno)
{
    int i;
    char *collabels[MAX_SET_COLS] = {"X", "Y", "Y1", "Y2", "Y3", "Y4"};
    char wname[256];
    char *label1[3] = {"Props...", "Update", "Close"};
    char *label2[2] = {"Delete", "Add"};
    EditPoints *ep;
    Atom WM_DELETE_WINDOW;
    Widget dialog;
    Widget but1[3], but2[2];
    
    ep = get_ep(gno, setno);
    if (ep != NULL) {
        XtRaise(ep->top);
        return;
    }
    
    set_wait_cursor();

    ep = malloc(sizeof(EditPoints));
    ep->next = ep_start;
    ep_start = ep;
    
    ep->gno = gno;
    ep->setno = setno;
    ep->ncols = getncols(gno, setno);
    ep->nrows = getsetlength(gno, setno);
    for (i = 0; i < MAX_SET_COLS; i++) {
        ep->cwidth[i] = CELL_WIDTH;
        ep->cprec[i] = CELL_PREC;
        ep->cformat[i] = CELL_FORMAT;
    }

    sprintf(wname, "Edit set: S%d of G%d", ep->setno, ep->gno);
    ep->top = XmCreateDialogShell(app_shell, wname, NULL, 0);
    XtVaSetValues(ep->top, XmNdeleteResponse, XmDO_NOTHING, NULL);
    WM_DELETE_WINDOW = XmInternAtom(XtDisplay(app_shell),
        "WM_DELETE_WINDOW", False);
    XmAddProtocolCallback(ep->top,
        XM_WM_PROTOCOL_ATOM(ep->top), WM_DELETE_WINDOW,
        destroy_ep, (XtPointer) ep);

    dialog = XmCreateRowColumn(ep->top, "dialog_rc", NULL, 0);

    ep->mw = XtVaCreateManagedWidget("mw",
        xbaeMatrixWidgetClass, dialog,
        XmNrows, ep->nrows,
        XmNcolumns, ep->ncols,
        XmNvisibleRows, 10,
        XmNvisibleColumns, 2,
        XmNcolumnLabels, collabels,
        XmNgridType, XmGRID_SHADOW_IN,
        XmNcellShadowType, XmSHADOW_ETCHED_OUT,
        XmNcellShadowThickness, 4,
        NULL);

    update_cells(ep);
    				     
    XtAddCallback(ep->mw, XmNselectCellCallback, selectCB, ep);	
    XtAddCallback(ep->mw, XmNdrawCellCallback, drawcellCB, ep);	
    XtAddCallback(ep->mw, XmNleaveCellCallback, leaveCB, ep);
    XtAddCallback(ep->mw, XmNwriteCellCallback, writeCB, ep);  

    CreateSeparator(dialog);
    
    CreateCommandButtons(dialog, 2, but2, label2);
    XtAddCallback(but2[0], XmNactivateCallback, (XtCallbackProc) del_point_cb,
    	    (XtPointer) ep);
    XtAddCallback(but2[1], XmNactivateCallback, (XtCallbackProc) add_pt_cb,
    	    (XtPointer) ep);
    
    CreateSeparator(dialog);

    CreateCommandButtons(dialog, 3, but1, label1);
    XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc) do_props_proc,
    	    (XtPointer) ep);
    XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc) do_update_cells,
    	    (XtPointer) ep);
    XtAddCallback(but1[2], XmNactivateCallback, (XtCallbackProc) destroy_ep,
    	    (XtPointer) ep);

    XtManageChild(dialog);
    XtRaise(ep->top);
    unset_wait_cursor();
}
#endif

/*
 * Start up editor using GRACE_EDITOR variable
 * Note the change to the GRACE_EDITOR variable: If it requires a text 
 * terminal it must provide it explicitly with an xterm -e prefix 
 */
void do_ext_editor(int gno, int setno)
{
    char *fname, ebuf[256], *s;
    FILE *cp;

    fname = tmpnam(NULL);
    cp = grace_openw(fname);
    if (cp == NULL) {
        return;
    }

    write_set(gno, setno, cp, sformat);
    grace_close(cp);

    if ((s = getenv("GRACE_EDITOR")) != NULL) {
    	strcpy(ebuf, s);
    } else {
    	strcpy(ebuf, "xterm -e vi");
    }
    sprintf(ebuf, "%s %s", ebuf, fname);
    system(ebuf);

    if (is_set_active(gno, setno)) {
	killsetdata(gno, setno);	
        getdata(gno, fname, SOURCE_DISK, dataset_type(gno, setno));
    } else {
        getdata(gno, fname, SOURCE_DISK, SET_XY);
    }
    unlink(fname);
    update_all();
    drawgraph();
}
