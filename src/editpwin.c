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
    int cformat[MAX_SET_COLS];
    int cprec[MAX_SET_COLS];
    int update;
    Widget top;
    Widget mw;
    Widget label;
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

/* minimum size of the spreadseet matrix */
#define MIN_SS_ROWS    10
#define MIN_SS_COLS    1

char *scformat[3] =
{"%.*lf", "%.*lg", "%.*le"};


int get_ep_set_dims(EditPoints *ep, int *nrows, int *ncols, int *scols)
{
    if (!ep || !is_valid_setno(ep->gno, ep->setno)) {
        return RETURN_FAILURE;
    }
    
    *nrows = getsetlength(ep->gno, ep->setno);
    *ncols = dataset_cols(ep->gno, ep->setno);
    if (get_set_strings(ep->gno, ep->setno) != NULL) {
        *scols = 1;
    } else {
        *scols = 0;
    }
    
    return RETURN_SUCCESS;
}


/*
 * delete the selected row
 */
void del_point_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
    int i, j;
    int nrows, ncols, scols;
    EditPoints *ep = (EditPoints *) client_data;

    XbaeMatrixGetCurrentCell(ep->mw, &i, &j);
    
    if (get_ep_set_dims(ep, &nrows, &ncols, &scols) != RETURN_SUCCESS) {
        return;
    }
    
    if (i >= nrows) {
        errmsg("Selected row out of range");
        return;
    }
    
    del_point(ep->gno, ep->setno, i);
    
    update_set_lists(ep->gno);
    update_cells(ep);
    
    xdrawgraph();
}


/*
 * add a point to a set by copying the selected cell and placing it after it
 */
void add_pt_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
    int i, j, k;
    int nrows, ncols, scols;
    char **s;
    Datapoint dpoint;
    EditPoints *ep = (EditPoints *) client_data;
    int gno = ep->gno, setno = ep->setno;

    XbaeMatrixGetCurrentCell(ep->mw, &i, &j);
    
    if (get_ep_set_dims(ep, &nrows, &ncols, &scols) != RETURN_SUCCESS) {
        return;
    }

    if (i > nrows || i < 0){
        errmsg("Selected row out of range");
        return;
    }
    
    zero_datapoint(&dpoint);
    
    if (i < nrows) {
        for (k = 0; k < ncols; k++) {
            dpoint.ex[k] = *(getcol(gno, setno, k) + i);
        }
        if ((s = get_set_strings(gno, setno)) != NULL) {
            dpoint.s = s[i];
        }
        add_point_at(gno, setno, i + 1, &dpoint);
    } else {
        add_point_at(gno, setno, i, &dpoint);
    }
    
    update_set_lists(gno);
    update_cells(ep);
    
    xdrawgraph();
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
    int nrows, ncols, scols;
    int changed = FALSE;
    EditPoints *ep = (EditPoints *) client_data;
    XbaeMatrixLeaveCellCallbackStruct *cs =
    	    (XbaeMatrixLeaveCellCallbackStruct *) calld;

    if (get_ep_set_dims(ep, &nrows, &ncols, &scols) != RETURN_SUCCESS) {
        return;
    }
    
    if (cs->column >= ncols + scols || cs->row >= nrows) {
        return;
    }
    
    /* TODO: add edit_point() function to setutils.c */
    if (cs->column < ncols) {
        char buf[128];
        double *datap = getcol(ep->gno, ep->setno, cs->column);
        sprintf(buf, scformat[(ep->cformat[cs->column])], ep->cprec[cs->column],
    	        datap[cs->row]);
        if (strcmp(buf, cs->value) != 0) {
	    datap[cs->row] = atof(cs->value);
            changed = TRUE;
        }
    } else if (cs->column < ncols + scols) {
        char **datap = get_set_strings(ep->gno, ep->setno);
        if (compare_strings(datap[cs->row], cs->value) == 0) {
	    datap[cs->row] = copy_string(datap[cs->row], cs->value);
            changed = TRUE;
        }
    }
    
    if (changed) {
        set_dirtystate();
        
        /* don't refresh this editor */
        ep->update = FALSE;
        update_set_lists(ep->gno);
        ep->update = TRUE;
        
        xdrawgraph();
    }
}


