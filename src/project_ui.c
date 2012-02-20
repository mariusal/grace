/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
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

/* Project UI */

#include <stdlib.h>
       
#include "explorer.h"
#include "utils.h"
#include "xprotos.h"

static void wrap_year_cb(Widget but, int onoff, void *data)
{
    Widget wrap_year = (Widget) data;
    
    SetSensitive(wrap_year, onoff);
}

#define PAGE_UNITS_PP   0
#define PAGE_UNITS_IN   1
#define PAGE_UNITS_CM   2

static void do_format_toggle(OptionStructure *opt, int value, void *data)
{
    ProjectUI *ui = (ProjectUI *) data;
    int orientation;
    double px, py;
    int page_units;
    char buf[32];
    
    if (value == PAGE_FORMAT_CUSTOM) {
        SetSensitive(ui->page_x, TRUE);
        SetSensitive(ui->page_y, TRUE);
        SetSensitive(ui->page_orient->menu, FALSE);
        return;
    } else {
        SetSensitive(ui->page_x, FALSE);
        SetSensitive(ui->page_y, FALSE);
        SetSensitive(ui->page_orient->menu, TRUE);
    }
    
    switch (value) {
    case PAGE_FORMAT_USLETTER:
        px = 612.0;
        py = 792.0;
        break;
    case PAGE_FORMAT_A4:
        px = 595.0;
        py = 842.0;
        break;
    default:
        return;
    }

    
    page_units = GetOptionChoice(ui->page_size_unit);
    orientation = GetOptionChoice(ui->page_orient);
    
    switch (page_units) {
    case PAGE_UNITS_IN:
        px /= 72.0;
        py /= 72.0;
        break;
    case PAGE_UNITS_CM:
        px /= 72.0/CM_PER_INCH;
        py /= 72.0/CM_PER_INCH;
        break;
    }
    
    if ((orientation == PAGE_ORIENT_LANDSCAPE && px > py) ||
        (orientation == PAGE_ORIENT_PORTRAIT  && px < py) ) {
        sprintf (buf, "%.2f", px);
        xv_setstr(ui->page_x, buf);
        sprintf (buf, "%.2f", py);
        xv_setstr(ui->page_y, buf);
    } else {
        sprintf (buf, "%.2f", py);
        xv_setstr(ui->page_x, buf);
        sprintf (buf, "%.2f", px);
        xv_setstr(ui->page_y, buf);
    }
}

static void do_orient_toggle(OptionStructure *opt, int value, void *data)
{
    ProjectUI *ui = (ProjectUI *) data;
    double px, py;
    char buf[32];
    int orientation = value;

    if (xv_evalexpr(ui->page_x, &px) != RETURN_SUCCESS || 
        xv_evalexpr(ui->page_y, &py) != RETURN_SUCCESS ) {
        errmsg("Invalid page dimension(s)");
        return;
    }
    
    if ((orientation == PAGE_ORIENT_LANDSCAPE && px < py) ||
        (orientation == PAGE_ORIENT_PORTRAIT  && px > py) ) {
        sprintf (buf, "%.2f", py);
        xv_setstr(ui->page_x, buf);
        sprintf (buf, "%.2f", px);
        xv_setstr(ui->page_y, buf);
    }
}

static void do_units_toggle(OptionStructure *opt, int value, void *data)
{
    ProjectUI *ui = (ProjectUI *) data;
    char buf[32];
    double page_x, page_y;
    int page_units = value;
    
    if (xv_evalexpr(ui->page_x, &page_x) != RETURN_SUCCESS || 
        xv_evalexpr(ui->page_y, &page_y) != RETURN_SUCCESS ) {
        errmsg("Invalid page dimension(s)");
        return;
    }
    
    if (ui->current_page_units == page_units) {
        return;
    }
    
    switch (ui->current_page_units) {
    case PAGE_UNITS_IN:
        page_x *= 72.0;
        page_y *= 72.0;
        break;
    case PAGE_UNITS_CM:
        page_x *= 72.0/CM_PER_INCH;
        page_y *= 72.0/CM_PER_INCH;
        break;
    }

    switch (page_units) {
    case PAGE_UNITS_IN:
        page_x /= 72.0;
        page_y /= 72.0;
        break;
    case PAGE_UNITS_CM:
        page_x /= 72.0/CM_PER_INCH;
        page_y /= 72.0/CM_PER_INCH;
        break;
    }
    
    ui->current_page_units = page_units;
    
    sprintf (buf, "%.2f", page_x); 
    xv_setstr(ui->page_x, buf);
    sprintf (buf, "%.2f", page_y); 
    xv_setstr(ui->page_y, buf);
}

