/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1996-2004 Grace Development Team
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
 * ticks / tick labels / axis labels
 *
 */

#include <config.h>

#include "core_utils.h"
#include <Xm/ScrolledW.h>
#include "explorer.h"
#include "protos.h"

static void auto_spec_cb(OptionStructure *opt, int value, void *data);

AxisUI *create_axis_ui(ExplorerUI *eui)
{
    AxisUI *ui;
    int i;
    OptionItem opitems[3];
    char buf[32];
    Widget tab, rc, rc2, rc3, fr, sw;

    ui = xmalloc(sizeof(AxisUI));

    /* ------------ Tabs --------------*/
    tab = CreateTab(eui->scrolled_window); 

    ui->main_tp = CreateTabPage(tab, "Main");

    rc = CreateHContainer(ui->main_tp);
    ui->active = CreateToggleButton(rc, "Active");
    AddToggleButtonCB(ui->active, tb_explorer_cb, eui);
    opitems[0].value = AXIS_TYPE_X;
    opitems[0].label = "X";
    opitems[1].value = AXIS_TYPE_Y;
    opitems[1].label = "Y";
    ui->type = CreateOptionChoice(rc, "Type:", 0, 2, opitems);
    AddOptionChoiceCB(ui->type, oc_explorer_cb, eui);

    fr = CreateFrame(ui->main_tp, "Axis label");

    ui->label = CreateCSText(fr, "Label string:");
    AddTextInputCB(ui->label, text_explorer_cb, eui);

    fr = CreateFrame(ui->main_tp, "Tick properties");
    rc = CreateVContainer(fr);

    rc2 = CreateHContainer(rc);
    ui->tmajor = CreateTextItem2(rc2, 8, "Major spacing:");
    AddTextItemCB(ui->tmajor, titem_explorer_cb, eui);
    ui->nminor = CreateSpinChoice(rc2, "Minor ticks:",
        2, SPIN_TYPE_INT, 0.0, (double) MAX_TICKS - 1, 1.0);
    AddSpinChoiceCB(ui->nminor, sp_explorer_cb, eui);

    rc2 = CreateHContainer(rc);
    ui->tlform = CreateFormatChoice(rc2, "Format:");
    AddOptionChoiceCB(ui->tlform , oc_explorer_cb, eui);
    ui->tlprec = CreatePrecisionChoice(rc2, "Precision:");
    AddOptionChoiceCB(ui->tlprec, oc_explorer_cb, eui);

    fr = CreateFrame(ui->main_tp, "Display options");
    rc = CreateHContainer(fr);

    rc2 = CreateVContainer(rc);
    ui->tlonoff = CreateToggleButton(rc2, "Display tick labels");
    AddToggleButtonCB(ui->tlonoff, tb_explorer_cb, eui);
    ui->tonoff = CreateToggleButton(rc2, "Display tick marks");
    AddToggleButtonCB(ui->tonoff, tb_explorer_cb, eui);

    rc2 = CreateVContainer(rc);
    ui->baronoff = CreateToggleButton(rc2, "Display  bar");
    AddToggleButtonCB(ui->baronoff, tb_explorer_cb, eui);

    fr = CreateFrame(ui->main_tp, "Axis placement");
    rc = CreateHContainer(fr);
    ui->zero = CreateToggleButton(rc, "Zero ");
    AddToggleButtonCB(ui->zero, tb_explorer_cb, eui);
    ui->offx = CreateTextItem2(rc, 5, "Offsets - Left/bottom:");
    AddTextItemCB(ui->offx, titem_explorer_cb, eui);
    ui->offy = CreateTextItem2(rc, 5, "Right/top:");
    AddTextItemCB(ui->offy, titem_explorer_cb, eui);

    fr = CreateFrame(ui->main_tp, "Tick label properties");
    rc = CreateHContainer(fr);

    ui->tlfont = CreateFontChoice(rc, "Font:");
    AddOptionChoiceCB(ui->tlfont, oc_explorer_cb, eui);
    ui->tlcolor = CreateColorChoice(rc, "Color:");
    AddOptionChoiceCB(ui->tlcolor, oc_explorer_cb, eui);


    ui->label_tp = CreateTabPage(tab, "Axis label & bar");

    fr = CreateFrame(ui->label_tp, "Label properties");
    rc = CreateVContainer(fr);

    rc2 = CreateHContainer(rc);
    ui->labelfont = CreateFontChoice(rc2, "Font:");
    AddOptionChoiceCB(ui->labelfont, oc_explorer_cb, eui);
    ui->labelcolor = CreateColorChoice(rc2, "Color:");
    AddOptionChoiceCB(ui->labelcolor, oc_explorer_cb, eui);

    rc2 = CreateHContainer(rc);
    ui->labelcharsize = CreateCharSizeChoice(rc2, "Size:");
    AddSpinChoiceCB(ui->labelcharsize, sp_explorer_cb, eui);

    ui->labellayout = CreatePanelChoice(rc2, "Layout:",
                                        "Parallel to ",
                                        "Perpendicular to ",
                                        NULL);
    AddOptionChoiceCB(ui->labellayout, oc_explorer_cb, eui);

    rc2 = CreateHContainer(rc);
    ui->labelop = CreatePanelChoice(rc2, "Side:",
                                         "Normal",
                                         "Opposite",
                                         "Both",
                                         NULL);
    AddOptionChoiceCB(ui->labelop, oc_explorer_cb, eui);
    opitems[0].value = TYPE_AUTO;
    opitems[0].label = "Auto";
    opitems[1].value = TYPE_SPEC;
    opitems[1].label = "Specified";
    ui->labelplace = CreateOptionChoice(rc2, "Location:", 0, 2, opitems);
    AddOptionChoiceCB(ui->labelplace, oc_explorer_cb, eui);
    ui->labelspec_rc = CreateHContainer(rc);
    AddOptionChoiceCB(ui->labelplace, auto_spec_cb, ui->labelspec_rc);
    ui->labelspec_para = CreateTextItem2(ui->labelspec_rc, 5, "Parallel offset:");
    AddTextItemCB(ui->labelspec_para, titem_explorer_cb, eui);
    ui->labelspec_perp = CreateTextItem2(ui->labelspec_rc, 5, "Perpendicular offset:");
    AddTextItemCB(ui->labelspec_perp, titem_explorer_cb, eui);

    fr = CreateFrame(ui->label_tp, "Bar properties");
    rc = CreateVContainer(fr);

    rc2 = CreateHContainer(rc);
    ui->barcolor = CreateColorChoice(rc2, "Color:");
    AddOptionChoiceCB(ui->barcolor, oc_explorer_cb, eui);
    ui->barlinew = CreateLineWidthChoice(rc2, "Width:");
    AddSpinChoiceCB(ui->barlinew, sp_explorer_cb, eui);

    ui->barlines = CreateLineStyleChoice(rc, "Line style:");
    AddOptionChoiceCB(ui->barlines, oc_explorer_cb, eui);


    ui->ticklabel_tp = CreateTabPage(tab, "Tick labels");

    fr = CreateFrame(ui->ticklabel_tp, "Labels");
    rc2 = CreateHContainer(fr);
    ui->tlcharsize = CreateCharSizeChoice(rc2, "Char size");
    AddSpinChoiceCB(ui->tlcharsize, sp_explorer_cb, eui);

    fr = CreateFrame(ui->ticklabel_tp, "Placement");
    rc = CreateHContainer(fr);

    rc2 = CreateVContainer(rc);
    ui->ticklop = CreatePanelChoice(rc2, "Side:",
                                "Normal",
                                "Opposite",
                                "Both",
                                NULL);
    AddOptionChoiceCB(ui->ticklop, oc_explorer_cb, eui);
    ui->tlstagger = CreatePanelChoice(rc2, "Stagger:",
                    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", NULL);
    AddOptionChoiceCB(ui->tlstagger, oc_explorer_cb, eui);


    rc2 = CreateVContainer(rc);
    rc3 = CreateHContainer(rc2);
    ui->tlstarttype = CreatePanelChoice(rc3, "Start at:",
                                    "Axis min", "Specified:", NULL);
    AddOptionChoiceCB(ui->tlstarttype, oc_explorer_cb, eui);
    ui->tlstart = CreateTextItem2(rc3, 8, "");
    AddTextItemCB(ui->tlstart, titem_explorer_cb, eui);

    rc3 = CreateHContainer(rc2);
    ui->tlstoptype = CreatePanelChoice(rc3, "Stop at:",
                                   "Axis max", "Specified:", NULL);
    AddOptionChoiceCB(ui->tlstoptype, oc_explorer_cb, eui);
    ui->tlstop = CreateTextItem2(rc3, 8, "");
    AddTextItemCB(ui->tlstop, titem_explorer_cb, eui);

    fr = CreateFrame(ui->ticklabel_tp, "Extra");
    rc = CreateVContainer(fr);

    opitems[0].value = TYPE_AUTO;
    opitems[0].label = "Auto";
    opitems[1].value = TYPE_SPEC;
    opitems[1].label = "Specified";
    rc2 = CreateHContainer(rc);
    ui->tlgaptype = CreateOptionChoice(rc2, "Location:", 0, 2, opitems);
    ui->tlangle = CreateAngleChoice(rc2, "Angle");
    SetScaleWidth(ui->tlangle, 200);
    AddScaleCB(ui->tlangle, scale_explorer_cb, eui);

    rc2 = CreateHContainer(rc);
    ui->tlskip = CreatePanelChoice(rc2, "Skip every:",
                    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", NULL);
    AddOptionChoiceCB(ui->tlskip, oc_explorer_cb, eui);

    ui->tlformula = CreateTextInput(rc2, "Axis transform:");
    AddTextInputCB(ui->tlformula, text_explorer_cb, eui);

    rc2 = CreateHContainer(rc);
    ui->tlprestr = CreateTextItem2(rc2, 13, "Prepend:");
    AddTextItemCB(ui->tlprestr, titem_explorer_cb, eui);
    ui->tlappstr = CreateTextItem2(rc2, 13, "Append:");
    AddTextItemCB(ui->tlappstr, titem_explorer_cb, eui);

    ui->tlgap_rc = CreateHContainer(rc);
    AddOptionChoiceCB(ui->tlgaptype, auto_spec_cb, ui->tlgap_rc);
    AddOptionChoiceCB(ui->tlgaptype, oc_explorer_cb, eui);
    ui->tlgap_para = CreateTextItem2(ui->tlgap_rc, 5, "Parallel offset:");
    AddTextItemCB(ui->tlgap_para, titem_explorer_cb, eui);
    ui->tlgap_perp = CreateTextItem2(ui->tlgap_rc, 5, "Perpendicular offset:");
    AddTextItemCB(ui->tlgap_perp, titem_explorer_cb, eui);


    ui->tickmark_tp = CreateTabPage(tab, "Tick marks");

    fr = CreateFrame(ui->tickmark_tp, "Placement");
    rc2 = CreateVContainer(fr);
    rc = CreateHContainer(rc2);
    ui->tickop = CreatePanelChoice(rc, "Draw on:",
                               "Normal side",
                               "Opposite side",
                               "Both sides",
                               NULL);
    AddOptionChoiceCB(ui->tickop, oc_explorer_cb, eui);
    rc = CreateHContainer(rc2);
    ui->tround = CreateToggleButton(rc, "Place at rounded positions");
    AddToggleButtonCB(ui->tround, tb_explorer_cb, eui);
    ui->autonum = CreatePanelChoice(rc, "Autotick divisions:",
		                "2",
                                "3",
                                "4",
                                "5",
                                "6",
                                "7",
                                "8",
                                "9",
                                "10",
                                "11",
                                "12",
				NULL);
    AddOptionChoiceCB(ui->autonum, oc_explorer_cb, eui);

    rc2 = CreateHContainer(ui->tickmark_tp);

    /* major tick marks */
    fr = CreateFrame(rc2, "Major ticks");
    rc = CreateVContainer(fr);
    ui->tgrid = CreateToggleButton(rc, "Draw grid lines");
    AddToggleButtonCB(ui->tgrid, tb_explorer_cb, eui);
    ui->tinout = CreatePanelChoice(rc, "Pointing:",
                                   "In", "Out", "Both", NULL);
    AddOptionChoiceCB(ui->tinout, oc_explorer_cb, eui);
    ui->tlen = CreateSpinChoice(rc, "Tick length",
        4, SPIN_TYPE_FLOAT, 0.0, 100.0, 0.25);
    AddSpinChoiceCB(ui->tlen, sp_explorer_cb, eui);
    ui->tgridcol = CreateColorChoice(rc, "Color:");
    AddOptionChoiceCB(ui->tgridcol, oc_explorer_cb, eui);
    ui->tgridlinew = CreateLineWidthChoice(rc, "Line width:");
    AddSpinChoiceCB(ui->tgridlinew, sp_explorer_cb, eui);
    ui->tgridlines = CreateLineStyleChoice(rc, "Line style:");
    AddOptionChoiceCB(ui->tgridlines, oc_explorer_cb, eui);

    fr = CreateFrame(rc2, "Minor ticks");
    rc = CreateVContainer(fr);
    ui->tmgrid = CreateToggleButton(rc, "Draw grid lines");
    AddToggleButtonCB(ui->tmgrid, tb_explorer_cb, eui);
    ui->tminout = CreatePanelChoice(rc, "Pointing:",
                                   "In", "Out", "Both", NULL);
    AddOptionChoiceCB(ui->tminout, oc_explorer_cb, eui);
    ui->tmlen = CreateSpinChoice(rc, "Tick length",
        4, SPIN_TYPE_FLOAT, 0.0, 100.0, 0.25);
    AddSpinChoiceCB(ui->tmlen, sp_explorer_cb, eui);
    ui->tmgridcol = CreateColorChoice(rc, "Color:");
    AddOptionChoiceCB(ui->tmgridcol, oc_explorer_cb, eui);
    ui->tmgridlinew = CreateLineWidthChoice(rc, "Line width:");
    AddSpinChoiceCB(ui->tmgridlinew, sp_explorer_cb, eui);
    ui->tmgridlines = CreateLineStyleChoice(rc, "Line style:");
    AddOptionChoiceCB(ui->tmgridlines, oc_explorer_cb, eui);


    ui->special_tp = CreateTabPage(tab, "Special");

    opitems[0].value = TICKS_SPEC_NONE;
    opitems[0].label = "None";
    opitems[1].value = TICKS_SPEC_MARKS;
    opitems[1].label = "Tick marks";
    opitems[2].value = TICKS_SPEC_BOTH;
    opitems[2].label = "Tick marks and labels";
    ui->specticks = CreateOptionChoice(ui->special_tp, "Special ticks:", 0, 3, opitems);
    AddOptionChoiceCB(ui->specticks, oc_explorer_cb, eui);

    ui->nspec = CreateSpinChoice(ui->special_tp, "Number of user ticks to use:",
        3, SPIN_TYPE_INT, 0.0, (double) MAX_TICKS, 1.0);
    AddSpinChoiceCB(ui->nspec, sp_explorer_cb, eui);
    CreateLabel(ui->special_tp, "Tick location - Label:");

    sw = XtVaCreateManagedWidget("sw",
                                 xmScrolledWindowWidgetClass, ui->special_tp,
				 XmNheight, 240,
                                 XmNscrollingPolicy, XmAUTOMATIC,
                                 NULL);
    rc = CreateVContainer(sw);

    for (i = 0; i < MAX_TICKS; i++) {
        rc3 = CreateHContainer(rc);
        sprintf(buf, "%2d", i);
        ui->specloc[i]   = CreateTextItem4(rc3, 12, buf);
        AddTextItemCB(ui->specloc[i], titem_explorer_cb, eui);
        ui->speclabel[i] = CreateTextItem4(rc3, 30, "");
        AddTextItemCB(ui->speclabel[i], titem_explorer_cb, eui);
    }

    SelectTabPage(tab, ui->main_tp);
    
    ui->top = tab;
    
    return ui;
}

/*
 * Fill 'Axes' dialog with values
 */
void update_axis_ui(AxisUI *ui, Quark *q)
{
    tickmarks *t = axis_get_data(q);

    if (t && ui) {
        char buf[128];
        int i;

        SetToggleButtonState(ui->active, is_axis_active(t));
        
        SetOptionChoice(ui->type, t->type);

        SetToggleButtonState(ui->zero, is_zero_axis(t));

        sprintf(buf, "%.2f", t->offsx);
        xv_setstr(ui->offx, buf);
        sprintf(buf, "%.2f", t->offsy);
        xv_setstr(ui->offy, buf);

        SetOptionChoice(ui->labellayout, t->label_layout == LAYOUT_PERPENDICULAR ? 1 : 0);
        SetOptionChoice(ui->labelplace, t->label_place);
        sprintf(buf, "%.2f", t->label_offset.x);
        xv_setstr(ui->labelspec_para, buf);
        sprintf(buf, "%.2f", t->label_offset.y);
        xv_setstr(ui->labelspec_perp, buf);
        SetSensitive(ui->labelspec_rc, t->label_place == TYPE_SPEC);
        SetOptionChoice(ui->labelfont, t->label_tprops.font);
        SetOptionChoice(ui->labelcolor, t->label_tprops.color);
        SetSpinChoice(ui->labelcharsize, t->label_tprops.charsize);
        SetOptionChoice(ui->labelop, t->label_op);

        SetToggleButtonState(ui->tlonoff, t->tl_flag);
        SetToggleButtonState(ui->tonoff, t->t_flag);
        SetToggleButtonState(ui->baronoff, t->t_drawbar);
        SetTextString(ui->label, t->label);

        if (is_log_axis(q)) {
            if (t->tmajor <= 1.0) {
                t->tmajor = 10.0;
            }
            sprintf(buf, "%g", t->tmajor);	    
        } else if (is_logit_axis(q)) {
	    if (t->tmajor <= 0.0) {
                t->tmajor = 0.1;
            }
	    else if (t->tmajor >= 0.5) {
                t->tmajor = 0.4;
	    }
            sprintf(buf, "%g", t->tmajor);
        } else if (t->tmajor > 0) {
            sprintf(buf, "%g", t->tmajor);
        } else {
            strcpy(buf, "UNDEFINED");
        }
        xv_setstr(ui->tmajor, buf);
 
        SetSpinChoice(ui->nminor, t->nminor);

        SetOptionChoice(ui->tlfont, t->tl_tprops.font);
        SetOptionChoice(ui->tlcolor, t->tl_tprops.color);
        SetOptionChoice(ui->tlskip, t->tl_skip);
        SetOptionChoice(ui->tlstagger, t->tl_staggered);
        xv_setstr(ui->tlappstr, t->tl_appstr);
        xv_setstr(ui->tlprestr, t->tl_prestr);
        SetOptionChoice(ui->tlstarttype, t->tl_starttype == TYPE_SPEC);
        if (t->tl_starttype == TYPE_SPEC) {
            sprintf(buf, "%f", t->tl_start);
            xv_setstr(ui->tlstart, buf);
            sprintf(buf, "%f", t->tl_stop);
            xv_setstr(ui->tlstop, buf);
        }
        SetOptionChoice(ui->tlstoptype, t->tl_stoptype == TYPE_SPEC);
        if (t->tl_stoptype == TYPE_SPEC) {
            sprintf(buf, "%f", t->tl_stop);
            xv_setstr(ui->tlstop, buf);
        }
        SetOptionChoice(ui->tlform, t->tl_format);
        SetOptionChoice(ui->ticklop, t->tl_op);
        SetTextString(ui->tlformula, t->tl_formula);
        SetOptionChoice(ui->tlprec, t->tl_prec);

        SetOptionChoice(ui->tlgaptype, t->tl_gaptype);
        sprintf(buf, "%.2f", t->tl_gap.x);
        xv_setstr(ui->tlgap_para, buf);
        sprintf(buf, "%.2f", t->tl_gap.y);
        xv_setstr(ui->tlgap_perp, buf);
        SetSensitive(ui->tlgap_rc, t->tl_gaptype == TYPE_SPEC);

        SetSpinChoice(ui->tlcharsize, t->tl_tprops.charsize);
        SetAngleChoice(ui->tlangle, t->tl_tprops.angle);

        
        SetOptionChoice(ui->tickop, t->t_op);
        
        SetOptionChoice(ui->autonum, t->t_autonum - 2);

        SetToggleButtonState(ui->tround, t->t_round);

        SetToggleButtonState(ui->tgrid, t->props.gridflag);
        SetOptionChoice(ui->tinout, t->props.inout);
        SetSpinChoice(ui->tlen, t->props.size);
        SetOptionChoice(ui->tgridcol, t->props.color);
        SetSpinChoice(ui->tgridlinew, t->props.linew);
        SetOptionChoice(ui->tgridlines, t->props.lines);
        
        SetToggleButtonState(ui->tmgrid, t->mprops.gridflag);
        SetOptionChoice(ui->tminout, t->mprops.inout);
        SetOptionChoice(ui->tmgridcol, t->mprops.color);
        SetSpinChoice(ui->tmgridlinew, t->mprops.linew);
        SetOptionChoice(ui->tmgridlines, t->mprops.lines);
        SetSpinChoice(ui->tmlen, t->mprops.size);

        SetOptionChoice(ui->barcolor, t->t_drawbarcolor);
        SetSpinChoice(ui->barlinew, t->t_drawbarlinew);
        SetOptionChoice(ui->barlines, t->t_drawbarlines);

        SetOptionChoice(ui->specticks, t->t_spec);
        SetSpinChoice(ui->nspec, t->nticks);
        for (i = 0; i < t->nticks; i++) {
            sprintf(buf, "%.9g", t->tloc[i].wtpos);
            xv_setstr(ui->specloc[i], buf);
            if (t->tloc[i].type == TICK_TYPE_MAJOR) {
                xv_setstr(ui->speclabel[i], t->tloc[i].label);
            } else {
                xv_setstr(ui->speclabel[i], "");
            }
        }
    }
}


int set_axis_data(AxisUI *ui, Quark *q, void *caller)
{
    tickmarks *t = axis_get_data(q);

    if (t && ui) {
        int i;
        
        if (!caller || caller == ui->active) {
            t->active = GetToggleButtonState(ui->active);
        }

        if (!caller || caller == ui->type) {
            t->type = GetOptionChoice(ui->type);
        }

        if (!caller || caller == ui->label) {
            char *s = GetTextString(ui->label);
            t->label = copy_string(t->label, s);
            xfree(s);
        }
        if (!caller || caller == ui->tmajor) {
            if (xv_evalexpr(ui->tmajor, &t->tmajor) != RETURN_SUCCESS) {
                errmsg("Specify major tick spacing");
                return RETURN_FAILURE;
            }
        }
        if (!caller || caller == ui->nminor) {
            t->nminor = (int) GetSpinChoice(ui->nminor);
        }
        if (!caller || caller == ui->tlform) {
            t->tl_format = GetOptionChoice(ui->tlform);
        }
        if (!caller || caller == ui->tlprec) {
            t->tl_prec = GetOptionChoice(ui->tlprec);
        }
        if (!caller || caller == ui->tlonoff) {
            t->tl_flag = GetToggleButtonState(ui->tlonoff);
        }
        if (!caller || caller == ui->baronoff) {
            t->t_drawbar = GetToggleButtonState(ui->baronoff);
        }
        if (!caller || caller == ui->tonoff) {
            t->t_flag = GetToggleButtonState(ui->tonoff);
        }
        if (!caller || caller == ui->zero) {
            t->zero = GetToggleButtonState(ui->zero);
        }
        if (!caller || caller == ui->offx) {
            xv_evalexpr(ui->offx, &t->offsx);
        }
        if (!caller || caller == ui->offy) {
            xv_evalexpr(ui->offy, &t->offsy);
        }
        if (!caller || caller == ui->tlfont) {
            t->tl_tprops.font = GetOptionChoice(ui->tlfont);
        }
        if (!caller || caller == ui->tlcolor) {
            t->tl_tprops.color = GetOptionChoice(ui->tlcolor);
        }
        if (!caller || caller == ui->labelfont) {
            t->label_tprops.font = GetOptionChoice(ui->labelfont);
        }
        if (!caller || caller == ui->labelcolor) {
            t->label_tprops.color = GetOptionChoice(ui->labelcolor);
        }
        if (!caller || caller == ui->labelcharsize) {
            t->label_tprops.charsize = GetSpinChoice(ui->labelcharsize);
        }
        if (!caller || caller == ui->labellayout) {
            t->label_layout = GetOptionChoice(ui->labellayout) ?
                LAYOUT_PERPENDICULAR:LAYOUT_PARALLEL;
        }
        if (!caller || caller == ui->labelop) {
            t->label_op = GetOptionChoice(ui->labelop);
        }
        if (!caller || caller == ui->labelplace) {
            t->label_place = GetOptionChoice(ui->labelplace);
        }
        if (!caller || caller == ui->labelspec_para) {
            xv_evalexpr(ui->labelspec_para, &t->label_offset.x);
        }
        if (!caller || caller == ui->labelspec_perp) {
            xv_evalexpr(ui->labelspec_perp, &t->label_offset.y);
        }
        if (!caller || caller == ui->barcolor) {
            t->t_drawbarcolor = GetOptionChoice(ui->barcolor);
        }
        if (!caller || caller == ui->barlinew) {
            t->t_drawbarlinew = GetSpinChoice(ui->barlinew);
        }
        if (!caller || caller == ui->barlines) {
            t->t_drawbarlines = GetOptionChoice(ui->barlines);
        }
        if (!caller || caller == ui->tlcharsize) {
            t->tl_tprops.charsize = GetSpinChoice(ui->tlcharsize);
        }
        if (!caller || caller == ui->tlangle) {
            t->tl_tprops.angle = GetAngleChoice(ui->tlangle);
        }
        if (!caller || caller == ui->ticklop) {
            t->tl_op = GetOptionChoice(ui->ticklop);
        }
        if (!caller || caller == ui->tlstagger) {
            t->tl_staggered = GetOptionChoice(ui->tlstagger);
        }
        if (!caller || caller == ui->tlstarttype) {
            t->tl_starttype = GetOptionChoice(ui->tlstarttype) == 0 ?
                TYPE_AUTO : TYPE_SPEC;
        }
        if (!caller || caller == ui->tlstart) {
            if (t->tl_starttype == TYPE_SPEC) {
                if (xv_evalexpr(ui->tlstart, &t->tl_start) != RETURN_SUCCESS) {
                errmsg("Specify tick label start");
                    return RETURN_FAILURE;
                }
            }
        }
        if (!caller || caller == ui->tlstoptype) {
            t->tl_stoptype = GetOptionChoice(ui->tlstoptype) == 0 ?
                TYPE_AUTO : TYPE_SPEC;
        }
        if (!caller || caller == ui->tlstop) {
            if (t->tl_stoptype == TYPE_SPEC) {
                if (xv_evalexpr(ui->tlstop, &t->tl_stop) != RETURN_SUCCESS) {
                    errmsg("Specify tick label stop");
                    return RETURN_FAILURE;
                }
            }
        }
        if (!caller || caller == ui->tlskip) {
            t->tl_skip = GetOptionChoice(ui->tlskip);
        }
        if (!caller || caller == ui->tlformula) {
            xfree(t->tl_formula);
            t->tl_formula = GetTextString(ui->tlformula);
        }
        if (!caller || caller == ui->tlprestr) {
            strcpy(t->tl_prestr, xv_getstr(ui->tlprestr));
        }
        if (!caller || caller == ui->tlappstr) {
            strcpy(t->tl_appstr, xv_getstr(ui->tlappstr));
        }
        if (!caller || caller == ui->tlgaptype) {
            t->tl_gaptype = GetOptionChoice(ui->tlgaptype);
        }
        if (!caller || caller == ui->tlgap_para) {
            xv_evalexpr(ui->tlgap_para, &t->tl_gap.x);
        }
        if (!caller || caller == ui->tlgap_perp) {
            xv_evalexpr(ui->tlgap_perp, &t->tl_gap.y);
        }
        if (!caller || caller == ui->tickop) {
            t->t_op = GetOptionChoice(ui->tickop);
        }
        if (!caller || caller == ui->tround) {
            t->t_round = GetToggleButtonState(ui->tround);
        }
        if (!caller || caller == ui->autonum) {
            t->t_autonum = GetOptionChoice(ui->autonum) + 2;
        }

        if (!caller || caller == ui->tgrid) {
            t->props.gridflag = GetToggleButtonState(ui->tgrid);
        }
        if (!caller || caller == ui->tinout) {
            t->props.inout = GetOptionChoice(ui->tinout);
        }
        if (!caller || caller == ui->tlen) {
            t->props.size = GetSpinChoice(ui->tlen);
        }
        if (!caller || caller == ui->tgridcol) {
            t->props.color = GetOptionChoice(ui->tgridcol);
        }
        if (!caller || caller == ui->tgridlinew) {
            t->props.linew = GetSpinChoice(ui->tgridlinew);
        }
        if (!caller || caller == ui->tgridlines) {
            t->props.lines = GetOptionChoice(ui->tgridlines);
        }
        if (!caller || caller == ui->tmgrid) {
            t->mprops.gridflag = GetToggleButtonState(ui->tmgrid);
        }
        if (!caller || caller == ui->tminout) {
            t->mprops.inout = GetOptionChoice(ui->tminout);
        }
        if (!caller || caller == ui->tmlen) {
            t->mprops.size = GetSpinChoice(ui->tmlen);
        }
        if (!caller || caller == ui->tmgridcol) {
            t->mprops.color = GetOptionChoice(ui->tmgridcol);
        }
        if (!caller || caller == ui->tmgridlinew) {
            t->mprops.linew = GetSpinChoice(ui->tmgridlinew);
        }
        if (!caller || caller == ui->tmgridlines) {
            t->mprops.lines = GetOptionChoice(ui->tmgridlines);
        }
        if (!caller ||
            caller == ui->specticks || caller == ui->nspec || caller == ui->specloc) {
            t->t_spec = GetOptionChoice(ui->specticks);
            /* only read special info if special ticks used */
            if (t->t_spec != TICKS_SPEC_NONE) {
                t->nticks = (int) GetSpinChoice(ui->nspec);
                /* ensure that enough tick positions have been specified */
                for (i = 0; i < t->nticks; i++) {
                    if (xv_evalexpr(ui->specloc[i], &t->tloc[i].wtpos) ==
                                                        RETURN_SUCCESS) {
                        char *cp;
                        cp = xv_getstr(ui->speclabel[i]);
                        if (cp[0] == '\0') {
                            t->tloc[i].type = TICK_TYPE_MINOR;
                        } else {
                            t->tloc[i].type = TICK_TYPE_MAJOR;
                        }
                        if (t->t_spec == TICKS_SPEC_BOTH) {
                            t->tloc[i].label =
                                copy_string(t->tloc[i].label, cp);
                        } else {
                            t->tloc[i].label = 
                                copy_string(t->tloc[i].label, NULL);
                        }
                    }
                } 
            }
        }
        
        quark_dirtystate_set(q, TRUE);
        
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

static void auto_spec_cb(OptionStructure *opt, int value, void *data)
{
    Widget rc = (Widget) data;
    SetSensitive(rc, value);
}

void update_ticks(Quark *gr)
{
}
