/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2003,2004 Grace Development Team
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

#ifndef __EXPLORER_H_
#define __EXPLORER_H_

/* for MAX_TICKS */
#include "core_utils.h"

#include "motifinc.h"
#include "ListTree.h"

typedef struct {
    Widget          top;

    Widget          sformat;
    Widget          description;
    
    OptionStructure *bg_color;
    Widget          bg_fill;

    SpinStructure   *fsize_scale;
    SpinStructure   *lwidth_scale;

    Widget          refdate;
    Widget          wrap_year;
    Widget          two_digits_years;
} ProjectUI;

typedef struct {
    Widget          top;
    
    Widget          main_tp;
    Widget          legends_tp;
    Widget          legendbox_tp;

    Widget          active;

    Widget          view_xv1;
    Widget          view_xv2;
    Widget          view_yv1;
    Widget          view_yv2;

    OptionStructure *frame_framestyle_choice;
    Widget          frame_pen;
    OptionStructure *frame_lines_choice;
    SpinStructure   *frame_linew_choice;
    Widget          frame_fillpen;

    OptionStructure *legend_acorner;
    SpinStructure   *legend_x;
    SpinStructure   *legend_y;
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

    Widget          active;

    OptionStructure *graph_type;
    Widget          stacked;

    Widget          start_x;
    Widget          stop_x;
    OptionStructure *scale_x;
    Widget          invert_x;
    Widget          start_y;
    Widget          stop_y;
    OptionStructure *scale_y;
    Widget          invert_y;

    Widget          flip_xy;
    SpinStructure   *bargap;
    Widget          znorm;

    OptionStructure *delta;
    OptionStructure *loc_formatx;
    OptionStructure *loc_formaty;
    OptionStructure *loc_precx;
    OptionStructure *loc_precy;
    Widget          locx;
    Widget          locy;
    Widget          fixedp;
    
} GraphUI;

typedef struct {
    Widget          top;
    
    Widget          main_tp;
    Widget          symbol_tp;
    Widget          line_tp;
    Widget          errbar_tp;
    Widget          avalue_tp;

    Widget          active;

    OptionStructure *type;
    OptionStructure *symbols;
    SpinStructure   *symsize;
    SpinStructure   *symskip;
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
    OptionStructure *errbar_ptype;
    Widget          errbar_pen;
    SpinStructure   *errbar_size;
    SpinStructure   *errbar_width;
    OptionStructure *errbar_lines;
    SpinStructure   *errbar_riserlinew;
    OptionStructure *errbar_riserlines;
    Widget          errbar_aclip;
    SpinStructure   *errbar_cliplen;

    Widget          avalue_active;
    OptionStructure *avalue_type;
    OptionStructure *avalue_font;
    OptionStructure *avalue_color;
    SpinStructure   *avalue_charsize;
    Widget          avalue_angle;
    OptionStructure *avalue_format;
    OptionStructure *avalue_precision;
    Widget          avalue_offsetx;
    Widget          avalue_offsety;
    OptionStructure *avalue_just;
    Widget          avalue_prestr;
    Widget          avalue_appstr;
} SetUI;

typedef struct {
    Widget          top;

    Widget          main_tp;
    Widget          label_tp;
    Widget          ticklabel_tp;
    Widget          tickmark_tp;
    Widget          special_tp;

    Widget          active;

    OptionStructure *type;
    
    Widget          zero;
    Widget          offx;
    Widget          offy;
    Widget          tonoff;
    Widget          tlonoff;
    TextStructure   *label;
    OptionStructure *labellayout;
    OptionStructure *labelplace;
    Widget          labelspec_rc;

    Widget          labelspec_para;
    Widget          labelspec_perp;
    OptionStructure *labelfont;
    SpinStructure   *labelcharsize;
    OptionStructure *labelcolor;
    OptionStructure *labelop;
    
    Widget          tmajor;
    SpinStructure   *nminor;
    
    OptionStructure *tickop;
    OptionStructure *ticklop;
    OptionStructure *tlform;
    OptionStructure *tlprec;
    OptionStructure *tlfont;
    SpinStructure   *tlcharsize;
    OptionStructure *tlcolor;
    Widget          tlappstr;
    Widget          tlprestr;
    OptionStructure *tlskip;
    OptionStructure *tlstarttype;
    Widget          tlstart;
    OptionStructure *tlstoptype;
    Widget          tlstop;
    OptionStructure *tlgaptype;
    Widget          tlgap_rc;
    Widget          tlgap_para;
    Widget          tlgap_perp;
    Widget          tlangle;
    OptionStructure *tlstagger;
    TextStructure   *tlformula;
    OptionStructure *autonum;
    Widget          tround;
    Widget          tgrid;
    OptionStructure *tinout;
    OptionStructure *tgridcol;
    SpinStructure   *tgridlinew;
    OptionStructure *tgridlines;
    Widget          tmgrid;
    OptionStructure *tminout;
    OptionStructure *tmgridcol;
    SpinStructure   *tmgridlinew;
    OptionStructure *tmgridlines;
    SpinStructure   *tlen;
    SpinStructure   *tmlen;
    Widget          baronoff;
    OptionStructure *barcolor;
    SpinStructure   *barlinew;
    OptionStructure *barlines;

    OptionStructure *specticks;
    SpinStructure   *nspec;
    Widget          specloc[MAX_TICKS];
    Widget          speclabel[MAX_TICKS];
} AxisUI;

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
    
    Widget          angle1;
    Widget          angle2;
    
    OptionStructure *fillmode;
} ArcUI;

