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

#include <Xm/Xm.h>
#include <Xbae/Matrix.h>
#include "motifinc.h"


typedef struct _EditPoints {
    struct _EditPoints *next;
    int gno;
    int setno;
    int ncols;
    int scols;
    int nrows;
    Widget top;
    Widget mw;
    Widget label;
    int cformat[MAX_SET_COLS];
    int cprec[MAX_SET_COLS];
} EditPoints;

void update_cells(EditPoints *ep);
void do_update_cells(Widget w, XtPointer client_data, XtPointer call_data);

/* default cell value precision */
#define CELL_PREC 8

/* default cell value format (0 - Decimal; 1 - General; 2 - Exponential) */
#define CELL_FORMAT 1

/* default cell width */
#define CELL_WIDTH 12

/* string cell width */
#define STRING_CELL_WIDTH 128

char *scformat[3] =
{"%.*lf", "%.*lg", "%.*le"};

/*
 * delete the selected row
 */
void del_point_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
    int i,j;
    EditPoints *ep = (EditPoints *) client_data;

    XbaeMatrixGetCurrentCell(ep->mw, &i, &j);
    if (i >= ep->nrows || j >= ep->ncols + ep->scols) {
        errwin("Selected cell out of range");
        return;
    }
    del_point(ep->gno, ep->setno, i);
    update_set_lists(ep->gno);
    if(is_set_active(ep->gno, ep->setno)) {
        update_cells(ep);
    }
    drawgraph();
}


/*
 * add a point to a set by copying the selected cell and placing it after it
 */
void add_pt_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
    int i,j, k;
    char **s;
    Datapoint dpoint;
    EditPoints *ep = (EditPoints *)client_data;
    int gno = ep->gno, setno = ep->setno;

    XbaeMatrixGetCurrentCell(ep->mw, &i, &j);
    if (i >= ep->nrows || j >= ep->ncols + ep->scols || i < 0 || j < 0){
        errmsg("Selected cell out of range");
        return;
    }
    zero_datapoint(&dpoint);
    for (k = 0; k < ep->ncols; k++) {
        dpoint.ex[k] = *(getcol(gno, setno, k) + i);
    }
    if ((s = get_set_strings(gno, setno)) != NULL) {
        dpoint.s = s[i];
    }
    add_point_at(gno, setno, i + 1, &dpoint);
    if(is_set_active(gno, setno)) {
        update_cells(ep);
    }
    update_set_lists(gno);
    drawgraph();
}

static Widget *editp_col_item;
static Widget *editp_format_item;
static Widget *editp_precision_item;

static void update_props(EditPoints *ep)
{
    int col;

    col = GetChoice(editp_col_item);
    if (col >= MAX_SET_COLS) {
    	col = 0;
    }

    SetChoice(editp_format_item, ep->cformat[col]); 

    SetChoice(editp_precision_item, ep->cprec[col]);
}

static int do_accept_props(void *data)
{
    int i, col, cformat, cprec;
    EditPoints *ep = *((EditPoints **) data);

    col = GetChoice(editp_col_item);
    cformat = GetChoice(editp_format_item);
    cprec = GetChoice(editp_precision_item);
    
    if (col < MAX_SET_COLS) {
        ep->cformat[col] = cformat;
        ep->cprec[col] = cprec;
    } else {	    /* do it for all columns */
    	for (i = 0; i < MAX_SET_COLS; i++) {
    	    ep->cformat[i] = cformat;
    	    ep->cprec[i] = cprec;
        }
    }
    
    update_cells(ep);
    
    return RETURN_SUCCESS;
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
    short widths[MAX_SET_COLS + 1];
    int maxlengths[MAX_SET_COLS + 1];
    short width;
    char buf[32];
    char **rowlabels;

    sprintf(buf, "Set G%d.S%d", ep->gno, ep->setno);
    SetLabel(ep->label, buf);
	
    ep->nrows = getsetlength(ep->gno, ep->setno);
    ep->ncols = dataset_cols(ep->gno, ep->setno);
    for (i = 0; i < MAX_SET_COLS; i++) {
        widths[i] = CELL_WIDTH;
        maxlengths[i] = CELL_WIDTH;
    }
    if (get_set_strings(ep->gno, ep->setno) != NULL) {
        ep->scols = 1;
        widths[i] = CELL_WIDTH;
        maxlengths[i] = STRING_CELL_WIDTH;
    } else {
        ep->scols = 0;
    }
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
    if (ep->ncols + ep->scols > nc) {
        XbaeMatrixAddColumns(ep->mw, 0, NULL, NULL, widths, maxlengths, 
            NULL, NULL, NULL, ep->ncols + ep->scols - nc);
    } else if (ep->ncols + ep->scols < nc) {
        XbaeMatrixDeleteColumns(ep->mw, 0, nc - (ep->ncols + ep->scols));
    }
		
    rowlabels = xmalloc(ep->nrows*sizeof(char *));
    for (i = 0; i < ep->nrows; i++) {
    	sprintf(buf, "%d", i);
    	rowlabels[i] = copy_string(NULL, buf);
    }
    width = (short) ceil(log10(i)) + 2;	/* increase row label width by 1 */

    XtVaSetValues(ep->mw,
        XmNrowLabels, rowlabels,
	XmNrowLabelWidth, width,
	NULL);

    /* free memory used to hold strings */
    for (i = 0; i < ep->nrows; i++) {
	xfree(rowlabels[i]);
    }
    xfree(rowlabels);
}

