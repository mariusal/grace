/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2001-2004 Grace Development Team
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
 * UI for editing drawing object properties
 *
 */

#include <config.h>

#include "explorer.h"

static void update_line_ui(LineUI *ui, DOLineData *odata);
static void set_line_odata(LineUI *ui, DOLineData *odata, void *caller);
static LineUI *create_line_ui(Widget parent, ExplorerUI *eui);
static void update_box_ui(BoxUI *ui, DOBoxData *odata);
static void set_box_odata(BoxUI *ui, DOBoxData *odata, void *caller);
static BoxUI *create_box_ui(Widget parent, ExplorerUI *eui);
static void update_arc_ui(ArcUI *ui, DOArcData *odata);
static void set_arc_odata(ArcUI *ui, DOArcData *odata, void *caller);
static ArcUI *create_arc_ui(Widget parent, ExplorerUI *eui);

ObjectUI *create_object_ui(ExplorerUI *eui)
{    
    ObjectUI *ui;
    Widget tab, fr, rc, rc1;

    ui = xmalloc(sizeof(ObjectUI));

    /* ------------ Tabs -------------- */
    tab = CreateTab(eui->scrolled_window);        
    AddHelpCB(tab, "doc/UsersGuide.html#dobject-properties");

    /* ------------ Main tab -------------- */
    ui->main_tp = CreateTabPage(tab, "General");

    fr = CreateFrame(ui->main_tp, "Anchor point");
    rc = CreateHContainer(fr);
    ui->x = CreateText2(rc, "X:", 10);
    AddTextActivateCB(ui->x, text_explorer_cb, eui);
    ui->y = CreateText2(rc, "Y:", 10);
    AddTextActivateCB(ui->y, text_explorer_cb, eui);

    fr = CreateFrame(ui->main_tp, "Placement");
    rc = CreateVContainer(fr);
    rc1 = CreateHContainer(rc);
    ui->offsetx = CreateViewCoordInput(rc1, "dX:");
    AddSpinChoiceCB(ui->offsetx, sp_explorer_cb, eui);
    ui->offsety = CreateViewCoordInput(rc1, "dY:");
    AddSpinChoiceCB(ui->offsety, sp_explorer_cb, eui);
    ui->angle = CreateAngleChoice(rc, "Angle:");
    AddSpinChoiceCB(ui->angle, sp_explorer_cb, eui);

    fr = CreateFrame(ui->main_tp, "Drawing properties");
    rc = CreateVContainer(fr);
    rc1 = CreateHContainer(rc);
    ui->linew = CreateLineWidthChoice(rc1, "Line width:");
    AddSpinChoiceCB(ui->linew, sp_explorer_cb, eui);
    ui->lines = CreateLineStyleChoice(rc1, "Line style:");
    AddOptionChoiceCB(ui->lines, oc_explorer_cb, eui);
    rc1 = CreateHContainer(rc);
    ui->linepen = CreatePenChoice(rc1, "Outline pen:");
    AddPenChoiceCB(ui->linepen, pen_explorer_cb, eui);
    ui->fillpen = CreatePenChoice(rc1, "Fill pen:");
    AddPenChoiceCB(ui->fillpen, pen_explorer_cb, eui);

    /* ------------ Object data tab -------------- */
    ui->odata_tp = CreateTabPage(tab, "Object data");

    ui->line_ui = create_line_ui(ui->odata_tp, eui);
    WidgetUnmanage(ui->line_ui->top);
    ui->box_ui = create_box_ui(ui->odata_tp, eui);
    WidgetUnmanage(ui->box_ui->top);
    ui->arc_ui = create_arc_ui(ui->odata_tp, eui);
    WidgetUnmanage(ui->arc_ui->top);

    SelectTabPage(tab, ui->main_tp);
    
    ui->top = tab;
    
    return ui;
}

