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

AGridUI *create_axisgrid_ui(ExplorerUI *eui)
{
    AGridUI *ui;
    int i;
    OptionItem opitems[3];
    char buf[32];
    Widget tab, rc, rc2, rc3, fr;

    ui = xmalloc(sizeof(AGridUI));

    /* ------------ Tabs --------------*/
    tab = CreateTab(eui->scrolled_window); 
    AddHelpCB(tab, "doc/UsersGuide.html#axisgrid-properties");

    ui->main_tp = CreateTabPage(tab, "Grid");

    rc = CreateHContainer(ui->main_tp);
    opitems[0].value = AXIS_TYPE_X;
    opitems[0].label = "X";
    opitems[1].value = AXIS_TYPE_Y;
    opitems[1].label = "Y";
    ui->type = CreateOptionChoice(rc, "Type:", 0, 2, opitems);
    AddOptionChoiceCB(ui->type, oc_explorer_cb, eui);

    fr = CreateFrame(ui->main_tp, "Spacing");
    rc = CreateVContainer(fr);

    rc2 = CreateHContainer(rc);
    ui->tmajor = CreateTextItem(rc2, 8, "Major spacing:");
    AddTextItemCB(ui->tmajor, titem_explorer_cb, eui);
    ui->nminor = CreateSpinChoice(rc2, "Minor ticks:",
        2, SPIN_TYPE_INT, 0.0, (double) MAX_TICKS - 1, 1.0);
    AddSpinChoiceCB(ui->nminor, sp_explorer_cb, eui);

    rc2 = CreateHContainer(rc);
    ui->tround = CreateToggleButton(rc2, "Place at rounded positions");
    AddToggleButtonCB(ui->tround, tb_explorer_cb, eui);
    ui->autonum = CreatePanelChoice(rc2, "Autotick divisions:",
        "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", NULL);
    AddOptionChoiceCB(ui->autonum, oc_explorer_cb, eui);


    ui->label_tp = CreateTabPage(tab, "Axis bar");

    fr = CreateFrame(ui->label_tp, "Bar properties");
    rc = CreateVContainer(fr);

    rc2 = CreateHContainer(rc);
    ui->barcolor = CreateColorChoice(rc2, "Color:");
    AddOptionChoiceCB(ui->barcolor, oc_explorer_cb, eui);
    ui->barlinew = CreateLineWidthChoice(rc2, "Width:");
    AddSpinChoiceCB(ui->barlinew, sp_explorer_cb, eui);

    ui->barlines = CreateLineStyleChoice(rc, "Line style:");
    AddOptionChoiceCB(ui->barlines, oc_explorer_cb, eui);


    ui->tickmark_tp = CreateTabPage(tab, "Tick marks");

    rc2 = CreateHContainer(ui->tickmark_tp);

    /* major tick marks */
    fr = CreateFrame(rc2, "Major ticks");
    rc = CreateVContainer(fr);
    ui->tgrid = CreateToggleButton(rc, "Draw grid lines");
    AddToggleButtonCB(ui->tgrid, tb_explorer_cb, eui);
    ui->tinout = CreateOptionChoiceVA(rc, "Pointing:",
        "In",   TICKS_IN,
        "Out",  TICKS_OUT,
        "Both", TICKS_BOTH,
        NULL);
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
    ui->tminout = CreateOptionChoiceVA(rc, "Pointing:",
        "In",   TICKS_IN,
        "Out",  TICKS_OUT,
        "Both", TICKS_BOTH,
        NULL);
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


    ui->ticklabel_tp = CreateTabPage(tab, "Tick labels");

    fr = CreateFrame(ui->ticklabel_tp, "Formatting");
    rc2 = CreateVContainer(fr);
    rc = CreateHContainer(rc2);
    ui->tlcharsize = CreateCharSizeChoice(rc, "Char size");
    AddSpinChoiceCB(ui->tlcharsize, sp_explorer_cb, eui);

    rc = CreateHContainer(rc2);
    ui->tlfont = CreateFontChoice(rc, "Font:");
    AddOptionChoiceCB(ui->tlfont, oc_explorer_cb, eui);
    ui->tlcolor = CreateColorChoice(rc, "Color:");
    AddOptionChoiceCB(ui->tlcolor, oc_explorer_cb, eui);

    ui->tlangle = CreateAngleChoice(rc2, "Angle");
    AddScaleCB(ui->tlangle, scale_explorer_cb, eui);

    rc = CreateHContainer(rc2);
    ui->tlform = CreateFormatChoice(rc, "Format:");
    AddOptionChoiceCB(ui->tlform , oc_explorer_cb, eui);
    ui->tlprec = CreatePrecisionChoice(rc, "Precision:");
    AddOptionChoiceCB(ui->tlprec, oc_explorer_cb, eui);


    fr = CreateFrame(ui->ticklabel_tp, "Placement");

    rc2 = CreateVContainer(fr);
    rc3 = CreateHContainer(rc2);
    ui->tlstarttype = CreatePanelChoice(rc3, "Start at:",
                                    "Axis min", "Specified:", NULL);
    AddOptionChoiceCB(ui->tlstarttype, oc_explorer_cb, eui);
    ui->tlstart = CreateTextItem(rc3, 8, "");
    AddTextItemCB(ui->tlstart, titem_explorer_cb, eui);

    rc3 = CreateHContainer(rc2);
    ui->tlstoptype = CreatePanelChoice(rc3, "Stop at:",
                                   "Axis max", "Specified:", NULL);
    AddOptionChoiceCB(ui->tlstoptype, oc_explorer_cb, eui);
    ui->tlstop = CreateTextItem(rc3, 8, "");
    AddTextItemCB(ui->tlstop, titem_explorer_cb, eui);

    fr = CreateFrame(ui->ticklabel_tp, "Extra");
    rc = CreateVContainer(fr);

    opitems[0].value = TYPE_AUTO;
    opitems[0].label = "Auto";
    opitems[1].value = TYPE_SPEC;
    opitems[1].label = "Specified";
    rc2 = CreateHContainer(rc);
    ui->tlgaptype = CreateOptionChoice(rc2, "Location:", 0, 2, opitems);

    rc2 = CreateHContainer(rc);
    ui->tlskip = CreatePanelChoice(rc2, "Skip every:",
        "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", NULL);
    AddOptionChoiceCB(ui->tlskip, oc_explorer_cb, eui);
    ui->tlstagger = CreatePanelChoice(rc2, "Stagger:",
        "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", NULL);
    AddOptionChoiceCB(ui->tlstagger, oc_explorer_cb, eui);


    ui->tlformula = CreateTextInput(rc, "Axis transform:");
    AddTextInputCB(ui->tlformula, text_explorer_cb, eui);

    rc2 = CreateHContainer(rc);
    ui->tlprestr = CreateTextInput(rc2, "Prepend:");
    SetTextInputLength(ui->tlprestr, 13);
    AddTextInputCB(ui->tlprestr, text_explorer_cb, eui);
    ui->tlappstr = CreateTextInput(rc2, "Append:");
    SetTextInputLength(ui->tlappstr, 13);
    AddTextInputCB(ui->tlappstr, text_explorer_cb, eui);

    ui->tlgap_rc = CreateHContainer(rc);
    AddOptionChoiceCB(ui->tlgaptype, auto_spec_cb, ui->tlgap_rc);
    AddOptionChoiceCB(ui->tlgaptype, oc_explorer_cb, eui);
    ui->tlgap_para = CreateTextItem(ui->tlgap_rc, 5, "Parallel offset:");
    AddTextItemCB(ui->tlgap_para, titem_explorer_cb, eui);
    ui->tlgap_perp = CreateTextItem(ui->tlgap_rc, 5, "Perpendicular offset:");
    AddTextItemCB(ui->tlgap_perp, titem_explorer_cb, eui);


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

    ui->sw = XtVaCreateManagedWidget("ui->sw",
                                 xmScrolledWindowWidgetClass, ui->special_tp,
				 XmNheight, 320,
                                 XmNscrollingPolicy, XmAUTOMATIC,
                                 NULL);
    rc = CreateVContainer(ui->sw);

    for (i = 0; i < MAX_TICKS; i++) {
        rc3 = CreateHContainer(rc);
        sprintf(buf, "%2d", i);
        ui->specloc[i]   = CreateTextItem(rc3, 12, buf);
        AddTextItemCB(ui->specloc[i], titem_explorer_cb, eui);
        ui->speclabel[i] = CreateTextItem(rc3, 30, "");
        AddTextItemCB(ui->speclabel[i], titem_explorer_cb, eui);
    }

    SelectTabPage(tab, ui->main_tp);
    
    ui->top = tab;
    
    return ui;
}

