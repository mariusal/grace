/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2004 Grace Development Team
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
 *
 * utilities for Motif
 *
 */

#include <config.h>

#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>

#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>

#include <Xm/Xm.h>
#include <Xm/Protocols.h>
#include <Xm/BulletinB.h>
#include <Xm/MessageB.h>
#include <Xm/DialogS.h>
#include <Xm/FileSB.h>
#include <Xm/Frame.h>
#include <Xm/Form.h>
#include <Xm/Scale.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/List.h>
#include <Xm/PushB.h>
#include <Xm/CascadeBG.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#include <Xm/Text.h>
#include <Xm/ToggleB.h>
#include <Xm/ArrowBG.h>

#ifdef WITH_EDITRES
#  include <X11/Xmu/Editres.h>
#endif

#if XmVersion < 2000
#  define XmStringConcatAndFree(a, b) XmStringConcat(a, b); XmStringFree(a); XmStringFree(b)
#endif

#include "Tab.h"
#include "motifinc.h"

#include "defines.h"
#include "globals.h"
#include "grace/canvas.h"
#include "jbitmaps.h"
#include "core_utils.h"
#include "utils.h"
#include "dicts.h"
#include "parser.h"
#include "protos.h"

#define canvas grace->rt->canvas

static XmStringCharSet charset = XmFONTLIST_DEFAULT_TAG;

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


extern XtAppContext app_con;

static unsigned long xvlibcolors[MAXCOLORS];


static OptionItem *color_option_items = NULL;
static unsigned int ncolor_option_items = 0;
static OptionStructure **color_selectors = NULL;
static unsigned int ncolor_selectors = 0;

static char *label_to_resname(const char *s, const char *suffix)
{
    char *retval, *rs;
    int capitalize = FALSE;
    
    retval = copy_string(NULL, s);
    rs = retval;
    while (*s) {
        if (isalnum(*s)) {
            if (capitalize == TRUE) {
                *rs = toupper(*s);
                capitalize = FALSE;
            } else {
                *rs = tolower(*s);
            }
            rs++;
        } else {
            capitalize = TRUE;
        }
        s++;
    }
    *rs = '\0';
    if (suffix != NULL) {
        retval = concat_strings(retval, suffix);
    }
    return retval;
}


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

void SetSensitive(Widget w, int onoff)
{
    XtSetSensitive(w, onoff ? True : False);
}

Widget GetParent(Widget w)
{
    if (w) {
        return (XtParent(w));
    } else {
        errmsg("Internal error: GetParent() called with NULL widget");
        return NULL;
    }
}

void RegisterEditRes(Widget shell)
{
#ifdef WITH_EDITRES    
    XtAddEventHandler(shell, (EventMask) 0, True, _XEditResCheckMessages, NULL);
#endif
}

void SetDimensions(Widget w, unsigned int width, unsigned int height)
{
    XtVaSetValues(w,
        XmNwidth, (Dimension) width,
        XmNheight, (Dimension) height,
        NULL);
}

