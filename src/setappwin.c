/*
 * Grace - Graphics for Exploratory Data Analysis
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-95 Paul J Turner, Portland, OR
 * Copyright (c) 1996-98 GRACE Development Team
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
 * symbols and error bars
 *
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>

#include <Xm/Xm.h>
#include <Xm/DialogS.h>
#include <Xm/RowColumn.h>

#include "globals.h"

#include "graphs.h"
#include "utils.h"
#include "plotone.h"
#include "motifinc.h"
#include "protos.h"

#define cg get_cg()

int cset = 0;                   /* the current set from the symbols panel */

static Widget setapp_dialog = NULL;

static Widget *type_item;
static Widget *toggle_symbols_item;
static Widget symsize_item;
static Widget symskip_item;
static Widget *symcolor_item;
static OptionStructure sympattern_item;
static Widget *symfillcolor_item;
static OptionStructure symfillpattern_item;
static Widget *symlinew_item;
static OptionStructure symlines_item;
static Widget symchar_item;
static OptionStructure char_font_item;

static Widget *toggle_color_item;
static OptionStructure toggle_pattern_item;
static Widget *toggle_width_item;
static Widget dropline_item;
static OptionStructure toggle_lines_item;
static Widget *toggle_linet_item;
static Widget *toggle_filltype_item;
static Widget *toggle_fillrule_item;
static OptionStructure toggle_fillpat_item;
static Widget *toggle_fillcol_item;
static SetChoiceItem toggle_symset_item;
static Widget baseline_item;
static Widget *baselinetype_item;

static Widget legend_str_panel;

static Widget errbar_active_item;
static Widget errbar_size_item;
static Widget *errbar_width_item;
static OptionStructure errbar_lines_item;
static Widget *errbar_type_item;
static Widget *errbar_riserlinew_item;
static OptionStructure errbar_riserlines_item;

static Widget avalue_active_item;
static Widget *avalue_type_item;
static OptionStructure avalue_font_item;
static Widget *avalue_color_item;
static Widget avalue_charsize_item ;
static Widget avalue_angle_item;
static Widget *avalue_format_item;
static Widget *avalue_precision_item;
static Widget avalue_offsetx;
static Widget avalue_offsety;
static Widget avalue_prestr;
static Widget avalue_appstr;

static void setapp_aac_cb(Widget w, XtPointer client_data, XtPointer call_data);
static void setall_colors_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void setall_sym_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void setall_linew_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void set_cset_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void setall_linesty_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void set_bw_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void strip_leg_proc( Widget w, XtPointer client_data, XtPointer call_data);

static void UpdateSymbols(int gno, int value);

/*
 * create the symbols popup
 */
