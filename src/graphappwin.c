/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2001 Grace Development Team
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

#include "globals.h"
#include "graphs.h"
#include "graphutils.h"
#include "utils.h"
#include "draw.h"
#include "protos.h"

#include "motifinc.h"

static Widget graphapp_dialog = NULL;

/*
 * Widget item declarations
 */
static ListStructure *graph_selector;

static Widget define_view_xv1;
static Widget define_view_xv2;
static Widget define_view_yv1;
static Widget define_view_yv2;

static OptionStructure *graph_type_choice_item;

static Widget stacked_item;

static TextStructure *label_title_text_item;
static TextStructure *label_subtitle_text_item;
static OptionStructure *title_color_item;
static OptionStructure *title_font_item;
static Widget title_size_item;
static OptionStructure *stitle_color_item;
static OptionStructure *stitle_font_item;
static Widget stitle_size_item;

static Widget graph_flipxy_item;

static SpinStructure *bargap_item;
static Widget znorm_item;

static OptionStructure *frame_framestyle_choice_item;
static OptionStructure *frame_color_choice_item;
static OptionStructure *frame_pattern_choice_item;
static OptionStructure *frame_lines_choice_item;
static SpinStructure *frame_linew_choice_item;
static OptionStructure *frame_fillcolor_choice_item;
static OptionStructure *frame_fillpattern_choice_item;

static Widget legend_x_item;
static Widget legend_y_item;
static Widget toggle_legends_item;
static OptionStructure *toggle_legendloc_item;
static OptionStructure *legends_vgap_item;
static OptionStructure *legends_hgap_item;
static OptionStructure *legends_len_item;
static Widget legends_invert_item;
static OptionStructure *legend_font_item;
static Widget legend_charsize_item;
static OptionStructure *legend_color_item;
static OptionStructure *legend_boxfillcolor_item;
static OptionStructure *legend_boxfillpat_item;
static SpinStructure *legend_boxlinew_item;
static OptionStructure *legend_boxlines_item;
static OptionStructure *legend_boxcolor_item;
static OptionStructure *legend_boxpattern_item;

static Widget instantupdate_item;

/*
 * Event and Notify proc declarations
 */
static int graphapp_aac_cb(void *data);
void updatelegends(int gno);
void update_view(int gno);
static void update_frame_items(int gno);
void update_graphapp_items(int n, int *values, void *data);

static void oc_graph_cb(int a, void *data)
{
    graphapp_aac_cb(data);
}
static void tb_graph_cb(int a, void *data)
{
    graphapp_aac_cb(data);
}
static void scale_graph_cb(int a, void *data)
{
    graphapp_aac_cb(data);
}
static void sp_graph_cb(double a, void *data)
{
    graphapp_aac_cb(data);
}
static void text_graph_cb(void *data)
{
    graphapp_aac_cb(data);
}

void create_graphapp_frame_cb(void *data)
{
    create_graphapp_frame((int) data);
}

static void flipxy_cb(int onoff, void *data)
{
    if (onoff) {
        errmsg("Flip XY is not implemented yet.");
    }
}

/*
 * 
 */
