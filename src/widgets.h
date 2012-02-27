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
void WidgetManage(Widget w);
void WidgetUnmanage(Widget w);
int WidgetIsManaged(Widget w);
void *WidgetGetUserData(Widget w);
void WidgetSetUserData(Widget w, void *udata);
void WidgetSetSensitive(Widget w, int onoff);
void WidgetSetHeight(Widget w, unsigned int height);
void WidgetSetSize(Widget w, unsigned int width, unsigned int height);
void WidgetGetSize(Widget w, unsigned int *width, unsigned int *height);
void WidgetSetMinWidth(Widget w, unsigned int width);

typedef void (*Key_CBProc)(void *anydata);
typedef struct {
    Widget w;
    int modifiers;
    int key;
    Key_CBProc cbproc;
    void *anydata;
} Key_CBData;

void AddWidgetKeyPressCB(Widget w, int key, Key_CBProc cbproc, void *anydata);
void AddWidgetKeyPressCB2(Widget w, int modifiers, int key, Key_CBProc cbproc, void *anydata);

typedef struct _Widget_CBData Widget_CBData;
typedef void (*Widget_CBProc)(Widget_CBData *wcbdata);
struct _Widget_CBData {
    Widget w;
    Widget_CBProc cbproc;
    void *anydata;
    void *calldata;
};
void AddWidgetCB(Widget w, const char *callback, Widget_CBProc cbproc, void *anydata);

/* Dialog */
Widget CreateDialog(Widget parent, const char *s);
void DialogRaise(Widget form);
void DialogClose(Widget form);
void DialogSetResizable(Widget form, int onoff);

/* File selection dialog */
typedef struct {
    Widget dialog;
    Widget FSB;
    Widget rc;
} FSBStructure;
FSBStructure *CreateFileSelectionBox(Widget parent, char *s);

typedef int (*FSB_CBProc)(
    FSBStructure *fsbp,
    char *,              /* filename */
    void *               /* data the application registered */
);
void AddFileSelectionBoxCB(FSBStructure *fsbp, FSB_CBProc cbproc, void *anydata);
void SetFileSelectionBoxPattern(FSBStructure *fsb, char *pattern);

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
void LabelSetString(Widget w, char *s);
void LabelSetPixmap(Widget w, int width, int height, const unsigned char *bits);

/* Text */
Widget CreateLineTextEdit(Widget parent, int len);
Widget CreateMultiLineTextEdit(Widget parent, int nrows);

typedef struct {
    Widget label;
    Widget form;
    Widget text;
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

/* Tab */
Widget CreateTab(Widget parent);
Widget CreateTabPage(Widget parent, char *s);
void SelectTabPage(Widget tab, Widget w);

/* Button */
typedef void (*Button_CBProc)(
    Widget but,
    void *               /* data the application registered */
);
Widget CreateButton(Widget parent, char *label);
Widget CreateBitmapButton(Widget parent,
    int width, int height, const unsigned char *bits);

/* ToggleButton */
typedef void (*TB_CBProc)(
    Widget but,
    int onoff,           /* True/False */
    void *               /* data the application registered */
);
Widget CreateToggleButton(Widget parent, char *s);
int GetToggleButtonState(Widget w);
void SetToggleButtonState(Widget w, int value);

/* Paned window */
Widget CreatePanedWindow(Widget parent);

/* Grid */
Widget CreateGrid(Widget parent, int ncols, int nrows);
void PlaceGridChild(Widget grid, Widget w, int col, int row);

/* OptionChoice */
typedef struct _OptionStructure OptionStructure;
typedef struct {
    int value;
    char *label;
} OptionItem;
OptionStructure *CreateOptionChoice(Widget parent, char *labelstr, int ncols,
                                                int nchoices, OptionItem *items);
OptionStructure *CreateOptionChoiceVA(Widget parent, char *labelstr, ...);
int GetOptionChoice(OptionStructure *opt);
void UpdateOptionChoice(OptionStructure *optp, int nchoices, OptionItem *items);

typedef void (*OC_CBProc)(
    OptionStructure *opt,
    int value,           /* value */
    void *               /* data the application registered */
);

typedef struct {
    int value;
    Widget widget;
} OptionWidgetItem;

typedef struct {
    OptionStructure *opt;
    OC_CBProc cbproc;
    void *anydata;
} OC_CBdata;

struct _OptionStructure {
    int nchoices;
    int ncols;  /* preferred number of columns */
    Widget menu;
    Widget pulldown;
    OptionWidgetItem *options;

    unsigned int cbnum;
    OC_CBdata **cblist;
};
void AddOptionChoiceCB(OptionStructure *opt, OC_CBProc cbproc, void *anydata);

/* Menu */
Widget CreatePopupMenu(Widget parent);
Widget CreateMenuBar(Widget parent);
Widget CreateMenu(Widget parent, char *label, char mnemonic, int help);
Widget CreateMenuButton(Widget parent, char *label, char mnemonic,
        Button_CBProc cb, void *data);
Widget CreateMenuButtonA(Widget parent, char *label, char mnemonic,
        char *accelerator, char* acceleratorText, Button_CBProc cb, void *data);
Widget CreateMenuHelpButton(Widget parent, char *label, char mnemonic,
    Widget form, char *ha);
Widget CreateMenuToggle(Widget parent, char *label, char mnemonic,
        TB_CBProc cb, void *data);

#endif /* __WIDGETS_H_ */
