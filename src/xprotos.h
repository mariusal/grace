/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2003 Grace Development Team
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
 * Prototypes involving X
 *
 */
#ifndef __XPROTOS_H_
#define __XPROTOS_H_

#ifndef NONE_GUI

#include "defines.h"
#include "core_utils.h"
#include "motifinc.h"

typedef int (*CanvasPointSink) (
    unsigned int npoints,
    const VPoint *vps,
    void *data
);

typedef struct {
    Screen *screen;
    Pixmap pixmap;
} X11stream;

struct _X11Stuff {
    Display *disp;
    int screennumber;
    
    Window root;
    Window xwin;

    Widget canvas;

    GC gc;
    int depth;
    Colormap cmap;
    
    double dpi;

    Pixmap bufpixmap;

    unsigned int win_h;
    unsigned int win_w;
    unsigned int win_scale;

    /* cursors */
    Cursor wait_cursor;
    Cursor line_cursor;
    Cursor find_cursor;
    Cursor move_cursor;
    Cursor text_cursor;
    Cursor kill_cursor;
    Cursor drag_cursor;
    int cur_cursor;

    /* coords of focus markers*/
    short f_x1, f_y1, f_x2, f_y2;
    view f_v;
    
    unsigned int npoints;
    XPoint *xps;
    
    unsigned int npoints_requested;
    int collect_points;
    
    CanvasPointSink point_sink;
    void *sink_data;
    int sel_type;
};

struct _MainWinUI {
    Widget drawing_window;
    Widget frleft, frtop, frbot;
    Widget windowbarw[3];
    Widget loclab;           /* locator label */
    Widget statlab;          /* status line at the bottom */
    Widget undo_button;
    Widget redo_button;
};

void x11_VPoint2dev(const VPoint *vp, short *x, short *y);
void x11_dev2VPoint(short x, short y, VPoint *vp);

int x11_get_pixelsize(const GUI *gui);
long x11_allocate_color(GUI *gui, const RGB *rgb);
void x11_redraw(Window window, int x, int y, int widht, int height);

int x11_init(GraceApp *gapp);

int initialize_gui(int *argc, char **argv);
void startup_gui(GraceApp *gapp);

void xdrawgraph(const Quark *q);
void expose_resize(Widget w, XtPointer client_data, XtPointer call_data);

void setpointer(VPoint vp);

void select_line(GUI *gui, int x1, int y1, int x2, int y2, int erase);
void select_region(GUI *gui, int x1, int y1, int x2, int y2, int erase);
void slide_region(GUI *gui, view bbox, int shift_x, int shift_y, int erase);
void select_vregion(GUI *gui, int x1, int x2, int erase);
void select_hregion(GUI *gui, int y1, int y2, int erase);
void resize_region(GUI *gui, view bb, int on_focus,
    int shift_x, int shift_y, int erase);
void reset_crosshair(GUI *gui, int clear);
void crosshair_motion(GUI *gui, int x, int y);

void draw_focus(Quark *gr);
void switch_current_graph(Quark *gr);

char *display_name(GUI *gui);

void xregister_rti(Input_buffer *ib);
void xunregister_rti(const Input_buffer *ib);

void errwin(const char *s);
int yesnowin(char *msg1, char *msg2, char *s1, char *help_anchor);
void stufftextwin(char *s);

void do_hotupdate_proc(void *data);

void create_file_popup(Widget but, void *data);
void create_netcdfs_popup(Widget but, void *data);
void create_saveproject_popup(void);
void create_openproject_popup(void);
void create_write_popup(Widget but, void *data);

void create_printer_setup(Widget but, void *data);

void create_eval_frame(Widget but, void *data);
void create_load_frame(Widget but, void *data);
void create_histo_frame(Widget but, void *data);
void create_fourier_frame(Widget but, void *data);
void create_run_frame(Widget but, void *data);
void create_reg_frame(Widget but, void *data);
void create_diff_frame(Widget but, void *data);
void create_interp_frame(Widget but, void *data);
void create_int_frame(Widget but, void *data);
void create_xcor_frame(Widget but, void *data);
void create_samp_frame(Widget but, void *data);
void create_prune_frame(Widget but, void *data);
void create_lconv_frame(Widget but, void *data);
void create_leval_frame(Widget but, void *data);

void create_points_frame(Widget but, void *data);

void create_arrange_frame(Widget but, void *data);
void create_autos_frame(Widget but, void *data);

void raise_explorer(GUI *gui, Quark *q);
void define_explorer_popup(Widget but, void *data);
void update_explorer(ExplorerUI *ui, int reselect);

void create_about_grtool(Widget but, void *data);

void create_monitor_frame_cb(Widget but, void *data);

void update_undo_buttons(Quark *project);
void update_set_lists(Quark *gr);
void update_props_items(void);
void update_all(void);
void update_all_cb(Widget but, void *data);

void update_set_selectors(Quark *gr);

void graph_set_selectors(Quark *gr);
int clean_graph_selectors(Quark *pr, int etype, void *data);
int clean_frame_selectors(Quark *pr, int etype, void *data);
int clean_set_selectors(Quark *gr, int etype, void *data);

void HelpCB(Widget w, void *data);

void create_nonl_frame(Widget but, void *data);

void create_props_frame(Widget but, void *data);

void create_fonttool(TextStructure *cstext_parent);
void create_fonttool_cb(Widget but, void *data);

void create_datasetprop_popup(Widget but, void *data);
void create_datasetop_popup(Widget but, void *data);

void create_featext_frame(Widget but, void *data);
void create_cumulative_frame(Widget but, void *data);

void init_cursors(GUI *gui);
void set_cursor(GUI *gui, int c);
void set_wait_cursor(void);
void unset_wait_cursor(void);

int init_option_menus(void);

void update_app_title(const Quark *pr);
void set_left_footer(char *s);
void set_tracker_string(char *s);

void page_zoom_inout(GraceApp *gapp, int inout);

void sync_canvas_size(GraceApp *gapp);

void installXErrorHandler(void);

Pixmap char_to_pixmap(Widget w, int font, char c, int csize);

void snapshot_and_update(Quark *q, int all);
#endif /* NONE_GUI */

#endif /* __XPROTOS_H_ */