ProjectUI *create_project_ui(ExplorerUI *eui)
{
    ProjectUI *ui;
    Widget form, fr, rc, rc1;

    form = CreateVContainer(eui->scrolled_window);
    AddHelpCB(form, "doc/UsersGuide.html#project-properties");
    
    ui = xmalloc(sizeof(ProjectUI));
    ui->current_page_units = PAGE_UNITS_PP;

    fr = CreateFrame(form, "Project description");
    ui->description  = CreateScrolledTextInput(fr, "", 5);
    AddTextInputCB(ui->description, text_explorer_cb, eui);

    fr = CreateFrame(form, "Page dimensions");
    rc1 = CreateVContainer(fr);

    rc = CreateHContainer(rc1);
    ui->page_orient = CreatePaperOrientationChoice(rc, "Orientation:");
    AddOptionChoiceCB(ui->page_orient, do_orient_toggle, ui);
    AddOptionChoiceCB(ui->page_orient, oc_explorer_cb, eui);

    ui->page_format = CreatePaperFormatChoice(rc, "Size:");
    AddOptionChoiceCB(ui->page_format, do_format_toggle, ui);
    AddOptionChoiceCB(ui->page_format, oc_explorer_cb, eui);

    rc = CreateHContainer(rc1);
    ui->page_x = CreateTextItem(rc, 7, "Dimensions:");
    AddTextItemCB(ui->page_x, titem_explorer_cb, eui);
    ui->page_y = CreateTextItem(rc, 7, "x ");
    AddTextItemCB(ui->page_y, titem_explorer_cb, eui);
    ui->page_size_unit = CreateOptionChoiceVA(rc, " ",
        "pp", PAGE_UNITS_PP,
        "in", PAGE_UNITS_IN,
        "cm", PAGE_UNITS_CM,
        NULL);
    SetOptionChoice(ui->page_size_unit, ui->current_page_units);
    AddOptionChoiceCB(ui->page_size_unit, do_units_toggle, ui);

    
    fr = CreateFrame(form, "Page background");
    FormAddVChild(form, fr);
    rc = CreateHContainer(fr);
    ui->bg_color = CreateColorChoice(rc, "Color:");
    AddOptionChoiceCB(ui->bg_color, oc_explorer_cb, eui);
    ui->bg_fill = CreateToggleButton(rc, "Fill");
    AddToggleButtonCB(ui->bg_fill, tb_explorer_cb, eui);

    fr = CreateFrame(form, "Scaling factors");
    FormAddVChild(form, fr);
    rc = CreateVContainer(fr);
    ui->fsize_scale = CreateSpinChoice(rc, "Font size:", 5,
        SPIN_TYPE_FLOAT, 0.0, 1.0, 0.005);
    AddSpinChoiceCB(ui->fsize_scale, sp_explorer_cb, eui);
    ui->lwidth_scale = CreateSpinChoice(rc, "Line width:", 6,
        SPIN_TYPE_FLOAT, 0.0, 1.0, 0.0005);
    AddSpinChoiceCB(ui->lwidth_scale, sp_explorer_cb, eui);

    fr = CreateFrame(form, "Data & Dates");
    rc1 = CreateVContainer(fr);
    ui->prec = CreateSpinChoice(rc1, "Data precision:", 3,
        SPIN_TYPE_INT, DATA_PREC_MIN, DATA_PREC_MAX, 1);
    AddSpinChoiceCB(ui->prec, sp_explorer_cb, eui);
    ui->refdate = CreateTextItem(rc1, 20, "Reference date:");
    AddTextItemCB(ui->refdate, titem_explorer_cb, eui);
    rc = CreateHContainer(rc1);
    ui->two_digits_years = CreateToggleButton(rc, "Two-digit year span");
    AddToggleButtonCB(ui->two_digits_years, tb_explorer_cb, eui);
    ui->wrap_year = CreateTextItem(rc, 4, "Wrap year:");
    AddTextItemCB(ui->wrap_year, titem_explorer_cb, eui);
    AddToggleButtonCB(ui->two_digits_years, wrap_year_cb, ui->wrap_year);
    
    ui->top = form;
    
    return ui;
}

