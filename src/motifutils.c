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
 *
 * utilities for Motif
 *
 */

#include <config.h>
#include <cmath.h>

#include <stdlib.h>
#include <stdarg.h>

#include <X11/X.h>
#include <X11/Xatom.h>

#include <Xm/Xm.h>
#include <Xm/BulletinB.h>
#include <Xm/DialogS.h>
#include <Xm/Frame.h>
#include <Xm/Form.h>
#include <Xm/Scale.h>
#include <Xm/Label.h>
#include <Xm/List.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/Text.h>
#include <Xm/ToggleB.h>
#include <Xm/ArrowBG.h>
#include <Xm/Protocols.h>

#include "Tab.h"
#include "motifinc.h"

#include "defines.h"
#include "globals.h"
#include "draw.h"
#include "patterns.h"
#include "t1fonts.h"
#include "graphs.h"
#include "plotone.h"
#include "utils.h"
#include "events.h"
#include "protos.h"

extern double result;

/* lookup table to determine if character is a floating point digit 
 * only allowable char's [0-9.eE]
 */
unsigned char fpdigit[256] = {  
			      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
			      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
			      0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			      0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };


extern Display *disp;
extern Window root;
extern int depth;

extern XtAppContext app_con;

extern unsigned long xvlibcolors[];


static OptionItem *color_option_items = NULL;
static int ncolor_option_items = 0;
static OptionStructure **color_selectors = NULL;
static int ncolor_selectors = 0;

OptionStructure *CreateOptionChoice(Widget parent, char *labelstr, int ncols,
                                                int nchoices, OptionItem *items)
{
    XmString str;
    OptionStructure *retval;

    retval = malloc(sizeof(OptionStructure));

    retval->rc = XmCreateRowColumn(parent, "rcOption", NULL, 0);
    XtVaSetValues(retval->rc, XmNorientation, XmHORIZONTAL, NULL);

    retval->pulldown = XmCreatePulldownMenu(retval->rc, "pulldownMenu", NULL, 0);

    if (ncols > 0) {
        XtVaSetValues(retval->pulldown,
                      XmNorientation, XmVERTICAL,
                      XmNpacking, XmPACK_COLUMN,
                      XmNnumColumns, ncols,
                      NULL);
    }
    
    retval->nchoices = 0;
    retval->options = NULL;
    UpdateOptionChoice(retval, nchoices, items);

    retval->menu = XmCreateOptionMenu(retval->rc, "optionMenu", NULL, 0);
    str = XmStringCreate(labelstr, charset);
    XtVaSetValues(retval->menu,
		  XmNlabelString, str,
		  XmNsubMenuId, retval->pulldown,
                  NULL);
    XmStringFree(str);
    XtManageChild(retval->menu);

    XtManageChild(retval->rc);
    
    return retval;
}

void UpdateOptionChoice(OptionStructure *optp, int nchoices, OptionItem *items)
{
    int i, nold;
    XmString str;
    
    nold = optp->nchoices;

    for (i = nchoices; i < nold; i++) {
        XtDestroyWidget(optp->options[i].widget);
    }

    optp->options = xrealloc(optp->options, nchoices*sizeof(OptionWidgetItem));
    optp->nchoices = nchoices;

    for (i = nold; i < nchoices; i++) {
        optp->options[i].widget = 
                  XmCreatePushButton(optp->pulldown, "None", NULL, 0);
    }
    for (i = nold; i < nchoices; i++) {
        XtManageChild(optp->options[i].widget);
    }
    for (i = 0; i < nchoices; i++) {
	optp->options[i].value = items[i].value;
	if (items[i].label != NULL) {
            str = XmStringCreateSimple(items[i].label);
            XtVaSetValues(optp->options[i].widget, XmNlabelString, str, NULL);
            XmStringFree(str);
        }
    }
}

OptionStructure *CreateBitmapOptionChoice(Widget parent, char *labelstr, int ncols,
                int nchoices, int width, int height, BitmapOptionItem *items)
{
    int i;
    XmString str;
    OptionStructure *retval;
    Pixel fg, bg;
    Pixmap ptmp;

    retval = malloc(sizeof(OptionStructure));
    if (retval == NULL) {
        errmsg("Malloc error in CreateBitmapOptionChoice()");
    }
    retval->nchoices = nchoices;
    retval->options = malloc(nchoices*sizeof(OptionWidgetItem));
    if (retval->options == NULL) {
        errmsg("Malloc error in CreateBitmapOptionChoice()");
        cxfree(retval);
        return retval;
    }


    retval->rc = XmCreateRowColumn(parent, "rcOption", NULL, 0);
    XtVaSetValues(retval->rc, XmNorientation, XmHORIZONTAL, NULL);

    retval->pulldown = XmCreatePulldownMenu(retval->rc, "pulldownMenu", NULL, 0);
    XtVaSetValues(retval->pulldown, 
                  XmNentryAlignment, XmALIGNMENT_CENTER,
                  NULL);

    if (ncols > 0) {
        XtVaSetValues(retval->pulldown,
                      XmNorientation, XmVERTICAL,
                      XmNpacking, XmPACK_COLUMN,
                      XmNnumColumns, ncols,
                      NULL);
    }
    
    XtVaGetValues(retval->pulldown,
                  XmNforeground, &fg,
                  XmNbackground, &bg,
                  NULL);
    
    for (i = 0; i < nchoices; i++) {
	retval->options[i].value = items[i].value;
        if (items[i].bitmap != NULL) {
            ptmp = XCreatePixmapFromBitmapData(disp, root, 
                                    (char *) items[i].bitmap, width, height,
                                    fg, bg, depth);
            retval->options[i].widget = 
                XtVaCreateWidget("pixButton", xmPushButtonWidgetClass,
                                 retval->pulldown,
	                         XmNlabelType, XmPIXMAP,
	                         XmNlabelPixmap, ptmp,
	                         NULL);
        } else {
	    retval->options[i].widget = 
                  XmCreatePushButton(retval->pulldown, "None", NULL, 0);
        }
                                
    }
    for (i = 0; i < nchoices; i++) {
        XtManageChild(retval->options[i].widget);
    }

    retval->menu = XmCreateOptionMenu(retval->rc, "optionMenu", NULL, 0);
    str = XmStringCreate(labelstr, charset);
    XtVaSetValues(retval->menu,
		  XmNlabelString, str,
		  XmNsubMenuId, retval->pulldown,
		  NULL);
    XmStringFree(str);
    XtManageChild(retval->menu);

    XtManageChild(retval->rc);
    
    return retval;
}


void SetOptionChoice(OptionStructure *opt, int value)
{
    int i;
    Arg a;
    
    if (opt->options == NULL || opt->nchoices <= 0) {
        return;
    }
    
    for (i = 0; i < opt->nchoices; i++) {
        if (opt->options[i].value == value) {
            XtSetArg(a, XmNmenuHistory, opt->options[i].widget);
            XtSetValues(opt->menu, &a, 1);
            return;
        }
    }
}

int GetOptionChoice(OptionStructure *opt)
{
    Arg a;
    Widget warg;
    int i;

    if (opt->options == NULL || opt->nchoices <= 0) {
        errmsg("Internal error in GetOptionChoice()");
        return 0;
    }

    XtSetArg(a, XmNmenuHistory, &warg);
    XtGetValues(opt->menu, &a, 1);

    for (i = 0; i < opt->nchoices; i++) {
        if (opt->options[i].widget == warg) {
            return(opt->options[i].value);
        }
    }
    errmsg("Internal error in GetOptionChoice()");
    return 0;
}

void AddOptionChoiceCB(OptionStructure *opt, XtCallbackProc cb)
{
    int i;
    
    for (i = 0; i < opt->nchoices; i++) {
        XtAddCallback(opt->options[i].widget, XmNactivateCallback, 
                                    cb, (XtPointer) opt->options[i].value);
    }
}


static char list_translation_table[] = "\
    Ctrl<Key>A: list_selectall_action()\n\
    Ctrl<Key>U: list_unselectall_action()";

ListStructure *CreateListChoice(Widget parent, char *labelstr, int type,
                                int nvisible, int nchoices, OptionItem *items)
{
    Arg args[4];
    Widget lab;
    ListStructure *retval;

    retval = malloc(sizeof(ListStructure));
    retval->rc = XmCreateRowColumn(parent, "rcList", NULL, 0);

    lab = XmCreateLabel(retval->rc, labelstr, NULL, 0);
    XtManageChild(lab);
    
/*
 *     XtSetArg(args[0], XmNlistSizePolicy, XmRESIZE_IF_POSSIBLE);
 */
    XtSetArg(args[0], XmNlistSizePolicy, XmCONSTANT);
    XtSetArg(args[1], XmNscrollBarDisplayPolicy, XmSTATIC);
    if (type == LIST_TYPE_SINGLE) {
        XtSetArg(args[2], XmNselectionPolicy, XmSINGLE_SELECT);
    } else {
        XtSetArg(args[2], XmNselectionPolicy, XmEXTENDED_SELECT);
    }
    XtSetArg(args[3], XmNvisibleItemCount, nvisible);
    retval->list = XmCreateScrolledList(retval->rc, "listList", args, 4);
    retval->values = NULL;

    XtOverrideTranslations(retval->list, 
                             XtParseTranslationTable(list_translation_table));
    
    UpdateListChoice(retval, nchoices, items);

    XtManageChild(retval->list);
    
    XtManageChild(retval->rc);
    
    return retval;
}

void UpdateListChoice(ListStructure *listp, int nchoices, OptionItem *items)
{
    int i, nsel;
    int *selvalues;
    XmString str;
    
    if (listp == NULL) {
        return;
    }
    
    nsel = GetListChoices(listp, &selvalues);

    listp->nchoices = nchoices;
    listp->values = xrealloc(listp->values, nchoices*SIZEOF_INT);
    for (i = 0; i < nchoices; i++) {
        listp->values[i] = items[i].value;
    }
    
    XmListDeleteAllItems(listp->list);
    for (i = 0; i < nchoices; i++) {
	str = XmStringCreateSimple(items[i].label);
        XmListAddItemUnselected(listp->list, str, 0);
        XmStringFree(str);
    }
    SelectListChoices(listp, nsel, selvalues);
    if (nsel > 0) {
        free(selvalues);
    }
}

int SelectListChoice(ListStructure *listp, int choice)
{
    int top, visible;
    int i = 0;
    
    while (i < listp->nchoices && listp->values[i] != choice) {
        i++;
    }
    if (i < listp->nchoices) {
        i++;
        XmListDeselectAllItems(listp->list);
        XmListSelectPos(listp->list, i, True);
        XtVaGetValues(listp->list, XmNtopItemPosition, &top,
                                 XmNvisibleItemCount, &visible,
                                 NULL);
        if (i < top) {
            XmListSetPos(listp->list, i);
        } else if (i >= top + visible) {
            XmListSetBottomPos(listp->list, i);
        }
        
        return GRACE_EXIT_SUCCESS;
    } else {
        return GRACE_EXIT_FAILURE;
    }
}

void SelectListChoices(ListStructure *listp, int nchoices, int *choices)
{
    int i = 0, j;
    unsigned char selection_type_save;
    int bottom, visible;
    
    XtVaGetValues(listp->list, XmNselectionPolicy, &selection_type_save, NULL);
    XtVaSetValues(listp->list, XmNselectionPolicy, XmMULTIPLE_SELECT, NULL);
                             
    XmListDeselectAllItems(listp->list);
    for (j = 0; j < nchoices; j++) {
        i = 0;
        while (i < listp->nchoices && listp->values[i] != choices[j]) {
            i++;
        }
        if (i < listp->nchoices) {
            i++;
            XmListSelectPos(listp->list, i, True);
        }
    }
    
    /* Rewind list so the last choice is always visible */
    XtVaGetValues(listp->list, XmNtopItemPosition, &bottom,
                             XmNvisibleItemCount, &visible,
                             NULL);
    if (i > bottom) {
        XmListSetBottomPos(listp->list, i);
    } else if (i <= bottom - visible) {
        XmListSetPos(listp->list, i);
    }

    XtVaSetValues(listp->list, XmNselectionPolicy, selection_type_save, NULL);
}

