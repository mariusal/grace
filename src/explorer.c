/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2002,2003 Grace Development Team
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

#include <config.h>

#include "globals.h"
#include "utils.h"
#include "dicts.h"
#include "objutils.h"

#include "explorer.h"
#include <Xm/ScrolledW.h>
#include <Xm/Form.h>
#include <Xm/RowColumn.h>

#include "protos.h"

/* Quark labeling procedure */
typedef char * (*Quark_LabelingProc)(
    Quark *q
);

/* Quark creating procedure */
typedef ListTreeItem * (*Quark_CreateProc)(
    Widget w,
    ListTreeItem *parent,
    char *string,
    void *udata
);

typedef struct {
    Quark *q;
    Widget tree;
    int nchoices;
    Quark_LabelingProc labeling_proc;
} TreeItemData;

static ListTreeItem *q_create(Widget w,
    ListTreeItem *parent, char *string, void *udata);

static char *default_quark_labeling_proc(Quark *q)
{
    char buf[128];
    
    sprintf(buf, "Item \"%s\" (data = %p)", QIDSTR(q), (void *) q);
    
    return copy_string(NULL, buf);
}

static int traverse_hook(unsigned int step, void *data, void *udata)
{
    char *s;
    ListTreeItem *ti = (ListTreeItem *) udata;
    TreeItemData *ti_data = (TreeItemData *) ti->user_data;
    Quark *q = (Quark *) data;
    
    s = ti_data->labeling_proc(q);
    if (s) {
        if (step >= ti_data->nchoices) {
            q_create(ti_data->tree, ti, s, data);
            ti_data->nchoices++;
        } else {
            ListTreeRenameItem(ti_data->tree, ti, s);
        }
        xfree(s);
    }
    
    return TRUE;
}

void UpdateQuarkTree(ListTreeItem *ti)
{
    TreeItemData *ti_data = (TreeItemData *) ti->user_data;
    if (!ti_data->q || quark_count_children(ti_data->q) <= 0) {
        ListTreeDeleteChildren(ti_data->tree, ti);
        ti_data->nchoices = 0;
    }
    if (ti_data->q) {
        storage_traverse(ti_data->q->children, traverse_hook, ti);
    }
}   

ListTreeItem *CreateQuarkTree(Widget tree, ListTreeItem *parent,
    Quark *q, const char *label, Quark_LabelingProc labeling_proc)
{
    ListTreeItem *item;
    TreeItemData *data;
    char *s;
    
    data = xmalloc(sizeof(TreeItemData));
    data->q = q;
    data->tree = tree;
    data->nchoices = 0;
    
    if (labeling_proc) {
        data->labeling_proc = labeling_proc;
    } else {
        data->labeling_proc = default_quark_labeling_proc;
    }
    
    if (label) {
        s = copy_string(NULL, label);
    } else {
        s = data->labeling_proc(q);
    }
    
    if (parent) {
        parent->open = True;
    }
    item = ListTreeAdd(tree, parent, s);
    xfree(s);
    item->user_data = data;
    
    UpdateQuarkTree(item);
    
    return item;
}

static int explorer_apply(ExplorerUI *ui, void *caller);

