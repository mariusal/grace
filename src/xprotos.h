/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
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
 * Prototypes involving X
 *
 */
#ifndef __XPROTOS_H_
#define __XPROTOS_H_

#include <Xm/Xm.h>

#include "defines.h"

int initialize_gui(int *argc, char **argv);
void startup_gui(void);

void autoon_proc(void);
void autoticks_proc(Widget w, XtPointer client_data, XtPointer call_data);
void set_left_footer(char *s);

void xdrawgraph(void);
void do_zoom(Widget w, XtPointer client_data, XtPointer call_data);
void do_zoomx(Widget w, XtPointer client_data, XtPointer call_data);
void do_zoomy(Widget w, XtPointer client_data, XtPointer call_data);
void do_select_point(Widget w, XtPointer client_data, XtPointer call_data);
void do_clear_point(Widget w, XtPointer client_data, XtPointer call_data);
void expose_resize(Widget w, XtPointer client_data, XmDrawingAreaCallbackStruct *cbs);

void setpointer(VPoint vp);

void select_line(int x1, int y1, int x2, int y2, int erase);
void select_region(int x1, int y1, int x2, int y2, int erase);
void slide_region(view bbox, int shift_x, int shift_y, int erase);
void reset_crosshair(void);
void crosshair_motion(int x, int y);

void draw_focus(int gno);
void switch_current_graph(int gto);

char *display_name(void);

void xunregister_rti(XtInputId);
void xregister_rti(Input_buffer *ib);

void yesnoCB(Widget w, Boolean * keep_grab, XmAnyCallbackStruct * reason);
int yesnowin(char *msg1, char *msg2, char *s1, char *help_anchor);

void create_workingdir_popup(Widget w, XtPointer client_data, XtPointer call_data);
void create_file_popup(Widget wid, XtPointer client_data, XtPointer call_data);
void create_netcdfs_popup(Widget w, XtPointer client_data, XtPointer call_data);
void create_rparams_popup(Widget w, XtPointer client_data, XtPointer call_data);
void create_wparam_frame(Widget w, XtPointer client_data, XtPointer call_data);
void update_saveproject_popup(void);
void create_saveproject_popup(void);
void create_openproject_popup(void);
void create_newproject_popup(void);
void update_describe_popup (void);
void do_hotupdate_proc(Widget w, XtPointer client_data, XtPointer call_data);

void create_block_popup(Widget w, XtPointer client_data, XtPointer call_data);

void create_eblock_frame(int gno);

void create_printer_setup(Widget w, XtPointer client_data, XtPointer call_data);

void create_draw_frame(Widget w, XtPointer client_data, XtPointer call_data);

void open_command(Widget w, XtPointer client_data, XtPointer call_data);
void close_rhist_popup(Widget w, XtPointer client_data, XtPointer call_data);
void do_rhist_proc(Widget w, XtPointer client_data, XtPointer call_data);
void create_rhist_popup(Widget w, XtPointer client_data, XtPointer call_data);
void create_whist_frame(Widget w, XtPointer client_data, XtPointer call_data);

void do_pick_compose(Widget w, XtPointer client_data, XtPointer call_data);
void create_eval_frame(Widget w, XtPointer client_data, XtPointer call_data);
void create_load_frame(Widget w, XtPointer client_data, XtPointer call_data);
void create_histo_frame(Widget w, XtPointer client_data, XtPointer call_data);
void create_fourier_frame(Widget w, XtPointer client_data, XtPointer call_data);
void create_run_frame(Widget w, XtPointer client_data, XtPointer call_data);
void create_reg_frame(Widget w, XtPointer client_data, XtPointer call_data);
void create_diff_frame(Widget w, XtPointer client_data, XtPointer call_data);
void create_seasonal_frame(Widget w, XtPointer client_data, XtPointer call_data);
void create_interp_frame(Widget w, XtPointer client_data, XtPointer call_data);
void create_int_frame(Widget w, XtPointer client_data, XtPointer call_data);
void create_xcor_frame(Widget w, XtPointer client_data, XtPointer call_data);
void create_spline_frame(Widget w, XtPointer client_data, XtPointer call_data);
void create_samp_frame(Widget w, XtPointer client_data, XtPointer call_data);
void create_prune_frame(Widget w, XtPointer client_data, XtPointer call_data);
void create_digf_frame(Widget w, XtPointer client_data, XtPointer call_data);
void create_lconv_frame(Widget w, XtPointer client_data, XtPointer call_data);
void create_leval_frame(Widget w, XtPointer client_data, XtPointer call_data);
void create_geom_frame(Widget w, XtPointer client_data, XtPointer call_data);
void execute_pick_compute(int gno, int setno, int function);

void create_write_popup(Widget w, XtPointer client_data, XtPointer call_data);
void create_hotlinks_popup(Widget w, XtPointer client_data, XtPointer call_data);
void update_hotlinks(void);
void create_saveall_popup(Widget w, XtPointer client_data, XtPointer call_data);

void create_points_frame(Widget w, XtPointer client_data, XtPointer call_data);
void create_goto_frame(Widget w, XtPointer client_data, XtPointer call_data);
void create_add_frame(Widget w, XtPointer client_data, XtPointer call_data);

void create_editp_frame(Widget w, XtPointer client_data, XtPointer call_data);

void create_region_frame(Widget w, XtPointer client_data, XtPointer call_data);
void create_define_frame(Widget w, XtPointer client_data, XtPointer call_data);
void create_clear_frame(Widget w, XtPointer client_data, XtPointer call_data);
void create_extract_frame(Widget w, XtPointer client_data, XtPointer call_data);
void create_delete_frame(Widget w, XtPointer client_data, XtPointer call_data);
void create_extractsets_frame(Widget w, XtPointer client_data, XtPointer call_data);
void create_deletesets_frame(Widget w, XtPointer client_data, XtPointer call_data);
void create_reporton_frame(Widget w, XtPointer client_data, XtPointer call_data);
void create_evalregion_frame(Widget w, XtPointer client_data, XtPointer call_data);
void create_area_frame(Widget w, XtPointer client_data, XtPointer call_data);