void define_symbols_popup(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget setapp_panel, setapp_tab, setapp_main, setapp_symbols, 
           setapp_line, setapp_errbar, setapp_avalue, rc_head, fr, rc, rc1, rc2;
    Widget menubar, menupane, cascade;

    set_wait_cursor();
    if (setapp_dialog == NULL) {
        setapp_dialog = XmCreateDialogShell(app_shell, "SetAppearance", NULL, 0);
        handle_close(setapp_dialog);
        setapp_panel = XtVaCreateWidget("setapp_panel", xmFormWidgetClass, 
                                          setapp_dialog, NULL, 0);

        menubar = CreateMenuBar(setapp_panel, "setappMenuBar", NULL);
        
        menupane = CreateMenu(menubar, "setappFileMenu", "File", 'F', NULL, NULL);
        CreateMenuButton(menupane, "close", "Close", 'C',
            (XtCallbackProc) setapp_aac_cb, (XtPointer) AAC_CLOSE, NULL);

        menupane = CreateMenu(menubar, "setappDataMenu", "Data", 'D', NULL, NULL);

        CreateMenuButton(menupane, "allColors", "All colors", 'c',
            (XtCallbackProc) setall_colors_proc, (XtPointer) NULL, 0);
        CreateMenuButton(menupane, "allSymbols", "All symbols", 's',
            (XtCallbackProc) setall_sym_proc, (XtPointer) NULL, 0);
        CreateMenuButton(menupane, "allLineWidths", "All line widths", 'w',
            (XtCallbackProc) setall_linew_proc, (XtPointer) NULL, 0);
        CreateMenuButton(menupane, "allLineStyles", "All line styles", 'y',
            (XtCallbackProc) setall_linesty_proc, (XtPointer) NULL, 0);
        CreateMenuButton(menupane, "setBW", "Black & white", 'B',
            (XtCallbackProc) set_bw_proc, (XtPointer) NULL, 0);
        CreateMenuSeparator(menupane, "sep");
        CreateMenuButton(menupane, "loadComments", "Load comments", 'm',
            (XtCallbackProc) legend_load_proc, (XtPointer) NULL, 0);
        CreateMenuButton(menupane, "stripLabels", "Strip labels", 'l',
            (XtCallbackProc) strip_leg_proc, (XtPointer) NULL, 0);
        
        
        menupane = CreateMenu(menubar, "setappOptionsMenu", "Options", 'O', NULL, NULL);
      
        /* nonl_autol_item =  */CreateMenuToggle(menupane, "masterSwitch", "Master switch (N/I)", 'M',
            (XtCallbackProc) NULL, (XtPointer) NULL, NULL);

        menupane = CreateMenu(menubar, "nonlHelpMenu", "Help", 'H', &cascade, NULL);
        XtVaSetValues(menubar, XmNmenuHelpWidget, cascade, NULL);

        CreateMenuButton(menupane, "onSetAppearance", "On set appearance", 's',
            (XtCallbackProc) HelpCB, (XtPointer) NULL, 0);

        CreateMenuButton(menupane, "onContext", "On context", 'x',
            (XtCallbackProc) ContextHelpCB, (XtPointer) NULL, 0);
        
        XtManageChild(menubar);
        XtVaSetValues(menubar,
                      XmNtopAttachment, XmATTACH_FORM,
                      XmNleftAttachment, XmATTACH_FORM,
                      XmNrightAttachment, XmATTACH_FORM,
                      NULL);



        rc_head = XmCreateRowColumn(setapp_panel, "rc_head", NULL, 0);

        toggle_symset_item = CreateSetSelector(rc_head, "Select set:",
                                             SET_SELECT_ACTIVE,
                                             FILTER_SELECT_NONE,
                                             GRAPH_SELECT_CURRENT,
                                             SELECTION_TYPE_MULTIPLE);

        XtAddCallback(toggle_symset_item.list, XmNextendedSelectionCallback,
                      (XtCallbackProc) set_cset_proc, (XtPointer) 0);

        XtManageChild(rc_head);
        XtVaSetValues(rc_head,
                      XmNtopAttachment, XmATTACH_WIDGET,
                      XmNtopWidget, menubar,
                      XmNleftAttachment, XmATTACH_FORM,
                      XmNrightAttachment, XmATTACH_FORM,
                      NULL);


        /* ------------ Tabs --------------*/

        setapp_tab = CreateTab(setapp_panel);        


        /* ------------ Main tab --------------*/
        
        setapp_main = CreateTabPage(setapp_tab, "Main");

        fr = CreateFrame(setapp_main, "Set presentation");
        rc = XmCreateRowColumn(fr, "rc", NULL, 0);
	type_item = CreatePanelChoice(rc, "Type:", 14,
				      "XY",
				      "XY DX",
				      "XY DY",
				      "XY DX1 DX2",
				      "XY DY1 DY2",
				      "XY DX DY",
				      "BAR",
				      "BAR DY",
				      "BAR DY DY",
				      "XY STRING",
				      "XY HILO",
				      "XY Z",
				      "XY RADIUS",
				      NULL, 0);
        XtManageChild(rc);

        rc2 = XmCreateRowColumn(setapp_main, "rc", NULL, 0);
        XtVaSetValues(rc2, XmNorientation, XmHORIZONTAL, NULL);


        fr = CreateFrame(rc2, "Symbol properties");
        rc = XtVaCreateWidget("symbolsbb", xmRowColumnWidgetClass, fr, NULL);
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

        symsize_item = CreateCharSizeChoice(rc, "Size");
        symcolor_item = CreateColorChoice(rc, "Color:");
        symchar_item = CreateTextItem2(rc, 3, "Symbol char:");
        XtManageChild(rc);

        fr = CreateFrame(rc2, "Line properties");
        rc = XtVaCreateWidget("linesrc", xmRowColumnWidgetClass, fr, NULL);
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
        toggle_lines_item = CreateLineStyleChoice(rc, "Style:");
        toggle_width_item = CreateLineWidthChoice(rc, "Width:");
        toggle_color_item = CreateColorChoice(rc, "Color:");
        XtManageChild(rc);
        
        XtManageChild(rc2);

        fr = CreateFrame(setapp_main, "Legend");
        rc = XmCreateRowColumn(fr, "rc", NULL, 0);
        legend_str_panel = CreateTextItem2(rc, 33, "String:");
        XtManageChild(rc);

        fr = CreateFrame(setapp_main, "Display options");
        rc2 = XmCreateRowColumn(fr, "rc", NULL, 0);
        XtVaSetValues(rc2, XmNorientation, XmHORIZONTAL, NULL);
        avalue_active_item = CreateToggleButton(rc2, "Annotate values");
        errbar_active_item = CreateToggleButton(rc2, "Display error bars");
        XtManageChild(rc2);


        /* ------------ Symbols tab --------------*/
        
        setapp_symbols = CreateTabPage(setapp_tab, "Symbols");

        fr = CreateFrame(setapp_symbols, "Symbol outline");
        rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, fr, NULL);

        rc2 = XmCreateRowColumn(rc, "rc", NULL, 0);
        XtVaSetValues(rc2, XmNorientation, XmHORIZONTAL, NULL);
        symlines_item = CreateLineStyleChoice(rc2, "Style:");
        symlinew_item = CreateLineWidthChoice(rc2, "Width:");
        XtManageChild(rc2);
        sympattern_item = CreatePatternChoice(rc, "Pattern:");
        XtManageChild(rc);

        fr = CreateFrame(setapp_symbols, "Symbol fill");
        rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, fr, NULL);
        XtVaSetValues(rc, XmNorientation, XmHORIZONTAL, NULL);
        symfillcolor_item = CreateColorChoice(rc, "Color:");
        symfillpattern_item = CreatePatternChoice(rc, "Pattern:");
        XtManageChild(rc);

        fr = CreateFrame(setapp_symbols, "Extra");
        rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, fr, NULL);
        symskip_item = CreateTextItem2(rc, 4, "Symbol skip:");
        char_font_item = CreateFontChoice(rc, "Font for char symbol:");
        XtManageChild(rc);


        /* ------------ Line tab --------------*/
        
        setapp_line = CreateTabPage(setapp_tab, "Line");

        fr = CreateFrame(setapp_line, "Line properties");
        rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, fr, NULL);
        XtVaSetValues(rc, XmNorientation, XmHORIZONTAL, NULL);
        toggle_pattern_item = CreatePatternChoice(rc, "Pattern:");
        dropline_item = CreateToggleButton(rc, "Draw drop lines");
        XtManageChild(rc);

        fr = CreateFrame(setapp_line, "Fill properties");
        rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, fr, NULL);
        rc2 = XmCreateRowColumn(rc, "rc", NULL, 0);
        XtVaSetValues(rc2, XmNorientation, XmHORIZONTAL, NULL);
        toggle_filltype_item = CreatePanelChoice(rc2, "Type:",
                                             4,
                                             "None",
                                             "As polygon",
                                             "To baseline",
                                             NULL,
                                             0);
        toggle_fillrule_item = CreatePanelChoice(rc2, "Rule:",
                                             3,
                                             "Winding",
                                             "Even-Odd",
                                             NULL,
                                             0);
        XtManageChild(rc2);
        rc2 = XmCreateRowColumn(rc, "rc", NULL, 0);
        XtVaSetValues(rc2, XmNorientation, XmHORIZONTAL, NULL);
        toggle_fillpat_item = CreatePatternChoice(rc2, "Pattern:");
        toggle_fillcol_item = CreateColorChoice(rc2, "Color:");
        XtManageChild(rc2);
        XtManageChild(rc);
        
        fr = CreateFrame(setapp_line, "Base line");
        rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, fr, NULL);
        XtVaSetValues(rc, XmNorientation, XmHORIZONTAL, NULL);
        baselinetype_item = CreatePanelChoice(rc, "Type:",
                                             6,
                                             "Zero",
                                             "Set min",
                                             "Set max",
                                             "Graph min",
                                             "Graph max",
                                             NULL,
                                             0);
        baseline_item = CreateToggleButton(rc, "Draw line");
        XtManageChild(rc);
        
        
        /* ------------ AValue tab --------------*/
        
        setapp_avalue = CreateTabPage(setapp_tab, "Ann. values");

	fr = CreateFrame(setapp_avalue, "Text properties");
	rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, fr, NULL);
        
        rc2 = XtVaCreateWidget("rc", xmRowColumnWidgetClass, rc, NULL);
	XtVaSetValues(rc2, XmNorientation, XmHORIZONTAL, NULL);
	avalue_font_item = CreateFontChoice(rc2, "Font:");
	avalue_charsize_item = CreateCharSizeChoice(rc2, "Char size");
	XtVaSetValues(avalue_charsize_item, XmNscaleWidth, 120, NULL);
	XtManageChild(rc2);
        
        rc2 = XtVaCreateWidget("rc", xmRowColumnWidgetClass, rc, NULL);
	XtVaSetValues(rc2, XmNorientation, XmHORIZONTAL, NULL);
	avalue_color_item = CreateColorChoice(rc2, "Color:");
	avalue_angle_item = CreateAngleChoice(rc2, "Angle");
	XtVaSetValues(avalue_angle_item, XmNscaleWidth, 180, NULL);
	XtManageChild(rc2);

        rc2 = XmCreateRowColumn(rc, "rc", NULL, 0);
	XtVaSetValues(rc2, XmNorientation, XmHORIZONTAL, NULL);
        avalue_prestr = CreateTextItem2(rc2, 10, "Prepend:");
        avalue_appstr = CreateTextItem2(rc2, 10, "Append:");
	XtManageChild(rc2);
        
        XtManageChild(rc);
        
	fr = CreateFrame(setapp_avalue, "Format options");
	rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, fr, NULL);
        rc2 = XtVaCreateWidget("rc", xmRowColumnWidgetClass, rc, NULL);
	XtVaSetValues(rc2, XmNorientation, XmHORIZONTAL, NULL);
	avalue_format_item = CreateFormatChoice(rc, "Format:");
        avalue_type_item = CreatePanelChoice(rc2, "Type:",
                                             6,
                                             "None",
                                             "X",
                                             "Y",
                                             "X, Y",
                                             "String",
                                             NULL,
                                             0);
	avalue_precision_item = CreatePrecisionChoice(rc2, "Precision:");
	XtManageChild(rc2);
        
        XtManageChild(rc);


	fr = CreateFrame(setapp_avalue, "Placement");
        rc2 = XmCreateRowColumn(fr, "rc", NULL, 0);
        XtVaSetValues(rc2, XmNorientation, XmHORIZONTAL, NULL);
        avalue_offsetx = CreateTextItem2(rc2, 10, "X offset:");
        avalue_offsety = CreateTextItem2(rc2, 10, "Y offset:");
        XtManageChild(rc2);
        

        /* ------------ Errbar tab --------------*/
        
        setapp_errbar = CreateTabPage(setapp_tab, "Error bars");

        rc2 = XmCreateRowColumn(setapp_errbar, "rc", NULL, 0);
        XtVaSetValues(rc2, XmNorientation, XmHORIZONTAL, NULL);

        fr = CreateFrame(rc2, "Bar line");
        rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, fr, NULL);
        errbar_size_item = CreateCharSizeChoice(rc, "Size");
        errbar_width_item = CreateLineWidthChoice(rc, "Width:");
        errbar_lines_item = CreateLineStyleChoice(rc, "Style:");
        XtManageChild(rc);

        rc1 = XtVaCreateWidget("rc", xmRowColumnWidgetClass, rc2, NULL);
        fr = CreateFrame(rc1, "Placement");
        rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, fr, NULL);
        errbar_type_item = CreatePanelChoice(rc,
                                             "Type:",
                                             4,
                                             "Both",
                                             "Top/left",
                                             "Bottom/right",
                                             NULL,
                                             0);
        XtManageChild(rc);
        
        fr = CreateFrame(rc1, "Riser line");
        rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, fr, NULL);
        errbar_riserlinew_item = CreateLineWidthChoice(rc, "Width:");
        errbar_riserlines_item = CreateLineStyleChoice(rc, "Style:");
        XtManageChild(rc);
        XtManageChild(rc1);
        
        XtManageChild(rc2);

        SelectTabPage(setapp_tab, setapp_main);


        fr = CreateFrame(setapp_panel, NULL); 
        rc = XmCreateRowColumn(fr, "rc", NULL, 0);

        CreateAACButtons(rc, setapp_panel, setapp_aac_cb);
        
        XtManageChild(rc);
        XtManageChild(fr);
        XtVaSetValues(fr,
                      XmNtopAttachment, XmATTACH_NONE,
                      XmNleftAttachment, XmATTACH_FORM,
                      XmNrightAttachment, XmATTACH_FORM,
                      XmNbottomAttachment, XmATTACH_FORM,
                      NULL);
        XtVaSetValues(setapp_tab,
                      XmNtopAttachment, XmATTACH_WIDGET,
                      XmNtopWidget, rc_head,
                      XmNleftAttachment, XmATTACH_FORM,
                      XmNrightAttachment, XmATTACH_FORM,
                      XmNbottomAttachment, XmATTACH_WIDGET,
                      XmNbottomWidget, fr,
                      NULL);

        XtManageChild(setapp_panel);
    }
    
    updatesymbols(cg, cset);
    XtRaise(setapp_dialog);
    unset_wait_cursor();
}

