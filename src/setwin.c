/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2003 Grace Development Team
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
 * setwin - GUI for operations on sets and datasets
 *
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>

#include <Xm/Xm.h>

#include <Xbae/Matrix.h>

#include "globals.h"
#include "graphs.h"
#include "utils.h"
#include "plotone.h"
#include "ssdata.h"
#include "parser.h"
#include "motifinc.h"
#include "protos.h"

static void enterCB(Widget w, XtPointer client_data, XtPointer call_data);
static void changetypeCB(int n, void **values, void *data);
static int datasetprop_aac_cb(void *data);
static void create_hotfiles_popup(void *data);

static int datasetop_aac_cb(void *data);
static void datasetoptypeCB(int value, void *data);

static int leval_aac_cb(void *data);

typedef struct _Type_ui {
    Widget top;
    StorageStructure *sel;
    TextStructure *comment_item;
    Widget length_item;
    OptionStructure *datatype_item;
    Widget hotlink_item;
    OptionStructure *hotsrc_item;
    Widget hotfile_item;
    Widget mw;
    char *rows[MAX_SET_COLS][6];
} Type_ui;

static Type_ui tui;

void create_datasetprop_popup(void *data)
{
    set_wait_cursor();

    if (tui.top == NULL) {
        Widget menubar, menupane, submenupane, dialog, rc, rc1, fr, wbut;
        int i, j;
        char *rowlabels[MAX_SET_COLS];
        char *collabels[6] = {"Min", "at", "Max", "at", "Mean", "Stdev"};
        short column_widths[6] = {10, 6, 10, 6, 10, 10};
        unsigned char column_alignments[6];
        unsigned char column_label_alignments[6];
        OptionItem optype_items[] = {
            {SOURCE_DISK,  "File"},
            {SOURCE_PIPE,  "Pipe"}
        };

	tui.top = CreateDialogForm(app_shell, "Data set properties");

        menubar = CreateMenuBar(tui.top);
        ManageChild(menubar);
        AddDialogFormChild(tui.top, menubar);

	dialog = CreateVContainer(tui.top);

	tui.sel = CreateSetChoice(dialog,
            "Data sets:", LIST_TYPE_MULTIPLE, NULL);
	AddStorageChoiceCB(tui.sel, changetypeCB, (void *) tui.sel);


        menupane = CreateMenu(menubar, "File", 'F', FALSE);
        CreateMenuButton(menupane,
            "Close", 'C', destroy_dialog_cb, GetParent(tui.top));

        menupane = CreateMenu(menubar, "Edit", 'E', FALSE);
#if 0
        CreateMenuButton(menupane, "Duplicate", 'D',
            duplicate_set_proc, (void *) tui.sel);
        CreateMenuButton(menupane, "Kill data", 'a',
            killd_set_proc, (void *) tui.sel);
        CreateMenuSeparator(menupane);
        submenupane = CreateMenu(menupane, "Edit data", 'E', FALSE);
        CreateMenuButton(submenupane, "In spreadsheet", 's',
            editS_set_proc, (void *) tui.sel);
        CreateMenuButton(submenupane, "In text editor", 'e',
            editE_set_proc, (void *) tui.sel);
        submenupane = CreateMenu(menupane, "Create new", 'n', FALSE);
        CreateMenuButton(submenupane, "By formula", 'f',
            newF_set_proc, (void *) tui.sel);
        CreateMenuButton(submenupane, "In spreadsheet", 's',
            newS_set_proc, (void *) tui.sel);
        CreateMenuButton(submenupane, "In text editor", 'e',
            newE_set_proc, (void *) tui.sel);
        CreateMenuButton(submenupane, "From block data", 'b',
            newB_set_proc, (void *) tui.sel);
        CreateMenuSeparator(menupane);
        CreateMenuButton(menupane, "Set appearance...", 'S',
            define_symbols_popup, (void *) -1);
#endif 
        menupane = CreateMenu(menubar, "Help", 'H', TRUE);
        CreateMenuHelpButton(menupane, "On data sets", 's',
            tui.top, "doc/UsersGuide.html#data-sets");

	fr = CreateFrame(dialog, NULL);
	rc = CreateVContainer(fr);
	rc1 = CreateHContainer(rc);
	tui.datatype_item = CreateSetTypeChoice(rc1, "Type:");
	tui.length_item = CreateTextItem2(rc1, 6, "Length:");
	tui.comment_item = CreateTextInput(rc, "Comment:");

	fr = CreateFrame(dialog, "Hotlink");
	rc = CreateVContainer(fr);
	rc1 = CreateHContainer(rc);
        tui.hotlink_item = CreateToggleButton(rc1, "Enabled");
        tui.hotsrc_item  = CreateOptionChoice(rc1,
            "Source type: ", 1, 2, optype_items);
	rc1 = CreateHContainer(rc);
	tui.hotfile_item = CreateTextItem2(rc1, 20, "File name:");
	wbut = CreateButton(rc1, "Browse...");
	AddButtonCB(wbut, create_hotfiles_popup, &tui);

        for (i = 0; i < 6; i++) {
            column_alignments[i] = XmALIGNMENT_END;
            column_label_alignments[i] = XmALIGNMENT_CENTER;
        }
        
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

        CreateAACDialog(tui.top, dialog, datasetprop_aac_cb, tui.sel);
    }
    
    RaiseWindow(GetParent(tui.top));
    unset_wait_cursor();
}