void create_graphapp_frame(int gno)
{
    set_wait_cursor();
    
    if (is_valid_gno(gno) == FALSE) {
        gno = get_cg();
    }
    
    if (graphapp_dialog == NULL) {
        Widget graphapp_tab, graphapp_frame, graphapp_main, graphapp_titles,
             graphapp_legends, graphapp_legendbox, graphapp_spec;
        Widget menubar, menupane, rc, rc1, rc2, fr;

	graphapp_dialog = CreateDialogForm(app_shell, "Graph Appearance");

        menubar = CreateMenuBar(graphapp_dialog);
        ManageChild(menubar);
        AddDialogFormChild(graphapp_dialog, menubar);
        
        graph_selector = CreateGraphChoice(graphapp_dialog, "Graph:",
                            LIST_TYPE_MULTIPLE);
        AddDialogFormChild(graphapp_dialog, graph_selector->rc);
        AddListChoiceCB(graph_selector, update_graphapp_items, NULL);

        menupane = CreateMenu(menubar, "File", 'F', FALSE);
        CreateMenuButton(menupane, "Open...", 'O', create_rparams_popup, NULL);
        CreateMenuButton(menupane, "Save...", 'S', create_wparam_frame, NULL);
        CreateMenuSeparator(menupane);
        CreateMenuCloseButton(menupane, graphapp_dialog);
        
        menupane = CreateMenu(menubar, "Edit", 'E', FALSE);

        CreateMenuButton(menupane,
            "Focus to", 'F', switch_focus_proc, graph_selector);
        CreateMenuSeparator(menupane);
        CreateMenuButton(menupane,
            "Hide", 'H', hide_graph_proc, graph_selector);
        CreateMenuButton(menupane,
            "Show", 'S', show_graph_proc, graph_selector);
        CreateMenuButton(menupane,
            "Duplicate", 'D', duplicate_graph_proc, graph_selector);
        CreateMenuButton(menupane,
            "Kill", 'K', kill_graph_proc, graph_selector);
        CreateMenuSeparator(menupane);
        CreateMenuButton(menupane,
            "Create new", 'C', create_new_graph_proc, graph_selector);

        menupane = CreateMenu(menubar, "Options", 'O', FALSE);
        instantupdate_item =
            CreateMenuToggle(menupane, "Instantaneous update", 'u', NULL, NULL);
        SetToggleButtonState(instantupdate_item, grace->gui->instant_update);
        
        menupane = CreateMenu(menubar, "Help", 'H', TRUE);
        CreateMenuHelpButton(menupane, "On graph appearance", 'g',
            graphapp_dialog, "doc/UsersGuide.html#graph-appearance");


        /* ------------ Tabs -------------- */

        graphapp_tab = CreateTab(graphapp_dialog);        


        /* ------------ Main tab -------------- */
        
        graphapp_main = CreateTabPage(graphapp_tab, "Main");

	fr = CreateFrame(graphapp_main, "Presentation");
        rc = CreateHContainer(fr);
	graph_type_choice_item = CreatePanelChoice(rc, 
                                                   "Type:",
						   7,
						   "XY graph",
						   "XY chart",
						   "Polar graph",
						   "Smith chart (N/I)",
						   "Fixed",
						   "Pie chart",
						   NULL,
                                                   0);
        AddOptionChoiceCB(graph_type_choice_item,
            oc_graph_cb, graph_type_choice_item);
	stacked_item = CreateToggleButton(rc, "Stacked chart");
        AddToggleButtonCB(stacked_item, tb_graph_cb, stacked_item);

	fr = CreateFrame(graphapp_main, "Titles");
	rc = CreateVContainer(fr);
	label_title_text_item = CreateCSText(rc, "Title: ");
        AddTextInputCB(label_title_text_item,
            text_graph_cb, label_title_text_item);
	label_subtitle_text_item = CreateCSText(rc, "Subtitle: ");
        AddTextInputCB(label_subtitle_text_item,
            text_graph_cb, label_subtitle_text_item);

        fr = CreateFrame(graphapp_main, "Viewport");
        rc = CreateVContainer(fr);

        rc1 = CreateHContainer(rc);
	define_view_xv1 = CreateTextItem2(rc1, 8, "Xmin:");
        AddTextItemCB(define_view_xv1, text_graph_cb, define_view_xv1);
	define_view_xv2 = CreateTextItem2(rc1, 8, "Xmax:");
        AddTextItemCB(define_view_xv2, text_graph_cb, define_view_xv2);

        rc1 = CreateHContainer(rc);
	define_view_yv1 = CreateTextItem2(rc1, 8, "Ymin:");
        AddTextItemCB(define_view_yv1, text_graph_cb, define_view_yv1);
	define_view_yv2 = CreateTextItem2(rc1, 8, "Ymax:");
        AddTextItemCB(define_view_yv2, text_graph_cb, define_view_yv2);

        fr = CreateFrame(graphapp_main, "Display options");
        rc = CreateHContainer(fr);
	toggle_legends_item = CreateToggleButton(rc, "Display legend");
        AddToggleButtonCB(toggle_legends_item, tb_graph_cb, toggle_legends_item);
	graph_flipxy_item = CreateToggleButton(rc, "Flip XY (N/I)");
        AddToggleButtonCB(graph_flipxy_item, flipxy_cb, NULL);
        /*AddToggleButtonCB(graph_flipxy_item, tb_graph_cb, (void *)graph_flipxy_item);*/


        /* ------------ Titles tab -------------- */
        
        graphapp_titles = CreateTabPage(graphapp_tab, "Titles");

	fr = CreateFrame(graphapp_titles, "Title");
	rc2 = CreateVContainer(fr);
	title_font_item = CreateFontChoice(rc2, "Font:");
        AddOptionChoiceCB(title_font_item, oc_graph_cb, title_font_item);
	title_size_item = CreateCharSizeChoice(rc2, "Character size");
        AddScaleCB(title_size_item, scale_graph_cb, title_size_item);
	title_color_item = CreateColorChoice(rc2, "Color:");
        AddOptionChoiceCB(title_color_item, oc_graph_cb, title_color_item);

	fr = CreateFrame(graphapp_titles, "Subtitle");
	rc2 = CreateVContainer(fr);
	stitle_font_item = CreateFontChoice(rc2, "Font:");
        AddOptionChoiceCB(stitle_font_item, oc_graph_cb, stitle_font_item);
	stitle_size_item = CreateCharSizeChoice(rc2, "Character size");
        AddScaleCB(stitle_size_item, scale_graph_cb, stitle_size_item);
	stitle_color_item = CreateColorChoice(rc2, "Color:");
        AddOptionChoiceCB(stitle_color_item, oc_graph_cb, stitle_color_item);

        /* ------------ Frame tab -------------- */
        
        graphapp_frame = CreateTabPage(graphapp_tab, "Frame");

	fr = CreateFrame(graphapp_frame, "Frame box");
	rc = CreateVContainer(fr);
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
        AddOptionChoiceCB(frame_framestyle_choice_item,
            oc_graph_cb, frame_framestyle_choice_item);

	rc2 = CreateHContainer(rc);
	frame_color_choice_item = CreateColorChoice(rc2, "Color:");
        AddOptionChoiceCB(frame_color_choice_item,
            oc_graph_cb, frame_color_choice_item);
	frame_pattern_choice_item = CreatePatternChoice(rc2, "Pattern:");
        AddOptionChoiceCB(frame_pattern_choice_item,
            oc_graph_cb, frame_pattern_choice_item);
	
        rc2 = CreateHContainer(rc);
	frame_linew_choice_item = CreateLineWidthChoice(rc2, "Width:");
        AddSpinButtonCB(frame_linew_choice_item, sp_graph_cb,
                                (void *)frame_linew_choice_item);
	frame_lines_choice_item = CreateLineStyleChoice(rc2, "Style:");
        AddOptionChoiceCB(frame_lines_choice_item,
            oc_graph_cb, frame_lines_choice_item);

	fr = CreateFrame(graphapp_frame, "Frame fill");
	rc = CreateHContainer(fr);
	frame_fillcolor_choice_item = CreateColorChoice(rc, "Color:");
        AddOptionChoiceCB(frame_fillcolor_choice_item,
            oc_graph_cb, frame_fillcolor_choice_item);
	frame_fillpattern_choice_item = CreatePatternChoice(rc, "Pattern:");
        AddOptionChoiceCB(frame_fillpattern_choice_item,
            oc_graph_cb, frame_fillpattern_choice_item);


        /* ------------ Legend frame tab -------------- */
        
        graphapp_legendbox = CreateTabPage(graphapp_tab, "Leg. box");

	fr = CreateFrame(graphapp_legendbox, "Location");
	rc = CreateVContainer(fr);
	toggle_legendloc_item = CreatePanelChoice(rc, "Locate in:",
						  3,
						  "World coords",
						  "Viewport coords",
						  0, 0);
        AddOptionChoiceCB(toggle_legendloc_item,
            oc_graph_cb, toggle_legendloc_item);

        rc1 = CreateHContainer(rc);
	legend_x_item = CreateTextItem2(rc1, 10, "X:");
        AddTextItemCB(legend_x_item, text_graph_cb, legend_x_item);
	legend_y_item = CreateTextItem2(rc1, 10, "Y:");
        AddTextItemCB(legend_y_item, text_graph_cb, legend_y_item);

	fr = CreateFrame(graphapp_legendbox, "Frame line");
	rc = CreateVContainer(fr);

	rc2 = CreateHContainer(rc);
	legend_boxcolor_item = CreateColorChoice(rc2, "Color:");
        AddOptionChoiceCB(legend_boxcolor_item,
            oc_graph_cb, legend_boxcolor_item);
	legend_boxpattern_item = CreatePatternChoice(rc2, "Pattern:");
        AddOptionChoiceCB(legend_boxpattern_item,
            oc_graph_cb, legend_boxpattern_item);
	
        rc2 = CreateHContainer(rc);
	legend_boxlinew_item = CreateLineWidthChoice(rc2, "Width:");
        AddSpinButtonCB(legend_boxlinew_item,
            sp_graph_cb, legend_boxlinew_item);
	legend_boxlines_item = CreateLineStyleChoice(rc2, "Style:");
        AddOptionChoiceCB(legend_boxlines_item,
            oc_graph_cb, legend_boxlines_item);

	fr = CreateFrame(graphapp_legendbox, "Frame fill");
	rc = CreateHContainer(fr);
	legend_boxfillcolor_item = CreateColorChoice(rc, "Color:");
        AddOptionChoiceCB(legend_boxfillcolor_item,
            oc_graph_cb, legend_boxfillcolor_item);
	legend_boxfillpat_item = CreatePatternChoice(rc, "Pattern:");
        AddOptionChoiceCB(legend_boxfillpat_item,
            oc_graph_cb, legend_boxfillpat_item);


        /* ------------ Legends tab -------------- */
        
        graphapp_legends = CreateTabPage(graphapp_tab, "Legends");

	fr = CreateFrame(graphapp_legends, "Text properties");
	rc = CreateVContainer(fr);
	legend_font_item = CreateFontChoice(rc, "Font:");
        AddOptionChoiceCB(legend_font_item, oc_graph_cb, legend_font_item);
	legend_charsize_item = CreateCharSizeChoice(rc, "Char size");
        AddScaleCB(legend_charsize_item, scale_graph_cb, legend_charsize_item);
	legend_color_item = CreateColorChoice(rc, "Color:");
        AddOptionChoiceCB(legend_color_item, oc_graph_cb, legend_color_item);

	fr = CreateFrame(graphapp_legends, "Placement");
	rc = CreateVContainer(fr);
        rc1 = CreateHContainer(rc);
        legends_vgap_item = CreatePanelChoice(rc1, "V-gap:",
					     7,
					     "0", "1", "2", "3", "4", "5",
					     0, 0);
        AddOptionChoiceCB(legends_vgap_item, oc_graph_cb, legends_vgap_item);
        legends_hgap_item = CreatePanelChoice(rc1, "H-gap:",
					     7,
					     "0", "1", "2", "3", "4", "5",
					     0, 0);
        AddOptionChoiceCB(legends_hgap_item, oc_graph_cb, legends_hgap_item);
	legends_len_item = CreatePanelChoice(rc, "Legend line length:",
					     10,
				             "0", "1", "2", "3", "4",
                                             "5", "6", "7", "8",
					     0, 0);
        AddOptionChoiceCB(legends_len_item, oc_graph_cb, legends_len_item);
	legends_invert_item = CreateToggleButton(rc, "Put in reverse order");
        AddToggleButtonCB(legends_invert_item, tb_graph_cb,legends_invert_item);
        

        /* ------------ Special tab -------------- */
        
        graphapp_spec = CreateTabPage(graphapp_tab, "Special");

	fr = CreateFrame(graphapp_spec, "2D+ graphs");
        znorm_item = CreateTextItem2(fr, 10, "Z normalization");
        AddTextItemCB(znorm_item, text_graph_cb, znorm_item); 

	fr = CreateFrame(graphapp_spec, "XY charts");
        bargap_item = CreateSpinChoice(fr, "Bar gap:", 5,
            SPIN_TYPE_FLOAT, 0.0, 1.0, 0.005);
        AddSpinButtonCB(bargap_item, sp_graph_cb, bargap_item);

       
        SelectTabPage(graphapp_tab, graphapp_main);

        CreateAACDialog(graphapp_dialog, graphapp_tab, graphapp_aac_cb, NULL);
        
#ifdef HAVE_LESSTIF
        /* a kludge against Lesstif geometry calculation bug */
        SelectTabPage(graphapp_tab, graphapp_legendbox);
        RaiseWindow(GetParent(graphapp_dialog));
        SelectTabPage(graphapp_tab, graphapp_main);
#endif
    }
    
    SelectListChoice(graph_selector, gno);
    
    RaiseWindow(GetParent(graphapp_dialog));
    unset_wait_cursor();
}

