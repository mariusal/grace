/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-95 Paul J Turner, Portland, OR
 * Copyright (c) 1996-99 Grace Development Team
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
#include <Xm/Form.h>
#include <Xm/DialogS.h>
#include <Xm/Label.h>
#include <Xm/RowColumn.h>

#include <Xbae/Matrix.h>

#include "globals.h"
#include "graphs.h"
#include "utils.h"
#include "plotone.h"
#include "ssdata.h"
#include "parser.h"
#include "motifinc.h"
#include "protos.h"

#define cg get_cg()

static void enterCB(Widget w, XtPointer client_data, XtPointer call_data);
static void changetypeCB(int n, int *values, void *data);
static void datasetprop_aac_cb(void *data);

static void datasetop_aac_cb(void *data);
static void datasetoptypeCB(int value, void *data);
static void setop_aac_cb(void *data);

static void leval_aac_cb(void *data);

typedef struct _Type_ui {
    Widget top;
    ListStructure *sel;
    Widget comment_item;
    Widget length_item;
    OptionStructure *datatype_item;
    Widget mw;
    char *rows[MAX_SET_COLS][6];
} Type_ui;

static Type_ui tui;

void create_datasetprop_popup(void *data)
{
    Widget panel, menubar, menupane, submenupane, dialog, rc, fr;
    int i, j;

    set_wait_cursor();
    if (tui.top == NULL) {
        char *rowlabels[MAX_SET_COLS];
        char *collabels[6] = {"Min", "at", "Max", "at", "Mean", "Stdev"};
        short column_widths[6] = {10, 6, 10, 6, 10, 10};
        unsigned char column_alignments[6] = {
                                                XmALIGNMENT_END,
                                                XmALIGNMENT_END,
                                                XmALIGNMENT_END,
                                                XmALIGNMENT_END,
                                                XmALIGNMENT_END,
                                                XmALIGNMENT_END
                                             };
        unsigned char column_label_alignments[6] = {
                                                      XmALIGNMENT_CENTER,
                                                      XmALIGNMENT_CENTER,
                                                      XmALIGNMENT_CENTER,
                                                      XmALIGNMENT_CENTER,
                                                      XmALIGNMENT_CENTER,
                                                      XmALIGNMENT_CENTER
                                                   };
	tui.top = XmCreateDialogShell(app_shell, "dataSetProps", NULL, 0);
	handle_close(tui.top);
        panel = XtVaCreateWidget("dataSetPanel",
            xmFormWidgetClass, tui.top, NULL, 0);

        menubar = CreateMenuBar(panel);
        
        XtManageChild(menubar);
        XtVaSetValues(menubar,
                      XmNtopAttachment, XmATTACH_FORM,
                      XmNleftAttachment, XmATTACH_FORM,
                      XmNrightAttachment, XmATTACH_FORM,
                      NULL);

	dialog = XmCreateRowColumn(panel, "dialog_rc", NULL, 0);

	tui.sel = CreateSetChoice(dialog,
            "Data sets:", LIST_TYPE_MULTIPLE, TRUE);
	AddListChoiceCB(tui.sel, changetypeCB, (void *) tui.sel);


        menupane = CreateMenu(menubar, "File", 'F', FALSE);
        CreateMenuButton(menupane, "Close", 'C', datasetprop_aac_cb, (void *) AAC_CLOSE);

        menupane = CreateMenu(menubar, "Edit", 'E', FALSE);

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
        CreateMenuButton(menupane, "Set operations...", 'o',
            create_setop_popup, NULL);
 
        menupane = CreateMenu(menubar, "Help", 'H', TRUE);
        CreateMenuButton(menupane, "On data sets", 's', HelpCB, NULL);

	rc = XmCreateRowColumn(dialog, "rc", NULL, 0);
        XtVaSetValues(rc, XmNorientation, XmHORIZONTAL, NULL);
	tui.datatype_item = CreateSetTypeChoice(rc, "Type:");
	tui.length_item = CreateTextItem2(rc, 6, "Length:");
        XtManageChild(rc);
	tui.comment_item = CreateTextItem2(dialog, 26, "Comment:");

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

/*
 *         XtUninstallTranslations(tui.mw);
 */
        XtAddCallback(tui.mw, XmNenterCellCallback, enterCB, NULL);	

	XtManageChild(dialog);
        XtVaSetValues(dialog,
            XmNtopAttachment, XmATTACH_WIDGET,
            XmNtopWidget, menubar,
            XmNleftAttachment, XmATTACH_FORM,
            XmNrightAttachment, XmATTACH_FORM,
            NULL);

	fr = CreateFrame(panel, NULL);
        CreateAACButtons(fr, panel, datasetprop_aac_cb);
        XtVaSetValues(fr,
            XmNtopAttachment, XmATTACH_WIDGET,
            XmNtopWidget, dialog,
            XmNleftAttachment, XmATTACH_FORM,
            XmNrightAttachment, XmATTACH_FORM,
            XmNbottomAttachment, XmATTACH_FORM,
            NULL);

	XtManageChild(panel);
    }
    XtRaise(tui.top);
    unset_wait_cursor();
}

