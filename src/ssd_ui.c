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
#include <string.h>

#include "events.h"
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

/* minimum size of the spreadsheet matrix */
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

static int enterCB(TableEvent *event)
{
    SSDataUI *ui = (SSDataUI *) event->anydata;

    int ncols = ssd_get_ncols(ui->q);
    
    if (event->col >= 0 && event->col <= ncols) {
        TableDeselectAllCells(ui->mw);
        return TRUE;
    } else {
        return FALSE;
    }
}

static int leaveCB(TableEvent *event)
{
    SSDataUI *ui = (SSDataUI *) event->anydata;

    int nrows = ssd_get_nrows(ui->q);
    int ncols = ssd_get_ncols(ui->q);
    int format;
    double value;
    
    int changed = FALSE;
    
    GraceApp *gapp = gapp_from_quark(ui->q);
    
    if (event->row < 0 || event->col < 0 || event->col > ncols) {
        return TRUE;
    }
    
    if (event->row >= nrows && !string_is_empty(event->value)) {
        if (ssd_set_nrows(ui->q, event->row + 1) == RETURN_SUCCESS) {
            changed = TRUE;
        }
    }
    
    if (event->col == ncols && !string_is_empty(event->value)) {
        if (parse_date_or_number(get_parent_project(ui->q),
            event->value, FALSE, get_date_hint(gapp), &value) == RETURN_SUCCESS) {
            format = FFORMAT_NUMBER;
        } else {
            format = FFORMAT_STRING;
        }
        if (ssd_add_col(ui->q, format)) {
            ncols++;
            changed = TRUE;
        }
    }
    
    if (event->col < ncols) {
        char *old_value = get_cell_content(ui, event->row, event->col, &format);
        if (!strings_are_equal(old_value, event->value)) {
            switch (format) {
            case FFORMAT_STRING:
                if (ssd_set_string(ui->q, event->row, event->col, event->value) ==
                    RETURN_SUCCESS) {
                    changed = TRUE;
                }
                break;    
            default:
                if (graal_eval_expr(grace_get_graal(gapp->grace),
                    event->value, &value, gproject_get_top(gapp->gp)) == RETURN_SUCCESS) {

                    unsigned int prec;
                    char buf[32];
                    double val;

                    prec = project_get_prec(get_parent_project(ui->q));
                    sprintf(buf, "%.*g", prec, value);

                    if (parse_date_or_number(get_parent_project(ui->q),
                        buf, FALSE, get_date_hint(gapp), &val) == RETURN_SUCCESS) {

                        if (ssd_set_value(ui->q, event->row, event->col, val) == RETURN_SUCCESS) {
                            changed = TRUE;
                        }
                    }
                } else {
                    errmsg("Can't parse input value");
                    return FALSE;
                }
                break;
            }
        }
    }
    
    if (changed) {
        snapshot_and_update(gapp->gp, FALSE);
    }

    return TRUE;
}

static int labelCB(TableEvent *event)
{
    SSDataUI *ui = (SSDataUI *) event->anydata;
    static int last_row, last_column;
    int i;
    
    if (!event || event->type != MOUSE_PRESS) {
        return TRUE;
    }

    if (event->button == LEFT_BUTTON) {
        TableCommitEdit(ui->mw, TRUE);

        if (event->row_label) {
            if (event->modifiers & CONTROL_MODIFIER) {
                if (TableIsRowSelected(ui->mw, event->row)) {
                    TableDeselectRow(ui->mw, event->row);
                } else {
                    TableSelectRow(ui->mw, event->row);
                }
                last_row = event->row;
            } else
            if ((event->modifiers & SHIFT_MODIFIER) && last_row >= 0) {
                for (i = MIN2(last_row, event->row); i <= MAX2(last_row, event->row); i++) {
                    TableSelectRow(ui->mw, i);
                }
            } else {
                TableDeselectAllCells(ui->mw);
                TableSelectRow(ui->mw, event->row);
                last_row = event->row;
            }

            last_column = -1;
        } else {
            if (event->modifiers & CONTROL_MODIFIER) {
                if (TableIsColSelected(ui->mw, event->col)) {
                    TableDeselectCol(ui->mw, event->col);
                } else {
                    TableSelectCol(ui->mw, event->col);
                }
                last_column = event->col;
            } else
            if ((event->modifiers & SHIFT_MODIFIER) && last_column >= 0) {
                for (i = MIN2(last_column, event->col); i <= MAX2(last_column, event->col); i++) {
                    TableSelectCol(ui->mw, i);
                }
            } else {
                TableDeselectAllCells(ui->mw);
                TableSelectCol(ui->mw, event->col);
                last_column = event->col;
            }

            last_row = -1;
        }
    }

    if (event->button == RIGHT_BUTTON) {
        ss_column *col;
        if (!event->row_label) {
            ui->cb_column = event->col;
        }

        col = ssd_get_col(ui->q, ui->cb_column);
        SetSensitive(ui->delete_btn, col != NULL);
        SetSensitive(ui->index_btn, col != NULL && ui->cb_column != 0 &&
            (col->format == FFORMAT_NUMBER || col->format == FFORMAT_DATE));
        SetSensitive(ui->unindex_btn, ui->cb_column == 0 && col != NULL &&
            ssd_is_indexed(ui->q));
        
        ShowMenu(ui->popup, event->udata);
    }

    return TRUE;
}

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
    TableSSDInit(ui->mw);
    TableSetDefaultColWidth(ui->mw, CELL_WIDTH);
    TableSetDefaultColLabelAlignment(ui->mw, ALIGN_CENTER);

    AddTableLeaveCellCB(ui->mw, leaveCB, ui);
    AddTableEnterCellCB(ui->mw, enterCB, ui);
    AddTableLabelActivateCB(ui->mw, labelCB, ui);

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
            TableDeselectAllCells(ui->mw);
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

        nr = TableGetNrows(ui->mw);
        nc = TableGetNcols(ui->mw);

        delta_nr = new_nr - nr;
        delta_nc = new_nc - nc;

        if (delta_nr > 0) {
            TableAddRows(ui->mw, delta_nr);
        } else if (delta_nr < 0) {
            TableDeleteRows(ui->mw, -delta_nr);
        }

        rowlabels = xmalloc(new_nr*sizeof(char *));
        for (i = 0; i < new_nr; i++) {
            char buf[32];
            sprintf(buf, "%d", i + 1);
            rowlabels[i] = copy_string(NULL, buf);
        }
        TableSetRowLabels(ui->mw, rowlabels);
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
            TableAddCols(ui->mw, delta_nc);
        } else if (delta_nc < 0) {
            TableDeleteCols(ui->mw, -delta_nc);
        }

        TableSetColMaxlengths(ui->mw, maxlengths);
        TableSetColLabels(ui->mw, collabels);
        TableSetFixedCols(ui->mw, nfixed_cols);

        for (cur_col = 0; cur_col < new_nc; cur_col++) {
            for (cur_row = 0; cur_row < new_nr; cur_row++) {
                TableSetCellContent(ui->mw, cur_row, cur_col,
                                       get_cell_content(ui, cur_row, cur_col, &format));
            }
        }

        TableUpdateVisibleRowsCols(ui->mw);

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
            TableCommitEdit(ui->mw, FALSE);
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
