/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c)1996-2003 Grace Development Team
 * 
 * Maintained by Evgeny Stambulchik <fnevgeny@plasma-gate.weizmann.ac.il>
 * 
 * 
 *                           All Rights Reserved
 * 
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option)any later version.
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
 * Set appearance
 *
 */

#include <config.h>

#include <stdlib.h>

#include "core_utils.h"
#include "explorer.h"

#if 0

#define SETAPP_STRIP_LEGENDS    0
#define SETAPP_LOAD_COMMENTS    1
#define SETAPP_ALL_COLORS       2
#define SETAPP_ALL_SYMBOLS      3
#define SETAPP_ALL_LINEW        4
#define SETAPP_ALL_LINES        5
#define SETAPP_ALL_BW           6

static void setapp_data_proc(Widget but, void *data)
{
    ExplorerUI *eui = (ExplorerUI *eui) data;
    
    int c = 0, bg = getbgcolor(grace->rt->canvas);
    
    for(i = 0; i < cd; i++) {
        pset = selset[i];
        if (!pset) {
            return;
        }
        p = pset->data;
        switch (proc_type) {
        case SETAPP_STRIP_LEGENDS:
            set_set_legstr(pset,
                mybasename(set_get_legstr(pset)));
            break;
        case SETAPP_LOAD_COMMENTS:
            load_comments_to_legend(pset);
            break;
        case SETAPP_ALL_COLORS:
            while (c == bg ||
                   get_colortype(grace->rt->canvas, c) != COLOR_MAIN) {
                c++;
                c %= number_of_colors(grace->rt->canvas);
            }
            setcolors(pset, c);
            c++;
            break;
        case SETAPP_ALL_SYMBOLS:
            p->sym.type = (i % (MAXSYM - 2))+ 1;
            break;
        case SETAPP_ALL_LINEW:
            p->line.line.width = ((i % (2*((int)MAX_LINEWIDTH)- 1))+ 1)/2.0;
            break;
        case SETAPP_ALL_LINES:
            p->line.line.style = (i % (number_of_linestyles(grace->rt->canvas)- 1))
                + 1;
            break;
        case SETAPP_ALL_BW:
            setcolors(pset, 1);
            break;
        }
    }

    UpdateSymbols(cset);
    xdrawgraph(grace->project, FALSE);
}
#endif

/*
 * create the symbols popup
 */
