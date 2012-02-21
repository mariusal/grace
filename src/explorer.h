/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2003-2005 Grace Development Team
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

#ifndef __EXPLORER_H_
#define __EXPLORER_H_

/* for MAX_TICKS */
#include "core_utils.h"

#include "motifinc.h"

typedef struct {
    Widget          top;

    SpinStructure   *prec;
    TextStructure   *description;
    
    OptionStructure *page_orient;
    OptionStructure *page_format;
    OptionStructure *page_size_unit;
    TextStructure   *page_x;
    TextStructure   *page_y;

    OptionStructure *bg_color;
    Widget          bg_fill;

    SpinStructure   *fsize_scale;
    SpinStructure   *lwidth_scale;

    TextStructure   *refdate;
    TextStructure   *wrap_year;
    Widget          two_digits_years;

    int             current_page_units;
} ProjectUI;

typedef struct {
    Quark           *q;
    
    Widget          top;

    Widget          main_tp;
    Widget          column_tp;
    Widget          hotlink_tp;

    Widget          mw;
    
    ListStructure   *col_sel;
    TextStructure   *col_label;

    Widget          hotlink;
    OptionStructure *hotsrc;
    TextStructure   *hotfile;
    
    Widget          popup;
    Widget          index_btn;
    Widget          unindex_btn;
    Widget          delete_btn;

    int             cb_column;
} SSDataUI;

typedef struct {
    Widget          top;
    
    Widget          main_tp;
    Widget          legends_tp;
    Widget          legendbox_tp;

    TextStructure   *view_xv1;
    TextStructure   *view_xv2;
    TextStructure   *view_yv1;
    TextStructure   *view_yv2;

    OptionStructure *frame_framestyle_choice;
    Widget          frame_pen;
    OptionStructure *frame_lines_choice;
    SpinStructure   *frame_linew_choice;
    Widget          frame_fillpen;

    SpinStructure   *legend_anchor_x;
    SpinStructure   *legend_anchor_y;
    OptionStructure *legend_just;
    SpinStructure   *legend_dx;
    SpinStructure   *legend_dy;
    Widget          toggle_legends;
    SpinStructure   *legends_vgap;
    SpinStructure   *legends_hgap;
    SpinStructure   *legends_len;
    Widget          legends_invert;
    Widget          legends_singlesym;
    OptionStructure *legend_font;
    SpinStructure   *legend_charsize;
    OptionStructure *legend_color;
    SpinStructure   *legend_boxlinew;
    OptionStructure *legend_boxlines;
    Widget          legend_boxpen;
    Widget          legend_boxfillpen;
} FrameUI;

typedef struct {
    Widget          top;
    
    Widget          main_tp;
    Widget          locator_tp;

    OptionStructure *graph_type;
    Widget          stacked;

    TextStructure   *start_x;
    TextStructure   *stop_x;
    OptionStructure *scale_x;
    Widget          invert_x;
    TextStructure   *start_y;
    TextStructure   *stop_y;
    OptionStructure *scale_y;
    Widget          invert_y;

    Widget          flip_xy;
    SpinStructure   *bargap;
    TextStructure   *znorm;

    OptionStructure *loc_type;
    FormatStructure *loc_fx;
    FormatStructure *loc_fy;
    TextStructure   *locx;
    TextStructure   *locy;
    Widget          fixedp;
    
} GraphUI;

typedef struct {
    Widget          top;
    
    Widget          main_tp;
    Widget          symbol_tp;
    Widget          line_tp;
    Widget          errbar_tp;
    Widget          avalue_tp;
    Widget          frame_tp;

    OptionStructure *type;
    
    OptionStructure *cols[MAX_SET_COLS];
    OptionStructure *acol;
    
    OptionStructure *symbols;
    SpinStructure   *symsize;
    SpinStructure   *symskip;
    SpinStructure   *symskipmindist;
    Widget          sympen;
    Widget          symfillpen;
    SpinStructure   *symlinew;
    OptionStructure *symlines;
    OptionStructure *symchar;
    OptionStructure *char_font;

    Widget          pen;
    SpinStructure   *width;
    Widget          dropline;
    OptionStructure *lines;
    OptionStructure *linet;
    OptionStructure *filltype;
    OptionStructure *fillrule;
    Widget          fillpen;
    Widget          baseline;
    OptionStructure *baselinetype;

    TextStructure   *legend_str;

    Widget          errbar_active;
    Widget          errbar_pen;
    SpinStructure   *errbar_size;
    SpinStructure   *errbar_width;
    OptionStructure *errbar_lines;
    SpinStructure   *errbar_riserlinew;
    OptionStructure *errbar_riserlines;
    Widget          errbar_aclip;
    SpinStructure   *errbar_cliplen;

    Widget          avalue_active;
    OptionStructure *avalue_font;
    OptionStructure *avalue_color;
    SpinStructure   *avalue_charsize;
    SpinStructure   *avalue_angle;
    FormatStructure *avalue_format;
    TextStructure   *avalue_offsetx;
    TextStructure   *avalue_offsety;
    OptionStructure *avalue_just;
    TextStructure   *avalue_prestr;
    TextStructure   *avalue_appstr;

    OptionStructure *frame_decor;
    SpinStructure   *frame_offset;
    SpinStructure   *frame_linew;
    OptionStructure *frame_lines;
    Widget          frame_linepen;
    Widget          frame_fillpen;
} SetUI;

