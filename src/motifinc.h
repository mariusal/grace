/*
 * Grace - Graphics for Exploratory Data Analysis
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-95 Paul J Turner, Portland, OR
 * Copyright (c) 1996-99 Grace Development Team
 * 
 * Maintained by Evgeny Stambulchik <fnevgeny@plasma-gate.weizmann.ac.il>
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

/*
 * for Motif specific items
 */

#ifndef __MOTIFINC_H_
#define __MOTIFINC_H_

/* 
 * Accept/Apply/Close for aac_cb callbacks
 */
#define AAC_ACCEPT  0
#define AAC_APPLY   1
#define AAC_CLOSE   2

#define LIST_TYPE_SINGLE    0
#define LIST_TYPE_MULTIPLE  1

#define SPIN_TYPE_INT       0
#define SPIN_TYPE_FLOAT     1

#define ALIGN_BEGINNING     0
#define ALIGN_CENTER        1
#define ALIGN_END           2

#define CreateMenuSeparator(w) CreateSeparator(w)

extern Widget app_shell;        /* defined in xmgrace.c */

/* set selection gadget */
typedef struct _SetChoiceItem {
    int type;
    int display;
    int gno;
    int spolicy;
    int indx;
    Widget list;
} SetChoiceItem;

typedef struct {
    int value;
    char *label;
} OptionItem;

typedef struct {
    int value;
    unsigned char *bitmap;
} BitmapOptionItem;

typedef struct {
    int value;
    Widget widget;
} OptionWidgetItem;

typedef struct {
    int nchoices;
    Widget menu;
    Widget pulldown;
    OptionWidgetItem *options;
} OptionStructure;

typedef struct {
    int nchoices;
    int *values;
    void *anydata;
    Widget rc;
    Widget list;
} ListStructure;

typedef struct {
    int type;
    double min;
    double max;
    double incr;
    Widget rc;
    Widget text;
    Widget arrow_up;
    Widget arrow_down;
} SpinStructure;

typedef struct {
    Widget label;
    Widget form;
    Widget text;
} TextStructure;

typedef struct {
    Widget popup;
    Widget label_item;
    Widget shownd_item;
    Widget showh_item;
    Widget hide_item;
    Widget show_item;
    Widget bringf_item;
    Widget sendb_item;
    Widget duplicate_item;
    Widget kill_item;
    Widget killd_item;
    Widget copy12_item;
    Widget copy21_item;
    Widget move12_item;
    Widget move21_item;
    Widget swap_item;
    Widget edit_item;
} SetPopupMenu;

typedef enum {
    SetMenuHideCB,
    SetMenuShowCB,
    SetMenuBringfCB,
    SetMenuSendbCB,
    SetMenuDuplicateCB,
    SetMenuKillCB,
    SetMenuKillDCB,
    SetMenuCopy12CB,
    SetMenuCopy21CB,
    SetMenuMove12CB,
    SetMenuMove21CB,
    SetMenuSwapCB,
    SetMenuNewFCB,
    SetMenuNewSCB,
    SetMenuNewECB,
    SetMenuNewBCB,
    SetMenuEditSCB,
    SetMenuEditECB,
    SetMenuPackCB
} SetMenuCBtype;

typedef struct {
    int standalone;
    int gno;
    int show_hidden;
    int show_nodata;
    SetPopupMenu *menu;
} SetChoiceData;

typedef struct {
    Widget dialog;
    Widget FSB;
    Widget rc;
} FSBStructure;

/* OptionChoice CB procedure */
typedef void (*OC_CBProc)(
    int value,           /* value */
    void *               /* data the application registered */
);

/* ToggleButton CB procedure */
typedef void (*TB_CBProc)(
    int onoff,           /* True/False */
    void *               /* data the application registered */
);

/* FileSelectionBox CB procedure */
typedef int (*FSB_CBProc)(
    char *,              /* filename */
    void *               /* data the application registered */
);

/* Button CB procedure */
typedef void (*Button_CBProc)(
    void *               /* data the application registered */
);

/* List CB procedure */
typedef void (*List_CBProc)(
    int n,               /* # of items selected */
    int *values,         /* list of values of the selected items */
    void *               /* data the application registered */
);

Widget CreateFrame(Widget parent, char *s);

Widget CreateSeparator(Widget parent);

Widget CreateTab(Widget parent);
Widget CreateTabPage(Widget parent, char *s);
void SelectTabPage(Widget tab, Widget w);

Widget CreateCharSizeChoice(Widget parent, char *s);
double GetCharSizeChoice(Widget w);
void SetCharSizeChoice(Widget w, double size);

Widget CreateAngleChoice(Widget parent, char *s);
int GetAngleChoice(Widget w);
void SetAngleChoice(Widget w, int angle);

Widget CreateToggleButton(Widget parent, char *s);
int GetToggleButtonState(Widget w);
void SetToggleButtonState(Widget w, int value);
void AddToggleButtonCB(Widget w, TB_CBProc cbproc, void *anydata);

Widget CreateButton(Widget parent, char *label);
Widget CreateBitmapButton(Widget parent,
    int width, int height, const unsigned char *bits);
void AddButtonCB(Widget button, Button_CBProc cbproc, void *data);

Widget CreateAACButtons(Widget parent, Widget form, Button_CBProc aac_cb);

OptionStructure *CreateOptionChoice(Widget parent, char *labelstr, int ncols,
                                                int nchoices, OptionItem *items);
OptionStructure *CreateBitmapOptionChoice(Widget parent, char *labelstr, int ncols,
                int nchoices, int width, int height, BitmapOptionItem *items);
