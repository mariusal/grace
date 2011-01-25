/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1996-2005 Grace Development Team
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

/* SSData UI */

#include <stdlib.h>

#include "utils.h"
#include "explorer.h"
#include "xprotos.h"
#include "globals.h"

/* default cell value precision */
#define CELL_PREC 8

/* default cell value format */
#define CELL_FORMAT FORMAT_GENERAL

/* default cell width */
#define CELL_WIDTH 12

/* string cell width */
#define STRING_CELL_WIDTH 128

/* minimum size of the spreadseet matrix */
#define EXTRA_SS_ROWS      20
#define EXTRA_SS_COLS       3

#define VISIBLE_SS_ROWS  18
#define VISIBLE_SS_COLS   3

static int do_hotlinkfile_proc(FSBStructure *fsb, char *filename, void *data)
{
    SSDataUI *ui = (SSDataUI *) data;
    
    xv_setstr(ui->hotfile, filename);
    
    return TRUE;
}

/*
 * create file selection pop up to choose the file for hotlink
 */
static void create_hotfiles_popup(Widget but, void *data)
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

/*
 * We use a stack of static buffers to work around asynchronous
 * refresh/redraw events
 */
#define STACKLEN    (VISIBLE_SS_ROWS*VISIBLE_SS_COLS)

static char *get_cell_content(SSDataUI *ui, int row, int column, int *format)
{
    static char buf[STACKLEN][32];
    static int stackp = 0;
    
    int nrows = ssd_get_nrows(ui->q);
    ss_column *col = ssd_get_col(ui->q, column);
    char *s;

    if (col && row >= 0 && row < nrows) {
        unsigned int prec;
        *format = col->format;
        switch (col->format) {
        case FFORMAT_STRING:
            s = ((char **) col->data)[row];
            break;
        default:
            prec = project_get_prec(get_parent_project(ui->q));
            sprintf(buf[stackp], "%.*g", prec, ((double *) col->data)[row]);
            s = buf[stackp];
            stackp++;
            stackp %= STACKLEN;
            
            /* get rid of spaces */
            while (s && *s == ' ') {
                s++;
            }
            
            break;
        }
    } else {
        s = "";
    }
    
    return s;
}

static void enterCB(Table t, int etype, void *cbdata)
{
    SSDataUI *ui = (SSDataUI *) cbdata;
    XbaeMatrixEnterCellCallbackStruct *cs =
        (XbaeMatrixEnterCellCallbackStruct *) call_data;
    int ncols = ssd_get_ncols(ui->q);
    
    if (cs->column >= 0 && cs->column <= ncols) {
        table_deselect_all_cells(ui->mw);
    } else {
        cs->doit = False;
        cs->map  = False;
    }
}