static char *q_labeling(Quark *q)
{
    char buf[128];
    Project *pr;
    frame *f;
    graph *g;
    set *s;
    tickmarks *t;
    DObject *o;
    region *r;
    
    switch (q->fid) {
    case QFlavorProject:
        pr = project_get_data(q);
        
        sprintf(buf, "Project \"%s\" (%d graphs)", QIDSTR(q),
            number_of_graphs(q));

        break;
    case QFlavorFrame:
        f = frame_get_data(q);
        
        sprintf(buf, "(%c) Frame \"%s\"", f->active ? '+':'-', QIDSTR(q));

        break;
    case QFlavorGraph:
        g = graph_get_data(q);
        
        sprintf(buf, "(%c) Graph \"%s\" (type: %s, sets: %d)",
            g->active ? '+':'-', QIDSTR(q), graph_types(q->grace->rt, g->type),
            number_of_sets(q));

        break;
    case QFlavorSet:
        s = set_get_data(q);
        
        sprintf(buf, "(%c) Set \"%s\" (%s)",
            s->active ? '+':'-', QIDSTR(q), set_types(q->grace->rt, s->type));

        break;
    case QFlavorAxis:
        t = axis_get_data(q);
        
        sprintf(buf, "(%c) %c Axis \"%s\"",
            t->active ? '+':'-', t->type == AXIS_TYPE_X ? 'X':'Y', QIDSTR(q));

        break;
    case QFlavorDObject:
        o = object_get_data(q);

        sprintf(buf, "(%c) DObject \"%s\" (%s)",
            o->active ? '+':'-', QIDSTR(q), object_types(o->type));
        
        break;
    case QFlavorRegion:
        r = region_get_data(q);

        sprintf(buf, "(%c) Region \"%s\" (%d pts)",
            r->active ? '+':'-', QIDSTR(q), r->n);
        
        break;
    default:
        sprintf(buf, "??? \"%s\"", QIDSTR(q));
        break;
    }
    
    return copy_string(NULL, buf);
}

static ListTreeItem *q_create(Widget w,
    ListTreeItem *parent, char *string, void *udata)
{
    ListTreeItem *item;
    Quark *q = (Quark *) udata;
    TreeItemData *p_data = (TreeItemData *) parent->user_data;
    
    item = CreateQuarkTree(w, parent, q, string, p_data->labeling_proc);
    
    return item;
}