void GetDimensions(Widget w, unsigned int *width, unsigned int *height)
{
    Dimension ww, wh;

    XtVaGetValues(w,
        XmNwidth, &ww,
        XmNheight, &wh,
        NULL);

    *width  = (unsigned int) ww;
    *height = (unsigned int) wh;
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


#define MAX_PULLDOWN_LENGTH 30

OptionStructure *CreateOptionChoice(Widget parent, char *labelstr, int ncols,
                                                int nchoices, OptionItem *items)
{
    Arg args[2];
    XmString str;
    OptionStructure *retval;

    retval = xmalloc(sizeof(OptionStructure));

    XtSetArg(args[0], XmNpacking, XmPACK_COLUMN);
    retval->pulldown = XmCreatePulldownMenu(parent, "pulldownMenu", args, 1);

    retval->ncols = ncols;
    
    retval->nchoices = 0;
    retval->options = NULL;
    UpdateOptionChoice(retval, nchoices, items);

    str = XmStringCreateLocalized(labelstr);
    XtSetArg(args[0], XmNlabelString, str);
    XtSetArg(args[1], XmNsubMenuId, retval->pulldown);

    retval->menu = XmCreateOptionMenu(parent, "optionMenu", args, 2);

    XmStringFree(str);

    XtManageChild(retval->menu);
    
    return retval;
}

OptionStructure *CreateOptionChoiceVA(Widget parent, char *labelstr, ...)
{
    OptionStructure *retval;
    int nchoices = 0;
    OptionItem *oi = NULL;
    va_list var;
    char *s;
    int value;

    va_start(var, labelstr);
    while ((s = va_arg(var, char *)) != NULL) {
        value = va_arg(var, int);
        nchoices++;
        oi = xrealloc(oi, nchoices*sizeof(OptionItem));
        oi[nchoices - 1].value = value;
        oi[nchoices - 1].label = copy_string(NULL, s);
    }
    va_end(var);

    retval = CreateOptionChoice(parent, labelstr, 1, nchoices, oi);
    
    while (nchoices) {
        nchoices--;
	xfree(oi[nchoices].label);
    }
    xfree(oi);
    
    return retval;
}

void UpdateOptionChoice(OptionStructure *optp, int nchoices, OptionItem *items)
{
    int i, nold, ncols, nw;
    Widget *wlist;
    
    nold = optp->nchoices;

    if (optp->ncols == 0) {
        ncols = 1;
    } else {
        ncols = optp->ncols;
    }
    
    /* Don't create too tall pulldowns */
    if (nchoices > MAX_PULLDOWN_LENGTH*ncols) {
        ncols = (nchoices + MAX_PULLDOWN_LENGTH - 1)/MAX_PULLDOWN_LENGTH;
    }
    
    XtVaSetValues(optp->pulldown, XmNnumColumns, ncols, NULL);

    nw = nold - nchoices;
    if (nw > 0) {
        /* Unmanage extra items before destroying to speed the things up */
        wlist = xmalloc(nw*sizeof(Widget));
        for (i = nchoices; i < nold; i++) {
            wlist[i - nchoices] = optp->options[i].widget;
        }
        XtUnmanageChildren(wlist, nw);
        xfree(wlist);
        
        for (i = nchoices; i < nold; i++) {
            XtDestroyWidget(optp->options[i].widget);
        }
    }

    optp->options = xrealloc(optp->options, nchoices*sizeof(OptionWidgetItem));
    optp->nchoices = nchoices;

    for (i = nold; i < nchoices; i++) {
        optp->options[i].widget = 
                  XmCreatePushButton(optp->pulldown, "button", NULL, 0);
    }
    
    for (i = 0; i < nchoices; i++) {
	optp->options[i].value = items[i].value;
	if (items[i].label != NULL) {
            XmString str, ostr;
            XtVaGetValues(optp->options[i].widget, XmNlabelString, &ostr, NULL);
            str = XmStringCreateLocalized(items[i].label);
            if (XmStringCompare(str, ostr) != True) {
                XtVaSetValues(optp->options[i].widget, XmNlabelString, str, NULL);
            }
            XmStringFree(str);
        }
    }
    
    nw = nchoices - nold;
    if (nw > 0) {
        wlist = xmalloc(nw*sizeof(Widget));
        for (i = nold; i < nchoices; i++) {
            wlist[i - nold] = optp->options[i].widget;
        }
        XtManageChildren(wlist, nw);
        xfree(wlist);
    }
}

OptionStructure *CreateBitmapOptionChoice(Widget parent, char *labelstr, int ncols,
                int nchoices, int width, int height, BitmapOptionItem *items)
{
    X11Stuff *xstuff = grace->gui->xstuff;
    int i;
    XmString str;
    OptionStructure *retval;
    Pixel fg, bg;
    Pixmap ptmp;

    retval = xmalloc(sizeof(OptionStructure));
    if (retval == NULL) {
        errmsg("Malloc error in CreateBitmapOptionChoice()");
    }
    retval->nchoices = nchoices;
    retval->options = xmalloc(nchoices*sizeof(OptionWidgetItem));
    if (retval->options == NULL) {
        errmsg("Malloc error in CreateBitmapOptionChoice()");
        XCFREE(retval);
        return retval;
    }


    retval->pulldown = XmCreatePulldownMenu(parent, "pulldownMenu", NULL, 0);
    XtVaSetValues(retval->pulldown, 
                  XmNentryAlignment, XmALIGNMENT_CENTER,
                  NULL);

    if (ncols > 0) {
        XtVaSetValues(retval->pulldown,
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
            ptmp = XCreatePixmapFromBitmapData(xstuff->disp, xstuff->root, 
                                    (char *) items[i].bitmap, width, height,
                                    fg, bg, xstuff->depth);
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

    retval->menu = XmCreateOptionMenu(parent, "optionMenu", NULL, 0);
    str = XmStringCreateLocalized(labelstr);
    XtVaSetValues(retval->menu,
		  XmNlabelString, str,
		  XmNsubMenuId, retval->pulldown,
		  NULL);
    XmStringFree(str);
    XtManageChild(retval->menu);
    
    return retval;
}

void UpdateCharOptionChoice(OptionStructure *opt, int font)
{
    int *old_font;
    old_font = (int *) GetUserData(opt->menu);
    if (*old_font != font) {
        int i, csize = 24;
        for (i = 0; i < opt->nchoices; i++) {
            Widget w = opt->options[i].widget;
            Pixmap ptmp = char_to_pixmap(opt->pulldown, font, (char) i, csize);
            XtVaSetValues(w, XmNlabelPixmap, ptmp, NULL);
            SetSensitive(w, ptmp ? TRUE:FALSE);
        }
        *old_font = font;
    }
}

static unsigned char dummy_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0xec, 0x2e, 0x04, 0x20, 0x00, 0x20, 0x04, 0x00,
   0x04, 0x20, 0x04, 0x20, 0x00, 0x20, 0x04, 0x00, 0x04, 0x20, 0x04, 0x20,
   0x00, 0x20, 0xdc, 0x1d, 0x00, 0x00, 0x00, 0x00};

OptionStructure *CreateCharOptionChoice(Widget parent, char *s)
{
    X11Stuff *xstuff = grace->gui->xstuff;
    int i;
    int ncols = 16, nchoices = 256;
    XmString str;
    OptionStructure *retval;
    long bg, fg;
    Pixmap ptmp;
    int *fontid;

    retval = xmalloc(sizeof(OptionStructure));
    if (retval == NULL) {
        errmsg("Malloc error in CreateBitmapOptionChoice()");
    }
    retval->nchoices = nchoices;
    retval->options = xmalloc(nchoices*sizeof(OptionWidgetItem));
    if (retval->options == NULL) {
        errmsg("Malloc error in CreateBitmapOptionChoice()");
        XCFREE(retval);
        return retval;
    }

    retval->pulldown = XmCreatePulldownMenu(parent, "pulldownMenu", NULL, 0);
    XtVaSetValues(retval->pulldown, 
                  XmNentryAlignment, XmALIGNMENT_CENTER,
                  XmNpacking, XmPACK_COLUMN,
                  XmNorientation, XmHORIZONTAL,
                  XmNnumColumns, ncols,
                  NULL);

    XtVaGetValues(retval->pulldown,
                  XmNforeground, &fg,
                  XmNbackground, &bg,
                  NULL);
    ptmp = XCreatePixmapFromBitmapData(xstuff->disp,
        xstuff->root, (char *) dummy_bits, 16, 16, fg, bg, xstuff->depth);

    for (i = 0; i < nchoices; i++) {
	retval->options[i].value = i;
        retval->options[i].widget =
            XtVaCreateWidget("pixButton", xmPushButtonWidgetClass,
                             retval->pulldown,
	                     XmNlabelType, XmPIXMAP,
	                     XmNlabelPixmap, ptmp,
	                     NULL);
    }
    for (i = 0; i < nchoices; i++) {
        XtManageChild(retval->options[i].widget);
    }

    retval->menu = XmCreateOptionMenu(parent, "optionMenu", NULL, 0);
    str = XmStringCreateLocalized(s);
    XtVaSetValues(retval->menu,
		  XmNlabelString, str,
		  XmNsubMenuId, retval->pulldown,
		  NULL);
    XmStringFree(str);
    XtManageChild(retval->menu);
    
    fontid = xmalloc(SIZEOF_INT);
    *fontid = -1;
    SetUserData(retval->menu, fontid);
    
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

    errmsg("Value not found in SetOptionChoice()");
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

typedef struct {
    OptionStructure *opt;
    OC_CBProc cbproc;
    void *anydata;
} OC_CBdata;

typedef struct {
    SpinStructure *spin;
    Spin_CBProc cbproc;
    void *anydata;
    XtIntervalId timeout_id;
} Spin_CBdata;

typedef struct {
    Widget scale;
    Scale_CBProc cbproc;
    void *anydata;
} Scale_CBdata;

static void oc_int_cb_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int value;
    
    OC_CBdata *cbdata = (OC_CBdata *) client_data;

    value = GetOptionChoice(cbdata->opt);
    cbdata->cbproc(cbdata->opt, value, cbdata->anydata);
}

void AddOptionChoiceCB(OptionStructure *opt, OC_CBProc cbproc, void *anydata)
{
    OC_CBdata *cbdata;
    int i;
    
    cbdata = xmalloc(sizeof(OC_CBdata));
    
    cbdata->opt = opt;
    cbdata->cbproc = cbproc;
    cbdata->anydata = anydata;
    for (i = 0; i < opt->nchoices; i++) {
        XtAddCallback(opt->options[i].widget, XmNactivateCallback, 
                                    oc_int_cb_proc, (XtPointer) cbdata);
    }
}


static char list_translation_table[] = "\
    Ctrl<Key>A: list_selectall_action()\n\
    Ctrl<Key>U: list_unselectall_action()\n\
    Ctrl<Key>I: list_invertselection_action()";

static void list_selectall(Widget list)
{
    XtCallActionProc(list, "ListKbdSelectAll", NULL, NULL, 0);
}

static void list_unselectall(Widget list)
{
    XmListDeselectAllItems(list);
}

static void list_invertselection(Widget list)
{
    X11Stuff *xstuff = grace->gui->xstuff;
    int i, n;
    unsigned char selection_type_save;
    
    XtVaGetValues(list,
        XmNselectionPolicy, &selection_type_save,
        XmNitemCount, &n,
        NULL);
    if (selection_type_save == XmSINGLE_SELECT) {
        XBell(xstuff->disp, 50);
        return;
    }
    
    XtVaSetValues(list, XmNselectionPolicy, XmMULTIPLE_SELECT, NULL);
    for (i = 0; i < n; i++) {
        XmListSelectPos(list, i, False);
    }
    XtVaSetValues(list, XmNselectionPolicy, selection_type_save, NULL);
}

static int list_get_selected_count(Widget list)
{
    int n;
#if XmVersion < 2000
    int *selected;
    if (XmListGetSelectedPos(list, &selected, &n) != True) {
        n = 0;
    } else {
        XFree(selected);
    }
#else
    XtVaGetValues(list, XmNselectedPositionCount, &n, NULL);
#endif    
    return n;
}

void list_selectall_action(Widget w, XEvent *e, String *par, Cardinal *npar)
{
    list_selectall(w);
}

void list_unselectall_action(Widget w, XEvent *e, String *par, Cardinal *npar)
{
    list_unselectall(w);
}

void list_invertselection_action(Widget w, XEvent *e, String *par,
				 Cardinal *npar)
{
    list_invertselection(w);
}

ListStructure *CreateListChoice(Widget parent, char *labelstr, int type,
                                int nvisible, int nchoices, OptionItem *items)
{
    Arg args[4];
    Widget lab;
    ListStructure *retval;

    retval = xmalloc(sizeof(ListStructure));
    retval->rc = XmCreateRowColumn(parent, "rcList", NULL, 0);
    AddHelpCB(retval->rc, "doc/UsersGuide.html#list-selector");

    lab = XmCreateLabel(retval->rc, labelstr, NULL, 0);
    XtManageChild(lab);
    
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
	str = XmStringCreateLocalized(items[i].label);
        XmListAddItemUnselected(listp->list, str, 0);
        XmStringFree(str);
    }
    SelectListChoices(listp, nsel, selvalues);
    if (nsel > 0) {
        xfree(selvalues);
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
        
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
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
    
    if (nchoices > 0) {
        /* Rewind list so the last choice is always visible */
        XtVaGetValues(listp->list, XmNtopItemPosition, &bottom,
                                 XmNvisibleItemCount, &visible,
                                 NULL);
        if (i > bottom) {
            XmListSetBottomPos(listp->list, i);
        } else if (i <= bottom - visible) {
            XmListSetPos(listp->list, i);
        }
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
        retval = RETURN_SUCCESS;
    } else {
        retval = RETURN_FAILURE;
    }
    if (n > 0) {
        xfree(values);
    }
    return retval;
}

typedef struct {
    ListStructure *listp;
    List_CBProc cbproc;
    void *anydata;
} List_CBdata;

static void list_int_cb_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int n, *values;
    List_CBdata *cbdata = (List_CBdata *) client_data;
 
    n = GetListChoices(cbdata->listp, &values);
    
    cbdata->cbproc(cbdata->listp, n, values, cbdata->anydata);

    if (n > 0) {
        xfree(values);
    }
}

void AddListChoiceCB(ListStructure *listp, List_CBProc cbproc, void *anydata)
{
    List_CBdata *cbdata;
    
    cbdata = xmalloc(sizeof(List_CBdata));
    cbdata->listp = listp;
    cbdata->cbproc = (List_CBProc) cbproc;
    cbdata->anydata = anydata;
    XtAddCallback(listp->list,
        XmNsingleSelectionCallback,   list_int_cb_proc, (XtPointer) cbdata);
    XtAddCallback(listp->list,
        XmNmultipleSelectionCallback, list_int_cb_proc, (XtPointer) cbdata);
    XtAddCallback(listp->list,
        XmNextendedSelectionCallback, list_int_cb_proc, (XtPointer) cbdata);
}



#define SS_HIDE_CB           0
#define SS_SHOW_CB           1
#define SS_DELETE_CB         2
#define SS_DUPLICATE_CB      3
#define SS_BRING_TO_FRONT_CB 4
#define SS_SEND_TO_BACK_CB   5
#define SS_MOVE_UP_CB        6
#define SS_MOVE_DOWN_CB      7

static char *default_storage_labeling_proc(Quark *q, unsigned int *rid)
{
    char buf[128];
    
    sprintf(buf, "Quark \"%s\"", QIDSTR(q));
    
    (*rid)++;
    
    return copy_string(NULL, buf);
}

typedef struct {
    StorageStructure *ss;
    unsigned int rid;
} storage_traverse_data;

static int traverse_hook(Quark *q, void *udata, QTraverseClosure *closure)
{
    char *s;
    storage_traverse_data *stdata = (storage_traverse_data *) udata;
    StorageStructure *ss = stdata->ss;
    
    s = ss->labeling_proc(q, &stdata->rid);
    if (s) {
        char buf[16], *sbuf;
        XmString str;
        
        ss->values[ss->nchoices++] = q;

        sprintf(buf, "(%c) ", quark_is_active(q) ? '+':'-');
        sbuf = copy_string(NULL, buf);
        sbuf = concat_strings(sbuf, s);
        xfree(s);

        str = XmStringCreateLocalized(sbuf);
        xfree(sbuf);

        XmListAddItemUnselected(ss->list, str, 0);
        XmStringFree(str);
    }
    
    return TRUE;
}

static void storage_popup(Widget parent,
    XtPointer closure, XEvent *event, Boolean *cont)
{
    XButtonPressedEvent *e = (XButtonPressedEvent *) event;
    StorageStructure *ss = (StorageStructure *) closure;
    Widget popup = ss->popup;
    int n, selected;
    
    if (e->button != 3) {
        return;
    }
    
    /* don't post menu if governor selection isn't single */
    if (ss->governor && list_get_selected_count(ss->governor->list) != 1) {
        return;
    }
    
    n = list_get_selected_count(ss->list);
    
    if (n != 0) {
        selected = TRUE;
    } else {
        selected = FALSE;
    }
    
    SetSensitive(ss->popup_hide_bt, selected);
    SetSensitive(ss->popup_show_bt, selected);
    SetSensitive(ss->popup_delete_bt, selected);
    SetSensitive(ss->popup_duplicate_bt, selected);
    SetSensitive(ss->popup_bring_to_front_bt, selected);
    SetSensitive(ss->popup_send_to_back_bt, selected);
    SetSensitive(ss->popup_move_up_bt, selected);
    SetSensitive(ss->popup_move_down_bt, selected);
    
    if (ss->popup_cb) {
        ss->popup_cb(ss, n);
    }
    
    XmMenuPosition(popup, e);
    XtManageChild(popup);
}

static void ss_any_cb(StorageStructure *ss, int type)
{
    int i, n;
    Quark **values;
    
    n = GetStorageChoices(ss, &values);
    
    for (i = 0; i < n; i ++) {
        Quark *q;
        
        switch (type) {
        case SS_SEND_TO_BACK_CB:
        case SS_MOVE_UP_CB:
            q = values[n - i - 1];
            break;
        default:
            q = values[i];
            break;
        }
        
        switch (type) {
        case SS_HIDE_CB:
            quark_set_active(q, FALSE);
            break;
        case SS_SHOW_CB:
            quark_set_active(q, TRUE);
            break;
        case SS_DELETE_CB:
            quark_free(q);
            break;
        case SS_BRING_TO_FRONT_CB:
            quark_push(q, TRUE);
            break;
        case SS_SEND_TO_BACK_CB:
            quark_push(q, FALSE);
            break;
        case SS_MOVE_UP_CB:
            quark_move(q, TRUE);
            break;
        case SS_MOVE_DOWN_CB:
            quark_move(q, FALSE);
            break;
        case SS_DUPLICATE_CB:
            quark_copy(q);
            break;
        }
    }
    
    if (n > 0) {
        xfree(values);
        update_all();
        xdrawgraph(grace->project, FALSE);
    }
}

static void ss_hide_cb(Widget but, void *udata)
{
    ss_any_cb((StorageStructure *) udata, SS_HIDE_CB);
}

static void ss_show_cb(Widget but, void *udata)
{
    ss_any_cb((StorageStructure *) udata, SS_SHOW_CB);
}

static void ss_delete_cb(Widget but, void *udata)
{
    if (yesno("Really delete selected item(s)?", NULL, NULL, NULL)) {
        ss_any_cb((StorageStructure *) udata, SS_DELETE_CB);
    }
}

static void ss_duplicate_cb(Widget but, void *udata)
{
    ss_any_cb((StorageStructure *) udata, SS_DUPLICATE_CB);
}

static void ss_bring_to_front_cb(Widget but, void *udata)
{
    ss_any_cb((StorageStructure *) udata, SS_BRING_TO_FRONT_CB);
}

static void ss_send_to_back_cb(Widget but, void *udata)
{
    ss_any_cb((StorageStructure *) udata, SS_SEND_TO_BACK_CB);
}

static void ss_move_up_cb(Widget but, void *udata)
{
    ss_any_cb((StorageStructure *) udata, SS_MOVE_UP_CB);
}

static void ss_move_down_cb(Widget but, void *udata)
{
    ss_any_cb((StorageStructure *) udata, SS_MOVE_DOWN_CB);
}

static void ss_select_all_cb(Widget but, void *udata)
{
    list_selectall(((StorageStructure *) udata)->list);
}

static void ss_unselect_all_cb(Widget but, void *udata)
{
    list_unselectall(((StorageStructure *) udata)->list);
}

static void ss_invert_selection_cb(Widget but, void *udata)
{
    list_invertselection(((StorageStructure *) udata)->list);
}

static void ss_update_cb(Widget but, void *udata)
{
    StorageStructure *ss = (StorageStructure *) udata;
    
    UpdateStorageChoice(ss);
}

static void CreateStorageChoicePopup(StorageStructure *ss)
{
    Widget popup, submenupane;
    
    popup = XmCreatePopupMenu(ss->list, "popupMenu", NULL, 0);
    ss->popup = popup;
    XtVaSetValues(popup, XmNpopupEnabled, XmPOPUP_DISABLED, NULL);

    ss->popup_hide_bt = CreateMenuButton(popup, "Hide", '\0', ss_hide_cb, ss);
    ss->popup_show_bt = CreateMenuButton(popup, "Show", '\0', ss_show_cb, ss);
    CreateMenuSeparator(popup);
    
    ss->popup_delete_bt =
        CreateMenuButton(popup, "Delete", '\0', ss_delete_cb, ss);
    ss->popup_duplicate_bt =
        CreateMenuButton(popup, "Duplicate", '\0', ss_duplicate_cb, ss);
    
    CreateMenuSeparator(popup);
    
    ss->popup_bring_to_front_bt = CreateMenuButton(popup,
        "Bring to front", '\0', ss_bring_to_front_cb, ss);
    ss->popup_move_up_bt =
        CreateMenuButton(popup, "Move up", '\0', ss_move_up_cb, ss);
    ss->popup_move_down_bt =
        CreateMenuButton(popup, "Move down", '\0', ss_move_down_cb, ss);
    ss->popup_send_to_back_bt =
        CreateMenuButton(popup, "Send to back", '\0', ss_send_to_back_cb, ss);

    CreateMenuSeparator(popup);

    submenupane = CreateMenu(popup, "Selector operations", 'o', FALSE);
    ss->selpopup = submenupane;
    
    ss->popup_select_all_bt =
        CreateMenuButton(submenupane, "Select all", '\0', ss_select_all_cb, ss);
    ss->popup_unselect_all_bt =
        CreateMenuButton(submenupane, "Unselect all", '\0', ss_unselect_all_cb, ss);
    ss->popup_invert_selection_bt = CreateMenuButton(submenupane,
        "Invert selection", '\0', ss_invert_selection_cb, ss);
    
    CreateMenuSeparator(submenupane);

    CreateMenuButton(submenupane, "Update list", '\0', ss_update_cb, ss);
}

StorageStructure *CreateStorageChoice(Widget parent,
    char *labelstr, int type, int nvisible)
{
    Arg args[4];
    Widget lab;
    StorageStructure *retval;

    retval = xmalloc(sizeof(StorageStructure));
    memset(retval, 0, sizeof(StorageStructure));
    
    retval->labeling_proc = default_storage_labeling_proc;
    retval->rc = XmCreateRowColumn(parent, "rc", NULL, 0);
    AddHelpCB(retval->rc, "doc/UsersGuide.html#list-selector");

    lab = XmCreateLabel(retval->rc, labelstr, NULL, 0);
    XtManageChild(lab);
    
    XtSetArg(args[0], XmNlistSizePolicy, XmCONSTANT);
    XtSetArg(args[1], XmNscrollBarDisplayPolicy, XmSTATIC);
    if (type == LIST_TYPE_SINGLE) {
        XtSetArg(args[2], XmNselectionPolicy, XmSINGLE_SELECT);
    } else {
        XtSetArg(args[2], XmNselectionPolicy, XmEXTENDED_SELECT);
    }
    XtSetArg(args[3], XmNvisibleItemCount, nvisible);
    retval->list = XmCreateScrolledList(retval->rc, "list", args, 4);
    retval->values = NULL;

    CreateStorageChoicePopup(retval);
    XtAddEventHandler(retval->list,
        ButtonPressMask, False, storage_popup, retval);

    XtOverrideTranslations(retval->list, 
                             XtParseTranslationTable(list_translation_table));

    XtManageChild(retval->list);
    
    XtManageChild(retval->rc);
    
    return retval;
}

void SetStorageChoiceLabeling(StorageStructure *ss, Storage_LabelingProc proc)
{
    ss->labeling_proc = proc;
}

int GetStorageChoices(StorageStructure *ss, Quark ***values)
{
    int i, n;
    int *selected;
    
    if (XmListGetSelectedPos(ss->list, &selected, &n) != True) {
        return 0;
    }
    
    *values = xmalloc(n*SIZEOF_VOID_P);
    for (i = 0; i < n; i++) {
        (*values)[i] = ss->values[selected[i] - 1];
    }
    
    if (n) {
        xfree(selected);
    }
    
    return n;
}

int GetSingleStorageChoice(StorageStructure *ss, Quark **value)
{
    int n, retval;
    Quark **values;
    
    n = GetStorageChoices(ss, &values);
    if (n == 1) {
        *value = values[0];
        retval = RETURN_SUCCESS;
    } else {
        retval = RETURN_FAILURE;
    }
    
    if (n) {
        xfree(values);
    }
    
    return retval;
}


int SelectStorageChoices(StorageStructure *ss, int nchoices, Quark **choices)
{
    int i = 0, j;
    unsigned char selection_type_save;
    int bottom, visible;
    
    XtVaGetValues(ss->list, XmNselectionPolicy, &selection_type_save, NULL);
    XtVaSetValues(ss->list, XmNselectionPolicy, XmMULTIPLE_SELECT, NULL);
                             
    XmListDeselectAllItems(ss->list);
    for (j = 0; j < nchoices; j++) {
        i = 0;
        while (i < ss->nchoices && ss->values[i] != choices[j]) {
            i++;
        }
        if (i < ss->nchoices) {
            i++;
            XmListSelectPos(ss->list, i, True);
        }
    }
    
    if (nchoices > 0) {
        /* Rewind list so the last choice is always visible */
        XtVaGetValues(ss->list, XmNtopItemPosition, &bottom,
                                XmNvisibleItemCount, &visible,
                                NULL);
        if (i > bottom) {
            XmListSetBottomPos(ss->list, i);
        } else if (i <= bottom - visible) {
            XmListSetPos(ss->list, i);
        }
    }

    XtVaSetValues(ss->list, XmNselectionPolicy, selection_type_save, NULL);
    
    return RETURN_SUCCESS;
}

int SelectStorageChoice(StorageStructure *ss, Quark *choice)
{
    return SelectStorageChoices(ss, 1, &choice);
}

void UpdateStorageChoice(StorageStructure *ss)
{
    Quark **selvalues;
    storage_traverse_data stdata;
    int nsel;

    nsel = GetStorageChoices(ss, &selvalues);

    XmListDeleteAllItems(ss->list);
    
    ss->nchoices = 0;
    stdata.rid = 0;
    stdata.ss = ss;
    if (ss->q) {
        ss->values = xrealloc(ss->values, SIZEOF_VOID_P*quark_count_children(ss->q));
        quark_traverse(ss->q, traverse_hook, &stdata);

        SelectStorageChoices(ss, nsel, selvalues);
    }
    
    if (nsel > 0) {
        xfree(selvalues);
    }
}   

void SetStorageChoiceQuark(StorageStructure *ss, Quark *q)
{
    ss->q = q;
    UpdateStorageChoice(ss);
}   


typedef struct {
    StorageStructure *ss;
    Storage_CBProc cbproc;
    void *anydata;
} Storage_CBdata;

typedef struct {
    StorageStructure *ss;
    Storage_DCCBProc cbproc;
    void *anydata;
} Storage_DCCBdata;

static void storage_int_cb_proc(Widget w,
    XtPointer client_data, XtPointer call_data)
{
    int n;
    Quark **values;
    Storage_CBdata *cbdata = (Storage_CBdata *) client_data;
 
    n = GetStorageChoices(cbdata->ss, &values);
    
    cbdata->cbproc(cbdata->ss, n, values, cbdata->anydata);

    if (n > 0) {
        xfree(values);
    }
}

void AddStorageChoiceCB(StorageStructure *ss,
    Storage_CBProc cbproc, void *anydata)
{
    Storage_CBdata *cbdata;
    
    cbdata = xmalloc(sizeof(Storage_CBdata));
    cbdata->ss = ss;
    cbdata->cbproc = cbproc;
    cbdata->anydata = anydata;
    XtAddCallback(ss->list,
        XmNsingleSelectionCallback,   storage_int_cb_proc, (XtPointer) cbdata);
    XtAddCallback(ss->list,
        XmNmultipleSelectionCallback, storage_int_cb_proc, (XtPointer) cbdata);
    XtAddCallback(ss->list,
        XmNextendedSelectionCallback, storage_int_cb_proc, (XtPointer) cbdata);
}

static void storage_int_dc_cb_proc(Widget w,
    XtPointer client_data, XtPointer call_data)
{
    void *value;
    XmListCallbackStruct *cbs = (XmListCallbackStruct *) call_data;
    Storage_DCCBdata *cbdata = (Storage_DCCBdata *) client_data;
 
    value = cbdata->ss->values[cbs->item_position - 1];
    
    cbdata->cbproc(cbdata->ss, value, cbdata->anydata);
}

void AddStorageChoiceDblClickCB(StorageStructure *ss,
    Storage_DCCBProc cbproc, void *anydata)
{
    Storage_DCCBdata *cbdata;
    
    cbdata = xmalloc(sizeof(Storage_DCCBdata));
    cbdata->ss = ss;
    cbdata->cbproc = cbproc;
    cbdata->anydata = anydata;
    XtAddCallback(ss->list,
        XmNdefaultActionCallback, storage_int_dc_cb_proc, (XtPointer) cbdata);
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

static void sp_double_cb_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    Spin_CBdata *cbdata = (Spin_CBdata *) client_data;
    XmAnyCallbackStruct* xmcb = call_data;

    if (w == cbdata->spin->arrow_up   ||
        w == cbdata->spin->arrow_down ||
        xmcb->reason == XmCR_ACTIVATE) {
        cbdata->cbproc(cbdata->spin, GetSpinChoice(cbdata->spin), cbdata->anydata);
    }
}

static void sp_timer_proc(XtPointer client_data, XtIntervalId *id)
{
    Spin_CBdata *cbdata = (Spin_CBdata *) client_data;

    cbdata->cbproc(cbdata->spin, GetSpinChoice(cbdata->spin), cbdata->anydata);
    cbdata->timeout_id = (XtIntervalId) 0;
}

static void sp_ev_proc(Widget w,
    XtPointer client_data, XEvent *event, Boolean *cont)
{
    XButtonPressedEvent *e = (XButtonPressedEvent *) event;
    Spin_CBdata *cbdata = (Spin_CBdata *) client_data;

    if (e->button == 4 || e->button == 5) {
        /* we count elapsed time since the last event, so first remove
           an existing timeout, if there is one */
        if (cbdata->timeout_id) {
            XtRemoveTimeOut(cbdata->timeout_id);
        }
        cbdata->timeout_id = XtAppAddTimeOut(app_con,
            250 /* 0.25 second */, sp_timer_proc, client_data);
    }
}

void AddSpinChoiceCB(SpinStructure *spinp, Spin_CBProc cbproc, void *anydata)
{
    Spin_CBdata *cbdata;
    
    cbdata = xmalloc(sizeof(Spin_CBdata));
    
    cbdata->spin = spinp;
    cbdata->cbproc = cbproc;
    cbdata->anydata = anydata;
    cbdata->timeout_id = (XtIntervalId) 0;
    XtAddCallback(spinp->text,
        XmNactivateCallback, sp_double_cb_proc, (XtPointer) cbdata);
    XtAddCallback(spinp->arrow_up,
        XmNactivateCallback, sp_double_cb_proc, (XtPointer) cbdata);
    XtAddCallback(spinp->arrow_down,
        XmNactivateCallback, sp_double_cb_proc, (XtPointer) cbdata);
    XtAddEventHandler(spinp->text,
        ButtonPressMask, False, sp_ev_proc, (XtPointer) cbdata);
}

static void spin_updown(Widget parent,
    XtPointer closure, XEvent *event, Boolean *cont)
{
    XButtonPressedEvent *e = (XButtonPressedEvent *) event;
    SpinStructure *spinp = (SpinStructure *) closure;
    double value, incr;
    
    if (e->button == 4) {
        incr =  spinp->incr;
    } else
    if (e->button == 5) {
        incr = -spinp->incr;
    } else {
        return;
    }
    value = GetSpinChoice(spinp) + incr;
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
    
    retval = xmalloc(sizeof(SpinStructure));
    
    retval->type = type;
    retval->min = min;
    retval->max = max;
    retval->incr = incr;
    
    retval->rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, parent,
        XmNorientation, XmHORIZONTAL,
        NULL);
    str = XmStringCreateLocalized(s);
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

    XtAddEventHandler(retval->text, ButtonPressMask, False, spin_updown, retval);

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
    X11Stuff *xstuff = grace->gui->xstuff;
    char buf[64];
    
    if (value < spinp->min) {
        XBell(xstuff->disp, 50);
        value = spinp->min;
    } else if (value > spinp->max) {
        XBell(xstuff->disp, 50);
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


TextStructure *CreateTextInput(Widget parent, char *s)
{
    TextStructure *retval;
    XmString str;
    
    retval = xmalloc(sizeof(TextStructure));
    retval->form = XtVaCreateWidget("form", xmFormWidgetClass, parent, NULL);

    str = XmStringCreateLocalized(s);
    retval->label = XtVaCreateManagedWidget("label", 
        xmLabelWidgetClass, retval->form,
        XmNlabelString, str,
        XmNtopAttachment, XmATTACH_FORM,
        XmNbottomAttachment, XmATTACH_FORM,
        XmNleftAttachment, XmATTACH_FORM,
        XmNrightAttachment, XmATTACH_NONE,
        NULL);
    XmStringFree(str);

    retval->text = XtVaCreateManagedWidget("cstext",
        xmTextWidgetClass, retval->form,
        XmNtraversalOn, True,
        XmNtopAttachment, XmATTACH_FORM,
        XmNbottomAttachment, XmATTACH_FORM,
        XmNleftAttachment, XmATTACH_WIDGET,
        XmNleftWidget, retval->label,
        XmNrightAttachment, XmATTACH_FORM,
        NULL);

    XtManageChild(retval->form);

    return retval;
}

/* 
 * create a multiline editable window
 * parent = parent widget
 * nrows  = number of lines in the window
 * s      = label for window
 */
TextStructure *CreateScrolledTextInput(Widget parent, char *s, int nrows)
{
    TextStructure *retval;
    XmString str;
    Arg args[3];
    int ac;
	
    retval = xmalloc(sizeof(TextStructure));
    retval->form = XtVaCreateWidget("form", xmFormWidgetClass, parent, NULL);

    str = XmStringCreateLocalized(s);
    retval->label = XtVaCreateManagedWidget("label",
        xmLabelWidgetClass, retval->form,
	XmNlabelString, str,
	XmNtopAttachment, XmATTACH_FORM,
	XmNleftAttachment, XmATTACH_FORM,
	XmNrightAttachment, XmATTACH_FORM,
	NULL);
    XmStringFree(str);

    ac = 0;
    if (nrows > 0) {
        XtSetArg(args[ac], XmNrows, nrows); ac++;
    }
    XtSetArg(args[ac], XmNeditMode, XmMULTI_LINE_EDIT); ac++;
    XtSetArg(args[ac], XmNvisualPolicy, XmVARIABLE); ac++;
    retval->text = XmCreateScrolledText(retval->form, "text", args, ac);
    XtVaSetValues(XtParent(retval->text),
	XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, retval->label,
	XmNleftAttachment, XmATTACH_FORM,
	XmNrightAttachment, XmATTACH_FORM,
	XmNbottomAttachment, XmATTACH_FORM,
	NULL);
    XtManageChild(retval->text);
    
    XtManageChild(retval->form);
    return retval;
}

void SetTextInputLength(TextStructure *cst, int len)
{
    XtVaSetValues(cst->text, XmNcolumns, len, NULL);
}

void cstext_edit_action(Widget w, XEvent *e, String *par, Cardinal *npar)
{
    TextStructure *cst = (TextStructure *) GetUserData(w);
    create_fonttool(cst);
}

static char cstext_translation_table[] = "\
    Ctrl<Key>E: cstext_edit_action()";

TextStructure *CreateCSText(Widget parent, char *s)
{
    TextStructure *retval;

    retval = CreateTextInput(parent, s);
    SetUserData(retval->text, retval);
    XtOverrideTranslations(retval->text, 
        XtParseTranslationTable(cstext_translation_table));
        
    return retval;
}

TextStructure *CreateScrolledCSText(Widget parent, char *s, int nrows)
{
    TextStructure *retval;

    retval = CreateScrolledTextInput(parent, s, nrows);
    SetUserData(retval->text, retval);
    XtOverrideTranslations(retval->text, 
        XtParseTranslationTable(cstext_translation_table));
        
    return retval;
}

char *GetTextString(TextStructure *cst)
{
    char *s, *buf;
    
    s = XmTextGetString(cst->text);
    buf = copy_string(NULL, s);
    XtFree(s);
    
    return buf;
}

void SetTextString(TextStructure *cst, char *s)
{
    cst->locked = TRUE;

    XmTextSetString(cst->text, s ? s : "");
    XmTextSetInsertionPosition(cst->text, s ? strlen(s):0);
}

typedef struct {
    TextStructure *cst;
    Text_CBProc cbproc;
    void *anydata;
    Widget w;
    XtIntervalId timeout_id;
} Text_CBdata;

static void text_timer_proc(XtPointer client_data, XtIntervalId *id)
{
    char *s;
    Text_CBdata *cbdata = (Text_CBdata *) client_data;

    s = XmTextGetString(cbdata->w);
    cbdata->cbproc(cbdata->cst, s, cbdata->anydata);
    XtFree(s);
    cbdata->timeout_id = (XtIntervalId) 0;
}

/* Text input timeout [ms] */
#define TEXT_TIMEOUT    0

static void text_int_mv_cb_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    Text_CBdata *cbdata = (Text_CBdata *) client_data;

    if (cbdata->cst->locked) {
        cbdata->cst->locked = FALSE;
        return;
    }
    cbdata->w = w;
    /* we count elapsed time since the last event, so first remove
       an existing timeout, if there is one */
    if (cbdata->timeout_id) {
        XtRemoveTimeOut(cbdata->timeout_id);
    }
    
    if (TEXT_TIMEOUT) {
        cbdata->timeout_id = XtAppAddTimeOut(app_con,
            TEXT_TIMEOUT, text_timer_proc, client_data);
    }
}

static void text_int_cb_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    char *s;
    Text_CBdata *cbdata = (Text_CBdata *) client_data;
    s = XmTextGetString(w);
    cbdata->cbproc(cbdata->cst, s, cbdata->anydata);
    XtFree(s);
}