/*
 * Notify and event procs
 */

static int graphapp_aac_cb(void *data)
{
    int j, gno, n, *values;
    view v;
    labels labs;
    framep f;
    legend l;
    double znorm;
/*
 *     int flipxy;
 */
    
    if (!GetToggleButtonState(instantupdate_item) && data != NULL) {
        return RETURN_SUCCESS;
    }
    
    if (data == znorm_item || data == NULL) {
        xv_evalexpr(znorm_item, &znorm);
    }
    
/*
 *     flipxy = GetToggleButtonState(graph_flipxy_item);
 */

    n = GetListChoices(graph_selector, &values);
    for (j = 0; j < n; j++) {
        gno = values[j];
        if (is_valid_gno(gno)) {

            get_graph_viewport(gno, &v);
            get_graph_labels(gno, &labs);
            labs.title.s = copy_string(NULL,labs.title.s);  
            labs.stitle.s = copy_string(NULL,labs.stitle.s);  
            get_graph_framep(gno, &f);
            get_graph_legend(gno, &l);

            if (data == define_view_xv1 || data == NULL) {
                xv_evalexpr(define_view_xv1, &v.xv1);  
            }
            if (data == define_view_xv2 || data == NULL) {
                xv_evalexpr(define_view_xv2, &v.xv2);  
            }
            if (data == define_view_yv1 || data == NULL) {
                xv_evalexpr(define_view_yv1, &v.yv1);  
            }
            if (data == define_view_yv2 || data == NULL) {
                xv_evalexpr(define_view_yv2, &v.yv2);  
            }
            if (isvalid_viewport(&v)==FALSE) {
                errmsg("Invalid viewport coordinates");
                return RETURN_FAILURE;
            } 

            if (data == graph_type_choice_item || data == NULL) {
                set_graph_type(gno, GetOptionChoice(graph_type_choice_item));
            }
            if (data == stacked_item || data == NULL) {
                set_graph_stacked(gno,GetToggleButtonState(stacked_item));
            }
            if (data == bargap_item || data == NULL) {
                set_graph_bargap(gno, GetSpinChoice(bargap_item));
            }
            if (data == znorm_item || data == NULL) {
                set_graph_znorm(gno, znorm);
            }
            if (data == label_title_text_item || data == NULL) {
                set_plotstr_string(&labs.title, GetTextString(label_title_text_item));
            }
            if (data == title_size_item || data == NULL) {
                labs.title.charsize = GetCharSizeChoice(title_size_item);
            }
            if (data == title_color_item || data == NULL) {
                labs.title.color = GetOptionChoice(title_color_item);
            }
            if (data == title_font_item || data == NULL) {
                labs.title.font = GetOptionChoice(title_font_item);
            }
            if (data == label_subtitle_text_item || data == NULL) {
                set_plotstr_string(&labs.stitle, GetTextString(label_subtitle_text_item));
            }
            if (data == stitle_size_item || data == NULL) {
                labs.stitle.charsize = GetCharSizeChoice(stitle_size_item);
            }
            if (data == stitle_color_item || data == NULL) {
                labs.stitle.color = GetOptionChoice(stitle_color_item);
            }
            if (data == stitle_font_item || data == NULL) {
                labs.stitle.font = GetOptionChoice(stitle_font_item);
            }
            if (data == frame_framestyle_choice_item || data == NULL) {
                f.type = GetOptionChoice(frame_framestyle_choice_item);
            }
            if (data == frame_color_choice_item || data == NULL) {
                f.pen.color = GetOptionChoice(frame_color_choice_item);
            }
            if (data == frame_pattern_choice_item || data == NULL) {
                f.pen.pattern = GetOptionChoice(frame_pattern_choice_item);
            }
            if (data == frame_linew_choice_item || data == NULL) {
                f.linew = GetSpinChoice(frame_linew_choice_item);
            }
            if (data == frame_lines_choice_item || data == NULL) {
                f.lines = GetOptionChoice(frame_lines_choice_item);
            }
            if (data == frame_fillcolor_choice_item || data == NULL) {
                f.fillpen.color = GetOptionChoice(frame_fillcolor_choice_item);
            }
            if (data == frame_fillpattern_choice_item || data == NULL) {
                f.fillpen.pattern = GetOptionChoice(frame_fillpattern_choice_item);
            }
            if (data == legend_charsize_item || data == NULL) {
                l.charsize = GetCharSizeChoice(legend_charsize_item);
            }
            if (data == toggle_legends_item || data == NULL) {
                l.active = GetToggleButtonState(toggle_legends_item);
            }
            if (data == legends_vgap_item || data == NULL) {
                l.vgap = GetOptionChoice(legends_vgap_item);
            }
            if (data == legends_hgap_item || data == NULL) {
                l.hgap = GetOptionChoice(legends_hgap_item);
            } 
            if (data == legends_len_item || data == NULL) {
                l.len = GetOptionChoice(legends_len_item);
            }
            if (data == legends_invert_item || data == NULL) {
                l.invert = GetToggleButtonState(legends_invert_item);
            }
            if (data == toggle_legendloc_item || data == NULL) {
                l.loctype = GetOptionChoice(toggle_legendloc_item) ? COORD_VIEW : COORD_WORLD;
            }
            if (data == legend_x_item || data == NULL) {
                xv_evalexpr(legend_x_item, &l.legx);
            }
            if (data == legend_y_item || data == NULL) {
                xv_evalexpr(legend_y_item, &l.legy);
            }
            if (data == legend_font_item || data == NULL) {
                l.font = GetOptionChoice(legend_font_item);
            }
            if (data == legend_color_item || data == NULL) {
                l.color = GetOptionChoice(legend_color_item);
            }
            if (data == legend_boxfillcolor_item || data == NULL) {
                l.boxfillpen.color = GetOptionChoice(legend_boxfillcolor_item);
            }
            if (data == legend_boxfillpat_item || data == NULL) {
                l.boxfillpen.pattern = GetOptionChoice(legend_boxfillpat_item);
            }
            if (data == legend_boxcolor_item || data == NULL) {
                l.boxpen.color = GetOptionChoice(legend_boxcolor_item);
            }
            if (data == legend_boxpattern_item || data == NULL) {
                l.boxpen.pattern = GetOptionChoice(legend_boxpattern_item);
            }
            if (data == legend_boxlinew_item || data == NULL) {
                l.boxlinew = GetSpinChoice(legend_boxlinew_item);
            }
            if (data == legend_boxlines_item || data == NULL) {
                l.boxlines = GetOptionChoice(legend_boxlines_item);
            }

            /* g[gno].xyflip = flipxy; */
            
            set_graph_viewport(gno, v);
            set_graph_labels(gno, &labs);
            set_graph_framep(gno, &f);
            set_graph_legend(gno, &l);
	}
    }
    
    xfree(values);

    xdrawgraph();
    
    return RETURN_SUCCESS;
}