typedef struct {
    Widget          top;

    Widget          main_tp;
    Widget          odata_tp;

    Widget          active;
    
    TextStructure   *x;
    TextStructure   *y;
    
    SpinStructure   *offsetx;
    SpinStructure   *offsety;
    Widget angle;
    
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
    
    Widget          active;
    
    TextStructure   *x;
    TextStructure   *y;
    
    SpinStructure   *offsetx;
    SpinStructure   *offsety;
    
    TextStructure   *text;

    OptionStructure *font;
    SpinStructure   *size;
    OptionStructure *color;
    OptionStructure *just;
    Widget          angle;

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

struct _ExplorerUI {
    Widget       top;
    Widget       tree;
    Widget       instantupdate;
    WidgetList   aacbuts;
    TextStructure *idstr;
    Widget       scrolled_window;
    ListTreeItem *project;
    
    ProjectUI    *project_ui;
    FrameUI      *frame_ui;
    GraphUI      *graph_ui;
    SetUI        *set_ui;
    AxisUI       *axis_ui;
    ObjectUI     *object_ui;
    ATextUI      *atext_ui;

    Widget       popup;

    Widget       popup_delete_bt;
    Widget       popup_duplicate_bt;

    Widget       popup_bring_to_front_bt;
    Widget       popup_send_to_back_bt;
    Widget       popup_move_up_bt;
    Widget       popup_move_down_bt;
    
    Widget       insert_frame_bt;
    Widget       insert_graph_bt;
    Widget       insert_set_bt;
    Widget       insert_axis_bt;
    Widget       insert_object_pane;
    Widget       insert_line_bt;
    Widget       insert_box_bt;
    Widget       insert_arc_bt;
    Widget       insert_text_bt;
};

void oc_explorer_cb(OptionStructure *opt, int a, void *data);
void tb_explorer_cb(Widget but, int a, void *data);
void scale_explorer_cb(Widget scale, int a, void *data);
void sp_explorer_cb(SpinStructure *spinp, double a, void *data);
void text_explorer_cb(TextStructure *cst, char *s, void *data);
void titem_explorer_cb(Widget w, char *s, void *data);
void pen_explorer_cb(Widget but, const Pen *pen, void *data);

ProjectUI *create_project_ui(ExplorerUI *eui);
void update_project_ui(ProjectUI *ui, Quark *q);
int set_project_data(ProjectUI *ui, Quark *q, void *caller);

FrameUI *create_frame_ui(ExplorerUI *eui);
void update_frame_ui(FrameUI *ui, Quark *q);
int set_frame_data(FrameUI *ui, Quark *q, void *caller);

GraphUI *create_graph_ui(ExplorerUI *eui);
void update_graph_ui(GraphUI *ui, Quark *q);
int graph_set_data(GraphUI *ui, Quark *q, void *caller);

SetUI *create_set_ui(ExplorerUI *eui);
void update_set_ui(SetUI *ui, Quark *q);
int set_set_data(SetUI *ui, Quark *q, void *caller);

AxisUI *create_axis_ui(ExplorerUI *eui);
void update_axis_ui(AxisUI *ui, Quark *q);
int set_axis_data(AxisUI *ui, Quark *q, void *caller);

ObjectUI *create_object_ui(ExplorerUI *eui);
void update_object_ui(ObjectUI *ui, Quark *q);
int set_object_data(ObjectUI *ui, Quark *q, void *caller);

ATextUI *create_atext_ui(ExplorerUI *eui);
void update_atext_ui(ATextUI *ui, Quark *q);
int set_atext_data(ATextUI *ui, Quark *q, void *caller);

#endif /* __EXPLORER_H_ */
