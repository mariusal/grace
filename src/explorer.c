/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2002-2005 Grace Development Team
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

#include <config.h>

#include "globals.h"
#include "utils.h"
#include "explorer.h"
#include "xprotos.h"
#include "files.h"

static void manage_plugin(ExplorerUI *ui, Widget managed_top)
{
    if (managed_top == ui->project_ui->top) {
        ManageChild(ui->project_ui->top);
    } else {
        UnmanageChild(ui->project_ui->top);
    }
    if (managed_top == ui->ssd_ui->top) {
        ManageChild(ui->ssd_ui->top);
    } else {
        UnmanageChild(ui->ssd_ui->top);
    }
    if (managed_top == ui->frame_ui->top) {
        ManageChild(ui->frame_ui->top);
    } else {
        UnmanageChild(ui->frame_ui->top);
    }
    if (managed_top == ui->graph_ui->top) {
        ManageChild(ui->graph_ui->top);
    } else {
        UnmanageChild(ui->graph_ui->top);
    }
    if (managed_top == ui->set_ui->top) {
        ManageChild(ui->set_ui->top);
    } else {
        UnmanageChild(ui->set_ui->top);
    }
    if (managed_top == ui->axisgrid_ui->top) {
        ManageChild(ui->axisgrid_ui->top);
    } else {
        UnmanageChild(ui->axisgrid_ui->top);
    }
    if (managed_top == ui->axis_ui->top) {
        ManageChild(ui->axis_ui->top);
    } else {
        UnmanageChild(ui->axis_ui->top);
    }
    if (managed_top == ui->object_ui->top) {
        ManageChild(ui->object_ui->top);
    } else {
        UnmanageChild(ui->object_ui->top);
    }
    if (managed_top == ui->atext_ui->top) {
        ManageChild(ui->atext_ui->top);
    } else {
        UnmanageChild(ui->atext_ui->top);
    }
    if (managed_top == ui->region_ui->top) {
        ManageChild(ui->region_ui->top);
    } else {
        UnmanageChild(ui->region_ui->top);
    }
}

static int highlight_cb(TreeEvent *event)
{
    ExplorerUI *ui = (ExplorerUI *) event->anydata;
    TreeItemList items;
    int count;
    Quark *q = NULL;
    int fid = -1;
    int parent_fid = -1;
    int homogeneous_selection = TRUE;

    TreeGetHighlighted(event->w, &items);
    count = items.count;
    
    ui->homogeneous_parent = TRUE;

    if (count > 0) {
        int i;

        q = TreeGetQuark(items.items[0]);
        fid = quark_fid_get(q);
        parent_fid = quark_fid_get(quark_parent_get(q));

        for (i = 1; i < count; i++) {
            Quark *iq = TreeGetQuark(items.items[i]);
            
            if ((int) quark_fid_get(iq) != fid) {
                homogeneous_selection = FALSE;
            }
            if ((int) quark_fid_get(quark_parent_get(iq)) != parent_fid) {
                ui->homogeneous_parent = FALSE;
            }
        }
    }
    xfree(items.items);

    if (!count || !homogeneous_selection) {
        SetSensitive(ui->aacbuts[0], FALSE);
        SetSensitive(ui->aacbuts[1], FALSE);
        
        manage_plugin(ui, NULL);
    } else {
        Widget managed_top;

        SetSensitive(ui->aacbuts[0], TRUE);
        SetSensitive(ui->aacbuts[1], TRUE);
        
        switch (fid) {
        case QFlavorProject:
            update_project_ui(ui->project_ui, q);
            managed_top = ui->project_ui->top;
            break;
        case QFlavorSSD:
            update_ssd_ui(ui->ssd_ui, q);
            managed_top = ui->ssd_ui->top;
            break;
        case QFlavorFrame:
            update_frame_ui(ui->frame_ui, q);
            managed_top = ui->frame_ui->top;
            break;
        case QFlavorGraph:
            update_graph_ui(ui->graph_ui, q);
            managed_top = ui->graph_ui->top;
            break;
        case QFlavorSet:
            update_set_ui(ui->set_ui, q);
            managed_top = ui->set_ui->top;
            break;
        case QFlavorAGrid:
            update_axisgrid_ui(ui->axisgrid_ui, q);
            managed_top = ui->axisgrid_ui->top;
            break;
        case QFlavorAxis:
            update_axis_ui(ui->axis_ui, q);
            managed_top = ui->axis_ui->top;
            break;
        case QFlavorDObject:
            update_object_ui(ui->object_ui, q);
            managed_top = ui->object_ui->top;
            break;
        case QFlavorAText:
            update_atext_ui(ui->atext_ui, q);
            managed_top = ui->atext_ui->top;
            break;
        case QFlavorRegion:
            update_region_ui(ui->region_ui, q);
            managed_top = ui->region_ui->top;
            break;
        default:
            managed_top = NULL;
            break;
        }

        manage_plugin(ui, managed_top);
    }

    SetSensitive(ui->insert_frame_bt,    FALSE);
    SetSensitive(ui->insert_graph_bt,    FALSE);
    SetSensitive(ui->insert_set_bt,      FALSE);
    SetSensitive(ui->insert_ssd_bt,      FALSE);
    SetSensitive(ui->insert_axisgrid_bt, FALSE);
    SetSensitive(ui->insert_axis_bt,     FALSE);
    SetSensitive(ui->insert_object_pane, FALSE);
    if (count == 1) {
        SetSensitive(ui->idstr->form, TRUE);
        SetTextString(ui->idstr, QIDSTR(q));
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
            SetSensitive(ui->insert_ssd_bt,      TRUE);
            SetSensitive(ui->insert_axisgrid_bt, TRUE);
            SetSensitive(ui->insert_object_pane, TRUE);
            break;
        case QFlavorAGrid:
            SetSensitive(ui->insert_axis_bt,     TRUE);
            break;
        case QFlavorAxis:
            SetSensitive(ui->insert_object_pane, TRUE);
            break;
        }
        
        if (get_parent_ssd(q) &&
            (fid == QFlavorGraph || fid == QFlavorSSD)) {
            SetSensitive(ui->insert_set_bt,      TRUE);
        }
    } else {
        SetSensitive(ui->idstr->form, FALSE);
        SetTextString(ui->idstr, NULL);
    }

    return TRUE;
}


