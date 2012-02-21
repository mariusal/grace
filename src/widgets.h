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

Widget CreateVContainer(Widget parent);
Widget CreateHContainer(Widget parent);

void DialogSetResizable(Widget form, int onoff);

Widget CreateForm(Widget parent);
void FormAddHChild(Widget form, Widget child);
void FormAddVChild(Widget form, Widget child);
void FormFixateVChild(Widget w);

Widget CreateLabel(Widget parent, char *s);
void SetLabel(Widget w, char *s);
void AlignLabel(Widget w, int alignment);

Widget CreateLineTextEdit(Widget parent, int len);
Widget CreateMultiLineTextEdit(Widget parent, int nrows);

typedef struct {
    Widget label;
    Widget form;
    Widget text;
    Boolean locked;
    int multiline;
} TextStructure;

TextStructure *CreateTextInput(Widget parent, char *s);
TextStructure *CreateTextInput2(Widget parent, char *s, int len);
TextStructure *CreateScrolledTextInput(Widget parent, char *s, int nrows);
TextStructure *CreateCSText(Widget parent, char *s);
TextStructure *CreateScrolledCSText(Widget parent, char *s, int nrows);
void SetTextInputLength(TextStructure *cst, int len);
char *GetTextString(TextStructure *cst);
void SetTextString(TextStructure *cst, char *s);
int xv_evalexpr(TextStructure *cst, double *);
int xv_evalexpr2(Widget w, double *);
int xv_evalexpri(TextStructure *cst, int *);

/* Text input CB procedure */
typedef void (*Text_CBProc)(
    TextStructure *cst,
    char *,              /* text string */
    void *               /* data the application registered */
);
void AddTextInputCB(TextStructure *cst, Text_CBProc cbproc, void *data);

/* Text validate CB procedure */
typedef int (*TextValidate_CBProc)(
        char **value,
        int *length,
        void *data
);
void AddTextValidateCB(TextStructure *cst, TextValidate_CBProc cbproc, void *anydata);
int GetTextCursorPos(TextStructure *cst);
void SetTextCursorPos(TextStructure *cst, int pos);
int GetTextLastPosition(TextStructure *cst);
void TextInsert(TextStructure *cst, int pos, char *s);
void SetTextEditable(TextStructure *cst, int onoff);

#endif /* __WIDGETS_H_ */
