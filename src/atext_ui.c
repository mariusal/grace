/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2001-2004 Grace Development Team
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
 * UI for editing atext properties
 *
 */

#include <config.h>

#include "explorer.h"


ATextUI *create_atext_ui(ExplorerUI *eui)
{
    ATextUI *ui;
    Widget fr, rc, rc1;

    OptionItem opitems[4] = {
        {FRAME_DECOR_NONE, "None"     },
        {FRAME_DECOR_LINE, "Underline"},
        {FRAME_DECOR_RECT, "Rectangle"},
        {FRAME_DECOR_OVAL, "Oval"     }
    };

    ui = xmalloc(sizeof(ATextUI));
    
    ui->top = CreateTab(eui->scrolled_window);

    /* ------------ Main tab -------------- */
    ui->main_tp = CreateTabPage(ui->top, "Main");

    fr = CreateFrame(ui->main_tp, "Text properties");
    rc = CreateVContainer(fr);
    
    ui->text = CreateCSText(rc, "Text:");
    AddTextInputCB(ui->text, text_explorer_cb, eui);
    ui->font = CreateFontChoice(rc, "Font:");
    AddOptionChoiceCB(ui->font, oc_explorer_cb, eui);
    rc1 = CreateHContainer(rc);
    ui->color = CreateColorChoice(rc1, "Color:");
    AddOptionChoiceCB(ui->color, oc_explorer_cb, eui);
    ui->size = CreateCharSizeChoice(rc1, "Size:");
    AddSpinChoiceCB(ui->size, sp_explorer_cb, eui);
    ui->just = CreateJustChoice(rc, "Justification:");
    AddOptionChoiceCB(ui->just, oc_explorer_cb, eui);

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
    AddSpinChoiceCB(ui->offsetx, sp_explorer_cb, eui);
    ui->offsety = CreateViewCoordInput(rc1, "dY:");
    AddSpinChoiceCB(ui->offsety, sp_explorer_cb, eui);
    ui->angle = CreateAngleChoice(rc, "Angle");
    AddScaleCB(ui->angle, scale_explorer_cb, eui);

    /* ------------ Frame tab -------------- */
    ui->frame_tp = CreateTabPage(ui->top, "Frame & Pointer");

    fr = CreateFrame(ui->frame_tp, "Frame");
    rc = CreateVContainer(fr);
    rc1 = CreateHContainer(rc);
    ui->frame_decor = CreateOptionChoice(rc1, "Type:", 0, 4, opitems);
    AddOptionChoiceCB(ui->frame_decor, oc_explorer_cb, eui);
    ui->frame_offset = CreateSpinChoice(rc1, "Offset:", 5,
        SPIN_TYPE_FLOAT, 0.0, 1.0, 0.005);
    AddSpinChoiceCB(ui->frame_offset, sp_explorer_cb, eui);
    rc1 = CreateHContainer(rc);
    ui->linew = CreateLineWidthChoice(rc1, "Width:");
    AddSpinChoiceCB(ui->linew, sp_explorer_cb, eui);
    ui->lines = CreateLineStyleChoice(rc1, "Style:");
    AddOptionChoiceCB(ui->lines, oc_explorer_cb, eui);
    rc1 = CreateHContainer(rc);
    ui->linepen = CreatePenChoice(rc1, "Outline pen:");
    AddPenChoiceCB(ui->linepen, pen_explorer_cb, eui);
    ui->fillpen = CreatePenChoice(rc1, "Fill pen:");
    AddPenChoiceCB(ui->fillpen, pen_explorer_cb, eui);

    fr = CreateFrame(ui->frame_tp, "Pointer");
    rc = CreateVContainer(fr);
    ui->arrow_flag = CreateToggleButton(rc, "Enabled");
    AddToggleButtonCB(ui->arrow_flag, tb_explorer_cb, eui);

    rc1 = CreateHContainer(rc);
    ui->a_type = CreateOptionChoiceVA(rc1, "Arrow type:",
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

void update_atext_ui(ATextUI *ui, Quark *q)
{
    if (q && quark_fid_get(q) == QFlavorAText) {
        char *format, buf[32];
        AText *at = atext_get_data(q);

        SetTextString(ui->text,    at->s);


        if (object_get_loctype(q) == COORD_WORLD) {
            format = "%.8g";
        } else {
            format = "%.4f";
        }
        sprintf(buf, format, at->ap.x);
        SetTextString(ui->x, buf);
        sprintf(buf, format, at->ap.y);
        SetTextString(ui->y, buf);
        
        SetSpinChoice(ui->offsetx, at->offset.x);
        SetSpinChoice(ui->offsety, at->offset.y);

        SetOptionChoice(ui->font,  at->text_props.font);
        SetSpinChoice(ui->size,    at->text_props.charsize);
        SetOptionChoice(ui->color, at->text_props.color);
        SetOptionChoice(ui->just,  at->text_props.just);
        SetAngleChoice(ui->angle,  (int) rint(at->text_props.angle));

        SetOptionChoice(ui->frame_decor, at->frame_decor);
        SetSpinChoice(ui->frame_offset, at->frame_offset);
        SetSpinChoice(ui->linew,   at->line.width);
        SetOptionChoice(ui->lines, at->line.style);
        SetPenChoice(ui->linepen, &at->line.pen);
        SetPenChoice(ui->fillpen, &at->fillpen);
        
        SetToggleButtonState(ui->arrow_flag, at->arrow_flag);
        SetOptionChoice(ui->a_type, at->arrow.type);
        SetSpinChoice(ui->a_length, at->arrow.length);
        SetSpinChoice(ui->a_dL_ff, at->arrow.dL_ff);
        SetSpinChoice(ui->a_lL_ff, at->arrow.lL_ff);
    }
}

int set_atext_data(ATextUI *ui, Quark *q, void *caller)
{
    if (ui && q && quark_fid_get(q) == QFlavorAText) {
        AText *at = atext_get_data(q);

        if (!caller || caller == ui->text) {
            char *s = GetTextString(ui->text);
            atext_set_string(q, s);
            xfree(s);
        }

        if (!caller || caller == ui->x) {
            xv_evalexpr(ui->x->text, &at->ap.x);
        }
        if (!caller || caller == ui->y) {
            xv_evalexpr(ui->y->text, &at->ap.y);
        }
        if (!caller || caller == ui->offsetx) {
            at->offset.x = GetSpinChoice(ui->offsetx);
        }
        if (!caller || caller == ui->offsety) {
            at->offset.y = GetSpinChoice(ui->offsety);
        }

        if (!caller || caller == ui->font) {
            atext_set_font(q, GetOptionChoice(ui->font));
        }
        if (!caller || caller == ui->size) {
            atext_set_char_size(q, GetSpinChoice(ui->size));
        }
        if (!caller || caller == ui->color) {
            atext_set_color(q, GetOptionChoice(ui->color));
        }
        if (!caller || caller == ui->just) {
            atext_set_just(q, GetOptionChoice(ui->just));
        }
        if (!caller || caller == ui->angle) {
            atext_set_angle(q, GetAngleChoice(ui->angle));
        }

        if (!caller || caller == ui->frame_decor) {
            at->frame_decor = GetOptionChoice(ui->frame_decor);
        }
        if (!caller || caller == ui->frame_offset) {
            at->frame_offset = GetSpinChoice(ui->frame_offset);
        }
        if (!caller || caller == ui->linew) {
            at->line.width = GetSpinChoice(ui->linew);
        }
        if (!caller || caller == ui->lines) {
            at->line.style = GetOptionChoice(ui->lines);
        }
        if (!caller || caller == ui->linepen) {
            GetPenChoice(ui->linepen, &at->line.pen);
        }
        if (!caller || caller == ui->fillpen) {
            GetPenChoice(ui->fillpen, &at->fillpen);
        }

        if (!caller || caller == ui->arrow_flag) {
            at->arrow_flag   = GetToggleButtonState(ui->arrow_flag);
        }
        if (!caller || caller == ui->a_type) {
            at->arrow.type   = GetOptionChoice(ui->a_type);
        }
        if (!caller || caller == ui->a_length) {
            at->arrow.length = GetSpinChoice(ui->a_length);
        }
        if (!caller || caller == ui->a_dL_ff) {
            at->arrow.dL_ff  = GetSpinChoice(ui->a_dL_ff);
        }
        if (!caller || caller == ui->a_lL_ff) {
            at->arrow.lL_ff  = GetSpinChoice(ui->a_lL_ff);
        }
    
        return RETURN_SUCCESS;
    } else {
        return RETURN_SUCCESS;
    }
}
