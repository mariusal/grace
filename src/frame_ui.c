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

/* Frame UI */

#include "explorer.h"
#include "protos.h"

#include "cbitmaps.h"

FrameUI *create_frame_ui(ExplorerUI *eui)
{
    FrameUI *ui;
    
    Widget tab;
    Widget rc, rc1, rc2, fr;

    BitmapOptionItem opitems[4] = {
        {CORNER_UL, ul_bits},
        {CORNER_LL, ll_bits},
        {CORNER_UR, ur_bits},
        {CORNER_LR, lr_bits}
    };

    ui = xmalloc(sizeof(FrameUI));

    /* ------------ Tabs -------------- */

    tab = CreateTab(eui->scrolled_window);        


    /* ------------ Main tab -------------- */

    ui->main_tp = CreateTabPage(tab, "Main");

    fr = CreateFrame(ui->main_tp, "Titles");
    rc = CreateVContainer(fr);
    ui->label_title_text = CreateCSText(rc, "Title: ");
    AddTextInputCB(ui->label_title_text, text_explorer_cb, eui);
    ui->label_subtitle_text = CreateCSText(rc, "Subtitle: ");
    AddTextInputCB(ui->label_subtitle_text, text_explorer_cb, eui);

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

    fr = CreateFrame(ui->main_tp, "Display options");
    rc = CreateHContainer(fr);
    ui->toggle_legends = CreateToggleButton(rc, "Display legend");
    AddToggleButtonCB(ui->toggle_legends, tb_explorer_cb, eui);


    /* ------------ Titles tab -------------- */

    ui->titles_tp = CreateTabPage(tab, "Titles");

    fr = CreateFrame(ui->titles_tp, "Title");
    rc2 = CreateVContainer(fr);
    ui->title_font = CreateFontChoice(rc2, "Font:");
    AddOptionChoiceCB(ui->title_font, oc_explorer_cb, eui);
    ui->title_size = CreateCharSizeChoice(rc2, "Character size");
    AddScaleCB(ui->title_size, scale_explorer_cb, eui);
    ui->title_color = CreateColorChoice(rc2, "Color:");
    AddOptionChoiceCB(ui->title_color, oc_explorer_cb, eui);

    fr = CreateFrame(ui->titles_tp, "Subtitle");
    rc2 = CreateVContainer(fr);
    ui->stitle_font = CreateFontChoice(rc2, "Font:");
    AddOptionChoiceCB(ui->stitle_font, oc_explorer_cb, eui);
    ui->stitle_size = CreateCharSizeChoice(rc2, "Character size");
    AddScaleCB(ui->stitle_size, scale_explorer_cb, eui);
    ui->stitle_color = CreateColorChoice(rc2, "Color:");
    AddOptionChoiceCB(ui->stitle_color, oc_explorer_cb, eui);

    /* ------------ Frame tab -------------- */

    ui->frame_tp = CreateTabPage(tab, "Frame");

    fr = CreateFrame(ui->frame_tp, "Frame box");
    rc = CreateVContainer(fr);
    ui->frame_framestyle_choice = CreatePanelChoice(rc, "Frame type:",
						     7,
						     "Closed",
						     "Half open",
						     "Break top",
						     "Break bottom",
						     "Break left",
						     "Break right",
						     NULL,
						     NULL);
    AddOptionChoiceCB(ui->frame_framestyle_choice, oc_explorer_cb, eui);

    rc2 = CreateHContainer(rc);
    ui->frame_color_choice = CreateColorChoice(rc2, "Color:");
    AddOptionChoiceCB(ui->frame_color_choice, oc_explorer_cb, eui);
    ui->frame_pattern_choice = CreatePatternChoice(rc2, "Pattern:");
    AddOptionChoiceCB(ui->frame_pattern_choice, oc_explorer_cb, eui);

    rc2 = CreateHContainer(rc);
    ui->frame_linew_choice = CreateLineWidthChoice(rc2, "Width:");
    AddSpinButtonCB(ui->frame_linew_choice, sp_explorer_cb, eui);
    ui->frame_lines_choice = CreateLineStyleChoice(rc2, "Style:");
    AddOptionChoiceCB(ui->frame_lines_choice, oc_explorer_cb, eui);

    fr = CreateFrame(ui->frame_tp, "Frame fill");
    rc = CreateHContainer(fr);
    ui->frame_fillcolor_choice = CreateColorChoice(rc, "Color:");
    AddOptionChoiceCB(ui->frame_fillcolor_choice, oc_explorer_cb, eui);
    ui->frame_fillpattern_choice = CreatePatternChoice(rc, "Pattern:");
    AddOptionChoiceCB(ui->frame_fillpattern_choice, oc_explorer_cb, eui);


    /* ------------ Legend frame tab -------------- */

    ui->legendbox_tp = CreateTabPage(tab, "Leg. box");

    fr = CreateFrame(ui->legendbox_tp, "Anchor point");
    rc = CreateVContainer(fr);
    ui->legend_acorner = CreateBitmapOptionChoice(rc,
        "Corner:", 2, 4, CBITMAP_WIDTH, CBITMAP_HEIGHT, opitems);
    AddOptionChoiceCB(ui->legend_acorner, oc_explorer_cb, eui);

    rc1 = CreateHContainer(rc);
    ui->legend_x = CreateSpinChoice(rc1, "dX:", 4,
        SPIN_TYPE_FLOAT, -1.0, 1.0, 0.01);
    AddSpinButtonCB(ui->legend_x, sp_explorer_cb, eui);
    ui->legend_y = CreateSpinChoice(rc1, "dY:", 4,
        SPIN_TYPE_FLOAT, -1.0, 1.0, 0.01);
    AddSpinButtonCB(ui->legend_y, sp_explorer_cb, eui);

    fr = CreateFrame(ui->legendbox_tp, "Frame line");
    rc = CreateVContainer(fr);

    rc2 = CreateHContainer(rc);
    ui->legend_boxcolor = CreateColorChoice(rc2, "Color:");
    AddOptionChoiceCB(ui->legend_boxcolor, oc_explorer_cb, eui);
    ui->legend_boxpattern = CreatePatternChoice(rc2, "Pattern:");
    AddOptionChoiceCB(ui->legend_boxpattern, oc_explorer_cb, eui);

    rc2 = CreateHContainer(rc);
    ui->legend_boxlinew = CreateLineWidthChoice(rc2, "Width:");
    AddSpinButtonCB(ui->legend_boxlinew, sp_explorer_cb, eui);
    ui->legend_boxlines = CreateLineStyleChoice(rc2, "Style:");
    AddOptionChoiceCB(ui->legend_boxlines, oc_explorer_cb, eui);

    fr = CreateFrame(ui->legendbox_tp, "Frame fill");
    rc = CreateHContainer(fr);
    ui->legend_boxfillcolor = CreateColorChoice(rc, "Color:");
    AddOptionChoiceCB(ui->legend_boxfillcolor, oc_explorer_cb, eui);
    ui->legend_boxfillpat = CreatePatternChoice(rc, "Pattern:");
    AddOptionChoiceCB(ui->legend_boxfillpat, oc_explorer_cb, eui);


    /* ------------ Legends tab -------------- */

    ui->legends_tp = CreateTabPage(tab, "Legends");

    fr = CreateFrame(ui->legends_tp, "Text properties");
    rc = CreateVContainer(fr);
    ui->legend_font = CreateFontChoice(rc, "Font:");
    AddOptionChoiceCB(ui->legend_font, oc_explorer_cb, eui);
    ui->legend_charsize = CreateCharSizeChoice(rc, "Char size");
    AddScaleCB(ui->legend_charsize, scale_explorer_cb, eui);
    ui->legend_color = CreateColorChoice(rc, "Color:");
    AddOptionChoiceCB(ui->legend_color, oc_explorer_cb, eui);

    fr = CreateFrame(ui->legends_tp, "Placement");
    rc = CreateVContainer(fr);
    rc1 = CreateHContainer(rc);
    ui->legends_vgap = CreateSpinChoice(rc1, "V-gap:",
        4, SPIN_TYPE_FLOAT, 0.0, 1.0, 0.01);
    AddSpinButtonCB(ui->legends_vgap, sp_explorer_cb, eui);
    ui->legends_hgap = CreateSpinChoice(rc1, "H-gap:",
        4, SPIN_TYPE_FLOAT, 0.0, 1.0, 0.01);
    AddSpinButtonCB(ui->legends_hgap, sp_explorer_cb, eui);
    ui->legends_len = CreateSpinChoice(rc, "Line length:",
        4, SPIN_TYPE_FLOAT, 0.0, 1.0, 0.01);
    AddSpinButtonCB(ui->legends_len, sp_explorer_cb, eui);
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
        view *v;
        legend *l;
        labels *labs;
    
	v = frame_get_view(q);
	l = frame_get_legend(q);
        labs = frame_get_labels(q);
        
        
        sprintf(buf, "%.9g", v->xv1);
	xv_setstr(ui->view_xv1, buf);
	sprintf(buf, "%.9g", v->xv2);
	xv_setstr(ui->view_xv2, buf);
	sprintf(buf, "%.9g", v->yv1);
	xv_setstr(ui->view_yv1, buf);
	sprintf(buf, "%.9g", v->yv2);
	xv_setstr(ui->view_yv2, buf);

	SetOptionChoice(ui->frame_framestyle_choice, f->type);
	SetOptionChoice(ui->frame_color_choice, f->outline.pen.color);
	SetOptionChoice(ui->frame_pattern_choice, f->outline.pen.pattern);
	SetSpinChoice(ui->frame_linew_choice, f->outline.width);
	SetOptionChoice(ui->frame_lines_choice, f->outline.style);
	SetOptionChoice(ui->frame_fillcolor_choice, f->fillpen.color);
	SetOptionChoice(ui->frame_fillpattern_choice, f->fillpen.pattern);
 
        SetCharSizeChoice(ui->legend_charsize, l->charsize);

	SetToggleButtonState(ui->toggle_legends, l->active == TRUE);

	SetOptionChoice(ui->legend_acorner, l->acorner);
	SetSpinChoice(ui->legend_x, l->offset.x);
	SetSpinChoice(ui->legend_y, l->offset.y);

	SetSpinChoice(ui->legends_vgap, l->vgap);
	SetSpinChoice(ui->legends_hgap, l->hgap);
	SetSpinChoice(ui->legends_len, l->len);
	SetToggleButtonState(ui->legends_invert, l->invert);
	SetToggleButtonState(ui->legends_singlesym, l->singlesym);

	SetOptionChoice(ui->legend_font, l->font);
	SetOptionChoice(ui->legend_color, l->color);
	SetOptionChoice(ui->legend_boxfillcolor, l->boxfillpen.color);
	SetOptionChoice(ui->legend_boxfillpat, l->boxfillpen.pattern);
	SetOptionChoice(ui->legend_boxcolor, l->boxline.pen.color);
	SetOptionChoice(ui->legend_boxpattern, l->boxline.pen.pattern);
	SetSpinChoice(ui->legend_boxlinew, l->boxline.width);
	SetOptionChoice(ui->legend_boxlines, l->boxline.style);

        SetTextString(ui->label_title_text, labs->title.s);
        SetTextString(ui->label_subtitle_text, labs->stitle.s);
 
        SetCharSizeChoice(ui->title_size, labs->title.charsize);
        SetCharSizeChoice(ui->stitle_size, labs->stitle.charsize);

        SetOptionChoice(ui->title_color, labs->title.color);
        SetOptionChoice(ui->stitle_color, labs->stitle.color);

        SetOptionChoice(ui->title_font, labs->title.font);
        SetOptionChoice(ui->stitle_font, labs->stitle.font);
    }
}

