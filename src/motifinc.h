/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2006 Grace Development Team
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

/*
 * for Motif specific items
 */

#ifndef __MOTIFINC_H_
#define __MOTIFINC_H_

#include "gui.h"
#include "graceapp.h"

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

#define TABLE_CELL_NONE    0
#define TABLE_CELL_STRING  1
#define TABLE_CELL_PIXMAP  2

#define DROP_ACTION_MOVE 0
#define DROP_ACTION_COPY 1

extern Widget app_shell;        /* defined in xmgapp.c */

typedef struct {
    int nchoices;
    int *values;
    void *anydata;
    Widget rc;
    Widget list;
} ListStructure;

/* Storage labeling procedure */
typedef char * (*Storage_LabelingProc)(
    Quark *q,
    unsigned int *rid
);

typedef struct _StorageStructure StorageStructure;

typedef void (*SS_PopupCBProc)(
    StorageStructure *ss,
    int nselected         /* # of selected list items */
);

struct _StorageStructure {
    int nchoices;
    Quark **values;
    void *anydata;
    
    Quark *q;
    Storage_LabelingProc labeling_proc;
    
    StorageStructure *governor;

    Widget rc;
    Widget list;
    Widget popup;
    Widget selpopup;
    
    Widget popup_hide_bt;
    Widget popup_show_bt;

    Widget popup_delete_bt;
    Widget popup_duplicate_bt;

    Widget popup_bring_to_front_bt;
    Widget popup_send_to_back_bt;
    Widget popup_move_up_bt;
    Widget popup_move_down_bt;

    Widget popup_properties_bt;

    Widget popup_select_all_bt;
    Widget popup_unselect_all_bt;
    Widget popup_invert_selection_bt;
    
    SS_PopupCBProc popup_cb;
    void *data;
};

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
    Widget frame;
    StorageStructure *ssd_sel;
    StorageStructure *set_sel;
} SSDSetStructure;

typedef struct {
    Widget frame;
    StorageStructure *ssd_sel;
    ListStructure *col_sel;
} SSDColStructure;

typedef struct {
    Widget form;
    SSDSetStructure *src;
    SSDSetStructure *dest;
} SrcDestStructure;

typedef struct {
    Widget frame;
    OptionStructure *r_sel;
    Widget negate;
} RestrictionStructure;

typedef struct _TransformStructure TransformStructure;

typedef void* (*TDBuild_CBProc)(
    TransformStructure *tdialog
);

typedef void* (*TDGet_CBProc)(
    void *gui           /* specific GUI structure */
);

typedef void (*TDFree_CBProc)(
    void *tddata        /* data */
);

typedef int (*TDRun_CBProc)(
    Quark *psrc,
    Quark *pdest,
    void *tddata        /* data */
);

struct _TransformStructure {
    Widget form;
    Widget menubar;
    Widget frame;
    SrcDestStructure *srcdest;
    int exclusive;
    TDBuild_CBProc build_cb;
    TDGet_CBProc   get_cb;
    TDFree_CBProc  free_cb;
    TDRun_CBProc   run_cb;
    void *gui;
};

typedef struct {
    TDBuild_CBProc build_cb;
    TDGet_CBProc   get_cb;
    TDFree_CBProc  free_cb;
    TDRun_CBProc   run_cb;
} TD_CBProcs;

/* List CB procedure */
typedef void (*List_CBProc)(
    ListStructure *listp,
    int n,               /* # of items selected */
    int *values,         /* list of values of the selected items */
    void *               /* data the application registered */
);

/* Storage CB procedure */
typedef void (*Storage_CBProc)(
    StorageStructure *ss,
    int n,               /* # of items selected */
    Quark **values,      /* list of values of the selected items */
    void *data           /* data the application registered */
);

/* Storage double click CB procedure */
typedef void (*Storage_DCCBProc)(
    StorageStructure *ss,
    Quark *value,        /* list of values of the selected items */
    void *data           /* data the application registered */
);

/* AAC Dialog CB procedure */
typedef int (*AACDialog_CBProc)(
    void *               /* data the application registered */
);

/* PenChoice input CB procedure */
typedef void (*Pen_CBProc )(
    Widget,
    const Pen*,         /* pen value                       */
    void *              /* data the application registered */
);

typedef struct _FormatStructure FormatStructure;

/* FormatChoice input CB procedure */
typedef void (*Format_CBProc )(
    FormatStructure *,
    const Format*,
    void *              /* data the application registered */
);

struct _FormatStructure {
    OptionStructure *type;
    OptionStructure *prec;
    TextStructure   *fstring;

    Format_CBProc cb_proc;
    void *cb_data;
};

void Beep(void);

void InitWidgets(void);

Pixmap XpmToPixmap(char **xpm);

SpinStructure *CreateCharSizeChoice(Widget parent, char *s);

SpinStructure *CreateAngleChoice(Widget parent, char *s);
double GetAngleChoice(SpinStructure *sp);
void SetAngleChoice(SpinStructure *sp, double angle);

