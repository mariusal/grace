/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2004 Grace Development Team
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

/* GUI Actions */

#include <config.h>

#include <unistd.h>

#include "globals.h"
#include "bitmaps.h"
#include "utils.h"
#include "files.h"
#include "core_utils.h"
#include "events.h"
#include "xprotos.h"

#include "motifinc.h"

static int scroll_hook(Quark *q, void *udata, QTraverseClosure *closure)
{
    if (quark_fid_get(q) == QFlavorGraph) {
        int *type = (int *) udata;
        closure->descend = FALSE;
        graph_scroll(q, *type);
    }

    return TRUE;
}

static void graph_scroll_proc(GraceApp *gapp, int type)
{
    Quark *cg, *f;
    
    cg = graph_get_current(gproject_get_top(gapp->gp));
    f = get_parent_frame(cg);
    
    quark_traverse(f, scroll_hook, &type);
    
    snapshot_and_update(gapp->gp, TRUE);
}

static void graph_scroll_left_cb(Widget but, void *data)
{
    graph_scroll_proc((GraceApp *) data, GSCROLL_LEFT);
}

static void graph_scroll_right_cb(Widget but, void *data)
{
    graph_scroll_proc((GraceApp *) data, GSCROLL_RIGHT);
}

static void graph_scroll_up_cb(Widget but, void *data)
{
    graph_scroll_proc((GraceApp *) data, GSCROLL_UP);
}

static void graph_scroll_down_cb(Widget but, void *data)
{
    graph_scroll_proc((GraceApp *) data, GSCROLL_DOWN);
}

static int zoom_hook(Quark *q, void *udata, QTraverseClosure *closure)
{
    if (quark_fid_get(q) == QFlavorGraph) {
        int *type = (int *) udata;
        closure->descend = FALSE;
        graph_zoom(q, *type);
    }

    return TRUE;
}

static void graph_zoom_proc(GraceApp *gapp, int type)
{
    Quark *cg, *f;
    
    cg = graph_get_current(gproject_get_top(gapp->gp));
    f = get_parent_frame(cg);
    
    quark_traverse(f, zoom_hook, &type);
    
    snapshot_and_update(gapp->gp, TRUE);
}

static void graph_zoom_in_cb(Widget but, void *data)
{
    graph_zoom_proc((GraceApp *) data, GZOOM_EXPAND);
}

static void graph_zoom_out_cb(Widget but, void *data)
{
    graph_zoom_proc((GraceApp *) data, GZOOM_SHRINK);
}

static void load_example_cb(Widget but, void *data)
{
    char *s, buf[128];
    
    set_wait_cursor();
    
    s = (char *) data;
    sprintf(buf, "examples/%s", s);
    load_project(gapp, buf);

    xdrawgraph(gapp->gp);

    unset_wait_cursor();
}

static void do_drawgraph(Widget but, void *data)
{
    xdrawgraph(gapp->gp);
}


/*
 * service the autoscale buttons on the main panel
 */
static void autoscale_proc(GraceApp *gapp, int type)
{
    Quark *cg = graph_get_current(gproject_get_top(gapp->gp));
    
    if (autoscale_graph(cg, type) == RETURN_SUCCESS) {
        snapshot_and_update(gapp->gp, TRUE);
    } else {
	errmsg("Can't autoscale (no active sets?)");
    }
}

static void autoscale_xy_cb(Widget but, void *data)
{
    autoscale_proc((GraceApp *) data, AUTOSCALE_XY);
}

static void autoscale_x_cb(Widget but, void *data)
{
    autoscale_proc((GraceApp *) data, AUTOSCALE_X);
}

static void autoscale_y_cb(Widget but, void *data)
{
    autoscale_proc((GraceApp *) data, AUTOSCALE_Y);
}

/*
 * service the autoticks button on the main panel
 */
static void autoticks_cb(Widget but, void *data)
{
    autotick_graph_axes(graph_get_current(gproject_get_top(gapp->gp)), AXIS_MASK_XY);
    snapshot_and_update(gapp->gp, TRUE);
}


