/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
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
 * Font tool
 *
 */

#include <config.h>
#include <string.h>

#include "globals.h"
#include "motifinc.h"
#include "xprotos.h"

/* the size of the font tool matrix */
#define FONT_TOOL_ROWS      16
#define FONT_TOOL_COLS      16

typedef struct _fonttool_ui
{
    Widget fonttool_panel;
    OptionStructure *font_select;
    Widget font_table;
    TextStructure *cstext;
    WidgetList aac_buts;

    TextStructure *cstext_parent;

    int new_font;
    int font_id;
    int csize;

    int enable_edit_cb;

    char valid_chars[256];
} fonttool_ui;

static void DrawCB(TableEvent *event);
static void EnterCB(Widget w, XtPointer client_data, XtPointer call_data);
static void EditStringCB(Widget w, XtPointer client_data, XtPointer call_data);

static void update_fonttool_cb(OptionStructure *opt, int value, void *data);
static int fonttool_aac_cb(void *data);

void create_fonttool_cb(Widget but, void *data)
{
    create_fonttool((TextStructure *) data);
}

void create_fonttool(TextStructure *cstext_parent)
{
    static fonttool_ui *ui = NULL;
    
    if (ui == NULL) {
        ui = xmalloc(sizeof(fonttool_ui));
        memset(ui, 0, sizeof(fonttool_ui));
	
        ui->fonttool_panel = CreateDialogForm(app_shell, "Font tool");

        ui->font_select = CreateFontChoice(ui->fonttool_panel, "Font:");
        AddDialogFormChild(ui->fonttool_panel, ui->font_select->menu);
        
        ui->font_table = CreateTable(ui->fonttool_panel,
                                     FONT_TOOL_ROWS, FONT_TOOL_COLS,
                                     8, 16);
        TableFontInit(ui->font_table);
        TableSetDefaultColWidth(ui->font_table, 2);
        //TODO: not col label alin but col align
        TableSetDefaultColLabelAlignment(ui->font_table, ALIGN_BEGINNING);

        AddTableDrawCellCB(ui->font_table, DrawCB, ui);
//        XtAddCallback(ui->font_table, XmNenterCellCallback, EnterCB, ui);
        AddOptionChoiceCB(ui->font_select, update_fonttool_cb, ui);

        AddDialogFormChild(ui->fonttool_panel, ui->font_table);

        ui->cstext = CreateCSText(ui->fonttool_panel, "CString:");

//        XtAddCallback(ui->cstext->text,
//            XmNmodifyVerifyCallback, EditStringCB, ui);
        
        ui->aac_buts = CreateAACDialog(ui->fonttool_panel,
            ui->cstext->form, fonttool_aac_cb, ui);

        FixateDialogFormChild(ui->cstext->form);
        
        update_fonttool_cb(NULL, 0, ui);
    }

    if (cstext_parent == ui->cstext) {
        /* avoid recursion */
        return;
    }
    
    ui->cstext_parent = cstext_parent;
    
    ui->enable_edit_cb = FALSE;
    if (ui->cstext_parent == NULL) {
        SetTextString(ui->cstext, "");
        SetSensitive(ui->aac_buts[0], FALSE);
        SetSensitive(ui->aac_buts[1], FALSE);
    } else {
        char *s = GetTextString(ui->cstext_parent);
        int pos = GetTextCursorPos(ui->cstext_parent);
        SetTextString(ui->cstext, s);
        SetTextCursorPos(ui->cstext, pos);
        xfree(s);
        SetSensitive(ui->aac_buts[0], TRUE);
        SetSensitive(ui->aac_buts[1], TRUE);
    }
    ui->enable_edit_cb = TRUE;
    
    RaiseWindow(GetParent(ui->fonttool_panel));
}