int GetListChoices(ListStructure *listp, int **values)
{
    int i, n;
    
    if (XmListGetSelectedPos(listp->list, values, &n) != True) {
        return 0;
    }
    
    for (i = 0; i < n; i++) {
        (*values)[i] = listp->values[(*values)[i] - 1];
    }
    
    return n;
}

int GetSingleListChoice(ListStructure *listp, int *value)
{
    int n, *values, retval;
 
    n = GetListChoices(listp, &values);
    if (n == 1) {
        *value = values[0];
        retval = GRACE_EXIT_SUCCESS;
    } else {
        retval = GRACE_EXIT_FAILURE;
    }
    if (n > 0) {
        free(values);
    }
    return retval;
}

void AddListChoiceCB(ListStructure *listp, XtCallbackProc cb)
{
    XtAddCallback(listp->list, XmNsingleSelectionCallback, cb, listp);
    XtAddCallback(listp->list, XmNmultipleSelectionCallback, cb, listp);
    XtAddCallback(listp->list, XmNextendedSelectionCallback, cb, listp);
}


static void spin_arrow_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
    SpinStructure *spinp;
    double value, incr;
    
    spinp = (SpinStructure *) client_data;
    value = GetSpinChoice(spinp);
    incr = spinp->incr;
    
    if (w == spinp->arrow_up) {
        incr =  spinp->incr;
    } else if (w == spinp->arrow_down) {
        incr = -spinp->incr;
    } else {
        errmsg("Wrong call to spin_arrow_cb()");
        return;
    }
    value += incr;
    SetSpinChoice(spinp, value);
}

SpinStructure *CreateSpinChoice(Widget parent, char *s, int len,
                        int type, double min, double max, double incr)
{
    SpinStructure *retval;
    Widget fr, form;
    XmString str;
    
    if (min >= max) {
        errmsg("min >= max in CreateSpinChoice()!");
        return NULL;
    }
    
    retval = malloc(sizeof(SpinStructure));
    
    retval->type = type;
    retval->min = min;
    retval->max = max;
    retval->incr = incr;
    
    retval->rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, parent,
        XmNorientation, XmHORIZONTAL,
        NULL);
    str = XmStringCreateSimple(s);
    XtVaCreateManagedWidget("label", xmLabelWidgetClass, retval->rc,
	XmNlabelString, str,
	NULL);
    XmStringFree(str);
    fr = XtVaCreateWidget("fr", xmFrameWidgetClass, retval->rc,
        XmNshadowType, XmSHADOW_ETCHED_OUT,
        NULL);
    form = XtVaCreateWidget("form", xmFormWidgetClass, fr,
        NULL);
    retval->text = XtVaCreateWidget("text", xmTextWidgetClass, form,
	XmNtraversalOn, True,
	XmNcolumns, len,
	NULL);
    retval->arrow_up = XtVaCreateWidget("form", xmArrowButtonGadgetClass, form,
        XmNarrowDirection, XmARROW_UP,
        NULL);
    XtAddCallback(retval->arrow_up, XmNactivateCallback,
        spin_arrow_cb, (XtPointer) retval);
    retval->arrow_down = XtVaCreateWidget("form", xmArrowButtonGadgetClass, form,
        XmNarrowDirection, XmARROW_DOWN,
        NULL);
    XtAddCallback(retval->arrow_down, XmNactivateCallback,
        spin_arrow_cb, (XtPointer) retval);
    XtVaSetValues(retval->text,
        XmNtopAttachment, XmATTACH_FORM,
        XmNleftAttachment, XmATTACH_FORM,
        XmNbottomAttachment, XmATTACH_FORM,
        XmNrightAttachment, XmATTACH_NONE,
        NULL);
    XtVaSetValues(retval->arrow_down,
        XmNtopAttachment, XmATTACH_FORM,
        XmNbottomAttachment, XmATTACH_FORM,
        XmNleftAttachment, XmATTACH_WIDGET,
        XmNleftWidget, retval->text,
        XmNrightAttachment, XmATTACH_NONE,
        NULL);
    XtVaSetValues(retval->arrow_up,
        XmNtopAttachment, XmATTACH_FORM,
        XmNbottomAttachment, XmATTACH_FORM,
        XmNrightAttachment, XmATTACH_FORM,
        XmNleftAttachment, XmATTACH_WIDGET,
        XmNleftWidget, retval->arrow_down,
        NULL);
    
    XtManageChild(retval->text);
    XtManageChild(retval->arrow_up);
    XtManageChild(retval->arrow_down);
    XtManageChild(form);
    XtManageChild(fr);
    XtManageChild(retval->rc);
    
    return retval;
}

void SetSpinChoice(SpinStructure *spinp, double value)
{
    char buf[64];
    
    if (value < spinp->min) {
        XBell(disp, 50);
        value = spinp->min;
    } else if (value > spinp->max) {
        XBell(disp, 50);
        value = spinp->max;
    }
    
    if (spinp->type == SPIN_TYPE_FLOAT) {
        sprintf(buf, "%g", value);
    } else {
        sprintf(buf, "%d", (int) rint(value));
    }
    XmTextSetString(spinp->text, buf);
}

double GetSpinChoice(SpinStructure *spinp)
{
    double retval;
    
    xv_evalexpr(spinp->text, &retval);
    if (retval < spinp->min) {
        errmsg("Input value below min limit in GetSpinChoice()");
        retval = spinp->min;
        SetSpinChoice(spinp, retval);
    } else if (retval > spinp->max) {
        errmsg("Input value above max limit in GetSpinChoice()");
        retval = spinp->max;
        SetSpinChoice(spinp, retval);
    }
    
    if (spinp->type == SPIN_TYPE_INT) {
        return rint(retval);
    } else {
        return retval;
    }
}


void cstext_edit_action(Widget w, XEvent *e, String *par, Cardinal *npar)
{
    create_fonttool(w);
}

static char cstext_translation_table[] = "\
    Ctrl<Key>E: cstext_edit_action()";

CSTextStructure *CreateCSText(Widget parent, char *s)
{
    CSTextStructure *retval;
    XmString str;
    
    retval = malloc(sizeof(CSTextStructure));
    retval->form = XtVaCreateWidget("form", xmFormWidgetClass, parent, NULL, 0);

    str = XmStringCreateSimple(s);
    retval->label = XtVaCreateManagedWidget("label", 
        xmLabelWidgetClass, retval->form,
        XmNlabelString, str,
        XmNtopAttachment, XmATTACH_FORM,
        XmNbottomAttachment, XmATTACH_FORM,
        XmNleftAttachment, XmATTACH_FORM,
        XmNrightAttachment, XmATTACH_NONE,
        NULL);
    XmStringFree(str);

    retval->cstext = XtVaCreateManagedWidget("cstext",
        xmTextWidgetClass, retval->form,
        XmNtraversalOn, True,
        XmNtopAttachment, XmATTACH_FORM,
        XmNbottomAttachment, XmATTACH_FORM,
        XmNleftAttachment, XmATTACH_WIDGET,
        XmNleftWidget, retval->label,
        XmNrightAttachment, XmATTACH_FORM,
        NULL);

    XtOverrideTranslations(retval->cstext, 
        XtParseTranslationTable(cstext_translation_table));

    XtManageChild(retval->form);

    return retval;
}

char *GetCSTextString(CSTextStructure *cst)
{
    static char *buf = NULL;
    char *s;
    
    s = XmTextGetString(cst->cstext);
    buf = copy_string(buf, s);
    XtFree(s);
    
    return buf;
}

void SetCSTextString(CSTextStructure *cst, char *s)
{
    XmTextSetString(cst->cstext, s);
}

int GetCSTextCursorPos(CSTextStructure *cst)
{
    return XmTextGetInsertionPosition(cst->cstext);
}

void CSTextInsert(CSTextStructure *cst, int pos, char *s)
{
    XmTextInsert(cst->cstext, pos, s);
}

static OptionItem *font_option_items;
static OptionItem *settype_option_items;
static BitmapOptionItem *pattern_option_items;
static BitmapOptionItem *lines_option_items;

#define LINES_BM_HEIGHT 15
#define LINES_BM_WIDTH  64

int init_option_menus(void) {
    int i, j, k, l, n;
    
    n = number_of_fonts();
    font_option_items = malloc(n*sizeof(OptionItem));
    if (font_option_items == NULL) {
        errmsg("Malloc error in init_option_menus()");
        return GRACE_EXIT_FAILURE;
    }
    for (i = 0; i < n; i++) {
        font_option_items[i].value = i;
        font_option_items[i].label = get_fontalias(i);
    }
    
    n = number_of_patterns();
    pattern_option_items = malloc(n*sizeof(BitmapOptionItem));
    if (pattern_option_items == NULL) {
        errmsg("Malloc error in init_option_menus()");
        free(font_option_items);
        return GRACE_EXIT_FAILURE;
    }
    for (i = 0; i < n; i++) {
        pattern_option_items[i].value = i;
        if (i == 0) {
            pattern_option_items[i].bitmap = NULL;
        } else {
            pattern_option_items[i].bitmap = pat_bits[i];
        }
    }
    
    n = number_of_linestyles();
    lines_option_items = malloc(n*sizeof(BitmapOptionItem));
    if (lines_option_items == NULL) {
        errmsg("Malloc error in init_option_menus()");
        free(pattern_option_items);
        free(font_option_items);
        return GRACE_EXIT_FAILURE;
    }
    for (i = 0; i < n; i++) {
        lines_option_items[i].value = i;
        if (i == 0) {
            lines_option_items[i].bitmap = NULL;
            continue;
        }
        
        lines_option_items[i].bitmap = 
              calloc(LINES_BM_HEIGHT*LINES_BM_WIDTH/8/SIZEOF_CHAR, SIZEOF_CHAR);
        
        k = LINES_BM_WIDTH*(LINES_BM_HEIGHT/2);
        while (k < LINES_BM_WIDTH*(LINES_BM_HEIGHT/2 + 1)) {
            for (j = 0; j < dash_array_length[i]; j++) {
                for (l = 0; l < dash_array[i][j]; l++) {
                    if (k < LINES_BM_WIDTH*(LINES_BM_HEIGHT/2 + 1)) {
                        if (j % 2 == 0) { 
                            /* black */
                            lines_option_items[i].bitmap[k/8] |= 1 << k % 8;
                        }
                        k++;
                    }
                }
            }
        }
    }

    settype_option_items = malloc(NUMBER_OF_SETTYPES*sizeof(OptionItem));
    if (settype_option_items == NULL) {
        errmsg("Malloc error in init_option_menus()");
        return GRACE_EXIT_FAILURE;
    }
    for (i = 0; i < NUMBER_OF_SETTYPES; i++) {
        settype_option_items[i].value = i;
        settype_option_items[i].label = copy_string(NULL, set_types(i));
        lowtoupper(settype_option_items[i].label);
    }

    return GRACE_EXIT_SUCCESS;
}

OptionStructure *CreateFontChoice(Widget parent, char *s)
{
    return (CreateOptionChoice(parent,
        s, 0, number_of_fonts(), font_option_items));
}

OptionStructure *CreatePatternChoice(Widget parent, char *s)
{
    return (CreateBitmapOptionChoice(parent, s, 4, number_of_patterns(), 
                                     16, 16, pattern_option_items));
}

OptionStructure *CreateLineStyleChoice(Widget parent, char *s)
{
    return (CreateBitmapOptionChoice(parent, s, 0, number_of_linestyles(), 
                        LINES_BM_WIDTH, LINES_BM_HEIGHT, lines_option_items));
}

OptionStructure *CreateSetTypeChoice(Widget parent, char *s)
{
    return (CreateOptionChoice(parent,
        s, 0, NUMBER_OF_SETTYPES, settype_option_items));
}


static OptionItem *graph_select_items = NULL;
static int ngraph_select_items = 0;
static ListStructure **graph_selectors = NULL;
static int ngraph_selectors = 0;

