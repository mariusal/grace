/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c)1991-1995 Paul J Turner, Portland, OR
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

#include <stdio.h>
#include <stdlib.h>

#include "globals.h"
#include "graphs.h"
#include "grace/canvas.h"
#include "utils.h"
#include "motifinc.h"
#include "protos.h"

#define cg graph_get_current(grace->project)

#define SETAPP_STRIP_LEGENDS    0
#define SETAPP_LOAD_COMMENTS    1
#define SETAPP_ALL_COLORS       2
#define SETAPP_ALL_SYMBOLS      3
#define SETAPP_ALL_LINEW        4
#define SETAPP_ALL_LINES        5
#define SETAPP_ALL_BW           6

#define CSYNC_LINE      0
#define CSYNC_SYM       1

static Quark *cset = NULL;      /* the current set from the symbols panel */

static Widget setapp_dialog = NULL;

static Widget duplegs_item;

static OptionStructure *type_item;
static OptionStructure *toggle_symbols_item;
static Widget symsize_item;
static SpinStructure *symskip_item;
static OptionStructure *symcolor_item;
static OptionStructure *sympattern_item;
static OptionStructure *symfillcolor_item;
static OptionStructure *symfillpattern_item;
static SpinStructure *symlinew_item;
static OptionStructure *symlines_item;
static Widget symchar_item;
static OptionStructure *char_font_item;

static OptionStructure *toggle_color_item;
static OptionStructure *toggle_pattern_item;
static SpinStructure *toggle_width_item;
static Widget dropline_item;
static OptionStructure *toggle_lines_item;
static OptionStructure *toggle_linet_item;
static OptionStructure *toggle_filltype_item;
static OptionStructure *toggle_fillrule_item;
static OptionStructure *toggle_fillpat_item;
static OptionStructure *toggle_fillcol_item;
static StorageStructure *toggle_symset_item;
static Widget baseline_item;
static OptionStructure *baselinetype_item;

static TextStructure *legend_str_item;

static Widget errbar_active_item;
static OptionStructure *errbar_ptype_item;
static OptionStructure *errbar_color_item;
static OptionStructure *errbar_pattern_item;
static Widget errbar_size_item;
static SpinStructure *errbar_width_item;
static OptionStructure *errbar_lines_item;
static SpinStructure *errbar_riserlinew_item;
static OptionStructure *errbar_riserlines_item;
static Widget errbar_aclip_item;
static SpinStructure *errbar_cliplen_item;

static Widget avalue_active_item;
static OptionStructure *avalue_type_item;
static OptionStructure *avalue_font_item;
static OptionStructure *avalue_color_item;
static Widget avalue_charsize_item ;
static Widget avalue_angle_item;
static OptionStructure *avalue_format_item;
static OptionStructure *avalue_precision_item;
static Widget avalue_offsetx;
static Widget avalue_offsety;
static Widget avalue_prestr;
static Widget avalue_appstr;

static Widget csync_item;
static Widget instantupdate_item;

static void UpdateSymbols(Quark *pset);
static void set_cset_proc(int n, void **values, void *data);
static int setapp_aac_cb(void *data);
static void setapp_data_proc(void *data);
static void csync_cb(int value, void *data);

/* 
 * callback functions to do incremental set appearance updates
 */
static void oc_setapp_cb(int c, void *data)
{
    setapp_aac_cb(data);
}
static void tb_setapp_cb(int c, void *data)
{
    setapp_aac_cb(data);
}
static void scale_setapp_cb(int c, void *data)
{
    setapp_aac_cb(data);
}
static void sp_setapp_cb(double a, void *data)
{
    setapp_aac_cb(data);
}
static void text_setapp_cb(char *s, void *data)
{
    setapp_aac_cb(data);
}

/*
 * create the symbols popup
 */
