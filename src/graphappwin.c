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
 * Graph appearance
 *
 */

#include <config.h>

#include <Xm/Xm.h>
#include <Xm/DialogS.h>
#include <Xm/RowColumn.h>
#include <Xm/PanedW.h>

#include "Tab.h"

#include "graphs.h"
#include "graphutils.h"
#include "utils.h"
#include "plotone.h"
#include "motifinc.h"
#include "protos.h"

static Widget graphapp_dialog = NULL;
static Widget graphapp_panel;

/*
 * Widget item declarations
 */
static ListStructure *graph_selector;

static Widget define_view_xv1;
static Widget define_view_xv2;
static Widget define_view_yv1;
static Widget define_view_yv2;

static Widget *graph_type_choice_item;

static Widget stacked_item;

static Widget label_title_text_item;
static Widget label_subtitle_text_item;
static OptionStructure *title_color_item;
static OptionStructure *title_font_item;
static Widget title_size_item;
static OptionStructure *stitle_color_item;
static OptionStructure *stitle_font_item;
static Widget stitle_size_item;

static Widget graph_flipxy_item;

static Widget bargap_item;

static Widget *frame_framestyle_choice_item;
static OptionStructure *frame_color_choice_item;
static OptionStructure *frame_pattern_choice_item;
static OptionStructure *frame_lines_choice_item;
static Widget *frame_linew_choice_item;
static OptionStructure *frame_fillcolor_choice_item;
static OptionStructure *frame_fillpattern_choice_item;

static Widget legend_x_item;
static Widget legend_y_item;
static Widget toggle_legends_item;
static Widget *toggle_legendloc_item;
static Widget *legends_vgap_item;
static Widget *legends_hgap_item;
static Widget *legends_len_item;
static Widget legends_invert_item;
static OptionStructure *legend_font_item;
static Widget legend_charsize_item;
static OptionStructure *legend_color_item;
static OptionStructure *legend_boxfillcolor_item;
static OptionStructure *legend_boxfillpat_item;
static Widget *legend_boxlinew_item;
static OptionStructure *legend_boxlines_item;
static OptionStructure *legend_boxcolor_item;
static OptionStructure *legend_boxpattern_item;

/*
 * Event and Notify proc declarations
 */
static void graphapp_aac_cb(Widget w, XtPointer client_data, XtPointer call_data);
static void updatelegends(int gno);
void update_graphapp_items(Widget list, XtPointer client_data,
                                                    XmListCallbackStruct *cbs);


/*
 * 
 */
