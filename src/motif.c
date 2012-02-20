/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 *
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 *
 * Copyright (c) 2012 Grace Development Team
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

/* Motif widgets */

#include "widgets.h"

#include "events.h"

#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/Text.h>
#include <Xm/RowColumn.h>

void ManageChild(Widget w)
{
    XtManageChild(w);
}

void UnmanageChild(Widget w)
{
    XtUnmanageChild(w);
}

int IsManaged(Widget w)
{
    return (XtIsManaged(w) == True) ? TRUE:FALSE;
}

void *GetUserData(Widget w)
{
    void *udata = NULL;
    XtVaGetValues(w, XmNuserData, &udata, NULL);

    return udata;
}

void SetUserData(Widget w, void *udata)
{
    XtVaSetValues(w, XmNuserData, udata, NULL);
}

void SetSensitive(Widget w, int onoff)
{
    XtSetSensitive(w, onoff ? True : False);
}

static int toolkit_modifiers_to_grace_modifiers(void *event)
{
    XKeyEvent *xke;
    int modifiers = NO_MODIFIER;

    xke = (XKeyEvent *) event;

    if (xke->state & ControlMask) {
        modifiers = modifiers ^ CONTROL_MODIFIER;
    }

    if (xke->state & ShiftMask) {
        modifiers = modifiers ^ SHIFT_MODIFIER;
    }

    return modifiers;
}

static int toolkit_key_to_grace_key(void *event)
{
    XKeyEvent *xke;
    KeySym keybuf;

    xke = (XKeyEvent *) event;
    keybuf = XLookupKeysym(xke, 0);

    switch (keybuf) {
    case XK_e: /* e */
        return KEY_E;
    case XK_Up: /* Up */
        return KEY_UP;
    case XK_Down: /* Down */
        return KEY_DOWN;
    case XK_Return: /* Return */
        return KEY_RETURN;
    default:
        return KEY_NONE;
    }
}

static void keyCB(Widget w, XtPointer client_data, XEvent *event, Boolean *cont)
{
    KeyEvent kevent;
    Key_CBData *cbdata = (Key_CBData *) client_data;

    kevent.w = w;
    kevent.anydata = cbdata->anydata;

    if (cbdata->key != toolkit_key_to_grace_key(event)) return;
    if (cbdata->modifiers != toolkit_modifiers_to_grace_modifiers(event)) return;

    cbdata->cbproc(&kevent);

    *cont = False;
}

void AddWidgetKeyPressCB2(Widget w, int modifiers, int key, Key_CBProc cbproc, void *anydata)
{
    Key_CBData *cbdata;

    cbdata = (Key_CBData *) xmalloc(sizeof(Key_CBData));
    cbdata->w = w;
    cbdata->modifiers = modifiers;
    cbdata->key = key;
    cbdata->cbproc = cbproc;
    cbdata->anydata = anydata;

    XtAddEventHandler(w, KeyPressMask, False, keyCB, cbdata);
}

void AddWidgetKeyPressCB(Widget w, int key, Key_CBProc cbproc, void *anydata)
{
    AddWidgetKeyPressCB2(w, NO_MODIFIER, key, cbproc, anydata);
}

Widget CreateVContainer(Widget parent)
{
    Widget rc;

    rc = XmCreateRowColumn(parent, "VContainer", NULL, 0);
    ManageChild(rc);

    return rc;
}

Widget CreateHContainer(Widget parent)
{
    Widget rc;

    rc = XmCreateRowColumn(parent, "HContainer", NULL, 0);
    XtVaSetValues(rc, XmNorientation, XmHORIZONTAL, NULL);
    ManageChild(rc);

    return rc;
}

Widget CreateForm(Widget parent)
{
    return XmCreateForm(parent, "form", NULL, 0);
}

