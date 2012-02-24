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

#include <ctype.h>
#include <stdarg.h>
#include <Xm/Xm.h>
#include <Xm/CascadeBG.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/Text.h>
#include <Xm/ToggleB.h>
#include "Tab.h"

/* Widgets */
Widget WidgetGetParent(Widget w)
{
    return XtParent(w);
}

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
    default:
        return KEY_NONE;
    }
}

static void action(Widget w, XEvent *event, String *par, Cardinal *npar)
{
}

static void keyHook(Widget w, XtPointer client_data, String action_name,
                    XEvent *event, String *params, Cardinal *num_params)
{
    Key_CBData *cbdata = (Key_CBData *) client_data;

    if (strcmp(action_name, "action")) return;

    /* In case if we have the same widget */
    if (cbdata->key != toolkit_key_to_grace_key(event)) return;
    if (cbdata->modifiers != toolkit_modifiers_to_grace_modifiers(event)) return;

    if (w != cbdata->w) return;

    cbdata->cbproc(cbdata->anydata);
}

extern XtAppContext app_con;

void AddWidgetKeyPressCB(Widget w, int key, Key_CBProc cbproc, void *anydata)
{
    AddWidgetKeyPressCB2(w, NO_MODIFIER, key, cbproc, anydata);
}

void AddWidgetKeyPressCB2(Widget w, int modifiers, int key, Key_CBProc cbproc, void *anydata)
{
    char *table = NULL;
    XtActionsRec actions[1];
    Key_CBData *cbdata;

    cbdata = (Key_CBData *) xmalloc(sizeof(Key_CBData));
    cbdata->w = w;
    cbdata->modifiers = modifiers;
    cbdata->key = key;
    cbdata->cbproc = cbproc;
    cbdata->anydata = anydata;

    modifiers = modifiers ^ NO_MODIFIER;

    if (modifiers & CONTROL_MODIFIER) {
        table = copy_string(table, "Ctrl");
    }

    switch (key) {
    case KEY_E:
        table = concat_strings(table, "<Key>E: action()");
        break;
    case KEY_UP:
        table = concat_strings(table, "<Key>osfUp: action()");
        break;
    case KEY_DOWN:
        table = concat_strings(table, "<Key>osfDown: action()");
        break;
    default:
        return;
    }

    actions[0].string = "action";
    actions[0].proc = action;

    XtOverrideTranslations(w, XtParseTranslationTable(table));
    XtAppAddActions(app_con, actions, XtNumber(actions));
    XtAppAddActionHook(app_con, keyHook, cbdata);

    xfree(table);
}

static void widgetCB(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget_CBData *cbdata = (Widget_CBData *) client_data;

    cbdata->calldata = call_data;

    cbdata->cbproc(cbdata);
}

void AddWidgetCB(Widget w, const char *callback, Widget_CBProc cbproc, void *anydata)
{
    Widget_CBData *cbdata;

    cbdata = (Widget_CBData *) xmalloc(sizeof(Widget_CBData));
    cbdata->w = w;
    cbdata->cbproc = cbproc;
    cbdata->anydata = anydata;

    if (!strcmp(callback, "valueChanged"))
        XtAddCallback(w, XmNvalueChangedCallback, widgetCB, (XtPointer) cbdata);

    if (!strcmp(callback, "activate"))
        XtAddCallback(w,  XmNactivateCallback, widgetCB, (XtPointer) cbdata);

    if (!strcmp(callback, "modifyVerify"))
        XtAddCallback(w,  XmNmodifyVerifyCallback, widgetCB, (XtPointer) cbdata);
}

