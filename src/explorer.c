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
    int all_shown = TRUE;
    int all_hidden = TRUE;

    TreeGetHighlighted(event->w, &items);
    count = items.count;
    
    ui->homogeneous_selection = TRUE;
    ui->all_siblings = TRUE;

    if (count > 0) {
        int i;
        TreeItem *item = (TreeItem *) items.items[0];
        Quark *parent;

        q = TreeGetQuark(item);
        fid = quark_fid_get(q);
        parent = quark_parent_get(q);
        all_shown  = quark_is_active(q);
        all_hidden = !all_shown;

        for (i = 1; i < count; i++) {
            item = items.items[i];
            
            if ((int) quark_fid_get(q) != fid) {
                ui->homogeneous_selection = FALSE;
            }
            if (quark_parent_get(q) != parent) {
                ui->all_siblings = FALSE;
            }
            if (quark_is_active(q)) {
                all_hidden = FALSE;
            } else {
                all_shown = FALSE;
            }
        }
    }
    xfree(items.items);

    if (!count || !ui->homogeneous_selection) {
        SetSensitive(ui->aacbuts[0], FALSE);
        SetSensitive(ui->aacbuts[1], FALSE);
        
        manage_plugin(ui, NULL);
    } else {
        SetSensitive(ui->aacbuts[0], TRUE);
        SetSensitive(ui->aacbuts[1], TRUE);
        
        if (count == 1) {
            Widget managed_top;
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
    }

    if (!count || fid == QFlavorProject) {
        SetSensitive(ui->popup_hide_bt,           FALSE);
        SetSensitive(ui->popup_show_bt,           FALSE);
    } else {
        SetSensitive(ui->popup_hide_bt, !all_hidden);
        SetSensitive(ui->popup_show_bt, !all_shown);
    }
        
    if (!count || !ui->all_siblings || fid == QFlavorProject) {
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
            SetSensitive(ui->insert_ssd_bt, TRUE);
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

    ShowMenu(ui->popup, event->udata);

    return TRUE;
}

static int drop_cb(TreeEvent *event)
{
    ExplorerUI *ui = (ExplorerUI *) event->anydata;
    TreeItem *item = (TreeItem *) event->udata;
    Quark *drop_q = TreeGetQuark(item);

    if (ui->all_siblings && ui->homogeneous_selection) {
        int count;
        TreeItemList items;

        TreeGetHighlighted(ui->tree, &items);
        count = items.count;
        printf("count %d\n", count);
        if (count > 0) {
            int i;
            Quark *parent;
            TreeItem *item = items.items[0];
            parent = quark_parent_get(TreeGetQuark(item));

            if (parent && parent != drop_q &&
                quark_fid_get(parent) == quark_fid_get(drop_q)) {
                for (i = 0; i < count; i++) {
                    Quark *q;
                    item = items.items[i];
                    q = TreeGetQuark(item);
                    if (event->drop_action == DROP_ACTION_COPY) {
                        quark_copy2(drop_q, q);
                    } else {
                        quark_reparent(q, drop_q);
                    }
                }
                snapshot_and_update(gapp->gp, TRUE);
                return TRUE;
            }
        }
    }
    return FALSE;
}

static int create_hook(Quark *q, void *udata, QTraverseClosure *closure)
{
    TreeItem *item;
    ExplorerUI *eui = (ExplorerUI *) udata;
    Quark *qparent = quark_parent_get(q);

    if (qparent) {
        TreeItem *parent = quark_get_udata(qparent);
        item = TreeAddItem(eui->tree, parent, q);
    } else {
        item = TreeAddItem(eui->tree, NULL, q);
    }

    if (quark_is_active(q) && quark_count_children(q) > 0) {
        TreeSetItemOpen(eui->tree, item, TRUE);
    }

    if (quark_is_active(q)) {
        TreeSetItemPixmap(item, eui->a_icon);
    } else {
        TreeSetItemPixmap(item, eui->h_icon);
    }

    quark_set_udata(q, item);

    return TRUE;
}

static int reparent_hook(Quark *q, void *udata, QTraverseClosure *closure)
{
    ExplorerUI *eui = (ExplorerUI *) udata;
    TreeItem *item = quark_get_udata(q);

    if ((closure->depth == 0) && (closure->step == 0)) {
        TreeDeleteItem(eui->tree, item);
    }
    create_hook(q, eui, NULL);

    return TRUE;
}

static int explorer_cb(Quark *q, int etype, void *udata)
{
    ExplorerUI *eui = (ExplorerUI *) udata;
    TreeItem *item = quark_get_udata(q);

    switch (etype) {
    case QUARK_ETYPE_DELETE:
        TreeDeleteItem(eui->tree, item);
        break;
    case QUARK_ETYPE_MODIFY:
        TreeSetItemText(item, q_labeling(q));

        if (quark_is_active(q) && quark_count_children(q) > 0) {
            TreeSetItemOpen(eui->tree, item, TRUE);
        } else {
            TreeSetItemOpen(eui->tree, item, FALSE);
        }

        if (quark_is_active(q)) {
            TreeSetItemPixmap(item, eui->a_icon);
        } else {
            TreeSetItemPixmap(item, eui->h_icon);
        }
        break;
    case QUARK_ETYPE_REPARENT:
        quark_traverse(q, reparent_hook, eui);
        break;
    case QUARK_ETYPE_NEW:
        create_hook(q, eui, NULL);
        break;
    default:
        printf("Else event Quark: %s\n", q_labeling(q));
    }

    return TRUE;
}

static void init_quark_tree(ExplorerUI *eui)
{
    quark_traverse(gproject_get_top(gapp->gp), create_hook, eui);
    quark_cb_add(NULL, explorer_cb, eui);
}

void select_quark_explorer(Quark *q)
{
    GUI *gui = gui_from_quark(q);

    if (gui->eui) {
        TreeItem *item = quark_get_udata(q);

        TreeClearSelection(gui->eui->tree);
        TreeSelectItem(gui->eui->tree, item);
        TreeScrollToItem(gui->eui->tree, item);
    }
}

static int explorer_apply(ExplorerUI *ui, void *caller)
{
    TreeItemList items;
    int count, i, res = RETURN_SUCCESS;
    
    if (caller && !GetToggleButtonState(ui->instantupdate)) {
        return RETURN_FAILURE;
    }
    
    TreeGetHighlighted(ui->tree, &items);
    count = items.count;

    for (i = 0; i < count && res == RETURN_SUCCESS; i++) {
        TreeItem *item = items.items[i];
        Quark *q = TreeGetQuark(item);

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
    
    snapshot_and_update(gapp->gp, FALSE);

    return res;
}

static int explorer_aac(void *data)
{
    ExplorerUI *ui = (ExplorerUI *) data;
    
    return explorer_apply(ui, NULL);
}

void update_explorer(ExplorerUI *eui)
{
    TreeDeleteItem(eui->tree, NULL);
    quark_traverse(gproject_get_top(gapp->gp), create_hook, eui);
    TreeSelectItem(eui->tree, NULL);
}

#define HIDE_CB           0
#define SHOW_CB           1
#define DELETE_CB         2
#define DUPLICATE_CB      3
#define BRING_TO_FRONT_CB 4
#define SEND_TO_BACK_CB   5
#define MOVE_UP_CB        6
#define MOVE_DOWN_CB      7
#define ADD_FRAME_CB      8
#define ADD_GRAPH_CB      9
#define ADD_SSD_CB       10
#define ADD_SET_CB       11
#define ADD_AXISGRID_CB  12
#define ADD_AXIS_CB      13
#define ADD_LINE_CB      14
#define ADD_BOX_CB       15
#define ADD_ARC_CB       16
#define ADD_TEXT_CB      17

static void popup_any_cb(ExplorerUI *eui, int type)
{
    TreeItemList items;
    int count, i;
    Quark *qnew = NULL;
    
    TreeGetHighlighted(eui->tree, &items);
    count = items.count;
    
    for (i = 0; i < count; i++) {
        TreeItem *item;
        Quark *q;
        
        switch (type) {
        case SEND_TO_BACK_CB:
        case MOVE_UP_CB:
            item = items.items[count - i - 1];
            break;
        default:
            item = items.items[i];
            break;
        }

        q = TreeGetQuark(item);
        
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
    
    snapshot_and_update(gapp->gp, TRUE);
    
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
        AddDialogFormChild(eui->top, menubar);

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

        eui->tree = CreateTree(form);
        AddTreeHighlightItemsCB(eui->tree, highlight_cb, eui);
        AddTreeContextMenuCB(eui->tree, menu_cb, eui);
        AddTreeDropItemsCB(eui->tree, drop_cb, eui);

        fr = CreateFrame(form, NULL);
        eui->idstr = CreateTextInput(fr, "ID string:");
        AddTextInputCB(eui->idstr, text_explorer_cb, eui);
#ifndef QT_GUI
        XtVaSetValues(GetParent(eui->tree),
            XmNleftAttachment, XmATTACH_FORM,
            XmNrightAttachment, XmATTACH_FORM,
            XmNtopAttachment, XmATTACH_FORM,
            XmNbottomAttachment, XmATTACH_WIDGET,
            XmNbottomWidget, fr,
            NULL);
        XtVaSetValues(fr,
            XmNleftAttachment, XmATTACH_FORM,
            XmNrightAttachment, XmATTACH_FORM,
            XmNtopAttachment, XmATTACH_NONE,
            XmNbottomAttachment, XmATTACH_FORM,
            NULL);
#endif
	ManageChild(form);
        
        eui->scrolled_window = CreateScrolledWindow(panel);

        ManageChild(panel);
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

        init_quark_tree(eui);
        
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

        CreateMenuSeparator(eui->popup);

        eui->popup_bring_to_front_bt = CreateMenuButton(eui->popup,
            "Bring to front", '\0', bring_to_front_cb, eui);
        eui->popup_move_up_bt = CreateMenuButton(eui->popup,
            "Move up", '\0', move_up_cb, eui);
        eui->popup_move_down_bt = CreateMenuButton(eui->popup,
            "Move down", '\0', move_down_cb, eui);
        eui->popup_send_to_back_bt = CreateMenuButton(eui->popup,
            "Send to back", '\0', send_to_back_cb, eui);
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
void titem_explorer_cb(Widget w, char *s, void *data)
{
    ExplorerUI *eui = (ExplorerUI *) data;
    explorer_apply(eui, w);
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
