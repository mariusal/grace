/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2002 Grace Development Team
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

/* for Widget */
#include <X11/Intrinsic.h>
/* for XmString; TODO: remove! */
#include <Xm/Xm.h>

/* for Pen */
#include "defines.h"

/* for Storage */
#include "storage.h"
#include "grace.h"

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
    int ncols;  /* preferred number of columns */
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

/* Storage labeling procedure */
typedef char * (*Storage_LabelingProc)(
    unsigned int step,   /* # of the current item */
    void *data           /* data of the item */
);

typedef struct _StorageStructure StorageStructure;

typedef void (*SS_PopupCBProc)(
    StorageStructure *ss,
    int nselected         /* # of selected list items */
);

struct _StorageStructure {
    int nchoices;
    void **values;
    void *anydata;
    
    Storage *sto;
    Storage_LabelingProc labeling_proc;
    Storage *clipboard;

    Widget rc;
    Widget list;
    Widget popup;
    Widget selpopup;
    
    Widget popup_cut_bt;
    Widget popup_copy_bt;
    Widget popup_paste_bt;
    
    Widget popup_delete_bt;
    Widget popup_duplicate_bt;

    Widget popup_bring_to_front_bt;
    Widget popup_send_to_back_bt;
    Widget popup_move_up_bt;
    Widget popup_move_down_bt;

    Widget popup_select_all_bt;
    Widget popup_unselect_all_bt;
    Widget popup_invert_selection_bt;
    
    SS_PopupCBProc popup_cb;
    void *data;
};

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
    Widget frame;
    StorageStructure *graph_sel;
    StorageStructure *set_sel;
} GraphSetStructure;

typedef struct {
    Widget form;
    GraphSetStructure *src;
    GraphSetStructure *dest;
} SrcDestStructure;

typedef struct {
    Widget dialog;
    Widget FSB;
    Widget rc;
} FSBStructure;

typedef struct {
    Widget frame;
    OptionStructure *r_sel;
    Widget negate;
} RestrictionStructure;

typedef struct {
    Widget form;
    Widget menubar;
    SrcDestStructure *srcdest;
} TransformStructure;


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

/* Storage CB procedure */
typedef void (*Storage_CBProc)(
    int n,               /* # of items selected */
    void **values,       /* list of values of the selected items */
    void *data           /* data the application registered */
);

/* Storage double click CB procedure */
typedef void (*Storage_DCCBProc)(
    void *value,         /* list of values of the selected items */
    void *data           /* data the application registered */
);

/* Spin Button CB procedure */
typedef void (*Spin_CBProc)(
    double,             /* value of spinner                 */
    void *              /* data the application registered */
);

/* Text input CB procedure */
typedef void (*Text_CBProc)(
    char *,              /* text string */
    void *               /* data the application registered */
);

/*
 * Scale input CB procedure */
typedef void (*Scale_CBProc )(
    int,                /* scale value                     */
    void *              /* data the application registered */
);

/* AAC Dialog CB procedure */
typedef int (*AACDialog_CBProc)(
    void *               /* data the application registered */
);

void ManageChild(Widget w);
void UnmanageChild(Widget w);
void SetSensitive(Widget w, int onoff);

Widget GetParent(Widget w);

void RegisterEditRes(Widget shell);

void SetDimensions(Widget w, unsigned int width, unsigned int height);
void GetDimensions(Widget w, unsigned int *width, unsigned int *height);

void *GetUserData(Widget w);
void SetUserData(Widget w, void *udata);

Widget CreateDialogForm(Widget parent, const char *s);
void SetDialogFormResizable(Widget form, int onoff);
void AddDialogFormChild(Widget form, Widget child);
void FixateDialogFormChild(Widget w);

Widget CreateFrame(Widget parent, char *s);

Widget CreateSeparator(Widget parent);

Widget CreateVContainer(Widget parent);
Widget CreateHContainer(Widget parent);

Widget CreateGrid(Widget parent, int ncols, int nrows);
void PlaceGridChild(Widget grid, Widget w, int col, int row);

Widget CreateTab(Widget parent);
Widget CreateTabPage(Widget parent, char *s);
void SelectTabPage(Widget tab, Widget w);

Widget CreateScale(Widget parent, char *s, int min, int max, int delta);
void SetScaleValue(Widget w, int value);
int GetScaleValue(Widget w);
void SetScaleWidth(Widget w, int width);
void AddScaleCB(Widget w, Scale_CBProc cbproc, void *);

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

void CreateAACDialog(Widget form,
    Widget container, AACDialog_CBProc cbproc, void *data);

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

StorageStructure *CreateStorageChoice(Widget parent,
    char *labelstr, int type, int nvisible);
void SetStorageChoiceLabeling(StorageStructure *ss, Storage_LabelingProc proc);
int GetStorageChoices(StorageStructure *ss, void ***values);
int GetSingleStorageChoice(StorageStructure *ss, void **value);
int SelectStorageChoice(StorageStructure *ss, void *choice);
int SelectStorageChoices(StorageStructure *ss, int nchoices, void **choices);
void UpdateStorageChoice(StorageStructure *ss);
void SetStorageChoiceStorage(StorageStructure *ss, Storage *sto);
void AddStorageChoiceCB(StorageStructure *ss,
    Storage_CBProc cbproc, void *anydata);
void AddStorageChoiceDblClickCB(StorageStructure *ss,
    Storage_DCCBProc cbproc, void *anydata);