static void drawcellCB(Widget w, XtPointer client_data, XtPointer calld)
{
    int i, j;
    int ncols, nrows, scols;
    
    EditPoints *ep = (EditPoints *) client_data;
    XbaeMatrixDrawCellCallbackStruct *cs =
    	    (XbaeMatrixDrawCellCallbackStruct *) calld;

    i = cs->row;
    j = cs->column;

    if (get_ep_set_dims(ep, &nrows, &ncols, &scols) != RETURN_SUCCESS) {
        return;
    }

    cs->type = XbaeString;
    
    if (j >= ncols + scols || i >= nrows) {
        cs->string = "";
        return;
    }
    
    if (j < ncols) {
        static char buf[128];
        double *datap;
        datap = getcol(ep->gno, ep->setno, j);
        sprintf(buf, scformat[(ep->cformat[j])], ep->cprec[j], datap[i]);
        cs->string = buf;
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

void update_ss_editors(int gno)
{
    EditPoints *ep = ep_start;

    while (ep != NULL) {
        if (ep->gno == gno || gno == ALL_GRAPHS) {
            /* don't spend time on unmanaged SS editors */
            if (XtIsManaged(GetParent(ep->top))) {
                update_cells(ep);
            }
        } else if (!is_valid_gno(ep->gno)) {
            destroy_dialog_cb(GetParent(ep->top));
        }
        ep = ep->next;
    }
}

static void get_ep_dims(EditPoints *ep, int *nr, int *nc)
{
    XtVaGetValues(ep->mw, XmNrows, nr, XmNcolumns, nc, NULL);
}

/*
 * redo frame since number of data points or set type, etc.,  may change 
 */
void update_cells(EditPoints *ep)
{
    int i, nr, nc, new_nr, new_nc, delta_nr, delta_nc;
    int ncols, nrows, scols;
    short widths[MAX_SET_COLS + 1];
    int maxlengths[MAX_SET_COLS + 1];
    char *collabels[MAX_SET_COLS + 1];
    unsigned char column_label_alignments[MAX_SET_COLS + 1];
    short width;
    char buf[32];
    char **rowlabels;

    if (ep->update == FALSE) {
        return;
    }
    
    if (get_ep_set_dims(ep, &nrows, &ncols, &scols) != RETURN_SUCCESS) {
        destroy_dialog_cb(GetParent(ep->top));
        return;
    }
    
    sprintf(buf, "Dataset G%d.S%d", ep->gno, ep->setno);
    SetLabel(ep->label, buf);
	
    /* get current size of widget and update rows/columns as needed */
    get_ep_dims(ep, &nr, &nc);

    new_nc = MAX2(ncols + scols, MIN_SS_COLS);
    new_nr = MAX2(nrows, MIN_SS_ROWS);
    
    delta_nr = new_nr - nr;
    delta_nc = new_nc - nc;
    
    if (delta_nr == 0 && delta_nc == 0) {
        XbaeMatrixRefresh(ep->mw);
        return;
    }
    
    for (i = 0; i < ncols; i++) {
        widths[i] = CELL_WIDTH;
        maxlengths[i] = CELL_WIDTH;
        collabels[i] = copy_string(NULL, dataset_colname(i));
        column_label_alignments[i] = XmALIGNMENT_CENTER;
    }
    if (scols) {
        widths[i] = CELL_WIDTH;
        maxlengths[i] = STRING_CELL_WIDTH;
        collabels[i] = copy_string(NULL, "String");
        column_label_alignments[i] = XmALIGNMENT_CENTER;
    }

    if (delta_nr > 0) {
        rowlabels = xmalloc(delta_nr*sizeof(char *));
        for (i = 0; i < delta_nr; i++) {
    	    sprintf(buf, "%d", nr + i);
    	    rowlabels[i] = copy_string(NULL, buf);
        }
        XbaeMatrixAddRows(ep->mw, nr, NULL, rowlabels, NULL, delta_nr);
        for (i = 0; i < delta_nr; i++) {
	    xfree(rowlabels[i]);
        }
        xfree(rowlabels);
    } else if (delta_nr < 0) {
        XbaeMatrixDeleteRows(ep->mw, nrows, -delta_nr);
        if (nrows < MIN_SS_ROWS) {
            rowlabels = xmalloc(MIN_SS_ROWS*sizeof(char *));
            for (i = 0; i < MIN_SS_ROWS; i++) {
                sprintf(buf, "%d", i);
    	        rowlabels[i] = copy_string(NULL, buf);
            }
            XtVaSetValues(ep->mw, XmNrowLabels, rowlabels, NULL);
            for (i = 0; i < delta_nr; i++) {
	        xfree(rowlabels[i]);
            }
            xfree(rowlabels);
        }
    }
    
    if (delta_nc > 0) {
        XbaeMatrixAddColumns(ep->mw, 0, NULL, NULL, widths, maxlengths, 
            NULL, NULL, NULL, delta_nc);
    } else if (delta_nc < 0) {
        XbaeMatrixDeleteColumns(ep->mw, ncols, -delta_nc);
    }
		
    /* Adjust row label width */
    width = (short) ceil(log10(new_nr)) + 1;
    
    XtVaSetValues(ep->mw,
	XmNrowLabelWidth, width,
        XmNvisibleColumns, ncols + scols,
        XmNcolumnWidths, widths,
        XmNcolumnMaxLengths, maxlengths,
        XmNcolumnLabels, collabels,
        XmNcolumnLabelAlignments, column_label_alignments,
	NULL);

    /* free memory used to hold strings */
    for (i = 0; i < ncols + scols; i++) {
	xfree(collabels[i]);
    }
}

static EditPoints *new_ep(void)
{
    int i;
    short widths[MIN_SS_COLS];
    char *rowlabels[MIN_SS_ROWS];
    char *label1[3] = {"Props...", "Update", "Close"};
    char *label2[2] = {"Delete", "Add"};
    EditPoints *ep;
    Widget dialog, fr, but1[3], but2[2];
    
    ep = xmalloc(sizeof(EditPoints));
    ep->next = ep_start;
    ep_start = ep;
    
    ep->update = TRUE;
    
    for (i = 0; i < MAX_SET_COLS; i++) {
        ep->cprec[i] = CELL_PREC;
        ep->cformat[i] = CELL_FORMAT;
    }

    ep->top = CreateDialogForm(app_shell, "Spreadsheet dataset editor");
    fr = CreateFrame(ep->top, NULL);
    AddDialogFormChild(ep->top, fr);
    ep->label = CreateLabel(fr, "Dataset G*.S*");

    for (i = 0; i < MIN_SS_ROWS; i++) {
    	char buf[32];
        sprintf(buf, "%d", i);
    	rowlabels[i] = copy_string(NULL, buf);
    }
    for (i = 0; i < MIN_SS_COLS; i++) {
        widths[i] = CELL_WIDTH;
    }

    ep->mw = XtVaCreateManagedWidget("mw",
        xbaeMatrixWidgetClass, ep->top,
        XmNrows, MIN_SS_ROWS,
        XmNvisibleRows, MIN_SS_ROWS,
        XmNrowLabels, rowlabels,
        XmNcolumns, MIN_SS_COLS,
        XmNvisibleColumns, MIN_SS_COLS,
        XmNcolumnWidths, widths,
        XmNallowColumnResize, True,
        XmNgridType, XmGRID_CELL_SHADOW,
        XmNcellShadowType, XmSHADOW_ETCHED_OUT,
        XmNcellShadowThickness, 2,
        XmNaltRowCount, 0,
        NULL);

    for (i = 0; i < MIN_SS_ROWS; i++) {
	xfree(rowlabels[i]);
    }

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
    
    return ep;
}

void create_ss_frame(int gno, int setno)
{
    EditPoints *ep;

    set_wait_cursor();

    /* first, try a previously opened editor with the same set */
    ep = get_ep(gno, setno);
    if (ep == NULL) {
        /* if failed, a first unmanaged one */
        ep = get_unused_ep();
        /* if none available, create a new one */
        if (ep == NULL) {
            ep = new_ep();
        }
    }
    
    if (ep == NULL) {
        errmsg("Internal error in create_ss_frame()");
        unset_wait_cursor();
        return;
    }
    
    ep->gno = gno;
    ep->setno = setno;
    
    update_cells(ep);
    
    RaiseWindow(GetParent(ep->top));
    
    unset_wait_cursor();

    return;   
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
