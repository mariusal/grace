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
#include "plotone.h"
#include "motifinc.h"
#include "protos.h"

static SetChoiceItem editp_set_item;

#ifdef HAVE_LIBXBAE

#include <Xbae/Matrix.h>

void update_cells(Widget, XtPointer, XtPointer);

typedef struct _EditPoints {
    int gno;
    int setno;
    int ncols;
    int nrows;
    String *collabels;
    short *widths;
    Widget top;
    Widget mw;
    Widget *editp_format_item;
    Widget *editp_precision_item;
    Widget *editp_width_item;
    int cformat[MAX_SET_COLS];
    int cprec[MAX_SET_COLS];
    int cwidth[MAX_SET_COLS];
} EditPoints;

String **cells;
Widget ext_editor_item;

static short widths[MAX_SET_COLS] =
{10, 10, 10, 10, 10, 10};
static int precision[MAX_SET_COLS] =
{5, 5, 5, 5, 5, 5};

/* 0 - Decimal; 1 - General; 2 - Exponential */
static int format[MAX_SET_COLS] =
{1, 1, 1, 1, 1, 1};

char *scformat[3] =
{"%.*lf", "%.*lg", "%.*le"};

String labels1[2] =
{"X", "Y"};
String labels2[3] =
{"X", "Y", "DX"};
String labels3[3] =
{"X", "Y", "DY"};
String labels4[4] =
{"X", "Y", "DX1", "DX2"};
String labels5[4] =
{"X", "Y", "DY1", "DY2"};
String labels6[4] =
{"X", "Y", "DX", "DY"};
String labels7[3] =
{"X", "Y", "DZ"};
String labels8[5] =
{"X", "HI", "LO", "OPEN", "CLOSE"};
String labels9[3] =
{"X", "Y", "Radius"};

String *rowlabels;
String *collabels = labels1;

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
}
*SelectionPtr, SelectionStruct;

void create_ss_frame(EditPoints * ep);

EditPoints *newep(int gno, int setno)
{
    int i;
    EditPoints *ep;
    ep = malloc(sizeof(EditPoints));
    ep->gno = gno;
    ep->setno = setno;
    ep->ncols = getncols(gno, setno);
    ep->nrows = getsetlength(gno, setno);
    set_ep_structure(gno, setno, ep);
    switch (dataset_type(gno, setno)) {
    case SET_XY:
    case SET_BAR:
	ep->collabels = labels1;
	break;
    case SET_XYDX:
	ep->collabels = labels2;
	break;
    case SET_XYDY:
    case SET_BARDY:
	ep->collabels = labels3;
	break;
    case SET_XYDXDX:
	ep->collabels = labels4;
	break;
    case SET_XYDYDY:
    case SET_BARDYDY:
	ep->collabels = labels5;
	break;
    case SET_XYDXDY:
	ep->collabels = labels6;
	break;
    case SET_XYZ:
	ep->collabels = labels7;
	break;
    case SET_XYHILO:
	ep->collabels = labels8;
	break;
    case SET_XYR:
	ep->collabels = labels9;
	break;
    }
    for (i=0; i < MAX_SET_COLS; i++) {
		ep->cwidth[i] = widths[i];
		ep->cprec[i] = precision[i];
		ep->cformat[i] = format[i];
    }
    return ep;
}


/* 
 * Get the selected set and call the routine to open up an Xbae widget 
 */
