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
 * Font tool
 *
 */

#include <config.h>

#include <X11/X.h>

#include <Xm/Xm.h>
#include <Xm/DrawingA.h>
#include <Xm/Form.h>
#include <Xm/RowColumn.h>
#include <Xm/ScrolledW.h>
#include <Xm/DialogS.h>
#include <Xm/Text.h>
#include <Xm/XmosP.h>

#include <Xbae/Matrix.h>

#include "globals.h"
#include "grace/canvas.h"
#include "utils.h"
#include "motifinc.h"
#include "protos.h"


/* used globally */
extern Widget app_shell;

static Widget fonttool_frame = NULL;
static OptionStructure *font_select_item;
static TextStructure *string_item = NULL;

static Widget cstext_parent = NULL;

static int FontID;
static int csize;

static int enable_edit_cb;

static void DrawCB(Widget w,XtPointer cd, XbaeMatrixDrawCellCallbackStruct *cbs);
static void EnterCB(Widget w, XtPointer cd, XbaeMatrixEnterCellCallbackStruct *cbs);
static void update_fonttool_cb(OptionStructure *opt, int value, void *data);
static void EditStringCB(Widget w, XtPointer client_data, XmAnyCallbackStruct *cbs);
static void fonttool_aac_cb(Widget but, void *data);

void create_fonttool_cb(Widget but, void *data)
{
    create_fonttool((Widget) data);
}

void create_fonttool(Widget cstext)
{
    int i;
    short widths[16];
    unsigned char column_alignments[16];
    Widget fonttool_panel, font_table, aac_buts;
    
    if (string_item != NULL && cstext == string_item->text) {
        /* avoid recursion */
        return;
    }
    
    if (cstext_parent != NULL) {
        /* unlock previous parent */
        SetSensitive(cstext_parent, True);
    }
    
    cstext_parent = cstext;
    
    if (fonttool_frame == NULL) {
	fonttool_frame = XmCreateDialogShell(app_shell, "Font tool", NULL, 0);
	handle_close(fonttool_frame);
        fonttool_panel = XtVaCreateWidget("fonttool_panel", xmFormWidgetClass, 
                                        fonttool_frame, NULL, 0);

        font_select_item = CreateFontChoice(fonttool_panel, "Font:");
        XtVaSetValues(font_select_item->menu,
            XmNleftAttachment, XmATTACH_FORM,
            XmNrightAttachment, XmATTACH_FORM,
            XmNtopAttachment, XmATTACH_FORM,
            NULL);
        
        for (i = 0; i < 16; i++) {
            widths[i] = 2;
            column_alignments[i] = XmALIGNMENT_BEGINNING;
        }
        font_table = XtVaCreateManagedWidget(
            "fontTable", xbaeMatrixWidgetClass, fonttool_panel,
            XmNrows, 16,
            XmNcolumns, 16,
            XmNvisibleRows, 8,
            XmNvisibleColumns, 16,
            XmNfill, True,
            XmNcolumnWidths, widths,
            XmNcolumnAlignments, column_alignments,
	    XmNgridType, XmGRID_CELL_SHADOW,
	    XmNcellShadowType, XmSHADOW_ETCHED_OUT,
	    XmNcellShadowThickness, 2,
            XmNaltRowCount, 0,
            XmNleftAttachment, XmATTACH_FORM,
            XmNrightAttachment, XmATTACH_FORM,
            XmNtopAttachment, XmATTACH_WIDGET,
            XmNtopWidget, font_select_item->menu,
            NULL);
            
        XtAddCallback(font_table, XmNdrawCellCallback, (XtCallbackProc) DrawCB, NULL);
        XtAddCallback(font_table, XmNenterCellCallback, (XtCallbackProc) EnterCB, NULL);
        AddOptionChoiceCB(font_select_item, update_fonttool_cb, font_table);

        string_item = CreateCSText(fonttool_panel, "CString:");
        XtVaSetValues(string_item->form,
            XmNleftAttachment, XmATTACH_FORM,
            XmNrightAttachment, XmATTACH_FORM,
            NULL);

        XtAddCallback(string_item->text,
            XmNmodifyVerifyCallback, (XtCallbackProc) EditStringCB, font_table);
        
        aac_buts = CreateAACButtons(fonttool_panel,
            fonttool_panel, fonttool_aac_cb);
        XtVaSetValues(aac_buts,
            XmNleftAttachment, XmATTACH_FORM,
            XmNrightAttachment, XmATTACH_FORM,
            XmNbottomAttachment, XmATTACH_FORM,
            NULL);

        XtVaSetValues(string_item->form,
            XmNbottomAttachment, XmATTACH_WIDGET,
            XmNbottomWidget, aac_buts,
            NULL);
        
        XtVaSetValues(font_table,
            XmNbottomAttachment, XmATTACH_WIDGET,
            XmNbottomWidget, string_item->form,
            NULL);
        
        update_fonttool_cb(NULL, 0, font_table);
        ManageChild(fonttool_panel);
    }

    enable_edit_cb = FALSE;
    if (cstext_parent == NULL) {
        SetTextString(string_item, "");
    } else {
        SetTextString(string_item, xv_getstr(cstext_parent));
        /* Lock editable text */
        SetSensitive(cstext_parent, False);
    }
    enable_edit_cb = TRUE;
    
    RaiseWindow(fonttool_frame);
}

