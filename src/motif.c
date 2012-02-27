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
#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/Text.h>
#include <Xm/ToggleB.h>
#include "Tab.h"
#include "ListTree.h"

#include "globals.h"

/* Widgets */
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

void WidgetSetSensitive(Widget w, int onoff)
{
    XtSetSensitive(w, onoff ? True : False);
}

void SetHeight(Widget w, unsigned int height)
{
    XtVaSetValues(w, XmNheight, (Dimension) height, NULL);
}

void SetDimensions(Widget w, unsigned int width, unsigned int height)
{
    XtVaSetValues(w,
        XmNwidth, (Dimension) width,
        XmNheight, (Dimension) height,
        NULL);
}

void GetDimensions(Widget w, unsigned int *width, unsigned int *height)
{
    Dimension ww, wh;

    XtVaGetValues(w,
        XmNwidth, &ww,
        XmNheight, &wh,
        NULL);

    *width  = (unsigned int) ww;
    *height = (unsigned int) wh;
}

void SetMinimumDimensions(Widget w, unsigned int width, unsigned int height)
{
    XtVaSetValues(w, XmNpaneMinimum, width, NULL);
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

/* Dialog */
Widget CreateDialog(Widget parent, const char *s)
{
    Widget dialog, w;
    char *bufp;

    bufp = label_to_resname(s, "Dialog");
    dialog = XmCreateDialogShell(parent, bufp, NULL, 0);
    xfree(bufp);

    handle_close(dialog);

    bufp = copy_string(NULL, "Grace: ");
    bufp = concat_strings(bufp, s);
    XtVaSetValues(dialog,
        XmNtitle, bufp,
        NULL);
    xfree(bufp);

    w = CreateForm(dialog);

    return w;
}

void DialogRaise(Widget form)
{
    Widget w = XtParent(form);

    ManageChild(w);
    XMapRaised(XtDisplay(w), XtWindow(w));
}

void DialogClose(Widget form)
{
    UnmanageChild(XtParent(form));
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

/* File selection dialog */
static XmStringCharSet charset = XmFONTLIST_DEFAULT_TAG;

static char *GetStringSimple(XmString xms)
{
    char *s;

    if (XmStringGetLtoR(xms, charset, &s)) {
        return s;
    } else {
        return NULL;
    }
}

static void destroy_dialog(Widget w, XtPointer client_data, XtPointer call_data)
{
    UnmanageChild((Widget) client_data);
}

static void fsb_setcwd_cb(Widget but, void *data)
{
    char *bufp;
    XmString directory;
    Widget fsb = (Widget) data;

    XtVaGetValues(fsb, XmNdirectory, &directory, NULL);
    bufp = GetStringSimple(directory);
    XmStringFree(directory);
    if (bufp != NULL) {
        set_workingdir(gapp, bufp);
        XtFree(bufp);
    }
}

#define FSB_CWD     0
#define FSB_HOME    1
#define FSB_ROOT    2
#define FSB_CYGDRV  3

static void fsb_cd_cb(OptionStructure *opt, int value, void *data)
{
    char *bufp;
    XmString dir, pattern, dirmask;
    Widget FSB = (Widget) data;

    switch (value) {
    case FSB_CWD:
        bufp = get_workingdir(gapp);
        break;
    case FSB_HOME:
        bufp = grace_get_userhome(gapp->grace);
        break;
    case FSB_ROOT:
        bufp = "/";
        break;
    case FSB_CYGDRV:
        bufp = "/cygdrive/";
        break;
    default:
        return;
    }

    XtVaGetValues(FSB, XmNpattern, &pattern, NULL);

    dir = XmStringCreateLocalized(bufp);
    dirmask = XmStringConcatAndFree(dir, pattern);

    XmFileSelectionDoSearch(FSB, dirmask);
    XmStringFree(dirmask);
}

static OptionItem fsb_items[] = {
    {FSB_CWD,  "Cwd"},
    {FSB_HOME, "Home"},
    {FSB_ROOT, "/"}
#ifdef __CYGWIN__
    ,{FSB_CYGDRV, "My Computer"}
#endif
};

#define FSB_ITEMS_NUM   sizeof(fsb_items)/sizeof(OptionItem)

#if XmVersion >= 2000
static void show_hidden_cb(Widget but, int onoff, void *data)
{
    FSBStructure *fsb = (FSBStructure *) data;
    XtVaSetValues(fsb->FSB, XmNfileFilterStyle,
        onoff ? XmFILTER_NONE:XmFILTER_HIDDEN_FILES, NULL);
}
#endif


FSBStructure *CreateFileSelectionBox(Widget parent, char *s)
{
    FSBStructure *retval;
    OptionStructure *opt;
    Widget fr, form, button;
    XmString xmstr;
    char *bufp, *resname;

    retval = xmalloc(sizeof(FSBStructure));
    resname = label_to_resname(s, "FSB");
    retval->FSB = XmCreateFileSelectionDialog(parent, resname, NULL, 0);
    xfree(resname);
    retval->dialog = XtParent(retval->FSB);
    handle_close(retval->dialog);
    bufp = copy_string(NULL, "Grace: ");
    bufp = concat_strings(bufp, s);
    XtVaSetValues(retval->dialog, XmNtitle, bufp, NULL);
    xfree(bufp);

    xmstr = XmStringCreateLocalized(get_workingdir(gapp));
    XtVaSetValues(retval->FSB, XmNdirectory, xmstr, NULL);
    XmStringFree(xmstr);

    XtAddCallback(retval->FSB,
        XmNcancelCallback, destroy_dialog, retval->dialog);
    AddHelpCB(retval->FSB, "doc/UsersGuide.html#FS-dialog");

    retval->rc = XmCreateRowColumn(retval->FSB, "rc", NULL, 0);
#if XmVersion >= 2000
    button = CreateToggleButton(retval->rc, "Show hidden files");
    AddToggleButtonCB(button, show_hidden_cb, retval);
    XtVaSetValues(retval->FSB, XmNfileFilterStyle, XmFILTER_HIDDEN_FILES, NULL);
#endif
    fr = CreateFrame(retval->rc, NULL);
    form = XtVaCreateWidget("form", xmFormWidgetClass, fr, NULL);
    opt = CreateOptionChoice(form, "Chdir to:", 1, FSB_ITEMS_NUM, fsb_items);
    AddOptionChoiceCB(opt, fsb_cd_cb, (void *) retval->FSB);
    button = CreateButton(form, "Set as cwd");
    AddButtonCB(button, fsb_setcwd_cb, (void *) retval->FSB);

    XtVaSetValues(opt->menu,
        XmNleftAttachment, XmATTACH_FORM,
        XmNtopAttachment, XmATTACH_FORM,
        XmNbottomAttachment, XmATTACH_FORM,
        XmNrightAttachment, XmATTACH_NONE,
        NULL);
    XtVaSetValues(button,
        XmNleftAttachment, XmATTACH_NONE,
        XmNtopAttachment, XmATTACH_FORM,
        XmNbottomAttachment, XmATTACH_FORM,
        XmNrightAttachment, XmATTACH_FORM,
        NULL);
    ManageChild(form);

    ManageChild(retval->rc);

    AddMouseWheelSupport(XmFileSelectionBoxGetChild(retval->FSB,
        XmDIALOG_LIST));
    AddMouseWheelSupport(XmFileSelectionBoxGetChild(retval->FSB,
        XmDIALOG_DIR_LIST));

    return retval;
}

typedef struct {
    FSBStructure *fsb;
    FSB_CBProc cbproc;
    void *anydata;
} FSB_CBdata;

static void fsb_int_cb_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    char *s;
    int ok;

    FSB_CBdata *cbdata = (FSB_CBdata *) client_data;
    XmFileSelectionBoxCallbackStruct *cbs =
        (XmFileSelectionBoxCallbackStruct *) call_data;

    s = GetStringSimple(cbs->value);
    if (s == NULL) {
        errmsg("Error converting XmString to char string");
        return;
    }

    set_wait_cursor();

    ok = cbdata->cbproc(cbdata->fsb, s, cbdata->anydata);
    XtFree(s);
    if (ok) {
        UnmanageChild(cbdata->fsb->dialog);
    }
    unset_wait_cursor();
}

void AddFileSelectionBoxCB(FSBStructure *fsb, FSB_CBProc cbproc, void *anydata)
{
    FSB_CBdata *cbdata;

    cbdata = xmalloc(sizeof(FSB_CBdata));
    cbdata->fsb = fsb;
    cbdata->cbproc = (FSB_CBProc) cbproc;
    cbdata->anydata = anydata;
    XtAddCallback(fsb->FSB,
        XmNokCallback, fsb_int_cb_proc, (XtPointer) cbdata);
}

void SetFileSelectionBoxPattern(FSBStructure *fsb, char *pattern)
{
    XmString xmstr;

    if (pattern != NULL) {
        xmstr = XmStringCreateLocalized(pattern);
        XtVaSetValues(fsb->FSB, XmNpattern, xmstr, NULL);
        XmStringFree(xmstr);
    }
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

    if (XtIsSubclass(child, listtreeWidgetClass) ||
        (XmIsText(child) && XmIsScrolledWindow(XtParent(child)))) {
        child = XtParent(child);
    }

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

    LabelSetString(label, s);

    return label;
}

void LabelSetString(Widget w, char *s)
{
    XmString str;

    str = XmStringCreateLocalized(s);
    XtVaSetValues(w, XmNlabelString, str, NULL);
    XmStringFree(str);
}


void LabelSetPixmap(Widget w, int width, int height, const unsigned char *bits)
{
    Pixmap pm;

    X11Stuff *xstuff = gapp->gui->xstuff;
    Pixel fg, bg;

    XtVaGetValues(w,
            XmNforeground, &fg,
            XmNbackground, &bg,
            NULL);

    pm = XCreatePixmapFromBitmapData(xstuff->disp,
            xstuff->root, (char *) bits, width, height, fg, bg, xstuff->depth);

    XtVaSetValues(w,
            XmNlabelType, XmPIXMAP,
            XmNlabelPixmap, pm,
            NULL);
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

    button = XtVaCreateManagedWidget(label,
        xmPushButtonWidgetClass, parent,
        NULL);

    XtVaSetValues(button,
        XmNalignment, XmALIGNMENT_CENTER,
        NULL);

    return button;
}

Widget CreateBitmapButton(Widget parent,
    int width, int height, const unsigned char *bits)
{
    Widget button;

    button = XtVaCreateWidget("button",
        xmPushButtonWidgetClass, parent,
        NULL);
    LabelSetPixmap(button, width, height, bits);
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

    ManageChild(w);

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

    LabelSetString(cascade, label);

    if (help) {
        XtVaSetValues(parent, XmNmenuHelpWidget, cascade, NULL);
        CreateMenuButton(menupane, "On context", 'x',
            ContextHelpCB, NULL);
        CreateSeparator(menupane);
    }

    return menupane;
}

Widget CreateMenuButton(Widget parent, char *label, char mnemonic,
        Button_CBProc cb, void *data)
{
    return CreateMenuButtonA(parent, label, mnemonic, NULL, NULL, cb, data);
}

Widget CreateMenuButtonA(Widget parent, char *label, char mnemonic,
        char *accelerator, char* acceleratorText, Button_CBProc cb, void *data)
{
    Widget button;
    XmString str;
    char *name, ms[2];

    ms[0] = mnemonic;
    ms[1] = '\0';

    name = label_to_resname(label, "Button");

    button = CreateButton(parent, name);
    xfree(name);

    XtVaSetValues(button,
        XmNmnemonic, XStringToKeysym(ms),
        XmNalignment, XmALIGNMENT_BEGINNING,
        NULL);

    if (accelerator && acceleratorText) {
        str = XmStringCreateLocalized(acceleratorText);
        XtVaSetValues(button,
                XmNaccelerator, accelerator,
                XmNacceleratorText, str,
                NULL);
        XmStringFree(str);
    }

    LabelSetString(button, label);

    AddButtonCB(button, cb, data);

    return button;
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
    char *name, ms[2];

    ms[0] = mnemonic;
    ms[1] = '\0';

    name = label_to_resname(label, NULL);
    button = CreateToggleButton(parent, name);
    xfree(name);

    XtVaSetValues(button,
            XmNmnemonic, XStringToKeysym(ms),
            XmNvisibleWhenOff, True,
            XmNindicatorOn, True,
            NULL);

    LabelSetString(button, label);

    if (cb) {
        AddToggleButtonCB(button, cb, data);
    }

    return button;
}
