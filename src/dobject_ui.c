/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2001-2003 Grace Development Team
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

/*
 *
 * UI for editing drawing object properties
 *
 */

#include <config.h>

#include "explorer.h"
#include "objutils.h"

static void update_line_ui(LineUI *ui, DOLineData *odata);
static void set_line_odata(LineUI *ui, DOLineData *odata, void *caller);
static LineUI *create_line_ui(Widget parent, ExplorerUI *eui);
static void update_box_ui(BoxUI *ui, DOBoxData *odata);
static void set_box_odata(BoxUI *ui, DOBoxData *odata, void *caller);
static BoxUI *create_box_ui(Widget parent, ExplorerUI *eui);
static void update_arc_ui(ArcUI *ui, DOArcData *odata);
static void set_arc_odata(ArcUI *ui, DOArcData *odata, void *caller);
static ArcUI *create_arc_ui(Widget parent, ExplorerUI *eui);
static void update_string_ui(StringUI *ui, DOStringData *odata);
static void set_string_odata(StringUI *ui, DOStringData *odata, void *caller);
static StringUI *create_string_ui(Widget parent, ExplorerUI *eui);

SpinStructure *CreateViewCoordInput(Widget parent, char *s)
{
    return CreateSpinChoice(parent, s, 6, SPIN_TYPE_FLOAT, -10.0, 10.0, 0.05);
}


ObjectUI *create_object_ui(ExplorerUI *eui)
{    
    ObjectUI *ui;
    Widget tab, fr, rc, rc1;

    ui = xmalloc(sizeof(ObjectUI));

    /* ------------ Tabs -------------- */
    tab = CreateTab(eui->scrolled_window);        

    /* ------------ Main tab -------------- */
    ui->main_tp = CreateTabPage(tab, "General");

    ui->active = CreateToggleButton(ui->main_tp, "Active");
    AddToggleButtonCB(ui->active, tb_explorer_cb, eui);

    fr = CreateFrame(ui->main_tp, "Anchor point");
    rc = CreateHContainer(fr);
    ui->x = CreateTextInput(rc, "X:");
    XtVaSetValues(ui->x->text, XmNcolumns, 10, NULL);
    AddTextInputCB(ui->x, text_explorer_cb, eui);
    ui->y = CreateTextInput(rc, "Y:");
    XtVaSetValues(ui->y->text, XmNcolumns, 10, NULL);
    AddTextInputCB(ui->y, text_explorer_cb, eui);

    fr = CreateFrame(ui->main_tp, "Placement");
    rc = CreateVContainer(fr);
    rc1 = CreateHContainer(rc);
    ui->offsetx = CreateViewCoordInput(rc1, "dX:");
    AddSpinButtonCB(ui->offsetx, sp_explorer_cb, eui);
    ui->offsety = CreateViewCoordInput(rc1, "dY:");
    AddSpinButtonCB(ui->offsety, sp_explorer_cb, eui);
    ui->angle = CreateAngleChoice(rc, "Angle");
    AddScaleCB(ui->angle, scale_explorer_cb, eui);

    fr = CreateFrame(ui->main_tp, "Drawing properties");
    rc = CreateVContainer(fr);
    rc1 = CreateHContainer(rc);
    ui->linew = CreateLineWidthChoice(rc1, "Line width:");
    AddSpinButtonCB(ui->linew, sp_explorer_cb, eui);
    ui->lines = CreateLineStyleChoice(rc1, "Line style:");
    AddOptionChoiceCB(ui->lines, oc_explorer_cb, eui);
    rc1 = CreateHContainer(rc);
    ui->linepen = CreatePenChoice(rc1, "Outline pen:");
    ui->fillpen = CreatePenChoice(rc1, "Fill pen:");

    /* ------------ Object data tab -------------- */
    ui->odata_tp = CreateTabPage(tab, "Object data");

    ui->line_ui = create_line_ui(ui->odata_tp, eui);
    UnmanageChild(ui->line_ui->top);
    ui->box_ui = create_box_ui(ui->odata_tp, eui);
    UnmanageChild(ui->box_ui->top);
    ui->arc_ui = create_arc_ui(ui->odata_tp, eui);
    UnmanageChild(ui->arc_ui->top);
    ui->string_ui = create_string_ui(ui->odata_tp, eui);
    UnmanageChild(ui->string_ui->top);

    SelectTabPage(tab, ui->main_tp);
    
    ui->top = tab;
    
    return ui;
}

