/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2004 Grace Development Team
 * 
 * Maintained by Evgeny Stambulchik
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
 * setwin - GUI for operations on sets and datasets
 *
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>

#include <Xm/Xm.h>

#include <Xbae/Matrix.h>

#include "core_utils.h"
#include "utils.h"
#include "ssdata.h"
#include "parser.h"
#include "motifinc.h"
#include "protos.h"

#define DATA_STAT_COLS   6

static void enterCB(Widget w, XtPointer client_data, XtPointer call_data);
static void changetypeCB(StorageStructure *ss, int n, Quark **values, void *data);

static int datasetop_aac_cb(void *data);
static void datasetoptypeCB(OptionStructure *opt, int value, void *data);

static int leval_aac_cb(void *data);

typedef struct _Type_ui {
    Widget top;
    GraphSetStructure *sel;
    Widget mw;
    char *rows[MAX_SET_COLS][DATA_STAT_COLS];
} Type_ui;

static Type_ui tui;

void create_datasetprop_popup(Widget but, void *data)
{
    set_wait_cursor();

    if (tui.top == NULL) {
        Widget menubar, menupane, fr;
        int i, j;
        char *rowlabels[MAX_SET_COLS];
        char *collabels[DATA_STAT_COLS] =
            {"Min", "at", "Max", "at", "Mean", "Stdev"};
        short column_widths[DATA_STAT_COLS] = {12, 6, 12, 6, 12, 12};
        unsigned char column_alignments[DATA_STAT_COLS];
        unsigned char column_label_alignments[DATA_STAT_COLS];

	tui.top = CreateDialogForm(app_shell, "Data set statistics");

        menubar = CreateMenuBar(tui.top);
        ManageChild(menubar);
        AddDialogFormChild(tui.top, menubar);

	tui.sel = CreateGraphSetSelector(tui.top,
            "Data sets:", LIST_TYPE_SINGLE);
        AddDialogFormChild(tui.top, tui.sel->frame);
	AddStorageChoiceCB(tui.sel->set_sel, changetypeCB, tui.sel);


        menupane = CreateMenu(menubar, "File", 'F', FALSE);
        CreateMenuCloseButton(menupane, tui.top);

        menupane = CreateMenu(menubar, "Help", 'H', TRUE);
        CreateMenuHelpButton(menupane, "On data sets", 's',
            tui.top, "doc/UsersGuide.html#data-sets");

        for (i = 0; i < DATA_STAT_COLS; i++) {
            column_alignments[i] = XmALIGNMENT_END;
            column_label_alignments[i] = XmALIGNMENT_CENTER;
        }
        
        for (i = 0; i < MAX_SET_COLS; i++) {
            rowlabels[i] = copy_string(NULL, dataset_colname(i));
            for (j = 0; j < DATA_STAT_COLS; j++) {
                tui.rows[i][j] = NULL;
            }
        }

	fr = CreateFrame(tui.top, "Statistics");
        tui.mw = XtVaCreateManagedWidget("mw",
            xbaeMatrixWidgetClass, fr,
            XmNrows, MAX_SET_COLS,
            XmNcolumns, DATA_STAT_COLS,
            XmNvisibleRows, MAX_SET_COLS,
            XmNvisibleColumns, 4,
            XmNcolumnLabels, collabels,
            XmNcolumnWidths, column_widths,
            XmNcolumnAlignments, column_alignments,
            XmNcolumnLabelAlignments, column_label_alignments,
            XmNrowLabels, rowlabels,
	    XmNrowLabelWidth, 3,
            XmNrowLabelAlignment, XmALIGNMENT_CENTER,
            XmNshowArrows, True,
            XmNallowColumnResize, True,
            XmNgridType, XmGRID_COLUMN_SHADOW,
            XmNcellShadowType, XmSHADOW_OUT,
            XmNcellShadowThickness, 1,
            XmNaltRowCount, 1,
            XmNtraversalOn, False,
            NULL);

        XtAddCallback(tui.mw, XmNenterCellCallback, enterCB, NULL);	

        AddDialogFormChild(tui.top, fr);
        XtManageChild(tui.top);
    }
    
    RaiseWindow(GetParent(tui.top));
    unset_wait_cursor();
}