void update_project_ui(ProjectUI *ui, Quark *q)
{
    Project *pr = project_get_data(q);
    if (pr) {
        int y, m, d, h, mm, sec;
        char date_string[64], wrap_year_string[64], buf[32];
        double factor;
        int format;

        SetSpinChoice(ui->prec, project_get_prec(q));
        SetTextString(ui->description, project_get_description(q));

        switch (GetOptionChoice(ui->page_size_unit)) {
        case PAGE_UNITS_IN:
            factor = 1.0/72.0;
            break;
        case PAGE_UNITS_CM:
            factor = CM_PER_INCH/72.0;
            break;
        default:
            factor = 1.0;
        }

        sprintf (buf, "%.2f", factor*pr->page_wpp); 
        xv_setstr(ui->page_x, buf);
        sprintf (buf, "%.2f", factor*pr->page_hpp); 
        xv_setstr(ui->page_y, buf);

        if ((pr->page_wpp == 612 && pr->page_hpp == 792) ||
            (pr->page_hpp == 612 && pr->page_wpp == 792)) {
            format = PAGE_FORMAT_USLETTER;
        } else
        if ((pr->page_wpp == 595 && pr->page_hpp == 842) ||
            (pr->page_hpp == 595 && pr->page_wpp == 842)) {
            format = PAGE_FORMAT_A4;
        } else {
            format = PAGE_FORMAT_CUSTOM;
        }
        if (format == PAGE_FORMAT_CUSTOM) {
            SetSensitive(ui->page_x, TRUE);
            SetSensitive(ui->page_y, TRUE);
            SetSensitive(ui->page_orient->menu, FALSE);
        } else {
            SetSensitive(ui->page_x, FALSE);
            SetSensitive(ui->page_y, FALSE);
            SetSensitive(ui->page_orient->menu, TRUE);
        }
        SetOptionChoice(ui->page_format, format);
        
        if (pr->page_wpp > pr->page_hpp) {
            SetOptionChoice(ui->page_orient, PAGE_ORIENT_LANDSCAPE);
        } else {
            SetOptionChoice(ui->page_orient, PAGE_ORIENT_PORTRAIT);
        }

        SetOptionChoice(ui->bg_color, pr->bgcolor);
        SetToggleButtonState(ui->bg_fill, pr->bgfill);

        SetSpinChoice(ui->fsize_scale, pr->fscale);
        SetSpinChoice(ui->lwidth_scale, pr->lscale);

	    jdate_to_datetime(q, 0.0, ROUND_SECOND, &y, &m, &d, &h, &mm, &sec);
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
        GraceApp *gapp = gapp_from_quark(q);
        double jul;
    
        if (!caller || caller == ui->prec) {
            project_set_prec(q, GetSpinChoice(ui->prec));
        }
        if (!caller || caller == ui->description) {
            char *s = GetTextString(ui->description);
            project_set_description(q, s);
            xfree(s);
        }

        if (caller == ui->page_orient) {
            int wpp, hpp;
            int orientation = GetOptionChoice(ui->page_orient);
            project_get_page_dimensions(q, &wpp, &hpp);
            if ((orientation == PAGE_ORIENT_LANDSCAPE && wpp < hpp) ||
                (orientation == PAGE_ORIENT_PORTRAIT  && wpp > hpp)) {
                set_page_dimensions(gapp, hpp, wpp, TRUE);
            }
        }
        if (caller == ui->page_format) {
            int wpp, hpp;
            int orientation = GetOptionChoice(ui->page_orient);
            int format = GetOptionChoice(ui->page_format);
            GraceApp *gapp = gapp_from_quark(q);

            switch (format) {
            case PAGE_FORMAT_USLETTER:
                wpp = 792.0;
                hpp = 612.0;
                break;
            case PAGE_FORMAT_A4:
                wpp = 842.0;
                hpp = 595.0;
                break;
            default:
                return RETURN_SUCCESS;
            }
            
            if (orientation == PAGE_ORIENT_PORTRAIT) {
                iswap(&wpp, &hpp);
            }

            set_page_dimensions(gapp, wpp, hpp, TRUE);
        }
        
        if (!caller || caller == ui->page_x || caller == ui->page_y) {
            int page_units = GetOptionChoice(ui->page_size_unit);
            double factor, page_x, page_y;
            GraceApp *gapp = gapp_from_quark(q);

            if (xv_evalexpr(ui->page_x, &page_x) != RETURN_SUCCESS ||
                xv_evalexpr(ui->page_y, &page_y) != RETURN_SUCCESS) {
                errmsg("Invalid page dimension(s)");
                return RETURN_FAILURE;
            }

            switch (page_units) {
            case PAGE_UNITS_IN:
                factor = 72.0;
                break;
            case PAGE_UNITS_CM:
                factor = 72.0/CM_PER_INCH;
                break;
            default:
                factor = 1.0;
                break;
            }

            page_x *= factor;
            page_y *= factor;
            
            set_page_dimensions(gapp, (int) rint(page_x), (int) rint(page_y),
                TRUE);
        }

        if (!caller || caller == ui->bg_color) {
            pr->bgcolor = GetOptionChoice(ui->bg_color);
        }
        if (!caller || caller == ui->bg_fill) {
            pr->bgfill = GetToggleButtonState(ui->bg_fill);
        }

        if (!caller || caller == ui->fsize_scale) {
            pr->fscale = GetSpinChoice(ui->fsize_scale);
        }
        if (!caller || caller == ui->lwidth_scale) {
            pr->lscale = GetSpinChoice(ui->lwidth_scale);
        }

        if (!caller || caller == ui->refdate) {
            if (parse_date_or_number(q, xv_getstr(ui->refdate),
                TRUE, get_date_hint(gapp), &jul) ==
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
