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

/* Frame UI */

#include "core_utils.h"
#include "explorer.h"
#include "protos.h"

FrameUI *create_frame_ui(ExplorerUI *eui)
{
    FrameUI *ui;
    
    Widget tab;
    Widget rc, rc1, rc2, fr;

    ui = xmalloc(sizeof(FrameUI));

    /* ------------ Tabs -------------- */

    tab = CreateTab(eui->scrolled_window);        
    AddHelpCB(tab, "doc/UsersGuide.html#frame-properties");


    /* ------------ Main tab -------------- */

    ui->main_tp = CreateTabPage(tab, "Main");

    fr = CreateFrame(ui->main_tp, "Viewport");
    rc = CreateVContainer(fr);

    rc1 = CreateHContainer(rc);
    ui->view_xv1 = CreateTextItem2(rc1, 8, "Xmin:");
    AddTextItemCB(ui->view_xv1, titem_explorer_cb, eui);
    ui->view_xv2 = CreateTextItem2(rc1, 8, "Xmax:");
    AddTextItemCB(ui->view_xv2, titem_explorer_cb, eui);

    rc1 = CreateHContainer(rc);
    ui->view_yv1 = CreateTextItem2(rc1, 8, "Ymin:");
    AddTextItemCB(ui->view_yv1, titem_explorer_cb, eui);
    ui->view_yv2 = CreateTextItem2(rc1, 8, "Ymax:");
    AddTextItemCB(ui->view_yv2, titem_explorer_cb, eui);

    fr = CreateFrame(ui->main_tp, "Frame box");
    rc = CreateVContainer(fr);
    ui->frame_framestyle_choice = CreatePanelChoice(rc, "Frame type:",
						     "Closed",
						     "Half open",
						     "Break top",
						     "Break bottom",
						     "Break left",
						     "Break right",
						     NULL);
    AddOptionChoiceCB(ui->frame_framestyle_choice, oc_explorer_cb, eui);

    rc2 = CreateHContainer(rc);
    ui->frame_linew_choice = CreateLineWidthChoice(rc2, "Width:");
    AddSpinChoiceCB(ui->frame_linew_choice, sp_explorer_cb, eui);
    ui->frame_lines_choice = CreateLineStyleChoice(rc2, "Style:");
    AddOptionChoiceCB(ui->frame_lines_choice, oc_explorer_cb, eui);

    rc2 = CreateHContainer(rc);
    ui->frame_pen = CreatePenChoice(rc2, "Outline pen:");
    AddPenChoiceCB(ui->frame_pen, pen_explorer_cb, eui);
    ui->frame_fillpen = CreatePenChoice(rc2, "Fill pen:");
    AddPenChoiceCB(ui->frame_fillpen, pen_explorer_cb, eui);

    fr = CreateFrame(ui->main_tp, "Display options");
    rc = CreateHContainer(fr);
    ui->toggle_legends = CreateToggleButton(rc, "Display legend");
    AddToggleButtonCB(ui->toggle_legends, tb_explorer_cb, eui);


    /* ------------ Legend frame tab -------------- */

    ui->legendbox_tp = CreateTabPage(tab, "Leg. box");

    fr = CreateFrame(ui->legendbox_tp, "Anchor point");

    rc1 = CreateHContainer(fr);
    ui->legend_anchor_x = CreateSpinChoice(rc1, "X:", 5,
        SPIN_TYPE_FLOAT, 0.0, 1.0, 0.01);
    AddSpinChoiceCB(ui->legend_anchor_x, sp_explorer_cb, eui);
    ui->legend_anchor_y = CreateSpinChoice(rc1, "Y:", 5,
        SPIN_TYPE_FLOAT, 0.0, 1.0, 0.01);
    AddSpinChoiceCB(ui->legend_anchor_y, sp_explorer_cb, eui);

    fr = CreateFrame(ui->legendbox_tp, "Offset");
    rc1 = CreateHContainer(fr);
    ui->legend_dx = CreateSpinChoice(rc1, "dX:", 5,
        SPIN_TYPE_FLOAT, -1.0, 1.0, 0.01);
    AddSpinChoiceCB(ui->legend_dx, sp_explorer_cb, eui);
    ui->legend_dy = CreateSpinChoice(rc1, "dY:", 5,
        SPIN_TYPE_FLOAT, -1.0, 1.0, 0.01);
    AddSpinChoiceCB(ui->legend_dy, sp_explorer_cb, eui);

    fr = CreateFrame(ui->legendbox_tp, "Frame box");
    rc = CreateVContainer(fr);

    ui->legend_just = CreateJustChoice(rc, "Justification:");
    AddOptionChoiceCB(ui->legend_just, oc_explorer_cb, eui);

    rc2 = CreateHContainer(rc);
    ui->legend_boxlinew = CreateLineWidthChoice(rc2, "Width:");
    AddSpinChoiceCB(ui->legend_boxlinew, sp_explorer_cb, eui);
    ui->legend_boxlines = CreateLineStyleChoice(rc2, "Style:");
    AddOptionChoiceCB(ui->legend_boxlines, oc_explorer_cb, eui);

    rc2 = CreateHContainer(rc);
    ui->legend_boxpen = CreatePenChoice(rc2, "Outline pen:");
    AddPenChoiceCB(ui->legend_boxpen, pen_explorer_cb, eui);
    ui->legend_boxfillpen = CreatePenChoice(rc2, "Fill pen:");
    AddPenChoiceCB(ui->legend_boxfillpen, pen_explorer_cb, eui);

    /* ------------ Legends tab -------------- */

    ui->legends_tp = CreateTabPage(tab, "Legends");

    fr = CreateFrame(ui->legends_tp, "Text properties");
    rc = CreateVContainer(fr);
    ui->legend_font = CreateFontChoice(rc, "Font:");
    AddOptionChoiceCB(ui->legend_font, oc_explorer_cb, eui);
    rc1 = CreateHContainer(rc);
    ui->legend_charsize = CreateCharSizeChoice(rc1, "Size:");
    AddSpinChoiceCB(ui->legend_charsize, sp_explorer_cb, eui);
    ui->legend_color = CreateColorChoice(rc1, "Color:");
    AddOptionChoiceCB(ui->legend_color, oc_explorer_cb, eui);

    fr = CreateFrame(ui->legends_tp, "Placement");
    rc = CreateVContainer(fr);
    rc1 = CreateHContainer(rc);
    ui->legends_vgap = CreateSpinChoice(rc1, "V-gap:",
        4, SPIN_TYPE_FLOAT, 0.0, 1.0, 0.01);
    AddSpinChoiceCB(ui->legends_vgap, sp_explorer_cb, eui);
    ui->legends_hgap = CreateSpinChoice(rc1, "H-gap:",
        4, SPIN_TYPE_FLOAT, 0.0, 1.0, 0.01);
    AddSpinChoiceCB(ui->legends_hgap, sp_explorer_cb, eui);
    ui->legends_len = CreateSpinChoice(rc, "Line length:",
        4, SPIN_TYPE_FLOAT, 0.0, 1.0, 0.01);
    AddSpinChoiceCB(ui->legends_len, sp_explorer_cb, eui);
    rc1 = CreateHContainer(rc);
    ui->legends_invert = CreateToggleButton(rc1, "Put in reverse order");
    AddToggleButtonCB(ui->legends_invert, tb_explorer_cb, eui);
    ui->legends_singlesym = CreateToggleButton(rc1, "Single symbol");
    AddToggleButtonCB(ui->legends_singlesym, tb_explorer_cb, eui);


    SelectTabPage(tab, ui->main_tp);

    ui->top = tab;
    
    return ui;
}