static void leaveCB(Widget w, XtPointer client_data, XtPointer call_data)
{
    SSDataUI *ui = (SSDataUI *) client_data;
    XbaeMatrixLeaveCellCallbackStruct *cs =
        (XbaeMatrixLeaveCellCallbackStruct *) call_data;

    int nrows = ssd_get_nrows(ui->q);
    int ncols = ssd_get_ncols(ui->q);
    int format;
    double value;
    
    int changed = FALSE;
    
    GraceApp *gapp = gapp_from_quark(ui->q);
    
    if (cs->row < 0 || cs->column < 0 || cs->column > ncols) {
        return;
    }
    
    if (cs->row >= nrows && !string_is_empty(cs->value)) {
        if (ssd_set_nrows(ui->q, cs->row + 1) == RETURN_SUCCESS) {
            changed = TRUE;
        }
    }
    
    if (cs->column == ncols && !string_is_empty(cs->value)) {
        if (parse_date_or_number(get_parent_project(ui->q),
            cs->value, FALSE, get_date_hint(gapp), &value) == RETURN_SUCCESS) {
            format = FFORMAT_NUMBER;
        } else {
            format = FFORMAT_STRING;
        }
        if (ssd_add_col(ui->q, format)) {
            ncols++;
            changed = TRUE;
        }
    }
    
    if (cs->column < ncols) {
        char *old_value = get_cell_content(ui, cs->row, cs->column, &format);
        if (!strings_are_equal(old_value, cs->value)) {
            switch (format) {
            case FFORMAT_STRING:
                if (ssd_set_string(ui->q, cs->row, cs->column, cs->value) ==
                    RETURN_SUCCESS) {
                    changed = TRUE;
                }
                break;    
            default:
                if (parse_date_or_number(get_parent_project(ui->q),
                    cs->value, FALSE, get_date_hint(gapp), &value) == RETURN_SUCCESS) {
                    if (ssd_set_value(ui->q, cs->row, cs->column, value) ==
                        RETURN_SUCCESS) {
                        changed = TRUE;
                    }
                } else {
                    errmsg("Can't parse input value");
                    cs->doit = False;
                }
                break;
            }
        }
    }
    
    if (changed) {
        snapshot_and_update(gapp->gp, FALSE);
    }
}
#ifndef QT_GUI
static void labelCB(Widget w, XtPointer client_data, XtPointer call_data)
{
    SSDataUI *ui = (SSDataUI *) client_data;
    XbaeMatrixLabelActivateCallbackStruct *cbs =
        (XbaeMatrixLabelActivateCallbackStruct *) call_data;
    XEvent *e = cbs->event;
    XButtonEvent *xbe;
    static int last_row, last_column;
    int i;
    
    if (!e || (e->type != ButtonRelease && e->type != ButtonPress)) {
        return;
    }

    xbe = (XButtonEvent *) e;

    if (xbe->button == Button1) {
        XbaeMatrixCommitEdit(ui->mw, True);

        if (cbs->row_label) {
            if (xbe->state & ControlMask) {
                if (XbaeMatrixIsRowSelected(ui->mw, cbs->row)) {
                    XbaeMatrixDeselectRow(ui->mw, cbs->row);
                } else {
                    XbaeMatrixSelectRow(ui->mw, cbs->row);
                }
                last_row = cbs->row;
            } else
            if ((xbe->state & ShiftMask) && last_row >= 0) {
                for (i = MIN2(last_row, cbs->row); i <= MAX2(last_row, cbs->row); i++) {
                    XbaeMatrixSelectRow(ui->mw, i);
                }
            } else {
                XbaeMatrixDeselectAll(ui->mw);
                XbaeMatrixSelectRow(ui->mw, cbs->row);
                last_row = cbs->row;
            }

            last_column = -1;
        } else {
            if (xbe->state & ControlMask) {
                if (XbaeMatrixIsColumnSelected(ui->mw, cbs->column)) {
                    XbaeMatrixDeselectColumn(ui->mw, cbs->column);
                } else {
                    XbaeMatrixSelectColumn(ui->mw, cbs->column);
                }
                last_column = cbs->column;
            } else
            if ((xbe->state & ShiftMask) && last_column >= 0) {
                for (i = MIN2(last_column, cbs->column); i <= MAX2(last_column, cbs->column); i++) {
                    XbaeMatrixSelectColumn(ui->mw, i);
                }
            } else {
                XbaeMatrixDeselectAll(ui->mw);
                XbaeMatrixSelectColumn(ui->mw, cbs->column);
                last_column = cbs->column;
            }

            last_row = -1;
        }
    }

    if (xbe->button == Button3) {
        ss_column *col;
        if (!cbs->row_label) {
            ui->cb_column = cbs->column;
        }

        col = ssd_get_col(ui->q, ui->cb_column);
        SetSensitive(ui->delete_btn, col != NULL);
        SetSensitive(ui->index_btn, col != NULL && ui->cb_column != 0 &&
            (col->format == FFORMAT_NUMBER || col->format == FFORMAT_DATE));
        SetSensitive(ui->unindex_btn, ui->cb_column == 0 && col != NULL &&
            ssd_is_indexed(ui->q));
        
        XmMenuPosition(ui->popup, xbe);
        ManageChild(ui->popup);
        return;
    }
}
#endif

static void col_delete_cb(Widget but, void *udata)
{
    SSDataUI *ui = (SSDataUI *) udata;
    if (ssd_delete_col(ui->q, ui->cb_column) == RETURN_SUCCESS) {
        snapshot_and_update(gapp->gp, TRUE);
    }
}