SetUI *create_set_ui(ExplorerUI *eui)
{
    SetUI *ui;
    Widget tab, fr, rc, rc1, rc2;

    ui = xmalloc(sizeof(SetUI));

#if 0
    CreateMenuSeparator(eui->editmenu);
    CreateMenuButton(eui->editmenu,
        "Set different colors", 'c', setapp_data_proc, eui);
    CreateMenuButton(eui->editmenu,
        "Set different symbols", 's', setapp_data_proc, eui);
    CreateMenuButton(eui->editmenu,
        "Set different line widths", 'w', setapp_data_proc, eui);
    CreateMenuButton(eui->editmenu,
        "Set different line styles", 'y', setapp_data_proc, eui);
    CreateMenuButton(eui->editmenu,
        "Set black & white", 'B', setapp_data_proc, eui);
    CreateMenuSeparator(eui->editmenu);
    CreateMenuButton(eui->editmenu,
        "Load comments", 'm', setapp_data_proc, eui);
    CreateMenuButton(eui->editmenu,
        "Strip legends", 'l', setapp_data_proc, eui);
#endif

    /* ------------ Tabs -------------- */

    tab = CreateTab(eui->scrolled_window);        


    /* ------------ Main tab -------------- */

    ui->main_tp = CreateTabPage(tab, "Main");

    rc = CreateHContainer(ui->main_tp);
    ui->active = CreateToggleButton(rc, "Active");
    AddToggleButtonCB(ui->active, tb_explorer_cb, eui);

    fr = CreateFrame(ui->main_tp, "Set presentation");
    ui->type = CreateSetTypeChoice(fr, "Type:");
    AddOptionChoiceCB(ui->type, oc_explorer_cb, eui);

    rc2 = CreateHContainer(ui->main_tp);

    fr = CreateFrame(rc2, "Symbol properties");
    rc = CreateVContainer(fr);
    ui->symbols = CreatePanelChoice(rc,
                                             "Type:",
                                             13,
                                             "None",            /* 0 */
                                             "Circle",          /* 1 */
                                             "Square",          /* 2 */
                                             "Diamond",         /* 3 */
                                             "Triangle up",     /* 4 */
                                             "Triangle left",   /* 5 */
                                             "Triangle down",   /* 6 */
                                             "Triangle right",  /* 7 */
                                             "Plus",            /* 8 */
                                             "X",               /* 9 */
                                             "Star",            /* 10 */
                                             "Char",            /* 11 */
                                             NULL);
    AddOptionChoiceCB(ui->symbols, oc_explorer_cb, eui);
    ui->symsize = CreateCharSizeChoice(rc, "Size");
    AddScaleCB(ui->symsize, scale_explorer_cb, eui);
    ui->symcolor = CreateColorChoice(rc, "Color:");
    AddOptionChoiceCB(ui->symcolor, oc_explorer_cb, eui);
    ui->symchar = CreateTextItem2(rc, 3, "Symbol char:");
    AddTextItemCB(ui->symchar, titem_explorer_cb, eui);

    fr = CreateFrame(rc2, "Line properties");
    rc = CreateVContainer(fr);
    ui->linet = CreatePanelChoice(rc, "Type:",
                                          7,
                                          "None",
                                          "Straight",
                                          "Left stairs",
                                          "Right stairs",
                                          "Segments",
                                          "3-Segments",
                                          NULL);
    AddOptionChoiceCB(ui->linet, oc_explorer_cb, eui); 
    ui->lines = CreateLineStyleChoice(rc, "Style:");
    AddOptionChoiceCB(ui->lines, oc_explorer_cb, eui);
    ui->width = CreateLineWidthChoice(rc, "Width:");
    AddSpinButtonCB(ui->width, sp_explorer_cb, eui); 
    ui->color = CreateColorChoice(rc, "Color:");
    AddOptionChoiceCB(ui->color, oc_explorer_cb, eui);

    fr = CreateFrame(ui->main_tp, "Legend");
    ui->legend_str = CreateCSText(fr, "String:");
    AddTextInputCB(ui->legend_str, text_explorer_cb, eui);

    fr = CreateFrame(ui->main_tp, "Display options");
    rc2 = CreateHContainer(fr);
    ui->avalue_active = CreateToggleButton(rc2, "Annotate values");
    AddToggleButtonCB(ui->avalue_active, tb_explorer_cb, eui);
    ui->errbar_active = CreateToggleButton(rc2, "Display error bars");
    AddToggleButtonCB(ui->errbar_active, tb_explorer_cb, eui);


    /* ------------ Symbols tab -------------- */

    ui->symbol_tp = CreateTabPage(tab, "Symbols");

    fr = CreateFrame(ui->symbol_tp, "Symbol outline");
    rc = CreateVContainer(fr);

    rc2 = CreateHContainer(rc);
    ui->symlines = CreateLineStyleChoice(rc2, "Style:");
    AddOptionChoiceCB(ui->symlines, oc_explorer_cb, eui);
    ui->symlinew = CreateLineWidthChoice(rc2, "Width:");
    AddSpinButtonCB(ui->symlinew, sp_explorer_cb, eui);
    ui->sympattern = CreatePatternChoice(rc, "Pattern:");
    AddOptionChoiceCB(ui->sympattern, oc_explorer_cb, eui);

    fr = CreateFrame(ui->symbol_tp, "Symbol fill");
    rc = CreateHContainer(fr);
    ui->symfillcolor = CreateColorChoice(rc, "Color:");
    AddOptionChoiceCB(ui->symfillcolor, oc_explorer_cb, eui);
    ui->symfillpattern = CreatePatternChoice(rc, "Pattern:");
    AddOptionChoiceCB(ui->symfillpattern, oc_explorer_cb, eui);

    fr = CreateFrame(ui->symbol_tp, "Extra");
    rc = CreateVContainer(fr);
    ui->symskip = CreateSpinChoice(rc, "Symbol skip:",
        5, SPIN_TYPE_INT, (double) 0, (double) 100000, (double) 1);
    AddSpinButtonCB(ui->symskip, sp_explorer_cb, eui);
    ui->char_font = CreateFontChoice(rc, "Font for char symbol:");
    AddOptionChoiceCB(ui->char_font, oc_explorer_cb, eui);


    /* ------------ Line tab -------------- */

    ui->line_tp = CreateTabPage(tab, "Line");

    fr = CreateFrame(ui->line_tp, "Line properties");
    rc = CreateHContainer(fr);
    ui->pattern = CreatePatternChoice(rc, "Pattern:");
    AddOptionChoiceCB(ui->pattern, oc_explorer_cb, eui);
    ui->dropline = CreateToggleButton(rc, "Draw drop lines");
    AddToggleButtonCB(ui->dropline, tb_explorer_cb, eui);

    fr = CreateFrame(ui->line_tp, "Fill properties");
    rc = CreateVContainer(fr);
    rc2 = CreateHContainer(rc);
    ui->filltype = CreatePanelChoice(rc2, "Type:",
                                         4,
                                         "None",
                                         "As polygon",
                                         "To baseline",
                                         NULL);
    AddOptionChoiceCB(ui->filltype, oc_explorer_cb, eui); 
    ui->fillrule = CreatePanelChoice(rc2, "Rule:",
                                         3,
                                         "Winding",
                                         "Even-Odd",
                                         NULL);
    AddOptionChoiceCB(ui->fillrule, oc_explorer_cb, eui); 
    rc2 = CreateHContainer(rc);
    ui->fillpat = CreatePatternChoice(rc2, "Pattern:");
    AddOptionChoiceCB(ui->fillpat, oc_explorer_cb, eui);
    ui->fillcol = CreateColorChoice(rc2, "Color:");
    AddOptionChoiceCB(ui->fillcol, oc_explorer_cb, eui);

    fr = CreateFrame(ui->line_tp, "Base line");
    rc = CreateHContainer(fr);
    ui->baselinetype = CreatePanelChoice(rc, "Type:",
                                         6,
                                         "Zero",
                                         "Set min",
                                         "Set max",
                                         "Graph min",
                                         "Graph max",
                                         NULL);
    AddOptionChoiceCB(ui->baselinetype, oc_explorer_cb, eui);
    ui->baseline = CreateToggleButton(rc, "Draw line");
    AddToggleButtonCB(ui->baseline, tb_explorer_cb, eui);


    /* ------------ AValue tab -------------- */

    ui->avalue_tp = CreateTabPage(tab, "Ann. values");

    fr = CreateFrame(ui->avalue_tp, "Text properties");
    rc = CreateVContainer(fr);

    rc2 = CreateHContainer(rc);
    ui->avalue_font = CreateFontChoice(rc2, "Font:");
    AddOptionChoiceCB(ui->avalue_font, oc_explorer_cb, eui);
    ui->avalue_charsize = CreateCharSizeChoice(rc2, "Char size");
    SetScaleWidth(ui->avalue_charsize, 120);
    AddScaleCB(ui->avalue_charsize, scale_explorer_cb, eui);

    rc2 = CreateHContainer(rc);
    ui->avalue_color = CreateColorChoice(rc2, "Color:");
    AddOptionChoiceCB(ui->avalue_color, oc_explorer_cb, eui);
    ui->avalue_angle = CreateAngleChoice(rc2, "Angle");
    SetScaleWidth(ui->avalue_angle, 180);
    AddScaleCB(ui->avalue_angle, scale_explorer_cb, eui);

    rc2 = CreateHContainer(rc);
    ui->avalue_prestr = CreateTextItem2(rc2, 10, "Prepend:");
    AddTextItemCB(ui->avalue_prestr, titem_explorer_cb, eui);
    ui->avalue_appstr = CreateTextItem2(rc2, 10, "Append:");
    AddTextItemCB(ui->avalue_appstr, titem_explorer_cb, eui);

    fr = CreateFrame(ui->avalue_tp, "Format options");
    rc = CreateVContainer(fr);
    rc2 = CreateHContainer(rc);
    ui->avalue_format = CreateFormatChoice(rc, "Format:");
    AddOptionChoiceCB(ui->avalue_format, oc_explorer_cb, eui);
    ui->avalue_type = CreatePanelChoice(rc2, "Type:",
                                         7,
                                         "None",
                                         "X",
                                         "Y",
                                         "X, Y",
                                         "String",
                                         "Z",
                                         NULL);
    AddOptionChoiceCB(ui->avalue_type, oc_explorer_cb, eui); 
    ui->avalue_precision = CreatePrecisionChoice(rc2, "Precision:");
    AddOptionChoiceCB(ui->avalue_precision, oc_explorer_cb, eui);

    fr = CreateFrame(ui->avalue_tp, "Placement");
    rc = CreateVContainer(fr);
    rc2 = CreateHContainer(rc);
    ui->avalue_offsetx = CreateTextItem2(rc2, 10, "X offset:");
    AddTextItemCB(ui->avalue_offsetx, titem_explorer_cb, eui);
    ui->avalue_offsety = CreateTextItem2(rc2, 10, "Y offset:");
    AddTextItemCB(ui->avalue_offsety, titem_explorer_cb, eui);
    ui->avalue_just = CreateJustChoice(rc, "Justification:");
    AddOptionChoiceCB(ui->avalue_just, oc_explorer_cb, eui);


    /* ------------ Errbar tab -------------- */

    ui->errbar_tp = CreateTabPage(tab, "Error bars");

    rc2 = CreateHContainer(ui->errbar_tp);

    rc1 = CreateVContainer(rc2);

    fr = CreateFrame(rc1, "Common");
    rc = CreateVContainer(fr);
    ui->errbar_ptype = CreatePanelChoice(rc,
                                         "Placement:",
                                         4,
                                         "Normal",
                                         "Opposite",
                                         "Both",
                                         NULL);
    AddOptionChoiceCB(ui->errbar_ptype, oc_explorer_cb, eui); 
    ui->errbar_color = CreateColorChoice(rc, "Color:");
    AddOptionChoiceCB(ui->errbar_color, oc_explorer_cb, eui);
    ui->errbar_pattern = CreatePatternChoice(rc, "Pattern:");
    AddOptionChoiceCB(ui->errbar_pattern, oc_explorer_cb, eui);

    fr = CreateFrame(rc1, "Clipping");
    rc = CreateVContainer(fr);
    ui->errbar_aclip = CreateToggleButton(rc, "Arrow clip");
    AddToggleButtonCB(ui->errbar_aclip, tb_explorer_cb, eui);
    ui->errbar_cliplen = CreateSpinChoice(rc, "Max length:",
        3, SPIN_TYPE_FLOAT, 0.0, 10.0, 0.1);
    AddSpinButtonCB(ui->errbar_cliplen, sp_explorer_cb, eui);

    rc1 = CreateVContainer(rc2);

    fr = CreateFrame(rc1, "Bar line");
    rc = CreateVContainer(fr);
    ui->errbar_size = CreateCharSizeChoice(rc, "Size");
    AddScaleCB(ui->errbar_size, scale_explorer_cb, eui);
    ui->errbar_width = CreateLineWidthChoice(rc, "Width:");
    AddSpinButtonCB(ui->errbar_width, sp_explorer_cb, eui);
    ui->errbar_lines = CreateLineStyleChoice(rc, "Style:");
    AddOptionChoiceCB(ui->errbar_lines, oc_explorer_cb, eui);

    fr = CreateFrame(rc1, "Riser line");
    rc = CreateVContainer(fr);
    ui->errbar_riserlinew = CreateLineWidthChoice(rc, "Width:");
    AddSpinButtonCB(ui->errbar_riserlinew, sp_explorer_cb, eui);
    ui->errbar_riserlines = CreateLineStyleChoice(rc, "Style:");
    AddOptionChoiceCB(ui->errbar_riserlines, oc_explorer_cb, eui);

    SelectTabPage(tab, ui->main_tp);

    ui->top = tab;

    return ui;
}