static void changetypeCB(StorageStructure *ss, int n, Quark **values, void *data)
{
    int i, j;
    double *datap;
    int imin, imax;
    double dmin, dmax, dmean, dsd;
    StorageStructure *sp;
    char buf[32];
    char **cells[MAX_SET_COLS];
    Quark *pset;
    
    sp = (StorageStructure *) data;
    if (sp == NULL) {
        return;
    }
    
    if (n == 1) {
	pset = values[0];
    } else {
	pset = NULL;
    }
    for (i = 0; i < MAX_SET_COLS; i++) {
        datap = set_get_col(pset, i);
	if (datap) {
            minmax(datap, set_get_length(pset), &dmin, &dmax, &imin, &imax);
	    stasum(datap, set_get_length(pset), &dmean, &dsd);
            for (j = 0; j < DATA_STAT_COLS; j++) {
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
            }
        } else {
            for (j = 0; j < DATA_STAT_COLS; j++) {
                tui.rows[i][j] = copy_string(tui.rows[i][j], "");
            }
        }
        cells[i] = &tui.rows[i][0];
    }
    XtVaSetValues(tui.mw, XmNcells, cells, NULL);
}

static void enterCB(Widget w, XtPointer client_data, XtPointer call_data)
{
    XbaeMatrixEnterCellCallbackStruct *cbs =
        (XbaeMatrixEnterCellCallbackStruct *) call_data;
    
    cbs->doit = False;
    cbs->map  = False;
}

typedef enum {
    DATASETOP_SORT,
    DATASETOP_REVERSE,
    DATASETOP_JOIN,
    DATASETOP_SPLIT,
    DATASETOP_DROP
}dataSetOpType;

typedef struct _Datasetop_ui {
    Widget top;
    StorageStructure *sel;
    OptionStructure *optype_item;
    OptionStructure *up_down_item;
    Widget length_item;
    Widget start_item;
    Widget stop_item;
} Datasetop_ui;

static Datasetop_ui datasetopui;

static Widget datasettype_controls[5];

void create_datasetop_popup(Widget but, void *data)
{
    Widget dialog, menubar, menupane, rc;

    set_wait_cursor();
    if (datasetopui.top == NULL) {
        
	datasetopui.top = CreateDialogForm(app_shell, "Data set operations");
        SetDialogFormResizable(datasetopui.top, TRUE);

        menubar = CreateMenuBar(datasetopui.top);
        ManageChild(menubar);
        AddDialogFormChild(datasetopui.top, menubar);
        

	dialog = CreateVContainer(datasetopui.top);
        XtVaSetValues(dialog, XmNrecomputeSize, True, NULL);

	datasetopui.sel = CreateSSDChoice(dialog,
            "Data sets:", LIST_TYPE_MULTIPLE);

        menupane = CreateMenu(menubar, "File", 'F', FALSE);
        CreateMenuButton(menupane,
            "Close", 'C', destroy_dialog_cb, GetParent(datasetopui.top));

        menupane = CreateMenu(menubar, "Help", 'H', TRUE);
        CreateMenuHelpButton(menupane, "On dataset operations", 's',
            datasetopui.top, "doc/UsersGuide.html#data-set-operations");

	datasetopui.optype_item =
            CreateOptionChoiceVA(dialog, "Operation type:",
            "Reverse",   DATASETOP_REVERSE, 
#if 0
            "Sort",      DATASETOP_SORT,    
            "Join",      DATASETOP_JOIN,    
            "Split",     DATASETOP_SPLIT,   
#endif
            "Drop rows", DATASETOP_DROP,    
            NULL);
   	AddOptionChoiceCB(datasetopui.optype_item, datasetoptypeCB, NULL);

	rc = CreateHContainer(dialog);
        XtVaSetValues(rc, XmNrecomputeSize, True, NULL);
	
	datasetopui.up_down_item = CreateOptionChoiceVA(rc, "Order:",
            "Ascending", FALSE,
            "Descending", TRUE,
            NULL);
        datasettype_controls[0] = rc;

	/* Reverse */
        rc = CreateVContainer(dialog);
        CreateSeparator(rc);
        datasettype_controls[1] = rc;

	/* Join */
	rc = CreateVContainer(dialog);
        CreateSeparator(rc);
        datasettype_controls[2] = rc;

	/* Split */
	rc = CreateVContainer(dialog);
        datasetopui.length_item = CreateTextItem(rc, 6, "Length:");
        datasettype_controls[3] = rc;

	/* Drop points */
	rc = CreateHContainer(dialog);
        datasetopui.start_item = CreateTextItem(rc, 6, "Start at:");
        datasetopui.stop_item  = CreateTextItem(rc, 6, "Stop at:");
        datasettype_controls[4] = rc;

	ManageChild(datasettype_controls[0]);
	UnmanageChild(datasettype_controls[1]);
	UnmanageChild(datasettype_controls[2]);
	UnmanageChild(datasettype_controls[3]);
	UnmanageChild(datasettype_controls[4]);

        CreateAACDialog(datasetopui.top, dialog, datasetop_aac_cb,
            &datasetopui);
    }
    
    RaiseWindow(GetParent(datasetopui.top));
    
    unset_wait_cursor();
}