static void update_line_ui(LineUI *ui, DOLineData *odata)
{
    SetSpinChoice(ui->v_x, odata->vector.x);
    SetSpinChoice(ui->v_y, odata->vector.y);
    SetOptionChoice(ui->arrow_end, odata->arrow_end);

    SetOptionChoice(ui->a_type, odata->arrow.type);
    SetSpinChoice(ui->a_length, odata->arrow.length);
    SetSpinChoice(ui->a_dL_ff, odata->arrow.dL_ff);
    SetSpinChoice(ui->a_lL_ff, odata->arrow.lL_ff);
}

static void set_line_odata(LineUI *ui, DOLineData *odata, void *caller)
{
    if (ui && odata && WidgetIsManaged(ui->top)) {
        if (!caller || caller == ui->v_x) {
            odata->vector.x = GetSpinChoice(ui->v_x);
        }
        if (!caller || caller == ui->v_y) {
            odata->vector.y  = GetSpinChoice(ui->v_y);
        }
        if (!caller || caller == ui->arrow_end) {
            odata->arrow_end = GetOptionChoice(ui->arrow_end);
        }
        if (!caller || caller == ui->a_type) {
            odata->arrow.type   = GetOptionChoice(ui->a_type);
        }
        if (!caller || caller == ui->a_length) {
            odata->arrow.length = GetSpinChoice(ui->a_length);
        }
        if (!caller || caller == ui->a_dL_ff) {
            odata->arrow.dL_ff  = GetSpinChoice(ui->a_dL_ff);
        }
        if (!caller || caller == ui->a_lL_ff) {
            odata->arrow.lL_ff  = GetSpinChoice(ui->a_lL_ff);
        }
    }
}

static LineUI *create_line_ui(Widget parent, ExplorerUI *eui)
{
    LineUI *ui;
    Widget fr, rc, rc1;
    
    ui = xmalloc(sizeof(LineUI));
    
    ui->top = CreateVContainer(parent);
    fr = CreateFrame(ui->top, "Vector");
    rc = CreateHContainer(fr);
    ui->v_x = CreateSpinChoice(rc, "X: ",
        8, SPIN_TYPE_FLOAT, -10.0, 10.0, 0.05);
    AddSpinChoiceCB(ui->v_x, sp_explorer_cb, eui);
    ui->v_y = CreateSpinChoice(rc, "Y:",
        8, SPIN_TYPE_FLOAT, -10.0, 10.0, 0.05);
    AddSpinChoiceCB(ui->v_y, sp_explorer_cb, eui);

    fr = CreateFrame(ui->top, "Arrows");
    rc = CreateVContainer(fr);
    ui->arrow_end = CreatePanelChoice(rc, "Place at:",
				      "None",
				      "Start",
				      "End",
				      "Both ends",
				      NULL);
    AddOptionChoiceCB(ui->arrow_end, oc_explorer_cb, eui);

    rc1 = CreateHContainer(rc);
    ui->a_type = CreateOptionChoiceVA(rc1, "Type:",
        "Line",   ARROW_TYPE_LINE,
        "Filled", ARROW_TYPE_FILLED,
        "Circle", ARROW_TYPE_CIRCLE,
        NULL);
    AddOptionChoiceCB(ui->a_type, oc_explorer_cb, eui);
    ui->a_length = CreateSpinChoice(rc1, "Length:",
        4, SPIN_TYPE_FLOAT, -10.0, 10.0, 0.5);
    AddSpinChoiceCB(ui->a_length, sp_explorer_cb, eui);
    rc1 = CreateHContainer(rc);
    ui->a_dL_ff = CreateSpinChoice(rc1, "d/L FF:",
        4, SPIN_TYPE_FLOAT, 0.0, 10.0, 0.1);
    AddSpinChoiceCB(ui->a_dL_ff, sp_explorer_cb, eui);
    ui->a_lL_ff = CreateSpinChoice(rc1, "l/L FF:",
        4, SPIN_TYPE_FLOAT, -1.0, 1.0, 0.1);
    AddSpinChoiceCB(ui->a_lL_ff, sp_explorer_cb, eui);
    
    return ui;
}