void do_props_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    static Widget top;
    static EditPoints *ep;

    set_wait_cursor();
    ep = (EditPoints *) client_data;
    
    if (top == NULL) {
        Widget dialog;
	top = CreateDialogForm(app_shell, "Edit set properties");
	dialog = CreateVContainer(top);

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

	CreateAACDialog(top, dialog, do_accept_props, &ep);
    }
    update_props(ep);
    
    RaiseWindow(GetParent(top));
    unset_wait_cursor();
}


static void leaveCB(Widget w, XtPointer client_data, XtPointer calld)
{
    int changed = FALSE;
    EditPoints *ep = (EditPoints *) client_data;
    XbaeMatrixLeaveCellCallbackStruct *cs =
    	    (XbaeMatrixLeaveCellCallbackStruct *) calld;

    /* TODO: add edit_point() function to setutils.c */
    if (cs->column < ep->ncols) {
        char buf[128];
        double *datap = getcol(ep->gno, ep->setno, cs->column);
        sprintf(buf, scformat[(ep->cformat[cs->column])], ep->cprec[cs->column],
    	        datap[cs->row]);
        if (strcmp(buf, cs->value) != 0) {
	    datap[cs->row] = atof(cs->value);
            changed = TRUE;
        }
    } else if (cs->column < ep->ncols + ep->scols) {
        char **datap = get_set_strings(ep->gno, ep->setno);
        if (compare_strings(datap[cs->row], cs->value) == 0) {
	    datap[cs->row] = copy_string(datap[cs->row], cs->value);
            changed = TRUE;
        }
    } else {
        errmsg("Internal error in leaveCB()");
    }
    
    if (changed) {
        set_dirtystate();
        update_set_lists(ep->gno);
        drawgraph();
    }
}


static void drawcellCB(Widget w, XtPointer client_data, XtPointer calld)
{
    int i, j;
    EditPoints *ep = (EditPoints *) client_data;
    XbaeMatrixDrawCellCallbackStruct *cs =
    	    (XbaeMatrixDrawCellCallbackStruct *) calld;

    i = cs->row;
    j = cs->column;
    
    cs->type = XbaeString;
    if (j < ep->ncols) {
        static char buf[128];
        double *datap;
        datap = getcol(ep->gno, ep->setno, j);
        sprintf(buf, scformat[(ep->cformat[j])], ep->cprec[j], datap[i]);
        cs->string = copy_string(NULL, buf);
    } else {
        char **datap;
        datap = get_set_strings(ep->gno, ep->setno);
        cs->string = datap[i];
    }
}

static void selectCB(Widget w, XtPointer client_data, XtPointer call_data)
{
    XbaeMatrixSelectCellCallbackStruct *sc =
        (XbaeMatrixSelectCellCallbackStruct *) call_data;

    XbaeMatrixSelectCell(w, sc->row, sc->column);
}

