/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1996-2006 Grace Development Team
 * 
 * Maintained by Evgeny Stambulchik
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
#include "globals.h"

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
    
    int c = 0, bg = getbgcolor(gapp->rt->canvas);
    
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
            /* FIXME!!!
            while (c == bg ||
                   get_colortype(gapp->rt->canvas, c) != COLOR_MAIN) {
                c++;
                c %= number_of_colors(gapp->rt->canvas);
            }
            */
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
            p->line.line.style = (i % (number_of_linestyles(gapp->rt->canvas)- 1))
                + 1;
            break;
        case SETAPP_ALL_BW:
            setcolors(pset, 1);
            break;
        }
    }

    UpdateSymbols(cset);
    xdrawgraph(gapp->gp, FALSE);
}
#endif

static void charfont_cb(OptionStructure *opt, int a, void *data)
{
    SetUI *ui = (SetUI *) data;
    UpdateCharOptionChoice(ui->symchar, a);
}

void type_cb(OptionStructure *opt, int a, void *data)
{
    SetUI *ui = (SetUI *) data;
    unsigned int i, nncols;
    
    nncols = settype_cols(a);
    for (i = 0; i < MAX_SET_COLS; i++) {
        WidgetSetSensitive(ui->cols[i]->menu, (i < nncols));
    }
}

/*
 * create the symbols popup
 */