static int menu_cb(TreeEvent *event)
{
    ExplorerUI *ui = (ExplorerUI *) event->anydata;
    TreeItemList items;
    int count;
    Quark *q = NULL;
    Quark *p = NULL;
    int parent_child_selection = FALSE;
    int all_shown = TRUE;
    int all_hidden = TRUE;

    TreeGetHighlighted(event->w, &items);
    count = items.count;

    if (count > 0) {
        int i, j;

        q = TreeGetQuark(items.items[0]);

        for (i = 0; i < count; i++) {
            Quark *iq = TreeGetQuark(items.items[i]);

            p = quark_parent_get(iq);
            while (p) {
                for (j = 0; j < count; j++) {
                    if (TreeGetQuark(items.items[j]) == p) {
                        parent_child_selection = TRUE;
                    }
                }
                p = quark_parent_get(p);
            }

            if (quark_is_active(iq)) {
                all_hidden = FALSE;
            } else {
                all_shown = FALSE;
            }
        }

        if (count == 1 && quark_fid_get(q) == QFlavorProject) {
            SetSensitive(ui->project_popup_show_bt,            FALSE);
            SetSensitive(ui->project_popup_save_bt,            FALSE);
            SetSensitive(ui->project_popup_save_as_bt,         FALSE);
            SetSensitive(ui->project_popup_revert_to_saved_bt, FALSE);
            SetSensitive(ui->project_popup_close_bt,           FALSE);

            if (quark_is_active(q)) {
                SetSensitive(ui->project_popup_show_bt,   FALSE);
            } else {
                SetSensitive(ui->project_popup_show_bt,   TRUE);
            }

            SetSensitive(ui->project_popup_save_bt,            TRUE);
            SetSensitive(ui->project_popup_save_as_bt,         TRUE);
            SetSensitive(ui->project_popup_revert_to_saved_bt, TRUE);

            if (gapp->gpcount > 1) {
                SetSensitive(ui->project_popup_close_bt,  TRUE);
            } else {
                SetSensitive(ui->project_popup_close_bt,  FALSE);
            }

            ShowMenu(ui->project_popup, event->udata);
        } else {
            SetSensitive(ui->popup_hide_bt, !all_hidden);
            SetSensitive(ui->popup_show_bt, !all_shown);

            if (parent_child_selection) {
                SetSensitive(ui->popup_delete_bt,         FALSE);
                SetSensitive(ui->popup_duplicate_bt,      FALSE);
            } else {
                SetSensitive(ui->popup_delete_bt,         TRUE);
                SetSensitive(ui->popup_duplicate_bt,      TRUE);
            }

            ShowMenu(ui->popup, event->udata);
        }
    }

    xfree(items.items);

    return TRUE;
}