ListStructure *CreateListChoice(Widget parent, char *labelstr, int type,
                                int nvisible, int nchoices, LabelOptionItem *items);
int SelectListChoice(ListStructure *listp, int choice);
void SelectListChoices(ListStructure *listp, int nchoices, int *choices);
void UpdateListChoice(ListStructure *listp, int nchoices, LabelOptionItem *items);
int GetListChoices(ListStructure *listp, int **values);
int GetSingleListChoice(ListStructure *listp, int *value);
int GetListSelectedCount(ListStructure *listp);
void AddListChoiceCB(ListStructure *listp, List_CBProc cbproc, void *anydata);

StorageStructure *CreateStorageChoice(Widget parent,
    char *labelstr, int type, int nvisible);
void SetStorageChoiceLabeling(StorageStructure *ss, Storage_LabelingProc proc);
int GetStorageChoices(StorageStructure *ss, Quark ***values);
int GetSingleStorageChoice(StorageStructure *ss, Quark **value);
int SelectStorageChoice(StorageStructure *ss, Quark *choice);
int SelectStorageChoices(StorageStructure *ss, int nchoices, Quark **choices);
void UpdateStorageChoice(StorageStructure *ss);
void SetStorageChoiceQuark(StorageStructure *ss, Quark *q);
void AddStorageChoiceCB(StorageStructure *ss,
    Storage_CBProc cbproc, void *anydata);
void AddStorageChoiceDblClickCB(StorageStructure *ss,
    Storage_DCCBProc cbproc, void *anydata);

OptionStructure *CreateFontChoice(Widget parent, char *s);
OptionStructure *CreatePatternChoice(Widget parent, char *s);
OptionStructure *CreateLineStyleChoice(Widget parent, char *s);
OptionStructure *CreateSetTypeChoice(Widget parent, char *s);
OptionStructure *CreateColorChoice(Widget parent, char *s);
OptionStructure *CreateASChoice(Widget parent, char *s);
OptionStructure *CreateTextJustChoice(Widget parent, char *s);
OptionStructure *CreateJustChoice(Widget parent, char *s);
OptionStructure *CreateFrameTypeChoice(Widget parent, char *s);

Widget CreatePenChoice(Widget parent, char *s);
void SetPenChoice(Widget button, Pen *pen);
int GetPenChoice(Widget pen_button, Pen *pen);
void AddPenChoiceCB(Widget button, Pen_CBProc cbproc, void *anydata);

FormatStructure *CreateFormatChoice(Widget parent);
void SetFormatChoice(FormatStructure *fstr, const Format *format);
Format *GetFormatChoice(FormatStructure *fstr);
void AddFormatChoiceCB(FormatStructure *fstr, Format_CBProc cbproc, void *data);

SpinStructure *CreateViewCoordInput(Widget parent, char *s);

RestrictionStructure *CreateRestrictionChoice(Widget parent, char *s);

StorageStructure *CreateSSDChoice(Widget parent, char *labelstr, int type);

StorageStructure *CreateFrameChoice(Widget parent, char *labelstr, int type);

StorageStructure *CreateGraphChoice(Widget parent, char *labelstr, int type);

StorageStructure *CreateSetChoice(Widget parent, char *labelstr, 
                                        int type, StorageStructure *graphss);
void UpdateSetChoice(StorageStructure *ss);
Quark *get_set_choice_gr(StorageStructure *ss);

GraphSetStructure *CreateGraphSetSelector(Widget parent, char *s, int sel_type);
SSDSetStructure *CreateSSDSetSelector(Widget parent, char *s, int sel_type);
SrcDestStructure *CreateSrcDestSelector(Widget parent, int sel_type);

ListStructure *CreateColChoice(Widget parent, char *labelstr, int type);
void UpdateColChoice(ListStructure *sel, const Quark *ssd);

SSDColStructure *CreateSSDColSelector(Widget parent, char *s, int sel_type);
int GetSSDColChoices(SSDColStructure *sc, Quark **ssd, int **cols);

SpinStructure *CreateLineWidthChoice(Widget parent, char *s);

OptionStructure *CreatePanelChoice(Widget parent, char *labstr, ...);
OptionStructure *CreatePrecisionChoice(Widget parent, char *s);
OptionStructure *CreatePaperOrientationChoice(Widget parent, char *s);
OptionStructure *CreatePaperFormatChoice(Widget parent, char *s);

WidgetList CreateAACDialog(Widget form,
    Widget container, AACDialog_CBProc cbproc, void *data);

TransformStructure *CreateTransformDialogForm(Widget parent,
    const char *s, int sel_type, int exclusive, const TD_CBProcs *cbs);
int GetTransformDialogSettings(TransformStructure *tdialog,
    int *nssrc, Quark ***srcsets, Quark ***destsets);
void RaiseTransformationDialog(TransformStructure *tdialog);

void AddHelpCB(Widget w, char *ha);
void ContextHelpCB(Widget w, void *data);

void DefineDialogCursor(Cursor c);
void UndefineDialogCursor(void);

void AddWindowCloseCB(Widget w, Widget_CBProc cbproc, void *anydata);
void destroy_dialog_cb(Widget but, void *data);

