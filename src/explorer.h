/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2003 Grace Development Team
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
#include "graphs.h"

#include "motifinc.h"
#include "ListTree.h"

typedef struct {
    Widget          top;
    
    OptionStructure *bg_color;
    Widget          bg_fill;

    OptionStructure *datehint;
    Widget          refdate;
    Widget          wrap_year;
    Widget          two_digits_years;
} ProjectUI;

typedef struct {
    Widget          top;
    
    Widget          main_tp;
    Widget          frame_tp;
    Widget          titles_tp;
    Widget          legends_tp;
    Widget          legendbox_tp;

    Widget          view_xv1;
    Widget          view_xv2;
    Widget          view_yv1;
    Widget          view_yv2;

    TextStructure   *label_title_text;
    TextStructure   *label_subtitle_text;
    OptionStructure *title_color;
    OptionStructure *title_font;
    Widget          title_size;
    OptionStructure *stitle_color;
    OptionStructure *stitle_font;
    Widget          stitle_size;

    OptionStructure *frame_framestyle_choice;
    OptionStructure *frame_color_choice;
    OptionStructure *frame_pattern_choice;
    OptionStructure *frame_lines_choice;
    SpinStructure   *frame_linew_choice;
    OptionStructure *frame_fillcolor_choice;
    OptionStructure *frame_fillpattern_choice;

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
    Widget          legend_charsize;
    OptionStructure *legend_color;
    OptionStructure *legend_boxfillcolor;
    OptionStructure *legend_boxfillpat;
    SpinStructure   *legend_boxlinew;
    OptionStructure *legend_boxlines;
    OptionStructure *legend_boxcolor;
    OptionStructure *legend_boxpattern;
} FrameUI;

typedef struct {
    Widget          top;
    
    Widget          main_tp;
    Widget          locator_tp;

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

    OptionStructure *type;
    OptionStructure *symbols;
    Widget          symsize;
    SpinStructure   *symskip;
    OptionStructure *symcolor;
    OptionStructure *sympattern;
    OptionStructure *symfillcolor;
    OptionStructure *symfillpattern;
    SpinStructure   *symlinew;
    OptionStructure *symlines;
    Widget          symchar;
    OptionStructure *char_font;

    OptionStructure *color;
    OptionStructure *pattern;
    SpinStructure   *width;
    Widget          dropline;
    OptionStructure *lines;
    OptionStructure *linet;
    OptionStructure *filltype;
    OptionStructure *fillrule;
    OptionStructure *fillpat;
    OptionStructure *fillcol;
    Widget          baseline;
    OptionStructure *baselinetype;

    TextStructure   *legend_str;

    Widget          errbar_active;
    OptionStructure *errbar_ptype;
    OptionStructure *errbar_color;
    OptionStructure *errbar_pattern;
    Widget          errbar_size;
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
    Widget          avalue_charsize ;
    Widget          avalue_angle;
    OptionStructure *avalue_format;
    OptionStructure *avalue_precision;
    Widget          avalue_offsetx;
    Widget          avalue_offsety;
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
    Widget          labelcharsize;
    OptionStructure *labelcolor;
    OptionStructure *labelop;
    
    Widget          tmajor;
    SpinStructure   *nminor;
    
    OptionStructure *tickop;
    OptionStructure *ticklop;
    OptionStructure *tlform;
    OptionStructure *tlprec;
    OptionStructure *tlfont;
    Widget          tlcharsize;
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
    OptionStructure *tgridcol;
    SpinStructure   *tgridlinew;
    OptionStructure *tgridlines;
    Widget          tmgrid;
    OptionStructure *tmgridcol;
    SpinStructure   *tmgridlinew;
    OptionStructure *tmgridlines;
    Widget          tlen;
    Widget          tmlen;
    OptionStructure *tinout;
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
    
    SpinStructure   *length;
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
    
    TextStructure   *text;
    OptionStructure *font;
    Widget size;
    OptionStructure *just;

    SpinStructure   *linew;
    OptionStructure *lines;
    Widget          linepen;
    Widget          fillpen;
} StringUI;

typedef struct {
    Widget          top;

    Widget          main_tp;
    Widget          odata_tp;

    Widget          active;
    
    TextStructure   *x;
    TextStructure   *y;
    OptionStructure *loctype;
    
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
    StringUI        *string_ui;
} ObjectUI;


typedef struct {
    Widget       top;
    Widget       tree;
    Widget       instantupdate;
    WidgetList   aacbuts;
    Widget       scrolled_window;
    ListTreeItem *project;
    
    Widget       editmenu;
    
    ProjectUI    *project_ui;
    FrameUI      *frame_ui;
    GraphUI      *graph_ui;
    SetUI        *set_ui;
    AxisUI       *axis_ui;
    ObjectUI     *object_ui;
} ExplorerUI;

void oc_explorer_cb(OptionStructure *opt, int a, void *data);
void tb_explorer_cb(Widget but, int a, void *data);
void scale_explorer_cb(Widget scale, int a, void *data);
void sp_explorer_cb(SpinStructure *spinp, double a, void *data);
void text_explorer_cb(TextStructure *cst, char *s, void *data);
void titem_explorer_cb(Widget w, char *s, void *data);

ProjectUI *create_project_ui(ExplorerUI *eui);
void update_project_ui(ProjectUI *ui, Quark *q);
int set_project_data(ProjectUI *ui, Quark *q, void *caller);

FrameUI *create_frame_ui(ExplorerUI *eui);
void update_frame_ui(FrameUI *ui, Quark *q);
int set_frame_data(FrameUI *ui, Quark *q, void *caller);

GraphUI *create_graph_ui(ExplorerUI *eui);
void update_graph_ui(GraphUI *ui, Quark *q);
int set_graph_data(GraphUI *ui, Quark *q, void *caller);

SetUI *create_set_ui(ExplorerUI *eui);
void update_set_ui(SetUI *ui, Quark *q);
int set_set_data(SetUI *ui, Quark *q, void *caller);

AxisUI *create_axis_ui(ExplorerUI *eui);
void update_axis_ui(AxisUI *ui, Quark *q);
int set_axis_data(AxisUI *ui, Quark *q, void *caller);

ObjectUI *create_object_ui(ExplorerUI *eui);
void update_object_ui(ObjectUI *ui, Quark *q);
int set_object_data(ObjectUI *ui, Quark *q, void *caller);

#endif /* __EXPLORER_H_ */