static void explorer_snapshot(GraceApp *gapp, GProject *gp, int all)
{
    if (gp == gapp->gp) {
        snapshot_and_update(gp, all);
    } else {
        amem_snapshot(quark_get_amem(gproject_get_top(gp)));
    }
}

static void add_to_list(GProject **gplist, int *gpcount, GProject *gp)
{
    int i, add = TRUE;

    for (i = 0; i < *gpcount; i++) {
        if (gp == gplist[i]) {
            add = FALSE;
        }
    }

    if (add) {
        gplist[*gpcount] = gp;
        (*gpcount)++;
    }
}

static int drop_cb(TreeEvent *event)
{
    ExplorerUI *ui = (ExplorerUI *) event->anydata;

    int gpcount = 0;
    GProject **gplist;

    if (ui->homogeneous_parent) {
        int count;
        TreeItemList items;

        TreeGetHighlighted(ui->tree, &items);
        count = items.count;
        if (count > 0) {
            TreeItem *item = (TreeItem *) event->udata;
            Quark *drop_q = TreeGetQuark(item);
            Quark *drop_parent = quark_parent_get(drop_q);
            GProject *drop_gp = gproject_from_quark(drop_q);
            Quark *parent = quark_parent_get(TreeGetQuark(items.items[0]));

            gplist = xmalloc(gapp->gpcount*sizeof(GProject));
            if (!gplist) {
                return FALSE;
            }

            if (parent) {
                int i, id;
                Quark *newparent;

                if (quark_fid_get(parent) == quark_fid_get(drop_q)) {
                    id = 0;
                    newparent = drop_q;
                } else if (quark_fid_get(parent) == quark_fid_get(drop_parent)) {
                    id = quark_get_id(drop_q) + 1;
                    newparent = quark_parent_get(drop_q);
                } else {
                    return FALSE;
                }

                for (i = 0; i < count; i++) {
                    Quark *q = TreeGetQuark(items.items[i]);

                    switch (event->drop_action) {
                    case DROP_ACTION_COPY:
                        quark_copy2(q, newparent, id);
                        break;
                    case DROP_ACTION_MOVE:
                        add_to_list(gplist, &gpcount, gproject_from_quark(q));
                        quark_move2(q, newparent, id);
                        break;
                    default:
                        errmsg("unknown drop type");
                        break;
                    }
                }

                switch (event->drop_action) {
                case DROP_ACTION_COPY:
                    explorer_snapshot(gapp, drop_gp, TRUE);
                    break;
                case DROP_ACTION_MOVE:
                    add_to_list(gplist, &gpcount, drop_gp);
                    for (i = 0; i < gpcount; i++) {
                        explorer_snapshot(gapp, gplist[i], TRUE);
                    }
                    xfree(gplist);
                    break;
                }

                return TRUE;
            }
        }
    }
    return FALSE;
}

static void init_item(ExplorerUI *eui, TreeItem *item, Quark *q)
{
    int active;
    char *s;

    s = q_labeling(q);
    TreeSetItemText(eui->tree, item, s);
    xfree(s);

    active = quark_is_active(q);

    if (active && quark_count_children(q) > 0) {
        TreeSetItemOpen(eui->tree, item, TRUE);
    } else {
        if (quark_fid_get(q) != QFlavorProject) {
            TreeSetItemOpen(eui->tree, item, FALSE);
        }
    }

    if (active) {
        TreeSetItemPixmap(eui->tree, item, eui->a_icon);
    } else {
        TreeSetItemPixmap(eui->tree, item, eui->h_icon);
    }
}

static int create_hook(Quark *q, void *udata, QTraverseClosure *closure)
{
    TreeItem *item;
    ExplorerUI *eui = (ExplorerUI *) udata;
    Quark *qparent = quark_parent_get(q);

    item = TreeInsertItem(eui->tree, quark_get_udata(qparent), q, quark_get_id(q));

    init_item(eui, item, q);

    quark_set_udata(q, item);

    return TRUE;
}

static int nsquarks;
static Quark **squarks;