/*
 * service routines for the View pulldown
 */
static void set_statusbar(Widget but, int onoff, void *data)
{
    gapp->gui->statusbar = onoff;
    set_view_items();
}

static void set_toolbar(Widget but, int onoff, void *data)
{
    gapp->gui->toolbar = onoff;
    set_view_items();
}

static void set_locbar(Widget but, int onoff, void *data)
{
    gapp->gui->locbar = onoff;
    set_view_items();
}

static void zoom_in_cb(Widget but, void *data)
{
    GraceApp *gapp = (GraceApp *) data;
    page_zoom_inout(gapp, +1);
}

static void zoom_out_cb(Widget but, void *data)
{
    GraceApp *gapp = (GraceApp *) data;
    page_zoom_inout(gapp, -1);
}

static void zoom_1_cb(Widget but, void *data)
{
    GraceApp *gapp = (GraceApp *) data;
    page_zoom_inout(gapp, 0);
}

void update_all_cb(Widget but, void *data)
{
    update_all();
}

static void new_cb(Widget but, void *data)
{
    GraceApp *gapp = (GraceApp *) data;
    new_project(gapp, NULL);
    xdrawgraph(gapp->gp);
}


static void exit_cb(Widget but, void *data)
{
    GraceApp *gapp = (GraceApp *) data;
    bailout(gapp);
}

static void open_cb(Widget but, void *data)
{
    create_openproject_popup();
}

void project_save(GProject *gp)
{
    if (gproject_get_docname(gp)) {
        set_wait_cursor();

        save_project(gp, gproject_get_docname(gp));
        update_all();

        unset_wait_cursor();
    } else {
        create_saveproject_popup(gp);
    }
}

static void save_cb(Widget but, void *data)
{
    GraceApp *gapp = (GraceApp *) data;

    project_save(gapp->gp);
}

void project_save_as(GProject *gp)
{
    create_saveproject_popup(gp);
}

static void save_as_cb(Widget but, void *data)
{
    GraceApp *gapp = (GraceApp *) data;

    project_save_as(gapp->gp);
}

void revert_project(GraceApp *gapp, GProject *gp)
{
    char *docname;

    set_wait_cursor();
    docname = gproject_get_docname(gp);
    if (docname) {
        load_project(gapp, docname);
    } else {
        new_project(gapp, NULL);
    }

    gapp_remove_project(gapp, gp);
    gproject_free(gp);

    xdrawgraph(gapp->gp);
    update_all();
    unset_wait_cursor();
}

static void revert_cb(Widget but, void *data)
{
    GraceApp *gapp = (GraceApp *) data;

    revert_project(gapp, gapp->gp);
}

void close_project(GraceApp *gapp, GProject *gp)
{
    if (gp && gproject_get_top(gp) &&
        quark_dirtystate_get(gproject_get_top(gp)) &&
        !yesno("Abandon unsaved changes?", NULL, NULL, NULL)) {
        return;
    }

    gapp_remove_project(gapp, gp);
    gproject_free(gp);
    gapp_set_active_project(gapp, gapp->gplist[gapp->gpcount - 1]);

    xdrawgraph(gapp->gp);
    update_all();
}

static void close_cb(Widget but, void *data)
{
    GraceApp *gapp = (GraceApp *) data;

    close_project(gapp, gapp->gp);
}

static void print_cb(Widget but, void *data)
{
    GraceApp *gapp = (GraceApp *) data;
    set_wait_cursor();
    do_hardcopy(gapp->gp);
    unset_wait_cursor();
}

void undo_cb(Widget but, void *data)
{
    GraceApp *gapp = (GraceApp *) data;

    explorer_undo(gapp, TRUE);
}

void redo_cb(Widget but, void *data)
{
    GraceApp *gapp = (GraceApp *) data;

    explorer_undo(gapp, FALSE);
}

/*
 * create the main menubar
 */