void create_graphapp_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget graphapp_tab, graphapp_frame, graphapp_main, graphapp_titles,
         graphapp_legends, graphapp_legendbox;
    Widget rc, rc1, rc2, fr;

    set_wait_cursor();
    
    if (graphapp_dialog == NULL) {
	graphapp_dialog = XmCreateDialogShell(app_shell, "GraphAppearance", NULL, 0);
	handle_close(graphapp_dialog);
        graphapp_panel = XtVaCreateWidget("graphapp_panel", xmFormWidgetClass, 
                                          graphapp_dialog, NULL, 0);
/*
 *         graphapp_panel = XtVaCreateWidget("graphapp_panel", xmPanedWindowWidgetClass, 
 *                                           graphapp_dialog, NULL, 0);
 */

/*
 *         rc_head = XmCreateRowColumn(graphapp_panel, "rc_head", NULL, 0);
 */
        graph_selector = CreateGraphChoice(graphapp_panel, "Graph:",
                            LIST_TYPE_MULTIPLE);
        AddListChoiceCB(graph_selector,
                            (XtCallbackProc) update_graphapp_items);
/*
 *         XtManageChild(rc_head);
 */
        XtVaSetValues(graph_selector->rc,
                      XmNtopAttachment, XmATTACH_FORM,
                      XmNleftAttachment, XmATTACH_FORM,
                      XmNrightAttachment, XmATTACH_FORM,
                      NULL);


        /* ------------ Tabs --------------*/

        graphapp_tab = CreateTab(graphapp_panel);        


        /* ------------ Main tab --------------*/
        
        graphapp_main = CreateTabPage(graphapp_tab, "Main");

	fr = CreateFrame(graphapp_main, "Presentation:");
	rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, fr, NULL);
	graph_type_choice_item = CreatePanelChoice(rc, 
                                                   "Type:",
						   5,
						   "XY graph",
						   "XY chart",
						   "Polar graph",
						   "Smith chart (N/I)",
						   NULL,
						   NULL);

        rc1 = XtVaCreateWidget("rc", xmRowColumnWidgetClass, rc, NULL);
	XtVaSetValues(rc1, XmNorientation, XmHORIZONTAL, NULL);
	stacked_item = CreateToggleButton(rc1, "Stacked chart");
        bargap_item = CreateTextItem2(rc1, 4, "Inter-bar gap:");
        XtManageChild(rc1);
        
        XtManageChild(rc);

	fr = CreateFrame(graphapp_main, "Titles");
	rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, fr, NULL);
	label_title_text_item = CreateTextItem2(rc, 25, "Title:");
	label_subtitle_text_item = CreateTextItem2(rc, 22, "Subtitle:");
        XtManageChild(rc);

        fr = CreateFrame(graphapp_main, "Viewport");
        rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, fr, NULL);

        rc1 = XtVaCreateWidget("rc", xmRowColumnWidgetClass, rc, NULL);
	XtVaSetValues(rc1, XmNorientation, XmHORIZONTAL, NULL);
	define_view_xv1 = CreateTextItem2(rc1, 8, "Xmin:");
	define_view_xv2 = CreateTextItem2(rc1, 8, "Xmax:");
        XtManageChild(rc1);

        rc1 = XtVaCreateWidget("rc", xmRowColumnWidgetClass, rc, NULL);
	XtVaSetValues(rc1, XmNorientation, XmHORIZONTAL, NULL);
	define_view_yv1 = CreateTextItem2(rc1, 8, "Ymin:");
	define_view_yv2 = CreateTextItem2(rc1, 8, "Ymax:");
        XtManageChild(rc1);

        XtManageChild(rc);
        
        fr = CreateFrame(graphapp_main, "Display options");
        rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, fr, NULL);
	XtVaSetValues(rc, XmNorientation, XmHORIZONTAL, NULL);
	graph_flipxy_item = CreateToggleButton(rc, "Flip XY (N/I)");
	toggle_legends_item = CreateToggleButton(rc, "Display legend");
        XtManageChild(rc);



        /* ------------ Titles tab --------------*/
        
        graphapp_titles = CreateTabPage(graphapp_tab, "Titles");

	fr = CreateFrame(graphapp_titles, "Title");
	rc2 = XtVaCreateWidget("rc2", xmRowColumnWidgetClass, fr,
				      NULL);
	title_font_item = CreateFontChoice(rc2, "Font:");
	title_size_item = CreateCharSizeChoice(rc2, "Character size");

	title_color_item = CreateColorChoice(rc2, "Color:");

	XtManageChild(rc2);

	fr = CreateFrame(graphapp_titles, "Subtitle");
	rc2 = XtVaCreateWidget("rc2", xmRowColumnWidgetClass, fr, NULL);

	stitle_font_item = CreateFontChoice(rc2, "Font:");
	stitle_size_item = CreateCharSizeChoice(rc2, "Character size");
	stitle_color_item = CreateColorChoice(rc2, "Color:");

	XtManageChild(rc2);


        /* ------------ Frame tab --------------*/
        
        graphapp_frame = CreateTabPage(graphapp_tab, "Frame");

	fr = CreateFrame(graphapp_frame, "Frame box");
	rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, fr, NULL);
	frame_framestyle_choice_item = CreatePanelChoice(rc, "Frame type:",
							 7,
							 "Closed",
							 "Half open",
							 "Break top",
							 "Break bottom",
							 "Break left",
							 "Break right",
							 NULL,
							 NULL);

	frame_color_choice_item = CreateColorChoice(rc, "Line color:");
	frame_pattern_choice_item = CreatePatternChoice(rc, "Line pattern:");
	frame_linew_choice_item = CreateLineWidthChoice(rc, "Line width:");
	frame_lines_choice_item = CreateLineStyleChoice(rc, "Line style:");
        XtManageChild(rc);

	fr = CreateFrame(graphapp_frame, "Frame fill");
	rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, fr, NULL);
	frame_fillcolor_choice_item = CreateColorChoice(rc, "Fill color:");
	frame_fillpattern_choice_item = CreatePatternChoice(rc, "Fill pattern:");
        XtManageChild(rc);



        /* ------------ Legend frame tab --------------*/
        
        graphapp_legendbox = CreateTabPage(graphapp_tab, "Leg. box");

	fr = CreateFrame(graphapp_legendbox, "Location");
	rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, fr, NULL);
	toggle_legendloc_item = CreatePanelChoice(rc, "Locate in:",
						  3,
						  "World coords",
						  "Viewport coords",
						  0, 0);

        rc1 = XtVaCreateWidget("rc", xmRowColumnWidgetClass, rc, NULL);
	XtVaSetValues(rc1, XmNorientation, XmHORIZONTAL, NULL);
	legend_x_item = CreateTextItem2(rc1, 10, "X:");
	legend_y_item = CreateTextItem2(rc1, 10, "Y:");
	XtManageChild(rc1);
	XtManageChild(rc);

	fr = CreateFrame(graphapp_legendbox, "Frame box");
	rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, fr, NULL);

	legend_boxcolor_item = CreateColorChoice(rc, "Line color:");
	legend_boxpattern_item = CreatePatternChoice(rc, "Line pattern:");
	legend_boxlinew_item = CreateLineWidthChoice(rc, "Line width:");
	legend_boxlines_item = CreateLineStyleChoice(rc, "Line style:");
	XtManageChild(rc);

	fr = CreateFrame(graphapp_legendbox, "Frame fill");
	rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, fr, NULL);
	legend_boxfillcolor_item = CreateColorChoice(rc, "Color:");
	legend_boxfillpat_item = CreatePatternChoice(rc, "Pattern:");
	XtManageChild(rc);




        /* ------------ Legends tab --------------*/
        
        graphapp_legends = CreateTabPage(graphapp_tab, "Legends");

	fr = CreateFrame(graphapp_legends, "Text properties");
	rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, fr, NULL);
	legend_font_item = CreateFontChoice(rc, "Font:");

	legend_charsize_item = CreateCharSizeChoice(rc, "Char size");
	legend_color_item = CreateColorChoice(rc, "Color:");
        
        XtManageChild(rc);

	fr = CreateFrame(graphapp_legends, "Placement");
	rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, fr, NULL);
	
        rc1 = XtVaCreateWidget("rc", xmRowColumnWidgetClass, rc, NULL);
	XtVaSetValues(rc1, XmNorientation, XmHORIZONTAL, NULL);
        legends_vgap_item = CreatePanelChoice(rc1, "V-gap:",
					     7,
					     "0", "1", "2", "3", "4", "5",
					     0, 0);
        legends_hgap_item = CreatePanelChoice(rc1, "H-gap:",
					     7,
					     "0", "1", "2", "3", "4", "5",
					     0, 0);
	XtManageChild(rc1);

	legends_len_item = CreatePanelChoice(rc, "Legend line length:",
					     10,
				             "0", "1", "2", "3", "4",
                                             "5", "6", "7", "8",
					     0, 0);
	legends_invert_item = CreateToggleButton(rc, "Put in reverse order");
	XtManageChild(rc);
        
        
        SelectTabPage(graphapp_tab, graphapp_main);


        fr = XmCreateFrame(graphapp_panel, "fr", NULL, 0); 
        rc = XmCreateRowColumn(fr, "rc", NULL, 0);

        CreateAACButtons(rc, graphapp_panel, graphapp_aac_cb);
        
        XtManageChild(rc);
        XtManageChild(fr);
        XtVaSetValues(fr,
                      XmNtopAttachment, XmATTACH_NONE,
                      XmNleftAttachment, XmATTACH_FORM,
                      XmNrightAttachment, XmATTACH_FORM,
                      XmNbottomAttachment, XmATTACH_FORM,
                      NULL);
        XtVaSetValues(graphapp_tab,
                      XmNtopAttachment, XmATTACH_WIDGET,
                      XmNtopWidget, graph_selector->rc,
                      XmNleftAttachment, XmATTACH_FORM,
                      XmNrightAttachment, XmATTACH_FORM,
                      XmNbottomAttachment, XmATTACH_WIDGET,
                      XmNbottomWidget, fr,
                      NULL);

        XtManageChild(graphapp_panel);

    }
    
    SelectListChoice(graph_selector, get_cg());
    XtRaise(graphapp_dialog);
    unset_wait_cursor();
}

