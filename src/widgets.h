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

/* Timer */
typedef void (*Timer_CBProc)(void *anydata);
typedef struct {
    unsigned long timer_id;
    unsigned long interval;
    Timer_CBProc cbproc;
    void *anydata;
} Timer_CBdata;
Timer_CBdata *CreateTimer(unsigned long interval, Timer_CBProc cbproc, void *anydata);
void TimerStart(Timer_CBdata *cbdata);

/* Widgets */
void WidgetManage(Widget w);
void WidgetUnmanage(Widget w);
int WidgetIsManaged(Widget w);
void *WidgetGetUserData(Widget w);
void WidgetSetUserData(Widget w, void *udata);
void WidgetSetSensitive(Widget w, int onoff);
void WidgetSetFocus(Widget w);
void WidgetSetWidth(Widget w, unsigned int width);
void WidgetSetHeight(Widget w, unsigned int height);
void WidgetSetSize(Widget w, unsigned int width, unsigned int height);
void WidgetGetSize(Widget w, unsigned int *width, unsigned int *height);

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

void AddWidgetButtonPressCB(Widget w, int button, Key_CBProc cbproc, void *anydata);

typedef struct {
    char **text;
    int allow_change;
} TextValidate_CD;

typedef struct _Widget_CBData Widget_CBData;
typedef void (*Widget_CBProc)(Widget_CBData *wcbdata);
struct _Widget_CBData {
    Widget w;
    Widget_CBProc cbproc;
    void *anydata;
    void *calldata;
};
void AddWidgetCB(Widget w, const char *callback, Widget_CBProc cbproc, void *anydata);

/* Dialog Window */
Widget CreateDialogWindow(Widget parent, const char *s);

/* Dialog */
Widget CreateDialog(Widget parent, const char *s);
void DialogRaise(Widget form);
void DialogClose(Widget form);
void DialogSetResizable(Widget form, int onoff);

/* File selection box */
Widget CreateFileSelectionBox(Widget parent);

/* File selection filter */
void CreateFileSelectionFilter(Widget parent, Widget fsb);

/* File selection dialog */
typedef struct {
    Widget FSB;
    Widget rc;
} FSBStructure;
FSBStructure *CreateFSBDialog(Widget parent, char *s);

typedef int (*FSB_CBProc)(
    FSBStructure *fsbp,
    char *,              /* filename */
    void *               /* data the application registered */
);
void AddFSBDialogCB(FSBStructure *fsbp, FSB_CBProc cbproc, void *anydata);
void FSBDialogSetPattern(FSBStructure *fsb, char *pattern);
void FSBDialogSetDirectory(FSBStructure *fsb, char *directory);

/* Containers */
Widget CreateVContainer(Widget parent);
Widget CreateHContainer(Widget parent);

/* Form */
Widget CreateForm(Widget parent);
void FormAddHChild(Widget form, Widget child);
void FormAddVChild(Widget form, Widget child);
void FormFixateHChild(Widget w);
void FormFixateVChild(Widget w);

/* Grid */
Widget CreateGrid(Widget parent, int ncols, int nrows);
void PlaceGridChild(Widget grid, Widget w, int col, int row);

/* Frame */
Widget CreateFrame(Widget parent, char *s);

/* Scrolled window */
Widget CreateScrolledWindow(Widget parent);

/* Paned window */
Widget CreatePanedWindow(Widget parent);
void PanedWindowSetMinWidth(Widget w, unsigned int width);

/* Tab */
Widget CreateTab(Widget parent);
Widget CreateTabPage(Widget parent, char *s);
void SelectTabPage(Widget tab, Widget w);

/* Separator */
Widget CreateSeparator(Widget parent);

/* Label */
Widget CreateLabel(Widget parent, char *s);
void LabelSetString(Widget w, char *s);
void LabelSetPixmap(Widget w, Pixmap pixmap);

/* Text edit */
Widget CreateLineTextEdit(Widget parent, int len);
Widget CreateMultiLineTextEdit(Widget parent, int nrows);
char *TextEditGetString(Widget w);
void TextEditSetString(Widget w, char *s);

/* Text */
typedef struct {
    Widget label;
    Widget form;
    Widget text;
    int locked;
} TextStructure;

void TextInsertString(TextStructure *cst, int pos, char *s);
int TextGetCursorPos(TextStructure *cst);
void TextSetCursorPos(TextStructure *cst, int pos);
int TextGetLastPosition(TextStructure *cst);
void TextSetLength(TextStructure *cst, int len);
void TextSetEditable(TextStructure *cst, int onoff);

/* Button */
typedef void (*Button_CBProc)(
    Widget but,
    void *               /* data the application registered */
);
Widget CreateButton(Widget parent, char *label);
Widget CreateBitmapButton(Widget parent,
    int width, int height, const unsigned char *bits);

#define ARROW_UP   1
#define ARROW_DOWN 2
Widget CreateArrowButton(Widget parent, int arrow_type);

/* ToggleButton */
typedef void (*TB_CBProc)(
    Widget but,
    int onoff,           /* True/False */
    void *               /* data the application registered */
);
Widget CreateToggleButton(Widget parent, char *s);
int ToggleButtonGetState(Widget w);
void ToggleButtonSetState(Widget w, int value);

/* Scale */
Widget CreateScale(Widget parent, char *s, int min, int max, int delta);
void ScaleSetValue(Widget w, int value);
int ScaleGetValue(Widget w);

/* ComboBox */
typedef struct {
    Widget combobox;
    Widget popup;
} ComboBoxStructure;
ComboBoxStructure *CreateComboBox(Widget parent);

/* OptionChoice */
typedef struct _OptionStructure OptionStructure;
typedef struct {
    int value;
    char *label;
    Pixmap pixmap;
    unsigned long background;
    unsigned long foreground;
} LabelOptionItem;
OptionStructure *CreateLabelOptionChoice(Widget parent, char *labelstr, int ncols,
                                                int nchoices, LabelOptionItem *items);
OptionStructure *CreateLabelOptionChoiceVA(Widget parent, char *labelstr, ...);

typedef struct {
    int value;
    unsigned char *bitmap;
} BitmapOptionItem;
OptionStructure *CreateBitmapOptionChoice(Widget parent, char *labelstr, int ncols,
                int nchoices, int width, int height, BitmapOptionItem *items);
OptionStructure *CreateCharOptionChoice(Widget parent, char *s);
void UpdateCharOptionChoice(OptionStructure *opt, int font);
void SetOptionChoice(OptionStructure *opt, int value);
int GetOptionChoice(OptionStructure *opt);
void UpdateLabelOptionChoice(OptionStructure *optp, int nchoices, LabelOptionItem *items);
void OptionChoiceSetColorUpdate(OptionStructure *opt, int update);

typedef void (*OC_CBProc)(
    OptionStructure *opt,
    int value,           /* value */
    void *               /* data the application registered */
);

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
    LabelOptionItem *items;
    int cvalue;
    int update_colors;

    unsigned int cbnum;
    OC_CBdata **cblist;
};
void AddOptionChoiceCB(OptionStructure *opt, OC_CBProc cbproc, void *anydata);

/* Menu */
Widget CreatePopupMenu(Widget parent);
void PopupMenuShow(Widget w, void *data);
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
Widget CreateMenuSeparator(Widget parent);

#endif /* __WIDGETS_H_ */