void define_symbols_popup(void *data)
{
    Quark *pset;
    
    set_wait_cursor();
    
    pset = (Quark *) data;
    if (pset) {
        cset = pset;
    }
    
    if (setapp_dialog == NULL) {
        Widget setapp_tab, setapp_main, setapp_symbols, 
               setapp_line, setapp_errbar, setapp_avalue, fr, rc, rc1, rc2;
        Widget menubar, menupane;

        setapp_dialog = CreateDialogForm(app_shell, "Set Appearance");

        menubar = CreateMenuBar(setapp_dialog);
        AddDialogFormChild(setapp_dialog, menubar);
        ManageChild(menubar);

        menupane = CreateMenu(menubar, "File", 'F', FALSE);
        CreateMenuCloseButton(menupane, setapp_dialog);

        menupane = CreateMenu(menubar, "Edit", 'E', FALSE);
        CreateMenuButton(menupane, "Set different colors", 'c',
            setapp_data_proc, (void *)SETAPP_ALL_COLORS);
        CreateMenuButton(menupane, "Set different symbols", 's',
            setapp_data_proc, (void *)SETAPP_ALL_SYMBOLS);
        CreateMenuButton(menupane, "Set different line widths", 'w',
            setapp_data_proc, (void *)SETAPP_ALL_LINEW);
        CreateMenuButton(menupane, "Set different line styles", 'y',
            setapp_data_proc, (void *)SETAPP_ALL_LINES);
        CreateMenuButton(menupane, "Set black & white", 'B',
            setapp_data_proc, (void *)SETAPP_ALL_BW);
        CreateMenuSeparator(menupane);
        CreateMenuButton(menupane, "Load comments", 'm',
            setapp_data_proc, (void *)SETAPP_LOAD_COMMENTS);
        CreateMenuButton(menupane, "Strip legends", 'l',
            setapp_data_proc, (void *)SETAPP_STRIP_LEGENDS);
        
        menupane = CreateMenu(menubar, "Options", 'O', FALSE);
        duplegs_item = CreateMenuToggle(menupane,
            "Duplicate legends", 'D', NULL, NULL);
        csync_item = CreateMenuToggle(menupane, "Color sync", 's', NULL, NULL);
        SetToggleButtonState(csync_item, TRUE);
        instantupdate_item = CreateMenuToggle(menupane, "Instantaneous update",
                            'u', NULL, NULL);
        SetToggleButtonState(instantupdate_item, grace->gui->instant_update);

        menupane = CreateMenu(menubar, "Help", 'H', TRUE);
        CreateMenuHelpButton(menupane, "On set appearance", 's',
            setapp_dialog, "doc/UsersGuide.html#set-appearance");
        
        toggle_symset_item = CreateSetChoice(setapp_dialog, "Select set:",
                                                LIST_TYPE_MULTIPLE, NULL);
        AddDialogFormChild(setapp_dialog, toggle_symset_item->rc);
        AddStorageChoiceCB(toggle_symset_item, set_cset_proc, NULL);


        /* ------------ Tabs -------------- */

        setapp_tab = CreateTab(setapp_dialog);        


        /* ------------ Main tab -------------- */
        
        setapp_main = CreateTabPage(setapp_tab, "Main");

        fr = CreateFrame(setapp_main, "Set presentation");
	type_item = CreateSetTypeChoice(fr, "Type:");
        AddOptionChoiceCB(type_item, oc_setapp_cb, type_item);

        rc2 = CreateHContainer(setapp_main);

        fr = CreateFrame(rc2, "Symbol properties");
        rc = CreateVContainer(fr);
        toggle_symbols_item = CreatePanelChoice(rc,
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
                                                 NULL,
                                                 0);
        AddOptionChoiceCB(toggle_symbols_item, oc_setapp_cb, toggle_symbols_item);
        symsize_item = CreateCharSizeChoice(rc, "Size");
        AddScaleCB(symsize_item, scale_setapp_cb, symsize_item);
        symcolor_item = CreateColorChoice(rc, "Color:");
        AddOptionChoiceCB(symcolor_item, csync_cb, (void *)CSYNC_SYM);
        AddOptionChoiceCB(symcolor_item, oc_setapp_cb, symcolor_item);
        symchar_item = CreateTextItem2(rc, 3, "Symbol char:");
        AddTextItemCB(symchar_item, text_setapp_cb, symchar_item);

        fr = CreateFrame(rc2, "Line properties");
        rc = CreateVContainer(fr);
        toggle_linet_item = CreatePanelChoice(rc, "Type:",
                                              7,
                                              "None",
                                              "Straight",
                                              "Left stairs",
                                              "Right stairs",
                                              "Segments",
                                              "3-Segments",
                                              NULL,
                                              0);
        AddOptionChoiceCB(toggle_linet_item, oc_setapp_cb, toggle_linet_item); 
        toggle_lines_item = CreateLineStyleChoice(rc, "Style:");
        AddOptionChoiceCB(toggle_lines_item, oc_setapp_cb, toggle_lines_item);
        toggle_width_item = CreateLineWidthChoice(rc, "Width:");
        AddSpinButtonCB(toggle_width_item, sp_setapp_cb, toggle_width_item); 
        toggle_color_item = CreateColorChoice(rc, "Color:");
        AddOptionChoiceCB(toggle_color_item, csync_cb, (void *)CSYNC_LINE);
        AddOptionChoiceCB(toggle_color_item, oc_setapp_cb, toggle_color_item);
        
        fr = CreateFrame(setapp_main, "Legend");
        legend_str_item = CreateCSText(fr, "String:");
        AddTextInputCB(legend_str_item, text_setapp_cb, legend_str_item);

        fr = CreateFrame(setapp_main, "Display options");
        rc2 = CreateHContainer(fr);
        avalue_active_item = CreateToggleButton(rc2, "Annotate values");
        AddToggleButtonCB(avalue_active_item, tb_setapp_cb, avalue_active_item);
        errbar_active_item = CreateToggleButton(rc2, "Display error bars");
        AddToggleButtonCB(errbar_active_item, tb_setapp_cb, errbar_active_item);


        /* ------------ Symbols tab -------------- */
        
        setapp_symbols = CreateTabPage(setapp_tab, "Symbols");

        fr = CreateFrame(setapp_symbols, "Symbol outline");
        rc = CreateVContainer(fr);

        rc2 = CreateHContainer(rc);
        symlines_item = CreateLineStyleChoice(rc2, "Style:");
        AddOptionChoiceCB(symlines_item, oc_setapp_cb, symlines_item);
        symlinew_item = CreateLineWidthChoice(rc2, "Width:");
        AddSpinButtonCB(symlinew_item, sp_setapp_cb, symlinew_item);
        sympattern_item = CreatePatternChoice(rc, "Pattern:");
        AddOptionChoiceCB(sympattern_item, oc_setapp_cb, sympattern_item);

        fr = CreateFrame(setapp_symbols, "Symbol fill");
        rc = CreateHContainer(fr);
        symfillcolor_item = CreateColorChoice(rc, "Color:");
        AddOptionChoiceCB(symfillcolor_item, oc_setapp_cb, symfillcolor_item);
        symfillpattern_item = CreatePatternChoice(rc, "Pattern:");
        AddOptionChoiceCB(symfillpattern_item, oc_setapp_cb, symfillpattern_item);

        fr = CreateFrame(setapp_symbols, "Extra");
        rc = CreateVContainer(fr);
        symskip_item = CreateSpinChoice(rc, "Symbol skip:",
            5, SPIN_TYPE_INT, (double) 0, (double) 100000, (double) 1);
        AddSpinButtonCB(symskip_item, sp_setapp_cb, symskip_item);
        char_font_item = CreateFontChoice(rc, "Font for char symbol:");
        AddOptionChoiceCB(char_font_item, oc_setapp_cb, char_font_item);


        /* ------------ Line tab -------------- */
        
        setapp_line = CreateTabPage(setapp_tab, "Line");

        fr = CreateFrame(setapp_line, "Line properties");
        rc = CreateHContainer(fr);
        toggle_pattern_item = CreatePatternChoice(rc, "Pattern:");
        AddOptionChoiceCB(toggle_pattern_item, oc_setapp_cb, toggle_pattern_item);
        dropline_item = CreateToggleButton(rc, "Draw drop lines");
        AddToggleButtonCB(dropline_item, tb_setapp_cb, dropline_item);

        fr = CreateFrame(setapp_line, "Fill properties");
        rc = CreateVContainer(fr);
        rc2 = CreateHContainer(rc);
        toggle_filltype_item = CreatePanelChoice(rc2, "Type:",
                                             4,
                                             "None",
                                             "As polygon",
                                             "To baseline",
                                             NULL,
                                             0);
        AddOptionChoiceCB(toggle_filltype_item, oc_setapp_cb, toggle_filltype_item); 
        toggle_fillrule_item = CreatePanelChoice(rc2, "Rule:",
                                             3,
                                             "Winding",
                                             "Even-Odd",
                                             NULL,
                                             0);
        AddOptionChoiceCB(toggle_fillrule_item, oc_setapp_cb, toggle_fillrule_item); 
        rc2 = CreateHContainer(rc);
        toggle_fillpat_item = CreatePatternChoice(rc2, "Pattern:");
        AddOptionChoiceCB(toggle_fillpat_item, oc_setapp_cb, toggle_fillpat_item);
        toggle_fillcol_item = CreateColorChoice(rc2, "Color:");
        AddOptionChoiceCB(toggle_fillcol_item, oc_setapp_cb, toggle_fillcol_item);
        
        fr = CreateFrame(setapp_line, "Base line");
        rc = CreateHContainer(fr);
        baselinetype_item = CreatePanelChoice(rc, "Type:",
                                             6,
                                             "Zero",
                                             "Set min",
                                             "Set max",
                                             "Graph min",
                                             "Graph max",
                                             NULL,
                                             0);
        AddOptionChoiceCB(baselinetype_item, oc_setapp_cb, baselinetype_item);
        baseline_item = CreateToggleButton(rc, "Draw line");
        AddToggleButtonCB(baseline_item, tb_setapp_cb, baseline_item);
        
        
        /* ------------ AValue tab -------------- */
        
        setapp_avalue = CreateTabPage(setapp_tab, "Ann. values");

	fr = CreateFrame(setapp_avalue, "Text properties");
	rc = CreateVContainer(fr);
        
        rc2 = CreateHContainer(rc);
	avalue_font_item = CreateFontChoice(rc2, "Font:");
        AddOptionChoiceCB(avalue_font_item, oc_setapp_cb, avalue_active_item);
	avalue_charsize_item = CreateCharSizeChoice(rc2, "Char size");
	SetScaleWidth(avalue_charsize_item, 120);
        AddScaleCB(avalue_charsize_item, scale_setapp_cb, avalue_active_item);
        
        rc2 = CreateHContainer(rc);
	avalue_color_item = CreateColorChoice(rc2, "Color:");
        AddOptionChoiceCB(avalue_color_item, oc_setapp_cb, avalue_active_item);
	avalue_angle_item = CreateAngleChoice(rc2, "Angle");
	SetScaleWidth(avalue_angle_item, 180);
        AddScaleCB(avalue_angle_item, scale_setapp_cb, avalue_active_item);

        rc2 = CreateHContainer(rc);
        avalue_prestr = CreateTextItem2(rc2, 10, "Prepend:");
        AddTextItemCB(avalue_prestr, text_setapp_cb, avalue_active_item);
        avalue_appstr = CreateTextItem2(rc2, 10, "Append:");
        AddTextItemCB(avalue_appstr, text_setapp_cb, avalue_active_item);
        
	fr = CreateFrame(setapp_avalue, "Format options");
	rc = CreateVContainer(fr);
        rc2 = CreateHContainer(rc);
	avalue_format_item = CreateFormatChoice(rc, "Format:");
        AddOptionChoiceCB(avalue_format_item, oc_setapp_cb, avalue_active_item);
        avalue_type_item = CreatePanelChoice(rc2, "Type:",
                                             7,
                                             "None",
                                             "X",
                                             "Y",
                                             "X, Y",
                                             "String",
                                             "Z",
                                             NULL,
                                             0);
        AddOptionChoiceCB(avalue_type_item, oc_setapp_cb, avalue_active_item); 
	avalue_precision_item = CreatePrecisionChoice(rc2, "Precision:");
        AddOptionChoiceCB(avalue_precision_item, oc_setapp_cb, avalue_active_item);
        
	fr = CreateFrame(setapp_avalue, "Placement");
        rc2 = CreateHContainer(fr);
        avalue_offsetx = CreateTextItem2(rc2, 10, "X offset:");
        AddTextItemCB(avalue_offsetx, text_setapp_cb, avalue_active_item);
        avalue_offsety = CreateTextItem2(rc2, 10, "Y offset:");
        AddTextItemCB(avalue_offsety, text_setapp_cb, avalue_active_item);
        

        /* ------------ Errbar tab -------------- */
        
        setapp_errbar = CreateTabPage(setapp_tab, "Error bars");

        rc2 = CreateHContainer(setapp_errbar);

        rc1 = CreateVContainer(rc2);

        fr = CreateFrame(rc1, "Common");
        rc = CreateVContainer(fr);
        errbar_ptype_item = CreatePanelChoice(rc,
                                             "Placement:",
                                             4,
                                             "Normal",
                                             "Opposite",
                                             "Both",
                                             NULL,
                                             0);
        AddOptionChoiceCB(errbar_ptype_item, oc_setapp_cb, errbar_ptype_item); 
	errbar_color_item = CreateColorChoice(rc, "Color:");
        AddOptionChoiceCB(errbar_color_item, oc_setapp_cb, errbar_color_item);
	errbar_pattern_item = CreatePatternChoice(rc, "Pattern:");
        AddOptionChoiceCB(errbar_pattern_item, oc_setapp_cb, errbar_pattern_item);

        fr = CreateFrame(rc1, "Clipping");
        rc = CreateVContainer(fr);
	errbar_aclip_item = CreateToggleButton(rc, "Arrow clip");
        AddToggleButtonCB(errbar_aclip_item, tb_setapp_cb, errbar_aclip_item);
	errbar_cliplen_item = CreateSpinChoice(rc, "Max length:",
            3, SPIN_TYPE_FLOAT, 0.0, 10.0, 0.1);
        AddSpinButtonCB(errbar_cliplen_item, sp_setapp_cb, errbar_cliplen_item);

        rc1 = CreateVContainer(rc2);

        fr = CreateFrame(rc1, "Bar line");
        rc = CreateVContainer(fr);
        errbar_size_item = CreateCharSizeChoice(rc, "Size");
        AddScaleCB(errbar_size_item, scale_setapp_cb, errbar_size_item);
        errbar_width_item = CreateLineWidthChoice(rc, "Width:");
        AddSpinButtonCB(errbar_width_item, sp_setapp_cb, errbar_width_item);
        errbar_lines_item = CreateLineStyleChoice(rc, "Style:");
        AddOptionChoiceCB(errbar_lines_item, oc_setapp_cb, errbar_lines_item);

        fr = CreateFrame(rc1, "Riser line");
        rc = CreateVContainer(fr);
        errbar_riserlinew_item = CreateLineWidthChoice(rc, "Width:");
        AddSpinButtonCB(errbar_riserlinew_item,
            sp_setapp_cb, errbar_riserlinew_item);
        errbar_riserlines_item = CreateLineStyleChoice(rc, "Style:");
        AddOptionChoiceCB(errbar_riserlines_item,
            oc_setapp_cb, errbar_riserlines_item);
        
        SelectTabPage(setapp_tab, setapp_main);


        CreateAACDialog(setapp_dialog, setapp_tab, setapp_aac_cb, NULL);
    }
    updatesymbols(cset);
    
    RaiseWindow(GetParent(setapp_dialog));
    unset_wait_cursor();
}