static void update_box_ui(BoxUI *ui, DOBoxData *odata)
{
    SetSpinChoice(ui->width,  odata->width);
    SetSpinChoice(ui->height, odata->height);
}

static void set_box_odata(BoxUI *ui, DOBoxData *odata, void *caller)
{
    if (ui && odata && WidgetIsManaged(ui->top)) {
        if (!caller || caller == ui->width) {
            odata->width  = GetSpinChoice(ui->width);
        }
        if (!caller || caller == ui->height) {
            odata->height = GetSpinChoice(ui->height);
        }
    }
}

static BoxUI *create_box_ui(Widget parent, ExplorerUI *eui)
{
    BoxUI *ui;
    Widget rc;
    
    ui = xmalloc(sizeof(BoxUI));
    
    ui->top = CreateFrame(parent, "Box properties");
    rc = CreateVContainer(ui->top);
    ui->width = CreateSpinChoice(rc, "Width: ",
        8, SPIN_TYPE_FLOAT, 0, 10.0, 0.05);
    AddSpinChoiceCB(ui->width, sp_explorer_cb, eui);
    ui->height = CreateSpinChoice(rc, "Height:",
        8, SPIN_TYPE_FLOAT, 0, 10.0, 0.05);
    AddSpinChoiceCB(ui->height, sp_explorer_cb, eui);
    
    return ui;
}

static void update_arc_ui(ArcUI *ui, DOArcData *odata)
{
    SetSpinChoice(ui->width,  odata->width);
    SetSpinChoice(ui->height, odata->height);
    
    SetAngleChoice(ui->angle1, (int) rint(odata->angle1));
    SetAngleChoice(ui->angle2, (int) rint(odata->angle2));
    
    SetOptionChoice(ui->closure_type, odata->closure_type);
    SetToggleButtonState(ui->draw_closure, odata->draw_closure);
}

static void set_arc_odata(ArcUI *ui, DOArcData *odata, void *caller)
{
    if (ui && odata && WidgetIsManaged(ui->top)) {
        if (!caller || caller == ui->width) {
            odata->width  = GetSpinChoice(ui->width);
        }
        if (!caller || caller == ui->height) {
            odata->height = GetSpinChoice(ui->height);
        }
        if (!caller || caller == ui->angle1) {
            odata->angle1 = GetAngleChoice(ui->angle1);
        }
        if (!caller || caller == ui->angle2) {
            odata->angle2 = GetAngleChoice(ui->angle2);
        }
        if (!caller || caller == ui->closure_type) {
            odata->closure_type = GetOptionChoice(ui->closure_type);
        }
        if (!caller || caller == ui->draw_closure) {
            odata->draw_closure = GetToggleButtonState(ui->draw_closure);
        }
    }
}

static ArcUI *create_arc_ui(Widget parent, ExplorerUI *eui)
{
    ArcUI *ui;
    Widget rc, rc1;
    OptionItem opitems[] = {
        {ARCCLOSURE_CHORD,    "Chord"    },
        {ARCCLOSURE_PIESLICE, "Pie slice"}
    };
    
    ui = xmalloc(sizeof(ArcUI));
    
    ui->top = CreateFrame(parent, "Arc properties");
    rc = CreateVContainer(ui->top);

    ui->width = CreateSpinChoice(rc, "Width: ",
        8, SPIN_TYPE_FLOAT, 0, 10.0, 0.05);
    AddSpinChoiceCB(ui->width, sp_explorer_cb, eui);
    ui->height = CreateSpinChoice(rc, "Height:",
        8, SPIN_TYPE_FLOAT, 0, 10.0, 0.05);
    AddSpinChoiceCB(ui->height, sp_explorer_cb, eui);
    
    ui->angle1 = CreateAngleChoice(rc, "Start angle:");
    AddSpinChoiceCB(ui->angle1, sp_explorer_cb, eui);
    ui->angle2 = CreateAngleChoice(rc, "Extent angle:");
    AddSpinChoiceCB(ui->angle2, sp_explorer_cb, eui);
    
    rc1 = CreateHContainer(rc);
    ui->closure_type = CreateOptionChoice(rc1, "Closure type:", 1, 2, opitems);
    AddOptionChoiceCB(ui->closure_type, oc_explorer_cb, eui);
    ui->draw_closure = CreateToggleButton(rc1, "Draw closure");
    AddToggleButtonCB(ui->draw_closure, tb_explorer_cb, eui);
    
    return ui;
}