void graph_select_cb(Widget list, XtPointer client_data, XmListCallbackStruct *cbs)
{
    ListStructure *plist;
    int gno;
    
    plist = (ListStructure *) client_data;
    gno = plist->values[cbs->item_position - 1];
    switch_current_graph(gno);
}

void update_graph_selectors(void)
{
    int i, new_n = number_of_graphs();
    char buf[64];
    OptionItem *p;
    
    for (i = 0; i < ngraph_select_items; i++) {
        free(graph_select_items[i].label);
    }
    p = xrealloc(graph_select_items, new_n*sizeof(OptionItem));
    if (p != NULL) {
        graph_select_items = p;
    } else {
        ngraph_select_items = 0;
        return;
    }
    for (i = 0; i < new_n; i++) {
        graph_select_items[i].value = i;
        sprintf(buf, "G%d (%s, %d sets)", i, 
                                          is_graph_hidden(i) ? "hidden":"shown",
                                          number_of_sets(i));
        graph_select_items[i].label = copy_string(NULL, buf);
    }
    ngraph_select_items = new_n;
    
    for (i = 0; i < ngraph_selectors; i++) {
        UpdateListChoice(graph_selectors[i],
            ngraph_select_items, graph_select_items);
    }
}

typedef struct {
    Widget popup;
    Widget label_item;
    Widget focus_item;
    Widget hide_item;
    Widget show_item;
    Widget duplicate_item;
    Widget kill_item;
    Widget copy12_item;
    Widget copy21_item;
    Widget move12_item;
    Widget move21_item;
    Widget swap_item;
} GraphPopupMenu;

typedef enum {
    GraphMenuFocusCB,
    GraphMenuHideCB,
    GraphMenuShowCB,
    GraphMenuDuplicateCB,
    GraphMenuKillCB,
    GraphMenuCopy12CB,
    GraphMenuCopy21CB,
    GraphMenuMove12CB,
    GraphMenuMove21CB,
    GraphMenuSwapCB,
    GraphMenuNewCB
} GraphMenuCBtype;

void graph_menu_cb(ListStructure *listp, GraphMenuCBtype type)
{
    int err = FALSE;
    int i, n, *values;
    char buf[32];

    n = GetListChoices(listp, &values);
    
    switch (type) {
    case GraphMenuFocusCB:
        if (n == 1) {
            switch_current_graph(values[0]);
        } else {
            err = TRUE;
        }
        break;
    case GraphMenuHideCB:
        if (n > 0) {
            for (i = 0; i < n; i++) {
                set_graph_hidden(values[i], TRUE);
            }
        } else {
            err = TRUE;
        }
        break;
    case GraphMenuShowCB:
        if (n > 0) {
            for (i = 0; i < n; i++) {
                set_graph_hidden(values[i], FALSE);
            }
        } else {
            err = TRUE;
        }
        break;
    case GraphMenuDuplicateCB:
        if (n > 0) {
            for (i = 0; i < n; i++) {
                duplicate_graph(values[i]);
            }
        } else {
            err = TRUE;
        }
        break;
    case GraphMenuKillCB:
        if (n > 0) {
            if (yesno("Kill selected graph(s)?", NULL, NULL, NULL)) {
                for (i = 0; i < n; i++) {
                    kill_graph(values[i]);
                }
            }
        } else {
            err = TRUE;
        }
        break;
    case GraphMenuCopy12CB:
        if (n == 2) {
            sprintf(buf, "Overwrite G%d?", values[1]);
            if (yesno(buf, NULL, NULL, NULL)) {
                copy_graph(values[0], values[1]);
            }
        } else {
            err = TRUE;
        }
        break;
    case GraphMenuCopy21CB:
        if (n == 2) {
            sprintf(buf, "Overwrite G%d?", values[0]);
            if (yesno(buf, NULL, NULL, NULL)) {
                copy_graph(values[1], values[0]);
            }
        } else {
            err = TRUE;
        }
        break;
    case GraphMenuMove12CB:
        if (n == 2) {
            sprintf(buf, "Replace G%d?", values[1]);
            if (yesno(buf, NULL, NULL, NULL)) {
                move_graph(values[0], values[1]);
            }
        } else {
            err = TRUE;
        }
        break;
    case GraphMenuMove21CB:
        if (n == 2) {
            sprintf(buf, "Replace G%d?", values[0]);
            if (yesno(buf, NULL, NULL, NULL)) {
                move_graph(values[1], values[0]);
            }
        } else {
            err = TRUE;
        }
        break;
    case GraphMenuSwapCB:
        if (n == 2) {
            swap_graph(values[0], values[1]);
        } else {
            err = TRUE;
        }
        break;
    case GraphMenuNewCB:
        set_graph_active(number_of_graphs(), TRUE);
        break;
    default:
        err = TRUE;
        break;
    }

    if (n > 0) {
        free(values);
    }

    if (err == TRUE) {
        errmsg("Internal error in a graph selector callback");
    } else {
        update_all();
        drawgraph();
    }
}

void switch_focus_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    graph_menu_cb((ListStructure *) client_data, GraphMenuFocusCB);
}

void hide_graph_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    graph_menu_cb((ListStructure *) client_data, GraphMenuHideCB);
}

void show_graph_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    graph_menu_cb((ListStructure *) client_data, GraphMenuShowCB);
}

void duplicate_graph_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    graph_menu_cb((ListStructure *) client_data, GraphMenuDuplicateCB);
}

void kill_graph_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    graph_menu_cb((ListStructure *) client_data, GraphMenuKillCB);
}

void copy12_graph_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    graph_menu_cb((ListStructure *) client_data, GraphMenuCopy12CB);
}

void copy21_graph_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    graph_menu_cb((ListStructure *) client_data, GraphMenuCopy21CB);
}

void move12_graph_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    graph_menu_cb((ListStructure *) client_data, GraphMenuMove12CB);
}

void move21_graph_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    graph_menu_cb((ListStructure *) client_data, GraphMenuMove21CB);
}

void swap_graph_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    graph_menu_cb((ListStructure *) client_data, GraphMenuSwapCB);
}

void create_new_graph_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    graph_menu_cb((ListStructure *) client_data, GraphMenuNewCB);
}

GraphPopupMenu *CreateGraphPopupEntries(ListStructure *listp)
{
    GraphPopupMenu *graph_popup_menu;
    Widget popup;
    
    graph_popup_menu = malloc(sizeof(GraphPopupMenu));

    popup = XmCreatePopupMenu(listp->list, "graphPopupMenu", NULL, 0);
    graph_popup_menu->popup = popup;
    
    graph_popup_menu->label_item = CreateMenuLabel(popup, "Selection:");
    CreateMenuSeparator(popup);
    graph_popup_menu->focus_item = CreateMenuButton(popup, "switchFocus", "Focus to", 'F',
    	switch_focus_proc, (XtPointer) listp, 0);
    CreateMenuSeparator(popup);
    graph_popup_menu->hide_item = CreateMenuButton(popup, "hide", "Hide", 'H',
    	hide_graph_proc, (XtPointer) listp, 0);
    graph_popup_menu->show_item = CreateMenuButton(popup, "show", "Show", 'S',
    	show_graph_proc, (XtPointer) listp, 0);
    graph_popup_menu->duplicate_item = CreateMenuButton(popup, "duplicate", "Duplicate", 'D',
    	duplicate_graph_proc, (XtPointer) listp, 0);
    graph_popup_menu->kill_item = CreateMenuButton(popup, "kill", "Kill", 'K',
    	kill_graph_proc, (XtPointer) listp, 0);
    CreateMenuSeparator(popup);
    graph_popup_menu->copy12_item = CreateMenuButton(popup, "copy12", "Copy 1 to 2", 'C',
    	copy12_graph_proc, (XtPointer) listp, 0);
    graph_popup_menu->copy21_item = CreateMenuButton(popup, "copy21", "Copy 2 to 1", 'C',
    	copy21_graph_proc, (XtPointer) listp, 0);
    graph_popup_menu->move12_item = CreateMenuButton(popup, "move12", "Move 1 to 2", 'M',
    	move12_graph_proc, (XtPointer) listp, 0);
    graph_popup_menu->move21_item = CreateMenuButton(popup, "move21", "Move 2 to 1", 'M',
    	move21_graph_proc, (XtPointer) listp, 0);
    graph_popup_menu->swap_item = CreateMenuButton(popup, "swap", "Swap", 'w',
    	swap_graph_proc, (XtPointer) listp, 0);
    CreateMenuSeparator(popup);
    CreateMenuButton(popup, "createNew", "Create new", 'C',
    	create_new_graph_proc, (XtPointer) listp, 0);

    return graph_popup_menu;
}

void graph_popup(Widget parent, ListStructure *listp, XButtonPressedEvent *event)
{
    int i, n;
    int *values;
    char buf[64];
    Widget popup;
    GraphPopupMenu* graph_popup_menu;
    
    if (event->button != 3) {
        return;
    }
    
    graph_popup_menu = (GraphPopupMenu*) listp->anydata;
    popup = graph_popup_menu->popup;
    
    n = GetListChoices(listp, &values);
    if (n > 0) {
        sprintf(buf, "G%d", values[0]);
        for (i = 1; i < n; i++) {
            if (strlen(buf) > 30) {
                strcat(buf, "...");
                break;
            }
            sprintf(buf, "%s, G%d", buf, values[i]);
        }
    } else {
        strcpy(buf, "None"); 
    }
    
    SetLabel(graph_popup_menu->label_item, buf);
    
    if (n == 0) {
        XtSetSensitive(graph_popup_menu->hide_item, False);
        XtSetSensitive(graph_popup_menu->show_item, False);
        XtSetSensitive(graph_popup_menu->duplicate_item, False);
        XtSetSensitive(graph_popup_menu->kill_item, False);
    } else {
        XtSetSensitive(graph_popup_menu->hide_item, True);
        XtSetSensitive(graph_popup_menu->show_item, True);
        XtSetSensitive(graph_popup_menu->duplicate_item, True);
        XtSetSensitive(graph_popup_menu->kill_item, True);
    }
    if (n == 1) {
        XtSetSensitive(graph_popup_menu->focus_item, True);
    } else {
        XtSetSensitive(graph_popup_menu->focus_item, False);
    }
    if (n == 2) {
        sprintf(buf, "Copy G%d to G%d", values[0], values[1]);
        SetLabel(graph_popup_menu->copy12_item, buf);
        XtManageChild(graph_popup_menu->copy12_item);
        sprintf(buf, "Copy G%d to G%d", values[1], values[0]);
        SetLabel(graph_popup_menu->copy21_item, buf);
        XtManageChild(graph_popup_menu->copy21_item);
        sprintf(buf, "Move G%d to G%d", values[0], values[1]);
        SetLabel(graph_popup_menu->move12_item, buf);
        XtManageChild(graph_popup_menu->move12_item);
        sprintf(buf, "Move G%d to G%d", values[1], values[0]);
        SetLabel(graph_popup_menu->move21_item, buf);
        XtManageChild(graph_popup_menu->move21_item);
        XtSetSensitive(graph_popup_menu->swap_item, True);
    } else {
        XtUnmanageChild(graph_popup_menu->copy12_item);
        XtUnmanageChild(graph_popup_menu->copy21_item);
        XtUnmanageChild(graph_popup_menu->move12_item);
        XtUnmanageChild(graph_popup_menu->move21_item);
        XtSetSensitive(graph_popup_menu->swap_item, False);
    }
    
    if (n > 0) {
        free(values);
    }
    XmMenuPosition(popup, event);
    XtManageChild(popup);
}

void list_selectall(Widget list)
{
    int i, n;
    unsigned char selection_type_save;
    
    XtVaGetValues(list,
                  XmNselectionPolicy, &selection_type_save,
                  XmNitemCount, &n,
                  NULL);
    if (selection_type_save == XmSINGLE_SELECT) {
        XBell(disp, 50);
        return;
    }
    
    XtVaSetValues(list, XmNselectionPolicy, XmMULTIPLE_SELECT, NULL);
                             
    XmListDeselectAllItems(list);
    for (i = 1; i <= n; i++) {
        XmListSelectPos(list, i, False);
    }
    
    XtVaSetValues(list, XmNselectionPolicy, selection_type_save, NULL);
}