void define_region(int nr, int regionlinkto, int rtype);

void update_status(int gno, int itemno);
void update_region_status(int rno);
void update_status_auto_redraw(void);
void clear_status(void);
void update_status_popup(Widget w, XtPointer client_data, XtPointer call_data);
void update_stuff_status(void);
void select_set(Widget w, XtPointer calld, XEvent * e);
void define_status_popup(Widget w, XtPointer client_data, XtPointer call_data);
void create_about_grtool(Widget w, XtPointer client_data, XtPointer call_data);

void update_set_lists(int gno);

void updatesymbols(int gno, int value);
void updatelegendstr(int gno);
void define_symbols_popup(Widget w, XtPointer client_data, XtPointer call_data);
void define_legends_proc(Widget w, XtPointer client_data, XtPointer call_data);
void legend_loc_proc(Widget w, XtPointer client_data, XtPointer call_data);
void legend_load_proc(Widget w, XtPointer client_data, XtPointer call_data);
void define_legend_popup(Widget w, XtPointer client_data, XtPointer call_data);
void define_errbar_popup(Widget w, XtPointer client_data, XtPointer call_data);

void update_ticks(int gno);
void create_axes_dialog(int axisno);
void create_axes_dialog_cb(Widget w, XtPointer client_data, XtPointer call_data);

void update_graph_items(void);
void create_graph_frame(Widget w, XtPointer client_data, XtPointer call_data);
void create_image_frame(Widget w, XtPointer client_data, XtPointer call_data);
void create_gtype_frame(Widget w, XtPointer client_data, XtPointer call_data);

void update_world(int gno);
void update_world_proc(void);
void create_world_frame(Widget w, XtPointer client_data, XtPointer call_data);
void update_view(int gno);
double fround(double x, double r);
void create_view_frame(Widget w, XtPointer client_data, XtPointer call_data);
void create_arrange_frame(Widget w, XtPointer client_data, XtPointer call_data);
void create_overlay_frame(Widget w, XtPointer client_data, XtPointer call_data);
void update_autos(int gno);
void create_autos_frame(Widget w, XtPointer client_data, XtPointer call_data);

void boxes_def_proc(Widget w, XtPointer client_data, XtPointer call_data);
void lines_def_proc(Widget w, XtPointer client_data, XtPointer call_data);
void updatestrings(void);
void update_lines(void);
void update_boxes(void);
void define_string_defaults(Widget w, XtPointer client_data, XtPointer call_data);
void define_objects_popup(Widget w, XtPointer client_data, XtPointer call_data);
void define_strings_popup(Widget w, XtPointer client_data, XtPointer call_data);
void define_lines_popup(Widget w, XtPointer client_data, XtPointer call_data);
void define_boxes_popup(Widget w, XtPointer client_data, XtPointer call_data);
void do_object_function(Widget w, XtPointer client_data, XtPointer call_data);

void update_label_proc(void);
void create_label_frame(Widget w, XtPointer client_data, XtPointer call_data);
void update_labelprops_proc(void);

void update_locator_items(int gno);
void create_locator_frame(Widget w, XtPointer client_data, XtPointer call_data);

void update_frame_items(int gno);
void create_graphapp_frame(int gno);
void create_graphapp_frame_cb(Widget w, XtPointer client_data, XtPointer call_data);

void create_monitor_frame(Widget w, XtPointer client_data, XtPointer call_data);
void stufftextwin(char *s, int sp);

void HelpCB(Widget w, XtPointer client_data, XtPointer call_data);
void ContextHelpCB(Widget w, XtPointer client_data, XtPointer call_data);

void create_nonl_frame(Widget w, XtPointer client_data, XtPointer call_data);
void update_nonl_frame(void);
void update_prune_frame(void);

void update_misc_items(void);
void create_plot_frame(void);
void create_plot_frame_cb(Widget w, XtPointer client_data, XtPointer call_data);
void create_props_frame(Widget w, XtPointer client_data, XtPointer call_data);

void create_fonttool(Widget w);
void create_fonttool_cb(Widget w, XtPointer client_data, XtPointer call_data);

void set_wait_cursor(void);
void unset_wait_cursor(void);
void set_cursor(int c);
void init_cursors(void);
int init_option_menus(void);

void get_canvas_size(Dimension *w, Dimension *h);
void set_canvas_size(Dimension w, Dimension h);
void get_scrolled_canvas_size(Dimension *w, Dimension *h);

void box_edit_popup(int no);
void ellipse_edit_popup(int no);
void line_edit_popup(int no);
void string_edit_popup(int no);
int object_edit_popup(int type, int id);

void set_title(char *ts);

void set_pagelayout(int layout);
int get_pagelayout(void);

void errwin(char *s);

void create_describe_popup(Widget w, XtPointer client_data, XtPointer call_data);

void create_datasetprop_popup(Widget w, XtPointer client_data, XtPointer call_data);
void create_datasetop_popup(Widget w, XtPointer client_data, XtPointer call_data);
void create_setop_popup(Widget w, XtPointer client_data, XtPointer call_data);

void create_featext_frame(Widget w, XtPointer client_data, XtPointer call_data);

void create_ss_frame(int gno, int setno);
void do_ext_editor(int gno, int setno);

void set_graph_selectors(int gno);

void update_props_items(void);
void update_all(void);
void update_all_cb(Widget w, XtPointer client_data, XtPointer call_data);

#endif /* __XPROTOS_H_ */