/* Dialog */
void DialogRaise(Widget form)
{
    Widget w = WidgetGetParent(form);

    ManageChild(w);
    XMapRaised(XtDisplay(w), XtWindow(w));
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

/* Containers */
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

/* Form */
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

/* Label */
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

/* Text */
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

void TextSetLength(TextStructure *cst, int len)
{
    XtVaSetValues(cst->text, XmNcolumns, len, NULL);
}

char *TextGetString(TextStructure *cst)
{
    char *s, *buf;

    s = XmTextGetString(cst->text);
    buf = copy_string(NULL, s);
    XtFree(s);

    return buf;
}

void TextSetString(TextStructure *cst, char *s)
{
    cst->locked = TRUE;
    XmTextSetString(cst->text, s ? s : "");
    XmTextSetInsertionPosition(cst->text, s ? strlen(s):0);
    cst->locked = FALSE;
}

typedef struct {
    TextStructure *cst;
    TextValidate_CBProc cbproc;
    void *anydata;
} TextValidate_CBData;

static void text_int_validate_cb_proc(Widget_CBData *wcbdata)
{
    XmTextBlock text;
    TextValidate_CBData *cbdata = (TextValidate_CBData *) wcbdata->anydata;
    XmTextVerifyCallbackStruct *tcbs =
            (XmTextVerifyCallbackStruct *) wcbdata->calldata;

    if (cbdata->cst->locked) return;

    text = tcbs->text;

    if (!cbdata->cbproc(&text->ptr, &text->length, cbdata->anydata)) {
        tcbs->doit = False;
    }
}

void AddTextValidateCB(TextStructure *cst, TextValidate_CBProc cbproc, void *anydata)
{
    TextValidate_CBData *cbdata;

    cbdata = (TextValidate_CBData *) xmalloc(sizeof(TextValidate_CBData));
    cbdata->cst = cst;
    cbdata->cbproc = cbproc;
    cbdata->anydata = anydata;
    cst->locked = FALSE;

    AddWidgetCB(cst->text, "modifyVerify", text_int_validate_cb_proc, cbdata);
}

int TextGetCursorPos(TextStructure *cst)
{
    return XmTextGetInsertionPosition(cst->text);
}

void TextSetCursorPos(TextStructure *cst, int pos)
{
    XmTextSetInsertionPosition(cst->text, pos);
}

int TextGetLastPosition(TextStructure *cst)
{
    return XmTextGetLastPosition(cst->text);
}

void TextInsert(TextStructure *cst, int pos, char *s)
{
    XmTextInsert(cst->text, pos, s);
}

void TextSetEditable(TextStructure *cst, int onoff)
{
    XtVaSetValues(cst->text, XmNeditable, onoff? True:False, NULL);
}

/* Tab */
Widget CreateTab(Widget parent)
{
    Widget tab;

    tab = XtVaCreateManagedWidget("tab", xmTabWidgetClass, parent, NULL);

    return tab;
}

Widget CreateTabPage(Widget parent, char *s)
{
    Widget w;
    XmString str;

    w = CreateVContainer(parent);
    str = XmStringCreateLocalized(s);
    XtVaSetValues(w, XmNtabLabel, str, NULL);
    XmStringFree(str);

    return w;
}

void SelectTabPage(Widget tab, Widget w)
{
    XmTabSetTabWidget(tab, w, True);
}

/* Button */
Widget CreateButton(Widget parent, char *label)
{
    Widget button;
    XmString xmstr;

    button = XtVaCreateManagedWidget("button",
        xmPushButtonWidgetClass, parent,

        NULL);

    xmstr = XmStringCreateLocalized(label);
    XtVaSetValues(button,
            XmNlabelString, xmstr,
            XmNalignment, XmALIGNMENT_CENTER,
            NULL);
    XmStringFree(xmstr);

    return button;
}

Widget CreateBitmapButton(Widget parent,
    int width, int height, const unsigned char *bits)
{
    Widget button;
    Pixmap pm;

    button = XtVaCreateWidget("button",
        xmPushButtonWidgetClass, parent,
        NULL);

    pm = CreatePixmapFromBitmap(button, width, height, bits);

    XtVaSetValues(button,
            XmNlabelType, XmPIXMAP,
            XmNlabelPixmap, pm,
            NULL);

    ManageChild(button);

    return button;
}

/* ToggleButton */
Widget CreateToggleButton(Widget parent, char *s)
{
    return (XtVaCreateManagedWidget(s, xmToggleButtonWidgetClass, parent, NULL));
}

int GetToggleButtonState(Widget w)
{
    if (!w) {
        errmsg("Internal error: GetToggleButtonState() called with NULL widget");
        return 0;
    } else {
        return XmToggleButtonGetState(w);
    }
}

void SetToggleButtonState(Widget w, int value)
{
    if (w == NULL) {
        return;
    }
    XmToggleButtonSetState(w, value ? True:False, False);

    return;
}

/* Grid */
typedef struct {
    int ncols;
    int nrows;
} GridData;

Widget CreateGrid(Widget parent, int ncols, int nrows)
{
    Widget w;
    int nfractions;
    GridData *gd;

    if (ncols <= 0 || nrows <= 0) {
        errmsg("Wrong call to CreateGrid()");
        ncols = 1;
        nrows = 1;
    }

    nfractions = 0;
    do {
        nfractions++;
    } while (nfractions % ncols || nfractions % nrows);

    gd = xmalloc(sizeof(GridData));
    gd->ncols = ncols;
    gd->nrows = nrows;

    w = CreateForm(parent);

    XtVaSetValues(w,
        XmNfractionBase, nfractions,
        XmNuserData, gd,
        NULL);

    return w;
}

void PlaceGridChild(Widget grid, Widget w, int col, int row)
{
    int nfractions, w1, h1;
    GridData *gd;

    XtVaGetValues(grid,
        XmNfractionBase, &nfractions,
        XmNuserData, &gd,
        NULL);

    if (gd == NULL) {
        /* errmsg("PlaceGridChild() called with a non-grid widget"); */
        return;
    }
    if (col < 0 || col >= gd->ncols) {
        errmsg("PlaceGridChild() called with wrong `col' argument");
        return;
    }
    if (row < 0 || row >= gd->nrows) {
        errmsg("PlaceGridChild() called with wrong `row' argument");
        return;
    }

    w1 = nfractions/gd->ncols;
    h1 = nfractions/gd->nrows;

    XtVaSetValues(w,
        XmNleftAttachment  , XmATTACH_POSITION,
        XmNleftPosition    , col*w1           ,
        XmNrightAttachment , XmATTACH_POSITION,
        XmNrightPosition   , (col + 1)*w1     ,
        XmNtopAttachment   , XmATTACH_POSITION,
        XmNtopPosition     , row*h1           ,
        XmNbottomAttachment, XmATTACH_POSITION,
        XmNbottomPosition  , (row + 1)*h1     ,
        NULL);
}

/* OptionChoice */
#define MAX_PULLDOWN_LENGTH 30

OptionStructure *CreateOptionChoice(Widget parent, char *labelstr,
    int ncols, int nchoices, OptionItem *items)
{
    Arg args[2];
    XmString str;
    OptionStructure *retval;

    retval = xcalloc(1, sizeof(OptionStructure));
    if (!retval) {
        return NULL;
    }

    XtSetArg(args[0], XmNpacking, XmPACK_COLUMN);
    retval->pulldown = XmCreatePulldownMenu(parent, "pulldownMenu", args, 1);

    retval->ncols = ncols;

    UpdateOptionChoice(retval, nchoices, items);

    str = XmStringCreateLocalized(labelstr);
    XtSetArg(args[0], XmNlabelString, str);
    XtSetArg(args[1], XmNsubMenuId, retval->pulldown);

    retval->menu = XmCreateOptionMenu(parent, "optionMenu", args, 2);

    XmStringFree(str);

    ManageChild(retval->menu);

    return retval;
}

OptionStructure *CreateOptionChoiceVA(Widget parent, char *labelstr, ...)
{
    OptionStructure *retval;
    int nchoices = 0;
    OptionItem *oi = NULL;
    va_list var;
    char *s;
    int value;

    va_start(var, labelstr);
    while ((s = va_arg(var, char *)) != NULL) {
        value = va_arg(var, int);
        nchoices++;
        oi = xrealloc(oi, nchoices*sizeof(OptionItem));
        oi[nchoices - 1].value = value;
        oi[nchoices - 1].label = copy_string(NULL, s);
    }
    va_end(var);

    retval = CreateOptionChoice(parent, labelstr, 1, nchoices, oi);

    while (nchoices) {
        nchoices--;
        xfree(oi[nchoices].label);
    }
    xfree(oi);

    return retval;
}

static void oc_int_cb_proc(Widget_CBData *wcbdata)
{
    int value;

    OC_CBdata *cbdata = (OC_CBdata *) wcbdata->anydata;

    value = GetOptionChoice(cbdata->opt);
    cbdata->cbproc(cbdata->opt, value, cbdata->anydata);
}

void AddOptionChoiceCB(OptionStructure *opt, OC_CBProc cbproc, void *anydata)
{
    OC_CBdata *cbdata;
    unsigned int i;

    cbdata = xmalloc(sizeof(OC_CBdata));

    cbdata->opt = opt;
    cbdata->cbproc = cbproc;
    cbdata->anydata = anydata;

    opt->cblist = xrealloc(opt->cblist, (opt->cbnum + 1)*sizeof(OC_CBdata *));
    opt->cblist[opt->cbnum] = cbdata;
    opt->cbnum++;

    for (i = 0; i < opt->nchoices; i++) {
        AddWidgetCB(opt->options[i].widget, "activate", oc_int_cb_proc, cbdata);
    }
}

void UpdateOptionChoice(OptionStructure *optp, int nchoices, OptionItem *items)
{
    int i, nold, ncols, nw;
    Widget *wlist;

    nold = optp->nchoices;

    if (optp->ncols == 0) {
        ncols = 1;
    } else {
        ncols = optp->ncols;
    }

    /* Don't create too tall pulldowns */
    if (nchoices > MAX_PULLDOWN_LENGTH*ncols) {
        ncols = (nchoices + MAX_PULLDOWN_LENGTH - 1)/MAX_PULLDOWN_LENGTH;
    }

    XtVaSetValues(optp->pulldown, XmNnumColumns, ncols, NULL);

    nw = nold - nchoices;
    if (nw > 0) {
        /* Unmanage extra items before destroying to speed the things up */
        wlist = xmalloc(nw*sizeof(Widget));
        for (i = nchoices; i < nold; i++) {
            wlist[i - nchoices] = optp->options[i].widget;
        }
        XtUnmanageChildren(wlist, nw);
        xfree(wlist);

        for (i = nchoices; i < nold; i++) {
            XtDestroyWidget(optp->options[i].widget);
        }
    }

    optp->options = xrealloc(optp->options, nchoices*sizeof(OptionWidgetItem));
    optp->nchoices = nchoices;

    for (i = nold; i < nchoices; i++) {
        unsigned int j;
        optp->options[i].widget = CreateButton(optp->pulldown, "");
        for (j = 0; j < optp->cbnum; j++) {
            OC_CBdata *cbdata = optp->cblist[j];
            AddWidgetCB(optp->options[i].widget, "activate", oc_int_cb_proc, cbdata);
        }
    }

    for (i = 0; i < nchoices; i++) {
        optp->options[i].value = items[i].value;
        if (items[i].label != NULL) {
            XmString str, ostr;
            XtVaGetValues(optp->options[i].widget, XmNlabelString, &ostr, NULL);
            str = XmStringCreateLocalized(items[i].label);
            if (XmStringCompare(str, ostr) != True) {
                XtVaSetValues(optp->options[i].widget, XmNlabelString, str, NULL);
            }
            XmStringFree(str);
        }
    }

    nw = nchoices - nold;
    if (nw > 0) {
        wlist = xmalloc(nw*sizeof(Widget));
        for (i = nold; i < nchoices; i++) {
            wlist[i - nold] = optp->options[i].widget;
        }
        XtManageChildren(wlist, nw);
        xfree(wlist);
    }
}

/* Menu */
Widget CreatePopupMenu(Widget parent)
{
    return XmCreatePopupMenu(parent, "popupMenu", NULL, 0);
}

Widget CreateMenuBar(Widget parent)
{
    Widget menubar;

    menubar = XmCreateMenuBar(parent, "menuBar", NULL, 0);
    return menubar;
}

Widget CreateMenu(Widget parent, char *label, char mnemonic, int help)
{
    Widget menupane, cascade;
    char ms[2];

    menupane = XmCreatePulldownMenu(parent, "menu", NULL, 0);

    ms[0] = mnemonic;
    ms[1] = '\0';

    cascade = XtVaCreateManagedWidget("cascade",
        xmCascadeButtonGadgetClass, parent,
        XmNsubMenuId, menupane,
        XmNmnemonic, XStringToKeysym(ms),
        NULL);

    SetLabel(cascade, label);

    if (help) {
        XtVaSetValues(parent, XmNmenuHelpWidget, cascade, NULL);
        CreateMenuButton(menupane, "On context", 'x',
            ContextHelpCB, NULL);
        CreateSeparator(menupane);
    }

    return menupane;
}

static char *label_to_resname(const char *s, const char *suffix)
{
    char *retval, *rs;
    int capitalize = FALSE;

    retval = copy_string(NULL, s);
    rs = retval;
    while (*s) {
        if (isalnum(*s)) {
            if (capitalize == TRUE) {
                *rs = toupper(*s);
                capitalize = FALSE;
            } else {
                *rs = tolower(*s);
            }
            rs++;
        } else {
            capitalize = TRUE;
        }
        s++;
    }
    *rs = '\0';
    if (suffix != NULL) {
        retval = concat_strings(retval, suffix);
    }
    return retval;
}

Widget CreateMenuButton(Widget parent, char *label, char mnemonic,
        Button_CBProc cb, void *data)
{
    Widget button;
    XmString str;
    char *name, ms[2];

    ms[0] = mnemonic;
    ms[1] = '\0';

    str = XmStringCreateLocalized(label);
    name = label_to_resname(label, "Button");
    button = XtVaCreateManagedWidget(name,
        xmPushButtonWidgetClass, parent,
        XmNlabelString, str,
        XmNmnemonic, XStringToKeysym(ms),
        NULL);
    xfree(name);
    XmStringFree(str);

    AddButtonCB(button, cb, data);

    return button;
}

Widget CreateMenuButtonA(Widget parent, char *label, char mnemonic,
        char *accelerator, Button_CBProc cb, void *data)
{
    return CreateMenuButton(parent, label, mnemonic, cb, data);
}

Widget CreateMenuCloseButton(Widget parent, Widget form)
{
    Widget wbut;
    XmString str;

    wbut = CreateMenuButton(parent,
        "Close", 'C', destroy_dialog_cb, XtParent(form));
    str = XmStringCreateLocalized("Esc");
    XtVaSetValues(wbut, XmNacceleratorText, str, NULL);
    XmStringFree(str);
    XtVaSetValues(form, XmNcancelButton, wbut, NULL);

    return wbut;
}

Widget CreateMenuHelpButton(Widget parent, char *label, char mnemonic,
    Widget form, char *ha)
{
    Widget wbut;

    wbut = CreateMenuButton(parent, label, mnemonic, HelpCB, ha);
    AddHelpCB(form, ha);

    return wbut;
}

Widget CreateMenuToggle(Widget parent, char *label, char mnemonic,
        TB_CBProc cb, void *data)
{
    Widget button;
    XmString str;
    char *name, ms[2];

    ms[0] = mnemonic;
    ms[1] = '\0';

    str = XmStringCreateLocalized(label);
    name = label_to_resname(label, NULL);
    button = CreateToggleButton(parent, name);

    XtVaSetValues(button,
            XmNlabelString, str,
            XmNmnemonic, XStringToKeysym(ms),
            XmNvisibleWhenOff, True,
            XmNindicatorOn, True,
            NULL);
    xfree(name);
    XmStringFree(str);

    if (cb) {
        AddToggleButtonCB(button, cb, data);
    }

    return button;
}

Widget CreateMenuLabel(Widget parent, char *name)
{
    Widget lab;

    lab = XmCreateLabel(parent, name, NULL, 0);
    ManageChild(lab);
    return lab;
}