void list_selectall_action(Widget w, XEvent *e, String *par, Cardinal *npar)
{
    list_selectall(w);
}

void list_selectall_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
    ListStructure *listp = (ListStructure *) client_data;
    list_selectall(listp->list);
}

void list_unselectall(Widget list)
{
    XmListDeselectAllItems(list);
}

void list_unselectall_action(Widget w, XEvent *e, String *par, Cardinal *npar)
{
    list_unselectall(w);
}

void list_unselectall_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
    ListStructure *listp = (ListStructure *) client_data;
    list_unselectall(listp->list);
}

void set_graph_selectors(int gno)
{
    int i;
    
    for (i = 0; i < ngraph_selectors; i++) {
        SelectListChoice(graph_selectors[i], gno);
    }
}

ListStructure *CreateGraphChoice(Widget parent, char *labelstr, int type)
{
    ListStructure *retvalp;
        
    ngraph_selectors++;
    graph_selectors = xrealloc(graph_selectors, 
                                    ngraph_selectors*sizeof(ListStructure *));

    retvalp = CreateListChoice(parent, labelstr, type, 4,
                               ngraph_select_items, graph_select_items);
    graph_selectors[ngraph_selectors - 1] = retvalp;
    
    XtAddCallback(retvalp->list, XmNdefaultActionCallback,
                               (XtCallbackProc) graph_select_cb, retvalp);
    retvalp->anydata = CreateGraphPopupEntries(retvalp);
    
    XtAddEventHandler(retvalp->list, ButtonPressMask, False, 
                            (XtEventHandler) graph_popup, retvalp);

    if (ngraph_select_items == 0) {
        update_graph_selectors();
    } else {
        UpdateListChoice(retvalp, ngraph_select_items, graph_select_items);
    }
    
    SelectListChoice(retvalp, get_cg());
    
    return retvalp;
}

/* Set selectors */
static ListStructure **set_selectors = NULL;
static int nset_selectors = 0;

void UpdateSetChoice(ListStructure *listp, int gno)
{
    int i, j, n = number_of_sets(gno);
    char buf[64];
    OptionItem *set_select_items;
    SetChoiceData *sdata;
    
    sdata = (SetChoiceData *) listp->anydata;
    sdata->gno = gno;
    
    if (n <= 0) {
        UpdateListChoice(listp, 0, NULL);
        return;
    }
    
    set_select_items = malloc(n*sizeof(OptionItem));
    if (set_select_items == NULL) {
        return;
    }
    
    for (i = 0, j = 0; i < n; i++) {
        if ((sdata->show_nodata == TRUE || is_set_active(gno, i) == TRUE) &&
            (sdata->show_hidden == TRUE || is_set_hidden(gno, i) != TRUE )) {
            set_select_items[j].value = i;
            sprintf(buf, "G%d.S%d[%d] (%d cols, %s)", gno, i,
                getsetlength(gno, i), dataset_cols(gno, i),
                is_set_hidden(gno, i) ? "hidden":"shown");
            set_select_items[j].label = copy_string(NULL, buf);
            j++;
        }
    }
    UpdateListChoice(listp, j, set_select_items);
    
    free(set_select_items);
}

void update_set_selectors(int gno)
{
    int i, cg;
    SetChoiceData *sdata;
    
    cg = get_cg();
    for (i = 0; i < nset_selectors; i++) {
        sdata = (SetChoiceData *) set_selectors[i]->anydata;
        if (sdata->standalone == TRUE && (gno == cg || gno == ALL_GRAPHS)) {
            UpdateSetChoice(set_selectors[i], cg);
        } else if (sdata->standalone == FALSE && sdata->gno == gno) {
            UpdateSetChoice(set_selectors[i], gno);
        }
    }
}

void set_menu_cb(ListStructure *listp, SetMenuCBtype type)
{
    SetChoiceData *sdata;
    int err = FALSE;
    int gno;
    int i, n, setno, *values;
    char buf[32];

    n = GetListChoices(listp, &values);
    sdata = (SetChoiceData *) listp->anydata;
    gno = sdata->gno;
    
    switch (type) {
    case SetMenuHideCB:
        if (n > 0) {
            for (i = 0; i < n; i++) {
                set_set_hidden(gno, values[i], TRUE);
            }
        } else {
            err = TRUE;
        }
        break;
    case SetMenuShowCB:
        if (n > 0) {
            for (i = 0; i < n; i++) {
                set_set_hidden(gno, values[i], FALSE);
            }
        } else {
            err = TRUE;
        }
        break;
    case SetMenuBringfCB:
        if (n == 1) {
            pushset(gno, values[0], PUSH_SET_TOFRONT);
        } else {
            err = TRUE;
        }
        break;
    case SetMenuSendbCB:
        if (n == 1) {
            pushset(gno, values[0], PUSH_SET_TOBACK);
        } else {
            err = TRUE;
        }
        break;
    case SetMenuDuplicateCB:
        if (n > 0) {
            for (i = 0; i < n; i++) {
                setno = nextset(gno);
                do_copyset(gno, values[i], gno, setno);
            }
        } else {
            err = TRUE;
        }
        break;
    case SetMenuKillCB:
        if (n > 0) {
            if (yesno("Kill selected set(s)?", NULL, NULL, NULL)) {
                for (i = 0; i < n; i++) {
                    killset(gno, values[i]);
                }
            }
        } else {
            err = TRUE;
        }
        break;
    case SetMenuKillDCB:
        if (n > 0) {
            if (yesno("Kill data in selected set(s)?", NULL, NULL, NULL)) {
                for (i = 0; i < n; i++) {
                    killsetdata(gno, values[i]);
                }
            }
        } else {
            err = TRUE;
        }
        break;
    case SetMenuCopy12CB:
        if (n == 2) {
            sprintf(buf, "Overwrite S%d?", values[1]);
            if (yesno(buf, NULL, NULL, NULL)) {
                do_copyset(gno, values[0], gno, values[1]);
            }
        } else {
            err = TRUE;
        }
        break;
    case SetMenuCopy21CB:
        if (n == 2) {
            sprintf(buf, "Overwrite S%d?", values[0]);
            if (yesno(buf, NULL, NULL, NULL)) {
                do_copyset(gno, values[1], gno, values[0]);
            }
        } else {
            err = TRUE;
        }
        break;
    case SetMenuMove12CB:
        if (n == 2) {
            sprintf(buf, "Replace S%d?", values[1]);
            if (yesno(buf, NULL, NULL, NULL)) {
                moveset(gno, values[0], gno, values[1]);
            }
        } else {
            err = TRUE;
        }
        break;
    case SetMenuMove21CB:
        if (n == 2) {
            sprintf(buf, "Replace S%d?", values[0]);
            if (yesno(buf, NULL, NULL, NULL)) {
                moveset(gno, values[1], gno, values[0]);
            }
        } else {
            err = TRUE;
        }
        break;
    case SetMenuSwapCB:
        if (n == 2) {
            swapset(gno, values[0], gno, values[1]);
        } else {
            err = TRUE;
        }
        break;
    case SetMenuNewFCB:
            create_leval_frame(listp->list, (XtPointer) gno, NULL);
        break;
    case SetMenuNewSCB:
            if ((setno = nextset(gno)) != -1) {
                add_point(gno, setno, 0.0, 0.0);
                setcomment(gno, setno, "Editor");
                set_set_hidden(gno, setno, FALSE);
                create_ss_frame(gno, setno);
            } else {
                err = TRUE;
            }
        break;
    case SetMenuNewECB:
            if ((setno = nextset(gno)) != -1) {
                add_point(gno, setno, 0.0, 0.0);
                setcomment(gno, setno, "Editor");
                set_set_hidden(gno, setno, FALSE);
                do_ext_editor(gno, setno);
            } else {
                err = TRUE;
            }
        break;
    case SetMenuNewBCB:
            create_eblock_frame(gno);
        break;
    case SetMenuEditSCB:
        if (n == 1) {
            create_ss_frame(gno, values[0]);
        } else {
            err = TRUE;
        }
        break;
    case SetMenuEditECB:
        if (n == 1) {
            do_ext_editor(gno, values[0]);
        } else {
            err = TRUE;
        }
        break;
    default:
        err = TRUE;
        break;
    }

    if (n > 0) {
        free(values);
    }

    if (err == FALSE) {
        update_all();
        drawgraph();
    }
}


void hide_set_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    set_menu_cb((ListStructure *) client_data, SetMenuHideCB);
}

void show_set_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    set_menu_cb((ListStructure *) client_data, SetMenuShowCB);
}

void bringf_set_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    set_menu_cb((ListStructure *) client_data, SetMenuBringfCB);
}

void sendb_set_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    set_menu_cb((ListStructure *) client_data, SetMenuSendbCB);
}

void duplicate_set_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    set_menu_cb((ListStructure *) client_data, SetMenuDuplicateCB);
}

void kill_set_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    set_menu_cb((ListStructure *) client_data, SetMenuKillCB);
}

void killd_set_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    set_menu_cb((ListStructure *) client_data, SetMenuKillDCB);
}

void copy12_set_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    set_menu_cb((ListStructure *) client_data, SetMenuCopy12CB);
}

void copy21_set_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    set_menu_cb((ListStructure *) client_data, SetMenuCopy21CB);
}

void move12_set_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    set_menu_cb((ListStructure *) client_data, SetMenuMove12CB);
}

void move21_set_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    set_menu_cb((ListStructure *) client_data, SetMenuMove21CB);
}

void swap_set_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    set_menu_cb((ListStructure *) client_data, SetMenuSwapCB);
}

void newF_set_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    set_menu_cb((ListStructure *) client_data, SetMenuNewFCB);
}

void newS_set_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    set_menu_cb((ListStructure *) client_data, SetMenuNewSCB);
}

void newE_set_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    set_menu_cb((ListStructure *) client_data, SetMenuNewECB);
}

void newB_set_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    set_menu_cb((ListStructure *) client_data, SetMenuNewBCB);
}

void editS_set_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    set_menu_cb((ListStructure *) client_data, SetMenuEditSCB);
}

void editE_set_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    set_menu_cb((ListStructure *) client_data, SetMenuEditECB);
}

void update_set_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    ListStructure *listp = (ListStructure *) client_data;
    SetChoiceData *sdata = (SetChoiceData *) listp->anydata;
    
    if (w == sdata->menu->shownd_item) {
        sdata->show_nodata = GetToggleButtonState(w);
    }
    if (w == sdata->menu->showh_item) {
        sdata->show_hidden = GetToggleButtonState(w);
    }
    
    UpdateSetChoice(listp, sdata->gno);
}

