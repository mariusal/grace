/*
 * Grace - Graphics for Exploratory Data Analysis
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-95 Paul J Turner, Portland, OR
 * Copyright (c) 1996-98 GRACE Development Team
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

#if (XmVersion < 1002)
#  define XmStringCreateLocalized XmStringCreateSimple
#endif

/* 
 * Accept/Apply/Close for aac_cb callbacks
 */
#define AAC_ACCEPT  0
#define AAC_APPLY   1
#define AAC_CLOSE   2

#define LIST_TYPE_SINGLE    0
#define LIST_TYPE_MULTIPLE  1

#define CreateMenuSeparator(w) CreateSeparator(w)

extern Widget app_shell;        /* defined in xmgrace.c */
extern XmStringCharSet charset; /* defined in xmgrace.c */

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
    Widget rc;
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

Widget CreateAACButtons(Widget parent, Widget form, XtCallbackProc aac_cb);

OptionStructure *CreateOptionChoice(Widget parent, char *labelstr, int ncols,
                                                int nchoices, OptionItem *items);
OptionStructure *CreateBitmapOptionChoice(Widget parent, char *labelstr, int ncols,
                int nchoices, int width, int height, BitmapOptionItem *items);
void SetOptionChoice(OptionStructure *opt, int value);
int GetOptionChoice(OptionStructure *opt);
void UpdateOptionChoice(OptionStructure *optp, int nchoices, OptionItem *items);

void AddOptionChoiceCB(OptionStructure *opt, XtCallbackProc cb);

ListStructure *CreateListChoice(Widget parent, char *labelstr, int type,
                                int nvisible, int nchoices, OptionItem *items);
int SelectListChoice(ListStructure *listp, int choice);
void SelectListChoices(ListStructure *listp, int nchoices, int *choices);
void UpdateListChoice(ListStructure *listp, int nchoices, OptionItem *items);
int GetListChoices(ListStructure *listp, int **values);
int GetSingleListChoice(ListStructure *listp, int *value);
void AddListChoiceCB(ListStructure *listp, XtCallbackProc cb);

void list_choice_selectall(Widget w, XEvent *e, String *par, Cardinal *npar);

OptionStructure *CreateFontChoice(Widget parent, char *s);
OptionStructure *CreatePatternChoice(Widget parent, char *s);
OptionStructure *CreateLineStyleChoice(Widget parent, char *s);
OptionStructure *CreateSetTypeChoice(Widget parent, char *s);
OptionStructure *CreateColorChoice(Widget parent, char *s);

ListStructure *CreateGraphChoice(Widget parent, char *labelstr, int type);

ListStructure *CreateSetChoice(Widget parent, char *labelstr, 
                                        int type, int standalone);
void UpdateSetChoice(ListStructure *listp, int gno);


SetChoiceItem CreateSetSelector(Widget parent, char *label, int type, int ff, int gtype, int stype);
int GetSelectedSet(SetChoiceItem l);
int GetSelectedSets(SetChoiceItem l, int **sets);
void update_set_list(int gno, SetChoiceItem l);
int save_set_list(SetChoiceItem l);
void update_save_set_list( SetChoiceItem l, int newgr );

int SetSelectedSet(int gno, int setno, SetChoiceItem l);


Widget *CreatePanelChoice(Widget parent, char *labstr, int nchoices, ...);
Widget *CreatePanelChoice0(Widget parent, char *labstr, int ncols, int nchoices, ...);
void SetChoice(Widget * w, int value);
int GetChoice(Widget * w);

Widget CreateTextItem2(Widget parent, int len, char *s);
Widget CreateTextItem4(Widget parent, int len, char *s);
Widget CreateScrollTextItem2(Widget parent, int len, int hgt, char *s);

Widget CreateCommandButtons(Widget parent, int n, Widget * buts, char **l);
Widget CreateCommandButtonsNoDefault(Widget parent, int n, Widget * buts, char **l);

Widget *CreateLineWidthChoice(Widget parent, char *s);
Widget *CreateFormatChoice(Widget parent, char *s);
Widget *CreatePrecisionChoice(Widget parent, char *s);

void SetLabel(Widget w, char *s);

Widget CreateMenuBar(Widget parent, char *name, char *help_anchor);
Widget CreateMenu(Widget parent, char *name, char *label, char mnemonic,
	Widget *cascade, char *help_anchor);
Widget CreateMenuButton(Widget parent, char *name, char *label, char mnemonic,
	XtCallbackProc cb, XtPointer data, char *help_anchor);
Widget CreateMenuToggle(Widget parent, char *name, char *label, char mnemonic,
	XtCallbackProc cb, XtPointer data, char *help_anchor);
Widget CreateMenuLabel(Widget parent, char *name);

void savewidget(Widget w);

#endif /* __MOTIFINC_H_ */