static void index_cb(Widget but, void *udata)
{
    SSDataUI *ui = (SSDataUI *) udata;
    if (ssd_set_index(ui->q, ui->cb_column) == RETURN_SUCCESS) {
        snapshot_and_update(gapp->gp, TRUE);
    }
}

static void unindex_cb(Widget but, void *udata)
{
    SSDataUI *ui = (SSDataUI *) udata;
    if (ssd_set_indexed(ui->q, FALSE) == RETURN_SUCCESS) {
        snapshot_and_update(gapp->gp, TRUE);
    }
}

static void col_cb(ListStructure *sel, int n, int *values, void *data)
{
    SSDataUI *ui = (SSDataUI *) data;
    Quark *ssd = (Quark *) sel->anydata;
    
    if (ssd && n == 1) {
        int col = values[0];
        SetSensitive(ui->col_label->text, TRUE);
        SetTextString(ui->col_label, ssd_get_col_label(ssd, col));
    } else {
        SetSensitive(ui->col_label->text, FALSE);
    }
}

SSDataUI *create_ssd_ui(ExplorerUI *eui)
{
    SSDataUI *ui;

    Widget tab, fr, rc, rc1, wbut;
    
    ui = xmalloc(sizeof(SSDataUI));
    if (!ui) {
        return NULL;
    }
    memset(ui, 0, sizeof(SSDataUI));

    /* ------------ Tabs -------------- */

    tab = CreateTab(eui->scrolled_window);        
    AddHelpCB(tab, "doc/UsersGuide.html#ssd-properties");

    ui->top = tab;

    /* ------------ Main tab -------------- */
    ui->main_tp = CreateTabPage(tab, "Data");

    ui->mw = CreateTable(ui->main_tp,
                         EXTRA_SS_ROWS, EXTRA_SS_COLS,
                         VISIBLE_SS_ROWS, VISIBLE_SS_COLS);
    table_set_default_col_width(ui->mw, CELL_WIDTH);
    table_set_default_col_label_alignment(ui->mw, ALIGN_CENTER);

    table_enter_cell_cb_add(ui->mw, enterCB, ui);
    table_leave_cell_cb_add(ui->mw, leaveCB, ui);
    table_label_activate_cb_add(ui->mw, labelCB, ui);

    ui->popup = CreatePopupMenu(ui->mw);
    ui->delete_btn  = CreateMenuButton(ui->popup, "Delete column", '\0', col_delete_cb, ui);
    ui->index_btn   = CreateMenuButton(ui->popup, "Set as index", '\0', index_cb, ui);
    ui->unindex_btn = CreateMenuButton(ui->popup, "Unset index", '\0', unindex_cb, ui);


    /* ------------ Column props -------------- */
    ui->column_tp = CreateTabPage(tab, "Columns");
    ui->col_sel = CreateColChoice(ui->column_tp, "Column:", LIST_TYPE_SINGLE);
    AddListChoiceCB(ui->col_sel, col_cb, ui);

    ui->col_label = CreateCSText(ui->column_tp, "Label:");
    SetSensitive(ui->col_label->text, FALSE);
    AddTextInputCB(ui->col_label, text_explorer_cb, eui);

    
    /* ------------ Hotlink tab -------------- */
    ui->hotlink_tp = CreateTabPage(tab, "Hotlink");

    fr = CreateFrame(ui->hotlink_tp, "Hotlink");
    rc = CreateVContainer(fr);
    rc1 = CreateHContainer(rc);
    ui->hotlink = CreateToggleButton(rc1, "Enabled");
    ui->hotsrc  = CreateOptionChoiceVA(rc1, "Source type:",
        "Disk", SOURCE_DISK,
        "Pipe", SOURCE_PIPE,
        NULL);
    rc1 = CreateHContainer(rc);
    ui->hotfile = CreateTextItem(rc1, 20, "File name:");
    wbut = CreateButton(rc1, "Browse...");
    AddButtonCB(wbut, create_hotfiles_popup, ui);

    return ui;
}