static void explorer_save_quark_state(ExplorerUI *eui)
{
    int i;
    TreeItemList items;

    TreeGetHighlighted(eui->tree, &items);
    nsquarks = items.count;

    squarks = xmalloc(nsquarks*SIZEOF_VOID_P);

    for (i = 0; i < nsquarks; i++) {
        TreeItem *item = items.items[i];
        squarks[i] = TreeGetQuark(item);
    }

    xfree(items.items);
}

static void explorer_restore_quark_state(ExplorerUI *eui)
{
    int i;

    for (i = 0; i < nsquarks; i++) {
        TreeItem *item = quark_get_udata(squarks[i]);
        TreeHighlightItem(eui->tree, item);
    }

    xfree(squarks);
}

static int create_children_hook(unsigned int step, void *data, void *udata)
{
    ExplorerUI *eui = (ExplorerUI *) udata;
    Quark *q = (Quark *) data;

    quark_traverse(q, create_hook, eui);

    return TRUE;
}

static int explorer_cb(Quark *q, int etype, void *udata)
{
    ExplorerUI *eui = (ExplorerUI *) udata;
    TreeItem *item = quark_get_udata(q);
    TreeItem *parent_item;

    switch (etype) {
    case QUARK_ETYPE_DELETE:
        TreeDeleteItem(eui->tree, item);
        break;
    case QUARK_ETYPE_MODIFY:
        init_item(eui, item, q);
        break;
    case QUARK_ETYPE_NEW:
        create_hook(q, eui, NULL);
        break;
    case QUARK_ETYPE_MOVE:
        explorer_save_quark_state(eui);
        TreeDeleteItem(eui->tree, item);

        parent_item = quark_get_udata(quark_parent_get(q));
        item = TreeInsertItem(eui->tree, parent_item, q, quark_get_id(q));
        init_item(eui, item, q);
        quark_set_udata(q, item);
        storage_traverse(quark_get_children(q), create_children_hook, eui);

        explorer_restore_quark_state(eui);
        break;
    }

    TreeRefresh(eui->tree);

    return TRUE;
}

static void init_quark_tree(ExplorerUI *eui)
{
    storage_traverse(quark_get_children(gapp->pc), create_children_hook, eui);
    quark_cb_add(gapp->pc, explorer_cb, eui);
    TreeRefresh(eui->tree);
}

static void select_quark_explorer(Quark *q)
{
    GUI *gui = gui_from_quark(q);

    if (gui->eui) {
        TreeItem *item = quark_get_udata(q);

        TreeClearSelection(gui->eui->tree);
        TreeHighlightItem(gui->eui->tree, item);
        TreeScrollToItem(gui->eui->tree, item);
    }
}

