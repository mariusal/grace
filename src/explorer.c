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
    DObject *o;
    
    switch (q->fid) {
    case QFlavorProject:
        pr = project_get_data(q);
        
        sprintf(buf, "Project \"%s\" (%d graphs)", QIDSTR(q),
            number_of_graphs(q));

        break;
    case QFlavorFrame:
        f = frame_get_data(q);
        
        sprintf(buf, "Frame \"%s\"", QIDSTR(q));

        break;
    case QFlavorGraph:
        g = graph_get_data(q);
        
        sprintf(buf, "(%c) Graph \"%s\" (type: %s, sets: %d)",
            !g->hidden ? '+':'-', QIDSTR(q), graph_types(grace->rt, g->type),
            number_of_sets(q));

        break;
    case QFlavorSet:
        s = set_get_data(q);
        
        sprintf(buf, "(%c) Set \"%s\" (%s)",
            !s->hidden ? '+':'-', QIDSTR(q), set_types(grace->rt, s->type));

        break;
    case QFlavorDObject:
        o = object_get_data(q);

        sprintf(buf, "(%c) DObject \"%s\" (%s)",
            o->active ? '+':'-', QIDSTR(q), object_types(o->type));
        
        break;
    default:
        sprintf(buf, "???");
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
    int count;
    Quark *q = NULL;
    int fid = -1;

    ret = (ListTreeMultiReturnStruct *) call;
    count = ret->count;
    
    if (count > 0) {
        int i;
        ListTreeItem *item = ret->items[0];
        TreeItemData *ti_data = (TreeItemData *) item->user_data;
        q = ti_data->q;
        fid = q->fid;
        
        for (i = 0; i < count; i++) {
            item = ret->items[i];
            ti_data = (TreeItemData *) item->user_data;
            
            if (ti_data->q->fid != fid) {
                homogeneous_selection = FALSE;
            }
        }
    }
    
    if (!count || !homogeneous_selection) {
        SetSensitive(ui->aacbuts[0], FALSE);
        SetSensitive(ui->aacbuts[1], FALSE);
        
        UnmanageChild(ui->project_ui->top);
    } else {
        SetSensitive(ui->aacbuts[0], TRUE);
        SetSensitive(ui->aacbuts[1], TRUE);
        
        switch (fid) {
        case QFlavorProject:
            update_project_ui(ui->project_ui, q);
            
            ManageChild(ui->project_ui->top);
            UnmanageChild(ui->frame_ui->top);
            UnmanageChild(ui->graph_ui->top);
            break;
        case QFlavorFrame:
            update_frame_ui(ui->frame_ui, q);
            
            UnmanageChild(ui->project_ui->top);
            ManageChild(ui->frame_ui->top);
            UnmanageChild(ui->graph_ui->top);
            break;
        case QFlavorGraph:
            update_graph_ui(ui->graph_ui, q);
            
            UnmanageChild(ui->project_ui->top);
            UnmanageChild(ui->frame_ui->top);
            ManageChild(ui->graph_ui->top);
            break;
        default:
            UnmanageChild(ui->project_ui->top);
            UnmanageChild(ui->frame_ui->top);
            UnmanageChild(ui->graph_ui->top);
            break;
        }
    }
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
        default:
            res = RETURN_FAILURE;
            break;
        }
    }
    
    xdrawgraph();
    
    return res;
}

static int explorer_aac(void *data)
{
    ExplorerUI *ui = (ExplorerUI *) data;
    
    return explorer_apply(ui, NULL);
}

static void update_explorer(ExplorerUI *ui)
{
    ListTreeRefreshOff(ui->tree);
    ListTreeDelete(ui->tree, ui->project);
    ui->project = CreateQuarkTree(ui->tree, NULL,
        grace->project, NULL, q_labeling);
    ListTreeRefreshOn(ui->tree);
}

static void update_explorer_cb(Widget but, void *data)
{
    ExplorerUI *ui = (ExplorerUI *) data;
    update_explorer(ui);
}

void define_explorer_popup(Widget but, void *data)
{
    static ExplorerUI *eui = NULL;

    set_wait_cursor();
    
    if (!eui) {
        Widget menubar, menupane, panel;

        eui = xmalloc(sizeof(ExplorerUI));
        
        eui->top = CreateDialogForm(app_shell, "Explorer");
        menubar = CreateMenuBar(eui->top);
        ManageChild(menubar);
        AddDialogFormChild(eui->top, menubar);

        menupane = CreateMenu(menubar, "File", 'F', FALSE);
        CreateMenuButton(menupane,
            "Close", 'C', destroy_dialog_cb, GetParent(eui->top));

        menupane = CreateMenu(menubar, "Edit", 'E', FALSE);
        CreateMenuButton(menupane, "Update", 'U', update_explorer_cb, eui);

        menupane = CreateMenu(menubar, "Options", 'O', FALSE);
        eui->instantupdate = CreateMenuToggle(menupane, "Instantaneous update",
                            'u', NULL, NULL);
        SetToggleButtonState(eui->instantupdate, grace->gui->instant_update);

        menupane = CreateMenu(menubar, "Help", 'H', TRUE);
        CreateMenuHelpButton(menupane, "On explorer", 'e',
            eui->top, "doc/UsersGuide.html#explorer");

        panel = CreateGrid(eui->top, 2, 1);
        eui->tree = XmCreateScrolledListTree(panel, "tree", NULL, 0);
        XtAddCallback(eui->tree, XtNhighlightCallback, highlight_cb, eui);
        XtAddCallback(eui->tree, XtNdestroyItemCallback, destroy_cb, eui);
        ManageChild(eui->tree);
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

        eui->aacbuts = CreateAACDialog(eui->top, panel, explorer_aac, eui);

        eui->project = CreateQuarkTree(eui->tree, NULL,
            grace->project, NULL, q_labeling);
    }

    RaiseWindow(GetParent(eui->top));
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