static void changetypeCB(int n, void **values, void *data)
{
    int i, j, ncols;
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
        ncols = dataset_cols(pset);
        SetTextString(tui.comment_item, getcomment(pset));
	sprintf(buf, "%d", getsetlength(pset));
        xv_setstr(tui.length_item, buf);
        SetOptionChoice(tui.datatype_item, dataset_type(pset));
        SetToggleButtonState(tui.hotlink_item, is_hotlinked(pset));
        SetOptionChoice(tui.hotsrc_item, get_hotlink_src(pset));
        xv_setstr(tui.hotfile_item, get_hotlink_file(pset));
    } else {
	pset = NULL;
        ncols = 0;
    }
    for (i = 0; i < MAX_SET_COLS; i++) {
        datap = getcol(pset, i);
	minmax(datap, getsetlength(pset), &dmin, &dmax, &imin, &imax);
	stasum(datap, getsetlength(pset), &dmean, &dsd);
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

static void enterCB(Widget w, XtPointer client_data, XtPointer call_data)
{
    XbaeMatrixEnterCellCallbackStruct *cbs =
        (XbaeMatrixEnterCellCallbackStruct *) call_data;
    
    cbs->doit = False;
    cbs->map  = False;
}

/*
 * change dataset properties
 */
static int datasetprop_aac_cb(void *data)
{
    int error = FALSE;
    int nsets, i, len, type, hotlink, hotsrc;
    char *s, *hotfile;
    Quark *pset, **selset;
    
    nsets = GetStorageChoices(tui.sel, (void ***) &selset);
    
    if (nsets < 1) {
        errmsg("No set selected");
        return RETURN_FAILURE;
    } else {
        type = GetOptionChoice(tui.datatype_item);
        xv_evalexpri(tui.length_item, &len);
        if (len < 0) {
            errmsg("Negative set length!");
            error = TRUE;
        }
        
        hotlink = GetToggleButtonState(tui.hotlink_item);
        hotsrc  = GetOptionChoice(tui.hotsrc_item);
        hotfile = xv_getstr(tui.hotfile_item);
 
        s = GetTextString(tui.comment_item);
        
        if (error == FALSE) {
            for (i = 0; i < nsets; i++) {
                pset = selset[i];
                set_dataset_type(pset, type);
                setlength(pset, len);
                setcomment(pset, s);
                set_hotlink(pset, hotlink, hotfile, hotsrc);
            }
        }
 
        xfree(s);
        xfree(selset);

        if (error == FALSE) {
            update_set_lists(get_set_choice_gr((StorageStructure *) data));
            xdrawgraph();
            return RETURN_SUCCESS;
        } else {
            return RETURN_FAILURE;
        }
    }
}

static int do_hotlinkfile_proc(char *filename, void *data)
{
    Type_ui *ui = (Type_ui *) data;
    
    xv_setstr(ui->hotfile_item, filename);
    
    return TRUE;
}

/*
 * create file selection pop up to choose the file for hotlink
 */
static void create_hotfiles_popup(void *data)
{
    static FSBStructure *fsb = NULL;

    set_wait_cursor();

    if (fsb == NULL) {
        fsb = CreateFileSelectionBox(app_shell, "Hotlinked file");
	AddFileSelectionBoxCB(fsb, do_hotlinkfile_proc, data);
        ManageChild(fsb->FSB);
    }
    
    RaiseWindow(fsb->dialog);

    unset_wait_cursor();
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
    OptionStructure *xy_item;
    OptionStructure *up_down_item;
    Widget length_item;
    Widget start_item;
    Widget stop_item;
} Datasetop_ui;

static Datasetop_ui datasetopui;

static Widget datasettype_controls[5];

void create_datasetop_popup(void *data)
{
    Widget dialog, menubar, menupane, rc;
    OptionItem optype_items[5];

    set_wait_cursor();
    if (datasetopui.top == NULL) {
        optype_items[0].value = DATASETOP_SORT;
        optype_items[0].label = "Sort";
        optype_items[1].value = DATASETOP_REVERSE;
        optype_items[1].label = "Reverse";
        optype_items[2].value = DATASETOP_JOIN;
        optype_items[2].label = "Join";
        optype_items[3].value = DATASETOP_SPLIT;
        optype_items[3].label = "Split";
        optype_items[4].value = DATASETOP_DROP;
        optype_items[4].label = "Drop points";
        
	datasetopui.top = CreateDialogForm(app_shell, "Data set operations");
        SetDialogFormResizable(datasetopui.top, TRUE);

        menubar = CreateMenuBar(datasetopui.top);
        ManageChild(menubar);
        AddDialogFormChild(datasetopui.top, menubar);
        

	dialog = CreateVContainer(datasetopui.top);
        XtVaSetValues(dialog, XmNrecomputeSize, True, NULL);

	datasetopui.sel = CreateSetChoice(dialog,
            "Data sets:", LIST_TYPE_MULTIPLE, NULL);

        menupane = CreateMenu(menubar, "File", 'F', FALSE);
        CreateMenuButton(menupane,
            "Close", 'C', destroy_dialog_cb, GetParent(datasetopui.top));

        menupane = CreateMenu(menubar, "Help", 'H', TRUE);
        CreateMenuHelpButton(menupane, "On dataset operations", 's',
            datasetopui.top, "doc/UsersGuide.html#data-set-operations");

	datasetopui.optype_item = CreateOptionChoice(dialog,
						"Operation type:",
						1, 5, optype_items);
   	AddOptionChoiceCB(datasetopui.optype_item, datasetoptypeCB, NULL);

	rc = CreateHContainer(dialog);
        XtVaSetValues(rc, XmNrecomputeSize, True, NULL);
	
        datasetopui.xy_item = CreatePanelChoice(rc,
					   "Sort on:",
					   7,
					   "X",
					   "Y",
					   "Y1",
					   "Y2",
					   "Y3",
					   "Y4",
					   0, 0);
	datasetopui.up_down_item = CreatePanelChoice(rc,
						"Order:",
						3,
						"Ascending",
						"Descending", 0,
						0);
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
        datasetopui.length_item = CreateTextItem2(rc, 6, "Length:");
        datasettype_controls[3] = rc;

	/* Drop points */
	rc = CreateHContainer(dialog);
        datasetopui.start_item = CreateTextItem2(rc, 6, "Start at:");
        datasetopui.stop_item  = CreateTextItem2(rc, 6, "Stop at:");
        datasettype_controls[4] = rc;

	ManageChild(datasettype_controls[0]);
	UnmanageChild(datasettype_controls[1]);
	UnmanageChild(datasettype_controls[2]);
	UnmanageChild(datasettype_controls[3]);
	UnmanageChild(datasettype_controls[4]);

        CreateAACDialog(datasetopui.top, dialog, datasetop_aac_cb, datasetopui.sel);
    }
    
    RaiseWindow(GetParent(datasetopui.top));
    
    unset_wait_cursor();
}

static void datasetoptypeCB(int value, void *data)
{
    int i;
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
    int nsets, i;
    int sorton, stype;
    int lpart;
    int startno, endno;
    static int son[MAX_SET_COLS] = {DATA_X, DATA_Y, DATA_Y1, DATA_Y2, DATA_Y3, DATA_Y4};
    dataSetOpType optype;
    Quark *pset, **selset;
       
    nsets = GetStorageChoices(datasetopui.sel, (void ***) &selset);
    if (nsets < 1) {
        errmsg("No set selected");
        return RETURN_FAILURE;
    } else {
        optype = GetOptionChoice(datasetopui.optype_item);
 
        switch (optype) {
        case DATASETOP_SORT:
            sorton = son[GetOptionChoice(datasetopui.xy_item)];
            stype = GetOptionChoice(datasetopui.up_down_item);

            for (i = 0; i < nsets; i++) {
                pset = selset[i];
	        do_sort(pset, sorton, stype);
            }
            break;
        case DATASETOP_REVERSE:
            for (i = 0; i < nsets; i++) {
                pset = selset[i];
	        reverse_set(pset);
            }
            break;
        case DATASETOP_JOIN:
            join_sets(selset, nsets);
            break;
        case DATASETOP_SPLIT:
            xv_evalexpri(datasetopui.length_item, &lpart);
            for (i = 0; i < nsets; i++) {
                pset = selset[i];
                do_splitsets(pset, lpart);
            }
            break;
        case DATASETOP_DROP:
            xv_evalexpri(datasetopui.start_item, &startno);
            xv_evalexpri(datasetopui.stop_item, &endno);
            for (i = 0; i < nsets; i++) {
                pset = selset[i];
		do_drop_points(pset, startno, endno);
            }
            break;
        }
        
        xfree(selset);

        update_set_lists(get_set_choice_gr((StorageStructure *) data));
        xdrawgraph();
        
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

void set_type_cb(int type, void *data)
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

void create_leval_frame(void *data)
{
    Quark *gr = (Quark *) data;

    set_wait_cursor();

    if (gr) {
        levalui.gr = gr;
    } else {
        levalui.gr = graph_get_current(grace->project);
    }

    if (levalui.top == NULL) {
        int i;
        Widget fr, rc1;
        int nscols;
        char *rows[MAX_SET_COLS][1];
        char **cells[MAX_SET_COLS];
        char *rowlabels[MAX_SET_COLS];
        short column_widths[1] = {50};
        int column_maxlengths[1] = {256};

	levalui.top = CreateDialogForm(app_shell, "Load & evaluate");

	fr = CreateFrame(levalui.top, "Parameter mesh ($t)");
        AddDialogFormChild(levalui.top, fr);
        rc1 = CreateHContainer(fr);
	levalui.start = CreateTextItem2(rc1, 10, "Start at:");
	levalui.stop = CreateTextItem2(rc1, 10, "Stop at:");
	levalui.npts = CreateTextItem2(rc1, 6, "Length:");

	levalui.set_type = CreateSetTypeChoice(levalui.top, "Set type:");
        AddDialogFormChild(levalui.top, levalui.set_type->menu);
        AddOptionChoiceCB(levalui.set_type, set_type_cb, (void *) &levalui);
	
        nscols = settype_cols(grace->rt->curtype);
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
    
    pset = set_new(gr);
    set_dataset_type(pset, type);
    set_set_hidden(pset, FALSE);
    if (setlength(pset, npts) != RETURN_SUCCESS) {
        killset(pset);
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
            killset(pset);
            
            XCFREE(t->data);
            t->length = 0;
            
            return RETURN_FAILURE;
        }
    }

    XCFREE(t->data);
    t->length = 0;

    setcomment(pset, "Formula");
    
    update_set_lists(gr);
    xdrawgraph();
    
    return RETURN_SUCCESS;
}