static void changetypeCB(int n, int *values, void *data)
{
    int i, j, ncols;
    double *datap;
    int imin, imax;
    double dmin, dmax, dmean, dsd;
    ListStructure *listp;
    SetChoiceData *sdata;
    int gno, setno;
    char buf[32];
    char **cells[MAX_SET_COLS];
    
    listp = (ListStructure *) data;
    if (listp == NULL) {
        return;
    }
    
    sdata = (SetChoiceData *) listp->anydata;
    gno = sdata->gno;
    
    if (n == 1 && is_set_active(gno, setno = values[0]) == TRUE) {
	ncols = dataset_cols(gno, setno);
        xv_setstr(tui.comment_item, getcomment(gno, setno));
	sprintf(buf, "%d", getsetlength(gno, setno));
        xv_setstr(tui.length_item, buf);
        SetOptionChoice(tui.datatype_item, dataset_type(gno, setno));
    } else {
	setno = -1;
        ncols = 0;
    }
    for (i = 0; i < MAX_SET_COLS; i++) {
        datap = getcol(gno, setno, i);
	minmax(datap, getsetlength(gno, setno), &dmin, &dmax, &imin, &imax);
	stasum(datap, getsetlength(gno, setno), &dmean, &dsd);
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
static void datasetprop_aac_cb(void *data)
{
    int aac_mode, error = FALSE;
    int *selset, nsets, i, len, setno, type;
    char *s;
    
    aac_mode = (int) data;
    if (aac_mode == AAC_CLOSE) {
        XtUnmanageChild(tui.top);
        return;
    }

    nsets = GetListChoices(tui.sel, &selset);
    if (nsets < 1) {
        errmsg("No set selected");
        return;
    } else {
        set_wait_cursor();
 
        type = GetOptionChoice(tui.datatype_item);
        xv_evalexpri(tui.length_item, &len);
        if (len < 0) {
            errmsg("Negative set length!");
            error = TRUE;
        }
        s = xv_getstr(tui.comment_item);
        
 
        if (error == FALSE) {
            for (i = 0; i < nsets; i++) {
                setno = selset[i];
                set_dataset_type(cg, setno, type);
                setlength(cg, setno, len);
                setcomment(cg, setno, s);
            }
        }
 
        if (aac_mode == AAC_ACCEPT && error == FALSE) {
            XtUnmanageChild(tui.top);
        }
        
        free(selset);

        update_set_lists(cg);
        unset_wait_cursor();
        drawgraph();
    }
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
    ListStructure *sel;
    OptionStructure *optype_item;
    Widget *xy_item;
    Widget *up_down_item;
    Widget length_item;
    Widget start_item;
    Widget stop_item;
} Datasetop_ui;

static Datasetop_ui datasetopui;

static Widget datasettype_controls[5];

void create_datasetop_popup(void *data)
{
    Widget dialog, panel, menubar, menupane, rc, fr;
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
        
	datasetopui.top = XmCreateDialogShell(app_shell, "dataSetOperations", NULL, 0);
        XtVaSetValues(datasetopui.top, XmNallowShellResize, True, NULL);
	handle_close(datasetopui.top);
        panel = XtVaCreateWidget("dataSetPanel",
            xmFormWidgetClass, datasetopui.top, NULL, 0);
        XtVaSetValues(panel, XmNresizePolicy, XmRESIZE_ANY, NULL);

        menubar = CreateMenuBar(panel);
        
        XtManageChild(menubar);
        XtVaSetValues(menubar,
                      XmNtopAttachment, XmATTACH_FORM,
                      XmNleftAttachment, XmATTACH_FORM,
                      XmNrightAttachment, XmATTACH_FORM,
                      NULL);

	dialog = XmCreateRowColumn(panel, "dialog_rc", NULL, 0);
        XtVaSetValues(dialog, XmNrecomputeSize, True, NULL);

	datasetopui.sel = CreateSetChoice(dialog,
            "Data sets:", LIST_TYPE_MULTIPLE, TRUE);

        menupane = CreateMenu(menubar, "File", 'F', FALSE);
        CreateMenuButton(menupane, "Close", 'C',
            datasetop_aac_cb, (void *) AAC_CLOSE);


        menupane = CreateMenu(menubar, "Help", 'H', TRUE);
        CreateMenuButton(menupane, "On dataset operations", 's',
            HelpCB, NULL);

	datasetopui.optype_item = CreateOptionChoice(dialog,
						"Operation type:",
						1, 5, optype_items);
   	AddOptionChoiceCB(datasetopui.optype_item, datasetoptypeCB, NULL);

	rc = XmCreateRowColumn(dialog, "rc", NULL, 0);
        XtVaSetValues(rc, XmNorientation, XmHORIZONTAL, NULL);
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
        rc = XmCreateRowColumn(dialog, "rc", NULL, 0);
        CreateSeparator(rc);
        datasettype_controls[1] = rc;

	/* Join */
	rc = XmCreateRowColumn(dialog, "rc", NULL, 0);
        CreateSeparator(rc);
        datasettype_controls[2] = rc;

	/* Split */
	rc = XmCreateRowColumn(dialog, "rc", NULL, 0);
        datasetopui.length_item = CreateTextItem2(rc, 6, "Length:");
        datasettype_controls[3] = rc;

	/* Drop points */
	rc = XmCreateRowColumn(dialog, "rc", NULL, 0);
        XtVaSetValues(rc, XmNorientation, XmHORIZONTAL, NULL);
        datasetopui.start_item = CreateTextItem2(rc, 6, "Start at:");
        datasetopui.stop_item  = CreateTextItem2(rc, 6, "Stop at:");
        datasettype_controls[4] = rc;

	XtManageChild(datasettype_controls[0]);

	XtManageChild(dialog);
        XtVaSetValues(dialog,
            XmNtopAttachment, XmATTACH_WIDGET,
            XmNtopWidget, menubar,
            XmNleftAttachment, XmATTACH_FORM,
            XmNrightAttachment, XmATTACH_FORM,
            NULL);

	fr = CreateFrame(panel, NULL);
        CreateAACButtons(fr, panel, datasetop_aac_cb);
        XtVaSetValues(fr,
            XmNtopAttachment, XmATTACH_WIDGET,
            XmNtopWidget, dialog,
            XmNleftAttachment, XmATTACH_FORM,
            XmNrightAttachment, XmATTACH_FORM,
            XmNbottomAttachment, XmATTACH_FORM,
            NULL);

	XtManageChild(panel);
    }
    XtRaise(datasetopui.top);
    unset_wait_cursor();
}

static void datasetoptypeCB(int value, void *data)
{
    int i;
    dataSetOpType type = value;
    
    for (i = 0; i < 5; i++) {
        if (i == type) {
            XtManageChild(datasettype_controls[i]);
        } else {
            XtUnmanageChild(datasettype_controls[i]);
        }
    }
}

static void datasetop_aac_cb(void *data)
{
    int aac_mode, error = FALSE;
    int *selset, nsets, i, setno;
    int sorton, stype;
    int lpart;
    int startno, endno;
    static int son[MAX_SET_COLS] = {DATA_X, DATA_Y, DATA_Y1, DATA_Y2, DATA_Y3, DATA_Y4};
    dataSetOpType optype;
    
    aac_mode = (int) data;
    if (aac_mode == AAC_CLOSE) {
        XtUnmanageChild(datasetopui.top);
        return;
    }
    
    nsets = GetListChoices(datasetopui.sel, &selset);
    if (nsets < 1) {
        errmsg("No set selected");
        return;
    } else {
        set_wait_cursor();
 
        optype = GetOptionChoice(datasetopui.optype_item);
 
        switch (optype) {
        case DATASETOP_SORT:
            sorton = son[GetChoice(datasetopui.xy_item)];
            stype = GetChoice(datasetopui.up_down_item);

            for (i = 0; i < nsets; i++) {
                setno = selset[i];
	        do_sort(setno, sorton, stype);
            }
            break;
        case DATASETOP_REVERSE:
            for (i = 0; i < nsets; i++) {
                setno = selset[i];
	        reverse_set(cg, setno);
            }
            break;
        case DATASETOP_JOIN:
            join_sets(cg, selset, nsets);
            break;
        case DATASETOP_SPLIT:
            xv_evalexpri(datasetopui.length_item, &lpart);
            for (i = 0; i < nsets; i++) {
                setno = selset[i];
                do_splitsets(cg, setno, lpart);
            }
            break;
        case DATASETOP_DROP:
            xv_evalexpri(datasetopui.start_item, &startno);
            xv_evalexpri(datasetopui.stop_item, &endno);
            for (i = 0; i < nsets; i++) {
                setno = selset[i];
		do_drop_points(setno, startno, endno);
            }
            break;
        }
 
        if (aac_mode == AAC_ACCEPT && error == FALSE) {
            XtUnmanageChild(datasetopui.top);
        }
        
        free(selset);

        update_set_lists(cg);
        unset_wait_cursor();
        drawgraph();
    }
}


typedef struct _Setop_ui {
    Widget top;
    SrcDestStructure *srcdest;
    OptionStructure *optype_item;
} Setop_ui;

static Setop_ui setopui;

#define OPTYPE_COPY 0
#define OPTYPE_MOVE 1
#define OPTYPE_SWAP 2

void create_setop_popup(void *data)
{
    set_wait_cursor();
    if (setopui.top == NULL) {
        Widget panel, fr;
        OptionItem opitems[3];

	setopui.top = XmCreateDialogShell(app_shell, "setOperations", NULL, 0);
	handle_close(setopui.top);
        panel = XtVaCreateWidget("panel", xmFormWidgetClass, 
                                          setopui.top, NULL, 0);

        setopui.srcdest = CreateSrcDestSelector(panel, LIST_TYPE_MULTIPLE);        

        opitems[0].value = OPTYPE_COPY;
        opitems[0].label = "Copy";
        opitems[1].value = OPTYPE_MOVE;
        opitems[1].label = "Move";
        opitems[2].value = OPTYPE_SWAP;
        opitems[2].label = "Swap";
        setopui.optype_item = CreateOptionChoice(panel,
            "Type of operation:", 0, 3, opitems);
        XtVaSetValues(setopui.optype_item->menu,
            XmNtopAttachment, XmATTACH_WIDGET,
            XmNtopWidget, setopui.srcdest->form,
            XmNleftAttachment, XmATTACH_FORM,
            XmNrightAttachment, XmATTACH_FORM,
            NULL);
        
	fr = CreateFrame(panel, NULL);
        XtVaSetValues(fr,
            XmNtopAttachment, XmATTACH_WIDGET,
            XmNtopWidget, setopui.optype_item->menu,
            XmNleftAttachment, XmATTACH_FORM,
            XmNrightAttachment, XmATTACH_FORM,
            XmNbottomAttachment, XmATTACH_FORM,
            NULL);
        CreateAACButtons(fr, panel, setop_aac_cb);

	XtManageChild(panel);
    }
    XtRaise(setopui.top);
    unset_wait_cursor();
}

static void setop_aac_cb(void *data)
{
    int aac_mode, optype, error;
    int i, g1_ok, g2_ok, ns1, ns2, *svalues1, *svalues2, gno1, gno2, setno2;

    aac_mode = (int) data;
    
    if (aac_mode == AAC_CLOSE) {
        XtUnmanageChild(setopui.top);
        return;
    }

    set_wait_cursor();
    
    optype = GetOptionChoice(setopui.optype_item);
    
    g1_ok = GetSingleListChoice(setopui.srcdest->src->graph_sel, &gno1);
    g2_ok = GetSingleListChoice(setopui.srcdest->dest->graph_sel, &gno2);
    ns1 = GetListChoices(setopui.srcdest->src->set_sel, &svalues1);
    ns2 = GetListChoices(setopui.srcdest->dest->set_sel, &svalues2);
    
    error = FALSE;
    if (g1_ok == GRACE_EXIT_FAILURE || g2_ok == GRACE_EXIT_FAILURE) {
        error = TRUE;
        errmsg("Please select single source and destination graphs");
    } else if (ns1 == 0) {
        error = TRUE;
        errmsg("No source sets selected");
    } else if (ns2 == 0 && optype == OPTYPE_SWAP) {
        error = TRUE;
        errmsg("No destination sets selected");
    } else if (ns1 != ns2 && (optype == OPTYPE_SWAP || ns2 != 0)) {
        error = TRUE;
        errmsg("Different number of source and destination sets");
    } else if (gno1 == gno2 && ns2 == 0 && optype == OPTYPE_MOVE) {
        error = TRUE;
        errmsg("Can't move a set to itself");
    } else {
        for (i = 0; i < ns1; i++) {
            switch (optype) {
            case OPTYPE_SWAP:
                if (do_swapset(gno1, svalues1[i], gno2, svalues2[i])
                                                != GRACE_EXIT_SUCCESS) {
                    error = TRUE;
                }
                break;
            case OPTYPE_COPY:
                if (ns2 == 0) {
                    setno2 = nextset(gno2);
                } else {
                    setno2 = svalues2[i];
                }
                if (do_copyset(gno1, svalues1[i], gno2, setno2)
                                                != GRACE_EXIT_SUCCESS) {
                    error = TRUE;
                }
                break;
            case OPTYPE_MOVE:
                if (ns2 == 0) {
                    setno2 = nextset(gno2);
                } else {
                    setno2 = svalues2[i];
                }
                if (do_moveset(gno1, svalues1[i], gno2, setno2)
                                                != GRACE_EXIT_SUCCESS) {
                    error = TRUE;
                }
                break;
            }
        }
    }
    
    if (aac_mode == AAC_ACCEPT && error == FALSE) {
        XtUnmanageChild(setopui.top);
    }

    if (ns1 > 0) {
        free(svalues1);
    }
    if (ns2 > 0) {
        free(svalues2);
    }
    if (error == FALSE) {
        update_all();
        drawgraph();
    }
    unset_wait_cursor();
}

typedef struct _Leval_ui {
    Widget top;
    OptionStructure *set_type;
    Widget start;
    Widget stop;
    Widget npts;
    Widget mw;
    int gno;
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
    int gno = (int) data;

    set_wait_cursor();

    if (is_valid_gno(gno)) {
        levalui.gno = gno;
    } else {
        levalui.gno = get_cg();
    }

    if (levalui.top == NULL) {
        int i;
        Widget panel, fr, rc1;
        int nscols;
        char *rows[MAX_SET_COLS][1];
        char **cells[MAX_SET_COLS];
        char *rowlabels[MAX_SET_COLS];
        short column_widths[1] = {50};

	levalui.top = XmCreateDialogShell(app_shell, "Load & evaluate", NULL, 0);
	handle_close(levalui.top);
	panel = XmCreateForm(levalui.top, "form", NULL, 0);

	fr = CreateFrame(panel, "Parameter mesh");
        rc1 = XtVaCreateWidget("rc1",
            xmRowColumnWidgetClass, fr,
            XmNorientation, XmHORIZONTAL,
            NULL);
	levalui.start = CreateTextItem2(rc1, 10, "Start at:");
	levalui.stop = CreateTextItem2(rc1, 10, "Stop at:");
	levalui.npts = CreateTextItem2(rc1, 6, "Length:");
	XtManageChild(rc1);

        XtVaSetValues(fr,
            XmNtopAttachment, XmATTACH_FORM,
            XmNleftAttachment, XmATTACH_FORM,
            XmNrightAttachment, XmATTACH_FORM,
            NULL);

 
	levalui.set_type = CreateSetTypeChoice(panel, "Set type:");
        AddOptionChoiceCB(levalui.set_type, set_type_cb, (void *) &levalui);
        XtVaSetValues(levalui.set_type->menu,
            XmNtopAttachment, XmATTACH_WIDGET,
            XmNtopWidget, fr,
            XmNleftAttachment, XmATTACH_FORM,
            XmNrightAttachment, XmATTACH_FORM,
            NULL);
	
        nscols = settype_cols(curtype);
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
            xbaeMatrixWidgetClass, panel,
            XmNrows, nscols,
            XmNcolumns, 1,
            XmNvisibleRows, MAX_SET_COLS,
            XmNvisibleColumns, 1,
            XmNcolumnWidths, column_widths,
            XmNrowLabels, rowlabels,
	    XmNrowLabelWidth, 6,
            XmNrowLabelAlignment, XmALIGNMENT_CENTER,
            XmNcells, cells,
            XmNgridType, XmGRID_CELL_SHADOW,
            XmNcellShadowType, XmSHADOW_ETCHED_OUT,
            XmNcellShadowThickness, 2,
            XmNaltRowCount, 0,
            NULL);

        XtAddCallback(levalui.mw, XmNleaveCellCallback, leaveCB, &levalui);
        
        XtVaSetValues(levalui.mw,
            XmNtopAttachment, XmATTACH_WIDGET,
            XmNtopWidget, levalui.set_type->menu,
            XmNleftAttachment, XmATTACH_FORM,
            XmNrightAttachment, XmATTACH_FORM,
            NULL);

	fr = CreateFrame(panel, NULL);
        XtVaSetValues(fr,
            XmNtopAttachment, XmATTACH_WIDGET,
            XmNtopWidget, levalui.mw,
            XmNleftAttachment, XmATTACH_FORM,
            XmNrightAttachment, XmATTACH_FORM,
            XmNbottomAttachment, XmATTACH_FORM,
            NULL);
        CreateAACButtons(fr, panel, leval_aac_cb);

	XtManageChild(panel);
    }
    XtRaise(levalui.top);
    unset_wait_cursor();
}