void update_set_ui(SetUI *ui, Quark *q)
{
    set *p = set_get_data(q);
    
    if (p && ui) {
        int i;
        char val[32];

        SetToggleButtonState(ui->active, p->active);
        
        SetOptionChoice(ui->type, p->type);
        for (i = 0; i < ui->type->nchoices; i++) {
            if (settype_cols(ui->type->options[i].value)==
                                            settype_cols(p->type)) {
                SetSensitive(ui->type->options[i].widget, True);
            } else {
                SetSensitive(ui->type->options[i].widget, False);
            }
        }

        SetCharSizeChoice(ui->symsize, p->sym.size);
        SetSpinChoice(ui->symskip, p->symskip);
        sprintf(val, "%d", p->sym.symchar);
        xv_setstr(ui->symchar, val);
        SetOptionChoice(ui->symbols, p->sym.type);
        
        SetOptionChoice(ui->symcolor, p->sym.line.pen.color);
        SetOptionChoice(ui->sympattern, p->sym.line.pen.pattern);
        SetOptionChoice(ui->symfillcolor, p->sym.fillpen.color);
        SetOptionChoice(ui->symfillpattern, p->sym.fillpen.pattern);
        SetSpinChoice(ui->symlinew, p->sym.line.width);
        SetOptionChoice(ui->symlines, p->sym.line.style);
        
        SetOptionChoice(ui->char_font, p->sym.charfont);        
        
        SetOptionChoice(ui->color, p->line.line.pen.color);
        SetOptionChoice(ui->pattern, p->line.line.pen.pattern);
        SetSpinChoice(ui->width, p->line.line.width);
        SetToggleButtonState(ui->dropline, p->line.droplines);
        SetOptionChoice(ui->lines, p->line.line.style);
        SetOptionChoice(ui->linet, p->line.type);
        SetOptionChoice(ui->filltype, p->line.filltype);
        SetOptionChoice(ui->fillrule, p->line.fillrule);
        SetOptionChoice(ui->fillcol, p->line.fillpen.color);
        SetOptionChoice(ui->fillpat, p->line.fillpen.pattern);
        
        SetToggleButtonState(ui->baseline, p->line.baseline);
        SetOptionChoice(ui->baselinetype, p->line.baseline_type);

        SetTextString(ui->legend_str, p->legstr);
        
        SetToggleButtonState(ui->errbar_active, p->errbar.active);
        
        switch (p->type) {
        case SET_XYDXDX:
        case SET_XYDYDY:
        case SET_XYDXDXDYDY:
            SetSensitive(ui->errbar_ptype->options[2].widget, FALSE);
            break;
        default:
            SetSensitive(ui->errbar_ptype->options[2].widget, TRUE);
            break;
        }
        SetOptionChoice(ui->errbar_ptype, p->errbar.ptype);
        SetOptionChoice(ui->errbar_color, p->errbar.pen.color);
        SetOptionChoice(ui->errbar_pattern, p->errbar.pen.pattern);
        SetToggleButtonState(ui->errbar_aclip, p->errbar.arrow_clip);
        SetSpinChoice(ui->errbar_cliplen, p->errbar.cliplen);
        SetSpinChoice(ui->errbar_width, p->errbar.linew);
        SetOptionChoice(ui->errbar_lines, p->errbar.lines);
        SetSpinChoice(ui->errbar_riserlinew, p->errbar.riser_linew);
        SetOptionChoice(ui->errbar_riserlines, p->errbar.riser_lines);
        SetCharSizeChoice(ui->errbar_size, p->errbar.barsize);

        SetToggleButtonState(ui->avalue_active, p->avalue.active);
        SetOptionChoice(ui->avalue_type, p->avalue.type);
        SetCharSizeChoice(ui->avalue_charsize, p->avalue.size);
        SetOptionChoice(ui->avalue_font, p->avalue.font);
        SetOptionChoice(ui->avalue_color, p->avalue.color);
        SetAngleChoice(ui->avalue_angle, p->avalue.angle);
        SetOptionChoice(ui->avalue_format, p->avalue.format);
        SetOptionChoice(ui->avalue_precision, p->avalue.prec);
        
        xv_setstr(ui->avalue_prestr, p->avalue.prestr);
        xv_setstr(ui->avalue_appstr, p->avalue.appstr);

        sprintf(val, "%f", p->avalue.offset.x);
        xv_setstr(ui->avalue_offsetx, val);
        sprintf(val, "%f", p->avalue.offset.y);
        xv_setstr(ui->avalue_offsety, val);

        SetOptionChoice(ui->avalue_just, p->avalue.just);
    }
}