void update_frame_ui(FrameUI *ui, Quark *q)
{
    frame *f = frame_get_data(q);
    if (f) {
        char buf[32];
        view v;
        legend *l;
    
	frame_get_view(q, &v);
	l = frame_get_legend(q);
        
        sprintf(buf, "%.9g", v.xv1);
	xv_setstr(ui->view_xv1, buf);
	sprintf(buf, "%.9g", v.xv2);
	xv_setstr(ui->view_xv2, buf);
	sprintf(buf, "%.9g", v.yv1);
	xv_setstr(ui->view_yv1, buf);
	sprintf(buf, "%.9g", v.yv2);
	xv_setstr(ui->view_yv2, buf);

	SetOptionChoice(ui->frame_framestyle_choice, f->type);
	SetPenChoice(ui->frame_pen, &f->outline.pen);
	SetSpinChoice(ui->frame_linew_choice, f->outline.width);
	SetOptionChoice(ui->frame_lines_choice, f->outline.style);
	SetPenChoice(ui->frame_fillpen, &f->fillpen);
 
        SetSpinChoice(ui->legend_charsize, l->charsize);

	SetToggleButtonState(ui->toggle_legends, l->active == TRUE);

	SetSpinChoice(ui->legend_anchor_x, l->anchor.x);
	SetSpinChoice(ui->legend_anchor_y, l->anchor.y);
	SetOptionChoice(ui->legend_just, l->just);
	SetSpinChoice(ui->legend_dx, l->offset.x);
	SetSpinChoice(ui->legend_dy, l->offset.y);

	SetSpinChoice(ui->legends_vgap, l->vgap);
	SetSpinChoice(ui->legends_hgap, l->hgap);
	SetSpinChoice(ui->legends_len, l->len);
	SetToggleButtonState(ui->legends_invert, l->invert);
	SetToggleButtonState(ui->legends_singlesym, l->singlesym);

	SetOptionChoice(ui->legend_font, l->font);
	SetOptionChoice(ui->legend_color, l->color);
	SetSpinChoice(ui->legend_boxlinew, l->boxline.width);
	SetOptionChoice(ui->legend_boxlines, l->boxline.style);
	SetPenChoice(ui->legend_boxpen, &l->boxline.pen);
	SetPenChoice(ui->legend_boxfillpen, &l->boxfillpen);
    }
}