/*
 * define symbols for the current set
 */
static void setapp_aac_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
    int aac_mode;
    int i;
    int type;
    int sym, symskip, symlinew, symlines;
    int line, linet, color, pattern, wid;
    int dropline, filltype, fillrule, fillpat, fillcol;
    int symcolor, sympattern, symfillcolor, symfillpattern;
    double symsize;
    int etype_tmp, errbar_type;
    int baseline, baselinetype;
    Errbar errbar;
    AValue avalue;
    char symchar;
    int charfont;
    plotarr p;
    
    int *selset, cd;

    aac_mode = (int) client_data;
    
    if (aac_mode == AAC_CLOSE) {
        XtUnmanageChild(setapp_dialog);
        return;
    }

    type = GetChoice(type_item);
    symsize = GetCharSizeChoice(symsize_item);
    sym = GetChoice(toggle_symbols_item);
    color = GetChoice(toggle_color_item);
    pattern = GetOptionChoice(toggle_pattern_item);
    wid = GetChoice(toggle_width_item);
    baseline = GetToggleButtonState(baseline_item);
    baselinetype = GetChoice(baselinetype_item);
    dropline = GetToggleButtonState(dropline_item);
    line = GetOptionChoice(toggle_lines_item);
    linet = GetChoice(toggle_linet_item);
    filltype = GetChoice(toggle_filltype_item);
    fillrule = GetChoice(toggle_fillrule_item);
    fillpat = GetOptionChoice(toggle_fillpat_item);
    fillcol = GetChoice(toggle_fillcol_item);
    xv_evalexpri(symskip_item, &symskip);
    symcolor = GetChoice(symcolor_item);
    sympattern = GetOptionChoice(sympattern_item);
    symfillcolor = GetChoice(symfillcolor_item);
    symfillpattern = GetOptionChoice(symfillpattern_item);
    symlinew = GetChoice(symlinew_item);
    symlines = GetOptionChoice(symlines_item);
    symchar = atoi(xv_getstr(symchar_item));
    charfont = GetOptionChoice(char_font_item);
    
    errbar.active = GetToggleButtonState(errbar_active_item);
    errbar.length = GetCharSizeChoice(errbar_size_item);
    errbar.linew = GetChoice(errbar_width_item);
    errbar.lines = GetOptionChoice(errbar_lines_item);
    errbar.riser_linew = GetChoice(errbar_riserlinew_item);
    errbar.riser_lines = GetOptionChoice(errbar_riserlines_item);
        
    avalue.active = GetToggleButtonState(avalue_active_item);
    avalue.type = GetChoice(avalue_type_item);
    avalue.size = GetCharSizeChoice(avalue_charsize_item);
    avalue.font = GetOptionChoice(avalue_font_item);
    avalue.color = GetChoice(avalue_color_item);
    avalue.angle = GetAngleChoice(avalue_angle_item);
    avalue.format = GetChoice(avalue_format_item);
    avalue.prec = GetChoice(avalue_precision_item);
    strcpy(avalue.prestr, xv_getstr(avalue_prestr));
    strcpy(avalue.appstr, xv_getstr(avalue_appstr));
    xv_evalexpr(avalue_offsetx, &avalue.offset.x );
    xv_evalexpr(avalue_offsety, &avalue.offset.y);
                    
    cd = GetSelectedSets(toggle_symset_item, &selset);
    if (cd == SET_SELECT_ERROR) {
        errwin("No set selected");
        return;
    } else {
        for(i = 0; i < cd; i++) {
            cset = selset[i];
            get_graph_plotarr(get_cg(), cset, &p);
            p.type = type;
            p.symskip = symskip;
            p.symsize = symsize;
            p.symlinew = symlinew;
            p.symlines = symlines;
            p.symchar = symchar;
            p.charfont = charfont;
            p.filltype = filltype;
            p.fillrule = fillrule;
            p.setfillpen.pattern = fillpat;
            p.setfillpen.color = fillcol;
            strcpy(p.lstr, xv_getstr(legend_str_panel));
            p.sym = sym;
            p.linet = linet;
            p.lines = line;
            p.linew = wid;
            p.linepen.color = color;
            p.linepen.pattern = pattern;
            p.sympen.color = symcolor;
            p.sympen.pattern = sympattern;
            p.symfillpen.color = symfillcolor;
            p.symfillpen.pattern = symfillpattern;
            p.dropline = dropline;
            p.baseline = baseline;
            p.baseline_type = baselinetype;

            etype_tmp = GetChoice(errbar_type_item);
            switch (dataset_type(cg, cset)) {
            case SET_XYDX:
            case SET_XYDXDX:
                if (etype_tmp == 0) {
                    errbar_type = PLACE_BOTH;;
                } else if (etype_tmp == 1) {
                    errbar_type = PLACE_LEFT;
                } else {
                    errbar_type = PLACE_RIGHT;
                }
                break;
            case SET_XYDY:
            case SET_XYDYDY:
            case SET_BARDY:
            case SET_BARDYDY:
                if (etype_tmp == 0) {
                    errbar_type = PLACE_BOTH;;
                } else if (etype_tmp == 1) {
                    errbar_type = PLACE_TOP;
                } else {
                    errbar_type = PLACE_BOTTOM;
                }
                break;
            default:
                errbar_type = PLACE_BOTH;
                break;
            }
            errbar.type = errbar_type;
    
            p.errbar = errbar;
            p.avalue = avalue;
        }
        set_graph_plotarr(get_cg(), cset, &p);
    } 

    if (aac_mode == AAC_ACCEPT) {
        XtUnmanageChild(setapp_dialog);
    }

    set_dirtystate();
    drawgraph();
}


