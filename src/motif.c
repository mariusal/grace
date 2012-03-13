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
#include "utils.h"

#include <ctype.h>
#include <stdarg.h>
#include <Xm/Xm.h>
#include <Xm/ArrowBG.h>
#include <Xm/CascadeBG.h>
#include <Xm/DialogS.h>
#include <Xm/FileSB.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xbae/Matrix.h>

#if XmVersion >= 2000
# define USE_PANEDW 1
#  include <Xm/PanedW.h>
#else
# define USE_PANEDW 0
#endif

#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#include <Xm/Scale.h>
#include <Xm/ScrolledW.h>
#include <Xm/Text.h>
#include <Xm/ToggleB.h>
#include "Tab.h"
#include "ListTree.h"

#include "globals.h"

extern XtAppContext app_con;

/* Timer */
Timer_CBdata *CreateTimer(unsigned long interval, Timer_CBProc cbproc, void *anydata)
{
    Timer_CBdata *cbdata;

    cbdata = xmalloc(sizeof(Timer_CBdata));

    cbdata->timer_id = 0;
    cbdata->interval = interval;
    cbdata->cbproc = cbproc;
    cbdata->anydata = anydata;

    return cbdata;
}

static void timer_proc(XtPointer client_data, XtIntervalId *id)
{
    Timer_CBdata *cbdata = (Timer_CBdata *) client_data;

    cbdata->cbproc(cbdata->anydata);
    cbdata->timer_id = 0;
}

void TimerStart(Timer_CBdata *cbdata)
{
    /* we count elapsed time since the last event, so first remove
           an existing timeout, if there is one */
    if (cbdata->timer_id) {
        XtRemoveTimeOut(cbdata->timer_id);
    }

    cbdata->timer_id = XtAppAddTimeOut(app_con, cbdata->interval, timer_proc, cbdata);
}

/* Widgets */
void WidgetManage(Widget w)
{
    XtManageChild(w);
}

void WidgetUnmanage(Widget w)
{
    XtUnmanageChild(w);
}

int WidgetIsManaged(Widget w)
{
    return (XtIsManaged(w) == True) ? TRUE:FALSE;
}

void *WidgetGetUserData(Widget w)
{
    void *udata = NULL;
    XtVaGetValues(w, XmNuserData, &udata, NULL);

    return udata;
}

void WidgetSetUserData(Widget w, void *udata)
{
    XtVaSetValues(w, XmNuserData, udata, NULL);
}

void WidgetSetSensitive(Widget w, int onoff)
{
    XtSetSensitive(w, onoff ? True : False);
}

void WidgetSetFocus(Widget w)
{
    XmProcessTraversal(w, XmTRAVERSE_CURRENT);
}

void WidgetSetWidth(Widget w, unsigned int width)
{
    XtVaSetValues(w, XmNwidth, (Dimension) width, NULL);
}

void WidgetSetHeight(Widget w, unsigned int height)
{
    XtVaSetValues(w, XmNheight, (Dimension) height, NULL);
}

void WidgetSetSize(Widget w, unsigned int width, unsigned int height)
{
    XtVaSetValues(w,
        XmNwidth, (Dimension) width,
        XmNheight, (Dimension) height,
        NULL);
}

void WidgetGetSize(Widget w, unsigned int *width, unsigned int *height)
{
    Dimension ww, wh;

    XtVaGetValues(w,
        XmNwidth, &ww,
        XmNheight, &wh,
        NULL);

    *width  = (unsigned int) ww;
    *height = (unsigned int) wh;
}

static int toolkit_modifiers_to_grace_modifiers(void *event)
{
    XEvent *e = (XEvent *) event;
    unsigned int state;
    int modifiers = NO_MODIFIER;

    switch (e->type) {
    case ButtonPress:
    case ButtonRelease:
        state = e->xbutton.state;
        break;
    case KeyPress:
    case KeyRelease:
        state = e->xkey.state;
        break;
    default:
        return modifiers;
    }

    if (state & ControlMask) {
        modifiers = modifiers ^ CONTROL_MODIFIER;
    }

    if (state & ShiftMask) {
        modifiers = modifiers ^ SHIFT_MODIFIER;
    }

    return modifiers;
}

