/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1996-2003 Grace Development Team
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

/* Project UI */

#include "explorer.h"

ProjectUI *create_project_ui(ExplorerUI *eui)
{
    ProjectUI *ui;
    Widget form, fr, rc;
    
    form = eui->scrolled_window;
    
    ui = xmalloc(sizeof(ProjectUI));
    
    fr = CreateFrame(form, "Page background");
    AddDialogFormChild(form, fr);
    rc = CreateHContainer(fr);
    ui->bg_color = CreateColorChoice(rc, "Color:");
    AddOptionChoiceCB(ui->bg_color, oc_explorer_cb, eui);
    ui->bg_fill = CreateToggleButton(rc, "Fill");
    AddToggleButtonCB(ui->bg_fill, tb_explorer_cb, eui);
    
    ui->top = fr;
    
    return ui;
}

void update_project_ui(ProjectUI *ui, Quark *q)
{
    Project *pr = project_get_data(q);
    if (pr) {
        SetOptionChoice(ui->bg_color, pr->bgcolor);
        SetToggleButtonState(ui->bg_fill, pr->bgfill);
    }
}

int set_project_data(ProjectUI *ui, Quark *q, void *caller)
{
    Project *pr = project_get_data(q);
    if (pr) {
        if (!caller || caller == ui->bg_color) {
            pr->bgcolor = GetOptionChoice(ui->bg_color);
        }
        if (!caller || caller == ui->bg_fill) {
            pr->bgfill = GetToggleButtonState(ui->bg_fill);
        }

        quark_dirtystate_set(q, TRUE);
        
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}