SetPopupMenu *CreateSetPopupEntries(ListStructure *listp)
{
    SetPopupMenu *set_popup_menu;
    Widget popup, submenupane;
    
    set_popup_menu = malloc(sizeof(SetPopupMenu));
    popup = XmCreatePopupMenu(listp->list, "setPopupMenu", NULL, 0);
    set_popup_menu->popup = popup;
    
    set_popup_menu->label_item = CreateMenuLabel(popup, "Selection:");

    CreateMenuSeparator(popup);

    set_popup_menu->hide_item = CreateMenuButton(popup, "hide", "Hide", 'H',
    	hide_set_proc, (XtPointer) listp, 0);
    set_popup_menu->show_item = CreateMenuButton(popup, "show", "Show", 'S',
    	show_set_proc, (XtPointer) listp, 0);
    set_popup_menu->bringf_item = CreateMenuButton(popup, "bringToFront", "Bring to front", '\0',
    	bringf_set_proc, (XtPointer) listp, 0);
    set_popup_menu->sendb_item = CreateMenuButton(popup, "sendToBack", "Send to back", '\0',
    	sendb_set_proc, (XtPointer) listp, 0);
    CreateMenuSeparator(popup);
    set_popup_menu->duplicate_item = CreateMenuButton(popup, "duplicate", "Duplicate", 'D',
    	duplicate_set_proc, (XtPointer) listp, 0);
    set_popup_menu->kill_item = CreateMenuButton(popup, "kill", "Kill", 'K',
    	kill_set_proc, (XtPointer) listp, 0);
    set_popup_menu->killd_item = CreateMenuButton(popup, "killData", "Kill data", 'a',
    	killd_set_proc, (XtPointer) listp, 0);
    CreateMenuSeparator(popup);
    set_popup_menu->copy12_item = CreateMenuButton(popup, "copy12", "Copy 1 to 2", 'C',
    	copy12_set_proc, (XtPointer) listp, 0);
    set_popup_menu->copy21_item = CreateMenuButton(popup, "copy21", "Copy 2 to 1", 'C',
    	copy21_set_proc, (XtPointer) listp, 0);
    set_popup_menu->move12_item = CreateMenuButton(popup, "move12", "Move 1 to 2", 'M',
    	move12_set_proc, (XtPointer) listp, 0);
    set_popup_menu->move21_item = CreateMenuButton(popup, "move21", "Move 2 to 1", 'M',
    	move21_set_proc, (XtPointer) listp, 0);
    set_popup_menu->swap_item = CreateMenuButton(popup, "swap", "Swap", 'w',
    	swap_set_proc, (XtPointer) listp, 0);
    CreateMenuSeparator(popup);
    set_popup_menu->edit_item = CreateMenu(popup, "edit", "Edit", 'E', NULL, NULL);
    CreateMenuButton(set_popup_menu->edit_item, "inShpreadsheet", "In spreadsheet", 's',
    	editS_set_proc, (XtPointer) listp, 0);
    CreateMenuButton(set_popup_menu->edit_item, "inEditor", "In text editor", 'e',
    	editE_set_proc, (XtPointer) listp, 0);
    submenupane = CreateMenu(popup, "createNew", "Create new", 'n', NULL, NULL);
    CreateMenuButton(submenupane, "byFormula", "By formula", 'f',
    	newF_set_proc, (XtPointer) listp, 0);
    CreateMenuButton(submenupane, "inShpreadsheet", "In spreadsheet", 's',
    	newS_set_proc, (XtPointer) listp, 0);
    CreateMenuButton(submenupane, "inEditor", "In text editor", 'e',
    	newE_set_proc, (XtPointer) listp, 0);
    CreateMenuButton(submenupane, "fromBlockData", "From block data", 'b',
    	newB_set_proc, (XtPointer) listp, 0);

    CreateMenuSeparator(popup);

    submenupane = CreateMenu(popup, "selectorOperations", "Selector operations", 'o', NULL, NULL);
    set_popup_menu->shownd_item = CreateToggleButton(submenupane, "Show data-less");
    XtAddCallback(set_popup_menu->shownd_item, XmNvalueChangedCallback,
        (XtCallbackProc) update_set_proc, (XtPointer) listp);
    set_popup_menu->showh_item = CreateToggleButton(submenupane, "Show hidden");
    XtAddCallback(set_popup_menu->showh_item, XmNvalueChangedCallback,
        (XtCallbackProc) update_set_proc, (XtPointer) listp);
    CreateMenuSeparator(submenupane);
    CreateMenuButton(submenupane, "selectAll", "Select all", '\0',
    	list_selectall_cb, (XtPointer) listp, 0);
    CreateMenuButton(submenupane, "unSelectAll", "Unselect all", '\0',
    	list_unselectall_cb, (XtPointer) listp, 0);
    CreateMenuSeparator(submenupane);
    CreateMenuButton(submenupane, "update", "Update", '\0',
    	update_set_proc, (XtPointer) listp, 0);

    return set_popup_menu;
}

void set_popup(Widget parent, ListStructure *listp, XButtonPressedEvent *event)
{
    SetChoiceData *sdata;
    int i, n;
    int *values;
    char buf[64];
    Widget popup;
    SetPopupMenu* set_popup_menu;
    
    if (event->button != 3) {
        return;
    }
    
    sdata = (SetChoiceData *) listp->anydata;
    set_popup_menu = sdata->menu;
    popup = set_popup_menu->popup;
    
    n = GetListChoices(listp, &values);
    if (n > 0) {
        sprintf(buf, "S%d", values[0]);
        for (i = 1; i < n; i++) {
            if (strlen(buf) > 30) {
                strcat(buf, "...");
                break;
            }
            sprintf(buf, "%s, S%d", buf, values[i]);
        }
    } else {
        strcpy(buf, "None"); 
    }
    
    SetLabel(set_popup_menu->label_item, buf);
    
    SetToggleButtonState(set_popup_menu->shownd_item, sdata->show_nodata);
    SetToggleButtonState(set_popup_menu->showh_item, sdata->show_hidden);
    
    if (n == 0) {
        XtSetSensitive(set_popup_menu->hide_item, False);
        XtSetSensitive(set_popup_menu->show_item, False);
        XtSetSensitive(set_popup_menu->duplicate_item, False);
        XtSetSensitive(set_popup_menu->kill_item, False);
        XtSetSensitive(set_popup_menu->killd_item, False);
    } else {
        XtSetSensitive(set_popup_menu->hide_item, True);
        XtSetSensitive(set_popup_menu->show_item, True);
        XtSetSensitive(set_popup_menu->duplicate_item, True);
        XtSetSensitive(set_popup_menu->kill_item, True);
        XtSetSensitive(set_popup_menu->killd_item, True);
    }
    if (n == 1) {
        XtSetSensitive(set_popup_menu->bringf_item, True);
        XtSetSensitive(set_popup_menu->sendb_item, True);
        XtSetSensitive(set_popup_menu->edit_item, True);
    } else {
        XtSetSensitive(set_popup_menu->bringf_item, False);
        XtSetSensitive(set_popup_menu->sendb_item, False);
        XtSetSensitive(set_popup_menu->edit_item, False);
    }
    if (n == 2) {
        sprintf(buf, "Copy S%d to S%d", values[0], values[1]);
        SetLabel(set_popup_menu->copy12_item, buf);
        XtManageChild(set_popup_menu->copy12_item);
        sprintf(buf, "Copy S%d to S%d", values[1], values[0]);
        SetLabel(set_popup_menu->copy21_item, buf);
        XtManageChild(set_popup_menu->copy21_item);
        sprintf(buf, "Move S%d to S%d", values[0], values[1]);
        SetLabel(set_popup_menu->move12_item, buf);
        XtManageChild(set_popup_menu->move12_item);
        sprintf(buf, "Move S%d to S%d", values[1], values[0]);
        SetLabel(set_popup_menu->move21_item, buf);
        XtManageChild(set_popup_menu->move21_item);
        XtSetSensitive(set_popup_menu->swap_item, True);
    } else {
        XtUnmanageChild(set_popup_menu->copy12_item);
        XtUnmanageChild(set_popup_menu->copy21_item);
        XtUnmanageChild(set_popup_menu->move12_item);
        XtUnmanageChild(set_popup_menu->move21_item);
        XtSetSensitive(set_popup_menu->swap_item, False);
    }
    
    if (n > 0) {
        free(values);
    }
    XmMenuPosition(popup, event);
    XtManageChild(popup);
}

ListStructure *CreateSetChoice(Widget parent, char *labelstr, 
                                        int type, int standalone)
{
    ListStructure *retvalp;
    SetChoiceData *sdata;

    retvalp = CreateListChoice(parent, labelstr, type, 8, 0, NULL);
    if (retvalp == NULL) {
        return NULL;
    }

    sdata = malloc(sizeof(SetChoiceData));
    if (sdata == NULL) {
        cxfree(retvalp);
        return NULL;
    }
    
    sdata->standalone = standalone;
    sdata->show_hidden = TRUE;
    sdata->show_nodata = FALSE;
    sdata->menu = CreateSetPopupEntries(retvalp);
    XtAddEventHandler(retvalp->list, ButtonPressMask, False, 
                            (XtEventHandler) set_popup, retvalp);
    
    retvalp->anydata = sdata;
    
    if (standalone == TRUE) {
        UpdateSetChoice(retvalp, get_cg());
    }
    
    nset_selectors++;
    set_selectors = xrealloc(set_selectors, 
                                nset_selectors*sizeof(ListStructure *));
    set_selectors[nset_selectors - 1] = retvalp;
    
    return retvalp;
}



void paint_color_selector(OptionStructure *optp)
{
    int i, color;
    long bg, fg;
    
    for (i = 0; i < ncolor_option_items; i++) {
        color = color_option_items[i].value;
        bg = xvlibcolors[color];
	if ((get_colorintensity(color) < 0.5 && is_video_reversed() == FALSE) ||
            (get_colorintensity(color) > 0.5 && is_video_reversed() == TRUE )) {
	    fg = xvlibcolors[0];
	} else {
	    fg = xvlibcolors[1];
	}
	XtVaSetValues(optp->options[i].widget, 
            XmNbackground, bg,
            XmNforeground, fg,
            NULL);
    }
}

void update_color_selectors(void)
{
    int i, j;
    CMap_entry *pcmap;
    
    for (i = 0, j = 0; i < number_of_colors(); i++) {
        pcmap = get_cmap_entry(i);
        if (pcmap != NULL && pcmap->ctype == COLOR_MAIN) {
            j++;
        }
    }
    ncolor_option_items = j;

    color_option_items = xrealloc(color_option_items,
                                    ncolor_option_items*sizeof(OptionItem));
    for (i = 0, j = 0; i < number_of_colors(); i++) {
        pcmap = get_cmap_entry(i);
        if (pcmap != NULL && pcmap->ctype == COLOR_MAIN) {
            color_option_items[j].value = i;
            color_option_items[j].label = get_colorname(i);
            j++;
        }
    }
    
    for (i = 0; i < ncolor_selectors; i++) {
        UpdateOptionChoice(color_selectors[i], 
                            ncolor_option_items, color_option_items);
        paint_color_selector(color_selectors[i]);
    }
    
}

OptionStructure *CreateColorChoice(Widget parent, char *s)
{
    OptionStructure *retvalp = NULL;

    ncolor_selectors++;
    color_selectors = xrealloc(color_selectors, 
                                    ncolor_selectors*sizeof(OptionStructure *));
    if (color_selectors == NULL) {
        errmsg("Malloc failed in CreateColorChoice()");
        return retvalp;
    }
    
    retvalp = CreateOptionChoice(parent, s, 4, 
                                ncolor_option_items, color_option_items);

    color_selectors[ncolor_selectors - 1] = retvalp;
    
    UpdateOptionChoice(retvalp, ncolor_option_items, color_option_items);
    paint_color_selector(retvalp);
    
    return retvalp;
}

SpinStructure *CreateLineWidthChoice(Widget parent, char *s)
{
    return CreateSpinChoice(parent, s, 3, SPIN_TYPE_FLOAT, 0.0, MAX_LINEWIDTH, 0.5);
}




Widget *CreatePanelChoice0(Widget parent, char *labelstr, int ncols, int nchoices,...)
{
    va_list var;
    int i = 0;
    XmString str;
    char *s;
    Widget *retval;

    nchoices--;

    retval = (Widget *) XtMalloc((nchoices + 2) * sizeof(Widget));

    retval[1] = XmCreatePulldownMenu(parent, "pulldown", NULL, 0);
    
    if (ncols > 0) {
        XtVaSetValues(retval[1],
                      XmNorientation, XmVERTICAL,
                      XmNpacking, XmPACK_COLUMN,
                      XmNnumColumns, ncols,
                      NULL);
    }

    va_start(var, nchoices);
    i = 0;
    while ((s = va_arg(var, char *)) != NULL) {
	retval[i + 2] = XmCreatePushButton(retval[1], s, NULL, 0);
	i++;
    }
    if (i != nchoices) {
	errmsg("Incorrect number of selections in CreatePanelChoice0()");
    }
    va_end(var);

    XtManageChildren(retval + 2, nchoices);

    retval[0] = XmCreateOptionMenu(parent, "optionmenu", NULL, 0);
    str = XmStringCreate(labelstr, charset);
    XtVaSetValues(retval[0],
		  XmNlabelString, str,
		  XmNsubMenuId, retval[1],
		  NULL);
    XmStringFree(str);
    XtManageChild(retval[0]);

    return retval;
}