void SetDialogFormResizable(Widget form, int onoff)
{
    XtVaSetValues(form,
        XmNresizePolicy, onoff ? XmRESIZE_ANY:XmRESIZE_NONE,
        NULL);
    XtVaSetValues(XtParent(form),
        XmNallowShellResize, onoff ? True:False,
        NULL);
}

void AddDialogFormChild(Widget form, Widget child)
{
    Widget last_widget;

    last_widget = GetUserData(form);
    if (last_widget) {
        XtVaSetValues(child,
            XmNtopAttachment, XmATTACH_WIDGET,
            XmNtopWidget, last_widget,
            NULL);
        XtVaSetValues(last_widget,
            XmNbottomAttachment, XmATTACH_NONE,
            NULL);
    } else {
        XtVaSetValues(child,
            XmNtopAttachment, XmATTACH_FORM,
            NULL);
    }
    XtVaSetValues(child,
        XmNleftAttachment, XmATTACH_FORM,
        XmNrightAttachment, XmATTACH_FORM,
        XmNbottomAttachment, XmATTACH_FORM,
        NULL);
    SetUserData(form, child);
}

void FixateDialogFormChild(Widget w)
{
    Widget prev;
    XtVaGetValues(w, XmNtopWidget, &prev, NULL);
    XtVaSetValues(w, XmNtopAttachment, XmATTACH_NONE, NULL);
    XtVaSetValues(prev, XmNbottomAttachment, XmATTACH_WIDGET,
        XmNbottomWidget, w,
        NULL);
}

Widget CreateLabel(Widget parent, char *s)
{
    Widget label;

    label = XtVaCreateManagedWidget("label",
        xmLabelWidgetClass, parent,
        XmNalignment, XmALIGNMENT_BEGINNING,
        XmNrecomputeSize, True,
        NULL);

    SetLabel(label, s);

    return label;
}

void SetLabel(Widget w, char *s)
{
    XmString str;

    str = XmStringCreateLocalized(s);
    XtVaSetValues(w, XmNlabelString, str, NULL);
    XmStringFree(str);
}

void AlignLabel(Widget w, int alignment)
{
    unsigned char xm_alignment;

    switch(alignment) {
    case ALIGN_BEGINNING:
        xm_alignment = XmALIGNMENT_BEGINNING;
        break;
    case ALIGN_CENTER:
        xm_alignment = XmALIGNMENT_CENTER;
        break;
    case ALIGN_END:
        xm_alignment = XmALIGNMENT_END;
        break;
    default:
        errmsg("Internal error in AlignLabel()");
        return;
        break;
    }
    XtVaSetValues(w,
        XmNalignment, xm_alignment,
        NULL);
}

Widget CreateLineTextEdit(Widget parent, int len)
{
    return XtVaCreateManagedWidget("text", xmTextWidgetClass, parent,
                                   XmNtraversalOn, True,
                                   XmNcolumns, len,
                                   NULL);
}

Widget CreateTextItem(Widget parent, int len, char *label)
{
    Widget rc;

    rc = CreateHContainer(parent);
    CreateLabel(rc, label);

    return CreateLineTextEdit(rc, len);
}

typedef struct {
    TItem_CBProc cbproc;
    void *anydata;
} TItem_CBdata;

static void titem_int_cb_proc(KeyEvent *event)
{
    char *s;
    TItem_CBdata *cbdata = (TItem_CBdata *) event->anydata;

    s = XmTextGetString(event->w);
    cbdata->cbproc(event->w, s, cbdata->anydata);
    XtFree(s);
}

void AddTextItemCB(Widget ti, TItem_CBProc cbproc, void *data)
{
    TItem_CBdata *cbdata;

    cbdata = xmalloc(sizeof(TItem_CBdata));
    cbdata->anydata = data;
    cbdata->cbproc = cbproc;
    AddWidgetKeyPressCB(ti, KEY_RETURN, titem_int_cb_proc, cbdata);
}