void AddTextInputCB(TextStructure *cst, Text_CBProc cbproc, void *data)
{
    Text_CBdata *cbdata;
    
    cbdata = xmalloc(sizeof(Text_CBdata));
    cbdata->cst = cst;
    cbdata->anydata = data;
    cbdata->cbproc = cbproc;
    cbdata->timeout_id = (XtIntervalId) 0;
    cbdata->cst->locked = FALSE;
    
    XtAddCallback(cst->text,
        XmNactivateCallback, text_int_cb_proc, (XtPointer) cbdata);
    XtAddCallback(cst->text,
        XmNmodifyVerifyCallback, text_int_mv_cb_proc, (XtPointer) cbdata);
}

int GetTextCursorPos(TextStructure *cst)
{
    return XmTextGetInsertionPosition(cst->text);
}

void SetTextCursorPos(TextStructure *cst, int pos)
{
    XmTextSetInsertionPosition(cst->text, pos);
}

void TextInsert(TextStructure *cst, int pos, char *s)
{
    XmTextInsert(cst->text, pos, s);
}

void SetTextEditable(TextStructure *cst, int onoff)
{
    XtVaSetValues(cst->text, XmNeditable, onoff? True:False, NULL);
}

typedef struct {
    Widget but;
    Button_CBProc cbproc;
    void *anydata;
} Button_CBdata;

Widget CreateButton(Widget parent, char *label)
{
    Widget button;
    XmString xmstr;
    
    xmstr = XmStringCreateLocalized(label);
    button = XtVaCreateManagedWidget("button",
        xmPushButtonWidgetClass, parent, 
        XmNalignment, XmALIGNMENT_CENTER,
    	XmNlabelString, xmstr,
/*
 *         XmNmarginLeft, 5,
 *         XmNmarginRight, 5,
 *         XmNmarginTop, 3,
 *         XmNmarginBottom, 2,
 */
    	NULL);
    XmStringFree(xmstr);

    return button;
}

Widget CreateBitmapButton(Widget parent,
    int width, int height, const unsigned char *bits)
{
    X11Stuff *xstuff = grace->gui->xstuff;
    Widget button;
    Pixmap pm;
    Pixel fg, bg;

    button = XtVaCreateManagedWidget("button",
        xmPushButtonWidgetClass, parent, 
	XmNlabelType, XmPIXMAP,
    	NULL);
    
/*
 * We need to get right fore- and background colors for pixmap.
 */    
    XtVaGetValues(button,
		  XmNforeground, &fg,
		  XmNbackground, &bg,
		  NULL);
    pm = XCreatePixmapFromBitmapData(xstuff->disp,
        xstuff->root, (char *) bits, width, height, fg, bg, xstuff->depth);
    XtVaSetValues(button, XmNlabelPixmap, pm, NULL);

    return button;
}

static void button_int_cb_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    Button_CBdata *cbdata = (Button_CBdata *) client_data;
    cbdata->cbproc(cbdata->but, cbdata->anydata);
}

void AddButtonCB(Widget button, Button_CBProc cbproc, void *data)
{
    Button_CBdata *cbdata;
    
    cbdata = xmalloc(sizeof(Button_CBdata));
    cbdata->but = button;
    cbdata->anydata = data;
    cbdata->cbproc = cbproc;
    XtAddCallback(button,
        XmNactivateCallback, button_int_cb_proc, (XtPointer) cbdata);
}

static void fsb_setcwd_cb(Widget but, void *data)
{
    char *bufp;
    XmString directory;
    Widget fsb = (Widget) data;
    
    XtVaGetValues(fsb, XmNdirectory, &directory, NULL);
    bufp = GetStringSimple(directory);
    XmStringFree(directory);
    if (bufp != NULL) {
        set_workingdir(grace, bufp);
        XtFree(bufp);
    }
}