SpinStructure *CreateSpinChoice(Widget parent, char *s, int len,
                        int type, double min, double max, double incr);
double GetSpinChoice(SpinStructure *spinp);
void SetSpinChoice(SpinStructure *spinp, double value);
void AddSpinButtonCB(SpinStructure *spinp, Spin_CBProc cbproc, void *data);

TextStructure *CreateTextInput(Widget parent, char *s);
TextStructure *CreateCSText(Widget parent, char *s);
char *GetTextString(TextStructure *cst);
void SetTextString(TextStructure *cst, char *s);
void AddTextInputCB(TextStructure *cst, Text_CBProc cbproc, void *data);
int GetTextCursorPos(TextStructure *cst);
void TextInsert(TextStructure *cst, int pos, char *s);
void SetTextEditable(TextStructure *cst, int onoff);
void cstext_edit_action(Widget w, XEvent *e, String *par, Cardinal *npar);

FSBStructure *CreateFileSelectionBox(Widget parent, char *s);
void AddFileSelectionBoxCB(FSBStructure *fsbp, FSB_CBProc cbproc, void *anydata);
void SetFileSelectionBoxPattern(FSBStructure *fsb, char *pattern);

Widget CreateLabel(Widget parent, char *s);

OptionStructure *CreateFontChoice(Widget parent, char *s);
OptionStructure *CreatePatternChoice(Widget parent, char *s);
OptionStructure *CreateLineStyleChoice(Widget parent, char *s);
OptionStructure *CreateSetTypeChoice(Widget parent, char *s);
OptionStructure *CreateColorChoice(Widget parent, char *s);
OptionStructure *CreateFormatChoice(Widget parent, char *s);
OptionStructure *CreateASChoice(Widget parent, char *s);
OptionStructure *CreateJustChoice(Widget parent, char *s);

Widget CreatePenChoice(Widget parent, char *s);
void SetPenChoice(Widget button, Pen *pen);
int GetPenChoice(Widget pen_button, Pen *pen);

RestrictionStructure *CreateRestrictionChoice(Widget parent, char *s);

StorageStructure *CreateGraphChoice(Widget parent, char *labelstr, int type);

StorageStructure *CreateSetChoice(Widget parent, char *labelstr, 
                                        int type, StorageStructure *graphss);
void UpdateSetChoice(StorageStructure *ss);
Quark *get_set_choice_gr(StorageStructure *ss);

GraphSetStructure *CreateGraphSetSelector(Widget parent, char *s, int sel_type);
SrcDestStructure *CreateSrcDestSelector(Widget parent, int sel_type);
void UpdateSrcDestSelector(SrcDestStructure *srcdest);

void switch_focus_proc(void *data);
void hide_graph_proc(void *data);
void show_graph_proc(void *data);
void duplicate_graph_proc(void *data);
void kill_graph_proc(void *data);
void copy12_graph_proc(void *data);
void copy21_graph_proc(void *data);
void move12_graph_proc(void *data);
void move21_graph_proc(void *data);
void swap_graph_proc(void *data);
void create_new_graph_proc(void *data);

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

OptionStructure *CreatePanelChoice(Widget parent, char *labstr, int nchoices, ...);
OptionStructure *CreatePrecisionChoice(Widget parent, char *s);

Widget CreateTextItem2(Widget parent, int len, char *s);
Widget CreateTextItem4(Widget parent, int len, char *s);
Widget CreateScrollTextItem2(Widget parent, int hgt, char *s);
void AddTextItemCB(Widget ti, Text_CBProc cbproc, void *data);

Widget CreateCommandButtons(Widget parent, int n, Widget * buts, char **l);
Widget CreateCommandButtonsNoDefault(Widget parent, int n, Widget * buts, char **l);

TransformStructure *CreateTransformDialogForm(Widget parent,
    const char *s, int sel_type);
int GetTransformDialogSettings(TransformStructure *tdialog, int exclusive,
    int *nssrc, Quark ***srcsets, Quark ***destsets);

void SetLabel(Widget w, char *s);
void AlignLabel(Widget w, int alignment);
void SetFixedFont(Widget w);

Widget CreateMenuBar(Widget parent);
Widget CreateMenu(Widget parent, char *label, char mnemonic, int help);
Widget CreateMenuButton(Widget parent, char *label, char mnemonic,
	Button_CBProc cb, void *data);
Widget CreateMenuCloseButton(Widget parent, Widget shell);
Widget CreateMenuHelpButton(Widget parent, char *label, char mnemonic,
    Widget form, char *ha);
Widget CreateMenuToggle(Widget parent, char *label, char mnemonic,
	TB_CBProc cb, void *data);
Widget CreateMenuLabel(Widget parent, char *name);

void AddHelpCB(Widget w, char *ha);
void ContextHelpCB(void *data);

char *GetStringSimple(XmString xms);

char *xv_getstr(Widget w);
Boolean xv_evalexpr(Widget w, double *);
Boolean xv_evalexpri(Widget w, int *);
void xv_setstr(Widget w, char *s);
void handle_close(Widget w);
void RaiseWindow(Widget w);
void destroy_dialog(Widget w, XtPointer client_data, XtPointer call_data);
void destroy_dialog_cb(void *data);
void savewidget(Widget w);
void deletewidget(Widget w);

#endif /* __MOTIFINC_H_ */