int set_set_data(SetUI *ui, Quark *q, void *caller)
{
    set *p = set_get_data(q);
    
    if (p && ui) {
        if (!caller || caller == ui->active) {
            p->active = GetToggleButtonState(ui->active);
        }

        if (!caller || caller == ui->symskip) {
            p->symskip = GetSpinChoice(ui->symskip);
        }
        if (!caller || caller == ui->symsize) {
            p->sym.size = GetCharSizeChoice(ui->symsize);
        }
        if (!caller || caller == ui->symlinew) {
            p->sym.line.width = GetSpinChoice(ui->symlinew);
        }
        if (!caller || caller == ui->symlines) {
            p->sym.line.style = GetOptionChoice(ui->symlines);
        }
        if (!caller || caller == ui->symchar) {
            p->sym.symchar = atoi(xv_getstr(ui->symchar));
        }
        if (!caller || caller == ui->char_font) {
            p->sym.charfont = GetOptionChoice(ui->char_font);
        }
        if (!caller || caller == ui->filltype) {
            p->line.filltype = GetOptionChoice(ui->filltype);
        }
        if (!caller || caller == ui->fillrule) {
            p->line.fillrule = GetOptionChoice(ui->fillrule);
        }
        if (!caller || caller == ui->fillpat) {
            p->line.fillpen.pattern = GetOptionChoice(ui->fillpat);
        }
        if (!caller || caller == ui->fillcol) {
            p->line.fillpen.color = GetOptionChoice(ui->fillcol);
        }
        if (!caller || caller == ui->legend_str) {
            xfree(p->legstr);
            p->legstr = GetTextString(ui->legend_str);
        }
        if (!caller || caller == ui->symbols) {
            p->sym.type = GetOptionChoice(ui->symbols);
        }
        if (!caller || caller == ui->linet) {
            p->line.type = GetOptionChoice(ui->linet);
        }
        if (!caller || caller == ui->lines) {
            p->line.line.style = GetOptionChoice(ui->lines);
        }
        if (!caller || caller == ui->width) {
            p->line.line.width = GetSpinChoice(ui->width);
        }
        if (!caller || caller == ui->color) {
            p->line.line.pen.color = GetOptionChoice(ui->color);
        }
        if (!caller || caller == ui->pattern) {
            p->line.line.pen.pattern = GetOptionChoice(ui->pattern);
        }
        if (!caller || caller == ui->symcolor) {
            p->sym.line.pen.color = GetOptionChoice(ui->symcolor);
        }
        if (!caller || caller == ui->sympattern) {
            p->sym.line.pen.pattern = GetOptionChoice(ui->sympattern);
        }
        if (!caller || caller == ui->symfillcolor) {
            p->sym.fillpen.color = GetOptionChoice(ui->symfillcolor);
        }
        if (!caller || caller == ui->symfillpattern) {
            p->sym.fillpen.pattern = GetOptionChoice(ui->symfillpattern);
        }
        if (!caller || caller == ui->dropline) {
            p->line.droplines = GetToggleButtonState(ui->dropline);
        }
        if (!caller || caller == ui->baseline) {
            p->line.baseline = GetToggleButtonState(ui->baseline);
        }
        if (!caller || caller == ui->baselinetype) {
            p->line.baseline_type = GetOptionChoice(ui->baselinetype);
        }
        if (!caller || caller == ui->errbar_active) {
            p->errbar.active = GetToggleButtonState(ui->errbar_active);
        }
        if (!caller || caller == ui->errbar_size) {
            p->errbar.barsize = GetCharSizeChoice(ui->errbar_size);
        }
        if (!caller || caller == ui->errbar_width) {
            p->errbar.linew = GetSpinChoice(ui->errbar_width);
        }
        if (!caller || caller == ui->errbar_lines) {
            p->errbar.lines = GetOptionChoice(ui->errbar_lines);
        }
        if (!caller || caller == ui->errbar_riserlinew) {
            p->errbar.riser_linew = GetSpinChoice(ui->errbar_riserlinew);
        }
        if (!caller || caller == ui->type) {
            set_set_type((q), GetOptionChoice(ui->type));
        }
        if (!caller || caller == ui->errbar_riserlines) {
            p->errbar.riser_lines = GetOptionChoice(ui->errbar_riserlines);
        }
        if (!caller || caller == ui->errbar_ptype) {
            p->errbar.ptype = GetOptionChoice(ui->errbar_ptype);
        }
        if (!caller || caller == ui->errbar_color) {
            p->errbar.pen.color = GetOptionChoice(ui->errbar_color);
        }
        if (!caller || caller == ui->errbar_pattern) {
            p->errbar.pen.pattern = GetOptionChoice(ui->errbar_pattern);
        }
        if (!caller || caller == ui->errbar_aclip) {
            p->errbar.arrow_clip = GetToggleButtonState(ui->errbar_aclip);
        }
        if (!caller || caller == ui->errbar_cliplen) {
            p->errbar.cliplen = GetSpinChoice(ui->errbar_cliplen);
        }
        if (!caller || caller == ui->avalue_active) {
            p->avalue.active = GetToggleButtonState(ui->avalue_active);
        }
        if (!caller || caller == ui->avalue_type) {
            p->avalue.type = GetOptionChoice(ui->avalue_type);
        }
        if (!caller || caller == ui->avalue_charsize) {
            p->avalue.size = GetCharSizeChoice(ui->avalue_charsize);
        }
        if (!caller || caller == ui->avalue_font) {
            p->avalue.font = GetOptionChoice(ui->avalue_font);
        }
        if (!caller || caller == ui->avalue_color) {
            p->avalue.color = GetOptionChoice(ui->avalue_color);
        }
        if (!caller || caller == ui->avalue_angle) {
            p->avalue.angle = GetAngleChoice(ui->avalue_angle);
        }
        if (!caller || caller == ui->avalue_format) {
            p->avalue.format = GetOptionChoice(ui->avalue_format);
        }
        if (!caller || caller == ui->avalue_precision) {
            p->avalue.prec = GetOptionChoice(ui->avalue_precision);
        }
        if (!caller || caller == ui->avalue_prestr) {
            strcpy(p->avalue.prestr, xv_getstr(ui->avalue_prestr));
        }
        if (!caller || caller == ui->avalue_appstr) {
            strcpy(p->avalue.appstr, xv_getstr(ui->avalue_appstr));
        }
        if (!caller || caller == ui->avalue_offsetx) {
            xv_evalexpr(ui->avalue_offsetx, &p->avalue.offset.x);
        }
        if (!caller || caller == ui->avalue_offsety) {
            xv_evalexpr(ui->avalue_offsety, &p->avalue.offset.y);
        }
        if (!caller || caller == ui->avalue_just) {
            p->avalue.just = GetOptionChoice(ui->avalue_just);
        }

        quark_dirtystate_set(q, TRUE);
    
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}