static void highlight_cb(Widget w, XtPointer client, XtPointer call)
{
    ExplorerUI *ui = (ExplorerUI *) client;
    ListTreeMultiReturnStruct *ret;
    int homogeneous_selection = TRUE;
    int all_siblings = TRUE;
    int count;
    Quark *q = NULL;
    int fid = -1;

    ret = (ListTreeMultiReturnStruct *) call;
    count = ret->count;
    
    if (count > 0) {
        int i;
        ListTreeItem *item = ret->items[0];
        TreeItemData *ti_data = (TreeItemData *) item->user_data;
        Quark *parent;
        
        q = ti_data->q;
        fid = q->fid;
        parent = quark_parent_get(q);
        
        for (i = 1; i < count; i++) {
            item = ret->items[i];
            ti_data = (TreeItemData *) item->user_data;
            
            if (ti_data->q->fid != fid) {
                homogeneous_selection = FALSE;
            }
            if (quark_parent_get(ti_data->q) != parent) {
                all_siblings = FALSE;
            }
        }
    }
    
    if (!count || !homogeneous_selection) {
        SetSensitive(ui->aacbuts[0], FALSE);
        SetSensitive(ui->aacbuts[1], FALSE);
        
        UnmanageChild(ui->project_ui->top);
        UnmanageChild(ui->frame_ui->top);
        UnmanageChild(ui->graph_ui->top);
        UnmanageChild(ui->set_ui->top);
        UnmanageChild(ui->axis_ui->top);
        UnmanageChild(ui->object_ui->top);
    } else {
        SetSensitive(ui->aacbuts[0], TRUE);
        SetSensitive(ui->aacbuts[1], TRUE);
        
        switch (fid) {
        case QFlavorProject:
            update_project_ui(ui->project_ui, q);
            
            ManageChild(ui->project_ui->top);
            UnmanageChild(ui->frame_ui->top);
            UnmanageChild(ui->graph_ui->top);
            UnmanageChild(ui->set_ui->top);
            UnmanageChild(ui->axis_ui->top);
            UnmanageChild(ui->object_ui->top);
            break;
        case QFlavorFrame:
            update_frame_ui(ui->frame_ui, q);
            
            UnmanageChild(ui->project_ui->top);
            ManageChild(ui->frame_ui->top);
            UnmanageChild(ui->graph_ui->top);
            UnmanageChild(ui->set_ui->top);
            UnmanageChild(ui->axis_ui->top);
            UnmanageChild(ui->object_ui->top);
            break;
        case QFlavorGraph:
            update_graph_ui(ui->graph_ui, q);
            
            UnmanageChild(ui->project_ui->top);
            UnmanageChild(ui->frame_ui->top);
            ManageChild(ui->graph_ui->top);
            UnmanageChild(ui->set_ui->top);
            UnmanageChild(ui->axis_ui->top);
            UnmanageChild(ui->object_ui->top);
            break;
        case QFlavorSet:
            update_set_ui(ui->set_ui, q);
            
            UnmanageChild(ui->project_ui->top);
            UnmanageChild(ui->frame_ui->top);
            UnmanageChild(ui->graph_ui->top);
            ManageChild(ui->set_ui->top);
            UnmanageChild(ui->axis_ui->top);
            UnmanageChild(ui->object_ui->top);
            break;
        case QFlavorAxis:
            update_axis_ui(ui->axis_ui, q);
            
            UnmanageChild(ui->project_ui->top);
            UnmanageChild(ui->frame_ui->top);
            UnmanageChild(ui->graph_ui->top);
            UnmanageChild(ui->set_ui->top);
            ManageChild(ui->axis_ui->top);
            UnmanageChild(ui->object_ui->top);
            break;
        case QFlavorDObject:
            update_object_ui(ui->object_ui, q);
            
            UnmanageChild(ui->project_ui->top);
            UnmanageChild(ui->frame_ui->top);
            UnmanageChild(ui->graph_ui->top);
            UnmanageChild(ui->set_ui->top);
            UnmanageChild(ui->axis_ui->top);
            ManageChild(ui->object_ui->top);
            break;
        default:
            UnmanageChild(ui->project_ui->top);
            UnmanageChild(ui->frame_ui->top);
            UnmanageChild(ui->graph_ui->top);
            UnmanageChild(ui->set_ui->top);
            UnmanageChild(ui->axis_ui->top);
            UnmanageChild(ui->object_ui->top);
            break;
        }
    }
        
    if (!count || !all_siblings || fid == QFlavorProject) {
        SetSensitive(ui->popup_delete_bt,         FALSE);
        SetSensitive(ui->popup_duplicate_bt,      FALSE);
        SetSensitive(ui->popup_bring_to_front_bt, FALSE);
        SetSensitive(ui->popup_send_to_back_bt,   FALSE);
        SetSensitive(ui->popup_move_up_bt,        FALSE);
        SetSensitive(ui->popup_move_down_bt,      FALSE);
    } else {
        SetSensitive(ui->popup_delete_bt,         TRUE);
        SetSensitive(ui->popup_duplicate_bt,      TRUE);
        SetSensitive(ui->popup_bring_to_front_bt, TRUE);
        SetSensitive(ui->popup_send_to_back_bt,   TRUE);
        SetSensitive(ui->popup_move_up_bt,        TRUE);
        SetSensitive(ui->popup_move_down_bt,      TRUE);
    }
    
    SetSensitive(ui->insert_frame_bt,    FALSE);
    SetSensitive(ui->insert_graph_bt,    FALSE);
    SetSensitive(ui->insert_set_bt,      FALSE);
    SetSensitive(ui->insert_axis_bt,     FALSE);
    SetSensitive(ui->insert_object_pane, FALSE);
    if (count == 1) {
        switch (fid) {
        case QFlavorProject:
            SetSensitive(ui->insert_frame_bt,    TRUE);
            SetSensitive(ui->insert_object_pane, TRUE);
            break;
        case QFlavorFrame:
            SetSensitive(ui->insert_graph_bt,    TRUE);
            SetSensitive(ui->insert_object_pane, TRUE);
            break;
        case QFlavorGraph:
            SetSensitive(ui->insert_set_bt,      TRUE);
            SetSensitive(ui->insert_axis_bt,     TRUE);
            SetSensitive(ui->insert_object_pane, TRUE);
            break;
        }
    }
}

