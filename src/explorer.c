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

#include "motifinc.h"
#include "ListTree.h"

#include "protos.h"

/* Storage labeling procedure */
typedef ListTreeItem * (*Storage_CreateProc)(
    Widget w,
    ListTreeItem *parent,
    char *string,
    void *udata
);

typedef struct {
    Widget tree;
    Storage *sto;
    int nchoices;
    Storage_LabelingProc labeling_proc;
    Storage_CreateProc create_proc;
} TreeItemData;

static char *default_storage_labeling_proc(unsigned int step, void *data)
{
    char buf[128];
    
    sprintf(buf, "Item #%d (data = %p)", step, data);
    
    return copy_string(NULL, buf);
}

static ListTreeItem *default_storage_create_proc(Widget w,
    ListTreeItem *parent, char *string, void *udata)
{
    ListTreeItem *item = ListTreeAdd(w, parent, string);
    return item;
}

ListTreeItem *CreateStorageTree(Widget tree, ListTreeItem *parent, char *label)
{
    ListTreeItem *item;
    TreeItemData *data;
    
    item = ListTreeAdd(tree, parent, label);
    
    data = xmalloc(sizeof(TreeItemData));
    data->tree = tree;
    data->sto = NULL;
    data->nchoices = 0;
    data->labeling_proc = default_storage_labeling_proc;
    data->create_proc   = default_storage_create_proc;
    
    item->user_data = data;
    
    return item;
}

void SetStorageTreeProcs(ListTreeItem *ti,
    Storage_LabelingProc labeling_proc, Storage_CreateProc create_proc)
{
    TreeItemData *data = (TreeItemData *) ti->user_data;
    if (labeling_proc) {
        data->labeling_proc = labeling_proc;
    }
    if (create_proc) {
        data->create_proc = create_proc;
    }
}

static int traverse_hook(unsigned int step, void *data, void *udata)
{
    char *s;
    ListTreeItem *ti = (ListTreeItem *) udata;
    TreeItemData *ti_data = (TreeItemData *) ti->user_data;
    
    s = ti_data->labeling_proc(step, data);
    if (s) {
        if (step >= ti_data->nchoices) {
            ti_data->create_proc(ti_data->tree, ti, s, data);
            ti_data->nchoices++;
        } else {
            ListTreeRenameItem(ti_data->tree, ti, s);
        }
        xfree(s);
    }
    
    return TRUE;
}

void UpdateStorageTree(ListTreeItem *ti)
{
    TreeItemData *ti_data = (TreeItemData *) ti->user_data;
    if (!ti_data->sto || storage_count(ti_data->sto) <= 0) {
        ListTreeDeleteChildren(ti_data->tree, ti);
        ti_data->nchoices = 0;
    }
    if (ti_data->sto) {
        storage_traverse(ti_data->sto, traverse_hook, ti);
    }
}   

void SetStorageTreeStorage(ListTreeItem *ti, Storage *sto)
{
    TreeItemData *data = (TreeItemData *) ti->user_data;
    data->sto = sto;
    UpdateStorageTree(ti);
}

typedef struct {
    Widget top;
    ListTreeItem *grace;
    ListTreeItem *rt;
    ListTreeItem *project;
    ListTreeItem *graphs;
} ExplorerUI;

static char *graph_labeling(unsigned int step, void *data)
{
    char buf[128];
    graph *g = graph_get_data((Quark *) data);
    
    sprintf(buf, "(%c) Graph #%d", !g->hidden ? '+':'-', step);
    
    return copy_string(NULL, buf);
}

static char *set_labeling(unsigned int step, void *data)
{
    char buf[128];
    set *s = set_get_data((Quark *) data);
    
    sprintf(buf, "(%c) Set #%d (%s)",
        !s->hidden ? '+':'-', step, set_types(grace->rt, s->type));
    
    return copy_string(NULL, buf);
}

static char *dobject_labeling(unsigned int step, void *data)
{
    char buf[128];
    DObject *o = object_get_data((Quark *) data);
    
    sprintf(buf, "(%c) DObject #%d (%s)",
        o->active ? '+':'-', step, object_types(o->type));
    
    return copy_string(NULL, buf);
}

static ListTreeItem *graph_create(Widget w,
    ListTreeItem *parent, char *string, void *udata)
{
    ListTreeItem *graph_item, *set_item, *do_item;
    graph *g = graph_get_data((Quark *) udata);
    
    graph_item = ListTreeAdd(w, parent, string);
    
    set_item = CreateStorageTree(w, graph_item, "Sets");
    SetStorageTreeProcs(set_item, set_labeling, NULL);
    SetStorageTreeStorage(set_item, g->sets);
    
    do_item = CreateStorageTree(w, graph_item, "Drawing objects");
    SetStorageTreeProcs(do_item, dobject_labeling, NULL);
    SetStorageTreeStorage(do_item, g->dobjects);
    
    return graph_item;
}


static int explorer_aac(void *data)
{
    return RETURN_SUCCESS;
}

static ExplorerUI *eui = NULL;

void define_explorer_popup(void *data)
{
    set_wait_cursor();
    
    if (!eui) {
        Widget menubar, menupane, panel, tree;

        eui = xmalloc(sizeof(ExplorerUI));
        
        eui->top = CreateDialogForm(app_shell, "Explorer");
        menubar = CreateMenuBar(eui->top);
        ManageChild(menubar);
        AddDialogFormChild(eui->top, menubar);

        menupane = CreateMenu(menubar, "File", 'F', FALSE);
        CreateMenuButton(menupane,
            "Close", 'C', destroy_dialog_cb, GetParent(eui->top));

        menupane = CreateMenu(menubar, "Help", 'H', TRUE);
        CreateMenuHelpButton(menupane, "On explorer", 'e',
            eui->top, "doc/UsersGuide.html#explorer");

        panel = CreateGrid(eui->top, 2, 1);
        tree = XmCreateScrolledListTree(panel, "tree", NULL, 0);
        ManageChild(tree);
        PlaceGridChild(panel, GetParent(tree), 0, 0);

        eui->grace   = ListTreeAdd(tree, NULL, "Grace");
        eui->rt      = ListTreeAdd(tree, eui->grace, "Run-time options");
        eui->project = ListTreeAdd(tree, eui->grace, "Project");
        eui->graphs  = CreateStorageTree(tree, eui->project, "Graphs");
        SetStorageTreeProcs(eui->graphs, graph_labeling, graph_create);
        
        CreateAACDialog(eui->top, panel, explorer_aac, eui);
    }

    SetStorageTreeStorage(eui->graphs, project_get_graphs(grace->project));

    RaiseWindow(GetParent(eui->top));
    unset_wait_cursor();
}