static void update_line_ui(LineUI *ui, DOLineData *odata)
{
    SetSpinChoice(ui->length, odata->length);
    SetOptionChoice(ui->arrow_end, odata->arrow_end);

    SetOptionChoice(ui->a_type, odata->arrow.type);
    SetSpinChoice(ui->a_length, odata->arrow.length);
    SetSpinChoice(ui->a_dL_ff, odata->arrow.dL_ff);
    SetSpinChoice(ui->a_lL_ff, odata->arrow.lL_ff);
}

static void set_line_odata(LineUI *ui, DOLineData *odata, void *caller)
{
    if (ui && odata && IsManaged(ui->top)) {
        if (!caller || caller == ui->length) {
            odata->length    = GetSpinChoice(ui->length);
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
    fr = CreateFrame(ui->top, "Line properties");
    rc = CreateVContainer(fr);
    ui->length = CreateSpinChoice(rc, "Length:",
        8, SPIN_TYPE_FLOAT, 0, 10.0, 0.05);
    AddSpinButtonCB(ui->length, sp_explorer_cb, eui);
    ui->arrow_end = CreatePanelChoice(rc, "Place arrows at:",
				      5,
				      "None",
				      "Start",
				      "End",
				      "Both ends",
				      0,
				      0);
    AddOptionChoiceCB(ui->arrow_end, oc_explorer_cb, eui);

    fr = CreateFrame(ui->top, "Arrows");
    rc = CreateVContainer(fr);
    rc1 = CreateHContainer(rc);
    ui->a_type = CreatePanelChoice(rc1, "Type:",
				   3,
				   "Line",
				   "Filled",
				   0,
				   0);
    AddOptionChoiceCB(ui->a_type, oc_explorer_cb, eui);
    ui->a_length = CreateSpinChoice(rc1, "Length:",
        4, SPIN_TYPE_FLOAT, -10.0, 10.0, 0.5);
    AddSpinButtonCB(ui->a_length, sp_explorer_cb, eui);
    rc1 = CreateHContainer(rc);
    ui->a_dL_ff = CreateSpinChoice(rc1, "d/L FF:",
        4, SPIN_TYPE_FLOAT, 0.0, 10.0, 0.1);
    AddSpinButtonCB(ui->a_dL_ff, sp_explorer_cb, eui);
    ui->a_lL_ff = CreateSpinChoice(rc1, "l/L FF:",
        4, SPIN_TYPE_FLOAT, -1.0, 1.0, 0.1);
    AddSpinButtonCB(ui->a_lL_ff, sp_explorer_cb, eui);
    
    return ui;
}

static void update_box_ui(BoxUI *ui, DOBoxData *odata)
{
    SetSpinChoice(ui->width,  odata->width);
    SetSpinChoice(ui->height, odata->height);
}

static void set_box_odata(BoxUI *ui, DOBoxData *odata, void *caller)
{
    if (ui && odata && IsManaged(ui->top)) {
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
    AddSpinButtonCB(ui->width, sp_explorer_cb, eui);
    ui->height = CreateSpinChoice(rc, "Height:",
        8, SPIN_TYPE_FLOAT, 0, 10.0, 0.05);
    AddSpinButtonCB(ui->height, sp_explorer_cb, eui);
    
    return ui;
}

static void update_arc_ui(ArcUI *ui, DOArcData *odata)
{
    SetSpinChoice(ui->width,  odata->width);
    SetSpinChoice(ui->height, odata->height);
    
    SetAngleChoice(ui->angle1, (int) rint(odata->angle1));
    SetAngleChoice(ui->angle2, (int) rint(odata->angle2));
    
    SetOptionChoice(ui->fillmode, odata->fillmode);
}

static void set_arc_odata(ArcUI *ui, DOArcData *odata, void *caller)
{
    if (ui && odata && IsManaged(ui->top)) {
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
        if (!caller || caller == ui->fillmode) {
            odata->fillmode = GetOptionChoice(ui->fillmode);
        }
    }
}

static ArcUI *create_arc_ui(Widget parent, ExplorerUI *eui)
{
    ArcUI *ui;
    Widget rc;
    OptionItem opitems[] = {
        {ARCFILL_CHORD,  "Cord"       },
        {ARCFILL_PIESLICE, "Pie slice"}
    };
    
    ui = xmalloc(sizeof(ArcUI));
    
    ui->top = CreateFrame(parent, "Arc properties");
    rc = CreateVContainer(ui->top);

    ui->width = CreateSpinChoice(rc, "Width: ",
        8, SPIN_TYPE_FLOAT, 0, 10.0, 0.05);
    AddSpinButtonCB(ui->width, sp_explorer_cb, eui);
    ui->height = CreateSpinChoice(rc, "Height:",
        8, SPIN_TYPE_FLOAT, 0, 10.0, 0.05);
    AddSpinButtonCB(ui->height, sp_explorer_cb, eui);
    
    ui->angle1 = CreateAngleChoice(rc, "Start angle");
    AddScaleCB(ui->angle1, scale_explorer_cb, eui);
    ui->angle2 = CreateAngleChoice(rc, "Extent angle");
    AddScaleCB(ui->angle2, scale_explorer_cb, eui);
    
    ui->fillmode = CreateOptionChoice(rc, "Fill mode:", 1, 2, opitems);
    AddOptionChoiceCB(ui->fillmode, oc_explorer_cb, eui);
    
    return ui;
}



static void update_string_ui(StringUI *ui, DOStringData *odata)
{
    SetTextString(ui->text,     odata->s);
    SetOptionChoice(ui->font,   odata->font);
    SetOptionChoice(ui->just,   odata->just);
    SetCharSizeChoice(ui->size, odata->size);

    SetSpinChoice(ui->linew,   odata->line.width);
    SetOptionChoice(ui->lines, odata->line.style);
    SetPenChoice(ui->linepen, &odata->line.pen);
    SetPenChoice(ui->fillpen, &odata->fillpen);
}

static void set_string_odata(StringUI *ui, DOStringData *odata, void *caller)
{
    if (ui && odata && IsManaged(ui->top)) {

        if (!caller || caller == ui->text) {
            xfree(odata->s);
            odata->s    = GetTextString(ui->text);
        }
        if (!caller || caller == ui->font) {
            odata->font = GetOptionChoice(ui->font);
        }
        if (!caller || caller == ui->just) {
            odata->just = GetOptionChoice(ui->just);
        }
        if (!caller || caller == ui->size) {
            odata->size = GetCharSizeChoice(ui->size);
        }
        if (!caller || caller == ui->linew) {
            odata->line.width = GetSpinChoice(ui->linew);
        }
        if (!caller || caller == ui->lines) {
            odata->line.style = GetOptionChoice(ui->lines);
        }
        if (!caller || caller == ui->linepen) {
            GetPenChoice(ui->linepen, &odata->line.pen);
        }
        if (!caller || caller == ui->fillpen) {
            GetPenChoice(ui->fillpen, &odata->fillpen);
        }
    }
}

static StringUI *create_string_ui(Widget parent, ExplorerUI *eui)
{
    StringUI *ui;
    Widget fr, rc, rc1;
    
    ui = xmalloc(sizeof(StringUI));
    
    ui->top = CreateVContainer(parent);
    fr = CreateFrame(ui->top, "String properties");
    rc = CreateVContainer(fr);
    
    ui->text = CreateCSText(rc, "Text:");
    AddTextInputCB(ui->text, text_explorer_cb, eui);
    ui->font = CreateFontChoice(rc, "Font:");
    AddOptionChoiceCB(ui->font, oc_explorer_cb, eui);
    ui->just = CreateJustChoice(rc, "Justification:");
    AddOptionChoiceCB(ui->just, oc_explorer_cb, eui);
    ui->size = CreateCharSizeChoice(rc, "Size");
    AddScaleCB(ui->size, scale_explorer_cb, eui);

    fr = CreateFrame(ui->top, "Frame");
    rc = CreateVContainer(fr);
    rc1 = CreateHContainer(rc);
    ui->linew = CreateLineWidthChoice(rc1, "Width:");
    AddSpinButtonCB(ui->linew, sp_explorer_cb, eui);
    ui->lines = CreateLineStyleChoice(rc1, "Style:");
    AddOptionChoiceCB(ui->lines, oc_explorer_cb, eui);
    rc1 = CreateHContainer(rc);
    ui->linepen = CreatePenChoice(rc1, "Outline pen:");
    ui->fillpen = CreatePenChoice(rc1, "Fill pen:");
    
    return ui;
}


void update_object_ui(ObjectUI *ui, Quark *q)
{
    DObject *o = object_get_data(q);
    if (o && ui) {
        char *format, buf[32];
        
        SetToggleButtonState(ui->active, o->active);
        
        if (object_get_loctype(q) == COORD_WORLD) {
            format = "%.8g";
        } else {
            format = "%.4f";
        }
        sprintf(buf, format, o->ap.x);
        SetTextString(ui->x, buf);
        sprintf(buf, format, o->ap.y);
        SetTextString(ui->y, buf);
        
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
            
            ManageChild(ui->line_ui->top);
            UnmanageChild(ui->box_ui->top);
            UnmanageChild(ui->arc_ui->top);
            UnmanageChild(ui->string_ui->top);
            break;
        case DO_BOX:
            update_box_ui(ui->box_ui, (DOBoxData *) o->odata);
            
            UnmanageChild(ui->line_ui->top);
            ManageChild(ui->box_ui->top);
            UnmanageChild(ui->arc_ui->top);
            UnmanageChild(ui->string_ui->top);
            break;
        case DO_ARC:
            update_arc_ui(ui->arc_ui, (DOArcData *) o->odata);
            
            UnmanageChild(ui->line_ui->top);
            UnmanageChild(ui->box_ui->top);
            ManageChild(ui->arc_ui->top);
            UnmanageChild(ui->string_ui->top);
            break;
        case DO_STRING:
            update_string_ui(ui->string_ui, (DOStringData *) o->odata);
            
            UnmanageChild(ui->line_ui->top);
            UnmanageChild(ui->box_ui->top);
            UnmanageChild(ui->arc_ui->top);
            ManageChild(ui->string_ui->top);
            break;
        default:
            UnmanageChild(ui->line_ui->top);
            UnmanageChild(ui->box_ui->top);
            UnmanageChild(ui->arc_ui->top);
            UnmanageChild(ui->string_ui->top);
            break;
        }
    }
}

int set_object_data(ObjectUI *ui, Quark *q, void *caller)
{
    DObject *o = object_get_data(q);
    
    if (o && ui) {
        if (!caller || caller == ui->active) {
            o->active = GetToggleButtonState(ui->active);
        }
        if (!caller || caller == ui->x) {
            xv_evalexpr(ui->x->text, &o->ap.x);
        }
        if (!caller || caller == ui->y) {
            xv_evalexpr(ui->y->text, &o->ap.y);
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
        case DO_STRING:
            set_string_odata(ui->string_ui, (DOStringData *) o->odata, caller);
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