Widget CreateMainMenuBar(Widget parent)
{
    MainWinUI *mwui = gapp->gui->mwui;
    Widget menubar;
    Widget menupane, submenupane, sub2menupane;
    static char buf[128];

    menubar = CreateMenuBar(parent);

    /* File menu */
    menupane = CreateMenu(menubar, "File", 'F', FALSE);

    CreateMenuButtonA(menupane, "New", 'N', "Ctrl+N", new_cb, gapp);
    CreateMenuButtonA(menupane, "Open...", 'O', "Ctrl+O", open_cb, gapp);
    CreateMenuButtonA(menupane, "Save", 'S', "Ctrl+S", save_cb, gapp);
    CreateMenuButton(menupane, "Save as...", 'a', save_as_cb, gapp);
    CreateMenuButton(menupane, "Revert to saved", 'v', revert_cb, gapp);

    CreateMenuSeparator(menupane);

    CreateMenuButtonA(menupane, "Print setup...", 't', "Ctrl+P", create_printer_setup, &gapp->rt->hdevice);
    CreateMenuButtonA(menupane, "Print", 'P', "Ctrl+Alt+P", print_cb, gapp);
    CreateMenuSeparator(menupane);
    CreateMenuButtonA(menupane, "Exit", 'x', "Ctrl+Q", exit_cb, gapp);

    /* Edit menu */
    menupane = CreateMenu(menubar, "Edit", 'E', FALSE);

    mwui->undo_button = CreateMenuButtonA(menupane, "Undo", 'U', "Ctrl+Z", undo_cb, gapp);
    mwui->redo_button = CreateMenuButtonA(menupane, "Redo", 'R', "Ctrl+Shift+Z", redo_cb, gapp);

    CreateMenuSeparator(menupane);

    CreateMenuButtonA(menupane, "Explorer...", 'E', "Ctrl+E", define_explorer_popup, gapp->gui);

    CreateMenuSeparator(menupane);

    CreateMenuButton(menupane, "Arrange frames...", 'f', create_arrange_frame, NULL);
    CreateMenuButton(menupane, "Autoscale graphs...", 'A', create_autos_frame, NULL);

    CreateMenuSeparator(menupane);

    CreateMenuButton(menupane, "Preferences...", 'P', create_props_frame, NULL);

    /* Data menu */
    menupane = CreateMenu(menubar, "Data", 'D', FALSE);

    CreateMenuButton(menupane, "Data set operations...", 'o', create_datasetop_popup, NULL);

    submenupane = CreateMenu(menupane, "Transformations", 'T', FALSE);
    CreateMenuButton(submenupane, "Evaluate expression...", 'E', create_eval_frame, NULL);
    CreateMenuSeparator(submenupane);
    CreateMenuButton(submenupane, "Histograms...", 'H', create_histo_frame, NULL);
    CreateMenuButton(submenupane, "Fourier transforms...", 'u', create_fourier_frame, NULL);
    CreateMenuSeparator(submenupane);
    CreateMenuButton(submenupane, "Running properties...", 's', create_run_frame, NULL);
    CreateMenuButton(submenupane, "Differences/derivatives...", 'D', create_diff_frame, NULL);
    CreateMenuButton(submenupane, "Integration...", 'I', create_int_frame, NULL);
    CreateMenuSeparator(submenupane);
    CreateMenuButton(submenupane, "Interpolation/splines...", 't', create_interp_frame, NULL);
    CreateMenuSeparator(submenupane);
    CreateMenuButton(submenupane, "Correlation/covariance...", 'C', create_xcor_frame, NULL);
    CreateMenuButton(submenupane, "Linear convolution...", 'v', create_lconv_frame, NULL);
    CreateMenuSeparator(submenupane);
    CreateMenuButton(submenupane, "Sample points...", 'm', create_samp_frame, NULL);
    CreateMenuButton(submenupane, "Prune data...", 'P', create_prune_frame, NULL);

    CreateMenuButton(menupane, "Feature extraction...", 'x', create_featext_frame, NULL);
    CreateMenuButton(menupane, "Cumulative properties...", 'C', create_cumulative_frame, NULL);

    CreateMenuSeparator(menupane);
    submenupane = CreateMenu(menupane, "Import", 'I', FALSE);
    CreateMenuButton(submenupane, "ASCII...", 'A', create_file_popup, NULL);
#ifdef HAVE_NETCDF
    CreateMenuButton(submenupane, "NetCDF...", 'N', create_netcdfs_popup, NULL);
#endif

    submenupane = CreateMenu(menupane, "Export", 'E', FALSE);
    CreateMenuButton(submenupane, "ASCII...", 'A', create_write_popup, NULL);

    /* View menu */
    menupane = CreateMenu(menubar, "View", 'V', FALSE);

    submenupane = CreateMenu(menupane, "Show/Hide", 'w', FALSE);
    mwui->windowbarw[0] = CreateMenuToggle(submenupane, "Locator bar", 'L', set_locbar, NULL);
    mwui->windowbarw[1] = CreateMenuToggle(submenupane, "Status bar", 'S', set_statusbar, NULL);
    mwui->windowbarw[2] = CreateMenuToggle(submenupane, "Tool bar", 'T', set_toolbar, NULL);

    if (!gui_is_page_free(gapp->gui)) {
        submenupane = CreateMenu(menupane, "Page zoom", 'z', FALSE);
        CreateMenuButtonA(submenupane, "Smaller", 'S', "Ctrl+-", zoom_out_cb, gapp);
        CreateMenuButtonA(submenupane, "Larger", 'L', "Ctrl++", zoom_in_cb, gapp);
        CreateMenuSeparator(submenupane);
        CreateMenuButtonA(submenupane, "Original size", 'O', "Ctrl+1", zoom_1_cb, gapp);
    }

    CreateMenuSeparator(menupane);

    CreateMenuButton(menupane, "Page rendering...", 'P', create_printer_setup, &gapp->rt->tdevice);

    CreateMenuSeparator(menupane);

    CreateMenuButtonA(menupane, "Redraw", 'R', "Ctrl+L", do_drawgraph, NULL);

    CreateMenuSeparator(menupane);

    CreateMenuButton(menupane, "Update all", 'U', update_all_cb, NULL);

    /* Window menu */
    menupane = CreateMenu(menubar, "Tools", 'T', FALSE);
   
    CreateMenuButton(menupane, "Console", 'C', create_monitor_frame_cb, NULL);
#if 0
    CreateMenuButton(menupane, "Point explorer", 'P', create_points_frame, NULL);
#endif
    CreateMenuButton(menupane, "Font tool", 'F', create_fonttool_cb, NULL);
/*
 *     CreateMenuButton(menupane, "Area/perimeter...", 'A', create_area_frame, NULL);
 */
    CreateMenuButton(menupane, "Dataset statistics", 'D',
        create_datasetprop_popup, gapp);

    /* Help menu */
    menupane = CreateMenu(menubar, "Help", 'H', TRUE);

    CreateMenuButton(menupane, "User's Guide", 'G', HelpCB, "doc/UsersGuide.html");
    CreateMenuButton(menupane, "FAQ", 'Q', HelpCB, "doc/FAQ.html");
    CreateMenuButton(menupane, "Changes", 'C', HelpCB, "doc/NEWS.html");

    CreateMenuSeparator(menupane);
 
    submenupane = CreateMenu(menupane, "Examples", 'E', FALSE);
    sub2menupane = CreateMenu(submenupane, "New 5.99 samples", 'N', FALSE);
    CreateMenuButton(sub2menupane, "Diode", '\0', load_example_cb, "diode.xgr");

    sub2menupane = CreateMenu(submenupane, "General intro", 'i', FALSE);
    CreateMenuButton(sub2menupane, "Explain", '\0', load_example_cb, "explain.agr");
    CreateMenuButton(sub2menupane, "Properties", '\0', load_example_cb, "props.agr");
    CreateMenuButton(sub2menupane, "Axes", '\0',load_example_cb, "axes.agr");
    CreateMenuButton(sub2menupane, "Fonts", '\0', load_example_cb, "tfonts.agr");
    CreateMenuButton(sub2menupane, "Arrows", '\0', load_example_cb, "arrows.agr");
    CreateMenuButton(sub2menupane, "Symbols and lines", '\0', load_example_cb, "symslines.agr");
    CreateMenuButton(sub2menupane, "Fills", '\0', load_example_cb, "fills.agr");
    CreateMenuButton(sub2menupane, "Inset graphs", '\0', load_example_cb, "tinset.agr");
    CreateMenuButton(sub2menupane, "Many graphs", '\0', load_example_cb, "manygraphs.agr");

    sub2menupane = CreateMenu(submenupane, "XY graphs", 'g', FALSE);
    CreateMenuButton(sub2menupane, "Log scale", '\0', load_example_cb, "tlog.agr");
    CreateMenuButton(sub2menupane, "Log2 scale", '\0', load_example_cb, "log2.agr");
    CreateMenuButton(sub2menupane, "Logit scale", '\0', load_example_cb, "logit.agr");
    CreateMenuButton(sub2menupane, "Reciprocal scale", '\0', load_example_cb, "reciprocal.agr");
    CreateMenuButton(sub2menupane, "Error bars", '\0', load_example_cb, "terr.agr");
    CreateMenuButton(sub2menupane, "Date/time axis formats", '\0', load_example_cb, "times.agr");
    CreateMenuButton(sub2menupane, "Australia map", '\0', load_example_cb, "au.agr");
    CreateMenuButton(sub2menupane, "A CO2 analysis", '\0', load_example_cb, "co2.agr");
    CreateMenuButton(sub2menupane, "Motif statistics", '\0', load_example_cb, "motif.agr");
    CreateMenuButton(sub2menupane, "Spectrum", '\0', load_example_cb, "spectrum.agr");

    sub2menupane = CreateMenu(submenupane, "XY charts", 'c', FALSE);
    CreateMenuButton(sub2menupane, "Bar chart", '\0', load_example_cb, "bar.agr");
    CreateMenuButton(sub2menupane, "Stacked bar", '\0', load_example_cb, "stackedb.agr");
    CreateMenuButton(sub2menupane, "Bar chart with error bars", '\0', load_example_cb, "chartebar.agr");
    CreateMenuButton(sub2menupane, "Different charts", '\0', load_example_cb, "charts.agr");

    sub2menupane = CreateMenu(submenupane, "Polar graphs", 'P', FALSE);
    CreateMenuButton(sub2menupane, "Polar graph", '\0', load_example_cb, "polar.agr");

    sub2menupane = CreateMenu(submenupane, "Pie charts", 'i', FALSE);
    CreateMenuButton(sub2menupane, "Pie chart", '\0', load_example_cb, "pie.agr");

    sub2menupane = CreateMenu(submenupane, "Special set presentations", 'S', FALSE);
    CreateMenuButton(sub2menupane, "HILO", '\0', load_example_cb, "hilo.agr");
    CreateMenuButton(sub2menupane, "XY Radius", '\0', load_example_cb, "txyr.agr");
    CreateMenuButton(sub2menupane, "XYZ", '\0', load_example_cb, "xyz.agr");
    CreateMenuButton(sub2menupane, "Box plot", '\0', load_example_cb, "boxplot.agr");
    CreateMenuButton(sub2menupane, "Vector map", '\0', load_example_cb, "vmap.agr");
    CreateMenuButton(sub2menupane, "XY Size", '\0', load_example_cb, "xysize.agr");
    CreateMenuButton(sub2menupane, "XY Color", '\0', load_example_cb, "xycolor.agr");

    sub2menupane = CreateMenu(submenupane, "Type setting", 'T', FALSE);
    CreateMenuButton(sub2menupane, "Simple", '\0', load_example_cb, "test2.agr");
    CreateMenuButton(sub2menupane, "Text transforms", '\0', load_example_cb, "txttrans.agr");
    CreateMenuButton(sub2menupane, "Advanced", '\0', load_example_cb, "typeset.agr");

#if 0
    sub2menupane = CreateMenu(submenupane, "Calculus", 'u', FALSE);
    CreateMenuButton(sub2menupane, "Non-linear fit", '\0', load_example_cb, "logistic.agr");
#endif
    CreateMenuSeparator(menupane);
    submenupane = CreateMenu(menupane, "Web support", 'W', FALSE);

    CreateMenuButton(submenupane, "Home page", 'H', HelpCB,
        "http://plasma-gate.weizmann.ac.il/Grace/");
    CreateMenuButton(submenupane, "Forums", 'F', HelpCB,
        "http://plasma-gate.weizmann.ac.il/Grace/phpbb/");
    sprintf(buf,
        "http://plasma-gate.weizmann.ac.il/Grace/report.php?version_id=%ld",
        bi_version_id());
    CreateMenuButton(submenupane, "Report an issue", 'R', HelpCB, buf);
    CreateMenuSeparator(menupane);

    CreateMenuButton(menupane, "License terms", 'L', HelpCB, "doc/GPL.html");
    CreateMenuButton(menupane, "About...", 'A', create_about_grtool, NULL);

    return (menubar);
}