SetUI *create_set_ui(ExplorerUI *eui)
{
    SetUI *ui;
    Widget tab, fr, rc, rc1, rc2;
    OptionItem blockitem = {COL_NONE, "None"};
    unsigned int i;
    Grace *grace = gapp->grace;

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
    AddHelpCB(tab, "doc/UsersGuide.html#set-properties");


    /* ------------ Main tab -------------- */

    ui->main_tp = CreateTabPage(tab, "Main");

    fr = CreateFrame(ui->main_tp, "Set presentation");
    ui->type = CreateSetTypeChoice(fr, "Type:");
    AddOptionChoiceCB(ui->type, oc_explorer_cb, eui);
    AddOptionChoiceCB(ui->type, type_cb, ui);

    fr = CreateFrame(ui->main_tp, "Data binding");
    rc = CreateVContainer(fr);
    for (i = 0; i < MAX_SET_COLS; i++) {
        char buf[32];
        sprintf(buf, "%s from column:", dataset_col_name(grace, i));
        ui->cols[i] = CreateOptionChoice(rc, buf, 0, 1, &blockitem);
        AddOptionChoiceCB(ui->cols[i], oc_explorer_cb, eui);
    }
    ui->acol = CreateOptionChoice(rc, "Annotations from column:", 0, 1, &blockitem);
    AddOptionChoiceCB(ui->acol, oc_explorer_cb, eui);

    fr = CreateFrame(ui->main_tp, "Legend");
    ui->legend_str = CreateCSText(fr, "String:");
    AddTextActivateCB(ui->legend_str, text_explorer_cb, eui);

    fr = CreateFrame(ui->main_tp, "Display options");
    rc2 = CreateHContainer(fr);
    ui->avalue_active = CreateToggleButton(rc2, "Annotate values");
    AddToggleButtonCB(ui->avalue_active, tb_explorer_cb, eui);
    ui->errbar_active = CreateToggleButton(rc2, "Display error bars");
    AddToggleButtonCB(ui->errbar_active, tb_explorer_cb, eui);


    /* ------------ Symbols tab -------------- */

    ui->symbol_tp = CreateTabPage(tab, "Symbols");

    fr = CreateFrame(ui->symbol_tp, "Symbol properties");
    rc = CreateVContainer(fr);

    rc2 = CreateHContainer(rc);
    ui->symbols = CreateOptionChoiceVA(rc2, "Type:",
        "None",           SYM_NONE,
        "Circle",         SYM_CIRCLE,
        "Square",         SYM_SQUARE,
        "Diamond",        SYM_DIAMOND,
        "Triangle up",    SYM_TRIANG1,
        "Triangle left",  SYM_TRIANG2,
        "Triangle down",  SYM_TRIANG3,
        "Triangle right", SYM_TRIANG4,
        "Plus",           SYM_PLUS,
        "X",              SYM_X,
        "Star",           SYM_SPLAT,
        "Char",           SYM_CHAR,
        NULL);
    AddOptionChoiceCB(ui->symbols, oc_explorer_cb, eui);
    ui->symsize = CreateCharSizeChoice(rc2, "Size:");
    AddSpinChoiceCB(ui->symsize, sp_explorer_cb, eui);


    fr = CreateFrame(ui->symbol_tp, "Drawing properties");
    rc = CreateVContainer(fr);

    rc2 = CreateHContainer(rc);
    ui->symlines = CreateLineStyleChoice(rc2, "Style:");
    AddOptionChoiceCB(ui->symlines, oc_explorer_cb, eui);
    ui->symlinew = CreateLineWidthChoice(rc2, "Width:");
    AddSpinChoiceCB(ui->symlinew, sp_explorer_cb, eui);

    rc2 = CreateHContainer(rc);
    ui->sympen = CreatePenChoice(rc2, "Outline pen:");
    AddPenChoiceCB(ui->sympen, pen_explorer_cb, eui);
    ui->symfillpen = CreatePenChoice(rc2, "Fill pen:");
    AddPenChoiceCB(ui->symfillpen, pen_explorer_cb, eui);

    fr = CreateFrame(ui->symbol_tp, "Symbol char");
    rc = CreateHContainer(fr);
    ui->char_font = CreateFontChoice(rc, "Font:");
    AddOptionChoiceCB(ui->char_font, oc_explorer_cb, eui);
    AddOptionChoiceCB(ui->char_font, charfont_cb, ui);
    ui->symchar = CreateCharOptionChoice(rc, "Glyph:");
    AddOptionChoiceCB(ui->symchar, oc_explorer_cb, eui);


    fr = CreateFrame(ui->symbol_tp, "Extra");
    rc = CreateVContainer(fr);
    ui->symskip = CreateSpinChoice(rc, "Symbol skip:",
        5, SPIN_TYPE_INT, (double) 0, (double) 100000, (double) 1);
    AddSpinChoiceCB(ui->symskip, sp_explorer_cb, eui);
    ui->symskipmindist = CreateSpinChoice(
	 rc, "Minimum symbol separation:",
	 5, SPIN_TYPE_FLOAT, 0.0, 100.0, 0.01);
    AddSpinChoiceCB(ui->symskipmindist, sp_explorer_cb, eui);


    /* ------------ Line tab -------------- */

    ui->line_tp = CreateTabPage(tab, "Line");

    fr = CreateFrame(ui->line_tp, "Line properties");
    rc = CreateHContainer(fr);
    ui->linet = CreateOptionChoiceVA(rc, "Connection:",
        "None",         LINE_TYPE_NONE,
        "Straight",     LINE_TYPE_STRAIGHT,
        "Left stairs",  LINE_TYPE_LEFTSTAIR,
        "Right stairs", LINE_TYPE_RIGHTSTAIR,
        "Segments",     LINE_TYPE_SEGMENT2,
        "3-Segments",   LINE_TYPE_SEGMENT3,
        NULL);
    AddOptionChoiceCB(ui->linet, oc_explorer_cb, eui); 
    ui->baselinetype = CreateOptionChoiceVA(rc, "Base:",
        "Zero",      BASELINE_TYPE_0,
        "Set min",   BASELINE_TYPE_SMIN,
        "Set max",   BASELINE_TYPE_SMAX,
        "Graph min", BASELINE_TYPE_GMIN,
        "Graph max", BASELINE_TYPE_GMAX,
        NULL);
    AddOptionChoiceCB(ui->baselinetype, oc_explorer_cb, eui);

    fr = CreateFrame(ui->line_tp, "Drawing properties");
    rc = CreateVContainer(fr);

    rc2 = CreateHContainer(rc);
    ui->lines = CreateLineStyleChoice(rc2, "Style:");
    AddOptionChoiceCB(ui->lines, oc_explorer_cb, eui);
    ui->width = CreateLineWidthChoice(rc2, "Width:");
    AddSpinChoiceCB(ui->width, sp_explorer_cb, eui); 
    rc2 = CreateHContainer(rc);
    ui->pen = CreatePenChoice(rc2, "Outline pen:");
    AddPenChoiceCB(ui->pen, pen_explorer_cb, eui);
    ui->fillpen = CreatePenChoice(rc2, "Fill pen:");
    AddPenChoiceCB(ui->fillpen, pen_explorer_cb, eui);

    fr = CreateFrame(ui->line_tp, "Fill properties");
    rc = CreateVContainer(fr);
    rc2 = CreateHContainer(rc);
    ui->filltype = CreateOptionChoiceVA(rc2, "Type:",
        "None",        SETFILL_NONE,
        "As polygon",  SETFILL_POLYGON,
        "To baseline", SETFILL_BASELINE,
        NULL);
    AddOptionChoiceCB(ui->filltype, oc_explorer_cb, eui); 
    ui->fillrule = CreateOptionChoiceVA(rc2, "Rule:",
        "Winding",  FILLRULE_WINDING,
        "Even-Odd", FILLRULE_EVENODD,
        NULL);
    AddOptionChoiceCB(ui->fillrule, oc_explorer_cb, eui); 


    fr = CreateFrame(ui->line_tp, "Extra");
    rc = CreateHContainer(fr);
    ui->dropline = CreateToggleButton(rc, "Draw drop lines");
    AddToggleButtonCB(ui->dropline, tb_explorer_cb, eui);
    ui->baseline = CreateToggleButton(rc, "Draw base line");
    AddToggleButtonCB(ui->baseline, tb_explorer_cb, eui);


    /* ------------ Errbar tab -------------- */

    ui->errbar_tp = CreateTabPage(tab, "Error bars");

    rc2 = CreateHContainer(ui->errbar_tp);

    rc1 = CreateVContainer(rc2);

    fr = CreateFrame(rc1, "Common");
    rc = CreateVContainer(fr);
    ui->errbar_pen = CreatePenChoice(rc, "Pen:");
    AddPenChoiceCB(ui->errbar_pen, pen_explorer_cb, eui);

    fr = CreateFrame(rc1, "Clipping");
    rc = CreateVContainer(fr);
    ui->errbar_aclip = CreateToggleButton(rc, "Arrow clip");
    AddToggleButtonCB(ui->errbar_aclip, tb_explorer_cb, eui);
    ui->errbar_cliplen = CreateSpinChoice(rc, "Max length:",
        3, SPIN_TYPE_FLOAT, 0.0, 10.0, 0.1);
    AddSpinChoiceCB(ui->errbar_cliplen, sp_explorer_cb, eui);

    rc1 = CreateVContainer(rc2);

    fr = CreateFrame(rc1, "Bar line");
    rc = CreateVContainer(fr);
    ui->errbar_size = CreateSpinChoice(rc, "Size",
        4, SPIN_TYPE_FLOAT, 0.0, 10.0, 0.1);
    AddSpinChoiceCB(ui->errbar_size, sp_explorer_cb, eui);
    ui->errbar_width = CreateLineWidthChoice(rc, "Width:");
    AddSpinChoiceCB(ui->errbar_width, sp_explorer_cb, eui);
    ui->errbar_lines = CreateLineStyleChoice(rc, "Style:");
    AddOptionChoiceCB(ui->errbar_lines, oc_explorer_cb, eui);

    fr = CreateFrame(rc1, "Riser line");
    rc = CreateVContainer(fr);
    ui->errbar_riserlinew = CreateLineWidthChoice(rc, "Width:");
    AddSpinChoiceCB(ui->errbar_riserlinew, sp_explorer_cb, eui);
    ui->errbar_riserlines = CreateLineStyleChoice(rc, "Style:");
    AddOptionChoiceCB(ui->errbar_riserlines, oc_explorer_cb, eui);


    /* ------------ AValue tab -------------- */

    ui->avalue_tp = CreateTabPage(tab, "Ann. values");

    fr = CreateFrame(ui->avalue_tp, "Text properties");
    rc = CreateVContainer(fr);

    rc2 = CreateHContainer(rc);
    ui->avalue_font = CreateFontChoice(rc2, "Font:");
    AddOptionChoiceCB(ui->avalue_font, oc_explorer_cb, eui);
    ui->avalue_color = CreateColorChoice(rc2, "Color:");
    AddOptionChoiceCB(ui->avalue_color, oc_explorer_cb, eui);

    rc2 = CreateHContainer(rc);
    ui->avalue_charsize = CreateCharSizeChoice(rc2, "Size:");
    AddSpinChoiceCB(ui->avalue_charsize, sp_explorer_cb, eui);
    ui->avalue_angle = CreateAngleChoice(rc2, "Angle:");
    AddSpinChoiceCB(ui->avalue_angle, sp_explorer_cb, eui);

    rc2 = CreateHContainer(rc);
    ui->avalue_prestr = CreateText(rc2, "Prepend:");
    TextSetLength(ui->avalue_prestr, 13);
    AddTextActivateCB(ui->avalue_prestr, text_explorer_cb, eui);
    ui->avalue_appstr = CreateText(rc2, "Append:");
    TextSetLength(ui->avalue_appstr, 13);
    AddTextActivateCB(ui->avalue_appstr, text_explorer_cb, eui);

    fr = CreateFrame(ui->avalue_tp, "Format options");
    rc = CreateVContainer(fr);
    rc2 = CreateHContainer(rc);
    ui->avalue_format = CreateFormatChoice(rc);
    AddFormatChoiceCB(ui->avalue_format, format_explorer_cb, eui);

    fr = CreateFrame(ui->avalue_tp, "Placement");
    rc = CreateVContainer(fr);
    rc2 = CreateHContainer(rc);
    ui->avalue_offsetx = CreateText2(rc2, "X offset:", 10);
    AddTextActivateCB(ui->avalue_offsetx, text_explorer_cb, eui);
    ui->avalue_offsety = CreateText2(rc2, "Y offset:", 10);
    AddTextActivateCB(ui->avalue_offsety, text_explorer_cb, eui);
    ui->avalue_just = CreateTextJustChoice(rc, "Justification:");
    AddOptionChoiceCB(ui->avalue_just, oc_explorer_cb, eui);

    fr = CreateFrame(ui->avalue_tp, "Frame");
    rc = CreateVContainer(fr);
    rc1 = CreateHContainer(rc);
    ui->frame_decor = CreateOptionChoiceVA(rc1, "Type:",
        "None",      FRAME_DECOR_NONE,
        "Underline", FRAME_DECOR_LINE,
        "Rectangle", FRAME_DECOR_RECT,
        "Oval",      FRAME_DECOR_OVAL,
        NULL);
    AddOptionChoiceCB(ui->frame_decor, oc_explorer_cb, eui);
    ui->frame_offset = CreateSpinChoice(rc1, "Offset:", 5,
        SPIN_TYPE_FLOAT, 0.0, 1.0, 0.005);
    AddSpinChoiceCB(ui->frame_offset, sp_explorer_cb, eui);
    rc1 = CreateHContainer(rc);
    ui->frame_linew = CreateLineWidthChoice(rc1, "Width:");
    AddSpinChoiceCB(ui->frame_linew, sp_explorer_cb, eui);
    ui->frame_lines = CreateLineStyleChoice(rc1, "Style:");
    AddOptionChoiceCB(ui->frame_lines, oc_explorer_cb, eui);
    rc1 = CreateHContainer(rc);
    ui->frame_linepen = CreatePenChoice(rc1, "Outline pen:");
    AddPenChoiceCB(ui->frame_linepen, pen_explorer_cb, eui);
    ui->frame_fillpen = CreatePenChoice(rc1, "Fill pen:");
    AddPenChoiceCB(ui->frame_fillpen, pen_explorer_cb, eui);


    SelectTabPage(tab, ui->main_tp);

    ui->top = tab;

    return ui;
}

