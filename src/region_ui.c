/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2004 Grace Development Team
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

/* Region UI */

#include <stdlib.h>

#include "explorer.h"
#include "protos.h"

RegionUI *create_region_ui(ExplorerUI *eui)
{
    RegionUI *ui;
    Widget form;

    form = CreateVContainer(eui->scrolled_window);
    AddHelpCB(form, "doc/UsersGuide.html#region-properties");
    
    ui = xmalloc(sizeof(ProjectUI));

    ui->top = form;
    
    ui->type = CreateOptionChoiceVA(form, "Type:",
        "Polygon", REGION_POLYGON,
        "Band",    REGION_BAND,
        /* "Formula", REGION_FORMULA, */
        NULL);
    AddOptionChoiceCB(ui->type, oc_explorer_cb, eui);

    ui->color = CreateColorChoice(form, "Color:");
    AddOptionChoiceCB(ui->color, oc_explorer_cb, eui);

    return ui;
}

void update_region_ui(RegionUI *ui, Quark *q)
{
    region *r = region_get_data(q);
    if (ui && r) {
        SetOptionChoice(ui->type, r->type);
        SetOptionChoice(ui->color, r->color);
    }
}

int set_region_data(RegionUI *ui, Quark *q, void *caller)
{
    int retval = RETURN_SUCCESS;
    
    if (ui && q) {
        if (!caller || caller == ui->type) {
            region_set_type(q, GetOptionChoice(ui->type));
        }
        if (!caller || caller == ui->color) {
            region_set_color(q, GetOptionChoice(ui->color));
        }
    }
    
    return retval;
}