void update_object_ui(ObjectUI *ui, Quark *q)
{
    DObject *o = object_get_data(q);
    if (o && ui) {
        char *format, buf[32];
        
        if (object_get_loctype(q) == COORD_WORLD) {
            format = "%.8g";
        } else {
            format = "%.4f";
        }
        sprintf(buf, format, o->ap.x);
        TextSetString(ui->x, buf);
        sprintf(buf, format, o->ap.y);
        TextSetString(ui->y, buf);
        
        SetSpinChoice(ui->offsetx, o->offset.x);
        SetSpinChoice(ui->offsety, o->offset.y);
        SetAngleChoice(ui->angle, (int) rint(o->angle));
        
        SetSpinChoice(ui->linew, o->line.width);
        SetOptionChoice(ui->lines, o->line.style);
        SetPenChoice(ui->linepen, &o->line.pen);
        SetPenChoice(ui->fillpen, &o->fillpen);
        
        switch (o->type) {
        case DO_LINE:
            update_line_ui(ui->line_ui, (DOLineData *) o->odata);
            
            WidgetManage(ui->line_ui->top);
            WidgetUnmanage(ui->box_ui->top);
            WidgetUnmanage(ui->arc_ui->top);
            break;
        case DO_BOX:
            update_box_ui(ui->box_ui, (DOBoxData *) o->odata);
            
            WidgetUnmanage(ui->line_ui->top);
            WidgetManage(ui->box_ui->top);
            WidgetUnmanage(ui->arc_ui->top);
            break;
        case DO_ARC:
            update_arc_ui(ui->arc_ui, (DOArcData *) o->odata);
            
            WidgetUnmanage(ui->line_ui->top);
            WidgetUnmanage(ui->box_ui->top);
            WidgetManage(ui->arc_ui->top);
            break;
        default:
            WidgetUnmanage(ui->line_ui->top);
            WidgetUnmanage(ui->box_ui->top);
            WidgetUnmanage(ui->arc_ui->top);
            break;
        }
    }
}

int set_object_data(ObjectUI *ui, Quark *q, void *caller)
{
    DObject *o = object_get_data(q);
    
    if (o && ui) {
        if (!caller || caller == ui->x) {
            xv_evalexpr(ui->x, &o->ap.x);
        }
        if (!caller || caller == ui->y) {
            xv_evalexpr(ui->y, &o->ap.y);
        }
        if (!caller || caller == ui->offsetx) {
            o->offset.x = GetSpinChoice(ui->offsetx);
        }
        if (!caller || caller == ui->offsety) {
            o->offset.y = GetSpinChoice(ui->offsety);
        }
        if (!caller || caller == ui->angle) {
            o->angle = GetAngleChoice(ui->angle);
        }
        if (!caller || caller == ui->linew) {
            o->line.width = GetSpinChoice(ui->linew);
        }
        if (!caller || caller == ui->lines) {
            o->line.style = GetOptionChoice(ui->lines);
        }
        if (!caller || caller == ui->linepen) {
            GetPenChoice(ui->linepen, &o->line.pen);
        }
        if (!caller || caller == ui->fillpen) {
            GetPenChoice(ui->fillpen, &o->fillpen);
        }

        switch (o->type) {
        case DO_LINE:
            set_line_odata(ui->line_ui, (DOLineData *) o->odata, caller);
            break;
        case DO_BOX:
            set_box_odata(ui->box_ui, (DOBoxData *) o->odata, caller);
            break;
        case DO_ARC:
            set_arc_odata(ui->arc_ui, (DOArcData *) o->odata, caller);
            break;
        default:
            break;
        }

        quark_dirtystate_set(q, TRUE);

        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}