/*
 * freshen up symbol items, generally after a parameter
 * file has been read
 */
static void UpdateSymbols(int gno, int value)
{
    int itmp;
    char val[24];
    plotarr p;

    if ((cset == value) && (value != -1)) {
    
        get_graph_plotarr(gno, cset, &p);
        SetChoice(type_item, p.type);
        SetCharSizeChoice(symsize_item, p.symsize);
        sprintf(val, "%d", p.symskip);
        xv_setstr(symskip_item, val);
        sprintf(val, "%d", p.symchar);
        xv_setstr(symchar_item, val);
        SetChoice(toggle_symbols_item, p.sym);
        
        SetChoice(symcolor_item, p.sympen.color);
        SetOptionChoice(sympattern_item, p.sympen.pattern);
        SetChoice(symfillcolor_item, p.symfillpen.color);
        SetOptionChoice(symfillpattern_item, p.symfillpen.pattern);
        SetChoice(symlinew_item, p.symlinew);
        SetOptionChoice(symlines_item, p.symlines);
        
        SetOptionChoice(char_font_item, p.charfont);        
        
        SetChoice(toggle_color_item, p.linepen.color);
        SetOptionChoice(toggle_pattern_item, p.linepen.pattern);
        SetChoice(toggle_width_item, p.linew);
        SetToggleButtonState(dropline_item, p.dropline);
        SetOptionChoice(toggle_lines_item, p.lines);
        SetChoice(toggle_linet_item, p.linet);
        SetChoice(toggle_filltype_item, p.filltype);
        SetChoice(toggle_fillrule_item, p.fillrule);
        SetChoice(toggle_fillcol_item, p.setfillpen.color);
        SetOptionChoice(toggle_fillpat_item, p.setfillpen.pattern);
        
        SetToggleButtonState(baseline_item, p.baseline);
        SetChoice(baselinetype_item, p.baseline_type);

        xv_setstr(legend_str_panel, p.lstr);
        
        switch (p.errbar.type) {
        case PLACE_BOTH:
            itmp = 0;
            break;
        case PLACE_TOP:
        case PLACE_LEFT:
            itmp = 1;
            break;
        case PLACE_BOTTOM:
        case PLACE_RIGHT:
            itmp = 2;
            break;
        default:
            itmp = 0;
            break;
        }
        SetToggleButtonState(errbar_active_item, p.errbar.active);
        SetChoice(errbar_type_item, itmp);
        SetChoice(errbar_width_item, p.errbar.linew);
        SetOptionChoice(errbar_lines_item, p.errbar.lines);
        SetChoice(errbar_riserlinew_item, p.errbar.riser_linew);
        SetOptionChoice(errbar_riserlines_item, p.errbar.riser_lines);
        SetCharSizeChoice(errbar_size_item, p.errbar.length);

        SetToggleButtonState(avalue_active_item, p.avalue.active);
        SetChoice(avalue_type_item, p.avalue.type);
        SetCharSizeChoice(avalue_charsize_item, p.avalue.size);
        SetOptionChoice(avalue_font_item, p.avalue.font);
        SetChoice(avalue_color_item, p.avalue.color);
        SetAngleChoice(avalue_angle_item, p.avalue.angle);
        SetChoice(avalue_format_item, p.avalue.format);
        SetChoice(avalue_precision_item, p.avalue.prec);
        
        xv_setstr(avalue_prestr, p.avalue.prestr);
        xv_setstr(avalue_appstr, p.avalue.appstr);

        sprintf(val, "%f", p.avalue.offset.x);
        xv_setstr(avalue_offsetx, val);
        sprintf(val, "%f", p.avalue.offset.y);
        xv_setstr(avalue_offsety, val);
        
/*
 *         set_graph_plotarr(gno, cset, &p);
 */
   }
}