static void menu_cb(Widget w, XtPointer client, XtPointer call)
{
    ListTreeItemReturnStruct *ret = (ListTreeItemReturnStruct *) call;
    XButtonEvent *xbe = (XButtonEvent *) ret->event;
    ExplorerUI *ui = (ExplorerUI *) client;

    XmMenuPosition(ui->popup, xbe);
    XtManageChild(ui->popup);
}

static void destroy_cb(Widget w, XtPointer client, XtPointer call)
{
    ListTreeItemReturnStruct *ret;

    ret = (ListTreeItemReturnStruct *) call;
    
    xfree(ret->item->user_data);
}

static int explorer_apply(ExplorerUI *ui, void *caller)
{
    ListTreeMultiReturnStruct ret;
    int count, i, res = RETURN_SUCCESS;
    
    if (caller && !GetToggleButtonState(ui->instantupdate)) {
        return RETURN_FAILURE;
    }
    
    ListTreeGetHighlighted(ui->tree, &ret);
    count = ret.count;

    for (i = 0; i < count && res == RETURN_SUCCESS; i++) {
        ListTreeItem *item = ret.items[i];
        TreeItemData *ti_data = (TreeItemData *) item->user_data;
        Quark *q = ti_data->q;

        switch (q->fid) {
        case QFlavorProject:
            if (set_project_data(ui->project_ui, q, caller) != RETURN_SUCCESS) {
                res = RETURN_FAILURE;
            }
            break;
        case QFlavorFrame:
            if (set_frame_data(ui->frame_ui, q, caller) != RETURN_SUCCESS) {
                res = RETURN_FAILURE;
            }
            break;
        case QFlavorGraph:
            if (set_graph_data(ui->graph_ui, q, caller) != RETURN_SUCCESS) {
                res = RETURN_FAILURE;
            }
            break;
        case QFlavorSet:
            if (set_set_data(ui->set_ui, q, caller) != RETURN_SUCCESS) {
                res = RETURN_FAILURE;
            }
            break;
        case QFlavorAxis:
            if (set_axis_data(ui->axis_ui, q, caller) != RETURN_SUCCESS) {
                res = RETURN_FAILURE;
            }
            break;
        case QFlavorDObject:
            if (set_object_data(ui->object_ui, q, caller) != RETURN_SUCCESS) {
                res = RETURN_FAILURE;
            }
            break;
        default:
            res = RETURN_FAILURE;
            break;
        }
    }
    
    xdrawgraph();
    
    update_explorer(ui, FALSE);
    update_app_title(grace->project);
    
    return res;
}

static int explorer_aac(void *data)
{
    ExplorerUI *ui = (ExplorerUI *) data;
    
    return explorer_apply(ui, NULL);
}

static void highlight_selected(Widget w, ListTreeItem *parent,
    int nsquarks, Quark **squarks)
{
    ListTreeItem *item, *sibling;

    item = parent;
    while (item) {
        TreeItemData *ti_data = (TreeItemData *) item->user_data;
        int i;
        for (i = 0; i < nsquarks; i++) {
            if (ti_data->q == squarks[i]) {
                ListTreeHighlightItemMultiple(w, item);
            }
        }
        if (item->firstchild) {
            highlight_selected(w, item->firstchild, nsquarks, squarks);
        }
        sibling = item->nextsibling;
        item = sibling;
    }
}

void update_explorer(ExplorerUI *ui, int reselect)
{
    ListTreeMultiReturnStruct ret;
    int i, nsquarks;
    Quark **squarks;
    
    if (!ui) {
        return;
    }
    
    ListTreeGetHighlighted(ui->tree, &ret);
    nsquarks = ret.count;
    
    squarks = xmalloc(nsquarks*SIZEOF_VOID_P);
    for (i = 0; i < nsquarks; i++) {
        ListTreeItem *item = ret.items[i];
        TreeItemData *ti_data = (TreeItemData *) item->user_data;
        squarks[i] = ti_data->q;
    }
    
    ListTreeRefreshOff(ui->tree);
    ListTreeDelete(ui->tree, ui->project);
    ui->project = CreateQuarkTree(ui->tree, NULL,
        grace->project, NULL, q_labeling);

    highlight_selected(ui->tree, ui->project, nsquarks, squarks);
    xfree(squarks);

    ListTreeRefreshOn(ui->tree);
    ListTreeRefresh(ui->tree);

    if (reselect) {
        ListTreeGetHighlighted(ui->tree, &ret);
        XtCallCallbacks(ui->tree, XtNhighlightCallback, (XtPointer) &ret);
    }
}