static void datasetoptypeCB(OptionStructure *opt, int value, void *data)
{
    unsigned int i;
    dataSetOpType type = value;
    
    for (i = 0; i < 5; i++) {
        if (i == type) {
            ManageChild(datasettype_controls[i]);
        } else {
            UnmanageChild(datasettype_controls[i]);
        }
    }
}

static int datasetop_aac_cb(void *data)
{
    int n, i;
    int stype;
    int lpart;
    int startno, endno;
    dataSetOpType optype;
    Quark *ss, **selssd;
       
    n = GetStorageChoices(datasetopui.sel, &selssd);
    if (n < 1) {
        errmsg("No SSD selected");
        return RETURN_FAILURE;
    } else {
        optype = GetOptionChoice(datasetopui.optype_item);
 
        switch (optype) {
        case DATASETOP_REVERSE:
            for (i = 0; i < n; i++) {
                ss = selssd[i];
	        ssd_reverse(ss);
            }
            break;
#if 0
        case DATASETOP_SORT:
            stype = GetOptionChoice(datasetopui.up_down_item);

            for (i = 0; i < n; i++) {
                ss = selssd[i];
	        ssd_sort(ss, stype);
            }
            break;
        case DATASETOP_JOIN:
            ssd_join(selssd, n);
            break;
        case DATASETOP_SPLIT:
            xv_evalexpri(datasetopui.length_item, &lpart);
            for (i = 0; i < n; i++) {
                ss = selssd[i];
                ssd_split(ss, lpart);
            }
            break;
#endif
        case DATASETOP_DROP:
            xv_evalexpri(datasetopui.start_item, &startno);
            xv_evalexpri(datasetopui.stop_item, &endno);
            for (i = 0; i < n; i++) {
                ss = selssd[i];
		ssd_delete_rows(ss, startno, endno);
            }
            break;
        default:
            xfree(selssd);
            return RETURN_FAILURE;
        }
        
        snapshot_and_update(get_parent_project(selssd[0]), TRUE);
        
        xfree(selssd);

        return RETURN_SUCCESS;
    }
}


typedef struct _Leval_ui {
    Widget top;
    OptionStructure *set_type;
    Widget start;
    Widget stop;
    Widget npts;
    Widget mw;
    Quark *gr;
} Leval_ui;

void set_type_cb(OptionStructure *opt, int type, void *data)
{
    int i, nmrows, nscols;
    char *rowlabels[MAX_SET_COLS];
    Leval_ui *ui = (Leval_ui *) data;
    
    nmrows = XbaeMatrixNumRows(ui->mw);
    nscols = settype_cols(type);
    
    if (nmrows > nscols) {
        XbaeMatrixDeleteRows(ui->mw, nscols, nmrows - nscols);
    } else if (nmrows < nscols) {
	for (i = nmrows; i < nscols; i++) {
            rowlabels[i - nmrows] = copy_string(NULL, dataset_colname(i));
            rowlabels[i - nmrows] = concat_strings(rowlabels[i - nmrows], " = ");
        }
        XbaeMatrixAddRows(ui->mw, nmrows, NULL, rowlabels, NULL, nscols - nmrows);
    }
}

static Leval_ui levalui;

static void leaveCB(Widget w, XtPointer client_data, XtPointer call_data)
{
    Leval_ui *ui = (Leval_ui *) client_data;
    
    XbaeMatrixLeaveCellCallbackStruct *cs =
    	    (XbaeMatrixLeaveCellCallbackStruct *) call_data;

    XbaeMatrixSetCell(ui->mw, cs->row, cs->column, cs->value);
}