void do_ss_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    EditPoints *ep;
    int gno = get_cg();
    int setno = GetSelectedSet(editp_set_item);
    if (setno == SET_SELECT_ERROR) {
        errwin("No set selected");
        return;
    }
    if( setno == SET_SELECT_NEXT ) {
        if( (setno=nextset(gno)) != -1 ) { 
            add_point(gno, setno, 0., 0., 0, 0, SET_XY);
            add_point(gno, setno, 1, 1, 0, 0, SET_XY);
            setcomment(gno, setno, "editor");
            update_set_status( gno, setno );
        } else {
            errwin("No set selected");
            return;
        }
    }
    if (is_set_active(gno, setno)) {
	if (((ep = get_ep_structure(gno, setno)) != NULL) && (ep->top != NULL)) {
	    XtRaise(ep->top);
	} else {
	    ep = newep(gno, setno);
	    create_ss_frame(ep);
	}
    } else {
        errwin("Set not active");
    }
}


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
        update_cells(NULL, (XtPointer)ep, 0);
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
        update_cells(NULL, (XtPointer) ep, 0 );
    }
}

String **MakeCells(EditPoints * ep)
{
    char buf[512];
    int i, j;
    double *datap;
    String **cells = NULL;
    if (ep != NULL) {
	cells = (String **) XtMalloc(sizeof(String *) * ep->nrows);
	if (cells != NULL) {
	    for (i = 0; i < ep->nrows; i++) {
		cells[i] = (String *) XtMalloc(sizeof(String) * ep->ncols);
		for (j = 0; j < ep->ncols; j++) {
		    datap = getcol(ep->gno, ep->setno, j);
		    sprintf(buf, scformat[(ep->cformat[j])], ep->cprec[j], datap[i]);
		    cells[i][j] = XtNewString(buf);
		}
	    }
	}
    }
    return cells;
}

static Widget *editp_col_item;
static Widget *editp_format_item;
static Widget *editp_precision_item;
static Widget *editp_width_item;

static void update_props(EditPoints * ep)
{
    int col;
    short *widths;
    Widget matrix = ep->mw;
    col = GetChoice(editp_col_item);
    if( col==6 )
    	col = 0;
    XtVaGetValues(matrix,
		  XmNcolumnWidths, &widths,
		  NULL);

    SetChoice(editp_format_item, ep->cformat[col]); 

    SetChoice(editp_precision_item, ep->cprec[col]);
    SetChoice(editp_width_item, ep->cwidth[col] - 1);
}

static void do_accept_props(Widget w, XtPointer client_data, XtPointer call_data)
{
    int i, col;
    short widths[6];
    EditPoints *ep = (EditPoints *) client_data;
    Widget matrix = ep->mw;
    col = GetChoice(editp_col_item);

	if( col<6 ) {
    	ep->cformat[col] = GetChoice(editp_format_item);
    	ep->cprec[col] = GetChoice(editp_precision_item);
    	ep->cwidth[col] = GetChoice(editp_width_item) + 1;
   		for (i=0;i<6;i++) {
			widths[i] = ep->cwidth[i];
    	}
    } else {	    /* do it for all columns */
    	for (i=0;i<6;i++) {
    		ep->cformat[i] = GetChoice(editp_format_item);
    		ep->cprec[i] = GetChoice(editp_precision_item);
    		widths[i] = ep->cwidth[i] = GetChoice(editp_width_item) + 1;
		}
	} 	
    XtVaSetValues(matrix, XmNcolumnWidths, widths, NULL);
}


