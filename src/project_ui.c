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

#include <stdlib.h>

#include <Xm/Text.h>
       
#include "explorer.h"
#include "protos.h"

static void wrap_year_cb(Widget but, int onoff, void *data)
{
    Widget wrap_year = (Widget) data;
    
    SetSensitive(wrap_year, onoff);
}

ProjectUI *create_project_ui(ExplorerUI *eui)
{
    ProjectUI *ui;
    Widget form, fr, rc, rc1;

    OptionItem opitems[4] = {
        {FMT_iso,      "ISO"     },
        {FMT_european, "European"},
        {FMT_us,       "US"      },
        {FMT_nohint,   "None"    }
    };
    
    form = CreateVContainer(eui->scrolled_window);
    
    ui = xmalloc(sizeof(ProjectUI));

    fr = CreateFrame(form, NULL);
    rc = CreateVContainer(fr);
    ui->description  = CreateScrollTextItem2(rc, 5, "Project description:");
    AddTextItemCB(ui->description, titem_explorer_cb, eui);
    ui->sformat = CreateTextItem2(rc, 15, "Data format:");
    AddTextItemCB(ui->sformat, titem_explorer_cb, eui);
    
    fr = CreateFrame(form, "Page background");
    AddDialogFormChild(form, fr);
    rc = CreateHContainer(fr);
    ui->bg_color = CreateColorChoice(rc, "Color:");
    AddOptionChoiceCB(ui->bg_color, oc_explorer_cb, eui);
    ui->bg_fill = CreateToggleButton(rc, "Fill");
    AddToggleButtonCB(ui->bg_fill, tb_explorer_cb, eui);

    fr = CreateFrame(form, "Dates");
    rc1 = CreateVContainer(fr);
    ui->datehint = CreateOptionChoice(rc1, "Date hint", 0, 4, opitems);
    AddOptionChoiceCB(ui->datehint, oc_explorer_cb, eui);
    ui->refdate = CreateTextItem2(rc1, 20, "Reference date:");
    AddTextItemCB(ui->refdate, titem_explorer_cb, eui);
    rc = CreateHContainer(rc1);
    ui->two_digits_years = CreateToggleButton(rc, "Two-digit year span");
    AddToggleButtonCB(ui->two_digits_years, wrap_year_cb, ui->wrap_year);
    AddToggleButtonCB(ui->two_digits_years, tb_explorer_cb, eui);
    ui->wrap_year = CreateTextItem2(rc, 4, "Wrap year:");
    AddTextItemCB(ui->wrap_year, titem_explorer_cb, eui);

    
    ui->top = form;
    
    return ui;
}

void update_project_ui(ProjectUI *ui, Quark *q)
{
    Project *pr = project_get_data(q);
    if (pr) {
        int y, m, d, h, mm, sec;
        char date_string[64], wrap_year_string[64];

        xv_setstr(ui->sformat, project_get_sformat(q));
        xv_setstr(ui->description, project_get_description(q));
        
        SetOptionChoice(ui->bg_color, pr->bgcolor);
        SetToggleButtonState(ui->bg_fill, pr->bgfill);

    	SetOptionChoice(ui->datehint, get_date_hint());
	jul_to_cal_and_time(q, 0.0, ROUND_SECOND, &y, &m, &d, &h, &mm, &sec);
	sprintf(date_string, "%d-%02d-%02d %02d:%02d:%02d",
                y, m, d, h, mm, sec);
        xv_setstr(ui->refdate, date_string);
        SetToggleButtonState(ui->two_digits_years, pr->two_digits_years);
        sprintf(wrap_year_string, "%04d", pr->wrap_year);
        xv_setstr(ui->wrap_year, wrap_year_string);
        SetSensitive(ui->wrap_year, pr->two_digits_years ? TRUE:FALSE);
    }
}

int set_project_data(ProjectUI *ui, Quark *q, void *caller)
{
    Project *pr = project_get_data(q);
    int retval = RETURN_SUCCESS;
    
    if (ui && pr) {
        double jul;
    
        if (!caller || caller == ui->sformat) {
            project_set_sformat(q, xv_getstr(ui->sformat));
        }
        if (!caller || caller == ui->description) {
            char *s = XmTextGetString(ui->description);
            project_set_description(q, s);
            XtFree(s);
        }
        
        if (!caller || caller == ui->bg_color) {
            pr->bgcolor = GetOptionChoice(ui->bg_color);
        }
        if (!caller || caller == ui->bg_fill) {
            pr->bgfill = GetToggleButtonState(ui->bg_fill);
        }

        if (!caller || caller == ui->datehint) {
            set_date_hint(GetOptionChoice(ui->datehint));
        }
        if (!caller || caller == ui->refdate) {
            if (parse_date_or_number(q, xv_getstr(ui->refdate), TRUE, &jul) ==
                RETURN_SUCCESS) {
                pr->ref_date = jul;
            } else {
                errmsg("Invalid date");
                retval = RETURN_FAILURE;
            }
        }
        if (!caller || caller == ui->two_digits_years) {
            pr->two_digits_years = GetToggleButtonState(ui->two_digits_years);
        }
        if (!caller || caller == ui->wrap_year) {
            pr->wrap_year = atoi(xv_getstr(ui->wrap_year));
        }

        quark_dirtystate_set(q, TRUE);
    }
    
    return retval;
}
