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

static int toolkit_key_to_grace_key(void *event)
{
    XKeyEvent *xke;
    KeySym keybuf;

    xke = (XKeyEvent *) event;
    keybuf = XLookupKeysym(xke, 0);

    switch (keybuf) {
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

    if (cbdata->key == toolkit_key_to_grace_key(event)) {
        cbdata->cbproc(&kevent);
    }
}

void AddWidgetKeyPressCB(Widget w, int key, Key_CBProc cbproc, void *anydata)
{
    Key_CBData *cbdata;

    cbdata = (Key_CBData *) xmalloc(sizeof(Key_CBData));
    cbdata->w = w;
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