static void DrawCB(TableEvent *event)
{
    fonttool_ui *ui = (fonttool_ui *) event->anydata;
    unsigned char c;
    Pixmap pixmap;

    c = 16*event->row + event->col;

    if (ui->font_id == BAD_FONT_ID) {
        pixmap = 0;
    } else {
        pixmap = char_to_pixmap(event->w, ui->font_id, c, ui->csize);
    }

    if (pixmap || c == ' ') {
        ui->valid_chars[c] = TRUE;
    } else {
        ui->valid_chars[c] = FALSE;
    }

    if (pixmap) {
        event->value_type = TABLE_CELL_PIXMAP;
        event->pixmap = pixmap;
    }

    return TRUE;
}

static void insert_into_string(TextStructure *cstext, char *s)
{
    int pos;
    
    pos = GetTextCursorPos(cstext);
    TextInsert(cstext, pos, s);
}
#if 0
static void EnterCB(Widget w, XtPointer client_data, XtPointer call_data)
{
    fonttool_ui *ui = (fonttool_ui *) client_data;
    XbaeMatrixEnterCellCallbackStruct *cbs =
        (XbaeMatrixEnterCellCallbackStruct *) call_data;
    X11Stuff *xstuff = gapp->gui->xstuff;
    char s[7];
    unsigned char c;
    
    c = 16*cbs->row + cbs->column;
    if (ui->valid_chars[c]) {
        c = 16*cbs->row + cbs->column;
        /* TODO: check for c being displayable in the _X_ font */
        if (c > 31) {
            s[0] = (char) c;
            s[1] = '\0';
        } else {
            sprintf(s, "\\#{%02x}", c);
        }
        insert_into_string(ui->cstext, s);
    } else {
        XBell(xstuff->disp, 25);
    }
    
    cbs->doit = False;
}
#endif

static void update_fonttool_cb(OptionStructure *opt, int value, void *data)
{
    fonttool_ui *ui = (fonttool_ui *) data;
    int cwidth, cheight;

    if (ui->font_id != value) {
        ui->font_id = value;
        ui->new_font = TRUE;
    }

    TableGetCellDimentions(ui->font_table, &cwidth, &cheight);

    /* 6 = 2*cellShadowThickness + 2 */
    ui->csize = MIN2(cwidth, cheight) - 6;

    TableUpdate(ui->font_table);
}

#if 0
static void EditStringCB(Widget w, XtPointer client_data, XtPointer call_data)
{
    fonttool_ui *ui = (fonttool_ui *) client_data;
    XmTextVerifyCallbackStruct *tcbs =
        (XmTextVerifyCallbackStruct *) call_data;
    unsigned char c;
    static int column = 0, row = 0;
    XmTextBlock text;
    
    if (ui->enable_edit_cb != TRUE) {
        return;
    }
    
    text = tcbs->text;
    
    XbaeMatrixDeselectCell(ui->font_table, row, column);
    
    if (text->length == 1) {
        /* */
        c = text->ptr[0];
        row = c/16;
        column = c % 16;

        if (ui->valid_chars[c]) {
            XbaeMatrixSelectCell(ui->font_table, row, column);
        } else {
            tcbs->doit = False;
            return;
        }
    }
    
    if (ui->new_font) {
        char *buf;
        
        buf = copy_string(NULL, "\\f{");
        buf = concat_strings(buf,
            project_get_font_name_by_id(gproject_get_top(gapp->gp), ui->font_id));
        buf = concat_strings(buf, "}");
        buf = concat_strings(buf, text->ptr);
        XtFree(text->ptr);
        text->ptr = XtNewString(buf);
        text->length = strlen(buf);
        xfree(buf);

        ui->new_font = FALSE;
    }
}
#endif
static int fonttool_aac_cb(void *data)
{
    fonttool_ui *ui = (fonttool_ui *) data;
    if (ui->cstext_parent != NULL) {
        char *s = GetTextString(ui->cstext);
        int pos = GetTextCursorPos(ui->cstext);
        SetTextString(ui->cstext_parent, s);
        SetTextCursorPos(ui->cstext_parent, pos);
        xfree(s);
    }
    
    return RETURN_SUCCESS;
}