void update_cells(Widget w, XtPointer client_data, XtPointer call_data)
/*
 * redo frame since number of data points or set type, etc.,  may change 
 */
{
	EditPoints *ep = (EditPoints *)client_data;
	int i, j, nr, nc;
	short width;
	char buf[32];
	
    ep->nrows = getsetlength(ep->gno, ep->setno);
    switch (dataset_type(ep->gno, ep->setno)) {
    	case SET_XY:
    	case SET_BAR:
			ep->collabels = labels1;
			break;
    	case SET_XYDX:
			ep->collabels = labels2;
			break;
    	case SET_XYDY:
    	case SET_BARDY:
			ep->collabels = labels3;
			break;
    	case SET_XYDXDX:
			ep->collabels = labels4;
			break;
    	case SET_XYDYDY:
    	case SET_BARDYDY:
			ep->collabels = labels5;
			break;
    	case SET_XYDXDY:
			ep->collabels = labels6;
			break;
    	case SET_XYZ:
			ep->collabels = labels7;
			break;
    	case SET_XYHILO:
			ep->collabels = labels8;
			break;
    	case SET_XYR:
			ep->collabels = labels9;
			break;
    }
    ep->ncols = settype_cols(dataset_type(ep->gno, ep->setno));

    cells = MakeCells( ep );
			
    rowlabels = (String *) malloc(ep->nrows * sizeof(String));
    for (i = 0; i < ep->nrows; i++) {
    	sprintf(buf, "%d", i + 1);
    	rowlabels[i] = (String) malloc((sizeof(buf) + 1) * sizeof(char));
    	strcpy(rowlabels[i], buf);
    }
    width = (short) ceil(log10(i))+2;	/* increase row label width by 1 */

	/* get current size of widget 			*/
	XtVaGetValues( ep->mw, XmNcolumns, &nc, 
						   XmNrows, &nr, 
    					   NULL );
	if( ep->nrows > nr )
		XbaeMatrixAddRows( ep->mw, 0, NULL, NULL, NULL, ep->nrows-nr );
	else if( ep->nrows < nr )
		XbaeMatrixDeleteRows( ep->mw, 0, nr - ep->nrows );
	if( ep->ncols > nc )
		XbaeMatrixAddColumns( ep->mw, 0, NULL, NULL, widths, NULL, 
					    NULL, NULL, NULL, ep->ncols-nc );
	else if( ep->ncols < nc )
		XbaeMatrixDeleteColumns( ep->mw, 0, nc - ep->ncols );

    XtVaSetValues(ep->mw, XmNrowLabels, rowlabels,
			  XmNcells, cells,
			  XmNcolumnLabels, ep->collabels,
			  XmNrowLabelWidth, width,
			  NULL);

    /* free memory used to hold strings */
    for (i = 0; i < ep->nrows; i++) {
		for (j = 0; j < ep->ncols; j++) {
	    	XtFree((XtPointer) cells[i][j]);
		}
		XtFree((XtPointer) cells[i]);
		free( rowlabels[i] );
    }
    XtFree((XtPointer) cells);
}

void do_props_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    static Widget top, acceptcallback;
    Widget dialog;
    EditPoints *ep = (EditPoints *) client_data;

    set_wait_cursor();
    if (top == NULL) {
	Widget but1[2];
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
	XtAddCallback(but1[0], XmNactivateCallback,
	    	(XtCallbackProc) do_accept_props, (XtPointer) ep);
	XtAddCallback(but1[1], XmNactivateCallback,
	    	(XtCallbackProc) destroy_dialog, (XtPointer) top);
	XtManageChild(dialog);
	acceptcallback = but1[0];
    }
    XtRemoveAllCallbacks(acceptcallback, XmNactivateCallback);
    XtAddCallback(acceptcallback, XmNactivateCallback,
    	    (XtCallbackProc) do_accept_props, (XtPointer) ep);
    update_props(ep);
    XtRaise(top);
    unset_wait_cursor();
}


void leaveCB(Widget w, XtPointer client_data, XtPointer calld)
{
    double *datap;
    String **cells;
    char buf[128];
    EditPoints *ep = (EditPoints *) client_data;
    XbaeMatrixLeaveCellCallbackStruct *cs =
    	    (XbaeMatrixLeaveCellCallbackStruct *) calld;
    XtVaGetValues(w, XmNcells, &cells, NULL);
    datap = getcol(ep->gno, ep->setno, cs->column);
    sprintf(buf, scformat[(ep->cformat[cs->column])], ep->cprec[cs->column],
    	    datap[cs->row]);
    if (strcmp(buf, cs->value) != 0) {
	String s = (String) XtMalloc(sizeof(char) * (strlen(buf) + 1));
	strcpy(s, buf);
	cells[cs->row][cs->column] = s;
	datap[cs->row] = atof(cs->value);

	update_set_status(ep->gno, ep->setno);
	drawgraph();
    }
}