#define FSB_CWD     0
#define FSB_HOME    1
#define FSB_ROOT    2
#define FSB_CYGDRV  3

static void fsb_cd_cb(OptionStructure *opt, int value, void *data)
{
    char *bufp;
    XmString dir, pattern, dirmask;
    Widget FSB = (Widget) data;
    
    switch (value) {
    case FSB_CWD:
        bufp = get_workingdir(grace);
        break;
    case FSB_HOME:
	bufp = get_userhome(grace);
        break;
    case FSB_ROOT:
        bufp = "/";
        break;
    case FSB_CYGDRV:
        bufp = "/cygdrive/";
        break;
    default:
        return;
    }
    
    XtVaGetValues(FSB, XmNpattern, &pattern, NULL);
    
    dir = XmStringCreateLocalized(bufp);
    dirmask = XmStringConcatAndFree(dir, pattern);

    XmFileSelectionDoSearch(FSB, dirmask);
    XmStringFree(dirmask);
}

static OptionItem fsb_items[] = {
    {FSB_CWD,  "Cwd"},
    {FSB_HOME, "Home"},
    {FSB_ROOT, "/"}
#ifdef __CYGWIN__
    ,{FSB_CYGDRV, "My Computer"}
#endif
};

#define FSB_ITEMS_NUM   sizeof(fsb_items)/sizeof(OptionItem)

#if XmVersion >= 2000    
static void show_hidden_cb(Widget but, int onoff, void *data)
{
    FSBStructure *fsb = (FSBStructure *) data;
    XtVaSetValues(fsb->FSB, XmNfileFilterStyle,
        onoff ? XmFILTER_NONE:XmFILTER_HIDDEN_FILES, NULL);
}
#endif

FSBStructure *CreateFileSelectionBox(Widget parent, char *s)
{
    FSBStructure *retval;
    OptionStructure *opt;
    Widget fr, form, button;
    XmString xmstr;
    char *bufp, *resname;
    
    retval = xmalloc(sizeof(FSBStructure));
    resname = label_to_resname(s, "FSB");
    retval->FSB = XmCreateFileSelectionDialog(parent, resname, NULL, 0);
    xfree(resname);
    retval->dialog = XtParent(retval->FSB);
    handle_close(retval->dialog);
    bufp = copy_string(NULL, "Grace: ");
    bufp = concat_strings(bufp, s);
    XtVaSetValues(retval->dialog, XmNtitle, bufp, NULL);
    xfree(bufp);
    
    xmstr = XmStringCreateLocalized(get_workingdir(grace));
    XtVaSetValues(retval->FSB, XmNdirectory, xmstr, NULL);
    XmStringFree(xmstr);
    
    XtAddCallback(retval->FSB,
        XmNcancelCallback, destroy_dialog, retval->dialog);
    AddHelpCB(retval->FSB, "doc/UsersGuide.html#FS-dialog");
    
    retval->rc = XmCreateRowColumn(retval->FSB, "rc", NULL, 0);
#if XmVersion >= 2000    
    button = CreateToggleButton(retval->rc, "Show hidden files");
    AddToggleButtonCB(button, show_hidden_cb, retval);
    XtVaSetValues(retval->FSB, XmNfileFilterStyle, XmFILTER_HIDDEN_FILES, NULL);
#endif
    fr = CreateFrame(retval->rc, NULL);
    form = XtVaCreateWidget("form", xmFormWidgetClass, fr, NULL);
    opt = CreateOptionChoice(form, "Chdir to:", 1, FSB_ITEMS_NUM, fsb_items);
    AddOptionChoiceCB(opt, fsb_cd_cb, (void *) retval->FSB);
    button = CreateButton(form, "Set as cwd");
    AddButtonCB(button, fsb_setcwd_cb, (void *) retval->FSB);

    XtVaSetValues(opt->menu,
        XmNleftAttachment, XmATTACH_FORM,
        XmNtopAttachment, XmATTACH_FORM,
        XmNbottomAttachment, XmATTACH_FORM,
        XmNrightAttachment, XmATTACH_NONE,
        NULL);
    XtVaSetValues(button,
        XmNleftAttachment, XmATTACH_NONE,
        XmNtopAttachment, XmATTACH_FORM,
        XmNbottomAttachment, XmATTACH_FORM,
        XmNrightAttachment, XmATTACH_FORM,
        NULL);
    XtManageChild(form);

    XtManageChild(retval->rc);
        
    return retval;
}

typedef struct {
    FSBStructure *fsb;
    FSB_CBProc cbproc;
    void *anydata;
} FSB_CBdata;

static void fsb_int_cb_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    char *s;
    int ok;
    
    FSB_CBdata *cbdata = (FSB_CBdata *) client_data;
    XmFileSelectionBoxCallbackStruct *cbs =
        (XmFileSelectionBoxCallbackStruct *) call_data;

    s = GetStringSimple(cbs->value);
    if (s == NULL) {
	errmsg("Error converting XmString to char string");
	return;
    }

    set_wait_cursor();

    ok = cbdata->cbproc(cbdata->fsb, s, cbdata->anydata);
    XtFree(s);
    if (ok) {
        XtUnmanageChild(cbdata->fsb->dialog);
    }
    unset_wait_cursor();
}

void AddFileSelectionBoxCB(FSBStructure *fsb, FSB_CBProc cbproc, void *anydata)
{
    FSB_CBdata *cbdata;
    
    cbdata = xmalloc(sizeof(FSB_CBdata));
    cbdata->fsb = fsb;
    cbdata->cbproc = (FSB_CBProc) cbproc;
    cbdata->anydata = anydata;
    XtAddCallback(fsb->FSB,
        XmNokCallback, fsb_int_cb_proc, (XtPointer) cbdata);
}

void SetFileSelectionBoxPattern(FSBStructure *fsb, char *pattern)
{
    XmString xmstr;
    
    if (pattern != NULL) {
        xmstr = XmStringCreateLocalized(pattern);
        XtVaSetValues(fsb->FSB, XmNpattern, xmstr, NULL);
        XmStringFree(xmstr);
    }
}