void update_set_ui(SetUI *ui, Quark *q)
{
    set *p = set_get_data(q);
    Quark *ss = get_parent_ssd(q);
    
    if (p && ui && ss) {
        int i;
        char val[32];
        int blocklen, blockncols;
        OptionItem *blockitems, *sblockitems;
        unsigned int nncols;

        SetOptionChoice(ui->type, p->type);

        blockncols  = ssd_get_ncols(ss);
        blocklen    = ssd_get_nrows(ss);

        blockitems  = xmalloc((blockncols + 1)*sizeof(OptionItem));
        sblockitems = xmalloc((blockncols + 1)*sizeof(OptionItem));
        blockitems[0].value = COL_NONE;
        blockitems[0].label = copy_string(NULL, "None");
        sblockitems[0].value = COL_NONE;
        sblockitems[0].label = copy_string(NULL, "None");
        nncols = 0;
        for (i = 0; i < blockncols; i++) {
            char buf[32], *s;
            int fformat = ssd_get_col_format(ss, i);
            char *label = ssd_get_col_label(ss, i);
            if (string_is_empty(label)) {
                sprintf(buf, "#%d", i + 1);
                s = copy_string(NULL, buf);
            } else {
                s = copy_string(NULL, label);
            }
            if (fformat != FFORMAT_STRING) {
                nncols++;
                blockitems[nncols].value = i;
                blockitems[nncols].label = s;
            }
            sblockitems[i + 1].value = i;
            sblockitems[i + 1].label = s;
        }
        for (i = 0; i < MAX_SET_COLS; i++) {
            UpdateOptionChoice(ui->cols[i], nncols + 1, blockitems);
            SetOptionChoice(ui->cols[i], p->ds.cols[i]);
        }
        UpdateOptionChoice(ui->acol, blockncols + 1, sblockitems);
        SetOptionChoice(ui->acol, p->ds.acol);

        xfree(blockitems);
        for (i = 0; i < blockncols + 1; i++) {
            xfree(sblockitems[i].label);
        }
        xfree(sblockitems);

        nncols = settype_cols(p->type);
        for (i = 0; i < MAX_SET_COLS; i++) {
            WidgetSetSensitive(ui->cols[i]->menu, (i < nncols));
        }


        SpinChoiceSetValue(ui->symsize, p->sym.size);
        SpinChoiceSetValue(ui->symskip, p->symskip);
        SpinChoiceSetValue(ui->symskipmindist, p->symskipmindist);
        UpdateCharOptionChoice(ui->symchar, p->sym.charfont);
        SetOptionChoice(ui->symchar, p->sym.symchar);
        SetOptionChoice(ui->symbols, p->sym.type);
        
        SetPenChoice(ui->sympen, &p->sym.line.pen);
        SetPenChoice(ui->symfillpen, &p->sym.fillpen);
        SpinChoiceSetValue(ui->symlinew, p->sym.line.width);
        SetOptionChoice(ui->symlines, p->sym.line.style);
        
        SetOptionChoice(ui->char_font, p->sym.charfont);        
        
        SetPenChoice(ui->pen, &p->line.line.pen);
        SpinChoiceSetValue(ui->width, p->line.line.width);
        ToggleButtonSetState(ui->dropline, p->line.droplines);
        SetOptionChoice(ui->lines, p->line.line.style);
        SetOptionChoice(ui->linet, p->line.type);
        SetOptionChoice(ui->filltype, p->line.filltype);
        SetOptionChoice(ui->fillrule, p->line.fillrule);
        SetPenChoice(ui->fillpen, &p->line.fillpen);
        
        ToggleButtonSetState(ui->baseline, p->line.baseline);
        SetOptionChoice(ui->baselinetype, p->line.baseline_type);

        TextSetString(ui->legend_str, p->legstr);
        
        ToggleButtonSetState(ui->errbar_active, p->errbar.active);
        
        SetPenChoice(ui->errbar_pen, &p->errbar.pen);
        ToggleButtonSetState(ui->errbar_aclip, p->errbar.arrow_clip);
        SpinChoiceSetValue(ui->errbar_cliplen, p->errbar.cliplen);
        SpinChoiceSetValue(ui->errbar_width, p->errbar.linew);
        SetOptionChoice(ui->errbar_lines, p->errbar.lines);
        SpinChoiceSetValue(ui->errbar_riserlinew, p->errbar.riser_linew);
        SetOptionChoice(ui->errbar_riserlines, p->errbar.riser_lines);
        SpinChoiceSetValue(ui->errbar_size, p->errbar.barsize);

        ToggleButtonSetState(ui->avalue_active, p->avalue.active);
        SpinChoiceSetValue(ui->avalue_charsize, p->avalue.tprops.charsize);
        SetOptionChoice(ui->avalue_font, p->avalue.tprops.font);
        SetOptionChoice(ui->avalue_color, p->avalue.tprops.color);
        SetAngleChoice(ui->avalue_angle, p->avalue.tprops.angle);
        SetFormatChoice(ui->avalue_format, &p->avalue.format);
        
        TextSetString(ui->avalue_prestr, p->avalue.prestr);
        TextSetString(ui->avalue_appstr, p->avalue.appstr);

        sprintf(val, "%f", p->avalue.offset.x);
        TextSetString(ui->avalue_offsetx, val);
        sprintf(val, "%f", p->avalue.offset.y);
        TextSetString(ui->avalue_offsety, val);

        SetOptionChoice(ui->avalue_just, p->avalue.tprops.just);

        SetOptionChoice(ui->frame_decor, p->avalue.frame.decor);
        SpinChoiceSetValue(ui->frame_offset, p->avalue.frame.offset);
        SpinChoiceSetValue(ui->frame_linew,   p->avalue.frame.line.width);
        SetOptionChoice(ui->frame_lines, p->avalue.frame.line.style);
        SetPenChoice(ui->frame_linepen, &p->avalue.frame.line.pen);
        SetPenChoice(ui->frame_fillpen, &p->avalue.frame.fillpen);
    }
}