static void DrawCB(Widget w, XtPointer cd, XbaeMatrixDrawCellCallbackStruct *cbs)
{
    X11Stuff *xstuff = grace->gui->xstuff;
    unsigned char c;
    Pixmap pixmap;
    char dummy_bits[1] = {0};
    int valid_char;
    
        
    c = 16*cbs->row + cbs->column;
        
    if (FontID == BAD_FONT_ID) {
        pixmap = 0;
    } else {
        pixmap = char_to_pixmap(w, FontID, c, csize);
    }
       
    if (!pixmap) {
        pixmap = XCreateBitmapFromData(xstuff->disp, xstuff->root,
             dummy_bits, 1, 1);
        valid_char = FALSE;
    } else {
        valid_char = TRUE;
    }
    
    /* Assign it a pixmap */
    cbs->pixmap = pixmap;
    cbs->type = XbaePixmap;
    XbaeMatrixSetCellUserData(w, cbs->row, cbs->column, (XtPointer) valid_char);  
   
    return;
}

static void insert_into_string(char *s)
{
    int pos;
    
    pos = GetTextCursorPos(string_item);
    TextInsert(string_item, pos, s);
}

static void EnterCB(Widget w, XtPointer cd, XbaeMatrixEnterCellCallbackStruct *cbs)
{
    X11Stuff *xstuff = grace->gui->xstuff;
    int valid_char;
    char s[7];
    unsigned char c;
    
    valid_char = (int) XbaeMatrixGetCellUserData(w, cbs->row, cbs->column);
    if (valid_char == TRUE) {
        c = 16*cbs->row + cbs->column;
        /* TODO: check for c being displayable in the _X_ font */
        if (c > 31) {
            s[0] = (char) c;
            s[1] = '\0';
        } else {
            sprintf(s, "\\#{%02x}", c);
        }
        insert_into_string(s);
    } else {
        XBell(xstuff->disp, 25);
    }
}


static void update_fonttool_cb(OptionStructure *opt, int value, void *data)
{
    Widget font_table = (Widget) data;
    int x0, y0, x1, y1, cwidth, cheight;
    char *buf;
    FontID = value;
    
    XbaeMatrixRowColToXY(font_table, 0, 0, &x0, &y0);
    XbaeMatrixRowColToXY(font_table, 1, 1, &x1, &y1);
    cwidth  = x1 - x0;
    cheight = y1 - y0;
    
    /* 6 = 2*cellShadowThickness + 2 */
    csize = MIN2(cwidth, cheight) - 6;

    buf = copy_string(NULL, "\\f{");
    buf = concat_strings(buf, get_fontalias(grace->rt->canvas, FontID));
    buf = concat_strings(buf, "}");
    insert_into_string(buf);
    xfree(buf);

    XbaeMatrixRefresh(font_table);
}


static void EditStringCB(Widget w, XtPointer client_data, XmAnyCallbackStruct *cbs)
{
    unsigned char c;
    int valid_char;
    static int column = 0, row = 0;
    XmTextVerifyCallbackStruct *tcbs;
    XmTextBlock text;
    Widget ftable = (Widget) client_data;
    
    if (enable_edit_cb != TRUE) {
        return;
    }
    
    XbaeMatrixDeselectCell(ftable, row, column);
    
    tcbs = (XmTextVerifyCallbackStruct *) cbs;
    
    text = tcbs->text;
    
    if (text->length == 1) {
        /* */
        c = text->ptr[0];
        row = c/16;
        column = c % 16;

        valid_char = (int) XbaeMatrixGetCellUserData(ftable, row, column);
        if (valid_char == TRUE) {
            XbaeMatrixSelectCell(ftable, row, column);
        } else {
            tcbs->doit = False;
        }
    }
}

static void fonttool_aac_cb(Widget but, void *data)
{
    int aac_mode;
    
    aac_mode = (int) data;
    
    if (aac_mode == AAC_CLOSE) {
        UnmanageChild(fonttool_frame);
        if (cstext_parent != NULL) {
            SetSensitive(cstext_parent, True);
        }
        return;
    }

    if (cstext_parent != NULL) {
        char *s = GetTextString(string_item);
        xv_setstr(cstext_parent, s);
        xfree(s);
    }
    
    if (aac_mode == AAC_ACCEPT) {
        UnmanageChild(fonttool_frame);
        if (cstext_parent != NULL) {
            SetSensitive(cstext_parent, True);
        }
    }
}