int set_frame_data(FrameUI *ui, Quark *q, void *caller)
{
    frame *f = frame_get_data(q);
    if (f) {
        view vtmp;
        legend *l;

        frame_get_view(q, &vtmp);
        l = frame_get_legend(q);

        if (!caller || caller == ui->view_xv1) {
            xv_evalexpr(ui->view_xv1, &vtmp.xv1);  
        }
        if (!caller || caller == ui->view_xv2) {
            xv_evalexpr(ui->view_xv2, &vtmp.xv2);  
        }
        if (!caller || caller == ui->view_yv1) {
            xv_evalexpr(ui->view_yv1, &vtmp.yv1);  
        }
        if (!caller || caller == ui->view_yv2) {
            xv_evalexpr(ui->view_yv2, &vtmp.yv2);  
        }
        if (!isvalid_viewport(&vtmp)) {
            errmsg("Invalid viewport coordinates");
            return RETURN_FAILURE;
        } else {
            frame_set_view(q, &vtmp);
        }

        if (!caller || caller == ui->frame_framestyle_choice) {
            f->type = GetOptionChoice(ui->frame_framestyle_choice);
        }
        if (!caller || caller == ui->frame_pen) {
            GetPenChoice(ui->frame_pen, &f->outline.pen);
        }
        if (!caller || caller == ui->frame_linew_choice) {
            f->outline.width = GetSpinChoice(ui->frame_linew_choice);
        }
        if (!caller || caller == ui->frame_lines_choice) {
            f->outline.style = GetOptionChoice(ui->frame_lines_choice);
        }
        if (!caller || caller == ui->frame_fillpen) {
            GetPenChoice(ui->frame_fillpen, &f->fillpen);
        }
        if (!caller || caller == ui->legend_charsize) {
            l->charsize = GetSpinChoice(ui->legend_charsize);
        }
        if (!caller || caller == ui->toggle_legends) {
            l->active = GetToggleButtonState(ui->toggle_legends);
        }
        if (!caller || caller == ui->legends_vgap) {
            l->vgap = GetSpinChoice(ui->legends_vgap);
        }
        if (!caller || caller == ui->legends_hgap) {
            l->hgap = GetSpinChoice(ui->legends_hgap);
        } 
        if (!caller || caller == ui->legends_len) {
            l->len = GetSpinChoice(ui->legends_len);
        }
        if (!caller || caller == ui->legends_invert) {
            l->invert = GetToggleButtonState(ui->legends_invert);
        }
        if (!caller || caller == ui->legends_singlesym) {
            l->singlesym = GetToggleButtonState(ui->legends_singlesym);
        }
        if (!caller || caller == ui->legend_anchor_x) {
            l->anchor.x = GetSpinChoice(ui->legend_anchor_x);
        }
        if (!caller || caller == ui->legend_anchor_y) {
            l->anchor.y = GetSpinChoice(ui->legend_anchor_y);
        }
        if (!caller || caller == ui->legend_just) {
            l->just = GetOptionChoice(ui->legend_just);
        }
        if (!caller || caller == ui->legend_dx) {
            l->offset.x = GetSpinChoice(ui->legend_dx);
        }
        if (!caller || caller == ui->legend_dy) {
            l->offset.y = GetSpinChoice(ui->legend_dy);
        }
        if (!caller || caller == ui->legend_font) {
            l->font = GetOptionChoice(ui->legend_font);
        }
        if (!caller || caller == ui->legend_color) {
            l->color = GetOptionChoice(ui->legend_color);
        }
        if (!caller || caller == ui->legend_boxpen) {
            GetPenChoice(ui->legend_boxpen, &l->boxline.pen);
        }
        if (!caller || caller == ui->legend_boxlinew) {
            l->boxline.width = GetSpinChoice(ui->legend_boxlinew);
        }
        if (!caller || caller == ui->legend_boxlines) {
            l->boxline.style = GetOptionChoice(ui->legend_boxlines);
        }
        if (!caller || caller == ui->legend_boxfillpen) {
            GetPenChoice(ui->legend_boxfillpen, &l->boxfillpen);
        }

        quark_dirtystate_set(q, TRUE);
        
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}
