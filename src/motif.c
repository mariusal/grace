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

#include "globals.h"

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

void AddWidgetKeyPressCB(Widget w, int key, Key_CBProc cbproc, void *anydata)
{
    AddWidgetKeyPressCB2(w, NO_MODIFIER, key, cbproc, anydata);
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

void DialogSetResizable(Widget form, int onoff)
{
    XtVaSetValues(form,
        XmNresizePolicy, onoff ? XmRESIZE_ANY:XmRESIZE_NONE,
        NULL);
    XtVaSetValues(XtParent(form),
        XmNallowShellResize, onoff ? True:False,
        NULL);
}

Widget CreateForm(Widget parent)
{
    Widget w;

    w = XmCreateForm(parent, "form", NULL, 0);
    ManageChild(w);

    return w;
}

void FormAddHChild(Widget form, Widget child)
{
    Widget last_widget;

    last_widget = GetUserData(form);
    if (last_widget) {
        XtVaSetValues(child,
            XmNleftAttachment, XmATTACH_WIDGET,
            XmNleftWidget, last_widget,
            NULL);
        XtVaSetValues(last_widget,
            XmNrightAttachment, XmATTACH_NONE,
            NULL);
    } else {
        XtVaSetValues(child,
            XmNleftAttachment, XmATTACH_FORM,
            NULL);
    }
    XtVaSetValues(child,
        XmNtopAttachment, XmATTACH_FORM,
        XmNbottomAttachment, XmATTACH_FORM,
        XmNrightAttachment, XmATTACH_FORM,
        NULL);
    SetUserData(form, child);
}

void FormAddVChild(Widget form, Widget child)
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

void FormFixateVChild(Widget w)
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

Widget CreateLineTextEdit(Widget parent, int len)
{
    Widget w;

    w = XtVaCreateManagedWidget("text", xmTextWidgetClass, parent,
                                   XmNtraversalOn, True,
                                   NULL);
    if (len > 0) {
        XtVaSetValues(w, XmNcolumns, len, NULL);
    }

    return w;
}

Widget CreateMultiLineTextEdit(Widget parent, int nrows)
{
    Widget w;
    Arg args[3];
    int ac;

    ac = 0;
    if (nrows > 0) {
        XtSetArg(args[ac], XmNrows, nrows); ac++;
    }
    XtSetArg(args[ac], XmNeditMode, XmMULTI_LINE_EDIT); ac++;
    XtSetArg(args[ac], XmNvisualPolicy, XmVARIABLE); ac++;

    w = XmCreateScrolledText(parent, "text", args, ac);
    ManageChild(w);

    return w;
}

TextStructure *CreateTextInput(Widget parent, char *s)
{
    return CreateTextInput2(parent, s, 0);
}

TextStructure *CreateTextInput2(Widget parent, char *s, int len)
{
    TextStructure *retval;

    retval = xmalloc(sizeof(TextStructure));
    retval->form = CreateForm(parent);

    retval->label = CreateLabel(retval->form, s);
    FormAddHChild(retval->form, retval->label);

    retval->text = CreateLineTextEdit(retval->form, len);
    FormAddHChild(retval->form, retval->text);

    retval->multiline = FALSE;

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

    retval = xmalloc(sizeof(TextStructure));
    retval->form = CreateForm(parent);

    retval->label = CreateLabel(retval->form, s);
    FormAddVChild(retval->form, retval->label);

    retval->text = CreateMultiLineTextEdit(retval->form, nrows);
    FormAddVChild(retval->form, XtParent(retval->text));

    retval->multiline = TRUE;

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
    XmTextSetString(cst->text, s ? s : "");
    XmTextSetInsertionPosition(cst->text, s ? strlen(s):0);
}

/*
 * xv_evalexpr - take a text field and pass it to the parser to evaluate
 */
int xv_evalexpr(TextStructure *cst, double *answer)
{
    int retval;
    char *s;

    s = GetTextString(cst);

    retval = graal_eval_expr(grace_get_graal(gapp->grace),
        s, answer, gproject_get_top(gapp->gp));

    xfree(s);

    return retval;
}

/*
 * xv_evalexpri - as xv_evalexpr, but for integers
 */
int xv_evalexpri(TextStructure *cst, int *answer)
{
    int retval;
    double buf;

    retval = xv_evalexpr(cst, &buf);

    *answer = rint(buf);

    return retval;
}

typedef struct {
    TextStructure *cst;
    Text_CBProc cbproc;
    void *anydata;
} Text_CBdata;

static void text_int_cb_proc(KeyEvent *event)
{
    char *s;
    Text_CBdata *cbdata = (Text_CBdata *) event->anydata;

    s = GetTextString(cbdata->cst);
    cbdata->cbproc(cbdata->cst, s, cbdata->anydata);
    xfree(s);
}

void AddTextInputCB(TextStructure *cst, Text_CBProc cbproc, void *data)
{
    Text_CBdata *cbdata;

    cbdata = xmalloc(sizeof(Text_CBdata));
    cbdata->cst = cst;
    cbdata->anydata = data;
    cbdata->cbproc = cbproc;

    if (cst->multiline) {
        AddWidgetKeyPressCB2(cst->text, CONTROL_MODIFIER, KEY_RETURN, text_int_cb_proc, cbdata);
    } else {
        AddWidgetKeyPressCB(cst->text, KEY_RETURN, text_int_cb_proc, cbdata);
    }
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