Widget *CreatePanelChoice(Widget parent, char *labelstr, int nchoices,...)
{
    va_list var;
    int i = 0;
    XmString str;
    char *s;
    Widget *retval;

    nchoices--;

    retval = (Widget *) XtMalloc((nchoices + 2) * sizeof(Widget));

    retval[1] = XmCreatePulldownMenu(parent, "pulldown", NULL, 0);
    
    va_start(var, nchoices);
    i = 0;
    while ((s = va_arg(var, char *)) != NULL) {
	retval[i + 2] = XmCreatePushButton(retval[1], s, NULL, 0);
	i++;
    }
    if (i != nchoices) {
	errmsg("Incorrect number of selections in CreatePanelChoice0()");
    }
    va_end(var);

    XtManageChildren(retval + 2, nchoices);

    retval[0] = XmCreateOptionMenu(parent, "optionmenu", NULL, 0);
    str = XmStringCreate(labelstr, charset);
    XtVaSetValues(retval[0],
		  XmNlabelString, str,
		  XmNsubMenuId, retval[1],
		  NULL);
    XmStringFree(str);
    XtManageChild(retval[0]);

    return retval;
}


void SetChoice(Widget * w, int value)
{
    Arg a;

    if (w == (Widget *) NULL) {
	return;
    }
    if (w[value + 2] == (Widget) NULL) {
	errwin("Internal error, SetChoice: Attempt to set NULL Widget");
	return;
    }
    XtSetArg(a, XmNmenuHistory, w[value + 2]);
    XtSetValues(w[0], &a, 1);
}

int GetChoice(Widget * w)
{
    Arg a;
    Widget warg;
    int i;

    if (w == NULL) {
	errwin("Internal error, GetChoice called with NULL argument");
	return 0;
    }
    XtSetArg(a, XmNmenuHistory, &warg);
    XtGetValues(w[0], &a, 1);
    i = 0;
    while (w[i + 2] != warg) {
	if (w[i + 2] == NULL) {
	    errwin("Internal error, GetChoice: Found NULL in Widget list");
	    return 0;
	}
	i++;
    }
    return i;
}

Widget *CreateFormatChoice(Widget parent, char *s)
{
    Widget *w;
    
    w = CreatePanelChoice0(parent,
                       s, 4,
                       32,
                       "Decimal",
                       "Exponential",
                       "General",
                       "Power",
                       "Scientific",
                       "Engineering",
                       "DD-MM-YY",
                       "MM-DD-YY",
                       "YY-MM-DD",
                       "MM-YY",
                       "MM-DD",
                       "Month-DD",
                       "DD-Month",
                       "Month (abrev.)",
                       "Month (abrev.)-YY",
                       "Month",
                       "Day of week (abrev.)",
                       "Day of week",
                       "Day of year",
                       "HH:MM:SS.s",
                       "MM-DD HH:MM:SS.s",
                       "MM-DD-YY HH:MM:SS.s",
                       "YY-MM-DD HH:MM:SS.s",
                       "Degrees (lon)",
                       "DD MM' (lon)",
                       "DD MM' SS.s\" (lon)",
                       "MM' SS.s\" (lon)",
                       "Degrees (lat)",
                       "DD MM' (lat)",
                       "DD MM' SS.s\" (lat)",
                       "MM' SS.s\" (lat)", 
                       NULL,
                       0);
    
    return(w);
}

Widget *CreatePrecisionChoice(Widget parent, char *s)
{
    Widget *w;
    
    w = CreatePanelChoice(parent, s,
                          11,
                          "0", "1", "2", "3", "4",
                          "5", "6", "7", "8", "9",
                          NULL,
                          0);

    return(w);
}
    


Widget CreateCharSizeChoice(Widget parent, char *s)
{
    Widget w;
    XmString str;
    
/*
 *     rc = XmCreateRowColumn(parent, "rcCharSize", NULL, 0);
 *     XtVaSetValues(rc, 
 *                   XmNorientation, XmHORIZONTAL,
 *                   XmNisAligned, True,
 *                   XmNentryAlignment, XmALIGNMENT_END,
 *                   XmNadjustLast, True,
 *                   NULL);
 * 
 *     XtVaCreateManagedWidget(s, xmLabelWidgetClass, rc, NULL);
 */

    str = XmStringCreate(s, charset);
    w = XtVaCreateManagedWidget("charSize", xmScaleWidgetClass, parent,
				XmNtitleString, str,
				XmNminimum, 0,
				XmNmaximum, 400,
				XmNvalue, 100,
				XmNshowValue, True,
#if XmVersion >= 2000    
				XmNsliderMark, XmROUND_MARK,
#endif
				XmNprocessingDirection, XmMAX_ON_RIGHT,
				XmNorientation, XmHORIZONTAL,
				NULL);
    XmStringFree(str);
/*
 *     XtManageChild(rc);
 */
    
    return w;
}

double GetCharSizeChoice(Widget w)
{
    Arg arg;
    int value;
    
    XtSetArg(arg, XmNvalue, &value);
    XtGetValues(w, &arg, 1);
    
    return ((double) value / 100);
}

void SetCharSizeChoice(Widget w, double size)
{
    Arg arg;
    int value;
    
    if (w == NULL) {
        return;
    }
    
    value = (int) rint(size*100);
    XtSetArg(arg, XmNvalue, value);
    XtSetValues(w, &arg, 1);
    
    return;
}

Widget CreateAngleChoice(Widget parent, char *s)
{
    Widget w;
    XmString str;
    
    str = XmStringCreate(s, charset);
    w = XtVaCreateManagedWidget("angle", xmScaleWidgetClass, parent,
				XmNtitleString, str,
				XmNminimum, 0,
				XmNmaximum, 360,
				XmNvalue, 100,
				XmNshowValue, True,
#if XmVersion >= 2000    
				XmNsliderMark, XmROUND_MARK,
#endif
				XmNprocessingDirection, XmMAX_ON_RIGHT,
				XmNorientation, XmHORIZONTAL,
				NULL);
    XmStringFree(str);

    return w;
}

int GetAngleChoice(Widget w)
{
    Arg arg;
    int value;
    
    XtSetArg(arg, XmNvalue, &value);
    XtGetValues(w, &arg, 1);
    
    return (value);
}

void SetAngleChoice(Widget w, int angle)
{
    Arg arg;
    
    XtSetArg(arg, XmNvalue, angle);
    XtSetValues(w, &arg, 1);
    
    return;
}


Widget CreateToggleButton(Widget parent, char *s)
{
    return (XtVaCreateManagedWidget(s, xmToggleButtonWidgetClass, parent, NULL));
}

int GetToggleButtonState(Widget w)
{
    return (XmToggleButtonGetState(w));
}

void SetToggleButtonState(Widget w, int value)
{
    if (w == NULL) {
        return;
    }
    XmToggleButtonSetState(w, value ? True:False, False);
    
    return;
}


Widget CreateFrame(Widget parent, char *s)
{
    Widget fr;
    
    fr = XtVaCreateManagedWidget("frame", xmFrameWidgetClass, parent, NULL);
    if (s != NULL) {
        XtVaCreateManagedWidget(s, xmLabelWidgetClass, fr,
				XmNchildType, XmFRAME_TITLE_CHILD,
				NULL);
    }
    
    return (fr);   
}

Widget CreateTab(Widget parent)
{
    Widget tab;
    
    tab = XtVaCreateManagedWidget("tab", xmTabWidgetClass, parent, NULL);
    
    return (tab);
}

Widget CreateTabPage(Widget parent, char *s)
{
    Widget w;
    XmString str;
    
    w = XmCreateRowColumn(parent, "tabPage", NULL, 0);
    str = XmStringCreateSimple(s);
    XtVaSetValues(w, XmNtabLabel, str, NULL);
    XmStringFree(str);
    XtManageChild(w);
    
    return (w);
}

void SelectTabPage(Widget tab, Widget w)
{
    XmTabSetTabWidget(tab, w, True);
}

Widget CreateTextItem2(Widget parent, int len, char *s)
{
    Widget w;
    Widget rc;
    XmString str;
    rc = XmCreateRowColumn(parent, "rc", NULL, 0);
    XtVaSetValues(rc, XmNorientation, XmHORIZONTAL, NULL);
    str = XmStringCreateLtoR(s, charset);
    XtVaCreateManagedWidget("label", xmLabelWidgetClass, rc,
			    XmNlabelString, str,
			    NULL);
    XmStringFree(str);
    w = XtVaCreateManagedWidget("text", xmTextWidgetClass, rc,
				XmNtraversalOn, True,
				XmNcolumns, len,
				NULL);
    XtManageChild(rc);
    return w;
}

Widget CreateTextItem4(Widget parent, int len, char *label)
{
    Widget retval;
    XtVaCreateManagedWidget(label, xmLabelWidgetClass, parent, NULL);
    retval = XtVaCreateManagedWidget("text", xmTextWidgetClass, parent, NULL);
    XtVaSetValues(retval, XmNcolumns, len, NULL);
    return retval;
}


/* 
 * create a multiline editable window
 * parent = parent widget
 * len    = width of edit window
 * hgt    = number of lines in edit window
 * s      = label for window
 * 
 * returns the edit window widget
 */
Widget CreateScrollTextItem2(Widget parent, int len, int hgt, char *s)
{
    Widget w;
    Widget rc;
    XmString str;
	
    rc = XmCreateRowColumn(parent, "rc", NULL, 0);
    XtVaSetValues(rc, XmNorientation, XmHORIZONTAL, NULL);
    str = XmStringCreateLtoR(s, charset);
    XtVaCreateManagedWidget("label", xmLabelWidgetClass, rc,
			    XmNlabelString, str,
			    NULL);
    XmStringFree(str);
    w = XmCreateScrolledText( rc, "text", NULL, 0 );
    XtVaSetValues(w,
		  XmNcolumns, len,
		  XmNrows, hgt,
		  XmNeditMode, XmMULTI_LINE_EDIT,
		  XmNwordWrap, True,
		  NULL);
    XtManageChild(w);
    XtManageChild(rc);
    return w;
}


char *xv_getstr(Widget w)
/* 
 * return the string from a text widget
 *
 * NB - newlines are converted to spaces
 */
{
    char *s;
    int i;
    static char buf[MAX_STRING_LENGTH];

    strncpy(buf, s = XmTextGetString(w), MAX_STRING_LENGTH - 1);
    XtFree(s);
    
    i=strlen(buf);
    for (i--; i >= 0; i--) {
        if (buf[i] == '\n') {
            buf[i] = ' ';
        }
    }
    return buf;
}


/*
 * xv_evalexpr - take a text field and pass it to the parser if it needs to
 * evaluated, else use atof().
 * place the double result in answer
 * if an error, return False, else True
 */
Boolean xv_evalexpr(Widget w, double *answer )
{
    char *s;
    static char buf[MAX_STRING_LENGTH];
    int i, len, ier = 0;
    double x, y, a, b, c, d;
	
    strncpy(buf, s = XmTextGetString(w), MAX_STRING_LENGTH - 1);
    XtFree(s);

    if (!(len = strlen( buf ) )) { /* check for zero length */
        *answer = 0;
        return GRACE_EXIT_FAILURE;
    }
    /* first character may be a sign */
    if (!fpdigit[(int) s[0]] && s[0] != '-' && s[0] != '+') {
        i = len +1;
    } else {
        i = 1;
    }

    for (; i<len; i++) {
        if (!fpdigit[(int) buf[i]]) {
            break;
        }
    }

    if (i == len) {         /* only floating point digits */
        *answer = atof( buf );
        return GRACE_EXIT_SUCCESS;
    } else {                /* must evaluate an expression */
        scanner(buf, &x, &y, 1, &a, &b, &c, &d, 1, 0, 0, &ier);
        if( !ier ) {
            *answer = result;
            return GRACE_EXIT_SUCCESS;
        } else {
            *answer = 0;
            return GRACE_EXIT_FAILURE;
        }
    }
}

