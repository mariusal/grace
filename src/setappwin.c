/*
 * Grace - Graphics for Exploratory Data Analysis
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-95 Paul J Turner, Portland, OR
 * Copyright (c) 1996-99 Grace Development Team
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
#include <Xm/Form.h>
#include <Xm/DialogS.h>
#include <Xm/RowColumn.h>

#include "globals.h"

#include "graphs.h"
#include "utils.h"
#include "plotone.h"
#include "motifinc.h"
#include "protos.h"

#define cg get_cg()

#define SETAPP_STRIP_LEGENDS    0
#define SETAPP_LOAD_COMMENTS    1
#define SETAPP_ALL_COLORS       2
#define SETAPP_ALL_SYMBOLS      3
#define SETAPP_ALL_LINEW        4
#define SETAPP_ALL_LINES        5
#define SETAPP_ALL_BW           6

static int cset = 0;            /* the current set from the symbols panel */

static Widget setapp_dialog = NULL;

static Widget duplegs_item;

static OptionStructure *type_item;
static Widget *toggle_symbols_item;
static Widget symsize_item;
static Widget symskip_item;
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
static Widget *toggle_linet_item;
static Widget *toggle_filltype_item;
static Widget *toggle_fillrule_item;
static OptionStructure *toggle_fillpat_item;
static OptionStructure *toggle_fillcol_item;
static ListStructure *toggle_symset_item;
static Widget baseline_item;
static Widget *baselinetype_item;

static TextStructure *legend_str_item;

static Widget errbar_active_item;
static Widget *errbar_ptype_item;
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
static Widget *avalue_type_item;
static OptionStructure *avalue_font_item;
static OptionStructure *avalue_color_item;
static Widget avalue_charsize_item ;
static Widget avalue_angle_item;
static Widget *avalue_format_item;
static Widget *avalue_precision_item;
static Widget avalue_offsetx;
static Widget avalue_offsety;
static Widget avalue_prestr;
static Widget avalue_appstr;

static void UpdateSymbols(int gno, int value);
static void set_cset_proc(int n, int *values, void *data);
static void setapp_aac_cb(void *data);
static void setapp_data_proc(void *data);

/*
 * create the symbols popup
 */