/*
 * Fill 'Axes' dialog with values
 */
void update_axisgrid_ui(AGridUI *ui, Quark *q)
{
    tickmarks *t = axisgrid_get_data(q);

    if (t && ui) {
        char buf[128];
        int i;
        Widget vbar;

        SetOptionChoice(ui->type, t->type);

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
        SetTextString(ui->tlappstr, t->tl_appstr);
        SetTextString(ui->tlprestr, t->tl_prestr);
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


        /* set reasonable scrolling */
        vbar = XtNameToWidget(ui->sw, "VertScrollBar");
        if (vbar) {
            int maxval;
            XtVaGetValues(vbar, XmNmaximum, &maxval, NULL);
            XtVaSetValues(vbar, XmNincrement, (int) rint(maxval/MAX_TICKS), NULL);
        }
    }
}


int set_axisgrid_data(AGridUI *ui, Quark *q, void *caller)
{
    tickmarks *t = axisgrid_get_data(q);

    if (t && ui) {
        int i;
        
        if (!caller || caller == ui->type) {
            t->type = GetOptionChoice(ui->type);
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
        if (!caller || caller == ui->tlfont) {
            t->tl_tprops.font = GetOptionChoice(ui->tlfont);
        }
        if (!caller || caller == ui->tlcolor) {
            t->tl_tprops.color = GetOptionChoice(ui->tlcolor);
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
            char *s = GetTextString(ui->tlprestr);
            t->tl_prestr = copy_string(t->tl_prestr, s);
        }
        if (!caller || caller == ui->tlappstr) {
            char *s = GetTextString(ui->tlappstr);
            t->tl_appstr = copy_string(t->tl_appstr, s);
            xfree(s);
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

AxisUI *create_axis_ui(ExplorerUI *eui)
{
    AxisUI *ui;
    Widget rc, fr;

    ui = xmalloc(sizeof(AxisUI));

    ui->top = CreateVContainer(eui->scrolled_window);
    AddHelpCB(ui->top, "doc/UsersGuide.html#axis-properties");

    fr = CreateFrame(ui->top, "Position");
    rc = CreateHContainer(fr);
    ui->position = CreateOptionChoiceVA(rc, "Placement:",
        "Normal",   AXIS_POS_NORMAL,
        "Opposite", AXIS_POS_OPPOSITE,
        "Zero",     AXIS_POS_ZERO,
        NULL);
    AddOptionChoiceCB(ui->position, oc_explorer_cb, eui);

    ui->offset = CreateSpinChoice(rc, "Offset:",
        4, SPIN_TYPE_FLOAT, -1.0, 1.0, 0.01);
    AddSpinChoiceCB(ui->offset, sp_explorer_cb, eui);
    
    fr = CreateFrame(ui->top, "Options");
    rc = CreateVContainer(fr);
    ui->draw_bar    = CreateToggleButton(rc, "Draw axis bar");
    AddToggleButtonCB(ui->draw_bar, tb_explorer_cb, eui);
    ui->draw_ticks  = CreateToggleButton(rc, "Draw tick marks");
    AddToggleButtonCB(ui->draw_ticks, tb_explorer_cb, eui);
    ui->draw_labels = CreateToggleButton(rc, "Draw tick labels");
    AddToggleButtonCB(ui->draw_labels, tb_explorer_cb, eui);
    
    return ui;
}

void update_axis_ui(AxisUI *ui, Quark *q)
{
    if (ui && q && quark_fid_get(q) == QFlavorAxis) {
        SetOptionChoice(ui->position, axis_get_position(q));
        SetSpinChoice(ui->offset, axis_get_offset(q));
        
        SetToggleButtonState(ui->draw_bar,    axis_bar_enabled(q));
        SetToggleButtonState(ui->draw_ticks,  axis_ticks_enabled(q));
        SetToggleButtonState(ui->draw_labels, axis_labels_enabled(q));
    }
}

int set_axis_data(AxisUI *ui, Quark *q, void *caller)
{
    if (ui && q && quark_fid_get(q) == QFlavorAxis) {
        if (!caller || caller == ui->position) {
            axis_set_position(q, GetOptionChoice(ui->position));
        }
        if (!caller || caller == ui->offset) {
            axis_set_offset(q, GetSpinChoice(ui->offset));
        }

        if (!caller || caller == ui->draw_bar) {
            axis_enable_bar(q, GetToggleButtonState(ui->draw_bar));
        }
        if (!caller || caller == ui->draw_ticks) {
            axis_enable_ticks(q, GetToggleButtonState(ui->draw_ticks));
        }
        if (!caller || caller == ui->draw_labels) {
            axis_enable_labels(q, GetToggleButtonState(ui->draw_labels));
        }

        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}