void undo_cb(Widget but, void *data);
void redo_cb(Widget but, void *data);

void unlink_ssd_ui(Quark *q);

void set_title(char *title, char *icon_name);

/* Tree Widget */
typedef void TreeItem;

typedef struct {
    TreeItem **items;
    int count;
} TreeItemList;

typedef struct {
    Widget w;
    void *anydata;
    void *udata;
    int drop_action;
} TreeEvent;

typedef int (*Tree_CBProc)(TreeEvent *event);
typedef struct {
    Widget w;
    Tree_CBProc cbproc;
    void *anydata;
} Tree_CBData;

Widget CreateTree(Widget parent);
TreeItem *TreeInsertItem(Widget w, TreeItem *parent, Quark *q, int row);
void TreeDeleteItem(Widget w, TreeItem *item);
void TreeSetItemOpen(Widget w, TreeItem *item, int open);
void TreeSetItemText(Widget w, TreeItem *item, char *text);
void TreeSetItemPixmap(Widget w, TreeItem *item, Pixmap pixmap);
Quark *TreeGetQuark(TreeItem *item);
void TreeGetHighlighted(Widget w, TreeItemList *items);
void TreeHighlightItem(Widget w, TreeItem *item);
void TreeClearSelection(Widget w);
void TreeScrollToItem(Widget w, TreeItem *item);
void TreeRefresh(Widget w);

void AddTreeContextMenuCB(Widget w, Tree_CBProc cbproc, void *anydata);
void AddTreeHighlightItemsCB(Widget w, Tree_CBProc cbproc, void *anydata);
void AddTreeDropItemsCB(Widget w, Tree_CBProc cbproc, void *anydata);


/* Table Widget */
Widget CreateTable(char *name, Widget parent, int nrows, int ncols, int nrows_visible, int ncols_visible);
void TableSSDInit(Widget w);
void TableFontInit(Widget w);
void TableDataSetPropInit(Widget w);
void TableLevalInit(Widget w);
void TableOptionChoiceInit(Widget w);
int TableGetNrows(Widget w);
int TableGetNcols(Widget w);
void TableAddRows(Widget w, int nrows);
void TableDeleteRows(Widget w, int nrows);
void TableAddCols(Widget w, int ncols);
void TableDeleteCols(Widget w, int ncols);
void TableGetCellDimentions(Widget w, int *cwidth, int *cheight);
void TableSetColWidths(Widget w, int *widths);
void TableSetDefaultRowLabelWidth(Widget w, int width);
void TableSetDefaultRowLabelAlignment(Widget w, int align);
void TableSetDefaultColWidth(Widget w, int width);
void TableSetDefaultColAlignment(Widget w, int align);
void TableSetDefaultColLabelAlignment(Widget w, int align);
void TableSetColMaxlengths(Widget w, int *maxlengths);
void TableSetRowLabels(Widget w, char **labels);
void TableSetColLabels(Widget w, char **labels);
void TableSetFixedCols(Widget w, int nfixed_cols);
void TableUpdateVisibleRowsCols(Widget w);
void TableCommitEdit(Widget w, int close);
void TableSetCells(Widget w, char ***cells);
void TableSetCell(Widget w, int row, int col, char *value);
char *TableGetCell(Widget w, int row, int col);
void TableSelectCell(Widget w, int row, int col);
void TableDeselectCell(Widget w, int row, int col);
void TableSelectRow(Widget w, int row);
void TableDeselectRow(Widget w, int row);
void TableSelectCol(Widget w, int col);
void TableDeselectCol(Widget w, int col);
void TableDeselectAllCells(Widget w);
int TableIsRowSelected(Widget w, int row);
int TableIsColSelected(Widget w, int col);
void TableUpdate(Widget w);

typedef struct {
    int type;
    int button;
    int modifiers;

    Widget w;
    int row;
    int col;
    unsigned long background;
    unsigned long foreground;
    int value_type;
    char *value;
    Pixmap pixmap;
    int row_label;
    void *anydata;
    void *udata;
} TableEvent;

typedef int (*Table_CBProc)(TableEvent *event);
typedef struct {
    Widget w;
    Table_CBProc cbproc;
    void *anydata;
} Table_CBData;

void AddTableDrawCellCB(Widget w, Table_CBProc cbproc, void *anydata);
void AddTableEnterCellCB(Widget w, Table_CBProc cbproc, void *anydata);
void AddTableLeaveCellCB(Widget w, Table_CBProc cbproc, void *anydata);
void AddTableLabelActivateCB(Widget w, Table_CBProc cbproc, void *anydata);

/* ScrollBar */
void GetScrollBarValues(Widget w, int *value, int *maxvalue, int *slider_size, int *increment);
void SetScrollBarValue(Widget w, int value);
void SetScrollBarIncrement(Widget w, int increment);
Widget GetHorizontalScrollBar(Widget w);
Widget GetVerticalScrollBar(Widget w);

void AddMouseWheelSupport(Widget w);

#endif /* __MOTIFINC_H_ */