void define_symbols_popup(void *data)
{
    int setno;
    Widget setapp_panel, setapp_tab, setapp_main, setapp_symbols, 
           setapp_line, setapp_errbar, setapp_avalue, rc_head, fr, rc, rc1, rc2;
    Widget menubar, menupane;

    set_wait_cursor();
    
    setno = (int) data;
    if (is_valid_setno(cg, setno) == TRUE) {
        cset = setno;
    }
    
    if (setapp_dialog == NULL) {
        setapp_dialog = XmCreateDialogShell(app_shell, "SetAppearance", NULL, 0);
        handle_close(setapp_dialog);
        setapp_panel = XtVaCreateWidget("setapp_panel", xmFormWidgetClass, 
                                          setapp_dialog, NULL, 0);

        menubar = CreateMenuBar(setapp_panel);
        
        menupane = CreateMenu(menubar, "File", 'F', FALSE);
        CreateMenuButton(menupane, "Close", 'C',
            setapp_aac_cb, (void *) AAC_CLOSE);

        menupane = CreateMenu(menubar, "Edit", 'E', FALSE);

        CreateMenuButton(menupane, "Set different colors", 'c',
            setapp_data_proc, (void *) SETAPP_ALL_COLORS);
        CreateMenuButton(menupane, "Set different symbols", 's',
            setapp_data_proc, (void *) SETAPP_ALL_SYMBOLS);
        CreateMenuButton(menupane, "Set different line widths", 'w',
            setapp_data_proc, (void *) SETAPP_ALL_LINEW);
        CreateMenuButton(menupane, "Set different line styles", 'y',
            setapp_data_proc, (void *) SETAPP_ALL_LINES);
        CreateMenuButton(menupane, "Set black & white", 'B',
            setapp_data_proc, (void *) SETAPP_ALL_BW);
        CreateMenuSeparator(menupane);
        CreateMenuButton(menupane, "Load comments", 'm',
            setapp_data_proc, (void *) SETAPP_LOAD_COMMENTS);
        CreateMenuButton(menupane, "Strip legends", 'l',
            setapp_data_proc, (void *) SETAPP_STRIP_LEGENDS);
        
        
        menupane = CreateMenu(menubar, "Options", 'O', FALSE);
      
        duplegs_item = CreateMenuToggle(menupane,
            "Duplicate legends", 'D', NULL, NULL);
        CreateMenuToggle(menupane, "Color sync (N/I)", 's', NULL, NULL);

        menupane = CreateMenu(menubar, "Help", 'H', TRUE);

        CreateMenuButton(menupane, "On set appearance", 's', HelpCB, NULL);
        
        XtManageChild(menubar);
        XtVaSetValues(menubar,
                      XmNtopAttachment, XmATTACH_FORM,
                      XmNleftAttachment, XmATTACH_FORM,
                      XmNrightAttachment, XmATTACH_FORM,
                      NULL);


        rc_head = XmCreateRowColumn(setapp_panel, "rc_head", NULL, 0);

        toggle_symset_item = CreateSetChoice(rc_head, "Select set:",
                                                LIST_TYPE_MULTIPLE, TRUE);

        AddListChoiceCB(toggle_symset_item, set_cset_proc, NULL);

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
	type_item = CreateSetTypeChoice(fr, "Type:");

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
        legend_str_item = CreateCSText(fr, "String:");

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
                                             7,
                                             "None",
                                             "X",
                                             "Y",
                                             "X, Y",
                                             "String",
                                             "Z",
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

        rc1 = XtVaCreateWidget("rc", xmRowColumnWidgetClass, rc2, NULL);

        fr = CreateFrame(rc1, "Common");
        rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, fr, NULL);
        errbar_ptype_item = CreatePanelChoice(rc,
                                             "Placement:",
                                             4,
                                             "Normal",
                                             "Opposite",
                                             "Both",
                                             NULL,
                                             0);
	errbar_color_item = CreateColorChoice(rc, "Color:");
	errbar_pattern_item = CreatePatternChoice(rc, "Pattern:");
        XtManageChild(rc);

        fr = CreateFrame(rc1, "Clipping");
        rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, fr, NULL);
	errbar_aclip_item = CreateToggleButton(rc, "Arrow clip");
	errbar_cliplen_item = CreateSpinChoice(rc, "Max riser length",
            3, SPIN_TYPE_FLOAT, 0.0, 10.0, 0.1);
        XtManageChild(rc);

        XtManageChild(rc1);

        rc1 = XtVaCreateWidget("rc", xmRowColumnWidgetClass, rc2, NULL);

        fr = CreateFrame(rc1, "Bar line");
        rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, fr, NULL);
        errbar_size_item = CreateCharSizeChoice(rc, "Size");
        errbar_width_item = CreateLineWidthChoice(rc, "Width:");
        errbar_lines_item = CreateLineStyleChoice(rc, "Style:");
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
        CreateAACButtons(fr, setapp_panel, setapp_aac_cb);
        
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
static void setapp_aac_cb(void *data)
{
    int aac_mode;
    int i;
    int duplegs;
    int type;
    int sym, symskip, symlines;
    double symlinew;
    int line, linet, color, pattern;
    double wid;
    int dropline, filltype, fillrule, fillpat, fillcol;
    int symcolor, sympattern, symfillcolor, symfillpattern;
    double symsize;
    int baseline, baselinetype;
    Errbar errbar;
    AValue avalue;
    char symchar;
    int charfont;
    plotarr p;
    
    int setno;
    int *selset, cd;
    
    aac_mode = (int) data;
    
    if (aac_mode == AAC_CLOSE) {
        XtUnmanageChild(setapp_dialog);
        return;
    }

    duplegs = GetToggleButtonState(duplegs_item);

    type = GetOptionChoice(type_item);
    symsize = GetCharSizeChoice(symsize_item);
    sym = GetChoice(toggle_symbols_item);
    color = GetOptionChoice(toggle_color_item);
    pattern = GetOptionChoice(toggle_pattern_item);
    wid = GetSpinChoice(toggle_width_item);
    baseline = GetToggleButtonState(baseline_item);
    baselinetype = GetChoice(baselinetype_item);
    dropline = GetToggleButtonState(dropline_item);
    line = GetOptionChoice(toggle_lines_item);
    linet = GetChoice(toggle_linet_item);
    filltype = GetChoice(toggle_filltype_item);
    fillrule = GetChoice(toggle_fillrule_item);
    fillpat = GetOptionChoice(toggle_fillpat_item);
    fillcol = GetOptionChoice(toggle_fillcol_item);
    xv_evalexpri(symskip_item, &symskip);
    symcolor = GetOptionChoice(symcolor_item);
    sympattern = GetOptionChoice(sympattern_item);
    symfillcolor = GetOptionChoice(symfillcolor_item);
    symfillpattern = GetOptionChoice(symfillpattern_item);
    symlinew = GetSpinChoice(symlinew_item);
    symlines = GetOptionChoice(symlines_item);
    symchar = atoi(xv_getstr(symchar_item));
    charfont = GetOptionChoice(char_font_item);
    
    errbar.active = GetToggleButtonState(errbar_active_item);
    errbar.barsize = GetCharSizeChoice(errbar_size_item);
    errbar.linew = GetSpinChoice(errbar_width_item);
    errbar.lines = GetOptionChoice(errbar_lines_item);
    errbar.riser_linew = GetSpinChoice(errbar_riserlinew_item);
    errbar.riser_lines = GetOptionChoice(errbar_riserlines_item);
        
    avalue.active = GetToggleButtonState(avalue_active_item);
    avalue.type = GetChoice(avalue_type_item);
    avalue.size = GetCharSizeChoice(avalue_charsize_item);
    avalue.font = GetOptionChoice(avalue_font_item);
    avalue.color = GetOptionChoice(avalue_color_item);
    avalue.angle = GetAngleChoice(avalue_angle_item);
    avalue.format = GetChoice(avalue_format_item);
    avalue.prec = GetChoice(avalue_precision_item);
    strcpy(avalue.prestr, xv_getstr(avalue_prestr));
    strcpy(avalue.appstr, xv_getstr(avalue_appstr));
    xv_evalexpr(avalue_offsetx, &avalue.offset.x );
    xv_evalexpr(avalue_offsety, &avalue.offset.y);
                    
    cd = GetListChoices(toggle_symset_item, &selset);
    if (cd < 1) {
        errwin("No set selected");
        return;
    } else {
        for(i = 0; i < cd; i++) {
            setno = selset[i];
            get_graph_plotarr(get_cg(), setno, &p);
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
            if (cd == 1 || duplegs) {
                strcpy(p.lstr, GetTextString(legend_str_item));
            }
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

            errbar.ptype = GetChoice(errbar_ptype_item);
            errbar.pen.color = GetOptionChoice(errbar_color_item);
            errbar.pen.pattern = GetOptionChoice(errbar_pattern_item);
            errbar.arrow_clip = GetToggleButtonState(errbar_aclip_item);
            errbar.cliplen = GetSpinChoice(errbar_cliplen_item);
    
            p.errbar = errbar;
            p.avalue = avalue;
            
            set_graph_plotarr(get_cg(), setno, &p);
            set_dataset_type(get_cg(), setno, type);
        }
        free(selset);
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
    int i;
    char val[24];
    plotarr p;

    if ((cset == value) && (value != -1)) {
        get_graph_plotarr(gno, cset, &p);
    
        SetOptionChoice(type_item, p.type);
        for (i = 0; i < type_item->nchoices; i++) {
            if (settype_cols(type_item->options[i].value) ==
                                            settype_cols(p.type)) {
                XtSetSensitive(type_item->options[i].widget, True);
            } else {
                XtSetSensitive(type_item->options[i].widget, False);
            }
        }

        SetCharSizeChoice(symsize_item, p.symsize);
        sprintf(val, "%d", p.symskip);
        xv_setstr(symskip_item, val);
        sprintf(val, "%d", p.symchar);
        xv_setstr(symchar_item, val);
        SetChoice(toggle_symbols_item, p.sym);
        
        SetOptionChoice(symcolor_item, p.sympen.color);
        SetOptionChoice(sympattern_item, p.sympen.pattern);
        SetOptionChoice(symfillcolor_item, p.symfillpen.color);
        SetOptionChoice(symfillpattern_item, p.symfillpen.pattern);
        SetSpinChoice(symlinew_item, p.symlinew);
        SetOptionChoice(symlines_item, p.symlines);
        
        SetOptionChoice(char_font_item, p.charfont);        
        
        SetOptionChoice(toggle_color_item, p.linepen.color);
        SetOptionChoice(toggle_pattern_item, p.linepen.pattern);
        SetSpinChoice(toggle_width_item, p.linew);
        SetToggleButtonState(dropline_item, p.dropline);
        SetOptionChoice(toggle_lines_item, p.lines);
        SetChoice(toggle_linet_item, p.linet);
        SetChoice(toggle_filltype_item, p.filltype);
        SetChoice(toggle_fillrule_item, p.fillrule);
        SetOptionChoice(toggle_fillcol_item, p.setfillpen.color);
        SetOptionChoice(toggle_fillpat_item, p.setfillpen.pattern);
        
        SetToggleButtonState(baseline_item, p.baseline);
        SetChoice(baselinetype_item, p.baseline_type);

        SetTextString(legend_str_item, p.lstr);
        
        SetToggleButtonState(errbar_active_item, p.errbar.active);
        SetChoice(errbar_ptype_item, p.errbar.ptype);
        SetOptionChoice(errbar_color_item, p.errbar.pen.color);
        SetOptionChoice(errbar_pattern_item, p.errbar.pen.pattern);
        SetToggleButtonState(errbar_aclip_item, p.errbar.arrow_clip);
        SetSpinChoice(errbar_cliplen_item, p.errbar.cliplen);
        SetSpinChoice(errbar_width_item, p.errbar.linew);
        SetOptionChoice(errbar_lines_item, p.errbar.lines);
        SetSpinChoice(errbar_riserlinew_item, p.errbar.riser_linew);
        SetOptionChoice(errbar_riserlines_item, p.errbar.riser_lines);
        SetCharSizeChoice(errbar_size_item, p.errbar.barsize);

        SetToggleButtonState(avalue_active_item, p.avalue.active);
        SetChoice(avalue_type_item, p.avalue.type);
        SetCharSizeChoice(avalue_charsize_item, p.avalue.size);
        SetOptionChoice(avalue_font_item, p.avalue.font);
        SetOptionChoice(avalue_color_item, p.avalue.color);
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


static void set_cset_proc(int n, int *values, void *data)
{
    if (n == 1) {
        cset = values[0];
        UpdateSymbols(cg, cset);
    }
}

void updatesymbols(int gno, int setno)
{    
    if (gno != cg) {
        return;
    }
    
    if (setapp_dialog != NULL) { 
        if (SelectListChoice(toggle_symset_item, setno) == GRACE_EXIT_SUCCESS) {
            cset = setno;
        }
    }
}


static void setapp_data_proc(void *data)
{
    int proc_type;
    int *selset, cd;
    int i, setno;
    plotarr p;
    int c = 0, bg = getbgcolor();
    
    proc_type = (int) data;

    cd = GetListChoices(toggle_symset_item, &selset);
    if (cd < 1) {
        errmsg("No set selected");
        return;
    } else {
        for(i = 0; i < cd; i++) {
            setno = selset[i];
            switch (proc_type) {
            case SETAPP_STRIP_LEGENDS:
                set_legend_string(cg, setno,
                    mybasename(get_legend_string(cg, setno)));
                break;
            case SETAPP_LOAD_COMMENTS:
                load_comments_to_legend(cg, setno);
                break;
            case SETAPP_ALL_COLORS:
                while (c == bg || get_colortype(c) != COLOR_MAIN) {
                    c++;
                    c %= number_of_colors();
                }
                set_set_colors(cg, setno, c);
                c++;
                break;
            case SETAPP_ALL_SYMBOLS:
                get_graph_plotarr(cg, setno, &p);
                p.sym = (i % (MAXSYM - 2)) + 1;
                set_graph_plotarr(cg, setno, &p);
                break;
            case SETAPP_ALL_LINEW:
                get_graph_plotarr(cg, setno, &p);
                p.linew = ((i % (2*((int) MAX_LINEWIDTH) - 1)) + 1)/2.0;
                set_graph_plotarr(cg, setno, &p);
                break;
            case SETAPP_ALL_LINES:
                get_graph_plotarr(cg, setno, &p);
                p.lines = (i % (number_of_linestyles() - 1)) + 1;
                set_graph_plotarr(cg, setno, &p);
                break;
            case SETAPP_ALL_BW:
                set_set_colors(cg, setno, 1);
                break;
            }
        }
        
        free(selset);
        
        UpdateSymbols(cg, cset);
        set_dirtystate();
        drawgraph();
    }
}