int set_set_data(SetUI *ui, Quark *q, void *caller)
{
    set *p = set_get_data(q);
    
    if (p && ui) {
        AMem *amem = quark_get_amem(q);
        unsigned int i, nncols = settype_cols(GetOptionChoice(ui->type));
        
        if (!caller || caller == ui->type) {
            set_set_type((q), GetOptionChoice(ui->type));
        }
        
        for (i = 0; i < nncols; i++) {
            if (!caller || caller == ui->cols[i]) {
                p->ds.cols[i] = GetOptionChoice(ui->cols[i]);
            }
        }
        if (!caller || caller == ui->acol) {
            p->ds.acol = GetOptionChoice(ui->acol);
        }

        if (!caller || caller == ui->symskip) {
            p->symskip = SpinChoiceGetValue(ui->symskip);
        }
        if (!caller || caller == ui->symskipmindist) {
            p->symskipmindist = SpinChoiceGetValue(ui->symskipmindist);
        }
        if (!caller || caller == ui->symsize) {
            p->sym.size = SpinChoiceGetValue(ui->symsize);
        }
        if (!caller || caller == ui->symlinew) {
            p->sym.line.width = SpinChoiceGetValue(ui->symlinew);
        }
        if (!caller || caller == ui->symlines) {
            p->sym.line.style = GetOptionChoice(ui->symlines);
        }
        if (!caller || caller == ui->symchar) {
            p->sym.symchar = GetOptionChoice(ui->symchar);
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
        if (!caller || caller == ui->fillpen) {
            GetPenChoice(ui->fillpen, &p->line.fillpen);
        }
        if (!caller || caller == ui->legend_str) {
            char *s = TextGetString(ui->legend_str);
            p->legstr = amem_strcpy(amem, p->legstr, s);
            xfree(s);
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
            p->line.line.width = SpinChoiceGetValue(ui->width);
        }
        if (!caller || caller == ui->pen) {
            GetPenChoice(ui->pen, &p->line.line.pen);
        }
        if (!caller || caller == ui->sympen) {
            GetPenChoice(ui->sympen, &p->sym.line.pen);
        }
        if (!caller || caller == ui->symfillpen) {
            GetPenChoice(ui->symfillpen, &p->sym.fillpen);
        }
        if (!caller || caller == ui->dropline) {
            p->line.droplines = ToggleButtonGetState(ui->dropline);
        }
        if (!caller || caller == ui->baseline) {
            p->line.baseline = ToggleButtonGetState(ui->baseline);
        }
        if (!caller || caller == ui->baselinetype) {
            p->line.baseline_type = GetOptionChoice(ui->baselinetype);
        }
        if (!caller || caller == ui->errbar_active) {
            p->errbar.active = ToggleButtonGetState(ui->errbar_active);
        }
        if (!caller || caller == ui->errbar_size) {
            p->errbar.barsize = SpinChoiceGetValue(ui->errbar_size);
        }
        if (!caller || caller == ui->errbar_width) {
            p->errbar.linew = SpinChoiceGetValue(ui->errbar_width);
        }
        if (!caller || caller == ui->errbar_lines) {
            p->errbar.lines = GetOptionChoice(ui->errbar_lines);
        }
        if (!caller || caller == ui->errbar_riserlinew) {
            p->errbar.riser_linew = SpinChoiceGetValue(ui->errbar_riserlinew);
        }
        if (!caller || caller == ui->errbar_riserlines) {
            p->errbar.riser_lines = GetOptionChoice(ui->errbar_riserlines);
        }
        if (!caller || caller == ui->errbar_pen) {
            GetPenChoice(ui->errbar_pen, &p->errbar.pen);
        }
        if (!caller || caller == ui->errbar_aclip) {
            p->errbar.arrow_clip = ToggleButtonGetState(ui->errbar_aclip);
        }
        if (!caller || caller == ui->errbar_cliplen) {
            p->errbar.cliplen = SpinChoiceGetValue(ui->errbar_cliplen);
        }
        if (!caller || caller == ui->avalue_active) {
            p->avalue.active = ToggleButtonGetState(ui->avalue_active);
        }
        if (!caller || caller == ui->avalue_charsize) {
            p->avalue.tprops.charsize = SpinChoiceGetValue(ui->avalue_charsize);
        }
        if (!caller || caller == ui->avalue_font) {
            p->avalue.tprops.font = GetOptionChoice(ui->avalue_font);
        }
        if (!caller || caller == ui->avalue_color) {
            p->avalue.tprops.color = GetOptionChoice(ui->avalue_color);
        }
        if (!caller || caller == ui->avalue_angle) {
            p->avalue.tprops.angle = GetAngleChoice(ui->avalue_angle);
        }
        if (!caller || caller == ui->avalue_format) {
            Format *format = GetFormatChoice(ui->avalue_format);
            AMem *amem = quark_get_amem(q);
            amem_free(amem, p->avalue.format.fstring);
            p->avalue.format = *format;
            p->avalue.format.fstring = amem_strdup(amem, format->fstring);
            format_free(format);
        }
        if (!caller || caller == ui->avalue_prestr) {
            char *s = TextGetString(ui->avalue_prestr);
            p->avalue.prestr = amem_strcpy(amem, p->avalue.prestr, s);
            xfree(s);
        }
        if (!caller || caller == ui->avalue_appstr) {
            char *s = TextGetString(ui->avalue_appstr);
            p->avalue.appstr = amem_strcpy(amem, p->avalue.appstr, s);
            xfree(s);
        }
        if (!caller || caller == ui->avalue_offsetx) {
            xv_evalexpr(ui->avalue_offsetx, &p->avalue.offset.x);
        }
        if (!caller || caller == ui->avalue_offsety) {
            xv_evalexpr(ui->avalue_offsety, &p->avalue.offset.y);
        }
        if (!caller || caller == ui->avalue_just) {
            p->avalue.tprops.just = GetOptionChoice(ui->avalue_just);
        }

        if (!caller || caller == ui->frame_decor) {
            p->avalue.frame.decor = GetOptionChoice(ui->frame_decor);
        }
        if (!caller || caller == ui->frame_offset) {
            p->avalue.frame.offset = SpinChoiceGetValue(ui->frame_offset);
        }
        if (!caller || caller == ui->frame_linew) {
            p->avalue.frame.line.width = SpinChoiceGetValue(ui->frame_linew);
        }
        if (!caller || caller == ui->frame_lines) {
            p->avalue.frame.line.style = GetOptionChoice(ui->frame_lines);
        }
        if (!caller || caller == ui->frame_linepen) {
            GetPenChoice(ui->frame_linepen, &p->avalue.frame.line.pen);
        }
        if (!caller || caller == ui->frame_fillpen) {
            GetPenChoice(ui->frame_fillpen, &p->avalue.frame.fillpen);
        }

        quark_dirtystate_set(q, TRUE);
    
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}