void update_graphapp_items(int n, int *values, void *data)
{
    int gno;
    labels labs;
    char buf[32];
    
    if (n != 1) {
        return;
    } else {
        gno = values[0];
    }

    if (is_valid_gno(gno) != TRUE) {
        return;
    }
    
    if (graphapp_dialog != NULL) {

        update_view(gno);

        update_frame_items(gno);
 
        updatelegends(gno);
        get_graph_labels(gno, &labs);
            
        SetOptionChoice(graph_type_choice_item, get_graph_type(gno));

        SetSpinChoice(bargap_item, get_graph_bargap(gno));

        SetToggleButtonState(stacked_item, is_graph_stacked(gno));

        sprintf(buf, "%g", get_graph_znorm(gno));
        xv_setstr(znorm_item, buf);

        SetTextString(label_title_text_item, labs.title.s);
        SetTextString(label_subtitle_text_item, labs.stitle.s);
 
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
void updatelegends(int gno)
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

	SetOptionChoice(legends_vgap_item, l.vgap);
	SetOptionChoice(legends_hgap_item, l.hgap);
	SetOptionChoice(legends_len_item, l.len);
	SetToggleButtonState(legends_invert_item, l.invert);

	SetOptionChoice(toggle_legendloc_item, l.loctype == COORD_VIEW);
	SetOptionChoice(legend_font_item, l.font);
	SetOptionChoice(legend_color_item, l.color);
	SetOptionChoice(legend_boxfillcolor_item, l.boxfillpen.color);
	SetOptionChoice(legend_boxfillpat_item, l.boxfillpen.pattern);
	SetOptionChoice(legend_boxcolor_item, l.boxpen.color);
	SetOptionChoice(legend_boxpattern_item, l.boxpen.pattern);
	SetSpinChoice(legend_boxlinew_item, l.boxlinew);
	SetOptionChoice(legend_boxlines_item, l.boxlines);
    }
}

static void update_frame_items(int gno)
{
    framep f;
    
    if (graphapp_dialog) {
        get_graph_framep(gno, &f);
    
	SetOptionChoice(frame_framestyle_choice_item, f.type);
	SetOptionChoice(frame_color_choice_item, f.pen.color);
	SetOptionChoice(frame_pattern_choice_item, f.pen.pattern);
	SetSpinChoice(frame_linew_choice_item, f.linew);
	SetOptionChoice(frame_lines_choice_item, f.lines);
	SetOptionChoice(frame_fillcolor_choice_item, f.fillpen.color);
	SetOptionChoice(frame_fillpattern_choice_item, f.fillpen.pattern);
    }
}