void drawcellCB(Widget w, XtPointer client_data, XtPointer calld)
{
    double *datap;
    EditPoints *ep = (EditPoints *) client_data;
    static char buf[128];
    int i, j;
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
    XbaeMatrixSelectRow(w, sc->row);
    XbaeMatrixSelectColumn(w, sc->column);
}


void writeCB(Widget w, XtPointer client_data, XtPointer calld)
{
}

void create_ss_frame(EditPoints * ep)
{
    Widget dialog;
    char buf[32], wname[256];
    int i, j;
    Widget but1[3], but2[2];
    char *label1[3] = { "Props...", "Update", "Close" };
    char *label2[2] = { "Delete", "Add" };
    short width;
    
    set_wait_cursor();
    sprintf( wname, "Edit set: S%d of G%d", ep->setno, ep->gno );
    ep->top = XmCreateDialogShell(app_shell, wname, NULL, 0);
    handle_close(ep->top);
    dialog = XmCreateRowColumn(ep->top, "dialog_rc", NULL, 0);

    cells = MakeCells(ep);

    rowlabels = (String *) malloc(ep->nrows * sizeof(String));
    for (i = 0; i < ep->nrows; i++) {
    	sprintf(buf, "%d", i + 1);
    	rowlabels[i] = (String) malloc((sizeof(buf) + 1) * sizeof(char));
    	strcpy(rowlabels[i], buf);
    }
    /* added by Ed Vigmond -- increase row label width by 1 */
    width = (short) ceil(log10(i))+1;

    ep->mw = XtVaCreateManagedWidget("mw",
				     xbaeMatrixWidgetClass, dialog,
				     XmNrows, ep->nrows,
				     XmNcolumns, ep->ncols,
				     XmNcolumnWidths, widths,
				     XmNvisibleRows, 10,
				     XmNvisibleColumns, 2,
				     XmNrowLabels, rowlabels,
				     XmNcolumnLabels, ep->collabels,
				     XmNcells, cells,
				     XmNgridType, XmGRID_SHADOW_IN,
				     XmNcellShadowType, XmSHADOW_ETCHED_OUT,
				     XmNcellShadowThickness, 4,
				     XmNrowLabelWidth, width,
				     NULL);
				     
    XtAddCallback(ep->mw, XmNselectCellCallback, selectCB, ep);	

    XtAddCallback(ep->mw, XmNdrawCellCallback, drawcellCB, ep);	
    XtAddCallback(ep->mw, XmNleaveCellCallback, leaveCB, ep);
    XtAddCallback(ep->mw, XmNwriteCellCallback, writeCB, ep);  
        
    for (i = 0; i < ep->nrows; i++) {
	for (j = 0; j < ep->ncols; j++) {
	    XtFree((XtPointer) cells[i][j]);
	}
	free( rowlabels[i] );
	XtFree((XtPointer) cells[i]);
    }
    XtFree((XtPointer) cells);

    XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);
    
    CreateCommandButtons(dialog, 2, but2, label2);
    XtAddCallback(but2[0], XmNactivateCallback, (XtCallbackProc) del_point_cb,
    	    (XtPointer) ep);
    XtAddCallback(but2[1], XmNactivateCallback, (XtCallbackProc) add_pt_cb,
    	    (XtPointer) ep);
    
    XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

    CreateCommandButtons(dialog, 3, but1, label1);
    XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc) do_props_proc,
    	    (XtPointer) ep);
    XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc) update_cells,
    	    (XtPointer) ep);
    XtAddCallback(but1[2], XmNactivateCallback, (XtCallbackProc) destroy_dialog,
    	    (XtPointer) ep->top);

    XtManageChild(dialog);
    XtRaise(ep->top);
    unset_wait_cursor();
}
#endif