static void set_cset_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int cd;
    int *selsets;
        
    cd = GetSelectedSets(toggle_symset_item, &selsets);
    if (cd != SET_SELECT_ERROR) {
        cset = selsets[0];
        UpdateSymbols(cg, cset);
    }
}

void updatesymbols(int gno, int setno)
{
    int cd;
    
    if (setapp_dialog != NULL) { 
        cd = SetSelectedSet(gno, setno, toggle_symset_item);
        if (cd != SET_SELECT_ERROR) {
            cset = setno;
        }
    }
}

/*
 * legends
 */

/*
 * strip leading pathname from comments
 */
static void strip_leg_proc( Widget w, XtPointer client_data, XtPointer call_data)
{
    int i;

    for (i = 0; i < number_of_sets(cg); i++) {
        if ( is_set_active(cg, i) ) {
            set_legend_string(cg, i, mybasename(get_legend_string(cg, i)) );
        }
    }
}

/*
 * load legend strings from set comments
 */
void legend_load_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int i;

    for (i = 0; i < number_of_sets(cg); i++) {
        if (is_set_active(cg, i)) {
            load_comments_to_legend(cg, i);
        }
    }
    
    xv_setstr(legend_str_panel, get_legend_string(cg, cset));

    set_dirtystate();
    drawgraph();
}