void create_leval_frame(Widget but, void *data)
{
    Quark *gr = (Quark *) data;

    if (!gr) {
        return;
    }
    
    set_wait_cursor();

    levalui.gr = gr;

    if (levalui.top == NULL) {
        int i;
        Widget fr, rc1;
        int nscols;
        char *rows[MAX_SET_COLS][1];
        char **cells[MAX_SET_COLS];
        char *rowlabels[MAX_SET_COLS];
        short column_widths[1] = {50};
        int column_maxlengths[1] = {256};
        RunTime *rt = rt_from_quark(gr);

	levalui.top = CreateDialogForm(app_shell, "Load & evaluate");

	fr = CreateFrame(levalui.top, "Parameter mesh ($t)");
        AddDialogFormChild(levalui.top, fr);
        rc1 = CreateHContainer(fr);
	levalui.start = CreateTextItem(rc1, 10, "Start at:");
	levalui.stop = CreateTextItem(rc1, 10, "Stop at:");
	levalui.npts = CreateTextItem(rc1, 6, "Length:");

	levalui.set_type = CreateSetTypeChoice(levalui.top, "Set type:");
        AddDialogFormChild(levalui.top, levalui.set_type->menu);
        AddOptionChoiceCB(levalui.set_type, set_type_cb, (void *) &levalui);
	
        nscols = settype_cols(rt->curtype);
	for (i = 0; i < nscols; i++) {
            rowlabels[i] = copy_string(NULL, dataset_colname(i));
            rowlabels[i] = concat_strings(rowlabels[i], " = ");
            if (i == 0) {
                rows[i][0] = "$t";
            } else {
                rows[i][0] = "";
            }
            cells[i] = &rows[i][0];
        }

        levalui.mw = XtVaCreateManagedWidget("mw",
            xbaeMatrixWidgetClass, levalui.top,
            XmNrows, nscols,
            XmNcolumns, 1,
            XmNvisibleRows, MAX_SET_COLS,
            XmNvisibleColumns, 1,
            XmNcolumnWidths, column_widths,
            XmNcolumnMaxLengths, column_maxlengths,
            XmNrowLabels, rowlabels,
	    XmNrowLabelWidth, 6,
            XmNrowLabelAlignment, XmALIGNMENT_CENTER,
            XmNcells, cells,
            XmNgridType, XmGRID_CELL_SHADOW,
            XmNcellShadowType, XmSHADOW_ETCHED_OUT,
            XmNcellShadowThickness, 2,
            XmNaltRowCount, 0,
            XmNallowColumnResize, True,
            NULL);

        XtAddCallback(levalui.mw, XmNleaveCellCallback, leaveCB, &levalui);
        
        CreateAACDialog(levalui.top, levalui.mw, leval_aac_cb, &levalui);
    }
    
    RaiseWindow(GetParent(levalui.top));
    unset_wait_cursor();
}

static int leval_aac_cb(void *data)
{
    int i, nscols, type;
    double start, stop;
    int npts;
    char *formula[MAX_SET_COLS];
    Quark *pset, *gr;
    grarr *t;
    Leval_ui *ui = (Leval_ui *) data;
    
    gr = ui->gr;
    type = GetOptionChoice(ui->set_type);
    nscols = settype_cols(type);

    if (xv_evalexpr(ui->start, &start) != RETURN_SUCCESS) {
	errmsg("Start item undefined");
        return RETURN_FAILURE;
    }

    if (xv_evalexpr(ui->stop, &stop) != RETURN_SUCCESS) {
	errmsg("Stop item undefined");
        return RETURN_FAILURE;
    }

    if (xv_evalexpri(ui->npts, &npts) != RETURN_SUCCESS) {
	errmsg("Number of points undefined");
        return RETURN_FAILURE;
    }

    XbaeMatrixCommitEdit(ui->mw, False);
    for (i = 0; i < nscols; i++) {
        formula[i] = XbaeMatrixGetCell(ui->mw, i, 0);
    }
    
    t = get_parser_arr_by_name("$t");
    if (t == NULL) {
        t = define_parser_arr("$t");
        if (t == NULL) {
	    errmsg("Internal error");
            return RETURN_FAILURE;
        }
    }
    
    if (t->length != 0) {
        xfree(t->data);
        t->length = 0;
    }
    t->data = allocate_mesh(start, stop, npts);
    if (t->data == NULL) {
        return RETURN_FAILURE;
    }
    t->length = npts;
    
    pset = grace_set_new(gr);
    set_set_type(pset, type);
    quark_set_active(pset, TRUE);
    if (set_set_length(pset, npts) != RETURN_SUCCESS) {
        quark_free(pset);
        XCFREE(t->data);
        t->length = 0;
        return RETURN_FAILURE;
    }
    
    set_parser_setno(pset);

    for (i = 0; i < nscols; i++) {
        char buf[32], *expr;
        int res;
        
        /* preparing the expression */
        sprintf(buf, "%s = ", dataset_colname(i));
        expr = copy_string(NULL, buf);
        expr = concat_strings(expr, formula[i]);
        
        /* evaluate the expression */
        res = scanner(expr);
        
        xfree(expr);
        
        if (res != RETURN_SUCCESS) {
            quark_free(pset);
            
            XCFREE(t->data);
            t->length = 0;
            
            return RETURN_FAILURE;
        }
    }

    XCFREE(t->data);
    t->length = 0;

    set_set_comment(pset, "Formula");
    
    update_set_lists(gr);
    
    return RETURN_SUCCESS;
}