typedef struct {
    Widget          top;

    OptionStructure *position;
    SpinStructure   *offset;
    Widget          draw_bar;
    Widget          draw_ticks;
    Widget          draw_labels;
} AxisUI;
    
typedef struct {
    Widget          top;

    Widget          main_tp;
    Widget          label_tp;
    Widget          ticklabel_tp;
    Widget          tickmark_tp;
    Widget          special_tp;
    Widget          sw;

    OptionStructure *type;
    
    TextStructure   *tmajor;
    SpinStructure   *nminor;
    
    Widget          tgrid;
    Widget          tgridpen;
    SpinStructure   *tgridlinew;
    OptionStructure *tgridlines;
    Widget          tmgrid;
    Widget          tmgridpen;
    SpinStructure   *tmgridlinew;
    OptionStructure *tmgridlines;

    FormatStructure *tlform;
    OptionStructure *tlfont;
    SpinStructure   *tlcharsize;
    OptionStructure *tlcolor;
    TextStructure   *tlappstr;
    TextStructure   *tlprestr;
    OptionStructure *tlskip;
    OptionStructure *tlstarttype;
    TextStructure   *tlstart;
    OptionStructure *tlstoptype;
    TextStructure   *tlstop;
    TextStructure   *tlgap_para;
    TextStructure   *tlgap_perp;
    SpinStructure   *tlangle;
    OptionStructure *tlstagger;
    TextStructure   *tlformula;
    OptionStructure *autonum;
    Widget          tround;
    OptionStructure *tinout;
    Widget          tpen;
    SpinStructure   *tlinew;
    OptionStructure *tlines;
    OptionStructure *tminout;
    Widget          tmpen;
    SpinStructure   *tmlinew;
    OptionStructure *tmlines;
    SpinStructure   *tlen;
    SpinStructure   *tmlen;
    Widget          baronoff;
    Widget          barpen;
    SpinStructure   *barlinew;
    OptionStructure *barlines;

    OptionStructure *specticks;
    SpinStructure   *nspec;
    TextStructure   *specloc[MAX_TICKS];
    TextStructure   *speclabel[MAX_TICKS];
} AGridUI;

typedef struct {
    Widget top;
    
    SpinStructure   *v_x;
    SpinStructure   *v_y;
    OptionStructure *arrow_end;
    
    OptionStructure *a_type;
    SpinStructure   *a_length;
    SpinStructure   *a_dL_ff;
    SpinStructure   *a_lL_ff;
} LineUI;

typedef struct {
    Widget top;
    
    SpinStructure   *width;
    SpinStructure   *height;
} BoxUI;

typedef struct {
    Widget top;
    
    SpinStructure   *width;
    SpinStructure   *height;
    
    SpinStructure   *angle1;
    SpinStructure   *angle2;
    
    OptionStructure *closure_type;
    Widget          draw_closure;
} ArcUI;

typedef struct {
    Widget          top;

    Widget          main_tp;
    Widget          odata_tp;

    TextStructure   *x;
    TextStructure   *y;
    
    SpinStructure   *offsetx;
    SpinStructure   *offsety;
    SpinStructure   *angle;
    
    SpinStructure   *linew;
    OptionStructure *lines;
    Widget          linepen;
    Widget          fillpen;
    
    LineUI          *line_ui;
    BoxUI           *box_ui;
    ArcUI           *arc_ui;
} ObjectUI;

typedef struct {
    Widget          top;
    Widget          main_tp;
    Widget          frame_tp;
    
    TextStructure   *x;
    TextStructure   *y;
    
    SpinStructure   *offsetx;
    SpinStructure   *offsety;
    
    TextStructure   *text;

    OptionStructure *font;
    SpinStructure   *size;
    OptionStructure *color;
    OptionStructure *just;
    SpinStructure   *angle;

    OptionStructure *frame_decor;
    SpinStructure   *frame_offset;
    SpinStructure   *linew;
    OptionStructure *lines;
    Widget          linepen;
    Widget          fillpen;
    
    Widget          arrow_flag;
    OptionStructure *a_type;
    SpinStructure   *a_length;
    SpinStructure   *a_dL_ff;
    SpinStructure   *a_lL_ff;
} ATextUI;