Widget CreateLabel(Widget parent, char *s)
{
    Widget label;
    
    label = XtVaCreateManagedWidget(s ? s:"",
        xmLabelWidgetClass, parent,
        XmNalignment, XmALIGNMENT_BEGINNING,
        XmNrecomputeSize, True,
        NULL);
    return label;
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

static OptionItem *settype_option_items;
static BitmapOptionItem *pattern_option_items;
static BitmapOptionItem *lines_option_items;

#define LINES_BM_HEIGHT 15
#define LINES_BM_WIDTH  64

static void init_xvlibcolors(void)
{
    X11Stuff *xstuff = grace->gui->xstuff;
    Project *pr = project_get_data(grace->project);
    unsigned int i;
    
    if (!pr) {
        return;
    }
    
    for (i = 0; i < pr->ncolors; i++) {
        long pixel;
        Colordef *c = &pr->colormap[i];
        
        pixel = x11_allocate_color(grace->gui, &c->rgb);
        
        if (pixel >= 0) {
            xvlibcolors[c->id] = pixel;
        } else {
            xvlibcolors[c->id] = BlackPixel(xstuff->disp, xstuff->screennumber);
        }
    }
}

int init_option_menus(void) {
    unsigned int i, j, k, l, n;
    
    init_xvlibcolors();
    
    n = number_of_patterns(canvas);
    if (n) {
        pattern_option_items = xmalloc(n*sizeof(BitmapOptionItem));
        if (pattern_option_items == NULL) {
            errmsg("Malloc error in init_option_menus()");
            return RETURN_FAILURE;
        }
        for (i = 0; i < n; i++) {
            pattern_option_items[i].value = i;
            if (i == 0) {
                pattern_option_items[i].bitmap = NULL;
            } else {
                Pattern *pat = canvas_get_pattern(canvas, i);
                pattern_option_items[i].bitmap = pat->bits;
            }
        }
    }
    
    n = number_of_linestyles(canvas);
    if (n) {
        lines_option_items = xmalloc(n*sizeof(BitmapOptionItem));
        if (lines_option_items == NULL) {
            errmsg("Malloc error in init_option_menus()");
            xfree(pattern_option_items);
            return RETURN_FAILURE;
        }
        for (i = 0; i < n; i++) {
            LineStyle *linestyle = canvas_get_linestyle(canvas, i);
            lines_option_items[i].value = i;
            if (i == 0) {
                lines_option_items[i].bitmap = NULL;
                continue;
            }

            lines_option_items[i].bitmap = 
                  xcalloc(LINES_BM_HEIGHT*LINES_BM_WIDTH/8/SIZEOF_CHAR, SIZEOF_CHAR);

            k = LINES_BM_WIDTH*(LINES_BM_HEIGHT/2);
            while (k < LINES_BM_WIDTH*(LINES_BM_HEIGHT/2 + 1)) {
                for (j = 0; j < linestyle->length; j++) {
                    for (l = 0; l < linestyle->array[j]; l++) {
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
    }

    settype_option_items = xmalloc(NUMBER_OF_SETTYPES*sizeof(OptionItem));
    if (settype_option_items == NULL) {
        errmsg("Malloc error in init_option_menus()");
        return RETURN_FAILURE;
    }
    for (i = 0; i < NUMBER_OF_SETTYPES; i++) {
        settype_option_items[i].value = i;
        settype_option_items[i].label = copy_string(NULL,
            set_type_descr(grace->rt, i));
    }

    return RETURN_SUCCESS;
}

static OptionItem *font_option_items = NULL;
static unsigned int nfont_option_items = 0;
static OptionStructure **font_selectors = NULL;
static unsigned int nfont_selectors = 0;

void update_font_selectors(void)
{
    unsigned int i;
    Project *pr = project_get_data(grace->project);
    
    nfont_option_items = pr->nfonts;
    font_option_items =
        xrealloc(font_option_items, nfont_option_items*sizeof(OptionItem));

    for (i = 0; i < nfont_option_items; i++) {
        Fontdef *f = &pr->fontmap[i];
        font_option_items[i].value = f->id;
        font_option_items[i].label = f->fontname;
    }
    
    for (i = 0; i < nfont_selectors; i++) {
        UpdateOptionChoice(font_selectors[i], 
                            nfont_option_items, font_option_items);
    }
}

OptionStructure *CreateFontChoice(Widget parent, char *s)
{
    OptionStructure *retvalp = NULL;

    nfont_selectors++;
    font_selectors = xrealloc(font_selectors, 
                                    nfont_selectors*sizeof(OptionStructure *));
    if (font_selectors == NULL) {
        errmsg("Malloc failed in CreateFontChoice()");
        return retvalp;
    }
    
    retvalp = CreateOptionChoice(parent, s, 0, 
                                nfont_option_items, font_option_items);

    font_selectors[nfont_selectors - 1] = retvalp;
    
    return retvalp;
}

OptionStructure *CreatePatternChoice(Widget parent, char *s)
{
    return (CreateBitmapOptionChoice(parent, s, 4, number_of_patterns(canvas), 
                                     16, 16, pattern_option_items));
}

OptionStructure *CreateLineStyleChoice(Widget parent, char *s)
{
    return (CreateBitmapOptionChoice(parent, s, 0, number_of_linestyles(canvas), 
                        LINES_BM_WIDTH, LINES_BM_HEIGHT, lines_option_items));
}

OptionStructure *CreateSetTypeChoice(Widget parent, char *s)
{
    return (CreateOptionChoice(parent,
        s, 0, NUMBER_OF_SETTYPES, settype_option_items));
}

static BitmapOptionItem text_just_option_items[12] =
{
    {JUST_LEFT  |JUST_BLINE , j_lm_o_bits},
    {JUST_CENTER|JUST_BLINE , j_cm_o_bits},
    {JUST_RIGHT |JUST_BLINE , j_rm_o_bits},
    {JUST_LEFT  |JUST_BOTTOM, j_lb_b_bits},
    {JUST_CENTER|JUST_BOTTOM, j_cb_b_bits},
    {JUST_RIGHT |JUST_BOTTOM, j_rb_b_bits},
    {JUST_LEFT  |JUST_MIDDLE, j_lm_b_bits},
    {JUST_CENTER|JUST_MIDDLE, j_cm_b_bits},
    {JUST_RIGHT |JUST_MIDDLE, j_rm_b_bits},
    {JUST_LEFT  |JUST_TOP   , j_lt_b_bits},
    {JUST_CENTER|JUST_TOP   , j_ct_b_bits},
    {JUST_RIGHT |JUST_TOP   , j_rt_b_bits}
};

OptionStructure *CreateTextJustChoice(Widget parent, char *s)
{
    return (CreateBitmapOptionChoice(parent, s, 4,
        12, JBITMAP_WIDTH, JBITMAP_HEIGHT, text_just_option_items));
}

static BitmapOptionItem just_option_items[9] =
{
    {JUST_LEFT  |JUST_BOTTOM, j_lb_b_bits},
    {JUST_CENTER|JUST_BOTTOM, j_cb_b_bits},
    {JUST_RIGHT |JUST_BOTTOM, j_rb_b_bits},
    {JUST_LEFT  |JUST_MIDDLE, j_lm_b_bits},
    {JUST_CENTER|JUST_MIDDLE, j_cm_b_bits},
    {JUST_RIGHT |JUST_MIDDLE, j_rm_b_bits},
    {JUST_LEFT  |JUST_TOP   , j_lt_b_bits},
    {JUST_CENTER|JUST_TOP   , j_ct_b_bits},
    {JUST_RIGHT |JUST_TOP   , j_rt_b_bits}
};

OptionStructure *CreateJustChoice(Widget parent, char *s)
{
    return (CreateBitmapOptionChoice(parent, s, 3,
        9, JBITMAP_WIDTH, JBITMAP_HEIGHT, just_option_items));
}

RestrictionStructure *CreateRestrictionChoice(Widget parent, char *s)
{
    RestrictionStructure *retval;
    Widget rc;
    OptionItem restr_items[7];

    restr_items[0].value = RESTRICT_NONE;
    restr_items[0].label = "None";
    restr_items[1].value = RESTRICT_REG0;
    restr_items[1].label = "Region 0";
    restr_items[2].value = RESTRICT_REG1;
    restr_items[2].label = "Region 1";
    restr_items[3].value = RESTRICT_REG2;
    restr_items[3].label = "Region 2";
    restr_items[4].value = RESTRICT_REG3;
    restr_items[4].label = "Region 3";
    restr_items[5].value = RESTRICT_REG4;
    restr_items[5].label = "Region 4";
    restr_items[6].value = RESTRICT_WORLD;
    restr_items[6].label = "Inside graph";

    retval = xmalloc(sizeof(RestrictionStructure));

    retval->frame = CreateFrame(parent, s);
    rc = XtVaCreateWidget("rc",
        xmRowColumnWidgetClass, retval->frame,
        XmNorientation, XmHORIZONTAL,
        NULL);

    retval->r_sel = CreateOptionChoice(rc,
        "Restriction:", 1, 7, restr_items);
    retval->negate = CreateToggleButton(rc, "Negated");
    XtManageChild(rc);

    return retval;
}

#define PEN_CHOICE_WIDTH  64
#define PEN_CHOICE_HEIGHT 16

typedef struct {
    Pen pen;
    Widget color_popup;
    Widget pattern_popup;
    
    Pen_CBProc cb_proc;
    void *cb_data;
} Button_PData;

static GC gc_pen;

static void SetPenChoice_int(Widget button, Pen *pen, int call_cb)
{
    X11Stuff *xstuff = grace->gui->xstuff;
    Pixel bg, fg;
    Pixmap pixtile, pixmap;
    Button_PData *pdata;
    Pattern *pat;

    /* Safety checks */
    if (!button || !pen ||
        (pen->pattern < 0 || pen->pattern >= number_of_patterns(canvas)) ||
        (pen->color < 0   || pen->color   >= number_of_colors(canvas))) {
        return;
    }
    
    pdata = GetUserData(button);
    pdata->pen = *pen;
    
    if (!gc_pen) {
        gc_pen = XCreateGC(xstuff->disp, xstuff->root, 0, NULL);
        XSetFillStyle(xstuff->disp, gc_pen, FillTiled);
    }
    
    fg = xvlibcolors[pen->color];
    bg = xvlibcolors[getbgcolor(canvas)];
    
    pat = canvas_get_pattern(canvas, pen->pattern);
    pixtile = XCreatePixmapFromBitmapData(xstuff->disp, xstuff->root, 
        (char *) pat->bits, pat->width, pat->height, fg, bg, xstuff->depth);

    XSetTile(xstuff->disp, gc_pen, pixtile);
    
    pixmap = XCreatePixmap(xstuff->disp, xstuff->root, PEN_CHOICE_WIDTH, PEN_CHOICE_HEIGHT, xstuff->depth);
    XFillRectangle(xstuff->disp, pixmap, gc_pen, 0, 0, PEN_CHOICE_WIDTH, PEN_CHOICE_HEIGHT);

    XtVaSetValues(button, XmNlabelPixmap, pixmap, NULL);
    
    XFreePixmap(xstuff->disp, pixtile);
    
    if (call_cb && pdata->cb_proc) {
        pdata->cb_proc(button, pen, pdata->cb_data);
    }
}

void SetPenChoice(Widget button, Pen *pen)
{
    SetPenChoice_int(button, pen, FALSE);
}

static void pen_popup(Widget parent,
    XtPointer closure, XEvent *event, Boolean *cont)
{
    XButtonPressedEvent *e = (XButtonPressedEvent *) event;
    Button_PData *pdata;
    Widget popup;
    
    if (e->button != 3) {
        return;
    }
    
    pdata = GetUserData(parent);
    
    if (e->state & ShiftMask) {
        popup = pdata->pattern_popup;
    } else {
        popup = pdata->color_popup;
    }
    
    XmMenuPosition(popup, e);
    XtManageChild(popup);
}

static void cc_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
    Pen pen;
    Widget button = GetParent(GetParent(GetParent(w)));
    Button_PData *pdata;
    
    pdata = GetUserData(button);
    pen = pdata->pen;
    pen.color = (int) client_data;
    
    SetPenChoice_int(button, &pen, TRUE);
}

static Widget CreateColorChoicePopup(Widget button)
{
    X11Stuff *xstuff = grace->gui->xstuff;
    Project *pr = project_get_data(grace->project);
    Widget popup;
    unsigned int ci;
    
    popup = XmCreatePopupMenu(button, "colorPopupMenu", NULL, 0);
    XtVaSetValues(popup,
                  XmNnumColumns, 4,
                  XmNpacking, XmPACK_COLUMN,
                  NULL);

    if (pr) {
        for (ci = 0; ci < pr->ncolors; ci++) {
            Colordef *c = &pr->colormap[ci];
            long bg, fg;
            Widget cb;
            int color = c->id;
            cb = CreateButton(popup, c->cname);
            XtAddCallback(cb,
                XmNactivateCallback, cc_cb, (XtPointer) color);

            bg = xvlibcolors[color];
	    if (get_rgb_intensity(&c->rgb) < 0.5) {
	        fg = WhitePixel(xstuff->disp, xstuff->screennumber);
	    } else {
	        fg = BlackPixel(xstuff->disp, xstuff->screennumber);
	    }
	    XtVaSetValues(cb, 
                XmNbackground, bg,
                XmNforeground, fg,
                NULL);
        }
    }
    
    return popup;
}

static Widget CreatePatternChoicePopup(Widget button)
{
    Widget popup;
    
    popup = XmCreatePopupMenu(button, "patternPopupMenu", NULL, 0);
    CreateMenuLabel(popup, "Pattern:");
    CreateMenuSeparator(popup);
    
    return popup;
}

typedef struct {
    Widget top;
    OptionStructure *color;
    OptionStructure *pattern;
    
    Widget pen_button;
} PenChoiceDialog;

static int pen_choice_aac(void *data)
{
    PenChoiceDialog *ui = (PenChoiceDialog *) data;
    
    if (ui->pen_button) {
        Pen pen;
        pen.color   = GetOptionChoice(ui->color);
        pen.pattern = GetOptionChoice(ui->pattern);
        
        SetPenChoice_int(ui->pen_button, &pen, TRUE);
    }
    
    return RETURN_SUCCESS;
}

static void define_pen_choice_dialog(Widget but, void *data)
{
    static PenChoiceDialog *ui = NULL;
    
    set_wait_cursor();
    
    if (!ui) {
        Widget fr, rc;

        ui = xmalloc(sizeof(PenChoiceDialog));
        
        ui->top = CreateDialogForm(app_shell, "Pen properties");
        
        fr = CreateFrame(ui->top, "Pen");
        rc = CreateVContainer(fr);
        ui->color   = CreateColorChoice(rc, "Color");
        ui->pattern = CreatePatternChoice(rc, "Pattern");
        
        CreateAACDialog(ui->top, fr, pen_choice_aac, ui);
    }
    
    ui->pen_button = (Widget) data;
    
    if (ui->pen_button) {
        Button_PData *pdata;
        Pen pen;
        
        pdata = GetUserData(ui->pen_button);
        
        pen = pdata->pen;
        
        SetOptionChoice(ui->color, pen.color);
        SetOptionChoice(ui->pattern, pen.pattern);
    }

    RaiseWindow(GetParent(ui->top));
    unset_wait_cursor();
}

Widget CreatePenChoice(Widget parent, char *s)
{
    Widget rc, button;
    Button_PData *pdata;
    Pen pen;
    
    pdata = xmalloc(sizeof(Button_PData));
    memset(pdata, 0, sizeof(Button_PData));
    
    rc = CreateHContainer(parent);
    CreateLabel(rc, s);
    button = XtVaCreateWidget("penButton",
        xmPushButtonWidgetClass, rc,
        XmNlabelType, XmPIXMAP,
        XmNuserData, pdata,
        NULL);
    
    AddHelpCB(button, "doc/UsersGuide.html#pen-chooser");

    AddButtonCB(button, define_pen_choice_dialog, button);
    
    pdata->color_popup   = CreateColorChoicePopup(button);
    pdata->pattern_popup = CreatePatternChoicePopup(button);
    
    XtAddEventHandler(button, ButtonPressMask, False, pen_popup, NULL);

    pen.color   = 0;
    pen.pattern = 0;
    SetPenChoice_int(button, &pen, FALSE);
    
    XtManageChild(button);
    
    return button;
}

int GetPenChoice(Widget pen_button, Pen *pen)
{
    Button_PData *pdata = GetUserData(pen_button);
    if (pdata) {
        *pen = pdata->pen;
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

void AddPenChoiceCB(Widget button, Pen_CBProc cbproc, void *anydata)
{
    Button_PData *pdata = GetUserData(button);
    
    if (pdata->cb_proc) {
        errmsg("AddPenChoiceCB: only one callback is supported");
    } else {
        pdata->cb_proc = cbproc;
        pdata->cb_data = anydata;
    }
}

SpinStructure *CreateViewCoordInput(Widget parent, char *s)
{
    return CreateSpinChoice(parent, s, 6, SPIN_TYPE_FLOAT, -10.0, 10.0, 0.05);
}

static StorageStructure **ssd_selectors = NULL;
static int nssd_selectors = 0;

static char *ssd_labeling(Quark *q, unsigned int *rid)
{
    char buf[128];
    
    if (quark_fid_get(q) == QFlavorSSD) {
        sprintf(buf, "SSD \"%s\" (%d x %d)", QIDSTR(q),
            ssd_get_ncols(q), ssd_get_nrows(q));

        (*rid)++;

        return copy_string(NULL, buf);
    } else {
        return NULL;
    }
}

StorageStructure *CreateSSDChoice(Widget parent, char *labelstr, int type)
{
    StorageStructure *ss;
    int nvisible;
    
    nvisible = (type == LIST_TYPE_SINGLE) ? 2 : 4; 

    ss = CreateStorageChoice(parent, labelstr, type, nvisible);
    SetStorageChoiceLabeling(ss, ssd_labeling);
    SetStorageChoiceQuark(ss, grace->project);

    nssd_selectors++;
    ssd_selectors =
        xrealloc(ssd_selectors, nssd_selectors*sizeof(StorageStructure *));
    ssd_selectors[nssd_selectors - 1] = ss;

    AddHelpCB(ss->rc, "doc/UsersGuide.html#ssd-selector");

    return ss;
}

void update_ssd_selectors(Quark *pr)
{
    int i;
    for (i = 0; i < nssd_selectors; i++) {
        StorageStructure *ss = ssd_selectors[i];
        if (!ss->q && pr) {
            ss->q = pr;
        } else 
        if (!pr) {
            ss->q = NULL;
        }
        UpdateStorageChoice(ss);
    }
}


static StorageStructure **graph_selectors = NULL;
static int ngraph_selectors = 0;

#define GSS_FOCUS_CB         0

typedef struct {
    Widget focus_bt;
} GSSData;

static void gss_any_cb(void *udata, int cbtype)
{
    StorageStructure *ss = (StorageStructure *) udata;
    int i, n;
    Quark **values;
    
    n = GetStorageChoices(ss, &values);
    
    for (i = 0; i < n; i ++) {
        Quark *gr = values[i];
        
        switch (cbtype) {
        case GSS_FOCUS_CB:
            switch_current_graph(gr);
            break;
        }
    }
    
    if (n > 0) {
        xfree(values);
        update_all();
        xdrawgraph(grace->project, FALSE);
    }
}

static void g_focus_cb(Widget but, void *udata)
{
    gss_any_cb(udata, GSS_FOCUS_CB);
}

static void g_popup_cb(StorageStructure *ss, int nselected)
{
    GSSData *gssdata = (GSSData *) ss->data;
    
    SetSensitive(gssdata->focus_bt, (nselected == 1));
}

static void g_new_cb(Widget but, void *udata)
{
    graph_next(grace->project);
    update_all();
    xdrawgraph(grace->project, FALSE);
}

static void g_dc_cb(StorageStructure *ss, Quark *gr, void *data)
{
    switch_current_graph(gr);
}

static char *graph_labeling(Quark *q, unsigned int *rid)
{
    char buf[128];
    
    if (quark_fid_get(q) == QFlavorGraph) {
        sprintf(buf, "Graph \"%s\" (type: %s, sets: %d)",
            QIDSTR(q),
            graph_types(grace->rt, graph_get_type(q)), quark_get_number_of_descendant_sets(q));

        (*rid)++;

        return copy_string(NULL, buf);
    } else {
        return NULL;
    }
}

StorageStructure *CreateGraphChoice(Widget parent, char *labelstr, int type)
{
    StorageStructure *ss;
    GSSData *gssdata;
    Widget popup;
    int nvisible;
    
    nvisible = (type == LIST_TYPE_SINGLE) ? 2 : 4; 
    ss = CreateStorageChoice(parent, labelstr, type, nvisible);
    SetStorageChoiceLabeling(ss, graph_labeling);
    SetStorageChoiceQuark(ss, grace->project);
    AddStorageChoiceDblClickCB(ss, g_dc_cb, NULL);
    AddHelpCB(ss->rc, "doc/UsersGuide.html#graph-selector");

    ngraph_selectors++;
    graph_selectors =
        xrealloc(graph_selectors, ngraph_selectors*sizeof(StorageStructure *));
    graph_selectors[ngraph_selectors - 1] = ss;
    
    gssdata = xmalloc(sizeof(GSSData));
    ss->data = gssdata;
    ss->popup_cb = g_popup_cb;
    
    popup = ss->popup;
    
    CreateMenuSeparator(popup);

    CreateMenuButton(popup, "Create new", '\0', g_new_cb, ss);
    
    CreateMenuSeparator(popup);

    gssdata->focus_bt =
        CreateMenuButton(popup, "Focus to selected", '\0', g_focus_cb, ss);
    
    return ss;
}

void update_graph_selectors(Quark *pr)
{
    int i;
    for (i = 0; i < ngraph_selectors; i++) {
        StorageStructure *ss = graph_selectors[i];
        if (!ss->q && pr) {
            ss->q = pr;
        } else 
        if (!pr) {
            ss->q = NULL;
        }
        UpdateStorageChoice(ss);
    }
}

void graph_set_selectors(Quark *gr)
{
    int i;
    
    for (i = 0; i < ngraph_selectors; i++) {
        SelectStorageChoice(graph_selectors[i], gr);
    }
}

static StorageStructure **frame_selectors = NULL;
static int nframe_selectors = 0;

static char *frame_labeling(Quark *q, unsigned int *rid)
{
    char buf[128];
    
    if (quark_fid_get(q) == QFlavorFrame) {
        sprintf(buf, "Frame \"%s\"", QIDSTR(q));

        (*rid)++;

        return copy_string(NULL, buf);
    } else {
        return NULL;
    }
}

StorageStructure *CreateFrameChoice(Widget parent, char *labelstr, int type)
{
    StorageStructure *ss;
    Widget popup;
    int nvisible;
    
    nvisible = (type == LIST_TYPE_SINGLE) ? 2 : 4; 
    ss = CreateStorageChoice(parent, labelstr, type, nvisible);
    SetStorageChoiceLabeling(ss, frame_labeling);
    SetStorageChoiceQuark(ss, grace->project);
    AddHelpCB(ss->rc, "doc/UsersGuide.html#frame-selector");

    nframe_selectors++;
    frame_selectors =
        xrealloc(frame_selectors, nframe_selectors*sizeof(StorageStructure *));
    frame_selectors[nframe_selectors - 1] = ss;
    
    popup = ss->popup;
    
    CreateMenuSeparator(popup);

    CreateMenuButton(popup, "Create new", '\0', g_new_cb, ss);
    
    CreateMenuSeparator(popup);

    return ss;
}

void update_frame_selectors(Quark *pr)
{
    int i;
    for (i = 0; i < nframe_selectors; i++) {
        StorageStructure *ss = frame_selectors[i];
        if (!ss->q && pr) {
            ss->q = pr;
        } else 
        if (!pr) {
            ss->q = NULL;
        }
        UpdateStorageChoice(ss);
    }
}

/* Set selectors */
static StorageStructure **set_selectors = NULL;
static int nset_selectors = 0;


#define SSS_EDITS_CB         0
#define SSS_EDITE_CB         1
#define SSS_NEWF_CB          2
#define SSS_NEWS_CB          3
#define SSS_NEWE_CB          4
#define SSS_NEWB_CB          5

typedef struct {
    StorageStructure *graphss;
    Widget edit_menu;
} SSSData;

Quark *get_set_choice_gr(StorageStructure *ss)
{
    Quark *gr;
    SSSData *sdata = (SSSData *) ss->data;
    
    if (sdata->graphss) {
        if (GetSingleStorageChoice(sdata->graphss, &gr) != RETURN_SUCCESS) {
            gr = NULL;
        }
    } else {
        gr = NULL;
    }
    
    return gr;
}

static void sss_any_cb(void *udata, int cbtype)
{
    StorageStructure *ss = (StorageStructure *) udata;
    Quark *gr = get_set_choice_gr(ss), *pset;
    int i, n;
    Quark **values;
    
    n = GetStorageChoices(ss, &values);
    
    for (i = 0; i < n; i ++) {
        Quark *pset = values[i];
        
        switch (cbtype) {
        case SSS_EDITS_CB:
            create_ss_frame(pset);
            break;
        case SSS_EDITE_CB:
            do_ext_editor(pset);
            break;
        }
    }
    
    switch (cbtype) {
    case SSS_NEWF_CB:
        create_leval_frame(NULL, gr);
        break;
    case SSS_NEWS_CB:
        if ((pset = grace_set_new(gr))) {
            set_set_comment(pset, "Editor");
            create_ss_frame(pset);
        }
        break;
    case SSS_NEWE_CB:
        if ((pset = grace_set_new(gr))) {
            set_set_comment(pset, "Editor");
            do_ext_editor(pset);
        }
        break;
    case SSS_NEWB_CB:
        create_eblock_frame(gr);
        break;
    }
    
    if (n > 0) {
        xfree(values);
    }
    
    update_all();
    xdrawgraph(grace->project, FALSE);
}

static void s_editS_cb(Widget but, void *udata)
{
    sss_any_cb(udata, SSS_EDITS_CB);
}

static void s_editE_cb(Widget but, void *udata)
{
    sss_any_cb(udata, SSS_EDITE_CB);
}

static void s_newF_cb(Widget but, void *udata)
{
    sss_any_cb(udata, SSS_NEWF_CB);
}

static void s_newS_cb(Widget but, void *udata)
{
    sss_any_cb(udata, SSS_NEWS_CB);
}

static void s_newE_cb(Widget but, void *udata)
{
    sss_any_cb(udata, SSS_NEWE_CB);
}

static void s_newB_cb(Widget but, void *udata)
{
    sss_any_cb(udata, SSS_NEWB_CB);
}

static void s_popup_cb(StorageStructure *ss, int nselected)
{
    SSSData *sssdata = (SSSData *) ss->data;
    int selected;
    
    if (nselected != 0) {
        selected = TRUE;
    } else {
        selected = FALSE;
    }
    
    SetSensitive(sssdata->edit_menu, selected);
}

static void s_dc_cb(StorageStructure *ss, Quark *pset, void *data)
{
    create_ss_frame(pset);
}

static char *set_labeling(Quark *q, unsigned int *rid)
{
    char buf[128];
    if (quark_fid_get(q) == QFlavorSet) {
        set *p = set_get_data(q);

        sprintf(buf, "Set \"%s\" (type: %s, length: %d)",
            QIDSTR(q), set_type_descr(grace->rt, p->type),
            set_get_length(q));

        (*rid)++;
        
        return copy_string(NULL, buf);
    } else {
        return NULL;
    }
}

StorageStructure *CreateSetChoice(Widget parent,
    char *labelstr, int type, StorageStructure *graphss)
{
    StorageStructure *ss;
    SSSData *sssdata;
    Widget popup, submenupane;
    int nvisible;
    
    nvisible = (type == LIST_TYPE_SINGLE) ? 4 : 8; 
    ss = CreateStorageChoice(parent, labelstr, type, nvisible);
    SetStorageChoiceLabeling(ss, set_labeling);
    AddStorageChoiceDblClickCB(ss, s_dc_cb, NULL);
    AddHelpCB(ss->rc, "doc/UsersGuide.html#set-selector");

    nset_selectors++;
    set_selectors =
        xrealloc(set_selectors, nset_selectors*sizeof(StorageStructure *));
    set_selectors[nset_selectors - 1] = ss;
    
    sssdata = xmalloc(sizeof(SSSData));
    ss->data = sssdata;
    ss->popup_cb = s_popup_cb;
    sssdata->graphss = graphss;
    
    popup = ss->popup;
    
    CreateMenuSeparator(popup);

    sssdata->edit_menu = CreateMenu(popup, "Edit", 'E', FALSE);
    CreateMenuButton(sssdata->edit_menu, "In spreadsheet", '\0',
    	s_editS_cb, ss);
    CreateMenuButton(sssdata->edit_menu, "In text editor", '\0',
    	s_editE_cb, ss);
    submenupane = CreateMenu(popup, "Create new", '\0', FALSE);
    CreateMenuButton(submenupane, "By formula", '\0', s_newF_cb, ss);
    CreateMenuButton(submenupane, "In spreadsheet", '\0', s_newS_cb, ss);
    CreateMenuButton(submenupane, "In text editor", '\0', s_newE_cb, ss);
    CreateMenuButton(submenupane, "From block data", '\0', s_newB_cb, ss);

    UpdateSetChoice(ss);
    
    return ss;
}

void UpdateSetChoice(StorageStructure *ss)
{
    Quark *gr;
    
    gr = get_set_choice_gr(ss);
    
    SetStorageChoiceQuark(ss, gr);
}


void update_set_selectors(Quark *gr)
{
    int i;
    
    if (gr) {
        update_graph_selectors(get_parent_project(gr));
    }
    
    for (i = 0; i < nset_selectors; i++) {
        Quark *cg;
        
        cg = get_set_choice_gr(set_selectors[i]);
        if (!gr || cg == gr) {
            UpdateSetChoice(set_selectors[i]);
        }
    }
}

static void update_sets_cb(StorageStructure *ss, int n, Quark **values, void *data)
{
    GraphSetStructure *gs = (GraphSetStructure *) data;

    UpdateSetChoice(gs->set_sel); 
}

GraphSetStructure *CreateGraphSetSelector(Widget parent, char *s, int sel_type)
{
    GraphSetStructure *retval;
    Widget rc;

    retval = xmalloc(sizeof(GraphSetStructure));
    retval->frame = CreateFrame(parent, s);
    rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, retval->frame, NULL);
    retval->graph_sel = CreateGraphChoice(rc, "Graph:", LIST_TYPE_SINGLE);
    retval->set_sel = CreateSetChoice(rc, "Set:", sel_type, retval->graph_sel);
    AddStorageChoiceCB(retval->graph_sel, update_sets_cb, (void *) retval);
    UpdateSetChoice(retval->set_sel);
    retval->set_sel->governor = retval->graph_sel;
    XtManageChild(rc);

    return retval;
}

SrcDestStructure *CreateSrcDestSelector(Widget parent, int sel_type)
{
    SrcDestStructure *retval;

    retval = xmalloc(sizeof(SrcDestStructure));

    retval->form = XtVaCreateWidget("form",
        xmFormWidgetClass, parent,
        XmNfractionBase, 2,
        NULL);
    retval->src  = CreateGraphSetSelector(retval->form, "Source", sel_type);
    retval->dest = CreateGraphSetSelector(retval->form, "Destination", sel_type);
    XtVaSetValues(retval->src->frame,
        XmNtopAttachment, XmATTACH_FORM,
        XmNbottomAttachment, XmATTACH_FORM,
        XmNleftAttachment, XmATTACH_FORM,
        XmNrightAttachment, XmATTACH_POSITION,
        XmNrightPosition, 1,
        NULL);

    XtVaSetValues(retval->dest->frame,
        XmNtopAttachment, XmATTACH_FORM,
        XmNbottomAttachment, XmATTACH_FORM,
        XmNleftAttachment, XmATTACH_POSITION,
        XmNleftPosition, 1,
        XmNrightAttachment, XmATTACH_FORM,
        NULL);

    XtManageChild(retval->form);

    return retval;
}

void UpdateSrcDestSelector(SrcDestStructure *srcdest)
{
    Quark *grsrc, *grdest;
    
    grsrc  = get_set_choice_gr(srcdest->src->set_sel);
    grdest = get_set_choice_gr(srcdest->dest->set_sel);
    
    update_set_lists(grsrc);
    if (grsrc != grdest) {
        update_set_lists(grdest);
    }
}


void paint_color_selector(OptionStructure *optp)
{
    X11Stuff *xstuff = grace->gui->xstuff;
    unsigned int i;
    long bg, fg;
    Project *pr = project_get_data(grace->project);
    
    if (!pr) {
        return;
    }
    
    for (i = 0; i < pr->ncolors; i++) {
        Colordef *c = &pr->colormap[i];
        bg = xvlibcolors[c->id];
	if (get_rgb_intensity(&c->rgb) < 0.5) {
	    fg = WhitePixel(xstuff->disp, xstuff->screennumber);
	} else {
	    fg = BlackPixel(xstuff->disp, xstuff->screennumber);
	}
	XtVaSetValues(optp->options[i].widget, 
            XmNbackground, bg,
            XmNforeground, fg,
            NULL);
    }
}

void update_color_selectors(void)
{
    unsigned int i;
    Project *pr = project_get_data(grace->project);
    
    ncolor_option_items = pr->ncolors;

    color_option_items = xrealloc(color_option_items,
                                    ncolor_option_items*sizeof(OptionItem));
    for (i = 0; i < pr->ncolors; i++) {
        Colordef *c = &pr->colormap[i];
        color_option_items[i].value = c->id;
        color_option_items[i].label = c->cname;
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
    
    paint_color_selector(retvalp);
    
    return retvalp;
}

SpinStructure *CreateLineWidthChoice(Widget parent, char *s)
{
    return CreateSpinChoice(parent, s, 3, SPIN_TYPE_FLOAT, 0.0, MAX_LINEWIDTH, 0.5);
}


OptionStructure *CreatePanelChoice(Widget parent, char *labelstr, ...)
{
    OptionStructure *retval;
    int nchoices = 0;
    OptionItem *oi = NULL;
    va_list var;
    char *s;

    va_start(var, labelstr);
    while ((s = va_arg(var, char *)) != NULL) {
        nchoices++;
        oi = xrealloc(oi, nchoices*sizeof(OptionItem));
        oi[nchoices - 1].value = nchoices - 1;
        oi[nchoices - 1].label = copy_string(NULL, s);
    }
    va_end(var);

    retval = CreateOptionChoice(parent, labelstr, 1, nchoices, oi);
    
    while (nchoices) {
        nchoices--;
	xfree(oi[nchoices].label);
    }
    xfree(oi);
    
    return retval;
}

static OptionItem fmt_option_items[31] =
{
    {FORMAT_DECIMAL,        "Decimal"             },
    {FORMAT_EXPONENTIAL,    "Exponential"         },
    {FORMAT_GENERAL,        "General"             },
    {FORMAT_POWER,          "Power"               },
    {FORMAT_SCIENTIFIC,     "Scientific"          },
    {FORMAT_ENGINEERING,    "Engineering"         },
    {FORMAT_DDMMYY,         "DD-MM-YY"            },
    {FORMAT_MMDDYY,         "MM-DD-YY"            },
    {FORMAT_YYMMDD,         "YY-MM-DD"            },
    {FORMAT_MMYY,           "MM-YY"               },
    {FORMAT_MMDD,           "MM-DD"               },
    {FORMAT_MONTHDAY,       "Month-DD"            },
    {FORMAT_DAYMONTH,       "DD-Month"            },
    {FORMAT_MONTHS,         "Month (abrev.)"      },
    {FORMAT_MONTHSY,        "Month (abrev.)-YY"   },
    {FORMAT_MONTHL,         "Month"               },
    {FORMAT_DAYOFWEEKS,     "Day of week (abrev.)"},
    {FORMAT_DAYOFWEEKL,     "Day of week"         },
    {FORMAT_DAYOFYEAR,      "Day of year"         },
    {FORMAT_HMS,            "HH:MM:SS"            },
    {FORMAT_MMDDHMS,        "MM-DD HH:MM:SS"      },
    {FORMAT_MMDDYYHMS,      "MM-DD-YY HH:MM:SS"   },
    {FORMAT_YYMMDDHMS,      "YY-MM-DD HH:MM:SS"   },
    {FORMAT_DEGREESLON,     "Degrees (lon)"       },
    {FORMAT_DEGREESMMLON,   "DD MM' (lon)"        },
    {FORMAT_DEGREESMMSSLON, "DD MM' SS.s\" (lon)" },
    {FORMAT_MMSSLON,        "MM' SS.s\" (lon)"    },
    {FORMAT_DEGREESLAT,     "Degrees (lat)"       },
    {FORMAT_DEGREESMMLAT,   "DD MM' (lat)"        },
    {FORMAT_DEGREESMMSSLAT, "DD MM' SS.s\" (lat)" },
    {FORMAT_MMSSLAT,        "MM' SS.s\" (lat)"    }
};

OptionStructure *CreateFormatChoice(Widget parent, char *s)
{
    OptionStructure *retval;
    
    retval = CreateOptionChoice(parent, s, 4, 31, fmt_option_items);
    
    return(retval);
}

static OptionItem as_option_items[4] = 
{
    {AUTOSCALE_NONE, "None"},
    {AUTOSCALE_X,    "X"},
    {AUTOSCALE_Y,    "Y"},
    {AUTOSCALE_XY,   "XY"}
};

OptionStructure *CreateASChoice(Widget parent, char *s)
{
    OptionStructure *retval;
    
    retval = CreateOptionChoice(parent, s, 1, 4, as_option_items);
    /* As init value, use this */
    SetOptionChoice(retval, grace->rt->autoscale_onread);
    
    return(retval);
}

OptionStructure *CreatePrecisionChoice(Widget parent, char *s)
{
    return CreateOptionChoiceVA(parent, s,
        "0", 0,
        "1", 1,
        "2", 2,
        "3", 3,
        "4", 4,
        "5", 5,
        "6", 6,
        "7", 7,
        "8", 8,
        "9", 9,
        NULL);
}

OptionStructure *CreatePlacementChoice(Widget parent, char *s)
{
    return CreateOptionChoiceVA(parent, s,
        "Normal",   PLACEMENT_NORMAL,
        "Opposite", PLACEMENT_OPPOSITE,
        "Both",     PLACEMENT_BOTH,
        NULL);
}

OptionStructure *CreatePaperOrientationChoice(Widget parent, char *s)
{
    return CreateOptionChoiceVA(parent, s,
        "Landscape", PAGE_ORIENT_LANDSCAPE,
        "Portrait",  PAGE_ORIENT_PORTRAIT,
        NULL);
}

OptionStructure *CreatePaperFormatChoice(Widget parent, char *s)
{
    return CreateOptionChoiceVA(parent, s,
        "Custom", PAGE_FORMAT_CUSTOM,
        "Letter", PAGE_FORMAT_USLETTER,
        "A4",     PAGE_FORMAT_A4,
        NULL);
}


Widget CreateScale(Widget parent, char *s, int min, int max, int delta)
{
    Widget w;
    XmString str;

    str = XmStringCreateLocalized(s);
    
    w = XtVaCreateManagedWidget("scroll",
        xmScaleWidgetClass, parent,
	XmNtitleString, str,
	XmNminimum, min,
	XmNmaximum, max,
        XmNscaleMultiple, delta,
	XmNvalue, 0,
	XmNshowValue, True,
	XmNprocessingDirection, XmMAX_ON_RIGHT,
	XmNorientation, XmHORIZONTAL,
#if XmVersion >= 2000    
	XmNsliderMark, XmROUND_MARK,
#endif
	NULL);

    XmStringFree(str);
    
    return w;
}

void scale_int_cb_proc( Widget w, XtPointer client_data, XtPointer call_data)
{
    Scale_CBdata *cbdata = (Scale_CBdata *) client_data;
 
    cbdata->cbproc(cbdata->scale, GetScaleValue(cbdata->scale), cbdata->anydata);
}

void AddScaleCB(Widget w, Scale_CBProc cbproc, void *anydata)
{
    Scale_CBdata *cbdata;
    
    cbdata = xmalloc(sizeof(Scale_CBdata));
    
    cbdata->scale = w;
    cbdata->cbproc = cbproc;
    cbdata->anydata = anydata;
    XtAddCallback(w,
        XmNvalueChangedCallback, scale_int_cb_proc, (XtPointer) cbdata);
}


void SetScaleValue(Widget w, int value)
{
    XtVaSetValues(w, XmNvalue, value, NULL);
}

int GetScaleValue(Widget w)
{
    int value;
    XtVaGetValues(w, XmNvalue, &value, NULL);
    return value;
}

void SetScaleWidth(Widget w, int width)
{
    XtVaSetValues(w, XmNscaleWidth, (Dimension) width, NULL);
}

Widget CreateAngleChoice(Widget parent, char *s)
{
    return CreateScale(parent, s, 0, 360, 10);
}

int GetAngleChoice(Widget w)
{
    return GetScaleValue(w);
}

void SetAngleChoice(Widget w, int angle)
{
    if (angle < 0 || angle > 360) {
        angle %= 360;
    }
    if (angle < 0) {
        angle += 360;
    }
    SetScaleValue(w, angle);
}

SpinStructure *CreateCharSizeChoice(Widget parent, char *s)
{
    return CreateSpinChoice(parent, s, 4, SPIN_TYPE_FLOAT, 0.0, 100.0, 0.25);
}


Widget CreateToggleButton(Widget parent, char *s)
{
    return (XtVaCreateManagedWidget(s, xmToggleButtonWidgetClass, parent, NULL));
}

int GetToggleButtonState(Widget w)
{
    if (!w) {
        errmsg("Internal error: GetToggleButtonState() called with NULL widget");
        return 0;
    } else {
        return XmToggleButtonGetState(w);
    }
}

void SetToggleButtonState(Widget w, int value)
{
    if (w == NULL) {
        return;
    }
    XmToggleButtonSetState(w, value ? True:False, False);
    
    return;
}

typedef struct {
    Widget tbut;
    TB_CBProc cbproc;
    void *anydata;
} TB_CBdata;

static void tb_int_cb_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int onoff;
    
    TB_CBdata *cbdata = (TB_CBdata *) client_data;

    onoff = GetToggleButtonState(w);
    cbdata->cbproc(cbdata->tbut, onoff, cbdata->anydata);
}

void AddToggleButtonCB(Widget w, TB_CBProc cbproc, void *anydata)
{
    TB_CBdata *cbdata;
    
    cbdata = xmalloc(sizeof(TB_CBdata));
    
    cbdata->tbut = w;
    cbdata->cbproc = cbproc;
    cbdata->anydata = anydata;
    XtAddCallback(w,
        XmNvalueChangedCallback, tb_int_cb_proc, (XtPointer) cbdata);
}

Widget CreateDialogForm(Widget parent, const char *s)
{
    X11Stuff *xstuff = grace->gui->xstuff;
    Widget dialog, w;
    char *bufp;
    int standalone;
    
    if (parent == NULL) {
        standalone = TRUE;
        parent = XtAppCreateShell("XMgrace", "XMgrace",
            topLevelShellWidgetClass, xstuff->disp,
            NULL, 0);
    } else {
        standalone = FALSE;
    }
    bufp = label_to_resname(s, "Dialog");
    dialog = XmCreateDialogShell(parent, bufp, NULL, 0);
    xfree(bufp);
    
    if (standalone) {
        RegisterEditRes(dialog);
    }
    
    handle_close(dialog);

    bufp = copy_string(NULL, "Grace: ");
    bufp = concat_strings(bufp, s);
    XtVaSetValues(dialog,
        XmNtitle, bufp,
        NULL);
    xfree(bufp);

    w = XmCreateForm(dialog, "form", NULL, 0);
    
    return w;
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

typedef struct {
    Widget form;
    int close;
    AACDialog_CBProc cbproc;
    void *anydata;
} AACDialog_CBdata;

void aacdialog_int_cb_proc(Widget but, void *data)
{
    AACDialog_CBdata *cbdata;
    int retval;
    
    set_wait_cursor();

    cbdata = (AACDialog_CBdata *) data;
    
    retval = cbdata->cbproc(cbdata->anydata);

    if (cbdata->close && retval == RETURN_SUCCESS) {
        XtUnmanageChild(XtParent(cbdata->form));
    }
    
    unset_wait_cursor();
}

WidgetList CreateAACDialog(Widget form,
    Widget container, AACDialog_CBProc cbproc, void *data)
{
    Widget fr;
    WidgetList aacbut;
    AACDialog_CBdata *cbdata_accept, *cbdata_apply;
    char *aaclab[3] = {"Apply", "Accept", "Close"};

    aacbut = (WidgetList) XtMalloc(3*sizeof(Widget));
    
    fr = CreateFrame(form, NULL);
    XtVaSetValues(fr,
        XmNtopAttachment, XmATTACH_NONE,
        XmNleftAttachment, XmATTACH_FORM,
        XmNrightAttachment, XmATTACH_FORM,
        XmNbottomAttachment, XmATTACH_FORM,
        NULL);
    CreateCommandButtons(fr, 3, aacbut, aaclab);

    AddDialogFormChild(form, container);
    XtVaSetValues(container,
        XmNbottomAttachment, XmATTACH_WIDGET,
        XmNbottomWidget, fr,
        NULL);
    
    XtVaSetValues(form, XmNcancelButton, aacbut[2], NULL);
    
    cbdata_accept = xmalloc(sizeof(AACDialog_CBdata));
    cbdata_accept->form    = form;
    cbdata_accept->anydata = data;
    cbdata_accept->cbproc  = cbproc;
    cbdata_accept->close   = TRUE;

    cbdata_apply  = xmalloc(sizeof(AACDialog_CBdata));
    cbdata_apply->form     = form;
    cbdata_apply->anydata  = data;
    cbdata_apply->cbproc   = cbproc;
    cbdata_apply->close    = FALSE;

    AddButtonCB(aacbut[0], aacdialog_int_cb_proc, cbdata_apply);
    AddButtonCB(aacbut[1], aacdialog_int_cb_proc, cbdata_accept);
    AddButtonCB(aacbut[2], destroy_dialog_cb, XtParent(form));
    
    XtManageChild(container);
    XtManageChild(form);
    
    return aacbut;
}

int td_cb(void *data)
{
    int res, i, nssrc, error;
    Quark **srcsets, **destsets;
    TransformStructure *tdialog = (TransformStructure *) data;
    void *tddata;

    res = GetTransformDialogSettings(tdialog, &nssrc, &srcsets, &destsets);
    
    if (res != RETURN_SUCCESS) {
        return RETURN_FAILURE;
    }
    
    tddata = tdialog->get_cb(tdialog->gui);
    if (!tddata) {
        return RETURN_FAILURE;
    }
    
    error = FALSE;
    
    for (i = 0; i < nssrc; i++) {
	Quark *psrc, *pdest;
        psrc  = srcsets[i];
	pdest = destsets[i];

        res = tdialog->run_cb(psrc, pdest, tddata);
	if (res != RETURN_SUCCESS) {
	    error = TRUE;
	    break;
	}
    }
    
    if (nssrc > 0) {
        xfree(srcsets);
        xfree(destsets);
    }
    
    tdialog->free_cb(tddata);
    UpdateSrcDestSelector(tdialog->srcdest);
    
    xdrawgraph(grace->project, FALSE);
    
    if (error == FALSE) {
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}


TransformStructure *CreateTransformDialogForm(Widget parent,
    const char *s, int sel_type, int exclusive, const TD_CBProcs *cbs)
{
    TransformStructure *retval;

    set_wait_cursor();
    
    retval = xmalloc(sizeof(TransformStructure));
    memset(retval, 0, sizeof(TransformStructure));
    
    retval->exclusive = exclusive;
    
    retval->build_cb = cbs->build_cb;
    retval->get_cb   = cbs->get_cb;
    retval->run_cb   = cbs->run_cb;
    retval->free_cb  = cbs->free_cb;
    
    retval->form = CreateDialogForm(parent, s);

    retval->menubar = CreateMenuBar(retval->form);
    AddDialogFormChild(retval->form, retval->menubar);
    
    retval->srcdest = CreateSrcDestSelector(retval->form, sel_type);
    AddDialogFormChild(retval->form, retval->srcdest->form);

    retval->frame = CreateFrame(retval->form, NULL);
    retval->gui = retval->build_cb(retval);

/*
 *     retval->restr = CreateRestrictionChoice(retval->form, "Source data filtering");
 *     AddDialogFormChild(retval->form, retval->restr->frame);
 */
    
    CreateAACDialog(retval->form, retval->frame, td_cb, retval);
    
    /* FixateDialogFormChild(retval->frame); */
    
    unset_wait_cursor();

    return retval;
}

int GetTransformDialogSettings(TransformStructure *tdialog,
        int *nssrc, Quark ***srcsets, Quark ***destsets)
{
    int i, nsdest;
    
    *nssrc = GetStorageChoices(tdialog->srcdest->src->set_sel, srcsets);
    if (*nssrc == 0) {
        errmsg("No source sets selected");
	return RETURN_FAILURE;
    }    
    
    nsdest = GetStorageChoices(tdialog->srcdest->dest->set_sel, destsets);
    if (nsdest != 0 && *nssrc != nsdest) {
        errmsg("Different number of source and destination sets");
        xfree(*srcsets);
        xfree(*destsets);
	return RETURN_FAILURE;
    }
    
    /* check for mutually exclusive selections */
    if (tdialog->exclusive && nsdest != 0) {
        for (i = 0; i < *nssrc; i++) {
            if ((*srcsets)[i] == (*destsets)[i]) {
                xfree(*srcsets);
                xfree(*destsets);
                errmsg("Source and destination set(s) are not mutually exclusive");
	        return RETURN_FAILURE;
            }
        }
    }
    
    if (nsdest == 0) {
        Quark *destgr;
        if (GetSingleStorageChoice(tdialog->srcdest->dest->graph_sel, &destgr)
            != RETURN_SUCCESS) {
            xfree(*srcsets);
            errmsg("No destination graph selected");
	    return RETURN_FAILURE;
        }
        
        *destsets = xmalloc((*nssrc)*sizeof(Quark *));
        for (i = 0; i < *nssrc; i++) {
            (*destsets)[i] = grace_set_new(destgr);
        }
        
        update_set_selectors(destgr);
        SelectStorageChoices(tdialog->srcdest->dest->set_sel, *nssrc, *destsets);
    }
    
    return RETURN_SUCCESS;
}

void RaiseTransformationDialog(TransformStructure *tdialog)
{
    RaiseWindow(GetParent(tdialog->form));
}

Widget CreateVContainer(Widget parent)
{
    Widget rc;
    
    rc = XmCreateRowColumn(parent, "VContainer", NULL, 0);
    XtManageChild(rc);
    
    return rc;
}

Widget CreateHContainer(Widget parent)
{
    Widget rc;
    
    rc = XmCreateRowColumn(parent, "HContainer", NULL, 0);
    XtVaSetValues(rc, XmNorientation, XmHORIZONTAL, NULL);
    XtManageChild(rc);
    
    return rc;
}


Widget CreateFrame(Widget parent, char *s)
{
    Widget fr;
    
    fr = XtVaCreateManagedWidget("frame", xmFrameWidgetClass, parent, NULL);
    if (s != NULL) {
        XtVaCreateManagedWidget(s, xmLabelGadgetClass, fr,
				XmNchildType, XmFRAME_TITLE_CHILD,
				NULL);
    }
    
    return (fr);   
}


typedef struct {
    int ncols;
    int nrows;
} GridData;

Widget CreateGrid(Widget parent, int ncols, int nrows)
{
    Widget w;
    int nfractions;
    GridData *gd;
    
    if (ncols <= 0 || nrows <= 0) {
        errmsg("Wrong call to CreateGrid()");
        ncols = 1;
        nrows = 1;
    }
    
    nfractions = 0;
    do {
        nfractions++;
    } while (nfractions % ncols || nfractions % nrows);
    
    gd = xmalloc(sizeof(GridData));
    gd->ncols = ncols;
    gd->nrows = nrows;
    
    w = XmCreateForm(parent, "grid_form", NULL, 0);
    XtVaSetValues(w,
        XmNfractionBase, nfractions,
        XmNuserData, gd,
        NULL);

    XtManageChild(w);
    return w;
}

void PlaceGridChild(Widget grid, Widget w, int col, int row)
{
    int nfractions, w1, h1;
    GridData *gd;
    
    XtVaGetValues(grid,
        XmNfractionBase, &nfractions,
        XmNuserData, &gd,
        NULL);
    
    if (gd == NULL) {
        errmsg("PlaceGridChild() called with a non-grid widget");
        return;
    }
    if (col < 0 || col >= gd->ncols) {
        errmsg("PlaceGridChild() called with wrong `col' argument");
        return;
    }
    if (row < 0 || row >= gd->nrows) {
        errmsg("PlaceGridChild() called with wrong `row' argument");
        return;
    }
    
    w1 = nfractions/gd->ncols;
    h1 = nfractions/gd->nrows;
    
    XtVaSetValues(w,
        XmNleftAttachment  , XmATTACH_POSITION,
        XmNleftPosition    , col*w1           ,
        XmNrightAttachment , XmATTACH_POSITION,
        XmNrightPosition   , (col + 1)*w1     ,
        XmNtopAttachment   , XmATTACH_POSITION,
        XmNtopPosition     , row*h1           ,
        XmNbottomAttachment, XmATTACH_POSITION,
        XmNbottomPosition  , (row + 1)*h1     ,
        NULL);
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
    str = XmStringCreateLocalized(s);
    XtVaSetValues(w, XmNtabLabel, str, NULL);
    XmStringFree(str);
    XtManageChild(w);
    
    return (w);
}

void SelectTabPage(Widget tab, Widget w)
{
    XmTabSetTabWidget(tab, w, True);
}

Widget CreateTextItem(Widget parent, int len, char *s)
{
    Widget w;
    Widget rc;
    XmString str;
    rc = XmCreateRowColumn(parent, "rc", NULL, 0);
    XtVaSetValues(rc, XmNorientation, XmHORIZONTAL, NULL);
    str = XmStringCreateLocalized(s);
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

typedef struct {
    TItem_CBProc cbproc;
    void *anydata;
} TItem_CBdata;

static void titem_int_cb_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    char *s;
    TItem_CBdata *cbdata = (TItem_CBdata *) client_data;
    s = XmTextGetString(w);
    cbdata->cbproc(w, s, cbdata->anydata);
    XtFree(s);
}


void AddTextItemCB(Widget ti, TItem_CBProc cbproc, void *data)
{
    TItem_CBdata *cbdata;
    
    cbdata = xmalloc(sizeof(TItem_CBdata));
    cbdata->anydata = data;
    cbdata->cbproc = cbproc;
    XtAddCallback(ti, XmNactivateCallback, titem_int_cb_proc, (XtPointer) cbdata);
}

/* FIXME: get rid of xv_getstr()!!! */
#define MAX_STRING_LENGTH 512
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
    static char *buf = NULL;
    int i, len, ier = 0;
    double result;
	
    buf = copy_string(buf, s = XmTextGetString(w));
    XtFree(s);

    if (!(len = strlen( buf ) )) { /* check for zero length */
        *answer = 0;
        return RETURN_FAILURE;
    }
    /* first character may be a sign */
    if (!fpdigit[(int) buf[0]] && buf[0] != '-' && buf[0] != '+') {
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
        return RETURN_SUCCESS;
    } else {                /* must evaluate an expression */
        ier = s_scanner(buf, &result);
        if( !ier ) {
            *answer = result;
            return RETURN_SUCCESS;
        } else {
            *answer = 0;
            return RETURN_FAILURE;
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
    static char *buf = NULL;
    int i, len, ier = 0;
    double result;
	
    buf = copy_string(buf, s = XmTextGetString(w));
    XtFree(s);

    if (!(len = strlen( buf ) )) { /* check for zero length */
        *answer = 0;
        return RETURN_FAILURE;
    }
    /* first character may be a sign */
    if (!fpdigit[(int) buf[0]] && buf[0] != '-' && buf[0] != '+') {
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
        return RETURN_SUCCESS;
    } else {                    /* must evaluate an expression */
        ier = s_scanner(buf, &result);
        if( !ier ) {
            *answer = (int)result;
            return RETURN_SUCCESS;
        } else {
            *answer = 0;
            return RETURN_FAILURE;
        }
    }
}


void xv_setstr(Widget w, char *s)
{
    if (w != NULL) {
        XmTextSetString(w, s ? s : "");
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
 * same for AddButtonCB
 */
void destroy_dialog_cb(Widget but, void *data)
{
    XtUnmanageChild((Widget) data);
}

/* if user tried to close from WM */
static void wm_exit_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
    bailout(grace);
}

/*
 * handle the close item on the WM menu
 */
void handle_close(Widget w)
{
    X11Stuff *xstuff = grace->gui->xstuff;
    Atom WM_DELETE_WINDOW;
    
    XtVaSetValues(w, XmNdeleteResponse, XmDO_NOTHING, NULL);
    WM_DELETE_WINDOW = XmInternAtom(xstuff->disp, "WM_DELETE_WINDOW", False);
    XmAddProtocolCallback(w,
        XM_WM_PROTOCOL_ATOM(w), WM_DELETE_WINDOW,
        (w == app_shell) ? wm_exit_cb : destroy_dialog, w);
    
    savewidget(w);
}

/*
 * Manage and raise
 */
void RaiseWindow(Widget w)
{
    XtManageChild(w);
    XMapRaised(XtDisplay(w), XtWindow(w));
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
    X11Stuff *xstuff = grace->gui->xstuff;
    int i;
    
    for (i = 0; i < nsavedwidgets; i++) {
	XDefineCursor(xstuff->disp, XtWindow(savewidgets[i]), c);
    }
    XFlush(xstuff->disp);
}

void UndefineDialogCursor(void)
{
    X11Stuff *xstuff = grace->gui->xstuff;
    int i;
    
    for (i = 0; i < nsavedwidgets; i++) {
	XUndefineCursor(xstuff->disp, XtWindow(savewidgets[i]));
    }
    XFlush(xstuff->disp);
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

Widget CreateSeparator(Widget parent)
{
    Widget sep;
    
    sep = XmCreateSeparator(parent, "sep", NULL, 0);
    XtManageChild(sep);
    return sep;
}

Widget CreateMenuBar(Widget parent)
{
    Widget menubar;
    
    menubar = XmCreateMenuBar(parent, "menuBar", NULL, 0);
    return menubar;
}

Widget CreateMenu(Widget parent, char *label, char mnemonic, int help)
{
    Widget menupane, cascade;
    XmString str;
    char *name, ms[2];
    
    name = label_to_resname(label, "Menu");
    menupane = XmCreatePulldownMenu(parent, name, NULL, 0);
    xfree(name);

    ms[0] = mnemonic;
    ms[1] = '\0';
    
    str = XmStringCreateLocalized(label);
    cascade = XtVaCreateManagedWidget("cascade",
        xmCascadeButtonGadgetClass, parent, 
    	XmNsubMenuId, menupane, 
    	XmNlabelString, str, 
    	XmNmnemonic, XStringToKeysym(ms),
    	NULL);
    XmStringFree(str);

    if (help) {
        XtVaSetValues(parent, XmNmenuHelpWidget, cascade, NULL);
        CreateMenuButton(menupane, "On context", 'x',
            ContextHelpCB, NULL);
        CreateSeparator(menupane);
    }
    
    SetUserData(menupane, cascade);

    return menupane;
}

void ManageMenu(Widget menupane)
{
    Widget cascade = GetUserData(menupane);
    XtManageChild(cascade);
}

void UnmanageMenu(Widget menupane)
{
    Widget cascade = GetUserData(menupane);
    XtUnmanageChild(cascade);
}

Widget CreateMenuButton(Widget parent, char *label, char mnemonic,
	Button_CBProc cb, void *data)
{
    Widget button;
    XmString str;
    char *name, ms[2];
    
    ms[0] = mnemonic;
    ms[1] = '\0';
    
    str = XmStringCreateLocalized(label);
    name = label_to_resname(label, "Button");
    button = XtVaCreateManagedWidget(name,
        xmPushButtonWidgetClass, parent, 
    	XmNlabelString, str,
    	XmNmnemonic, XStringToKeysym(ms),
    	NULL);
    xfree(name);
    XmStringFree(str);

    AddButtonCB(button, cb, data);

    return button;
}

Widget CreateMenuCloseButton(Widget parent, Widget form)
{
    Widget wbut;
    XmString str;
    
    wbut = CreateMenuButton(parent,
        "Close", 'C', destroy_dialog_cb, XtParent(form));
    str = XmStringCreateLocalized("Esc");
    XtVaSetValues(wbut, XmNacceleratorText, str, NULL);
    XmStringFree(str);
    XtVaSetValues(form, XmNcancelButton, wbut, NULL);
    
    return wbut;
}

Widget CreateMenuHelpButton(Widget parent, char *label, char mnemonic,
    Widget form, char *ha)
{
    Widget wbut;
    
    wbut = CreateMenuButton(parent, label, mnemonic, HelpCB, ha);
    AddHelpCB(form, ha);
    
    return wbut;
}

Widget CreateMenuToggle(Widget parent, char *label, char mnemonic,
	TB_CBProc cb, void *data)
{
    Widget button;
    XmString str;
    char *name, ms[2];
    
    ms[0] = mnemonic;
    ms[1] = '\0';
    
    str = XmStringCreateLocalized(label);
    name = label_to_resname(label, NULL);
    button = XtVaCreateManagedWidget(name,
        xmToggleButtonWidgetClass, parent, 
    	XmNlabelString, str,
    	XmNmnemonic, XStringToKeysym(ms),
    	XmNvisibleWhenOff, True,
    	XmNindicatorOn, True,
    	NULL);
    xfree(name);
    XmStringFree(str);

    if (cb) {
        AddToggleButtonCB(button, cb, data);
    }

    return button;
}

Widget CreateMenuLabel(Widget parent, char *name)
{
    Widget lab;
    
    lab = XmCreateLabel(parent, name, NULL, 0);
    XtManageChild(lab);
    return lab;
}

static void help_int_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
    HelpCB(w, client_data);
}

void AddHelpCB(Widget w, char *ha)
{
    if (XtHasCallbacks(w, XmNhelpCallback) == XtCallbackHasSome) {
        /* allow only one help callback */
        XtRemoveAllCallbacks(w, XmNhelpCallback);
    }
    
    XtAddCallback(w, XmNhelpCallback, help_int_cb, (XtPointer) ha);
}

void ContextHelpCB(Widget but, void *data)
{
    X11Stuff *xstuff = grace->gui->xstuff;
    Widget whelp;
    Cursor cursor;
    int ok = FALSE;
    
    cursor = XCreateFontCursor(xstuff->disp, XC_question_arrow);
    whelp = XmTrackingLocate(app_shell, cursor, False);
    while (whelp != NULL) {
        if (XtHasCallbacks(whelp, XmNhelpCallback) == XtCallbackHasSome) {
            XtCallCallbacks(whelp, XmNhelpCallback, NULL);
            ok = TRUE;
            break;
        } else {
            whelp = GetParent(whelp);
        }
    }
    if (!ok) {
        HelpCB(but, NULL);
    }
    XFreeCursor(xstuff->disp, cursor);
}


static int yesno_retval = FALSE;
static Boolean keep_grab = True;

void yesnoCB(Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *cbs = (XmAnyCallbackStruct *) call_data;
    int why = cbs->reason;

    keep_grab = False;
    
    XtRemoveGrab(XtParent(w));
    XtUnmanageChild(w);
    switch (why) {
    case XmCR_OK:
	yesno_retval = TRUE;
	/* process ok action */
	break;
    case XmCR_CANCEL:
	yesno_retval = FALSE;
	/* process cancel action */
	break;
    }
}

int yesnowin(char *msg, char *s1, char *s2, char *help_anchor)
{
    static Widget dialog = NULL;
    XEvent event;
    static char *ha = NULL;
    Widget but;
    XmString str;
    
    ha = help_anchor;

    keep_grab = True;

    if (dialog == NULL) {
        dialog = XmCreateErrorDialog(app_shell, "warningDialog", NULL, 0);

        str = XmStringCreateLocalized("Grace: Warning");
        XtVaSetValues(dialog, XmNdialogTitle, str, NULL);
        XmStringFree(str);
	
	XtAddCallback(dialog, XmNokCallback, yesnoCB, NULL);
	XtAddCallback(dialog, XmNcancelCallback, yesnoCB, NULL);
        
        but = XtNameToWidget(dialog, "Help");
        AddButtonCB(but, HelpCB, ha);
    }

    if (msg != NULL) {
        str = XmStringCreateLocalized(msg);
    } else {
        str = XmStringCreateLocalized("Warning");
    }
    XtVaSetValues(dialog, XmNmessageString, str, NULL);
    XmStringFree(str);
    
    but = XtNameToWidget(dialog, "OK");
    if (s1) {
    	SetLabel(but, s1);
    } else {    
    	SetLabel(but, "OK");
    }
    
    but = XtNameToWidget(dialog, "Cancel");
    if (s2) {
    	SetLabel(but, s2);
    } else {    
    	SetLabel(but, "Cancel");
    }
    
    but = XtNameToWidget(dialog, "Help");
    if (ha) {
        XtManageChild(but);
    } else {    
        XtUnmanageChild(but);
    }

    RaiseWindow(dialog);
    
    XtAddGrab(XtParent(dialog), True, False);
    while (keep_grab || XtAppPending(app_con)) {
	XtAppNextEvent(app_con, &event);
	XtDispatchEvent(&event);
    }
    return yesno_retval;
}


Widget CreateAACButtons(Widget parent, Widget form, Button_CBProc aac_cb)
{
    Widget w;
    Widget aacbut[3];
    static char *aaclab[3] = {"Apply", "Accept", "Close"};
    
    w = CreateCommandButtons(parent, 3, aacbut, aaclab);
    AddButtonCB(aacbut[0], aac_cb, (void *) AAC_APPLY);
    AddButtonCB(aacbut[1], aac_cb, (void *) AAC_ACCEPT);
    AddButtonCB(aacbut[2], aac_cb, (void *) AAC_CLOSE);
    
    if (form != NULL) {
        XtVaSetValues(form, XmNcancelButton, aacbut[2], NULL);
    }
    
    return w;
}

void SetLabel(Widget w, char *s)
{
    XmString str;

    str = XmStringCreateLocalized(s);
    XtVaSetValues(w, XmNlabelString, str, NULL);
    XmStringFree(str);
}

void SetFixedFont(Widget w)
{
    X11Stuff *xstuff = grace->gui->xstuff;
    XFontStruct *f;
    XmFontList xmf;

    f = XLoadQueryFont(xstuff->disp, "fixed");
    xmf = XmFontListCreate(f, charset);
    if (xmf == NULL) {
        errmsg("Can't load font \"fixed\"");
        return;
    } else {
        XtVaSetValues(w, XmNfontList, xmf, NULL);
        XmFontListFree(xmf);
    }
}

char *GetStringSimple(XmString xms)
{
    char *s;

    if (XmStringGetLtoR(xms, charset, &s)) {
        return s;
    } else {
        return NULL;
    }
}

void update_set_lists(Quark *gr)
{
    update_set_selectors(gr);
    update_ss_editors(gr);
    update_explorer(grace->gui->eui, TRUE);
    update_app_title(grace->project);
}

void update_undo_buttons(Quark *project)
{
    GUI *gui = gui_from_quark(project);
    AMem *amem = quark_get_amem(project);
    if (!gui || !amem) {
        return;
    }
    
    SetSensitive(gui->mwui->undo_button, amem_get_undo_count(amem));
    SetSensitive(gui->mwui->redo_button, amem_get_redo_count(amem));
}

void update_all(void)
{
    if (!grace->gui->inwin) {
        return;
    }

    if (gui_is_page_free(grace->gui)) {
        sync_canvas_size(grace);
    }
    
    update_ssd_selectors(grace->project);
    update_frame_selectors(grace->project);
    update_graph_selectors(grace->project);
    update_set_selectors(NULL);
    update_ss_editors(NULL);

    if (grace->gui->need_colorsel_update == TRUE) {
        init_xvlibcolors();
        update_color_selectors();
        grace->gui->need_colorsel_update = FALSE;
    }

    if (grace->gui->need_fontsel_update == TRUE) {
        update_font_selectors();
        grace->gui->need_fontsel_update = FALSE;
    }

    update_undo_buttons(grace->project);
    update_props_items();
    update_explorer(grace->gui->eui, TRUE);
    set_left_footer(NULL);
    update_app_title(grace->project);
}

void update_all_cb(Widget but, void *data)
{
    update_all();
}

int clean_graph_selectors(Quark *pr, int etype, void *data)
{
    if (etype == QUARK_ETYPE_DELETE) {
        int i;
        for (i = 0; i < ngraph_selectors; i++) {
            SetStorageChoiceQuark(graph_selectors[i], NULL);
        }
        for (i = 0; i < nssd_selectors; i++) {
            SetStorageChoiceQuark(ssd_selectors[i], NULL);
        }
    } else
    if (etype == QUARK_ETYPE_MODIFY) {
        /* update_graph_selectors(pr); */
    }
    
    return RETURN_SUCCESS;
}

int clean_frame_selectors(Quark *pr, int etype, void *data)
{
    if (etype == QUARK_ETYPE_DELETE) {
        int i;
        for (i = 0; i < nframe_selectors; i++) {
            SetStorageChoiceQuark(frame_selectors[i], NULL);
        }
    } else
    if (etype == QUARK_ETYPE_MODIFY) {
        /* update_frame_selectors(pr); */
    }
    
    return RETURN_SUCCESS;
}

int clean_set_selectors(Quark *gr, int etype, void *data)
{
    if (etype == QUARK_ETYPE_DELETE) {
        int i;
        for (i = 0; i < nset_selectors; i++) {
            Quark *cg;
            StorageStructure *ss = set_selectors[i];

            cg = get_set_choice_gr(ss);
            if (!gr || cg == gr) {
                SetStorageChoiceQuark(ss, NULL);
            }
        }
    }
    
    return RETURN_SUCCESS;
}
