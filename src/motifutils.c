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

#if defined(HAVE_XPM_H)
#  include <xpm.h>
#else
#  include <X11/xpm.h>
#endif

#include <Xm/Xm.h>
#include <Xm/Protocols.h>
#include <Xm/BulletinB.h>
#include <Xm/MessageB.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/List.h>
#include <Xm/PushB.h>
#include <Xm/CascadeBG.h>
#include <Xm/RowColumn.h>
#include <Xm/Text.h>
#include <Xm/ScrollBar.h>

#include <Xbae/Matrix.h>
#include "ListTree.h"

#if XmVersion < 2000
#  define XmStringConcatAndFree(a, b) XmStringConcat(a, b); XmStringFree(a); XmStringFree(b)
#endif

#include "motifinc.h"
#include "explorer.h"

#include "defines.h"
#include "globals.h"
#include "grace/canvas.h"
#include "jbitmaps.h"
#include "core_utils.h"
#include "utils.h"
#include "xprotos.h"
#include "events.h"

#define canvas grace_get_canvas(gapp->grace)

extern XtAppContext app_con;

static unsigned long *xvlibcolors;


static LabelOptionItem *color_option_items = NULL;
static unsigned int ncolor_option_items = 0;
static OptionStructure **color_selectors = NULL;
static unsigned int ncolor_selectors = 0;

static Widget color_choice_popup = NULL;

void Beep(void)
{
    X11Stuff *xstuff = gapp->gui->xstuff;

    XBell(xstuff->disp, 50);
}

static char list_translation_table[] = "\
    Ctrl<Key>E: list_activate_action()\n\
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
    X11Stuff *xstuff = gapp->gui->xstuff;
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

static void list_activate_action(Widget w, XEvent *e, String *par, Cardinal *npar)
{
    XtCallActionProc(w, "ListKbdActivate", NULL, NULL, 0);
}

static void list_selectall_action(Widget w, XEvent *e, String *par, Cardinal *npar)
{
    list_selectall(w);
}

static void list_unselectall_action(Widget w, XEvent *e, String *par, Cardinal *npar)
{
    list_unselectall(w);
}

static void list_invertselection_action(Widget w, XEvent *e, String *par,
				 Cardinal *npar)
{
    list_invertselection(w);
}

ListStructure *CreateListChoice(Widget parent, char *labelstr, int type,
                                int nvisible, int nchoices, LabelOptionItem *items)
{
    Arg args[4];
    Widget lab;
    ListStructure *retval;

    retval = xmalloc(sizeof(ListStructure));
    retval->rc = XmCreateRowColumn(parent, "rcList", NULL, 0);
    AddHelpCB(retval->rc, "doc/UsersGuide.html#list-selector");

    lab = XmCreateLabel(retval->rc, labelstr, NULL, 0);
    WidgetManage(lab);
    
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

    AddMouseWheelSupport(retval->list);

    XtOverrideTranslations(retval->list, 
                             XtParseTranslationTable(list_translation_table));
    
    UpdateListChoice(retval, nchoices, items);

    WidgetManage(retval->list);
    
    WidgetManage(retval->rc);
    
    return retval;
}

void UpdateListChoice(ListStructure *listp, int nchoices, LabelOptionItem *items)
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