typedef struct {
    Widget          top;

    OptionStructure *type;
    OptionStructure *color;
} RegionUI;

struct _ExplorerUI {
    Widget       top;
    Widget       tree;
    Widget       instantupdate;
    WidgetList   aacbuts;
    TextStructure *idstr;
    Widget       scrolled_window;
    
    int          homogeneous_parent;

    ProjectUI    *project_ui;
    SSDataUI     *ssd_ui;
    FrameUI      *frame_ui;
    GraphUI      *graph_ui;
    SetUI        *set_ui;
    AGridUI      *axisgrid_ui;
    AxisUI       *axis_ui;
    ObjectUI     *object_ui;
    ATextUI      *atext_ui;
    RegionUI     *region_ui;

    Widget       popup;

    Widget       popup_hide_bt;
    Widget       popup_show_bt;

    Widget       popup_delete_bt;
    Widget       popup_duplicate_bt;

    Widget       project_popup;

    Widget       project_popup_show_bt;

    Widget       project_popup_save_bt;
    Widget       project_popup_save_as_bt;
    Widget       project_popup_revert_to_saved_bt;

    Widget       project_popup_close_bt;

    Widget       edit_undo_bt;
    Widget       edit_redo_bt;
   
    Widget       insert_frame_bt;
    Widget       insert_graph_bt;
    Widget       insert_ssd_bt;
    Widget       insert_set_bt;
    Widget       insert_axisgrid_bt;
    Widget       insert_axis_bt;
    Widget       insert_object_pane;
    Widget       insert_line_bt;
    Widget       insert_box_bt;
    Widget       insert_arc_bt;
    Widget       insert_text_bt;
    
    Pixmap       a_icon;
    Pixmap       h_icon;
};

void oc_explorer_cb(OptionStructure *opt, int a, void *data);
void tb_explorer_cb(Widget but, int a, void *data);
void scale_explorer_cb(Widget scale, int a, void *data);
void sp_explorer_cb(SpinStructure *spinp, double a, void *data);
void text_explorer_cb(TextStructure *cst, char *s, void *data);
void pen_explorer_cb(Widget but, const Pen *pen, void *data);
void format_explorer_cb(FormatStructure *fstr, const Format *format, void *data);

ProjectUI *create_project_ui(ExplorerUI *eui);
void update_project_ui(ProjectUI *ui, Quark *q);
int set_project_data(ProjectUI *ui, Quark *q, void *caller);

SSDataUI *create_ssd_ui(ExplorerUI *eui);
void update_ssd_ui(SSDataUI *ui, Quark *q);
int set_ssd_data(SSDataUI *ui, Quark *q, void *caller);

FrameUI *create_frame_ui(ExplorerUI *eui);
void update_frame_ui(FrameUI *ui, Quark *q);
int set_frame_data(FrameUI *ui, Quark *q, void *caller);

GraphUI *create_graph_ui(ExplorerUI *eui);
void update_graph_ui(GraphUI *ui, Quark *q);
int graph_set_data(GraphUI *ui, Quark *q, void *caller);

SetUI *create_set_ui(ExplorerUI *eui);
void update_set_ui(SetUI *ui, Quark *q);
int set_set_data(SetUI *ui, Quark *q, void *caller);

AGridUI *create_axisgrid_ui(ExplorerUI *eui);
void update_axisgrid_ui(AGridUI *ui, Quark *q);
int set_axisgrid_data(AGridUI *ui, Quark *q, void *caller);

AxisUI *create_axis_ui(ExplorerUI *eui);
void update_axis_ui(AxisUI *ui, Quark *q);
int set_axis_data(AxisUI *ui, Quark *q, void *caller);

ObjectUI *create_object_ui(ExplorerUI *eui);
void update_object_ui(ObjectUI *ui, Quark *q);
int set_object_data(ObjectUI *ui, Quark *q, void *caller);

ATextUI *create_atext_ui(ExplorerUI *eui);
void update_atext_ui(ATextUI *ui, Quark *q);
int set_atext_data(ATextUI *ui, Quark *q, void *caller);

RegionUI *create_region_ui(ExplorerUI *eui);
void update_region_ui(RegionUI *ui, Quark *q);
int set_region_data(RegionUI *ui, Quark *q, void *caller);

#endif /* __EXPLORER_H_ */