/*
 * define symbols for the current set
 */
static int setapp_aac_cb(void *data)
{
    int i, cd;
    int duplegs;
    Quark *pset, **selset;
    
    if (!GetToggleButtonState(instantupdate_item) && data != NULL) {
        return RETURN_SUCCESS;
    }
    
    duplegs = GetToggleButtonState(duplegs_item);

    cd = GetStorageChoices(toggle_symset_item, (void ***) &selset);
    if (cd < 1) {
        errwin("No set selected");
        return RETURN_FAILURE;
    } else {
        for(i = 0; i < cd; i++) {
            set *p;
            pset = selset[i];
            p = pset->data;
            if (data == symskip_item || data == NULL) {
                p->symskip = GetSpinChoice(symskip_item);
            }
            if (data == symsize_item || data  ==  NULL) {
                p->sym.size = GetCharSizeChoice(symsize_item);
            }
            if (data == symlinew_item || data  ==  NULL) {
                p->sym.line.width = GetSpinChoice(symlinew_item);
            }
            if (data == symlines_item || data  ==  NULL) {
                p->sym.line.style = GetOptionChoice(symlines_item);
            }
            if (data == symchar_item || data == NULL) {
                p->sym.symchar = atoi(xv_getstr(symchar_item));
            }
            if (data == char_font_item || data == NULL) {
                p->sym.charfont = GetOptionChoice(char_font_item);
            }
            if (data == toggle_filltype_item || data == NULL) {
                p->line.filltype = GetOptionChoice(toggle_filltype_item);
            }
            if (data == toggle_fillrule_item || data == NULL) {
                p->line.fillrule = GetOptionChoice(toggle_fillrule_item);
            }
            if (data == toggle_fillpat_item || data == NULL) {
                p->line.fillpen.pattern = GetOptionChoice(toggle_fillpat_item);
            }
            if (data == toggle_fillcol_item || data == NULL) {
                p->line.fillpen.color = GetOptionChoice(toggle_fillcol_item);
            }
            if (data == legend_str_item || data == NULL) {
                if (cd == 1 || duplegs) {
                    xfree(p->legstr);
                    p->legstr = GetTextString(legend_str_item);
                }
            }
            if (data == toggle_symbols_item || data == NULL) {
                p->sym.type = GetOptionChoice(toggle_symbols_item);
            }
            if (data == toggle_linet_item || data == NULL) {
                p->line.type = GetOptionChoice(toggle_linet_item);
            }
            if (data == toggle_lines_item || data == NULL) {
                p->line.line.style = GetOptionChoice(toggle_lines_item);
            }
            if (data == toggle_width_item || data == NULL) {
                p->line.line.width = GetSpinChoice(toggle_width_item);
            }
            if (data == toggle_color_item || data == NULL) {
                p->line.line.pen.color = GetOptionChoice(toggle_color_item);
            }
            if (data == toggle_pattern_item || data == NULL) {
                p->line.line.pen.pattern = GetOptionChoice(toggle_pattern_item);
            }
            if (data == symcolor_item || data == NULL) {
                p->sym.line.pen.color = GetOptionChoice(symcolor_item);
            }
            if (data == sympattern_item || data == NULL) {
                p->sym.line.pen.pattern = GetOptionChoice(sympattern_item);
            }
            if (data == symfillcolor_item || data == NULL) {
                p->sym.fillpen.color = GetOptionChoice(symfillcolor_item);
            }
            if (data == symfillpattern_item || data == NULL) {
                p->sym.fillpen.pattern = GetOptionChoice(symfillpattern_item);
            }
            if (data ==  dropline_item || data == NULL) {
                p->line.droplines = GetToggleButtonState(dropline_item);
            }
            if (data == baseline_item || data == NULL) {
                p->line.baseline = GetToggleButtonState(baseline_item);
            }
            if (data ==  baselinetype_item || data == NULL) {
                p->line.baseline_type = GetOptionChoice(baselinetype_item);
            }
            if (data == errbar_active_item || data == NULL) {
                p->errbar.active = GetToggleButtonState(errbar_active_item);
            }
            if (data == errbar_size_item || data == NULL) {
                p->errbar.barsize = GetCharSizeChoice(errbar_size_item);
            }
            if (data == errbar_width_item || data == NULL) {
                p->errbar.linew = GetSpinChoice(errbar_width_item);
            }
            if (data == errbar_lines_item || data == NULL) {
                p->errbar.lines = GetOptionChoice(errbar_lines_item);
            }
            if (data == errbar_riserlinew_item || data == NULL) {
                p->errbar.riser_linew = GetSpinChoice(errbar_riserlinew_item);
            }
            if (data == type_item || data == NULL) {
                set_dataset_type(pset, GetOptionChoice(type_item));
            }
            if (data == errbar_riserlines_item || data == NULL) {
                p->errbar.riser_lines = GetOptionChoice(errbar_riserlines_item);
            }
            if (data == errbar_ptype_item || data == NULL) {
                p->errbar.ptype = GetOptionChoice(errbar_ptype_item);
            }
            if (data == errbar_color_item || data == NULL) {
                p->errbar.pen.color = GetOptionChoice(errbar_color_item);
            }
            if (data == errbar_pattern_item || data == NULL) {
                p->errbar.pen.pattern = GetOptionChoice(errbar_pattern_item);
            }
            if (data == errbar_aclip_item || data == NULL) {
                p->errbar.arrow_clip = GetToggleButtonState(errbar_aclip_item);
            }
            if (data == errbar_cliplen_item || data == NULL) {
                p->errbar.cliplen = GetSpinChoice(errbar_cliplen_item);
            }
            if (data == avalue_active_item || data == NULL) {
                p->avalue.active = GetToggleButtonState(avalue_active_item);
            }
            if (data == avalue_type_item || data == NULL) {
                p->avalue.type = GetOptionChoice(avalue_type_item);
            }
            if (data == avalue_charsize_item || data == NULL) {
                p->avalue.size = GetCharSizeChoice(avalue_charsize_item);
            }
            if (data == avalue_font_item || data == NULL) {
                p->avalue.font = GetOptionChoice(avalue_font_item);
            }
            if (data == avalue_color_item || data == NULL) {
                p->avalue.color = GetOptionChoice(avalue_color_item);
            }
            if (data == avalue_angle_item || data == NULL) {
                p->avalue.angle = GetAngleChoice(avalue_angle_item);
            }
            if (data == avalue_format_item || data == NULL) {
                p->avalue.format = GetOptionChoice(avalue_format_item);
            }
            if (data == avalue_precision_item || data == NULL) {
                p->avalue.prec = GetOptionChoice(avalue_precision_item);
            }
            if (data == avalue_prestr || data == NULL) {
                strcpy(p->avalue.prestr, xv_getstr(avalue_prestr));
            }
            if (data == avalue_appstr || data == NULL) {
                strcpy(p->avalue.appstr, xv_getstr(avalue_appstr));
            }
            if (data == avalue_offsetx || data == NULL) {
                xv_evalexpr(avalue_offsetx, &p->avalue.offset.x);
            }
            if (data == avalue_offsety || data == NULL) {
                xv_evalexpr(avalue_offsety, &p->avalue.offset.y);
            }
        }
        xfree(selset);
    } 

    set_dirtystate();
    xdrawgraph();
    
    return RETURN_SUCCESS;
}