int set_frame_data(FrameUI *ui, Quark *q, void *caller)
{
    frame *f = frame_get_data(q);
    if (f) {
        view *v, vtmp;
        legend *l;
        labels *labs;

        v    = frame_get_view(q);
        labs = frame_get_labels(q);
        l    = frame_get_legend(q);

        vtmp = *v;
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

        if (!caller || caller == ui->label_title_text) {
            char *s = GetTextString(ui->label_title_text);
            set_plotstr_string(&labs->title, s);
            xfree(s);
        }
        if (!caller || caller == ui->title_size) {
            labs->title.charsize = GetCharSizeChoice(ui->title_size);
        }
        if (!caller || caller == ui->title_color) {
            labs->title.color = GetOptionChoice(ui->title_color);
        }
        if (!caller || caller == ui->title_font) {
            labs->title.font = GetOptionChoice(ui->title_font);
        }
        if (!caller || caller == ui->label_subtitle_text) {
            char *s = GetTextString(ui->label_subtitle_text);
            set_plotstr_string(&labs->stitle, s);
            xfree(s);
        }
        if (!caller || caller == ui->stitle_size) {
            labs->stitle.charsize = GetCharSizeChoice(ui->stitle_size);
        }
        if (!caller || caller == ui->stitle_color) {
            labs->stitle.color = GetOptionChoice(ui->stitle_color);
        }
        if (!caller || caller == ui->stitle_font) {
            labs->stitle.font = GetOptionChoice(ui->stitle_font);
        }
        if (!caller || caller == ui->frame_framestyle_choice) {
            f->type = GetOptionChoice(ui->frame_framestyle_choice);
        }
        if (!caller || caller == ui->frame_color_choice) {
            f->outline.pen.color = GetOptionChoice(ui->frame_color_choice);
        }
        if (!caller || caller == ui->frame_pattern_choice) {
            f->outline.pen.pattern = GetOptionChoice(ui->frame_pattern_choice);
        }
        if (!caller || caller == ui->frame_linew_choice) {
            f->outline.width = GetSpinChoice(ui->frame_linew_choice);
        }
        if (!caller || caller == ui->frame_lines_choice) {
            f->outline.style = GetOptionChoice(ui->frame_lines_choice);
        }
        if (!caller || caller == ui->frame_fillcolor_choice) {
            f->fillpen.color = GetOptionChoice(ui->frame_fillcolor_choice);
        }
        if (!caller || caller == ui->frame_fillpattern_choice) {
            f->fillpen.pattern = GetOptionChoice(ui->frame_fillpattern_choice);
        }
        if (!caller || caller == ui->legend_charsize) {
            l->charsize = GetCharSizeChoice(ui->legend_charsize);
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
        if (!caller || caller == ui->legend_acorner) {
            l->acorner = GetOptionChoice(ui->legend_acorner);
        }
        if (!caller || caller == ui->legend_x) {
            l->offset.x = GetSpinChoice(ui->legend_x);
        }
        if (!caller || caller == ui->legend_y) {
            l->offset.y = GetSpinChoice(ui->legend_y);
        }
        if (!caller || caller == ui->legend_font) {
            l->font = GetOptionChoice(ui->legend_font);
        }
        if (!caller || caller == ui->legend_color) {
            l->color = GetOptionChoice(ui->legend_color);
        }
        if (!caller || caller == ui->legend_boxfillcolor) {
            l->boxfillpen.color = GetOptionChoice(ui->legend_boxfillcolor);
        }
        if (!caller || caller == ui->legend_boxfillpat) {
            l->boxfillpen.pattern = GetOptionChoice(ui->legend_boxfillpat);
        }
        if (!caller || caller == ui->legend_boxcolor) {
            l->boxline.pen.color = GetOptionChoice(ui->legend_boxcolor);
        }
        if (!caller || caller == ui->legend_boxpattern) {
            l->boxline.pen.pattern = GetOptionChoice(ui->legend_boxpattern);
        }
        if (!caller || caller == ui->legend_boxlinew) {
            l->boxline.width = GetSpinChoice(ui->legend_boxlinew);
        }
        if (!caller || caller == ui->legend_boxlines) {
            l->boxline.style = GetOptionChoice(ui->legend_boxlines);
        }

        quark_dirtystate_set(q, TRUE);
        
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}