static void leval_aac_cb(void *data)
{
    int aac_mode;
    int i, nscols, type;
    double start, stop;
    int npts;
    char *formula[MAX_SET_COLS];
    int res, len;
    int setno, gno;
    double *ex;
    grarr *t;
    
    aac_mode = (int) data;

    if (aac_mode == AAC_CLOSE) {
        XtUnmanageChild(levalui.top);
        return;
    }

    
    gno = levalui.gno;
    type = GetOptionChoice(levalui.set_type);
    nscols = settype_cols(type);

    if (xv_evalexpr(levalui.start, &start) != GRACE_EXIT_SUCCESS) {
	errmsg("Start item undefined");
        return;
    }

    if (xv_evalexpr(levalui.stop, &stop) != GRACE_EXIT_SUCCESS) {
	errmsg("Stop item undefined");
        return;
    }

    if (xv_evalexpri(levalui.npts, &npts) != GRACE_EXIT_SUCCESS) {
	errmsg("Number of points undefined");
        return;
    }

    XbaeMatrixCommitEdit(levalui.mw, False);
    for (i = 0; i < nscols; i++) {
        formula[i] = XbaeMatrixGetCell(levalui.mw, i, 0);
    }
    
    set_wait_cursor();

    t = get_parser_arr_by_name("$t");
    if (t == NULL) {
        t = define_parser_arr("$t");
        if (t == NULL) {
	    errmsg("Internal error");
            unset_wait_cursor();
            return;
        }
    }
    
    if (t->length != 0) {
        free(t->data);
        t->length = 0;
    }
    t->data = allocate_mesh(start, stop, npts);
    if (t->data == NULL) {
	errmsg("Not enough memory");
        unset_wait_cursor();
        return;
    }
    t->length = npts;
    
    setno = nextset(gno);
    set_dataset_type(gno, setno, type);
    set_set_hidden(gno, setno, FALSE);
    if (setlength(gno, setno, npts) != GRACE_EXIT_SUCCESS) {
	errmsg("Can't allocate more sets");
        killset(gno, setno);
        cxfree(t->data);
        t->length = 0;
        unset_wait_cursor();
        return;
    }

    for (i = 0; i < nscols; i++) {
        res = v_scanner(formula[i], &len, &ex);
        if (res != GRACE_EXIT_SUCCESS || len != npts) {
	    char buf[32];
            sprintf(buf, "Error in formula for %s", dataset_colname(i));
            errmsg(buf);
            killset(gno, setno);
            cxfree(t->data);
            t->length = 0;
            unset_wait_cursor();
            return;
        }
        setcol(gno, setno, i, ex, npts);
    }
    
    cxfree(t->data);
    t->length = 0;
    
    if (aac_mode == AAC_ACCEPT) {
        XtUnmanageChild(levalui.top);
    }

    update_set_lists(gno);
    drawgraph();

    unset_wait_cursor();
}