/*
 * freshen up symbol items, generally after a parameter
 * file has been read
 */
static void UpdateSymbols(Quark *pset)
{
    int i;
    char val[24];
    set *p;

    if (pset && cset == pset) {
        p = pset->data;
    
        SetOptionChoice(type_item, p->type);
        for (i = 0; i < type_item->nchoices; i++) {
            if (settype_cols(type_item->options[i].value)==
                                            settype_cols(p->type)) {
                SetSensitive(type_item->options[i].widget, True);
            } else {
                SetSensitive(type_item->options[i].widget, False);
            }
        }

        SetCharSizeChoice(symsize_item, p->sym.size);
        SetSpinChoice(symskip_item, p->symskip);
        sprintf(val, "%d", p->sym.symchar);
        xv_setstr(symchar_item, val);
        SetOptionChoice(toggle_symbols_item, p->sym.type);
        
        SetOptionChoice(symcolor_item, p->sym.line.pen.color);
        SetOptionChoice(sympattern_item, p->sym.line.pen.pattern);
        SetOptionChoice(symfillcolor_item, p->sym.fillpen.color);
        SetOptionChoice(symfillpattern_item, p->sym.fillpen.pattern);
        SetSpinChoice(symlinew_item, p->sym.line.width);
        SetOptionChoice(symlines_item, p->sym.line.style);
        
        SetOptionChoice(char_font_item, p->sym.charfont);        
        
        SetOptionChoice(toggle_color_item, p->line.line.pen.color);
        SetOptionChoice(toggle_pattern_item, p->line.line.pen.pattern);
        SetSpinChoice(toggle_width_item, p->line.line.width);
        SetToggleButtonState(dropline_item, p->line.droplines);
        SetOptionChoice(toggle_lines_item, p->line.line.style);
        SetOptionChoice(toggle_linet_item, p->line.type);
        SetOptionChoice(toggle_filltype_item, p->line.filltype);
        SetOptionChoice(toggle_fillrule_item, p->line.fillrule);
        SetOptionChoice(toggle_fillcol_item, p->line.fillpen.color);
        SetOptionChoice(toggle_fillpat_item, p->line.fillpen.pattern);
        
        SetToggleButtonState(baseline_item, p->line.baseline);
        SetOptionChoice(baselinetype_item, p->line.baseline_type);

        SetTextString(legend_str_item, p->legstr);
        
        SetToggleButtonState(errbar_active_item, p->errbar.active);
        
        switch (p->type) {
        case SET_XYDXDX:
        case SET_XYDYDY:
        case SET_XYDXDXDYDY:
            SetSensitive(errbar_ptype_item->options[2].widget, False);
            break;
        default:
            SetSensitive(errbar_ptype_item->options[2].widget, True);
            break;
        }
        SetOptionChoice(errbar_ptype_item, p->errbar.ptype);
        SetOptionChoice(errbar_color_item, p->errbar.pen.color);
        SetOptionChoice(errbar_pattern_item, p->errbar.pen.pattern);
        SetToggleButtonState(errbar_aclip_item, p->errbar.arrow_clip);
        SetSpinChoice(errbar_cliplen_item, p->errbar.cliplen);
        SetSpinChoice(errbar_width_item, p->errbar.linew);
        SetOptionChoice(errbar_lines_item, p->errbar.lines);
        SetSpinChoice(errbar_riserlinew_item, p->errbar.riser_linew);
        SetOptionChoice(errbar_riserlines_item, p->errbar.riser_lines);
        SetCharSizeChoice(errbar_size_item, p->errbar.barsize);

        SetToggleButtonState(avalue_active_item, p->avalue.active);
        SetOptionChoice(avalue_type_item, p->avalue.type);
        SetCharSizeChoice(avalue_charsize_item, p->avalue.size);
        SetOptionChoice(avalue_font_item, p->avalue.font);
        SetOptionChoice(avalue_color_item, p->avalue.color);
        SetAngleChoice(avalue_angle_item, p->avalue.angle);
        SetOptionChoice(avalue_format_item, p->avalue.format);
        SetOptionChoice(avalue_precision_item, p->avalue.prec);
        
        xv_setstr(avalue_prestr, p->avalue.prestr);
        xv_setstr(avalue_appstr, p->avalue.appstr);

        sprintf(val, "%f", p->avalue.offset.x);
        xv_setstr(avalue_offsetx, val);
        sprintf(val, "%f", p->avalue.offset.y);
        xv_setstr(avalue_offsety, val);
    }
}