static void update_explorer_cb(Widget but, void *data)
{
    ExplorerUI *ui = (ExplorerUI *) data;
    update_explorer(ui, TRUE);
}


#define DELETE_CB         0
#define DUPLICATE_CB      1
#define BRING_TO_FRONT_CB 2
#define SEND_TO_BACK_CB   3
#define MOVE_UP_CB        4
#define MOVE_DOWN_CB      5
#define ADD_FRAME_CB      6
#define ADD_GRAPH_CB      7
#define ADD_SET_CB        8
#define ADD_AXIS_CB       9
#define ADD_LINE_CB      10
#define ADD_BOX_CB       11
#define ADD_ARC_CB       12
#define ADD_TEXT_CB      13

static void popup_any_cb(ExplorerUI *eui, int type)
{
    ListTreeMultiReturnStruct ret;
    int count, i;
    
    ListTreeGetHighlighted(eui->tree, &ret);
    count = ret.count;
    
    for (i = 0; i < count; i ++) {
        ListTreeItem *item;
        TreeItemData *ti_data;
        Quark *q;
        
        switch (type) {
        case SEND_TO_BACK_CB:
        case MOVE_UP_CB:
            item = ret.items[count - i - 1];
            break;
        default:
            item = ret.items[i];
            break;
        }

        ti_data = (TreeItemData *) item->user_data;
        q = ti_data->q;
        
        switch (type) {
        case DELETE_CB:
            quark_free(q);
            break;
        case BRING_TO_FRONT_CB:
            quark_push(q, TRUE);
            break;
        case SEND_TO_BACK_CB:
            quark_push(q, FALSE);
            break;
        case MOVE_UP_CB:
            quark_move(q, TRUE);
            break;
        case MOVE_DOWN_CB:
            quark_move(q, FALSE);
            break;
        case DUPLICATE_CB:
            quark_copy(q);
            break;
        case ADD_FRAME_CB:
            frame_new(q);
            break;
        case ADD_GRAPH_CB:
            graph_new(q);
            break;
        case ADD_SET_CB:
            set_new(q);
            break;
        case ADD_AXIS_CB:
            axis_new(q);
            break;
        case ADD_LINE_CB:
            object_new_complete(q, DO_LINE);
            break;
        case ADD_BOX_CB:
            object_new_complete(q, DO_BOX);
            break;
        case ADD_ARC_CB:
            object_new_complete(q, DO_ARC);
            break;
        case ADD_TEXT_CB:
            object_new_complete(q, DO_STRING);
            break;
        }
    }
    
    xdrawgraph();
    update_explorer(eui, FALSE);
    update_app_title(grace->project);
}

static void delete_cb(Widget but, void *udata)
{
    popup_any_cb((ExplorerUI *) udata, DELETE_CB);
}

static void duplicate_cb(Widget but, void *udata)
{
    popup_any_cb((ExplorerUI *) udata, DUPLICATE_CB);
}

static void bring_to_front_cb(Widget but, void *udata)
{
    popup_any_cb((ExplorerUI *) udata, BRING_TO_FRONT_CB);
}

static void send_to_back_cb(Widget but, void *udata)
{
    popup_any_cb((ExplorerUI *) udata, SEND_TO_BACK_CB);
}

static void move_up_cb(Widget but, void *udata)
{
    popup_any_cb((ExplorerUI *) udata, MOVE_UP_CB);
}

static void move_down_cb(Widget but, void *udata)
{
    popup_any_cb((ExplorerUI *) udata, MOVE_DOWN_CB);
}