/*
 * Notify and event procs
 */

static void graphapp_aac_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
    int j, gno, n, *values;
    int aac_mode;
    view v;
    labels labs;
    framep f;
    legend l;
    int graphtype;
    int stacked;
    double bargap;
/*
 *     int flipxy;
 */
    
    aac_mode = (int) client_data;
    
    if (aac_mode == AAC_CLOSE) {
        XtUnmanageChild(graphapp_dialog);
        return;
    }
    
    set_default_string(&labs.title);
    set_default_string(&labs.stitle);
    
    xv_evalexpr(define_view_xv1, &v.xv1);  
    xv_evalexpr(define_view_xv2, &v.xv2);  
    xv_evalexpr(define_view_yv1, &v.yv1);  
    xv_evalexpr(define_view_yv2, &v.yv2);  
    
    if (isvalid_viewport(v) == FALSE) {
        errmsg("Invalid viewport coordinates");
        return;
    } 

    graphtype = GetChoice(graph_type_choice_item);
    stacked = GetToggleButtonState(stacked_item);

    xv_evalexpr(bargap_item, &bargap);
    
/*
 *     flipxy = GetToggleButtonState(graph_flipxy_item);
 */

    n = GetListChoices(graph_selector, &values);
    for (j = 0; j < n; j++) {
	gno = values[j];
        if (is_valid_gno(gno)) {

            set_graph_type(gno, graphtype);

            set_graph_stacked(gno, stacked);
            set_graph_bargap(gno, bargap);

            set_plotstr_string(&labs.title, xv_getstr(label_title_text_item));
            set_plotstr_string(&labs.stitle, xv_getstr(label_subtitle_text_item));

            labs.title.charsize = GetCharSizeChoice(title_size_item);
            labs.stitle.charsize = GetCharSizeChoice(stitle_size_item);

            labs.title.color = GetOptionChoice(title_color_item);
            labs.stitle.color = GetOptionChoice(stitle_color_item);
            
            labs.title.font = GetOptionChoice(title_font_item);
            labs.stitle.font = GetOptionChoice(stitle_font_item);
            
/*
 *             g[gno].xyflip = flipxy;
 */
           

	    f.type = GetChoice(frame_framestyle_choice_item);
	    f.pen.color = GetOptionChoice(frame_color_choice_item);
	    f.pen.pattern = GetOptionChoice(frame_pattern_choice_item);
	    f.linew = GetChoice(frame_linew_choice_item);
	    f.lines = GetOptionChoice(frame_lines_choice_item);
	    f.fillpen.color = GetOptionChoice(frame_fillcolor_choice_item);
	    f.fillpen.pattern = GetOptionChoice(frame_fillpattern_choice_item);

	    l.charsize = GetCharSizeChoice(legend_charsize_item);
	    l.active = GetToggleButtonState(toggle_legends_item);
	    l.vgap = GetChoice(legends_vgap_item);
	    l.hgap = GetChoice(legends_hgap_item);
	    l.len = GetChoice(legends_len_item);
	    l.invert = GetToggleButtonState(legends_invert_item);
	    l.loctype = GetChoice(toggle_legendloc_item) ? COORD_VIEW : COORD_WORLD;
	    xv_evalexpr(legend_x_item, &l.legx);
	    xv_evalexpr(legend_y_item, &l.legy);
	    l.font = GetOptionChoice(legend_font_item);
	    l.color = GetOptionChoice(legend_color_item);
	    l.boxfillpen.color = GetOptionChoice(legend_boxfillcolor_item);
	    l.boxfillpen.pattern = GetOptionChoice(legend_boxfillpat_item);
	    l.boxpen.color = GetOptionChoice(legend_boxcolor_item);
	    l.boxpen.pattern = GetOptionChoice(legend_boxpattern_item);
	    l.boxlinew = GetChoice(legend_boxlinew_item);
	    l.boxlines = GetOptionChoice(legend_boxlines_item);

            set_graph_viewport(gno, v);
            set_graph_labels(gno, &labs);
            set_graph_framep(gno, &f);
            set_graph_legend(gno, &l);
	}
    }
    
    free(values);

    if (aac_mode == AAC_ACCEPT) {
        XtUnmanageChild(graphapp_dialog);
    }
    
    drawgraph();
    set_dirtystate();
}