void CreateToolBar(Widget parent)
{
    Widget bt, rcleft;
    rcleft = parent;
    /* redraw */
    bt = CreateBitmapButton(rcleft, 16, 16, redraw_bits);
    AddButtonCB(bt, do_drawgraph, NULL);
    
    CreateSeparator(rcleft);

    /* zoom */
    bt = CreateBitmapButton(rcleft, 16, 16, zoom_bits);
    AddButtonCB(bt, set_zoom_cb, (void *) gapp);
    bt = CreateBitmapButton(rcleft, 16, 16, zoom_x_bits);
    AddButtonCB(bt, set_zoomx_cb, (void *) gapp);
    bt = CreateBitmapButton(rcleft, 16, 16, zoom_y_bits);
    AddButtonCB(bt, set_zoomy_cb, (void *) gapp);

    CreateSeparator(rcleft);

    /* autoscale */
    bt = CreateBitmapButton(rcleft, 16, 16, auto_bits);
    AddButtonCB(bt, autoscale_xy_cb, (void *) gapp);
    bt = CreateBitmapButton(rcleft, 16, 16, auto_x_bits);
    AddButtonCB(bt, autoscale_x_cb, (void *) gapp);
    bt = CreateBitmapButton(rcleft, 16, 16, auto_y_bits);
    AddButtonCB(bt, autoscale_y_cb, (void *) gapp);
    bt = CreateBitmapButton(rcleft, 16, 16, auto_tick_bits);
    AddButtonCB(bt, autoticks_cb, NULL);

    CreateSeparator(rcleft);

    /* scrolling buttons */
    bt = CreateBitmapButton(rcleft, 16, 16, left_bits);
    AddButtonCB(bt, graph_scroll_left_cb, (void *) gapp);
    bt = CreateBitmapButton(rcleft, 16, 16, right_bits);
    AddButtonCB(bt, graph_scroll_right_cb, (void *) gapp);
    bt = CreateBitmapButton(rcleft, 16, 16, up_bits);
    AddButtonCB(bt, graph_scroll_up_cb, (void *) gapp);
    bt = CreateBitmapButton(rcleft, 16, 16, down_bits);
    AddButtonCB(bt, graph_scroll_down_cb, (void *) gapp);

    CreateSeparator(rcleft);

    /* expand/shrink */
    bt = CreateBitmapButton(rcleft, 16, 16, expand_bits);
    AddButtonCB(bt, graph_zoom_in_cb, (void *) gapp);
    bt = CreateBitmapButton(rcleft, 16, 16, shrink_bits);
    AddButtonCB(bt, graph_zoom_out_cb, (void *) gapp);

    CreateSeparator(rcleft);

    bt = CreateBitmapButton(rcleft, 16, 16, atext_bits);
    AddButtonCB(bt, atext_add_proc, (void *) gapp);

    CreateSeparator(rcleft);
    CreateSeparator(rcleft);

    /* exit */
    bt = CreateBitmapButton(rcleft, 16, 16, exit_bits);
    AddButtonCB(bt, exit_cb, gapp);
}