void update_ssd_ui(SSDataUI *ui, Quark *q)
{
    if (ui && q) {
        int i, nc, nr, new_nc, new_nr, ncols, nrows, nfixed_cols;
        int delta_nc, delta_nr;
        int *maxlengths;
        char **rowlabels, **collabels;
        int cur_row, cur_col, format;
        
        if (ui->q != q) {
            table_deselect_all_cells(ui->mw);
        }
        
        ui->q = q;
        
        ncols = ssd_get_ncols(q);
        nrows = ssd_get_nrows(q);
        
        new_nc = ncols + EXTRA_SS_COLS;
        new_nr = nrows + EXTRA_SS_ROWS;
        
        if (ssd_is_indexed(q)) {
            nfixed_cols = 1;
        } else {
            nfixed_cols = 0;
        }

        nr = table_get_nrows(ui->mw);
        nc = table_get_ncols(ui->mw);

        delta_nr = new_nr - nr;
        delta_nc = new_nc - nc;

        if (delta_nr > 0) {
            table_add_rows(ui->mw, delta_nr);
        } else if (delta_nr < 0) {
            table_delete_rows(ui->mw, -delta_nr);
        }

        rowlabels = xmalloc(new_nr*sizeof(char *));
        for (i = 0; i < new_nr; i++) {
            char buf[32];
            sprintf(buf, "%d", i + 1);
            rowlabels[i] = copy_string(NULL, buf);
        }
        table_set_row_labels(ui->mw, rowlabels);
        for (i = 0; i < new_nr; i++) {
            xfree(rowlabels[i]);
        }
        xfree(rowlabels);

        maxlengths = xmalloc(new_nc*SIZEOF_INT);
        collabels = xmalloc(new_nc*sizeof(char *));

        for (i = 0; i < new_nc; i++) {
            ss_column *col = ssd_get_col(q, i);
            if (col && col->format == FFORMAT_STRING) {
                maxlengths[i] = STRING_CELL_WIDTH;
            } else {
                maxlengths[i] = 2*CELL_WIDTH;
            }
            if (col && !string_is_empty(col->label)) {
                collabels[i] = copy_string(NULL, col->label);
            } else {
                unsigned int coli;
                char buf[32];
                
                coli = i;
                sprintf(buf, "%c", coli%26 + 'A');
                while ((coli /= 26)) {
                    memmove(&buf[1], buf, strlen(buf) + 1);
                    buf[0] = coli%26 + 'A' - 1;
                }
                
                collabels[i] = copy_string(NULL, buf);
            }
        }

        if (delta_nc > 0) {
            table_add_cols(ui->mw, delta_nc);
        } else if (delta_nc < 0) {
            table_delete_cols(ui->mw, -delta_nc);
        }

        table_set_col_maxlengths(ui->mw, maxlengths);
        table_set_col_labels(ui->mw, collabels);
        table_set_fixed_cols(ui->mw, nfixed_cols);

        for (cur_col = 0; cur_col < new_nc; cur_col++) {
            for (cur_row = 0; cur_row < new_nr; cur_row++) {
                table_set_cell_content(ui->mw, cur_row, cur_col,
                                       get_cell_content(ui, cur_row, cur_col, &format));
            }
        }

        table_update_visible_rows_cols(ui->mw);

        xfree(maxlengths);
        for (i = 0; i < new_nc; i++) {
            xfree(collabels[i]);
        }
        xfree(collabels);
        
        UpdateColChoice(ui->col_sel, q);
    }
}

int set_ssd_data(SSDataUI *ui, Quark *q, void *caller)
{
    int retval = RETURN_SUCCESS;
    
    if (ui && q) {
        if (!caller) {
            /* commit the last entered cell changes */
#ifndef QT_GUI
            XbaeMatrixCommitEdit(ui->mw, False);
#endif
        }
        
        if (!caller || caller == ui->col_label) {
            int col;
            if (GetSingleListChoice(ui->col_sel, &col) == RETURN_SUCCESS) {
                char *s = GetTextString(ui->col_label);
                ssd_set_col_label(q, col, s);
                xfree(s);
                
                /* FIXME: this is an overkill */
                update_ssd_ui(ui, q);
            }
        }
    }
    
    return retval;
}