void update_graphapp_items(Widget list, XtPointer client_data,
                                                    XmListCallbackStruct *cbs)
{
    ListStructure *listp;
    int n, *values;
    int gno;
    labels labs;
    char val[24];
    
    listp = (ListStructure *) client_data;
    if (listp == NULL) {
        return;
    }

    n = GetListChoices(listp, &values);
    if (n < 1) {
        return;
    } else {
        gno = values[0];
        free(values);
        if (n > 1) {
            return;
        }
    }

    if (is_valid_gno(gno) != TRUE) {
        return;
    }
    
    if (graphapp_dialog != NULL) {

        update_view(gno);

        update_frame_items(gno);
 
        updatelegends(gno);
        get_graph_labels(gno, &labs);
            
        SetChoice(graph_type_choice_item, get_graph_type(gno));

        sprintf(val, "%g", get_graph_bargap(gno));
        xv_setstr(bargap_item, val);

        if (get_graph_type(gno) != GRAPH_CHART) {
            SetToggleButtonState(stacked_item, FALSE);
            XtSetSensitive(stacked_item, False);
        } else {
            SetToggleButtonState(stacked_item, is_graph_stacked(gno));
            XtSetSensitive(stacked_item, True);
        }

        xv_setstr(label_title_text_item, labs.title.s);
        xv_setstr(label_subtitle_text_item, labs.stitle.s);
 
        SetCharSizeChoice(title_size_item, labs.title.charsize);
        SetCharSizeChoice(stitle_size_item, labs.stitle.charsize);

        SetOptionChoice(title_color_item, labs.title.color);
        SetOptionChoice(stitle_color_item, labs.stitle.color);

        SetOptionChoice(title_font_item, labs.title.font);
        SetOptionChoice(stitle_font_item, labs.stitle.font);

/*
 *         SetToggleButtonState(graph_flipxy_item, g[gno].xyflip);
 */
    }
}
/*
 * Viewport update
 */