static int explorer_apply(ExplorerUI *ui, void *caller)
{
    TreeItemList items;
    int count, i, res = RETURN_SUCCESS;

    int gpcount = 0;
    GProject **gplist;
    
    if (caller && !GetToggleButtonState(ui->instantupdate)) {
        return RETURN_FAILURE;
    }
    
    TreeGetHighlighted(ui->tree, &items);
    count = items.count;

    if (!count) {
        xfree(items.items);
        return RETURN_FAILURE;
    }

    gplist = xmalloc(gapp->gpcount*sizeof(GProject));
    if (!gplist) {
        return RETURN_FAILURE;
    }

    for (i = 0; i < count && res == RETURN_SUCCESS; i++) {
        Quark *q = TreeGetQuark(items.items[i]);

        add_to_list(gplist, &gpcount, gproject_from_quark(q));

        if (count == 1 && (!caller || caller == ui->idstr)) {
            char *s = GetTextString(ui->idstr);
            quark_idstr_set(q, s);
            xfree(s);
        }

        switch (quark_fid_get(q)) {
        case QFlavorProject:
            if (set_project_data(ui->project_ui, q, caller) != RETURN_SUCCESS) {
                res = RETURN_FAILURE;
            }
            break;
        case QFlavorSSD:
            if (set_ssd_data(ui->ssd_ui, q, caller) != RETURN_SUCCESS) {
                res = RETURN_FAILURE;
            }
            break;
        case QFlavorFrame:
            if (set_frame_data(ui->frame_ui, q, caller) != RETURN_SUCCESS) {
                res = RETURN_FAILURE;
            }
            break;
        case QFlavorGraph:
            if (graph_set_data(ui->graph_ui, q, caller) != RETURN_SUCCESS) {
                res = RETURN_FAILURE;
            }
            break;
        case QFlavorSet:
            if (set_set_data(ui->set_ui, q, caller) != RETURN_SUCCESS) {
                res = RETURN_FAILURE;
            }
            break;
        case QFlavorAGrid:
            if (set_axisgrid_data(ui->axisgrid_ui, q, caller) != RETURN_SUCCESS) {
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
        case QFlavorAText:
            if (set_atext_data(ui->atext_ui, q, caller) != RETURN_SUCCESS) {
                res = RETURN_FAILURE;
            }
            break;
        case QFlavorRegion:
            if (set_region_data(ui->region_ui, q, caller) != RETURN_SUCCESS) {
                res = RETURN_FAILURE;
            }
            break;
        default:
            res = RETURN_FAILURE;
            break;
        }
    }
    xfree(items.items);

    for (i = 0; i < gpcount; i++) {
        explorer_snapshot(gapp, gplist[i], FALSE);
    }
    xfree(gplist);

    return res;
}

static int explorer_aac(void *data)
{
    ExplorerUI *ui = (ExplorerUI *) data;
    
    return explorer_apply(ui, NULL);
}

void explorer_before_undo(GraceApp *gapp, Quark *pr)
{
    ExplorerUI *eui = gapp->gui->eui;

    if (!eui) {
        return;
    }

    explorer_save_quark_state(eui);
    TreeDeleteItem(eui->tree, quark_get_udata(pr));
}

void explorer_after_undo(GraceApp *gapp, Quark *pr)
{
    ExplorerUI *eui = gapp->gui->eui;

    if (!eui) {
        return;
    }

    quark_traverse(pr, create_hook, eui);
    explorer_restore_quark_state(eui);
}


#define HIDE_CB           0
#define SHOW_CB           1
#define DELETE_CB         2
#define DUPLICATE_CB      3
#define ADD_FRAME_CB      4
#define ADD_GRAPH_CB      5
#define ADD_SSD_CB        6
#define ADD_SET_CB        7
#define ADD_AXISGRID_CB   8
#define ADD_AXIS_CB       9
#define ADD_LINE_CB      10
#define ADD_BOX_CB       11
#define ADD_ARC_CB       12
#define ADD_TEXT_CB      13

static void popup_any_cb(ExplorerUI *eui, int type)
{
    TreeItemList items;
    int count, i;
    Quark *qnew = NULL;
    Quark *q;
    GProject *gp;

    int gpcount = 0;
    GProject **gplist;
    
    TreeGetHighlighted(eui->tree, &items);
    count = items.count;

    if (!count) {
        xfree(items.items);
        return;
    }

    gplist = xmalloc(gapp->gpcount*sizeof(GProject));
    if (!gplist) {
        return;
    }
    
    for (i = 0; i < count; i++) {
        
        q = TreeGetQuark(items.items[i]);
        gp = gproject_from_quark(q);
        add_to_list(gplist, &gpcount, gp);
        
        switch (type) {
        case HIDE_CB:
            quark_set_active(q, FALSE);
            break;
        case SHOW_CB:
            quark_set_active(q, TRUE);
            break;
        case DELETE_CB:
            quark_free(q);
            break;
        case DUPLICATE_CB:
            quark_copy(q);
            break;
        case ADD_FRAME_CB:
            qnew = frame_new(q);
            break;
        case ADD_GRAPH_CB:
            qnew = graph_new(q);
            break;
        case ADD_SSD_CB:
            qnew = gapp_ssd_new(q);
            break;
        case ADD_SET_CB:
            qnew = gapp_set_new(q);
            break;
        case ADD_AXISGRID_CB:
            qnew = axisgrid_new(q);
            break;
        case ADD_AXIS_CB:
            qnew = axis_new(q);
            break;
        case ADD_LINE_CB:
            qnew = object_new_complete(q, DO_LINE);
            break;
        case ADD_BOX_CB:
            qnew = object_new_complete(q, DO_BOX);
            break;
        case ADD_ARC_CB:
            qnew = object_new_complete(q, DO_ARC);
            break;
        case ADD_TEXT_CB:
            qnew = atext_new(q);
            break;
        }
    }
    xfree(items.items);
    
    for (i = 0; i < gpcount; i++) {
        explorer_snapshot(gapp, gplist[i], TRUE);
    }
    xfree(gplist);

    if (qnew) {
        select_quark_explorer(qnew);
    }
}

static void hide_cb(Widget but, void *udata)
{
    popup_any_cb((ExplorerUI *) udata, HIDE_CB);
}

static void show_cb(Widget but, void *udata)
{
    popup_any_cb((ExplorerUI *) udata, SHOW_CB);
}

static void delete_cb(Widget but, void *udata)
{
    if (yesno("Really delete selected item(s)?", NULL, NULL, NULL)) {
        popup_any_cb((ExplorerUI *) udata, DELETE_CB);
    }
}

static void duplicate_cb(Widget but, void *udata)
{
    popup_any_cb((ExplorerUI *) udata, DUPLICATE_CB);
}

static void add_frame_cb(Widget but, void *udata)
{
    popup_any_cb((ExplorerUI *) udata, ADD_FRAME_CB);
}

static void add_graph_cb(Widget but, void *udata)
{
    popup_any_cb((ExplorerUI *) udata, ADD_GRAPH_CB);
}

static void add_ssd_cb(Widget but, void *udata)
{
    popup_any_cb((ExplorerUI *) udata, ADD_SSD_CB);
}

static void add_set_cb(Widget but, void *udata)
{
    popup_any_cb((ExplorerUI *) udata, ADD_SET_CB);
}

static void add_axisgrid_cb(Widget but, void *udata)
{
    popup_any_cb((ExplorerUI *) udata, ADD_AXISGRID_CB);
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

#define PROJECT_SHOW_CB            0
#define PROJECT_SAVE_CB            1
#define PROJECT_SAVE_AS_CB         2
#define PROJECT_REVERT_TO_SAVED_CB 3
#define PROJECT_CLOSE_CB           4

static void project_popup_any_cb(ExplorerUI *eui, int type)
{
    TreeItemList items;
    Quark *q;
    GraceApp *gapp;
    GProject *gp;

    TreeGetHighlighted(eui->tree, &items);

    if (!items.count || items.count > 1) {
        xfree(items.items);
        return;
    }

    q = TreeGetQuark(items.items[0]);
    gapp = gapp_from_quark(q);
    gp = gproject_from_quark(q);

    switch (type) {
    case PROJECT_SHOW_CB:
        gapp_set_active_gproject(gapp, gp);
        xdrawgraph(gp);
        update_all();
        break;
    case PROJECT_SAVE_CB:
        project_save(gp);
        break;
    case PROJECT_SAVE_AS_CB:
        project_save_as(gp);
        break;
    case PROJECT_REVERT_TO_SAVED_CB:
        revert_project(gapp, gp);
        xdrawgraph(gapp->gp);
        update_all();
        break;
    case PROJECT_CLOSE_CB:
        if (gapp->gpcount == 1) {
            errmsg("Can't close the last project");
            return;
        }

        if (gp && gproject_get_top(gp) &&
            quark_dirtystate_get(gproject_get_top(gp)) &&
            !yesno("Abandon unsaved changes?", NULL, NULL, NULL)) {
            return;
        }

        gapp_delete_gproject(gapp, gp);

        if (gapp->gp == NULL) {
            gapp_set_active_gproject(gapp, gapp->gplist[0]);
        }

        xdrawgraph(gapp->gp);
        update_all();
        break;
    }

    xfree(items.items);
}

static void project_show_cb(Widget but, void *udata)
{
    project_popup_any_cb((ExplorerUI *) udata, PROJECT_SHOW_CB);
}

static void project_save_cb(Widget but, void *udata)
{
    project_popup_any_cb((ExplorerUI *) udata, PROJECT_SAVE_CB);
}

static void project_save_as_cb(Widget but, void *udata)
{
    project_popup_any_cb((ExplorerUI *) udata, PROJECT_SAVE_AS_CB);
}

static void project_revert_to_saved_cb(Widget but, void *udata)
{
    project_popup_any_cb((ExplorerUI *) udata, PROJECT_REVERT_TO_SAVED_CB);
}

static void project_close_cb(Widget but, void *udata)
{
    project_popup_any_cb((ExplorerUI *) udata, PROJECT_CLOSE_CB);
}

void raise_explorer(GUI *gui, Quark *q)
{
    GraceApp *gapp = gui->P;

    set_wait_cursor();
    
    if (!gui->eui) {
        ExplorerUI *eui;
        Widget menubar, menupane, panel, form, fr;

        eui = xmalloc(sizeof(ExplorerUI));
        gui->eui = eui;

        /* Create pixmaps */
        CreatePixmaps(eui);

        eui->top = CreateDialogForm(app_shell, "Explorer");
        menubar = CreateMenuBar(eui->top);
        ManageChild(menubar);
        FormAddVChild(eui->top, menubar);

        menupane = CreateMenu(menubar, "File", 'F', FALSE);
        CreateMenuButton(menupane,
            "Close", 'C', destroy_dialog_cb, GetParent(eui->top));

        menupane = CreateMenu(menubar, "Edit", 'E', FALSE);
        eui->edit_undo_bt = CreateMenuButtonA(menupane, "Undo", 'U', "Ctrl+Z",
            undo_cb, gapp);
        eui->edit_redo_bt = CreateMenuButtonA(menupane, "Redo", 'R', "Ctrl+Shift+Z",
            redo_cb, gapp);

        menupane = CreateMenu(menubar, "Insert", 'I', FALSE);
        eui->insert_frame_bt = CreateMenuButton(menupane,
            "Frame", '\0', add_frame_cb, eui);
        SetSensitive(eui->insert_frame_bt,  FALSE);
        eui->insert_graph_bt = CreateMenuButton(menupane,
            "Graph", '\0', add_graph_cb, eui);
        SetSensitive(eui->insert_graph_bt,  FALSE);
        eui->insert_ssd_bt = CreateMenuButton(menupane,
            "SSData", '\0', add_ssd_cb, eui);
        SetSensitive(eui->insert_ssd_bt,    FALSE);
        eui->insert_set_bt = CreateMenuButton(menupane,
            "Set", '\0', add_set_cb, eui);
        SetSensitive(eui->insert_set_bt,    FALSE);
        eui->insert_axisgrid_bt = CreateMenuButton(menupane,
            "Axis grid", '\0', add_axisgrid_cb, eui);
        SetSensitive(eui->insert_axisgrid_bt,   FALSE);
        eui->insert_axis_bt = CreateMenuButton(menupane,
            "Axis", '\0', add_axis_cb, eui);
        SetSensitive(eui->insert_axis_bt,   FALSE);
        eui->insert_object_pane = CreateMenu(menupane, "Annotating objects", 'o', FALSE);
        SetSensitive(eui->insert_object_pane, FALSE);
        eui->insert_text_bt = CreateMenuButton(eui->insert_object_pane,
            "Text", '\0', add_object_cb, eui);
        eui->insert_line_bt = CreateMenuButton(eui->insert_object_pane,
            "Line", '\0', add_object_cb, eui);
        eui->insert_box_bt = CreateMenuButton(eui->insert_object_pane,
            "Box", '\0', add_object_cb, eui);
        eui->insert_arc_bt = CreateMenuButton(eui->insert_object_pane,
            "Arc", '\0', add_object_cb, eui);

        menupane = CreateMenu(menubar, "Options", 'O', FALSE);
        eui->instantupdate = CreateMenuToggle(menupane, "Instantaneous update",
                            'u', NULL, NULL);
        SetToggleButtonState(eui->instantupdate, gui->instant_update);

        menupane = CreateMenu(menubar, "Help", 'H', TRUE);
        CreateMenuHelpButton(menupane, "On explorer", 'e',
            eui->top, "doc/UsersGuide.html#explorer");

        panel = CreatePanedWindow(eui->top);

        form = CreateForm(panel);
        PlaceGridChild(panel, form, 0, 0); /* in case if CreatePanedWindow creates a grid */

        eui->tree = CreateTree(form);
        AddTreeHighlightItemsCB(eui->tree, highlight_cb, eui);
        AddTreeContextMenuCB(eui->tree, menu_cb, eui);
        AddTreeDropItemsCB(eui->tree, drop_cb, eui);
        FormAddVChild(form, GetParent(eui->tree));

        fr = CreateFrame(form, NULL);
        eui->idstr = CreateTextInput(fr, "ID string:");
        AddTextInputCB(eui->idstr, text_explorer_cb, eui);
        FormAddVChild(form, fr);
        FormFixateVChild(fr);
        
        eui->scrolled_window = CreateScrolledWindow(panel);
        PlaceGridChild(panel, eui->scrolled_window, 1, 0); /* in case if CreatePanedWindow creates a grid */

        SetMinimumDimensions(form, 150, 0);
        SetDimensions(form, 250, 0);

#ifdef HAVE_LESSTIF
# if !defined(SF_BUG_993209_FIXED) && !defined(SF_BUG_993209_NOT_FIXED)
#  error "You should check whether SF bug #993209 is fixed in your version of LessTif."
#  error "Then define either SF_BUG_993209_FIXED or SF_BUG_993209_NOT_FIXED, accordingly."
#  error "See http://sourceforge.net/tracker/index.php?func=detail&aid=993209&group_id=8596&atid=108596."
# endif
# ifdef SF_BUG_993209_NOT_FIXED
        /* A dirty workaround */
        eui->scrolled_window = CreateVContainer(eui->scrolled_window);
# endif
#endif

        eui->project_ui = create_project_ui(eui);
        UnmanageChild(eui->project_ui->top);

	eui->ssd_ui = create_ssd_ui(eui);
        UnmanageChild(eui->ssd_ui->top);

	eui->frame_ui = create_frame_ui(eui);
        UnmanageChild(eui->frame_ui->top);

	eui->graph_ui = create_graph_ui(eui);
        UnmanageChild(eui->graph_ui->top);

	eui->set_ui = create_set_ui(eui);
        UnmanageChild(eui->set_ui->top);

	eui->axisgrid_ui = create_axisgrid_ui(eui);
        UnmanageChild(eui->axisgrid_ui->top);

	eui->axis_ui = create_axis_ui(eui);
        UnmanageChild(eui->axis_ui->top);

	eui->object_ui = create_object_ui(eui);
        UnmanageChild(eui->object_ui->top);

	eui->atext_ui = create_atext_ui(eui);
        UnmanageChild(eui->atext_ui->top);

	eui->region_ui = create_region_ui(eui);
        UnmanageChild(eui->region_ui->top);

        eui->aacbuts = CreateAACDialog(eui->top, panel, explorer_aac, eui);

        ManageChild(eui->tree);

        /* Menu popup */
        eui->popup = CreatePopupMenu(eui->tree);
        eui->popup_hide_bt = CreateMenuButton(eui->popup,
            "Hide", '\0', hide_cb, eui);
        eui->popup_show_bt = CreateMenuButton(eui->popup,
            "Show", '\0', show_cb, eui);

        CreateMenuSeparator(eui->popup);

        eui->popup_delete_bt = CreateMenuButton(eui->popup,
            "Delete", '\0', delete_cb, eui);
        eui->popup_duplicate_bt = CreateMenuButton(eui->popup,
            "Duplicate", '\0', duplicate_cb, eui);

        /* Project menu popup */
        eui->project_popup = CreatePopupMenu(eui->tree);
        eui->project_popup_show_bt = CreateMenuButton(eui->project_popup,
            "Show", '\0', project_show_cb, eui);

        CreateMenuSeparator(eui->project_popup);

        eui->project_popup_save_bt = CreateMenuButton(eui->project_popup,
            "Save", '\0', project_save_cb, eui);
        eui->project_popup_save_as_bt = CreateMenuButton(eui->project_popup,
            "Save as...", '\0', project_save_as_cb, eui);
        eui->project_popup_revert_to_saved_bt = CreateMenuButton(eui->project_popup,
            "Revert to saved", '\0', project_revert_to_saved_cb, eui);

        CreateMenuSeparator(eui->project_popup);

        eui->project_popup_close_bt = CreateMenuButton(eui->project_popup,
            "Close", '\0', project_close_cb, eui);

        init_quark_tree(eui);

        if (!q && gapp->gp) {
            select_quark_explorer(gproject_get_top(gapp->gp));
        }
    }
#ifdef QT_GUI
    /* TODO: use resources */
    SetDimensions(gui->eui->top, 650, 600);
#endif
    RaiseWindow(GetParent(gui->eui->top));
    
    if (q) {
        select_quark_explorer(q);
    }

    update_undo_buttons(gapp->gp);

    unset_wait_cursor();
}

void define_explorer_popup(Widget but, void *data)
{
    GUI *gui = (GUI *) data;
    raise_explorer(gui, NULL);
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
void pen_explorer_cb(Widget but, const Pen *pen, void *data)
{
    ExplorerUI *eui = (ExplorerUI *) data;
    explorer_apply(eui, but);
}
void format_explorer_cb(FormatStructure *fstr, const Format *format, void *data)
{
    ExplorerUI *eui = (ExplorerUI *) data;
    explorer_apply(eui, fstr);
}