static void add_frame_cb(Widget but, void *udata)
{
    popup_any_cb((ExplorerUI *) udata, ADD_FRAME_CB);
}

static void add_graph_cb(Widget but, void *udata)
{
    popup_any_cb((ExplorerUI *) udata, ADD_GRAPH_CB);
}

static void add_set_cb(Widget but, void *udata)
{
    popup_any_cb((ExplorerUI *) udata, ADD_SET_CB);
}

static void add_axis_cb(Widget but, void *udata)
{
    popup_any_cb((ExplorerUI *) udata, ADD_AXIS_CB);
}

static void add_object_cb(Widget but, void *udata)
{
    ExplorerUI *eui = (ExplorerUI *) udata;
    if (but == eui->insert_line_bt) {
        popup_any_cb(eui, ADD_LINE_CB);
    } else
    if (but == eui->insert_box_bt) {
        popup_any_cb(eui, ADD_BOX_CB);
    } else
    if (but == eui->insert_arc_bt) {
        popup_any_cb(eui, ADD_ARC_CB);
    } else
    if (but == eui->insert_text_bt) {
        popup_any_cb(eui, ADD_TEXT_CB);
    }
}


void define_explorer_popup(Widget but, void *data)
{
    GUI *gui = (GUI *) data;
    Grace *grace = gui->P;

    set_wait_cursor();
    
    if (!gui->eui) {
        ExplorerUI *eui;
        
        Widget menubar, menupane, panel;

        eui = xmalloc(sizeof(ExplorerUI));
        
        eui->top = CreateDialogForm(app_shell, "Explorer");
        menubar = CreateMenuBar(eui->top);
        ManageChild(menubar);
        AddDialogFormChild(eui->top, menubar);

        menupane = CreateMenu(menubar, "File", 'F', FALSE);
        CreateMenuButton(menupane,
            "Close", 'C', destroy_dialog_cb, GetParent(eui->top));

        menupane = CreateMenu(menubar, "Insert", 'I', FALSE);
        eui->insert_frame_bt = CreateMenuButton(menupane,
            "Frame", '\0', add_frame_cb, eui);
        SetSensitive(eui->insert_frame_bt,  FALSE);
        eui->insert_graph_bt = CreateMenuButton(menupane,
            "Graph", '\0', add_graph_cb, eui);
        SetSensitive(eui->insert_graph_bt,  FALSE);
        eui->insert_set_bt = CreateMenuButton(menupane,
            "Set", '\0', add_set_cb, eui);
        SetSensitive(eui->insert_set_bt,    FALSE);
        eui->insert_axis_bt = CreateMenuButton(menupane,
            "Axis", '\0', add_axis_cb, eui);
        SetSensitive(eui->insert_axis_bt,   FALSE);
        eui->insert_object_pane = CreateMenu(menupane, "DObject", 'O', FALSE);
        SetSensitive(eui->insert_object_pane, FALSE);
        eui->insert_line_bt = CreateMenuButton(eui->insert_object_pane,
            "Line", '\0', add_object_cb, eui);
        eui->insert_box_bt = CreateMenuButton(eui->insert_object_pane,
            "Box", '\0', add_object_cb, eui);
        eui->insert_arc_bt = CreateMenuButton(eui->insert_object_pane,
            "Arc", '\0', add_object_cb, eui);
        eui->insert_text_bt = CreateMenuButton(eui->insert_object_pane,
            "Text", '\0', add_object_cb, eui);

        menupane = CreateMenu(menubar, "Options", 'O', FALSE);
        eui->instantupdate = CreateMenuToggle(menupane, "Instantaneous update",
                            'u', NULL, NULL);
        SetToggleButtonState(eui->instantupdate, gui->instant_update);

        menupane = CreateMenu(menubar, "Help", 'H', TRUE);
        CreateMenuHelpButton(menupane, "On explorer", 'e',
            eui->top, "doc/UsersGuide.html#explorer");

        panel = CreateGrid(eui->top, 2, 1);
        eui->tree = XmCreateScrolledListTree(panel, "tree", NULL, 0);
        XtAddCallback(eui->tree, XtNhighlightCallback, highlight_cb, eui);
        XtAddCallback(eui->tree, XtNmenuCallback, menu_cb, eui);
        XtAddCallback(eui->tree, XtNdestroyItemCallback, destroy_cb, eui);
        PlaceGridChild(panel, GetParent(eui->tree), 0, 0);

        eui->scrolled_window = XtVaCreateManagedWidget("scrolled_window",
	    xmScrolledWindowWidgetClass, panel,
            XmNscrollingPolicy, XmAUTOMATIC,
	    XmNscrollBarDisplayPolicy, XmSTATIC,
	    NULL);
        PlaceGridChild(panel, eui->scrolled_window, 1, 0);

	eui->project_ui = create_project_ui(eui);
        UnmanageChild(eui->project_ui->top);

	eui->frame_ui = create_frame_ui(eui);
        UnmanageChild(eui->frame_ui->top);

	eui->graph_ui = create_graph_ui(eui);
        UnmanageChild(eui->graph_ui->top);

	eui->set_ui = create_set_ui(eui);
        UnmanageChild(eui->set_ui->top);

	eui->axis_ui = create_axis_ui(eui);
        UnmanageChild(eui->axis_ui->top);

	eui->object_ui = create_object_ui(eui);
        UnmanageChild(eui->object_ui->top);

        eui->aacbuts = CreateAACDialog(eui->top, panel, explorer_aac, eui);

        eui->project = CreateQuarkTree(eui->tree, NULL,
            grace->project, NULL, q_labeling);
        
        ManageChild(eui->tree);
        ListTreeRefreshOn(eui->tree);
        ListTreeRefresh(eui->tree);

        /* Menu popup */
        eui->popup = XmCreatePopupMenu(eui->tree, "explorerPopupMenu", NULL, 0);
        eui->popup_delete_bt = CreateMenuButton(eui->popup,
            "Delete", '\0', delete_cb, eui);
        eui->popup_duplicate_bt = CreateMenuButton(eui->popup,
            "Duplicate", '\0', duplicate_cb, eui);

        CreateMenuSeparator(eui->popup);

        eui->popup_bring_to_front_bt = CreateMenuButton(eui->popup,
            "Bring to front", '\0', bring_to_front_cb, eui);
        eui->popup_move_up_bt = CreateMenuButton(eui->popup,
            "Move up", '\0', move_up_cb, eui);
        eui->popup_move_down_bt = CreateMenuButton(eui->popup,
            "Move down", '\0', move_down_cb, eui);
        eui->popup_send_to_back_bt = CreateMenuButton(eui->popup,
            "Send to back", '\0', send_to_back_cb, eui);

        CreateMenuSeparator(eui->popup);

        CreateMenuButton(eui->popup,
            "Update tree", '\0', update_explorer_cb, eui);
        
        gui->eui = eui;
    }

    RaiseWindow(GetParent(gui->eui->top));

    unset_wait_cursor();
}


void oc_explorer_cb(OptionStructure *opt, int a, void *data)
{
    ExplorerUI *eui = (ExplorerUI *) data;
    explorer_apply(eui, opt);
}
void tb_explorer_cb(Widget but, int a, void *data)
{
    ExplorerUI *eui = (ExplorerUI *) data;
    explorer_apply(eui, but);
}
void scale_explorer_cb(Widget scale, int a, void *data)
{
    ExplorerUI *eui = (ExplorerUI *) data;
    explorer_apply(eui, scale);
}
void sp_explorer_cb(SpinStructure *spinp, double a, void *data)
{
    ExplorerUI *eui = (ExplorerUI *) data;
    explorer_apply(eui, spinp);
}
void text_explorer_cb(TextStructure *cst, char *s, void *data)
{
    ExplorerUI *eui = (ExplorerUI *) data;
    explorer_apply(eui, cst);
}
void titem_explorer_cb(Widget w, char *s, void *data)
{
    ExplorerUI *eui = (ExplorerUI *) data;
    explorer_apply(eui, w);
}