void update_view(int gno)
{
    view v;
    char buf[32];
    
    if (graphapp_dialog) {
	get_graph_viewport(gno, &v);
        
        sprintf(buf, "%.9g", v.xv1);
	xv_setstr(define_view_xv1, buf);
	sprintf(buf, "%.9g", v.xv2);
	xv_setstr(define_view_xv2, buf);
	sprintf(buf, "%.9g", v.yv1);
	xv_setstr(define_view_yv1, buf);
	sprintf(buf, "%.9g", v.yv2);
	xv_setstr(define_view_yv2, buf);
    }
}

/*
 * legend popup
 */
static void updatelegends(int gno)
{
    legend l;
    char buf[32];
    
    if (graphapp_dialog != NULL) {
	get_graph_legend(gno, &l);
        
        SetCharSizeChoice(legend_charsize_item, l.charsize);

	SetToggleButtonState(toggle_legends_item, l.active == TRUE);

	sprintf(buf, "%.9g", l.legx);
	xv_setstr(legend_x_item, buf);
	sprintf(buf, "%.9g", l.legy);
	xv_setstr(legend_y_item, buf);

	SetChoice(legends_vgap_item, l.vgap);
	SetChoice(legends_hgap_item, l.hgap);
	SetChoice(legends_len_item, l.len);
	SetToggleButtonState(legends_invert_item, l.invert);

	SetChoice(toggle_legendloc_item, l.loctype == COORD_VIEW);
	SetOptionChoice(legend_font_item, l.font);
	SetOptionChoice(legend_color_item, l.color);
	SetOptionChoice(legend_boxfillcolor_item, l.boxfillpen.color);
	SetOptionChoice(legend_boxfillpat_item, l.boxfillpen.pattern);
	SetOptionChoice(legend_boxcolor_item, l.boxpen.color);
	SetOptionChoice(legend_boxpattern_item, l.boxpen.pattern);
	SetChoice(legend_boxlinew_item, l.boxlinew);
	SetOptionChoice(legend_boxlines_item, l.boxlines);
    }
}

void update_frame_items(int gno)
{
    framep f;
    
    if (graphapp_dialog) {
        get_graph_framep(gno, &f);
    
	SetChoice(frame_framestyle_choice_item, f.type);
	SetOptionChoice(frame_color_choice_item, f.pen.color);
	SetOptionChoice(frame_pattern_choice_item, f.pen.pattern);
	SetChoice(frame_linew_choice_item, f.linew);
	SetOptionChoice(frame_lines_choice_item, f.lines);
	SetOptionChoice(frame_fillcolor_choice_item, f.fillpen.color);
	SetOptionChoice(frame_fillpattern_choice_item, f.fillpen.pattern);
    }
}

