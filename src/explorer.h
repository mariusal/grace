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

#include "graphs.h"
#include "motifinc.h"
#include "ListTree.h"

typedef struct {
    Widget          top;
    
    OptionStructure *bg_color;
    Widget          bg_fill;
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
    
} GraphUI;

typedef struct {
    Widget       top;
    Widget       tree;
    Widget       instantupdate;
    WidgetList   aacbuts;
    Widget       scrolled_window;
    ListTreeItem *project;
    
    ProjectUI    *project_ui;
    FrameUI      *frame_ui;
    GraphUI      *graph_ui;
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

#endif /* __EXPLORER_H_ */