static void writeCB(Widget w, XtPointer client_data, XtPointer call_data)
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
        XCFREE(ep);
        return;
    }
    
    while (ep_tmp != NULL) {
        if (ep_tmp->next == ep) {
            ep_tmp->next = ep->next;
            XCFREE(ep);
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

EditPoints *get_unused_ep()
{
    EditPoints *ep_tmp = ep_start;

    while (ep_tmp != NULL) {
        if (XtIsManaged(GetParent(ep_tmp->top)) == False) {
            break;
        }
        ep_tmp = ep_tmp->next;
    }
    return ep_tmp;
}

void create_ss_frame(int gno, int setno)
{
    int i;
    char *collabels[MAX_SET_COLS + 1];
    short cwidths[MAX_SET_COLS + 1];
    int maxlengths[MAX_SET_COLS + 1];
    unsigned char column_label_alignments[MAX_SET_COLS + 1];
    char *label1[3] = {"Props...", "Update", "Close"};
    char *label2[2] = {"Delete", "Add"};
    EditPoints *ep;
    Widget dialog, fr, but1[3], but2[2];
    
    /* first, try a previously opened editor with the same set */
    ep = get_ep(gno, setno);
    if (ep == NULL) {
        /* if failed, a first unmanaged one */
        ep = get_unused_ep();
    }
    if (ep != NULL) {
        ep->gno = gno;
        ep->setno = setno;
        update_cells(ep);
        RaiseWindow(GetParent(ep->top));
        return;
    }
    
    set_wait_cursor();

    ep = xmalloc(sizeof(EditPoints));
    ep->next = ep_start;
    ep_start = ep;
    
    ep->gno = gno;
    ep->setno = setno;
    ep->ncols = dataset_cols(gno, setno);
    ep->scols = (get_set_strings(gno, setno) != NULL);
    ep->nrows = getsetlength(gno, setno);
    for (i = 0; i < ep->ncols; i++) {
        collabels[i] = copy_string(NULL, dataset_colname(i));
        cwidths[i] = CELL_WIDTH;
        maxlengths[i] = CELL_WIDTH;
        column_label_alignments[i] = XmALIGNMENT_CENTER;
        ep->cprec[i] = CELL_PREC;
        ep->cformat[i] = CELL_FORMAT;
    }
    if (ep->scols) {
        collabels[i] = copy_string(NULL, "String");
        cwidths[i] = CELL_WIDTH;
        maxlengths[i] = STRING_CELL_WIDTH;
        column_label_alignments[i] = XmALIGNMENT_CENTER;
    }

    ep->top = CreateDialogForm(app_shell, "Spreadsheet set editor");
    fr = CreateFrame(ep->top, NULL);
    AddDialogFormChild(ep->top, fr);
    ep->label = CreateLabel(fr, "");

    ep->mw = XtVaCreateManagedWidget("mw",
        xbaeMatrixWidgetClass, ep->top,
        XmNrows, ep->nrows,
        XmNcolumns, ep->ncols + ep->scols,
        XmNvisibleRows, 10,
        XmNvisibleColumns, 2,
        XmNcolumnWidths, cwidths,
        XmNcolumnMaxLengths, maxlengths,
        XmNcolumnLabels, collabels,
        XmNcolumnLabelAlignments, column_label_alignments,
        XmNallowColumnResize, True,
        XmNgridType, XmGRID_CELL_SHADOW,
        XmNcellShadowType, XmSHADOW_ETCHED_OUT,
        XmNcellShadowThickness, 2,
        XmNaltRowCount, 0,
        NULL);

    XtAddCallback(ep->mw, XmNselectCellCallback, selectCB, ep);	
    XtAddCallback(ep->mw, XmNdrawCellCallback, drawcellCB, ep);	
    XtAddCallback(ep->mw, XmNleaveCellCallback, leaveCB, ep);
    XtAddCallback(ep->mw, XmNwriteCellCallback, writeCB, ep);  

    AddDialogFormChild(ep->top, ep->mw);

    dialog = CreateVContainer(ep->top);
    AddDialogFormChild(ep->top, dialog);

    CreateCommandButtons(dialog, 2, but2, label2);
    XtAddCallback(but2[0], XmNactivateCallback, del_point_cb, (XtPointer) ep);
    XtAddCallback(but2[1], XmNactivateCallback, add_pt_cb, (XtPointer) ep);
    
    CreateSeparator(dialog);

    CreateCommandButtons(dialog, 3, but1, label1);
    XtAddCallback(but1[0], XmNactivateCallback, do_props_proc,
    	    (XtPointer) ep);
    XtAddCallback(but1[1], XmNactivateCallback, do_update_cells,
    	    (XtPointer) ep);
    XtAddCallback(but1[2], XmNactivateCallback, destroy_dialog,
    	    (XtPointer) GetParent(ep->top));

    ManageChild(ep->top);

    update_cells(ep);
    RaiseWindow(GetParent(ep->top));

    unset_wait_cursor();
}

/*
 * Start up external editor
 */
void do_ext_editor(int gno, int setno)
{
    char *fname, ebuf[256];
    FILE *cp;
    int save_autos;

    fname = tmpnam(NULL);
    cp = grace_openw(fname);
    if (cp == NULL) {
        return;
    }

    write_set(gno, setno, cp, sformat, FALSE);
    grace_close(cp);

    sprintf(ebuf, "%s %s", get_editor(), fname);
    system_wrap(ebuf);

    /* temporarily disable autoscale */
    save_autos = autoscale_onread;
    autoscale_onread = AUTOSCALE_NONE;
    if (is_set_active(gno, setno)) {
        curtype = dataset_type(gno, setno);
	killsetdata(gno, setno);	
    }
    getdata(gno, fname, SOURCE_DISK, LOAD_SINGLE);
    autoscale_onread = save_autos;
    unlink(fname);
    update_all();
    drawgraph();
}