int GetListSelectedCount(ListStructure *listp)
{
    int count;
    XtVaGetValues(listp->list, XmNselectedItemCount, &count, NULL);
    return count;
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
    char *buf;

    if (!q) {
        return NULL;
    }

    buf = (char *) xmalloc(strlen(QIDSTR(q)) + 128);
    if (!buf) {
        return NULL;
    }
    
    sprintf(buf, "Quark \"%s\"", QIDSTR(q));
    
    (*rid)++;
    
    return buf;
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
        
        ss->values = xrealloc(ss->values, SIZEOF_VOID_P*(ss->nchoices + 1));
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
    
    WidgetSetSensitive(ss->popup_hide_bt, selected);
    WidgetSetSensitive(ss->popup_show_bt, selected);
    WidgetSetSensitive(ss->popup_delete_bt, selected);
    WidgetSetSensitive(ss->popup_duplicate_bt, selected);
    WidgetSetSensitive(ss->popup_bring_to_front_bt, selected);
    WidgetSetSensitive(ss->popup_send_to_back_bt, selected);
    WidgetSetSensitive(ss->popup_move_up_bt, selected);
    WidgetSetSensitive(ss->popup_move_down_bt, selected);

    WidgetSetSensitive(ss->popup_properties_bt, n == 1);
    
    if (ss->popup_cb) {
        ss->popup_cb(ss, n);
    }
    
    XmMenuPosition(popup, e);
    WidgetManage(popup);
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
        snapshot_and_update(gapp->gp, TRUE);
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

static void ss_properties_cb(Widget but, void *udata)
{
    StorageStructure *ss = (StorageStructure *) udata;
    Quark *q;
    
    if (GetSingleStorageChoice(ss, &q) == RETURN_SUCCESS) {
        raise_explorer(gapp->gui, q);
    }
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

static void s_dc_cb(StorageStructure *ss, Quark *q, void *data)
{
    raise_explorer(gapp->gui, q);
}

static void CreateStorageChoicePopup(StorageStructure *ss)
{
    Widget popup, submenupane;
    
    popup = XmCreatePopupMenu(ss->list, "popupMenu", NULL, 0);
    ss->popup = popup;
#if XmVersion >= 2000    
    XtVaSetValues(popup, XmNpopupEnabled, XmPOPUP_DISABLED, NULL);
#else
    XtVaSetValues(popup, XmNpopupEnabled, False, NULL);
#endif

    ss->popup_properties_bt =
        CreateMenuButton(popup, "Properties...", '\0', ss_properties_cb, ss);

    CreateMenuSeparator(popup);

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

    AddStorageChoiceDblClickCB(ss, s_dc_cb, NULL);
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
    WidgetManage(lab);
    
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

    AddMouseWheelSupport(retval->list);

    CreateStorageChoicePopup(retval);
    XtAddEventHandler(retval->list,
        ButtonPressMask, False, storage_popup, retval);

    XtOverrideTranslations(retval->list, 
                             XtParseTranslationTable(list_translation_table));

    WidgetManage(retval->list);
    
    WidgetManage(retval->rc);
    
    return retval;
}

void SetStorageChoiceLabeling(StorageStructure *ss, Storage_LabelingProc proc)
{
    ss->labeling_proc = proc;
}

int GetStorageSelectedCount(StorageStructure *ss)
{
    int count;
    XtVaGetValues(ss->list, XmNselectedItemCount, &count, NULL);
    return count;
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
        XtFree((char *) selected);
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
        quark_traverse(ss->q, traverse_hook, &stdata);

        SelectStorageChoices(ss, nsel, selvalues);
    }
    
    if (nsel > 0) {
        xfree(selvalues);
    }
    
    nsel = GetStorageSelectedCount(ss);
    if (!nsel && XtHasCallbacks(ss->list, XmNsingleSelectionCallback) ==
            XtCallbackHasSome) {
        /* invoke callbacks to make any dependent GUI control to sync */
        XtCallCallbacks(ss->list, XmNsingleSelectionCallback, NULL);
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



/*
 * same for AddButtonCB
 */
void destroy_dialog_cb(Widget but, void *data)
{
    DialogClose(data);
}

static LabelOptionItem *settype_option_items;
static LabelOptionItem *fmt_option_items;
static LabelOptionItem *frametype_option_items;
static BitmapOptionItem *pattern_option_items;
static BitmapOptionItem *lines_option_items;

#define LINES_BM_HEIGHT 15
#define LINES_BM_WIDTH  64

static void init_xvlibcolors(void)
{
    X11Stuff *xstuff = gapp->gui->xstuff;
    Project *pr = project_get_data(gproject_get_top(gapp->gp));
    unsigned int i;
    
    if (!pr) {
        return;
    }
    
    xvlibcolors = xrealloc(xvlibcolors, pr->ncolors*sizeof(unsigned long));
    if (!xvlibcolors) {
        return;
    }
    
    for (i = 0; i < pr->ncolors; i++) {
        long pixel;
        Colordef *c = &pr->colormap[i];
        
        pixel = x11_allocate_color(gapp->gui, &c->rgb);
        
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

    settype_option_items = xmalloc(NUMBER_OF_SETTYPES*sizeof(LabelOptionItem));
    if (settype_option_items == NULL) {
        errmsg("Malloc error in init_option_menus()");
        return RETURN_FAILURE;
    }
    for (i = 0; i < NUMBER_OF_SETTYPES; i++) {
        settype_option_items[i].value = i;
        settype_option_items[i].label = copy_string(NULL,
            set_type_descr(gapp->grace, i));
    }

    fmt_option_items = xmalloc(NUMBER_OF_FORMATTYPES*sizeof(LabelOptionItem));
    if (fmt_option_items == NULL) {
        errmsg("Malloc error in init_option_menus()");
        return RETURN_FAILURE;
    }
    for (i = 0; i < NUMBER_OF_FORMATTYPES; i++) {
        fmt_option_items[i].value = i;
        fmt_option_items[i].label = copy_string(NULL,
            format_type_descr(gapp->grace, i));
    }

    frametype_option_items = xmalloc(NUMBER_OF_FRAMETYPES*sizeof(LabelOptionItem));
    if (frametype_option_items == NULL) {
        errmsg("Malloc error in init_option_menus()");
        return RETURN_FAILURE;
    }
    for (i = 0; i < NUMBER_OF_FRAMETYPES; i++) {
        frametype_option_items[i].value = i;
        frametype_option_items[i].label = copy_string(NULL,
            frame_type_descr(gapp->grace, i));
    }

    return RETURN_SUCCESS;
}

static LabelOptionItem *font_option_items = NULL;
static unsigned int nfont_option_items = 0;
static OptionStructure **font_selectors = NULL;
static unsigned int nfont_selectors = 0;

void update_font_selectors(void)
{
    unsigned int i;
    Project *pr = project_get_data(gproject_get_top(gapp->gp));
    
    nfont_option_items = pr->nfonts;
    font_option_items =
        xrealloc(font_option_items, nfont_option_items*sizeof(LabelOptionItem));

    for (i = 0; i < nfont_option_items; i++) {
        Fontdef *f = &pr->fontmap[i];
        font_option_items[i].value = f->id;
        font_option_items[i].label = f->fontname;
    }
    
    for (i = 0; i < nfont_selectors; i++) {
        UpdateLabelOptionChoice(font_selectors[i], 
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
    
    retvalp = CreateLabelOptionChoice(parent, s, 0, 
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
    return (CreateLabelOptionChoice(parent,
        s, 0, NUMBER_OF_SETTYPES, settype_option_items));
}

OptionStructure *CreateFrameTypeChoice(Widget parent, char *s)
{
    return (CreateLabelOptionChoice(parent,
        s, 0, NUMBER_OF_FRAMETYPES, frametype_option_items));
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
    LabelOptionItem restr_items[7];

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

    retval->r_sel = CreateLabelOptionChoice(rc,
        "Restriction:", 1, 7, restr_items);
    retval->negate = CreateToggleButton(rc, "Negated");
    WidgetManage(rc);

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
    X11Stuff *xstuff = gapp->gui->xstuff;
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
    
    pdata = WidgetGetUserData(button);
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
    
    pdata = WidgetGetUserData(parent);
    
    if (e->state & ShiftMask) {
        popup = pdata->pattern_popup;
    } else {
        popup = pdata->color_popup;
    }
    
    WidgetSetUserData(popup, parent);
    
    XmMenuPosition(popup, e);
    WidgetManage(popup);
}

static void cc_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
    Pen pen;
    Widget button = WidgetGetUserData(XtParent(w));
    Button_PData *pdata;
    
    pdata = WidgetGetUserData(button);
    pen = pdata->pen;
    pen.color = (long) client_data;
    
    SetPenChoice_int(button, &pen, TRUE);
}
#define MAX_PULLDOWN_LENGTH 30
void update_color_choice_popup(void)
{
    X11Stuff *xstuff = gapp->gui->xstuff;
    Project *pr = project_get_data(gproject_get_top(gapp->gp));
    unsigned int ci;

    if (pr && color_choice_popup) {
        int ncols = 4;
        Widget	*ws = NULL;
	Cardinal nw_old = 0, nw_new = pr->ncolors;
        
        XtVaGetValues(color_choice_popup,
            XtNnumChildren, &nw_old,
            XtNchildren, &ws,
            NULL);
    
        /* Delete extra button widgets, if any */
        if (nw_new < nw_old) {
            XtUnmanageChildren(ws + nw_new, nw_old - nw_new);

            for (ci = nw_new; ci < nw_old; ci++) {
                XtDestroyWidget(ws[ci]);
            }
        }
        
        /* Don't create too tall pulldowns */
        if (nw_new > MAX_PULLDOWN_LENGTH*ncols) {
            ncols = (nw_new + MAX_PULLDOWN_LENGTH - 1)/MAX_PULLDOWN_LENGTH;
        }
        
        XtVaSetValues(color_choice_popup,
                      XmNnumColumns, ncols,
                      XmNpacking, XmPACK_COLUMN,
                      NULL);
        
        /* Create new buttons, if needed */
        for (ci = nw_old; ci < nw_new; ci++) {
            Widget cb;
            cb = XmCreatePushButton(color_choice_popup, "cb", NULL, 0);
        }

        XtVaGetValues(color_choice_popup,
            XtNchildren, &ws,
            NULL);
        
        /* Manage newly added widgets */
        if (nw_new > nw_old) {
            XtManageChildren(ws + nw_old, nw_new - nw_old);
        }

        /* Paint/label new buttons */
        for (ci = 0; ci < pr->ncolors; ci++) {
            Colordef *c = &pr->colormap[ci];
            long bg, fg;
            long color = c->id;
            Widget cb = ws[ci];
            XmString str;

            bg = xvlibcolors[color];
	    if (get_rgb_intensity(&c->rgb) < 0.5) {
	        fg = WhitePixel(xstuff->disp, xstuff->screennumber);
	    } else {
	        fg = BlackPixel(xstuff->disp, xstuff->screennumber);
	    }
	    
            XtRemoveAllCallbacks(cb, XmNactivateCallback);
            XtAddCallback(cb, XmNactivateCallback, cc_cb, (XtPointer) color);
            
            str = XmStringCreateLocalized(c->cname);
            XtVaSetValues(cb, 
                XmNlabelString, str,
                XmNbackground, bg,
                XmNforeground, fg,
                NULL);
            XmStringFree(str);
        }
    }
}

static Widget CreateColorChoicePopup(Widget button)
{
    if (!color_choice_popup) {
        color_choice_popup =
            XmCreatePopupMenu(button, "colorPopupMenu", NULL, 0);
        update_color_choice_popup();
    } else {
        XmAddToPostFromList(color_choice_popup, button);
    }
    
    return color_choice_popup;
}

static Widget CreatePatternChoicePopup(Widget button)
{
    Widget popup;
    
    popup = XmCreatePopupMenu(button, "patternPopupMenu", NULL, 0);
    CreateLabel(popup, "Pattern:");
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
        
        ui->top = CreateDialog(app_shell, "Pen properties");
        
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
        
        pdata = WidgetGetUserData(ui->pen_button);
        
        pen = pdata->pen;
        
        SetOptionChoice(ui->color, pen.color);
        SetOptionChoice(ui->pattern, pen.pattern);
    }

    DialogRaise(ui->top);
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
    
    WidgetManage(button);
    
    return button;
}

int GetPenChoice(Widget pen_button, Pen *pen)
{
    Button_PData *pdata = WidgetGetUserData(pen_button);
    if (pdata) {
        *pen = pdata->pen;
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

void AddPenChoiceCB(Widget button, Pen_CBProc cbproc, void *anydata)
{
    Button_PData *pdata = WidgetGetUserData(button);
    
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
    char *buf;

    if (!q) {
        return NULL;
    }

    buf = (char *) xmalloc(strlen(QIDSTR(q)) + 128);
    if (!buf) {
        return NULL;
    }
    
    if (quark_fid_get(q) == QFlavorSSD) {
        sprintf(buf, "SSD \"%s\" (%d x %d)", QIDSTR(q),
            ssd_get_ncols(q), ssd_get_nrows(q));

        (*rid)++;

        return buf;
    } else {
        return NULL;
    }
}

StorageStructure *CreateSSDChoice(Widget parent, char *labelstr, int type)
{
    StorageStructure *ss;
    int nvisible;
    
    nvisible = (type == LIST_TYPE_SINGLE) ? 4 : 6; 

    ss = CreateStorageChoice(parent, labelstr, type, nvisible);
    SetStorageChoiceLabeling(ss, ssd_labeling);
    SetStorageChoiceQuark(ss, gproject_get_top(gapp->gp));

    nssd_selectors++;
    ssd_selectors =
        xrealloc(ssd_selectors, nssd_selectors*sizeof(StorageStructure *));
    ssd_selectors[nssd_selectors - 1] = ss;

    AddHelpCB(ss->rc, "doc/UsersGuide.html#ssd-selector");

    return ss;
}

int GetSSDColChoices(SSDColStructure *sc, Quark **ssd, int **cols)
{
    if (GetSingleStorageChoice(sc->ssd_sel, ssd) != RETURN_SUCCESS) {
        return -1;
    } else {
        return GetListChoices(sc->col_sel, cols);
    }
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
        xdrawgraph(gapp->gp);
    }
}

static void g_focus_cb(Widget but, void *udata)
{
    gss_any_cb(udata, GSS_FOCUS_CB);
}

static void g_popup_cb(StorageStructure *ss, int nselected)
{
    GSSData *gssdata = (GSSData *) ss->data;
    
    WidgetSetSensitive(gssdata->focus_bt, (nselected == 1));
}

static void g_new_cb(Widget but, void *udata)
{
    graph_next(gproject_get_top(gapp->gp));
    snapshot_and_update(gapp->gp, TRUE);
}

static char *graph_labeling(Quark *q, unsigned int *rid)
{
    char *buf;

    if (!q) {
        return NULL;
    }

    buf = (char *) xmalloc(strlen(QIDSTR(q)) + 128);
    if (!buf) {
        return NULL;
    }

    if (quark_fid_get(q) == QFlavorGraph) {
        sprintf(buf, "Graph \"%s\" (type: %s, sets: %d)",
            QIDSTR(q),
            graph_types(gapp->grace, graph_get_type(q)), quark_get_number_of_descendant_sets(q));

        (*rid)++;

        return buf;
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
    SetStorageChoiceQuark(ss, gproject_get_top(gapp->gp));
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
    char *buf;

    if (!q) {
        return NULL;
    }

    buf = (char *) xmalloc(strlen(QIDSTR(q)) + 128);
    if (!buf) {
        return NULL;
    }

    if (quark_fid_get(q) == QFlavorFrame) {
        sprintf(buf, "Frame \"%s\"", QIDSTR(q));

        (*rid)++;

        return buf;
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
    SetStorageChoiceQuark(ss, gproject_get_top(gapp->gp));
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


#define SSS_NEWF_CB          1

typedef struct {
    StorageStructure *graphss;
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
    Quark *gr = get_set_choice_gr(ss);
    
    switch (cbtype) {
    case SSS_NEWF_CB:
        create_leval_frame(NULL, gr);
        break;
    }
    
    snapshot_and_update(gapp->gp, TRUE);
}

static void s_newF_cb(Widget but, void *udata)
{
    sss_any_cb(udata, SSS_NEWF_CB);
}

static char *set_labeling(Quark *q, unsigned int *rid)
{
    char *buf;

    if (!q) {
        return NULL;
    }

    buf = (char *) xmalloc(strlen(QIDSTR(q)) + 128);
    if (!buf) {
        return NULL;
    }

    if (quark_fid_get(q) == QFlavorSet) {
        set *p = set_get_data(q);

        sprintf(buf, "Set \"%s\" (type: %s, length: %d)",
            QIDSTR(q), set_type_descr(gapp->grace, p->type),
            set_get_length(q));

        (*rid)++;
        
        return buf;
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
    AddHelpCB(ss->rc, "doc/UsersGuide.html#set-selector");

    nset_selectors++;
    set_selectors =
        xrealloc(set_selectors, nset_selectors*sizeof(StorageStructure *));
    set_selectors[nset_selectors - 1] = ss;
    
    sssdata = xmalloc(sizeof(SSSData));
    ss->data = sssdata;
    sssdata->graphss = graphss;
    
    popup = ss->popup;
    
    CreateMenuSeparator(popup);

    submenupane = CreateMenu(popup, "Create new", '\0', FALSE);
    CreateMenuButton(submenupane, "By formula", '\0', s_newF_cb, ss);

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
    WidgetManage(rc);

    return retval;
}


SSDSetStructure *CreateSSDSetSelector(Widget parent, char *s, int sel_type)
{
    SSDSetStructure *retval;
    Widget rc;

    retval = xmalloc(sizeof(SSDSetStructure));
    retval->frame = CreateFrame(parent, s);
    rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, retval->frame, NULL);
    retval->ssd_sel = CreateSSDChoice(rc, "SSD:", LIST_TYPE_SINGLE);
    retval->set_sel = CreateSetChoice(rc, "Set:", sel_type, retval->ssd_sel);
    AddStorageChoiceCB(retval->ssd_sel, update_sets_cb, (void *) retval);
    UpdateSetChoice(retval->set_sel);
    retval->set_sel->governor = retval->ssd_sel;
    WidgetManage(rc);

    return retval;
}



ListStructure *CreateColChoice(Widget parent, char *labelstr, int type)
{
    int nvisible;
    ListStructure *retval;
    
    nvisible = 6; 

    retval = CreateListChoice(parent, labelstr, type, nvisible, 0, NULL);
    
    return retval;
}

void UpdateColChoice(ListStructure *sel, const Quark *ssd)
{
    unsigned int i, ncols;
    LabelOptionItem *items;

    if (ssd) {
        ncols = ssd_get_ncols(ssd);
        items = xmalloc(ncols*sizeof(LabelOptionItem));
        for (i = 0; i < ncols; i++) {
            char *label, *s, buf[32];
            items[i].value = i;
            label = ssd_get_col_label(ssd, i);
            if (string_is_empty(label)) {
                sprintf(buf, "#%d", i + 1);
                s = copy_string(NULL, buf);
            } else {
                s = copy_string(NULL, label);
            }
            items[i].label = s;
        }
    } else {
        ncols = 0;
        items = NULL;
    }
    UpdateListChoice(sel, ncols, items);
    
    for (i = 0; i < ncols; i++) {
        xfree(items[i].label);
    }
    xfree(items);
    
    sel->anydata = (void *) ssd;
}

static void update_ssd_cb(StorageStructure *ss, int n, Quark **values, void *data)
{
    SSDColStructure *gs = (SSDColStructure *) data;
    Quark *ssd;
    
    if (n == 1) {
        ssd = values[0];
    } else {
        ssd = NULL;
    }
    
    UpdateColChoice(gs->col_sel, ssd);
}

SSDColStructure *CreateSSDColSelector(Widget parent, char *s, int sel_type)
{
    SSDColStructure *retval;
    Widget rc;

    retval = xmalloc(sizeof(SSDColStructure));
    retval->frame = CreateFrame(parent, s);
    rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, retval->frame, NULL);
    retval->ssd_sel = CreateSSDChoice(rc, "SSData:", LIST_TYPE_SINGLE);
    retval->col_sel = CreateColChoice(rc,
        sel_type == LIST_TYPE_SINGLE ? "Column:" : "Column(s):", sel_type);
    AddStorageChoiceCB(retval->ssd_sel, update_ssd_cb, (void *) retval);

    WidgetManage(rc);

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
    retval->src  = CreateSSDSetSelector(retval->form, "Source", sel_type);
    retval->dest = CreateSSDSetSelector(retval->form, "Destination", sel_type);
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

    WidgetManage(retval->form);

    return retval;
}

void update_color_selectors(void)
{
    X11Stuff *xstuff = gapp->gui->xstuff;
    unsigned int i;
    long bg, fg;
    Project *pr = project_get_data(gproject_get_top(gapp->gp));
    
    ncolor_option_items = pr->ncolors;

    color_option_items = xrealloc(color_option_items,
                                    ncolor_option_items*sizeof(LabelOptionItem));
    for (i = 0; i < pr->ncolors; i++) {
        Colordef *c = &pr->colormap[i];
        color_option_items[i].value = c->id;
        color_option_items[i].label = c->cname;

        bg = xvlibcolors[c->id];
        if (get_rgb_intensity(&c->rgb) < 0.5) {
            fg = WhitePixel(xstuff->disp, xstuff->screennumber);
        } else {
            fg = BlackPixel(xstuff->disp, xstuff->screennumber);
        }
        color_option_items[i].background = bg;
        color_option_items[i].foreground = fg;
    }
    
    for (i = 0; i < ncolor_selectors; i++) {
        UpdateLabelOptionChoice(color_selectors[i], 
                            ncolor_option_items, color_option_items);
    }
    
    update_color_choice_popup();
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
    
    retvalp = CreateLabelOptionChoice(parent, s, 4, 
                                ncolor_option_items, color_option_items);
    OptionChoiceSetColorUpdate(retvalp, TRUE);

    color_selectors[ncolor_selectors - 1] = retvalp;
    
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
    LabelOptionItem *oi = NULL;
    va_list var;
    char *s;

    va_start(var, labelstr);
    while ((s = va_arg(var, char *)) != NULL) {
        nchoices++;
        oi = xrealloc(oi, nchoices*sizeof(LabelOptionItem));
        oi[nchoices - 1].value = nchoices - 1;
        oi[nchoices - 1].label = copy_string(NULL, s);
    }
    va_end(var);

    retval = CreateLabelOptionChoice(parent, labelstr, 1, nchoices, oi);
    
    while (nchoices) {
        nchoices--;
	xfree(oi[nchoices].label);
    }
    xfree(oi);
    
    return retval;
}

static void format_call_cb(FormatStructure *fstr)
{
    if (fstr->cb_proc) {
        Format format;
        format.type    = GetOptionChoice(fstr->type);
        format.prec   = GetOptionChoice(fstr->prec);
        format.fstring = TextGetString(fstr->fstring);
        
        fstr->cb_proc(fstr, &format, fstr->cb_data);
        
        xfree(format.fstring);
    }
}

static void format_oc_cb(OptionStructure *opt, int a, void *data)
{
    FormatStructure *fstr = (FormatStructure *) data;
    if (!fstr) {
        return;
    }
    
    if (opt == fstr->type) {
        WidgetSetSensitive(fstr->fstring->form,
            a == FORMAT_DATETIME || a == FORMAT_GEOGRAPHIC);
    }
    
    format_call_cb(fstr);
}

static void format_text_cb(TextStructure *cst, char *s, void *data)
{
    FormatStructure *fstr = (FormatStructure *) data;
    if (!fstr) {
        return;
    }
    
    format_call_cb(fstr);
}

FormatStructure *CreateFormatChoice(Widget parent)
{
    FormatStructure *retval;
    Widget rc, rc1;
    
    retval = xmalloc(sizeof(FormatStructure));
    
    rc = CreateVContainer(parent);
    rc1 = CreateHContainer(rc);
    retval->type = CreateLabelOptionChoice(rc1, "Type:",
        1, NUMBER_OF_FORMATTYPES, fmt_option_items);
    AddOptionChoiceCB(retval->type, format_oc_cb, retval);
    retval->prec = CreatePrecisionChoice(rc1, "Precision:");
    AddOptionChoiceCB(retval->prec, format_oc_cb, retval);
    retval->fstring = CreateText(rc, "Format string:");
    AddTextActivateCB(retval->fstring, format_text_cb, retval);
    
    return retval;
}

void SetFormatChoice(FormatStructure *fstr, const Format *format)
{
    SetOptionChoice(fstr->type, format->type);
    SetOptionChoice(fstr->prec, format->prec);
    TextSetString(fstr->fstring, format->fstring);
    WidgetSetSensitive(fstr->fstring->form,
        format->type == FORMAT_DATETIME || format->type == FORMAT_GEOGRAPHIC);
}

Format *GetFormatChoice(FormatStructure *fstr)
{
    Format *format = format_new();
    if (format) {
        format->type    = GetOptionChoice(fstr->type);
        format->prec   = GetOptionChoice(fstr->prec);
        format->fstring = TextGetString(fstr->fstring);
    }
    
    return format;
}

void AddFormatChoiceCB(FormatStructure *fstr, Format_CBProc cbproc, void *data)
{
    fstr->cb_proc = cbproc;
    fstr->cb_data = data;
}


static LabelOptionItem as_option_items[4] = 
{
    {AUTOSCALE_NONE, "None"},
    {AUTOSCALE_X,    "X"},
    {AUTOSCALE_Y,    "Y"},
    {AUTOSCALE_XY,   "XY"}
};

OptionStructure *CreateASChoice(Widget parent, char *s)
{
    OptionStructure *retval;
    
    retval = CreateLabelOptionChoice(parent, s, 1, 4, as_option_items);
    /* As init value, use this */
    SetOptionChoice(retval, gapp->rt->autoscale_onread);
    
    return(retval);
}

OptionStructure *CreatePrecisionChoice(Widget parent, char *s)
{
    return CreateLabelOptionChoiceVA(parent, s,
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

OptionStructure *CreatePaperOrientationChoice(Widget parent, char *s)
{
    return CreateLabelOptionChoiceVA(parent, s,
        "Landscape", PAGE_ORIENT_LANDSCAPE,
        "Portrait",  PAGE_ORIENT_PORTRAIT,
        NULL);
}

OptionStructure *CreatePaperFormatChoice(Widget parent, char *s)
{
    return CreateLabelOptionChoiceVA(parent, s,
        "Custom", PAGE_FORMAT_CUSTOM,
        "Letter", PAGE_FORMAT_USLETTER,
        "A4",     PAGE_FORMAT_A4,
        NULL);
}


SpinStructure *CreateAngleChoice(Widget parent, char *s)
{
    return CreateSpinChoice(parent, s, 5, SPIN_TYPE_FLOAT, -360.0, 360.0, 10.0);
}

double GetAngleChoice(SpinStructure *sp)
{
    return SpinChoiceGetValue(sp);
}

void SetAngleChoice(SpinStructure *sp, double angle)
{
    if (angle < -360.0 || angle > 360.0) {
        angle = fmod(angle, 360.0);
    }
    SpinChoiceSetValue(sp, angle);
}

SpinStructure *CreateCharSizeChoice(Widget parent, char *s)
{
    return CreateSpinChoice(parent, s, 4, SPIN_TYPE_FLOAT, 0.0, 100.0, 0.25);
}

Pixmap XpmToPixmap(char **xpm)
{
    X11Stuff *xstuff = gapp->gui->xstuff;
    Pixel bg;
    XpmColorSymbol transparent;
    XpmAttributes attrib;
    Pixmap pixmap;

    XtVaGetValues(app_shell, XtNbackground, &bg, NULL);
    transparent.name  = NULL;
    transparent.value = "None";
    transparent.pixel = bg;
    attrib.colorsymbols = &transparent;
    attrib.valuemask    = XpmColorSymbols;
    attrib.numsymbols   = 1;

    XpmCreatePixmapFromData(xstuff->disp, xstuff->root,
                            xpm, &pixmap, NULL, &attrib);

    return pixmap;
}

static Widget CreateCommandButtons(Widget parent, int n, Widget * buts, char **l)
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
    WidgetManage(form);
    XtVaGetValues(buts[0], XmNheight, &h, NULL);
    XtVaSetValues(form, XmNpaneMaximum, h, XmNpaneMinimum, h, NULL);
    
    return form;
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
        WidgetUnmanage(XtParent(cbdata->form));
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

    FormAddVChild(form, container);
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
    AddButtonCB(aacbut[2], destroy_dialog_cb, form);
    
    WidgetManage(container);
    WidgetManage(form);
    
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
    
    error = FALSE;
    
    tddata = tdialog->get_cb(tdialog->gui);
    if (!tddata) {
        error = TRUE;
    }
    
    if (!error) {
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
    }
    
    if (nssrc > 0) {
        xfree(srcsets);
        xfree(destsets);
    }
    
    if (tddata) {
        tdialog->free_cb(tddata);
    }

    snapshot_and_update(gapp->gp, TRUE);
    
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

    retval->form = CreateDialog(parent, s);

    retval->menubar = CreateMenuBar(retval->form);
    FormAddVChild(retval->form, retval->menubar);
    
    retval->srcdest = CreateSrcDestSelector(retval->form, sel_type);
    FormAddVChild(retval->form, retval->srcdest->form);

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
    Quark *srcssd, *destssd;
    
    if (GetSingleStorageChoice(tdialog->srcdest->src->ssd_sel, &srcssd)
        != RETURN_SUCCESS) {
        errmsg("No source SSD selected");
	return RETURN_FAILURE;
    }
    
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
    
    if (GetSingleStorageChoice(tdialog->srcdest->dest->ssd_sel, &destssd)
        != RETURN_SUCCESS) {
        destssd = gapp_ssd_new(quark_parent_get(srcssd));
        if (!destssd) {
            xfree(*srcsets);
            errmsg("Cannot create new SSD");
	    return RETURN_FAILURE;
        }
    } else {
        /* check for mutually exclusive selections */
        if (tdialog->exclusive && destssd == srcssd) {
            xfree(*srcsets);
            if (nsdest) {
                xfree(*destsets);
            }
            errmsg("Source and destination SSD's are the same");
	    return RETURN_FAILURE;
        }
    }

    if (nsdest == 0) {
        ssd_set_indexed(destssd, TRUE);
        if (!ssd_is_indexed(destssd)) {
            if (!ssd_add_col(destssd, FFORMAT_NUMBER)) {
	        return RETURN_FAILURE;
            }
        }
        
        *destsets = xmalloc((*nssrc)*sizeof(Quark *));
        for (i = 0; i < *nssrc; i++) {
            if (ssd_add_col(destssd, FFORMAT_NUMBER)) {
                Dataset *dsp;
                (*destsets)[i] = gapp_set_new(destssd);
                dsp = set_get_dataset((*destsets)[i]);
                dsp->cols[0] = 0;
                dsp->cols[1] = ssd_get_ncols(destssd) - 1;
            }
        }
        
        update_ssd_selectors(gproject_get_top(gapp->gp));
        
        SelectStorageChoice(tdialog->srcdest->dest->ssd_sel, destssd);
        SelectStorageChoices(tdialog->srcdest->dest->set_sel, *nssrc, *destsets);
    }
    
    return RETURN_SUCCESS;
}

void RaiseTransformationDialog(TransformStructure *tdialog)
{
    DialogRaise(tdialog->form);
}

static Widget *savewidgets = NULL;
static int nsavedwidgets = 0;

static void savewidget(Widget w)
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

#if 0
static void deletewidget(Widget w)
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
#endif

static void windowCloseCB(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget_CBData *cbdata = (Widget_CBData *) client_data;

    cbdata->cbproc(cbdata);
}
/*
 * handle the close item on the WM menu
 */
void AddWindowCloseCB(Widget w, Widget_CBProc cbproc, void *anydata)
{
    X11Stuff *xstuff = gapp->gui->xstuff;
    Atom WM_DELETE_WINDOW;
    Widget_CBData *cbdata;

    cbdata = (Widget_CBData *) xmalloc(sizeof(Widget_CBData));
    cbdata->w = w;
    cbdata->cbproc = cbproc;
    cbdata->anydata = anydata;
    
    XtVaSetValues(w, XmNdeleteResponse, XmDO_NOTHING, NULL);

    WM_DELETE_WINDOW = XmInternAtom(xstuff->disp, "WM_DELETE_WINDOW", False);
    XmAddWMProtocolCallback(w, WM_DELETE_WINDOW, windowCloseCB, cbdata);
    
    savewidget(w);
}

void DefineDialogCursor(Cursor c)
{
    X11Stuff *xstuff = gapp->gui->xstuff;
    int i;
    
    for (i = 0; i < nsavedwidgets; i++) {
	XDefineCursor(xstuff->disp, XtWindow(savewidgets[i]), c);
    }
    XFlush(xstuff->disp);
}

void UndefineDialogCursor(void)
{
    X11Stuff *xstuff = gapp->gui->xstuff;
    int i;
    
    for (i = 0; i < nsavedwidgets; i++) {
	XUndefineCursor(xstuff->disp, XtWindow(savewidgets[i]));
    }
    XFlush(xstuff->disp);
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
    X11Stuff *xstuff = gapp->gui->xstuff;
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
            whelp = XtParent(whelp);
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
    WidgetUnmanage(w);
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
    	LabelSetString(but, s1);
    } else {    
    	LabelSetString(but, "OK");
    }
    
    but = XtNameToWidget(dialog, "Cancel");
    if (s2) {
    	LabelSetString(but, s2);
    } else {    
    	LabelSetString(but, "Cancel");
    }
    
    but = XtNameToWidget(dialog, "Help");
    if (ha) {
        WidgetManage(but);
    } else {    
        WidgetUnmanage(but);
    }

    WidgetManage(dialog);
    XMapRaised(XtDisplay(dialog), XtWindow(dialog));
    
    XtAddGrab(XtParent(dialog), True, False);
    while (keep_grab || XtAppPending(app_con)) {
	XtAppNextEvent(app_con, &event);
	XtDispatchEvent(&event);
    }
    return yesno_retval;
}

void update_set_lists(Quark *gr)
{
    update_set_selectors(gr);
    
    snapshot_and_update(gapp->gp, FALSE);
}

void update_all(void)
{
    if (!gapp->gui->inwin) {
        return;
    }

    if (gui_is_page_free(gapp->gui)) {
        sync_canvas_size(gapp);
    }
    
    update_ssd_selectors(gproject_get_top(gapp->gp));
    update_frame_selectors(gproject_get_top(gapp->gp));
    update_graph_selectors(gproject_get_top(gapp->gp));
    update_set_selectors(NULL);

    if (gapp->gui->need_colorsel_update == TRUE) {
        init_xvlibcolors();
        update_color_selectors();
        gapp->gui->need_colorsel_update = FALSE;
    }

    if (gapp->gui->need_fontsel_update == TRUE) {
        update_font_selectors();
        gapp->gui->need_fontsel_update = FALSE;
    }

    update_undo_buttons(gapp->gp);
    update_props_items();
    set_left_footer(NULL);
    update_app_title(gapp->gp);
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

/* what a mess... */
void unlink_ssd_ui(Quark *q)
{
    GUI *gui = gui_from_quark(q);
    if (gui && gui->eui && gui->eui->ssd_ui) {
        if (gui->eui->ssd_ui->q == q) {
            gui->eui->ssd_ui->q = NULL;
        }
        if (gui->eui->ssd_ui->col_sel->anydata == q) {
            gui->eui->ssd_ui->col_sel->anydata = NULL;
        }
    }
}


/*
 * action routines, to be used with translations
 */

/* This is for buggy Motif-2.1 that crashes with Ctrl+<Btn1Down> */
static void do_nothing_action(Widget w, XEvent *e, String *par, Cardinal *npar)
{
}

static void pageUp_action(Widget w, XEvent *event, String *args, Cardinal *nArgs)
{
    Widget scrolledWindow, scrollBar;
    String al[1];
    
    al[0] = "Up";
    scrolledWindow = XtParent(w);
    scrollBar = XtNameToWidget (scrolledWindow, "VertScrollBar");
    if (scrollBar)
        XtCallActionProc(scrollBar, "PageUpOrLeft", event, al, 1) ;
    return;
}

static void pageDown_action(Widget w, XEvent *event, String *args, Cardinal *nArgs)
{
    Widget scrolledWindow, scrollBar;
    String al[1];
    
    al[0] = "Down";
    scrolledWindow = XtParent(w);
    scrollBar = XtNameToWidget (scrolledWindow, "VertScrollBar");
    if (scrollBar)
        XtCallActionProc(scrollBar, "PageDownOrRight", event, al, 1) ;
    return;
}

static void scrollUp_action(Widget w, XEvent *event, String *args, Cardinal *nArgs)
{
    Widget scrolledWindow, scrollBar;
    String al[1];
    int i, nLines;
    
    if (*nArgs == 0 || sscanf(args[0], "%d", &nLines) != 1)
       return;
    al[0] = "Up";
    scrolledWindow = XtParent(w);
    scrollBar = XtNameToWidget (scrolledWindow, "VertScrollBar");
    if (scrollBar)
        for (i=0; i<nLines; i++)
            XtCallActionProc(scrollBar, "IncrementUpOrLeft", event, al, 1) ;
    return;
}

static void scrollDown_action(Widget w, XEvent *event, String *args, Cardinal *nArgs)
{
    Widget scrolledWindow, scrollBar;
    String al[1];
    int i, nLines;
    
    if (*nArgs == 0 || sscanf(args[0], "%d", &nLines) != 1)
       return;
    al[0] = "Down";
    scrolledWindow = XtParent(w);
    scrollBar = XtNameToWidget (scrolledWindow, "VertScrollBar");
    if (scrollBar)
        for (i=0; i<nLines; i++)
            XtCallActionProc(scrollBar, "IncrementDownOrRight", event, al, 1) ;
    return;
}

static XtActionsRec dummy_actions[] = {
    {"do_nothing", do_nothing_action}
};

static XtActionsRec list_select_actions[] = {
    {"list_activate_action",        list_activate_action       },
    {"list_selectall_action",       list_selectall_action      },
    {"list_unselectall_action",     list_unselectall_action    },
    {"list_invertselection_action", list_invertselection_action}
};

static XtActionsRec sw_scroll_actions[] = {
    {"scrolled-window-scroll-up",   scrollUp_action  },
    {"scrolled-window-page-up",     pageUp_action    },
    {"scrolled-window-scroll-down", scrollDown_action},
    {"scrolled-window-page-down",   pageDown_action  } 
};

void InitWidgets(void)
{
    XtAppAddActions(app_con, dummy_actions, XtNumber(dummy_actions));
    XtAppAddActions(app_con, list_select_actions, XtNumber(list_select_actions));
    XtAppAddActions(app_con, sw_scroll_actions, XtNumber(sw_scroll_actions));
}

void set_title(char *title, char *icon_name)
{
    XtVaSetValues(app_shell, XtNtitle, title, XtNiconName, icon_name, NULL);
}

/* Tree Widget */
static void tree_refresh_timer_proc(void *anydata)
{
    ListTreeRefreshOn(anydata);
    ListTreeRefresh(anydata);
    ListTreeRefreshOff(anydata);
}

Widget CreateTree(Widget parent)
{
    Timer_CBdata *tcbdata;
    Widget w;

    w = XmCreateScrolledListTree(parent, "tree", NULL, 0);
    ListTreeRefreshOff(w);

    tcbdata = CreateTimer(100 /* 0.1 second */, tree_refresh_timer_proc, w);

    WidgetSetUserData(w, tcbdata);

    return w;
}

TreeItem *TreeInsertItem(Widget w, TreeItem *parent, Quark *q, int row)
{
    ListTreeItem *item;
    ListTreeItem *parent_item = parent;

    if (row < 0) {
        item = ListTreeInsert(w, parent, "", parent_item->count + row + 1);
    } else {
        item = ListTreeInsert(w, parent, "", row);
    }
    item->user_data = q;

    return item;
}

void TreeDeleteItem(Widget w, TreeItem *item)
{
    ListTreeItem *titem = item;

    if (!item) {
        titem = ListTreeFirstItem(w);
    }

    ListTreeDelete(w, titem);
}

void TreeSetItemOpen(Widget w, TreeItem *item, int open)
{
    ListTreeItem *titem = item;

    titem->open = open;
}

void TreeSetItemText(Widget w, TreeItem *item, char *text)
{
    ListTreeRenameItem(w, item, text);
}

void TreeSetItemPixmap(Widget w, TreeItem *item, Pixmap pixmap)
{
    ListTreeSetItemPixmaps(w, item, pixmap, pixmap);
}

Quark *TreeGetQuark(TreeItem *item)
{
    ListTreeItem *titem = item;

    return titem->user_data;
}

void TreeGetHighlighted(Widget w, TreeItemList *items)
{
    int i;
    ListTreeMultiReturnStruct ret;

    ListTreeGetHighlighted(w, &ret);
    items->count = ret.count;
    items->items = (TreeItem **) xmalloc(items->count*sizeof(TreeItem *));
    for (i = 0; i < items->count; i++) {
        items->items[i] = (TreeItem *) ret.items[i];
    }
}

void TreeHighlightItem(Widget w, TreeItem *item)
{
    ListTreeItem *titem = item;
    ListTreeMultiReturnStruct ret;

    if (!item) {
        titem = ListTreeFirstItem(w);
    }

    ListTreeHighlightItemMultiple(w, titem);

    ListTreeGetHighlighted(w, &ret);
    XtCallCallbacks(w, XtNhighlightCallback, (XtPointer) &ret);
}

void TreeClearSelection(Widget w)
{
    ListTreeClearHighlighted(w);
}

void TreeScrollToItem(Widget w, TreeItem *item)
{
    ListTreeItem *titem = item;
    int top, visible;

    ListTreeRefreshOn(w);
    ListTreeRefresh(w);
    ListTreeRefreshOff(w);

    XtVaGetValues(w,
                  XtNtopItemPosition, &top,
                  XtNvisibleItemCount, &visible,
                  NULL);

    if (titem->count < top) {
        ListTreeSetPos(w, titem);
    } else
        if (titem->count >= top + visible) {
            ListTreeSetBottomPos(w, titem);
        }
}

void TreeRefresh(Widget w)
{
    TimerStart(WidgetGetUserData(w));
}

static void tree_context_menu_cb_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    Tree_CBData *cbdata = (Tree_CBData *) client_data;
    ListTreeItemReturnStruct *ret = (ListTreeItemReturnStruct *) call_data;
    XButtonEvent *xbe = (XButtonEvent *) ret->event;

    TreeEvent event;
    event.w = cbdata->w;
    event.anydata = cbdata->anydata;
    event.udata = xbe;

    cbdata->cbproc(&event);
}

void AddTreeContextMenuCB(Widget w, Tree_CBProc cbproc, void *anydata)
{
    Tree_CBData *cbdata;

    cbdata = (Tree_CBData *) xmalloc(sizeof(Tree_CBData));
    cbdata->w = w;
    cbdata->cbproc = cbproc;
    cbdata->anydata = anydata;

    XtAddCallback(w, XtNmenuCallback, tree_context_menu_cb_proc, cbdata);
}

static void tree_highlight_cb_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    Tree_CBData *cbdata = (Tree_CBData *) client_data;

    TreeEvent event;
    event.w = cbdata->w;
    event.anydata = cbdata->anydata;

    cbdata->cbproc(&event);
}

void AddTreeHighlightItemsCB(Widget w, Tree_CBProc cbproc, void *anydata)
{
    Tree_CBData *cbdata;

    cbdata = (Tree_CBData *) xmalloc(sizeof(Tree_CBData));
    cbdata->w = w;
    cbdata->cbproc = cbproc;
    cbdata->anydata = anydata;

    XtAddCallback(w, XtNhighlightCallback, tree_highlight_cb_proc, cbdata);
}

static void tree_drop_items_cb_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    Tree_CBData *cbdata = (Tree_CBData *) client_data;
    ListTreeDropStruct *cbs = (ListTreeDropStruct *) call_data;

    TreeEvent event;
    event.w = cbdata->w;
    event.anydata = cbdata->anydata;
    event.udata = cbs->item;

    switch (cbs->operation) {
    case XmDROP_MOVE:
        event.drop_action = DROP_ACTION_MOVE;
        break;
    case XmDROP_COPY:
        event.drop_action = DROP_ACTION_COPY;
        break;
    default:
        cbs->ok = FALSE;
        return;
    }

    if (cbdata->cbproc(&event)) {
        cbs->ok = TRUE;
    } else {
        cbs->ok = FALSE;
    }
}

void AddTreeDropItemsCB(Widget w, Tree_CBProc cbproc, void *anydata)
{
    Tree_CBData *cbdata;

    cbdata = (Tree_CBData *) xmalloc(sizeof(Tree_CBData));
    cbdata->w = w;
    cbdata->cbproc = cbproc;
    cbdata->anydata = anydata;

    XtAddCallback(w, XtNdropCallback, tree_drop_items_cb_proc, cbdata);
}

/* Table Widget */
typedef struct {
    int default_col_width;
    int default_col_label_alignment;
    int nrows_visible;
    int ncols_visible;
} TableData;

Widget CreateTable(char *name, Widget parent, int nrows, int ncols, int nrows_visible, int ncols_visible)
{
    Widget w;
    TableData *td;

    td = (TableData*) xmalloc(sizeof(TableData));
    td->default_col_width = 5;
    td->default_col_label_alignment = ALIGN_BEGINNING;
    td->nrows_visible = nrows_visible;
    td->ncols_visible = ncols_visible;

    w = XtVaCreateManagedWidget(name,
        xbaeMatrixWidgetClass, parent,
        XmNrows, nrows,
        XmNvisibleRows, nrows_visible,
        XmNcolumns, ncols,
        XmNvisibleColumns, ncols_visible,
        NULL);

    WidgetSetUserData(w, td);

    return w;
}

static char tfield_translation_table[] = "\
    <Key>osfCancel                : CancelEdit(True)\n\
    <Key>osfActivate              : EditCell(Down)\n\
    <Key>osfUp                    : EditCell(Up)\n\
    <Key>osfDown                  : EditCell(Down)\n\
    ~Shift ~Meta ~Alt <Key>Return : EditCell(Down)";

void TableSSDInit(Widget w)
{
    Widget tfield;

    XtVaSetValues(w,
#if 0
                  XmNhorizontalScrollBarDisplayPolicy, XmDISPLAY_NONE,
                  XmNverticalScrollBarDisplayPolicy, XmDISPLAY_NONE,
#endif
                  XmNbuttonLabels, True,
                  XmNallowColumnResize, True,
                  XmNgridType, XmGRID_CELL_SHADOW,
                  XmNcellShadowType, XmSHADOW_ETCHED_OUT,
                  XmNcellShadowThickness, 1,
                  XmNcellMarginHeight, 1,
                  XmNcellMarginWidth, 1,
                  XmNshadowThickness, 1,
                  XmNaltRowCount, 0,
                  XmNcalcCursorPosition, True,
                  XmNtraverseFixedCells, True,
                  NULL);

    tfield = XtNameToWidget(w, "textField");
    XtOverrideTranslations(tfield,
        XtParseTranslationTable(tfield_translation_table));
}

void TableFontInit(Widget w)
{
    XtVaSetValues(w,
                  XmNfill, True,
                  XmNgridType, XmGRID_CELL_SHADOW,
                  XmNcellShadowType, XmSHADOW_ETCHED_OUT,
                  XmNcellShadowThickness, 2,
                  XmNaltRowCount, 0,
                  NULL);
}

void TableDataSetPropInit(Widget w)
{
    XtVaSetValues(w,
                  XmNshowArrows, True,
                  XmNallowColumnResize, True,
                  XmNgridType, XmGRID_COLUMN_SHADOW,
                  XmNcellShadowType, XmSHADOW_OUT,
                  XmNcellShadowThickness, 1,
                  XmNaltRowCount, 1,
                  XmNtraversalOn, False,
                  NULL);
}

void TableLevalInit(Widget w)
{
    XtVaSetValues(w,
                  XmNgridType, XmGRID_CELL_SHADOW,
                  XmNcellShadowType, XmSHADOW_ETCHED_OUT,
                  XmNcellShadowThickness, 2,
                  XmNaltRowCount, 0,
                  XmNallowColumnResize, True,
                  NULL);
}

void TableOptionChoiceInit(Widget w)
{
    XtVaSetValues(w,
                  XmNgridType, XmGRID_CELL_SHADOW,
                  XmNcellShadowType, XmSHADOW_ETCHED_OUT,
                  XmNcellShadowThickness, 1,
                  XmNaltRowCount, 0,
                  XmNtraversalOn, False,
                  NULL);
}

static int align_to_xmalign(int align)
{
    switch(align) {
    case ALIGN_BEGINNING:
        return XmALIGNMENT_BEGINNING;
        break;
    case ALIGN_CENTER:
        return XmALIGNMENT_CENTER;
        break;
    case ALIGN_END:
        return XmALIGNMENT_END;
        break;
    default:
        return XmALIGNMENT_BEGINNING;
    }
}

int TableGetNrows(Widget w)
{
    int nr;

    XtVaGetValues(w, XmNrows, &nr, NULL);

    return nr;
}

int TableGetNcols(Widget w)
{
    int nc;

    XtVaGetValues(w, XmNcolumns, &nc, NULL);

    return nc;
}

void TableAddRows(Widget w, int nrows)
{
    XbaeMatrixAddRows(w, TableGetNrows(w), NULL, NULL, NULL, nrows);
}

void TableDeleteRows(Widget w, int nrows)
{
    XbaeMatrixDeleteRows(w, TableGetNrows(w) - nrows, nrows);
}

void TableAddCols(Widget w, int ncols)
{
    TableData *td;
    short *widths;
    int i;
    unsigned char *alignment, xm_alignment;

    td = (TableData*) WidgetGetUserData(w);
    xm_alignment = align_to_xmalign(td->default_col_label_alignment);
    widths = xmalloc(ncols*SIZEOF_SHORT);
    alignment = xmalloc(ncols);

    for (i = 0; i < ncols; i++) {
        widths[i] = td->default_col_width;
        alignment[i] = xm_alignment;
    }

    XbaeMatrixAddColumns(w, TableGetNcols(w), NULL, NULL, widths, NULL, NULL, alignment, NULL, ncols);

    xfree(alignment);
    xfree(widths);
}

void TableDeleteCols(Widget w, int ncols)
{
    XbaeMatrixDeleteColumns(w, TableGetNcols(w) - ncols, ncols);
}

void TableGetCellDimentions(Widget w, int *cwidth, int *cheight)
{
    int x0, x1, y0, y1;

    XbaeMatrixRowColToXY(w, 0, 0, &x0, &y0);
    XbaeMatrixRowColToXY(w, 1, 1, &x1, &y1);
    *cwidth  = x1 - x0;
    *cheight = y1 - y0;
}

void TableSetColWidths(Widget w, int *widths)
{
    int i, ncols;
    short *short_widths;

    ncols = TableGetNcols(w);

    short_widths = xmalloc(ncols*SIZEOF_SHORT);

    for (i = 0; i < ncols; i++) {
        short_widths[i] = (short) widths[i];
    }

    XtVaSetValues(w, XmNcolumnWidths, short_widths, NULL);

    xfree(short_widths);
}

void TableSetDefaultRowLabelWidth(Widget w, int width)
{
    XtVaSetValues(w, XmNrowLabelWidth, width, NULL);
}

void TableSetDefaultRowLabelAlignment(Widget w, int align)
{
    unsigned char xm_alignment;

    xm_alignment = align_to_xmalign(align);

    XtVaSetValues(w, XmNrowLabelAlignment, xm_alignment, NULL);
}

void TableSetDefaultColWidth(Widget w, int width)
{
    TableData *td;
    short *widths;
    int ncols, i;

    td = (TableData*) WidgetGetUserData(w);
    td->default_col_width = width;

    ncols = TableGetNcols(w);

    widths = xmalloc(ncols*SIZEOF_SHORT);

    for (i = 0; i < ncols; i++) {
        widths[i] = td->default_col_width;
    }

    XtVaSetValues(w, XmNcolumnWidths, widths, NULL);

    xfree(widths);
}

void TableSetDefaultColAlignment(Widget w, int align)
{
    unsigned char *alignment, xm_alignment;
    int ncols, i;

    xm_alignment = align_to_xmalign(align);
    ncols = TableGetNcols(w);

    alignment = xmalloc(ncols);

    for (i = 0; i < ncols; i++) {
        alignment[i] = xm_alignment;
    }

    XtVaSetValues(w, XmNcolumnAlignments, alignment, NULL);

    xfree(alignment);
}

void TableSetDefaultColLabelAlignment(Widget w, int align)
{
    TableData *td;
    unsigned char *alignment, xm_alignment;
    int ncols, i;

    td = (TableData*) WidgetGetUserData(w);
    td->default_col_label_alignment = align;

    xm_alignment = align_to_xmalign(align);
    ncols = TableGetNcols(w);

    alignment = xmalloc(ncols);

    for (i = 0; i < ncols; i++) {
        alignment[i] = xm_alignment;
    }

    XtVaSetValues(w, XmNcolumnLabelAlignments, alignment, NULL);

    xfree(alignment);
}

void TableSetColMaxlengths(Widget w, int *maxlengths)
{
    XtVaSetValues(w, XmNcolumnMaxLengths, maxlengths, NULL);
}

void TableSetRowLabels(Widget w, char **labels)
{
    XtVaSetValues(w, XmNrowLabels, labels, NULL);

    XtVaSetValues(w, XmNrowLabelWidth, 0, NULL);
}

void TableSetColLabels(Widget w, char **labels)
{
    XtVaSetValues(w, XmNcolumnLabels, labels, NULL);
}

void TableSetFixedCols(Widget w, int nfixed_cols)
{
    XtVaSetValues(w, XmNfixedColumns, nfixed_cols, NULL);
}

void TableUpdateVisibleRowsCols(Widget w)
{
    XtVaSetValues(w,
                  XmNheight, 0,
                  XmNwidth, 0,
                  NULL);
}

void TableCommitEdit(Widget w, int close)
{
    XbaeMatrixCommitEdit(w, close);
}

void TableSetCells(Widget w, char ***cells)
{
    XtVaSetValues(w, XmNcells, cells, NULL);
}

void TableSetCell(Widget w, int row, int col, char *value)
{
    XbaeMatrixSetCell(w, row, col, value);
}

char *TableGetCell(Widget w, int row, int col)
{
    return XbaeMatrixGetCell(w, row, col);
}

void TableSelectCell(Widget w, int row, int col)
{
    XbaeMatrixSelectCell(w, row, col);
}

void TableDeselectCell(Widget w, int row, int col)
{
    XbaeMatrixDeselectCell(w, row, col);
}

void TableSelectRow(Widget w, int row)
{
    XbaeMatrixSelectRow(w, row);
}

void TableDeselectRow(Widget w, int row)
{
    XbaeMatrixDeselectRow(w, row);
}

void TableSelectCol(Widget w, int col)
{
    XbaeMatrixSelectColumn(w, col);
}

void TableDeselectCol(Widget w, int col)
{
    XbaeMatrixDeselectColumn(w, col);
}

void TableDeselectAllCells(Widget w)
{
    XbaeMatrixDeselectAll(w);
}

int TableIsRowSelected(Widget w, int row)
{
    return XbaeMatrixIsRowSelected(w, row);
}

int TableIsColSelected(Widget w, int col)
{
    return XbaeMatrixIsColumnSelected(w, col);
}

void TableUpdate(Widget w)
{
    XbaeMatrixRefresh(w);
}

static void drawcellCB(Widget w, XtPointer client_data, XtPointer call_data)
{
    TableEvent event;
    Table_CBData *cbdata = (Table_CBData *) client_data;
    XbaeMatrixDrawCellCallbackStruct *cs =
        (XbaeMatrixDrawCellCallbackStruct *) call_data;

    event.w = cbdata->w;
    event.row = cs->row;
    event.col = cs->column;
    event.background = cs->background;
    event.foreground = cs->foreground;
    event.anydata = cbdata->anydata;
    event.value_type = TABLE_CELL_NONE;

    cbdata->cbproc(&event);

    cs->background = event.background;
    cs->foreground = event.foreground;

    if (event.value_type == TABLE_CELL_STRING) {
        cs->type = XbaeString;
        cs->string = event.value;
    } else  if (event.value_type == TABLE_CELL_PIXMAP) {
        cs->type = XbaePixmap;
        cs->pixmap = event.pixmap;
    }
}

void AddTableDrawCellCB(Widget w, Table_CBProc cbproc, void *anydata)
{
    Table_CBData *cbdata;

    cbdata = (Table_CBData *) xmalloc(sizeof(Table_CBData));
    cbdata->w = w;
    cbdata->cbproc = cbproc;
    cbdata->anydata = anydata;

    XtAddCallback(w, XmNdrawCellCallback, drawcellCB, cbdata);
}

static void enterCB(Widget w, XtPointer client_data, XtPointer call_data)
{
    TableEvent event;
    Table_CBData *cbdata = (Table_CBData *) client_data;
    XbaeMatrixEnterCellCallbackStruct *cs =
            (XbaeMatrixEnterCellCallbackStruct *) call_data;

    int ok;

    event.w = cbdata->w;
    event.row = cs->row;
    event.col = cs->column;
    event.anydata = cbdata->anydata;
    ok = cbdata->cbproc(&event);

    if (!ok) {
        cs->doit = False;
        cs->map  = False;
    }
}

void AddTableEnterCellCB(Widget w, Table_CBProc cbproc, void *anydata)
{
    Table_CBData *cbdata;

    cbdata = (Table_CBData *) xmalloc(sizeof(Table_CBData));
    cbdata->w = w;
    cbdata->cbproc = cbproc;
    cbdata->anydata = anydata;

    XtAddCallback(w, XmNenterCellCallback, enterCB, cbdata);
}

static void leaveCB(Widget w, XtPointer client_data, XtPointer call_data)
{
    TableEvent event;
    Table_CBData *cbdata = (Table_CBData *) client_data;
    XbaeMatrixLeaveCellCallbackStruct *cs =
            (XbaeMatrixLeaveCellCallbackStruct *) call_data;

    int ok;

    event.w = cbdata->w;
    event.row = cs->row;
    event.col = cs->column;
    event.value = cs->value;
    event.anydata = cbdata->anydata;
    ok = cbdata->cbproc(&event);

    if (!ok) {
        cs->doit = False;
    }
}

void AddTableLeaveCellCB(Widget w, Table_CBProc cbproc, void *anydata)
{
    Table_CBData *cbdata;

    cbdata = (Table_CBData *) xmalloc(sizeof(Table_CBData));
    cbdata->w = w;
    cbdata->cbproc = cbproc;
    cbdata->anydata = anydata;

    XtAddCallback(w, XmNleaveCellCallback, leaveCB, cbdata);
}

static void labelCB(Widget w, XtPointer client_data, XtPointer call_data)
{
    XButtonEvent *xbe;

    TableEvent event;
    Table_CBData *cbdata = (Table_CBData *) client_data;
    XbaeMatrixLabelActivateCallbackStruct *cbs =
            (XbaeMatrixLabelActivateCallbackStruct *) call_data;

    event.button = NO_BUTTON;
    event.modifiers = NO_MODIFIER;
    event.anydata = cbdata->anydata;

    event.w = w;
    event.row = cbs->row;
    event.col = cbs->column;
    event.row_label = cbs->row_label;

    switch (cbs->event->type) {
    case ButtonPress:
        event.type = MOUSE_PRESS;
        xbe = (XButtonEvent *) cbs->event;
        event.udata = xbe;
        switch (cbs->event->xbutton.button) {
        case Button1:
            event.button = event.button ^ LEFT_BUTTON;
            break;
        case Button3:
            event.button = event.button ^ RIGHT_BUTTON;
            break;
        }
        if (xbe->state & ControlMask) {
            event.modifiers = event.modifiers ^ CONTROL_MODIFIER;
        }
        if (xbe->state & ShiftMask) {
            event.modifiers = event.modifiers ^ SHIFT_MODIFIER;
        }
        break;
    default:
        break;
    }

    cbdata->cbproc(&event);
}

void AddTableLabelActivateCB(Widget w, Table_CBProc cbproc, void *anydata)
{
    Table_CBData *cbdata;

    cbdata = (Table_CBData *) xmalloc(sizeof(Table_CBData));
    cbdata->w = w;
    cbdata->cbproc = cbproc;
    cbdata->anydata = anydata;

    XtAddCallback(w, XmNlabelActivateCallback, labelCB, cbdata);
}

/* ScrollBar */
void GetScrollBarValues(Widget w, int *value, int *maxvalue, int *slider_size, int *increment)
{
    Arg args[4];
    int i = 0;

    if (value) {
        XtSetArg(args[i], XmNvalue, value); i++;
    }

    if (maxvalue) {
        XtSetArg(args[i], XmNmaximum, maxvalue); i++;
    }

    if (slider_size) {
        XtSetArg(args[i], XmNsliderSize, slider_size); i++;
    }

    if (increment) {
        XtSetArg(args[i], XmNincrement, increment); i++;
    }

    if (i != 0) {
        XtGetValues(w, args, i);
    }
}

void SetScrollBarValue(Widget w, int value)
{
    XmScrollBarSetValues(w, value, 0, 0, 0, True);
}

void SetScrollBarIncrement(Widget w, int increment)
{
    XtVaSetValues(w, XmNincrement, increment, NULL);
}

Widget GetHorizontalScrollBar(Widget w)
{
    return XtNameToWidget(w, "HorScrollBar");
}

Widget GetVerticalScrollBar(Widget w)
{
    return XtNameToWidget(w, "VertScrollBar");
}

/*
** Add mouse wheel support to a specific widget, which must be the scrollable
** widget of a ScrolledWindow.
*/
void AddMouseWheelSupport(Widget w)
{
    if (XmIsScrolledWindow(XtParent(w))) 
    {
        static const char scrollTranslations[] =
           "Shift<Btn4Down>: scrolled-window-scroll-up(1)\n"
           "Shift<Btn5Down>: scrolled-window-scroll-down(1)\n"
           "Ctrl<Btn4Down>:  scrolled-window-page-up()\n"
           "Ctrl<Btn5Down>:  scrolled-window-page-down()\n"
           "<Btn4Down>:      scrolled-window-scroll-up(3)\n"
           "<Btn5Down>:      scrolled-window-scroll-down(3)\n";
        static XtTranslations trans_table = NULL;
        
        if (trans_table == NULL)
        {
            trans_table = XtParseTranslationTable(scrollTranslations);
        }
        XtOverrideTranslations(w, trans_table);
    }
}