void SetOptionChoice(OptionStructure *opt, int value);
int GetOptionChoice(OptionStructure *opt);
void UpdateOptionChoice(OptionStructure *optp, int nchoices, OptionItem *items);

void AddOptionChoiceCB(OptionStructure *opt, TB_CBProc cbproc, void *anydata);

ListStructure *CreateListChoice(Widget parent, char *labelstr, int type,
                                int nvisible, int nchoices, OptionItem *items);
int SelectListChoice(ListStructure *listp, int choice);
void SelectListChoices(ListStructure *listp, int nchoices, int *choices);
void UpdateListChoice(ListStructure *listp, int nchoices, OptionItem *items);
int GetListChoices(ListStructure *listp, int **values);
int GetSingleListChoice(ListStructure *listp, int *value);
void AddListChoiceCB(ListStructure *listp, List_CBProc cbproc, void *anydata);

void list_selectall_action(Widget w, XEvent *e, String *par, Cardinal *npar);
void list_unselectall_action(Widget w, XEvent *e, String *par, Cardinal *npar);
void list_invertselection_action(Widget w, XEvent *e, String *par,
                                 Cardinal *npar);

SpinStructure *CreateSpinChoice(Widget parent, char *s, int len,
                        int type, double min, double max, double incr);
double GetSpinChoice(SpinStructure *spinp);
void SetSpinChoice(SpinStructure *spinp, double value);

TextStructure *CreateTextInput(Widget parent, char *s);
TextStructure *CreateCSText(Widget parent, char *s);
char *GetTextString(TextStructure *cst);
void SetTextString(TextStructure *cst, char *s);
int GetTextCursorPos(TextStructure *cst);
void TextInsert(TextStructure *cst, int pos, char *s);
void cstext_edit_action(Widget w, XEvent *e, String *par, Cardinal *npar);

FSBStructure *CreateFileSelectionBox(Widget parent, char *s, char *pattern);
void AddFileSelectionBoxCB(FSBStructure *fsbp, FSB_CBProc cbproc, void *anydata);
void SetFileSelectionBoxPattern(FSBStructure *fsb, char *pattern);

Widget CreateLabel(Widget parent, char *s);

OptionStructure *CreateFontChoice(Widget parent, char *s);
OptionStructure *CreatePatternChoice(Widget parent, char *s);
OptionStructure *CreateLineStyleChoice(Widget parent, char *s);
OptionStructure *CreateSetTypeChoice(Widget parent, char *s);
OptionStructure *CreateColorChoice(Widget parent, char *s);
OptionStructure *CreateFormatChoice(Widget parent, char *s);

ListStructure *CreateGraphChoice(Widget parent, char *labelstr, int type);

ListStructure *CreateSetChoice(Widget parent, char *labelstr, 
                                        int type, int standalone);
void UpdateSetChoice(ListStructure *listp, int gno);

void hide_set_proc(void *data);
void show_set_proc(void *data);
void duplicate_set_proc(void *data);
void kill_set_proc(void *data);
void killd_set_proc(void *data);
void copy12_set_proc(void *data);
void copy21_set_proc(void *data);
void move12_set_proc(void *data);
void move21_set_proc(void *data);
void swap_set_proc(void *data);
void newF_set_proc(void *data);
void newS_set_proc(void *data);
void newE_set_proc(void *data);
void newB_set_proc(void *data);
void editS_set_proc(void *data);
void editE_set_proc(void *data);

SpinStructure *CreateLineWidthChoice(Widget parent, char *s);


SetChoiceItem CreateSetSelector(Widget parent, char *label, int type, int ff, int gtype, int stype);
int GetSelectedSet(SetChoiceItem l);
int GetSelectedSets(SetChoiceItem l, int **sets);
void update_set_list(int gno, SetChoiceItem l);
int save_set_list(SetChoiceItem l);
void update_save_set_list( SetChoiceItem l, int newgr );

int SetSelectedSet(int gno, int setno, SetChoiceItem l);


Widget *CreatePanelChoice(Widget parent, char *labstr, int nchoices, ...);
void SetChoice(Widget * w, int value);
int GetChoice(Widget * w);

Widget CreateTextItem2(Widget parent, int len, char *s);
Widget CreateTextItem4(Widget parent, int len, char *s);
Widget CreateScrollTextItem2(Widget parent, int hgt, char *s);

Widget CreateCommandButtons(Widget parent, int n, Widget * buts, char **l);
Widget CreateCommandButtonsNoDefault(Widget parent, int n, Widget * buts, char **l);

Widget *CreatePrecisionChoice(Widget parent, char *s);

void SetLabel(Widget w, char *s);
void AlignLabel(Widget w, int alignment);
void SetFixedFont(Widget w);

Widget CreateMenuBar(Widget parent);
Widget CreateMenu(Widget parent, char *label, char mnemonic, int help);
Widget CreateMenuButton(Widget parent, char *label, char mnemonic,
	Button_CBProc cb, void *data);
Widget CreateMenuToggle(Widget parent, char *label, char mnemonic,
	TB_CBProc cb, void *data);
Widget CreateMenuLabel(Widget parent, char *name);

char *GetStringSimple(XmString xms);

char *xv_getstr(Widget w);
Boolean xv_evalexpr(Widget w, double *);
Boolean xv_evalexpri(Widget w, int *);
void xv_setstr(Widget w, char *s);
void handle_close(Widget w);
void XtRaise(Widget w);
void destroy_dialog(Widget w, XtPointer client_data, XtPointer call_data);
void savewidget(Widget w);
void deletewidget(Widget w);

#endif /* __MOTIFINC_H_ */