TextStructure *CreateTextInput(Widget parent, char *s)
{
    TextStructure *retval;
    XmString str;

    retval = xmalloc(sizeof(TextStructure));
    retval->form = XtVaCreateWidget("form", xmFormWidgetClass, parent, NULL);

    str = XmStringCreateLocalized(s);
    retval->label = XtVaCreateManagedWidget("label",
        xmLabelWidgetClass, retval->form,
        XmNlabelString, str,
        XmNtopAttachment, XmATTACH_FORM,
        XmNbottomAttachment, XmATTACH_FORM,
        XmNleftAttachment, XmATTACH_FORM,
        XmNrightAttachment, XmATTACH_NONE,
        NULL);
    XmStringFree(str);

    retval->text = XtVaCreateManagedWidget("cstext",
        xmTextWidgetClass, retval->form,
        XmNtraversalOn, True,
        XmNtopAttachment, XmATTACH_FORM,
        XmNbottomAttachment, XmATTACH_FORM,
        XmNleftAttachment, XmATTACH_WIDGET,
        XmNleftWidget, retval->label,
        XmNrightAttachment, XmATTACH_FORM,
        NULL);

    ManageChild(retval->form);

    return retval;
}

/*
 * create a multiline editable window
 * parent = parent widget
 * nrows  = number of lines in the window
 * s      = label for window
 */
TextStructure *CreateScrolledTextInput(Widget parent, char *s, int nrows)
{
    TextStructure *retval;
    XmString str;
    Arg args[3];
    int ac;

    retval = xmalloc(sizeof(TextStructure));
    retval->form = XtVaCreateWidget("form", xmFormWidgetClass, parent, NULL);

    str = XmStringCreateLocalized(s);
    retval->label = XtVaCreateManagedWidget("label",
        xmLabelWidgetClass, retval->form,
        XmNlabelString, str,
        XmNtopAttachment, XmATTACH_FORM,
        XmNleftAttachment, XmATTACH_FORM,
        XmNrightAttachment, XmATTACH_FORM,
        NULL);
    XmStringFree(str);

    ac = 0;
    if (nrows > 0) {
        XtSetArg(args[ac], XmNrows, nrows); ac++;
    }
    XtSetArg(args[ac], XmNeditMode, XmMULTI_LINE_EDIT); ac++;
    XtSetArg(args[ac], XmNvisualPolicy, XmVARIABLE); ac++;
    retval->text = XmCreateScrolledText(retval->form, "text", args, ac);
    XtVaSetValues(XtParent(retval->text),
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, retval->label,
        XmNleftAttachment, XmATTACH_FORM,
        XmNrightAttachment, XmATTACH_FORM,
        XmNbottomAttachment, XmATTACH_FORM,
        NULL);
    ManageChild(retval->text);

    ManageChild(retval->form);
    return retval;
}


static void cstext_edit_action(KeyEvent *event)
{
    TextStructure *cst = (TextStructure *) event->anydata;
    create_fonttool(cst);
}

TextStructure *CreateCSText(Widget parent, char *s)
{
    TextStructure *retval;

    retval = CreateTextInput(parent, s);
    AddWidgetKeyPressCB2(retval->text, CONTROL_MODIFIER, KEY_E, cstext_edit_action, retval);

    return retval;
}

TextStructure *CreateScrolledCSText(Widget parent, char *s, int nrows)
{
    TextStructure *retval;

    retval = CreateScrolledTextInput(parent, s, nrows);
    AddWidgetKeyPressCB2(retval->text, CONTROL_MODIFIER, KEY_E, cstext_edit_action, retval);

    return retval;
}

void SetTextInputLength(TextStructure *cst, int len)
{
    XtVaSetValues(cst->text, XmNcolumns, len, NULL);
}

char *GetTextString(TextStructure *cst)
{
    char *s, *buf;

    s = XmTextGetString(cst->text);
    buf = copy_string(NULL, s);
    XtFree(s);

    return buf;
}

void SetTextString(TextStructure *cst, char *s)
{
    cst->locked = TRUE;

    XmTextSetString(cst->text, s ? s : "");
    XmTextSetInsertionPosition(cst->text, s ? strlen(s):0);
}