/*
 * xv_evalexpri - take a text field and pass it to the parser if it needs to
 * evaluated, else use atoi().
 * place the integer result in answer
 * if an error, return False, else True
 */
Boolean xv_evalexpri(Widget w, int *answer )
{
    char *s;
    static char buf[MAX_STRING_LENGTH];
    int i, len, ier = 0;
    double x, y, a, b, c, d;
	
    strncpy(buf, s = XmTextGetString(w), MAX_STRING_LENGTH - 1);
    XtFree(s);

    if (!(len = strlen( buf ) )) { /* check for zero length */
        *answer = 0;
        return GRACE_EXIT_FAILURE;
    }
    /* first character may be a sign */
    if (!fpdigit[(int) s[0]] && s[0] != '-' && s[0] != '+') {
        i = len +1;
    } else {
        i = 1;
    }
    
    for (; i<len; i++) {
        if (!fpdigit[(int) buf[i]]) {
            break;
        }
    }

    if (i == len) {             /* only floating point digits */
        *answer = atoi(buf);
        return GRACE_EXIT_SUCCESS;
    } else {                    /* must evaluate an expression */
        scanner(buf, &x, &y, 1, &a, &b, &c, &d, 1, 0, 0, &ier);
        if( !ier ) {
            *answer = (int)result;
            return GRACE_EXIT_SUCCESS;
        } else {
            *answer = 0;
            return GRACE_EXIT_FAILURE;
        }
    }
}


void xv_setstr(Widget w, char *s)
{
    if (s != NULL && w != NULL) {
        XmTextSetString(w, s);
    }
}

/*
 * generic unmanage popup routine, used elswhere
 */
void destroy_dialog(Widget w, XtPointer client_data, XtPointer call_data)
{
    XtUnmanageChild((Widget) client_data);
}

/*
 * handle the close item on the WM menu
 */
void handle_close(Widget w)
{
    Atom WM_DELETE_WINDOW;
    XtVaSetValues(w, XmNdeleteResponse, XmDO_NOTHING, NULL);
    WM_DELETE_WINDOW = XmInternAtom(disp, "WM_DELETE_WINDOW", False);
    XmAddProtocolCallback(w,
        XM_WM_PROTOCOL_ATOM(w),
        WM_DELETE_WINDOW,
        destroy_dialog,
        (XtPointer) w);
}

/*
 * Manage and raise
 */
void XtRaise(Widget w)
{
    XtManageChild(w);
    XMapRaised(XtDisplay(w), XtWindow(w));
    savewidget(w);
}


static Widget *savewidgets = NULL;
static int nsavedwidgets = 0;

void savewidget(Widget w)
{
    int i;
    
    for (i = 0; i < nsavedwidgets; i++) {
        if (w == savewidgets[i]) {
            return;
        }
    }
    
    savewidgets = xrealloc(savewidgets, (nsavedwidgets + 1)*sizeof(Widget));
    savewidgets[nsavedwidgets] = w;
    nsavedwidgets++;
}

void deletewidget(Widget w)
{
    int i;
    
    for (i = 0; i < nsavedwidgets; i++) {
        if (w == savewidgets[i]) {
            nsavedwidgets--;
            for (; i <  nsavedwidgets; i++) {
                savewidgets[i] = savewidgets[i + 1];
            }
            savewidgets = xrealloc(savewidgets, nsavedwidgets*sizeof(Widget));
            XtDestroyWidget(w);
            return;
        }
    }
    
}

void DefineDialogCursor(Cursor c)
{
    int i;
    
    for (i = 0; i < nsavedwidgets; i++) {
	XDefineCursor(disp, XtWindow(savewidgets[i]), c);
    }
    XFlush(disp);
}

void UndefineDialogCursor()
{
    int i;
    
    for (i = 0; i < nsavedwidgets; i++) {
	XUndefineCursor(disp, XtWindow(savewidgets[i]));
    }
    XFlush(disp);
}

Widget CreateCommandButtonsNoDefault(Widget parent, int n, Widget * buts, char **l)
{
    int i;
    Widget form;
    Dimension h;

    form = XtVaCreateWidget("form", xmFormWidgetClass, parent,
			    XmNfractionBase, n,
			    NULL);

    for (i = 0; i < n; i++) {
	buts[i] = XtVaCreateManagedWidget(l[i],
					  xmPushButtonWidgetClass, form,
					  XmNtopAttachment, XmATTACH_FORM,
					  XmNbottomAttachment, XmATTACH_FORM,
					  XmNleftAttachment, XmATTACH_POSITION,
					  XmNleftPosition, i,
					  XmNrightAttachment, XmATTACH_POSITION,
					  XmNrightPosition, i + 1,
					  XmNleftOffset, (i == 0) ? 2 : 0,
					  XmNrightOffset, 3,
					  XmNtopOffset, 2,
					  XmNbottomOffset, 3,
					  NULL);
    }
    XtManageChild(form);
    XtVaGetValues(buts[0], XmNheight, &h, NULL);
    XtVaSetValues(form, XmNpaneMaximum, h, XmNpaneMinimum, h, NULL);
    
    return form;
}

Widget CreateCommandButtons(Widget parent, int n, Widget * buts, char **l)
{
    int i;
    Widget form;
    Dimension h;

    form = XtVaCreateWidget("form", xmFormWidgetClass, parent,
			    XmNfractionBase, n,
			    NULL);

    for (i = 0; i < n; i++) {
	buts[i] = XtVaCreateManagedWidget(l[i],
					  xmPushButtonWidgetClass, form,
					  XmNtopAttachment, XmATTACH_FORM,
					  XmNbottomAttachment, XmATTACH_FORM,
					  XmNleftAttachment, XmATTACH_POSITION,
					  XmNleftPosition, i,
					  XmNrightAttachment, XmATTACH_POSITION,
					  XmNrightPosition, i + 1,
					  XmNdefaultButtonShadowThickness, 1,
					  XmNshowAsDefault, (i == 0) ? True : False,
					  NULL);
    }
    XtManageChild(form);
    XtVaGetValues(buts[0], XmNheight, &h, NULL);
    XtVaSetValues(form, XmNpaneMaximum, h, XmNpaneMinimum, h, NULL);
    
    return form;
}



static SetChoiceItem *plist = NULL;
static int nplist = 0;

SetChoiceItem CreateSetSelector(Widget parent,
				char *label,
				int type,
				int ff,
				int gtype,
				int stype)
{
    Arg args[3];
    Widget rc2, lab;
    SetChoiceItem sel;
    
    rc2 = XtVaCreateWidget("rc", xmRowColumnWidgetClass, parent,
			      XmNorientation, XmVERTICAL, NULL);
    lab = XmCreateLabel(rc2, label, NULL, 0);
    XtManageChild(lab);
    XtSetArg(args[0], XmNlistSizePolicy, XmRESIZE_IF_POSSIBLE);
    XtSetArg(args[1], XmNvisibleItemCount, 8);
    sel.list = XmCreateScrolledList(rc2, "list", args, 2);
    if (stype == SELECTION_TYPE_MULTIPLE) {	/* multiple select */
	XtVaSetValues(sel.list,
		      XmNselectionPolicy, XmEXTENDED_SELECT,
		      NULL);
    } else {			/* single select */
    	XtVaSetValues(sel.list,
		      XmNselectionPolicy, XmSINGLE_SELECT,
		      NULL);
    }
    sel.type = type;
    sel.gno = gtype;
    XtManageChild(sel.list);
    sel.indx = save_set_list(sel);
    update_set_list(gtype == GRAPH_SELECT_CURRENT ? get_cg():sel.gno, sel);
    
    XtManageChild(rc2);
    return sel;
}

int GetSetFromString(char *buf)
{
    int retval = SET_SELECT_ERROR;
    if (strcmp(buf, "New set") == 0) {
	retval = SET_SELECT_NEXT;
    } else if (strcmp(buf, "All sets") == 0) {
	retval = SET_SELECT_ALL;
    } else {
	sscanf(buf, "S%d", &retval);
    }
    return retval;
}

int GetSelectedSet(SetChoiceItem l)
{
    int retval = SET_SELECT_ERROR;
    int *pos_list;
    int pos_cnt, cnt;
    char buf[256];
	
    if (XmListGetSelectedPos(l.list, &pos_list, &pos_cnt)) {
	XmString *s, cs;
	char *cstr;
	XtVaGetValues(l.list,
		      XmNselectedItemCount, &cnt,
		      XmNselectedItems, &s,
		      NULL);
	cs = XmStringCopy(*s);
	if (XmStringGetLtoR(cs, charset, &cstr)) {
	    strcpy(buf, cstr);
	    if (strcmp(buf, "New set") == 0) {
		retval = SET_SELECT_NEXT;
	    } else if (strcmp(buf, "All sets") == 0) {
		retval = SET_SELECT_ALL;
	    } else if (strcmp(buf, "Nearest set") == 0) {
		retval = SET_SELECT_NEAREST;
	    } else {
		sscanf(buf, "S%d", &retval);
	    }
	    XtFree(cstr);
	}
        XmStringFree(cs);
    }
    
    return retval;
}

int SetSelectedSet(int gno, int setno, SetChoiceItem l)
{
    char buf[1024];
    XmString xms;
    sprintf(buf, "S%d (N=%d, %s)", setno, getsetlength(gno, setno), getcomment(gno, setno));
    xms = XmStringCreateLtoR(buf, charset);
    XmListSelectItem(l.list, xms, True);
    XmStringFree(xms);
    return 0;
}

/*
 * if the set selection type is multiple, then get a
 * list of sets, returns the number of selected sets.
 */
int GetSelectedSets(SetChoiceItem l, int **sets)
{
    int i;
    int cnt = SET_SELECT_ERROR, retval = SET_SELECT_ERROR;
    int *ptr;
    int *pos_list;
    int pos_cnt, gno;
    if (XmListGetSelectedPos(l.list, &pos_list, &pos_cnt)) {
	char buf[256];
	char *cstr;
	XmString *s, cs;

	XtVaGetValues(l.list,
		      XmNselectedItemCount, &cnt,
		      XmNselectedItems, &s,
		      NULL);
	*sets = malloc(cnt * SIZEOF_INT);
	ptr = *sets;
	for (i = 0; i < cnt; i++) {
	    cs = XmStringCopy(s[i]);
	    if (XmStringGetLtoR(cs, charset, &cstr)) {
		strcpy(buf, cstr);
		if (strcmp(buf, "New set") == 0) {
		    retval = SET_SELECT_NEXT;
		    return retval;
		} else if (strcmp(buf, "All sets") == 0) {
		    int j, nsets = 0;
		    retval = SET_SELECT_ALL;
		    if (l.gno == GRAPH_SELECT_CURRENT) {
			gno = get_cg();
		    } else {
			gno = l.gno;
		    }
		    retval = nactive(gno);
		    *sets = xrealloc(*sets, retval * SIZEOF_INT);
		    ptr = *sets;
		    for (j = 0; j < number_of_sets(gno); j++) {
			if (is_set_active(gno, j)) {
			    ptr[nsets] = j;
			    nsets++;
			}
		    }
		    if (nsets != retval) {
			errwin("Nsets != reval, can't happen!");
		    }
		    return retval;
		} else {
		    sscanf(buf, "S%d", &retval);
		}
		ptr[i] = retval;
		XtFree(cstr);
	    }
	    XmStringFree(cs);
	}
    }
    return cnt;
}

int save_set_list(SetChoiceItem l)
{
    nplist++;
    plist = xrealloc(plist, nplist*sizeof(SetChoiceItem));
    plist[nplist - 1] = l;
    return nplist - 1;
}

void update_save_set_list( SetChoiceItem l, int newgr )
{
    plist[l.indx] = l;
    update_set_list( newgr, plist[l.indx] );
}