static void set_cset_proc(int n, void **values, void *data)
{
    if (n == 1) {
        cset = values[0];
        UpdateSymbols(cset);
    }
}

void updatesymbols(Quark *pset)
{    
    if (setapp_dialog != NULL) { 
        if (SelectStorageChoice(toggle_symset_item, pset) == RETURN_SUCCESS) {
            cset = pset;
        }
    }
}


static void setapp_data_proc(void *data)
{
    int proc_type;
    int cd;
    int i;
    Quark *pset, **selset;
    set *p;
    int c = 0, bg = getbgcolor(grace->rt->canvas);
    
    proc_type = (int)data;

    cd = GetStorageChoices(toggle_symset_item, (void ***) &selset);
    if (cd < 1) {
        errmsg("No set selected");
        return;
    } else {
        for(i = 0; i < cd; i++) {
            pset = selset[i];
            if (!pset) {
                return;
            }
            p = pset->data;;
            switch (proc_type) {
            case SETAPP_STRIP_LEGENDS:
                set_legend_string(pset,
                    mybasename(get_legend_string(pset)));
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
                set_set_colors(pset, c);
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
                set_set_colors(pset, 1);
                break;
            }
        }
        
        xfree(selset);
        
        UpdateSymbols(cset);
        set_dirtystate();
        xdrawgraph();
    }
}

static void csync_cb(int value, void *data)
{
    int mask = (int)data;
    
    if (GetToggleButtonState(csync_item)!= TRUE) {
        return;
    }
    
    if (mask == CSYNC_LINE) {
        SetOptionChoice(symcolor_item, value);
        mask++;
    }
    if (mask == CSYNC_SYM) {
        SetOptionChoice(symfillcolor_item, value);
        SetOptionChoice(errbar_color_item, value);
        mask++;
    }
}