typedef struct {
    TextStructure *cst;
    Text_CBProc cbproc;
    void *anydata;
    Widget w;
    XtIntervalId timeout_id;
} Text_CBdata;

static void text_timer_proc(XtPointer client_data, XtIntervalId *id)
{
    char *s;
    Text_CBdata *cbdata = (Text_CBdata *) client_data;

    s = XmTextGetString(cbdata->w);
    cbdata->cbproc(cbdata->cst, s, cbdata->anydata);
    XtFree(s);
    cbdata->timeout_id = (XtIntervalId) 0;
}

/* Text input timeout [ms] */
#define TEXT_TIMEOUT    0
extern XtAppContext app_con;

static void text_int_mv_cb_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    Text_CBdata *cbdata = (Text_CBdata *) client_data;

    if (cbdata->cst->locked) {
        cbdata->cst->locked = FALSE;
        return;
    }
    cbdata->w = w;
    /* we count elapsed time since the last event, so first remove
       an existing timeout, if there is one */
    if (cbdata->timeout_id) {
        XtRemoveTimeOut(cbdata->timeout_id);
    }

    if (TEXT_TIMEOUT) {
        cbdata->timeout_id = XtAppAddTimeOut(app_con,
            TEXT_TIMEOUT, text_timer_proc, client_data);
    }
}

static void text_int_cb_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    char *s;
    Text_CBdata *cbdata = (Text_CBdata *) client_data;
    s = XmTextGetString(w);
    cbdata->cbproc(cbdata->cst, s, cbdata->anydata);
    XtFree(s);
}

void AddTextInputCB(TextStructure *cst, Text_CBProc cbproc, void *data)
{
    Text_CBdata *cbdata;

    cbdata = xmalloc(sizeof(Text_CBdata));
    cbdata->cst = cst;
    cbdata->anydata = data;
    cbdata->cbproc = cbproc;
    cbdata->timeout_id = (XtIntervalId) 0;
    cbdata->cst->locked = FALSE;

    XtAddCallback(cst->text,
        XmNactivateCallback, text_int_cb_proc, (XtPointer) cbdata);
    XtAddCallback(cst->text,
        XmNmodifyVerifyCallback, text_int_mv_cb_proc, (XtPointer) cbdata);
}

static void text_int_validate_cb_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    XmTextBlock text;
    TextValidate_CBData *cbdata = (TextValidate_CBData *) client_data;
    XmTextVerifyCallbackStruct *tcbs =
            (XmTextVerifyCallbackStruct *) call_data;

    text = tcbs->text;

    if (!cbdata->cbproc(&text->ptr, &text->length, cbdata->anydata)) {
        tcbs->doit = FALSE;
    }
}

void AddTextValidateCB(TextStructure *cst, TextValidate_CBProc cbproc, void *anydata)
{
    TextValidate_CBData *cbdata;

    cbdata = (TextValidate_CBData *) xmalloc(sizeof(TextValidate_CBData));
    cbdata->w = cst->text;
    cbdata->cbproc = cbproc;
    cbdata->anydata = anydata;

    XtAddCallback(cst->text, XmNmodifyVerifyCallback, text_int_validate_cb_proc, cbdata);
}

int GetTextCursorPos(TextStructure *cst)
{
    return XmTextGetInsertionPosition(cst->text);
}

void SetTextCursorPos(TextStructure *cst, int pos)
{
    XmTextSetInsertionPosition(cst->text, pos);
}

int GetTextLastPosition(TextStructure *cst)
{
    return XmTextGetLastPosition(cst->text);
}

void TextInsert(TextStructure *cst, int pos, char *s)
{
    XmTextInsert(cst->text, pos, s);
}

void SetTextEditable(TextStructure *cst, int onoff)
{
    XtVaSetValues(cst->text, XmNeditable, onoff? True:False, NULL);
}