void setsel_cred_cb(Widget w, XtPointer client_data, XtPointer call_data)
/*
 * callback to grey out forumla button if "New set" not selcted
 */
{
	if( GetSelectedSet(editp_set_item) != SET_SELECT_NEXT ) 
		XtSetSensitive( (Widget)client_data, False );
	else
		XtSetSensitive( (Widget)client_data, True );
}


/*
 * Start up editor using GRACE_EDITOR variable
 * Note the change to the GRACE_EDITOR variable: If it requires a text 
 * terminal it must provide it explicitly with an xterm -e prefix 
 */
void do_ext_editor(Widget w, XtPointer client_data, XtPointer call_data)
{
    char *fname, ebuf[256], *s;
    int setno = GetSelectedSet(editp_set_item), curgraph=get_cg();

    if (setno == SET_SELECT_ERROR) {
        errwin("No set selected");
        return;
    }

    fname = tmpnam(NULL);

    if( setno == SET_SELECT_NEXT ){ 
        if( (setno=nextset(curgraph)) == -1 ){ 
            errwin("Not enough sets");
            return;
        }
        activateset( curgraph, setno );
        do_writesets(curgraph, setno, 0, fname, sformat);
    } else {
        do_writesets(curgraph, setno, 0, fname, sformat);
    }

    if ((s = getenv("GRACE_EDITOR")) != NULL) {
    	strcpy(ebuf, s);
    } else {
    	strcpy(ebuf, "xterm -e vi");
    }
    sprintf(ebuf, "%s %s", ebuf, fname);
    system(ebuf);
    if( is_set_active( curgraph, setno ) ) {
	killsetdata( curgraph, setno );	
        getdata(curgraph, fname, SOURCE_DISK, dataset_type( curgraph, setno ) );
    } else {
        setcomment( curgraph, setno, "editor" );
        getdata(curgraph, fname, SOURCE_DISK, SET_XY );
    }
    sprintf(ebuf, "rm %s", fname);
    system(ebuf);
    update_all();
    drawgraph();
}

void edit_set_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
#ifdef HAVE_LIBXBAE
 	if( XmToggleButtonGetState(ext_editor_item) == False )
		do_ss_frame( NULL, NULL, NULL );
	else
#endif
	do_ext_editor( NULL, NULL, NULL );
}

void create_editp_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    static Widget top;
    Widget dialog;

    set_wait_cursor();

    if (top == NULL) {
		Widget but1[3];
		char *label1[3];
		label1[0] = "Edit";
		label1[1] = "Formula";
		label1[2] = "Close";
		top = XmCreateDialogShell(app_shell, "Edit/Create set", NULL, 0);
		handle_close(top);
		dialog = XmCreateRowColumn(top, "dialog_rc", NULL, 0);
		editp_set_item = CreateSetSelector(dialog, "Edit set:",
										   SET_SELECT_NEXT,
										   FILTER_SELECT_NONE,
										   GRAPH_SELECT_CURRENT,
										   SELECTION_TYPE_SINGLE);

		XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

#ifdef HAVE_LIBXBAE
		ext_editor_item = XtVaCreateManagedWidget("Use external editor", 
                        xmToggleButtonWidgetClass, dialog, 
                        NULL);
		XmToggleButtonSetState( ext_editor_item, False, False );
		XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);
#endif
		CreateCommandButtons(dialog, 3, but1, label1);
		
		XtAddCallback(but1[0], XmNactivateCallback,
				(XtCallbackProc) edit_set_proc, NULL);
		XtAddCallback(but1[1], XmNactivateCallback,
				(XtCallbackProc) create_leval_frame, NULL);
		XtAddCallback(but1[2], XmNactivateCallback,
				(XtCallbackProc) destroy_dialog, (XtPointer) top);

		XtAddCallback(editp_set_item.list, XmNsingleSelectionCallback,
				(XtCallbackProc) setsel_cred_cb, (XtPointer)but1[1] );

	XtManageChild(dialog);
    }
    XtRaise(top);
    unset_wait_cursor();
}
