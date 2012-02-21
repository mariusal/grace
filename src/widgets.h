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

/* Widgets */

#ifndef __WIDGETS_H_
#define __WIDGETS_H_

#include <config.h>

#ifdef MOTIF_GUI
/* for Widget */
#include <X11/Intrinsic.h>
#endif /* MOTIF_GUI */

#ifdef QT_GUI
#include "qtgui/qtinc.h"
#endif /* QT_GUI */

/* Widgets */
Widget WidgetGetParent(Widget w);
void ManageChild(Widget w);
void UnmanageChild(Widget w);
int IsManaged(Widget w);
void *GetUserData(Widget w);
void SetUserData(Widget w, void *udata);
void SetSensitive(Widget w, int onoff);

typedef struct {
    Widget w;
    void *anydata;
} KeyEvent;

typedef void (*Key_CBProc)(KeyEvent *event);
typedef struct {
    Widget w;
    int modifiers;
    int key;
    Key_CBProc cbproc;
    void *anydata;
} Key_CBData;

void AddWidgetKeyPressCB(Widget w, int key, Key_CBProc cbproc, void *anydata);
void AddWidgetKeyPressCB2(Widget w, int modifiers, int key, Key_CBProc cbproc, void *anydata);

/* Dialog */
void RaiseWindow(Widget w);
void DialogSetResizable(Widget form, int onoff);

/* Containers */
Widget CreateVContainer(Widget parent);
Widget CreateHContainer(Widget parent);

/* Form */
Widget CreateForm(Widget parent);
void FormAddHChild(Widget form, Widget child);
void FormAddVChild(Widget form, Widget child);
void FormFixateVChild(Widget w);

/* Label */
Widget CreateLabel(Widget parent, char *s);
void SetLabel(Widget w, char *s);

/* Text */
Widget CreateLineTextEdit(Widget parent, int len);
Widget CreateMultiLineTextEdit(Widget parent, int nrows);

typedef struct {
    Widget label;
    Widget form;
    Widget text;
    int multiline;
    int locked;
} TextStructure;

void TextSetLength(TextStructure *cst, int len);
char *TextGetString(TextStructure *cst);
void TextSetString(TextStructure *cst, char *s);

typedef int (*TextValidate_CBProc)(
        char **value,
        int *length,
        void *data
);
void AddTextValidateCB(TextStructure *cst, TextValidate_CBProc cbproc, void *anydata);
int TextGetCursorPos(TextStructure *cst);
void TextSetCursorPos(TextStructure *cst, int pos);
int TextGetLastPosition(TextStructure *cst);
void TextInsert(TextStructure *cst, int pos, char *s);
void TextSetEditable(TextStructure *cst, int onoff);

#endif /* __WIDGETS_H_ */