void update_set_list(int gno, SetChoiceItem l)
{
    int i, cnt, scnt=0;
    char buf[1024];
    XmString *xms;
    
    XmListDeleteAllItems(l.list);
    for (i = 0; i < number_of_sets(gno); i++) {
	if (is_set_active(gno, i)) {
	    scnt++;
	}
    }

    switch (l.type) {		/* TODO */
    case SET_SELECT_ACTIVE:
	xms = malloc(sizeof(XmString) * scnt);
	cnt = 0;
	break;
    case SET_SELECT_ALL:
	xms = malloc(sizeof(XmString) * (scnt + 1));
	xms[0] = XmStringCreateLtoR("All sets", charset);
	cnt = 1;
	break;
    case SET_SELECT_NEXT:
	xms = malloc(sizeof(XmString) * (scnt + 1));
	xms[0] = XmStringCreateLtoR("New set", charset);
	cnt = 1;
	break;
    case SET_SELECT_NEAREST:
	xms = malloc(sizeof(XmString) * (scnt + 1));
	xms[0] = XmStringCreateLtoR("Nearest set", charset);
	cnt = 1;
	break;
    default:
	xms = malloc(sizeof(XmString) * scnt);
	cnt = 0;
	break;
    }

    for (i = 0; i < number_of_sets(gno); i++) {
        if (is_set_active(gno, i)) {
            sprintf(buf, "S%d (N=%d, %s)", i, getsetlength(gno, i), getcomment(gno, i));
            xms[cnt] = XmStringCreateLtoR(buf, charset);
            cnt++;
        }
    }
#if XmVersion > 1001    
    XmListAddItemsUnselected(l.list, xms, cnt, 0);
#endif

    /* automatically highlight if only 1 selection */
    if (scnt == 1) {
        XmListSelectItem(l.list, xms[cnt-1], True);
    }
	
    for (i = 0; i < cnt; i++) {
#if XmVersion < 1002   /* For Motif 1.1 */
        XmListAddItemUnselected(l.list, xms[i], 0);
#endif    
        XmStringFree(xms[i]);
    }
    free(xms);
}

void update_set_lists(int gno)
{
    int i;

    if (gno == GRAPH_SELECT_CURRENT) {
        update_set_selectors(get_cg());
    } else {
        update_set_selectors(gno);
    }

    if (inwin && lists_dirty()) {
	for (i = 0; i < nplist; i++) {
		if( plist[i].gno == gno || 
			(gno==get_cg() && plist[i].gno==GRAPH_SELECT_CURRENT) )
	    	update_set_list(gno, plist[i]);
	}
	set_lists_dirty(FALSE);
    }
}


Widget CreateMenuBar(Widget parent, char *name, char *help_anchor)
{
    Widget menubar;
    
    menubar = XmCreateMenuBar(parent, name, NULL, 0);
    
    if (help_anchor) {
     	XtAddCallback(menubar, XmNhelpCallback, (XtCallbackProc) HelpCB,
    			(XtPointer) help_anchor);
    }

    return menubar;
}

Widget CreateMenu(Widget parent, char *name, char *label, char mnemonic,
	Widget *cascade, char *help_anchor)
{
    Widget menu, cascadeTmp;
    XmString str;
    
    str = XmStringCreateSimple(label);
    menu = XmCreatePulldownMenu(parent, name, NULL, 0);
    cascadeTmp = XtVaCreateWidget((String) name, xmCascadeButtonWidgetClass, parent, 
    	XmNlabelString, str, 
    	XmNmnemonic, mnemonic,
    	XmNsubMenuId, menu, 
    	0);
    XmStringFree(str);
    if (help_anchor) {
     	XtAddCallback(menu, XmNhelpCallback, (XtCallbackProc) HelpCB,
    			(XtPointer) help_anchor);
    }
    XtManageChild(cascadeTmp);
    if (cascade != NULL) {
    	*cascade = cascadeTmp;
    }

    return menu;
}


Widget CreateMenuButton(Widget parent, char *name, char *label, char mnemonic,
	XtCallbackProc cb, XtPointer data, char *help_anchor)
{
    Widget button;
    XmString str;
    
    str = XmStringCreateSimple(label);
    button = XtVaCreateManagedWidget((String) name, xmPushButtonWidgetClass, parent, 
    	XmNlabelString, str,
    	XmNmnemonic, mnemonic,
    	NULL);
    XmStringFree(str);
    XtAddCallback(button, XmNactivateCallback, cb, data);
    if (help_anchor) {
     	XtAddCallback(button, XmNhelpCallback, (XtCallbackProc) HelpCB,
    			(XtPointer) help_anchor);
    }

    return button;
}

Widget CreateMenuToggle(Widget parent, char *name, char *label, char mnemonic,
	XtCallbackProc cb, XtPointer data, char *help_anchor)
{
    Widget button;
    XmString str;
    
    str = XmStringCreateSimple(label);
    button = XtVaCreateManagedWidget((String) name, xmToggleButtonWidgetClass, parent, 
    	XmNlabelString, str,
    	XmNmnemonic, mnemonic,
    	XmNvisibleWhenOff, True,
    	XmNindicatorOn, True,
    	NULL);
    XmStringFree(str);
    if (cb) {
        XtAddCallback(button, XmNvalueChangedCallback, cb, data);
    }
    if (help_anchor) {
     	XtAddCallback(button, XmNhelpCallback, (XtCallbackProc) HelpCB,
    			(XtPointer) help_anchor);
    }

    return button;
}

Widget CreateSeparator(Widget parent)
{
    Widget sep;
    
    sep = XmCreateSeparator(parent, "sep", NULL, 0);
    XtManageChild(sep);
    return sep;
}

Widget CreateMenuLabel(Widget parent, char *name)
{
    Widget lab;
    
    lab = XmCreateLabel(parent, name, NULL, 0);
    XtManageChild(lab);
    return lab;
}


static int yesno_retval = 0;
static Boolean keep_grab = True;

void yesnoCB(Widget w, Boolean * keep_grab, XmAnyCallbackStruct * reason)
{
    int why = reason->reason;

    *keep_grab = False;
    XtRemoveGrab(XtParent(w));
    XtUnmanageChild(w);
    switch (why) {
    case XmCR_OK:
	yesno_retval = 1;
	/* process ok action */
	break;
    case XmCR_CANCEL:
	yesno_retval = 0;
	/* process cancel action */
	break;
    }
}

int yesnowin(char *msg, char *s1, char *s2, char *help_anchor)
{
    Widget yesno_popup = NULL;
    XmString str;
    XEvent event;

    keep_grab = True;

    if (yesno_popup == NULL) {
	yesno_popup = XmCreateErrorDialog(app_shell, "warndlg", NULL, 0);
	if (msg != NULL) {
	    str = XmStringCreateLtoR(msg, charset);
	} else {
	    str = XmStringCreateLtoR("Warning", charset);
	}
	XtVaSetValues(yesno_popup, XmNmessageString, str, NULL);
	XmStringFree(str);
	
	if (s1 != NULL) {
	    str = XmStringCreateLtoR(s1, charset);
	} else {
	    str = XmStringCreateLtoR("OK", charset);
	}
	XtVaSetValues(yesno_popup, str, XmNokLabelString, NULL);
	XmStringFree(str);
	
	if (s2 != NULL) {
	    str = XmStringCreateLtoR(s2, charset);
	} else {
	    str = XmStringCreateLtoR("Cancel", charset);
	}
	XtVaSetValues(yesno_popup, str, XmNcancelLabelString, NULL);
	XmStringFree(str);
	
	XtAddCallback(yesno_popup, XmNokCallback, (XtCallbackProc) yesnoCB,
	(XtPointer) & keep_grab);
	XtAddCallback(yesno_popup, XmNcancelCallback, (XtCallbackProc) yesnoCB,
	(XtPointer) & keep_grab);

	if (help_anchor) {
	    XtAddCallback(yesno_popup, XmNhelpCallback, (XtCallbackProc) HelpCB,
	    (XtPointer) help_anchor);
	    XtSetSensitive(XtNameToWidget(yesno_popup, "Help"), True);
	} else {    
	    XtSetSensitive(XtNameToWidget(yesno_popup, "Help"), False);
	}
	XtManageChild(yesno_popup);
    }
    XtRaise(yesno_popup);
    XtAddGrab(XtParent(yesno_popup), True, False);
    while (keep_grab || XtAppPending(app_con)) {
	XtAppNextEvent(app_con, &event);
	XtDispatchEvent(&event);
    }
    return yesno_retval;
}


/*
 * close error window and set message to null
 */
void cancelerrwin( Widget w, XtPointer client_data, XtPointer call_data)
{
	XmString empty;
	
	empty = XmStringCreateLtoR( "", charset);
	XtVaSetValues( (Widget)client_data, XmNmessageString, empty, NULL );
	XmStringFree( empty );
	XtUnmanageChild( (Widget)client_data );
}	

/*
 * open error window
 * all errors are sent to the same window
 */
void errwin(char *s)
{
    XmString str, str1, nl;
    static Widget error_popup = NULL;

    keep_grab = True;
    
    log_results(s);
    
    if (error_popup == NULL) {
        error_popup = XmCreateErrorDialog(app_shell, "errorDialog", NULL, 0);
		str = XmStringCreateLtoR(s, charset);
		XtVaSetValues(error_popup, XmNmessageString, str, NULL);
		XmStringFree(str);
		XtVaSetValues(error_popup,
		  XmNdialogTitle, XmStringCreateLtoR("Error (you may hit return to cancel)", charset),
		  XmNdialogStyle, XmDIALOG_APPLICATION_MODAL,
		  NULL);
		XtAddCallback(error_popup, XmNokCallback, 
				(XtCallbackProc) cancelerrwin, (XtPointer) error_popup);
		XtAddCallback(error_popup, XmNhelpCallback, (XtCallbackProc) HelpCB,
		  (XtPointer) NULL);
		XtUnmanageChild(XmMessageBoxGetChild(error_popup,
		XmDIALOG_CANCEL_BUTTON));
		XtUnmanageChild(XmMessageBoxGetChild(error_popup,
		XmDIALOG_HELP_BUTTON));
		XtManageChild(error_popup);
    } else {									/* add new error onto end */
		nl = XmStringCreateLtoR( "\n", charset);
		XtVaGetValues( error_popup, XmNmessageString, &str, NULL );
		str1 = 	XmStringConcat( str, nl );
		XmStringFree( str );
		str = XmStringCreateLtoR(s, charset);
		XtVaSetValues(error_popup,XmNmessageString,
					XmStringConcat( str1, str), NULL);
		XmStringFree( str );
		XmStringFree( str1 );
		XmStringFree( nl );
	}
    XtRaise(error_popup);
}

Widget CreateAACButtons(Widget parent, Widget form, XtCallbackProc aac_cb)
{
    Widget w;
    Widget aacbut[3];
    static char *aaclab[3] = {"Apply", "Accept", "Close"};
    
    w = CreateCommandButtons(parent, 3, aacbut, aaclab);
    XtAddCallback(aacbut[0], XmNactivateCallback, aac_cb, (XtPointer) AAC_APPLY);
    XtAddCallback(aacbut[1], XmNactivateCallback, aac_cb, (XtPointer) AAC_ACCEPT);
    XtAddCallback(aacbut[2], XmNactivateCallback, aac_cb, (XtPointer) AAC_CLOSE);
    
    if (form != NULL) {
        XtVaSetValues(form, XmNcancelButton, aacbut[2], NULL);
    }
    
    return w;
}

void SetLabel(Widget w, char *s)
{
    XmString str;

    str = XmStringCreateSimple(s);
    XtVaSetValues(w, XmNlabelString, str, NULL);
    XmStringFree(str);
}


void update_all(void)
{
    int gno = get_cg();
    
    set_lists_dirty(TRUE);
    update_set_lists(gno);

    update_graph_selectors();
    update_set_selectors(ALL_GRAPHS);

    update_ticks(gno);
/*
 *     update_view(gno);
 */
/*
 *     update_graphapp_items(gno);
 */
/*
 *     update_frame_items(gno);
 */
    update_props_items();
    update_hotlinks();
    update_prune_frame();
    update_describe_popup ();
    update_locator_items(gno);
    set_stack_message();
}