/*
 * define colors incrementally
 */
void setall_colors_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int i, c, bg;

    bg = getbgcolor();
    c = 0;
    for (i = 0; i < number_of_sets(cg); i++) {
        if (is_set_active(cg, i)) {
            while (c == bg || get_colortype(c) != COLOR_MAIN) {
                c++;
                c %= number_of_colors();
            }
            set_set_colors(cg, i, c);
            c++;
        }
    }
    UpdateSymbols(cg, cset);
    set_dirtystate();
    drawgraph();
}

/*
 * define symbols incrementally mod 10
 */
static void setall_sym_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int i;
    plotarr p;

    for (i = 0; i < number_of_sets(cg); i++) {
        if (is_set_active(cg, i)) {
            get_graph_plotarr(cg, i, &p);
            p.sym = (i % 10) + 1;
            set_graph_plotarr(cg, i, &p);
        }
    }
    UpdateSymbols(cg, cset);
    set_dirtystate();
    drawgraph();
}

/*
 * define linewidths incrementally mod 7
 */
static void setall_linew_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int i;
    plotarr p;

    for (i = 0; i < number_of_sets(cg); i++) {
        if (is_set_active(cg, i)) {
            get_graph_plotarr(cg, i, &p);
            p.linew = (i % 7) + 1;
            set_graph_plotarr(cg, i, &p);
        }
    }
    UpdateSymbols(cg, cset);
    set_dirtystate();
    drawgraph();
}

/*
 * define line styles incrementally mod 5
 */
static void setall_linesty_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int i;
    plotarr p;

    for (i = 0; i < number_of_sets(cg); i++) {
        if (is_set_active(cg, i)) {
            get_graph_plotarr(cg, i, &p);
            p.lines = (i % (number_of_linestyles() - 1)) + 1;
            set_graph_plotarr(cg, i, &p);
        }
    }
    UpdateSymbols(cg, cset);
    set_dirtystate();
    drawgraph();
}

/*
 * make all lines black
 */
static void set_bw_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int i;

    for (i = 0; i < number_of_sets(cg); i++) {
        if (is_set_active(cg, i)) {
            set_set_colors(cg, i, 1);
        }
    }
    UpdateSymbols(cg, cset);
    set_dirtystate();
    drawgraph();
}