static int toolkit_key_to_grace_key(void *event)
{
    XKeyEvent *xke = (XKeyEvent *) event;
    KeySym keybuf;

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

static int toolkit_button_to_grace_button(void *event)
{
    XButtonEvent *xbe = (XButtonEvent *) event;

    switch (xbe->button) {
    case Button4:
        return WHEEL_UP_BUTTON;
    case Button5:
        return WHEEL_DOWN_BUTTON;
    default:
        return NO_BUTTON;
    }
}

static int toolkit_to_grace(void *event)
{
    XEvent *e = (XEvent *) event;

    switch (e->type) {
    case ButtonPress:
    case ButtonRelease:
        return toolkit_button_to_grace_button(event);
    case KeyPress:
    case KeyRelease:
        return toolkit_key_to_grace_key(event);
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
    if (cbdata->key != toolkit_to_grace(event)) return;
    if (cbdata->modifiers != toolkit_modifiers_to_grace_modifiers(event)) return;

    if (w != cbdata->w) return;

    cbdata->cbproc(cbdata->anydata);
}

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

void AddWidgetButtonPressCB(Widget w, int button, Key_CBProc cbproc, void *anydata)
{
    char *table = NULL;
    XtActionsRec actions[1];
    Key_CBData *cbdata;

    cbdata = (Key_CBData *) xmalloc(sizeof(Key_CBData));
    cbdata->w = w;
    cbdata->modifiers = NO_MODIFIER;
    cbdata->key = button;
    cbdata->cbproc = cbproc;
    cbdata->anydata = anydata;

    switch (button) {
    case WHEEL_UP_BUTTON:
        table = concat_strings(table, "<Btn4Down>: action()");
        break;
    case WHEEL_DOWN_BUTTON:
        table = concat_strings(table, "<Btn5Down>: action()");
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

static void widgetCB(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget_CBData *cbdata = (Widget_CBData *) client_data;
    TextValidate_CD *cdata;
    XmTextVerifyCallbackStruct *tcbs;
    XmTextBlock text;
    char *str = NULL;
    int is_fsb = XmIsFileSelectionBox(w);
    int is_text = XmIsText(w);

    if (is_fsb) {
        XmFileSelectionBoxCallbackStruct *cbs =
            (XmFileSelectionBoxCallbackStruct *) call_data;

        char *buf = GetStringSimple(cbs->value);

        if (buf == NULL) {
            errmsg("Error converting XmString to char string");
            return;
        }

        str = copy_string(NULL, buf);
        XtFree(buf);

        cbdata->calldata = str;
    } else if (is_text) {
        tcbs = (XmTextVerifyCallbackStruct *) call_data;

        text = tcbs->text;

        cdata = (TextValidate_CD *) xmalloc(sizeof(TextValidate_CD));
        cdata->text = &text->ptr;
        cdata->allow_change = TRUE;

        cbdata->calldata = cdata;
    } else {
        cbdata->calldata = call_data;
    }

    cbdata->cbproc(cbdata);

    if (is_fsb) {
        xfree(str);
    } else if (is_text) {

        text->length = strlen(text->ptr);

        if (!cdata->allow_change) {
            tcbs->doit = False;
        }

        xfree(cdata);
    }
}

void AddWidgetCB(Widget w, const char *callback, Widget_CBProc cbproc, void *anydata)
{
    char *cb;
    Widget_CBData *cbdata;

    cbdata = (Widget_CBData *) xmalloc(sizeof(Widget_CBData));
    cbdata->w = w;
    cbdata->cbproc = cbproc;
    cbdata->anydata = anydata;

    cb = copy_string(NULL, callback);
    cb = concat_strings(cb, "Callback");

    XtAddCallback(w, cb, widgetCB, (XtPointer) cbdata);

    xfree(cb);
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

/* Dialog Window */
static void close_dialogCB(Widget_CBData *wcbdata)
{
    WidgetUnmanage(wcbdata->anydata);
}

Widget CreateDialogWindow(Widget parent, const char *s)
{
    Widget dialog;
    char *bufp;

    bufp = label_to_resname(s, "Dialog");
    dialog = XmCreateDialogShell(parent, bufp, NULL, 0);
    xfree(bufp);

    AddWindowCloseCB(dialog, close_dialogCB, dialog);

    bufp = copy_string(NULL, "Grace: ");
    bufp = concat_strings(bufp, s);
    XtVaSetValues(dialog, XmNtitle, bufp, NULL);
    xfree(bufp);

    return dialog;
}

/* Dialog */
Widget CreateDialog(Widget parent, const char *s)
{
    Widget w;

    w = CreateDialogWindow(parent, s);
    w = CreateForm(w);

    return w;
}

void DialogRaise(Widget form)
{
    Widget w = XtParent(form);

    WidgetManage(w);
    XMapRaised(XtDisplay(w), XtWindow(w));
}

void DialogClose(Widget form)
{
    WidgetUnmanage(XtParent(form));
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

/* File selection box */
Widget CreateFileSelectionBox(Widget parent)
{
    Widget w;

    w = XmCreateFileSelectionBox(parent, "FSB", NULL, 0);

    AddMouseWheelSupport(XmFileSelectionBoxGetChild(w, XmDIALOG_LIST));
    AddMouseWheelSupport(XmFileSelectionBoxGetChild(w, XmDIALOG_DIR_LIST));

    return w;
}

/* File selection filter */
#if XmVersion >= 2000
static void show_hidden_cb(Widget but, int onoff, void *data)
{
    Widget fsb = (Widget) data;
    XtVaSetValues(fsb, XmNfileFilterStyle,
        onoff ? XmFILTER_NONE:XmFILTER_HIDDEN_FILES, NULL);
}
#endif

void CreateFileSelectionFilter(Widget parent, Widget fsb)
{
#if XmVersion >= 2000
    Widget button;

    button = CreateToggleButton(parent, "Show hidden files");
    AddToggleButtonCB(button, show_hidden_cb, fsb);
    XtVaSetValues(fsb, XmNfileFilterStyle, XmFILTER_HIDDEN_FILES, NULL);
#endif
}

/* File selection dialog */
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
    char *dir;
    FSBStructure *fsb = (FSBStructure *) data;

    switch (value) {
    case FSB_CWD:
        dir = get_workingdir(gapp);
        break;
    case FSB_HOME:
        dir = grace_get_userhome(gapp->grace);
        break;
    case FSB_ROOT:
        dir = "/";
        break;
    case FSB_CYGDRV:
        dir = "/cygdrive/";
        break;
    default:
        return;
    }

    FSBDialogSetDirectory(fsb, dir);
}

static LabelOptionItem fsb_items[] = {
    {FSB_CWD,  "Cwd"},
    {FSB_HOME, "Home"},
    {FSB_ROOT, "/"}
#ifdef __CYGWIN__
    ,{FSB_CYGDRV, "My Computer"}
#endif
};

#define FSB_ITEMS_NUM   sizeof(fsb_items)/sizeof(LabelOptionItem)

FSBStructure *CreateFSBDialog(Widget parent, char *s)
{
    FSBStructure *retval;
    OptionStructure *opt;
    Widget dialog, fr, form, button;

    retval = xmalloc(sizeof(FSBStructure));

    dialog = CreateDialogWindow(parent, s);
    retval->FSB = CreateFileSelectionBox(dialog);

    FSBDialogSetDirectory(retval, get_workingdir(gapp));
    AddWidgetCB(retval->FSB, "cancel", close_dialogCB, dialog);
    AddHelpCB(retval->FSB, "doc/UsersGuide.html#FS-dialog");

    retval->rc = CreateVContainer(retval->FSB);

    CreateFileSelectionFilter(retval->rc, retval->FSB);

    fr = CreateFrame(retval->rc, NULL);

    form = CreateForm(fr);

    opt = CreateLabelOptionChoice(form, "Chdir to:", 1, FSB_ITEMS_NUM, fsb_items);
    AddOptionChoiceCB(opt, fsb_cd_cb, retval);
    FormAddHChild(form, opt->menu);

    button = CreateButton(form, "Set as cwd");
    AddButtonCB(button, fsb_setcwd_cb, retval->FSB);
    FormAddHChild(form, button);
    FormFixateHChild(button);

    WidgetManage(form);

    return retval;
}

typedef struct {
    FSBStructure *fsb;
    FSB_CBProc cbproc;
    void *anydata;
} FSB_CBdata;

static void fsb_int_cb_proc(Widget_CBData *wcbdata)
{
    FSB_CBdata *cbdata = (FSB_CBdata *) wcbdata->anydata;
    char *s = (char *) wcbdata->calldata;
    int ok;

    set_wait_cursor();

    ok = cbdata->cbproc(cbdata->fsb, s, cbdata->anydata);

    if (ok) {
        DialogClose(cbdata->fsb->FSB);
    }
    unset_wait_cursor();
}

void AddFSBDialogCB(FSBStructure *fsb, FSB_CBProc cbproc, void *anydata)
{
    FSB_CBdata *cbdata;

    cbdata = xmalloc(sizeof(FSB_CBdata));
    cbdata->fsb = fsb;
    cbdata->cbproc = (FSB_CBProc) cbproc;
    cbdata->anydata = anydata;

    AddWidgetCB(fsb->FSB, "ok", fsb_int_cb_proc, cbdata);
}

void FSBDialogSetPattern(FSBStructure *fsb, char *pattern)
{
    XmString xmstr;

    if (pattern != NULL) {
        xmstr = XmStringCreateLocalized(pattern);
        XtVaSetValues(fsb->FSB, XmNpattern, xmstr, NULL);
        XmStringFree(xmstr);
    }
}

void FSBDialogSetDirectory(FSBStructure *fsb, char *directory)
{
    XmString xmstr;

    if (directory != NULL) {
        xmstr = XmStringCreateLocalized(directory);
        XtVaSetValues(fsb->FSB, XmNdirectory, xmstr, NULL);
        XmStringFree(xmstr);
    }
}

/* Containers */
Widget CreateVContainer(Widget parent)
{
    Widget rc;

    rc = XmCreateRowColumn(parent, "VContainer", NULL, 0);
    WidgetManage(rc);

    return rc;
}

Widget CreateHContainer(Widget parent)
{
    Widget rc;

    rc = XmCreateRowColumn(parent, "HContainer", NULL, 0);
    XtVaSetValues(rc, XmNorientation, XmHORIZONTAL, NULL);
    WidgetManage(rc);

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

    last_widget = WidgetGetUserData(form);
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
    WidgetSetUserData(form, child);
}

void FormAddVChild(Widget form, Widget child)
{
    Widget last_widget;

    if (XtIsSubclass(child, listtreeWidgetClass) ||
        (XmIsText(child) && XmIsScrolledWindow(XtParent(child)))) {
        child = XtParent(child);
    }

    last_widget = WidgetGetUserData(form);
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
    WidgetSetUserData(form, child);
}

void FormFixateHChild(Widget w)
{
    Widget prev;
    XtVaGetValues(w, XmNleftWidget, &prev, NULL);
    XtVaSetValues(w, XmNleftAttachment, XmATTACH_NONE, NULL);
    XtVaSetValues(prev, XmNrightAttachment, XmATTACH_WIDGET,
        XmNrightWidget, w,
        NULL);
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

    WidgetManage(w);

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

/* Frame */
Widget CreateFrame(Widget parent, char *s)
{
    Widget fr;

    fr = XtVaCreateManagedWidget("frame", xmFrameWidgetClass, parent, NULL);
    if (s != NULL) {
        XtVaCreateManagedWidget(s, xmLabelGadgetClass, fr,
                                XmNchildType, XmFRAME_TITLE_CHILD,
                                NULL);
    }

    return fr;
}

/* Scrolled window */
Widget CreateScrolledWindow(Widget parent)
{
    return XtVaCreateManagedWidget("scrolledWindow",
                                   xmScrolledWindowWidgetClass, parent,
                                   XmNscrollingPolicy, XmAUTOMATIC,
                                   NULL);
}

/* Paned window */
Widget CreatePanedWindow(Widget parent)
{
#if USE_PANEDW
    return XtVaCreateManagedWidget("panedWindow",
                                   xmPanedWindowWidgetClass, parent,
                                   XmNorientation, XmHORIZONTAL,
                                   NULL);
#else
    return CreateGrid(parent, 2, 1);
#endif
}

void PanedWindowSetMinWidth(Widget w, unsigned int width)
{
    XtVaSetValues(w, XmNpaneMinimum, (Dimension) width, NULL);
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

/* Separator */
Widget CreateSeparator(Widget parent)
{
    Widget sep;

    sep = XmCreateSeparator(parent, "sep", NULL, 0);
    WidgetManage(sep);

    return sep;
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

    if (s == NULL) return;

    str = XmStringCreateLocalized(s);
    XtVaSetValues(w,
            XmNlabelType, XmSTRING,
            XmNlabelString, str,
            NULL);
    XmStringFree(str);
}

static Pixmap BitmapToPixmap(Widget w, int width, int height, const unsigned char *bits)
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

    return pm;
}

void LabelSetPixmap(Widget w, Pixmap pixmap)
{
    XtVaSetValues(w,
            XmNlabelType, XmPIXMAP,
            XmNlabelPixmap, pixmap,
            NULL);
}

/* Text edit */
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
    WidgetManage(w);

    return w;
}

char *TextEditGetString(Widget w)
{
    char *s, *buf;

    s = XmTextGetString(w);
    buf = copy_string(NULL, s);
    XtFree(s);

    return buf;
}

void TextEditSetString(Widget w, char *s)
{
    XmTextSetString(w, s ? s : "");
}

/* Text */
void TextInsertString(TextStructure *cst, int pos, char *s)
{
    XmTextInsert(cst->text, pos, s);
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

void TextSetLength(TextStructure *cst, int len)
{
    XtVaSetValues(cst->text, XmNcolumns, len, NULL);
}

void TextSetEditable(TextStructure *cst, int onoff)
{
    XtVaSetValues(cst->text, XmNeditable, onoff? True:False, NULL);
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
    Pixmap pm;

    button = XtVaCreateWidget("button",
        xmPushButtonWidgetClass, parent,
        NULL);

    pm = BitmapToPixmap(button, width, height, bits);

    LabelSetPixmap(button, pm);
    WidgetManage(button);

    return button;
}

Widget CreateArrowButton(Widget parent, int arrow_type)
{
    Widget w;

    w = XtVaCreateManagedWidget("arrow", xmArrowButtonGadgetClass, parent,
            XmNarrowDirection, (arrow_type == ARROW_UP) ? XmARROW_UP : XmARROW_DOWN,
            NULL);

    return w;
}

/* ToggleButton */
Widget CreateToggleButton(Widget parent, char *s)
{
    return (XtVaCreateManagedWidget(s, xmToggleButtonWidgetClass, parent, NULL));
}

int ToggleButtonGetState(Widget w)
{
    if (!w) {
        errmsg("Internal error: GetToggleButtonState() called with NULL widget");
        return 0;
    } else {
        return XmToggleButtonGetState(w);
    }
}

void ToggleButtonSetState(Widget w, int value)
{
    if (w == NULL) {
        return;
    }
    XmToggleButtonSetState(w, value ? True:False, False);

    return;
}

/* Scale */
Widget CreateScale(Widget parent, char *s, int min, int max, int delta)
{
    Widget w;
    XmString str;

    str = XmStringCreateLocalized(s);

    w = XtVaCreateManagedWidget("scroll",
        xmScaleWidgetClass, parent,
        XmNtitleString, str,
        XmNminimum, min,
        XmNmaximum, max,
        XmNscaleMultiple, delta,
        XmNvalue, 0,
        XmNshowValue, True,
        XmNprocessingDirection, XmMAX_ON_RIGHT,
        XmNorientation, XmHORIZONTAL,
#if XmVersion >= 2000
        XmNsliderMark, XmROUND_MARK,
#endif
        NULL);

    XmStringFree(str);

    return w;
}

void ScaleSetValue(Widget w, int value)
{
    XtVaSetValues(w, XmNvalue, value, NULL);
}

int ScaleGetValue(Widget w)
{
    int value;
    XtVaGetValues(w, XmNvalue, &value, NULL);
    return value;
}

/* OptionChoice */
static void combobox_press(Widget_CBData *wcbdata)
{
    Widget but = wcbdata->w;
    Widget popup = XtParent(wcbdata->anydata);
    Widget table = wcbdata->anydata;

    Dimension wh;
    Position wx, wy, root_x_return, root_y_return;

    XtVaGetValues(but,
            XtNx, &wx,
            XtNy, &wy,
            XmNheight, &wh,
            NULL);

    XtTranslateCoords(but, 0, 0, &root_x_return, &root_y_return);

    XtVaSetValues(popup,
            XtNx, root_x_return,
            XtNy, root_y_return + wh,
            NULL);

    XtPopup(popup, XtGrabNone);
    XtGrabPointer(table, False, ButtonReleaseMask, GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
}

static int oc_drawCB(TableEvent *event)
{
    OptionStructure *optp = (OptionStructure *) event->anydata;
    int i;

    i = event->row + event->col * TableGetNrows(optp->pulldown);

    if (optp->items[i].label) {
        event->value_type = TABLE_CELL_STRING;
        event->value = optp->items[i].label;
    } else {
        event->value_type = TABLE_CELL_PIXMAP;
        event->pixmap = optp->items[i].pixmap;
    }

    if (optp->update_colors) {
        event->background = optp->items[i].background;
        event->foreground = optp->items[i].foreground;
    }

    return TRUE;
}

static void table_event_proc(Widget w, XtPointer data, XEvent *event, Boolean *cont)
{
    OptionStructure *opt = (OptionStructure *)data;
    int row, col;

    XtCallActionProc(opt->menu, "Disarm", NULL, NULL, 0);
    XtUngrabPointer(w, CurrentTime);

    if(XbaeMatrixGetEventRowColumn(w, event, &row, &col)) {
        int i;
        OC_CBdata *cbdata;

        i = row + col * TableGetNrows(opt->pulldown);

        if (opt->items[i].label) {
            LabelSetString(opt->menu, opt->items[i].label);
        } else {
            LabelSetPixmap(opt->menu, opt->items[i].pixmap);
        }
        opt->cvalue = opt->items[i].value;

        for (i = 0; i < opt->cbnum; i++) {
            cbdata = opt->cblist[i];
            cbdata->cbproc(opt, opt->cvalue, cbdata->anydata);
        }
    }

    XtPopdown(XtParent(opt->pulldown));
}

OptionStructure *CreateOptionChoice(Widget parent, char *labelstr,
    int ncols, int nchoices, LabelOptionItem *items)
{
    OptionStructure *retval;
    Widget rc, popup;

    retval = xcalloc(1, sizeof(OptionStructure));
    if (!retval) {
        return NULL;
    }

    rc = CreateHContainer(parent);
    CreateLabel(rc, labelstr);

    retval->menu = CreateButton(rc, "ComboBox");

    popup = XtCreatePopupShell("popup", overrideShellWidgetClass, retval->menu, NULL, 0);
    retval->pulldown = CreateTable("pulldownTable", popup, 1, 1, 0, 0);
    TableOptionChoiceInit(retval->pulldown);
    AddWidgetCB(retval->menu, "arm", combobox_press, retval->pulldown);
    XtAddEventHandler(retval->pulldown, ButtonReleaseMask, False, table_event_proc, retval);

    retval->ncols = ncols;
    retval->nchoices = 0;
    retval->items = NULL;
    retval->update_colors = FALSE;

    UpdateLabelOptionChoice(retval, nchoices, items);
    AddTableDrawCellCB(retval->pulldown, oc_drawCB, retval);

    if (retval->items[0].label) {
        LabelSetString(retval->menu, retval->items[0].label);
    } else {
        LabelSetPixmap(retval->menu, retval->items[0].pixmap);
    }
    retval->cvalue = retval->items[0].value;

    return retval;
}

#define MAX_PULLDOWN_LENGTH 30
void UpdateOptionChoice(OptionStructure *optp, int nchoices, LabelOptionItem *items)
{
    int i, ncols, nrows, nc, nr;
    int delta_nc, delta_nr;
    int width = 0;

    if (optp->ncols == 0) {
        ncols = 1;
    } else {
        ncols = optp->ncols;
    }

    /* Don't create too tall pulldowns */
    if (nchoices > MAX_PULLDOWN_LENGTH*ncols) {
        ncols = (nchoices + MAX_PULLDOWN_LENGTH - 1)/MAX_PULLDOWN_LENGTH;
    }

    nrows = nchoices / ncols;

    nr = TableGetNrows(optp->pulldown);
    nc = TableGetNcols(optp->pulldown);

    delta_nr = nrows - nr;
    delta_nc = ncols - nc;

    if (delta_nr > 0) {
        TableAddRows(optp->pulldown, delta_nr);
    } else if (delta_nr < 0) {
        TableDeleteRows(optp->pulldown, -delta_nr);
    }
    if (delta_nc > 0) {
        TableAddCols(optp->pulldown, delta_nc);
    } else if (delta_nc < 0) {
        TableDeleteCols(optp->pulldown, -delta_nc);
    }

    for (i = 0; i < optp->nchoices; i++) {
        xfree(optp->items[i].label);
    }

    optp->items = xrealloc(optp->items, nchoices*sizeof(LabelOptionItem));

    for (i = 0; i < nchoices; i++) {
        char *label;
        optp->items[i].value = items[i].value;

        label = items[i].label;
        if (label) {
            if (width < strlen(label)) {
                width = strlen(label);
            }
            optp->items[i].label = copy_string(NULL, label);
        } else {
            optp->items[i].label = NULL;
            optp->items[i].pixmap = items[i].pixmap;
        }

        optp->items[i].background = items[i].background;
        optp->items[i].foreground = items[i].foreground;
    }

    optp->nchoices = nchoices;

    if (width) {
        TableSetDefaultColWidth(optp->pulldown, width);
    }
    TableUpdateVisibleRowsCols(optp->pulldown);
}

OptionStructure *CreateLabelOptionChoice(Widget parent, char *labelstr,
    int ncols, int nchoices, LabelOptionItem *items)
{
    return CreateOptionChoice(parent, labelstr, ncols, nchoices, items);
}

OptionStructure *CreateLabelOptionChoiceVA(Widget parent, char *labelstr, ...)
{
    OptionStructure *retval;
    int nchoices = 0;
    LabelOptionItem *oi = NULL;
    va_list var;
    char *s;
    int value;

    va_start(var, labelstr);
    while ((s = va_arg(var, char *)) != NULL) {
        value = va_arg(var, int);
        nchoices++;
        oi = xrealloc(oi, nchoices*sizeof(LabelOptionItem));
        oi[nchoices - 1].value = value;
        oi[nchoices - 1].label = copy_string(NULL, s);
    }
    va_end(var);

    retval = CreateLabelOptionChoice(parent, labelstr, 1, nchoices, oi);

    while (nchoices) {
        nchoices--;
        xfree(oi[nchoices].label);
    }
    xfree(oi);

    return retval;
}

void UpdateLabelOptionChoice(OptionStructure *optp, int nchoices, LabelOptionItem *items)
{
    UpdateOptionChoice(optp, nchoices, items);
}

OptionStructure *CreateBitmapOptionChoice(Widget parent, char *labelstr, int ncols,
                int nchoices, int width, int height, BitmapOptionItem *items)
{
    int i;
    OptionStructure *retval;
    LabelOptionItem *pixmap_items;

    pixmap_items = xmalloc(nchoices*sizeof(LabelOptionItem));
    if (pixmap_items == NULL) {
        errmsg("Malloc error in CreateBitmapOptionChoice()");
        return NULL;
    }

    for (i = 0; i < nchoices; i++) {
        pixmap_items[i].value = items[i].value;
        if (items[i].bitmap) {
            pixmap_items[i].label = NULL;
            pixmap_items[i].pixmap = BitmapToPixmap(parent, width, height, items[i].bitmap);
        } else {
            pixmap_items[i].label = "None";
        }
    }

    retval = CreateLabelOptionChoice(parent, labelstr, ncols, nchoices, pixmap_items);

    xfree(pixmap_items);

    return retval;
}

static unsigned char dummy_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0xec, 0x2e, 0x04, 0x20, 0x00, 0x20, 0x04, 0x00,
   0x04, 0x20, 0x04, 0x20, 0x00, 0x20, 0x04, 0x00, 0x04, 0x20, 0x04, 0x20,
   0x00, 0x20, 0xdc, 0x1d, 0x00, 0x00, 0x00, 0x00};

OptionStructure *CreateCharOptionChoice(Widget parent, char *s)
{
    int i;
    int nchoices = 256;
    int *fontid;

    OptionStructure *retval;
    LabelOptionItem *pixmap_items;
    Pixmap pixmap;

    pixmap_items = xmalloc(nchoices*sizeof(LabelOptionItem));
    if (pixmap_items == NULL) {
        errmsg("Malloc error in CreateCharOptionChoice()");
        return NULL;
    }

    pixmap = BitmapToPixmap(parent, 16, 16, dummy_bits);
    for (i = 0; i < nchoices; i++) {
        pixmap_items[i].value = (char) i;
        pixmap_items[i].label = NULL;
        pixmap_items[i].pixmap = pixmap;
    }

    retval = CreateLabelOptionChoice(parent, s, 16, nchoices, pixmap_items);

    xfree(pixmap_items);

    fontid = xmalloc(SIZEOF_INT);
    *fontid = -1;
    WidgetSetUserData(retval->menu, fontid);

    return retval;
}

void UpdateCharOptionChoice(OptionStructure *opt, int font)
{
    int *old_font;
    old_font = (int *) WidgetGetUserData(opt->menu);
    if (*old_font != font) {
        int i;
        for (i = 0; i < opt->nchoices; i++) {
            Pixmap pixmap = char_to_pixmap(opt->pulldown, font, (char) i, 24);
            if (pixmap) {
                opt->items[i].pixmap = pixmap;
            }
            //TODO:
//            WidgetSetSensitive(w, ptmp ? TRUE:FALSE);
        }
        *old_font = font;
    }
}

void SetOptionChoice(OptionStructure *opt, int value)
{
    int i;

    if (opt->items == NULL || opt->nchoices <= 0) {
        return;
    }

    for (i = 0; i < opt->nchoices; i++) {
        if (opt->items[i].value == value) {
            if (opt->items[i].label) {
                LabelSetString(opt->menu, opt->items[i].label);
            } else {
                LabelSetPixmap(opt->menu, opt->items[i].pixmap);
            }
            opt->cvalue = opt->items[i].value;
            return;
        }
    }

    errmsg("Value not found in SetOptionChoice()");
}

int GetOptionChoice(OptionStructure *opt)
{
    if (opt->items == NULL || opt->nchoices <= 0) {
        errmsg("Internal error in GetOptionChoice()");
        return 0;
    }

    return opt->cvalue;
}

void OptionChoiceSetColorUpdate(OptionStructure *opt, int update)
{
    opt->update_colors = update;
}

void AddOptionChoiceCB(OptionStructure *opt, OC_CBProc cbproc, void *anydata)
{
    OC_CBdata *cbdata;

    cbdata = xmalloc(sizeof(OC_CBdata));

    cbdata->opt = opt;
    cbdata->cbproc = cbproc;
    cbdata->anydata = anydata;

    opt->cblist = xrealloc(opt->cblist, (opt->cbnum + 1)*sizeof(OC_CBdata *));
    opt->cblist[opt->cbnum] = cbdata;
    opt->cbnum++;
}

/* Menu */
Widget CreatePopupMenu(Widget parent)
{
    return XmCreatePopupMenu(parent, "popupMenu", NULL, 0);
}

void PopupMenuShow(Widget w, void *data)
{
    XmMenuPosition(w, (XButtonEvent *) data);
    WidgetManage(w);
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

Widget CreateMenuSeparator(Widget parent)
{
    return CreateSeparator(parent);
}

