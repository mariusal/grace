/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2003 Grace Development Team
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

#ifndef __CORE_UTILS_H_
#define __CORE_UTILS_H_

#include "grace/core.h"

/*
 * axis type masks
 */
#define AXIS_MASK_X  1
#define AXIS_MASK_Y  2
#define AXIS_MASK_XY 3

#define GSCROLL_LEFT    0
#define GSCROLL_RIGHT   1
#define GSCROLL_DOWN    2
#define GSCROLL_UP      3

#define GZOOM_SHRINK    0
#define GZOOM_EXPAND    1

/* Order of matrix fill (inversion mask bits) */
#define GA_ORDER_V_INV  1
#define GA_ORDER_H_INV  2
#define GA_ORDER_HV_INV 4

/* Default page offsets and gaps for graph arranging */
#define GA_OFFSET_DEFAULT    0.15
#define GA_GAP_DEFAULT       0.2


Symbol *symbol_new();
void symbol_free(Symbol *sym);
SetLine *setline_new();
void setline_free(SetLine *sl);
BarLine *barline_new(void);
RiserLine *riserline_new(void);

Quark *graph_next(Quark *project);

Quark *graph_get_current(const Quark *project);

int is_zero_axis(tickmarks *t);
int activate_tick_labels(tickmarks *t, int flag);

int islogx(Quark *gr);
int islogy(Quark *gr);

int islogitx(Quark *gr);
int islogity(Quark *gr);

int graph_get_viewport(Quark *gr, view *v);

int is_log_axis(const Quark *q);
int is_logit_axis(const Quark *q);

int number_of_graphs(Quark *project);
int select_graph(Quark *g);

int is_set_dataless(Quark *pset);

#define is_set_drawable(p) (quark_is_active(p) && !is_set_dataless(p))

int number_of_sets(Quark *gr);

int load_comments_to_legend(Quark *p);

char *dataset_colname(int col);

int is_refpoint_active(Quark *gr);

int set_refpoint(Quark *gr, const WPoint *wp);

WPoint get_refpoint(Quark *gr);

#define getx(p) set_get_col(p, DATA_X)
#define gety(p) set_get_col(p, DATA_Y)

double setybase(Quark *p);

int get_descendant_sets(Quark *q, Quark ***sets);

int set_set_colors(Quark *p, unsigned int color);
Quark *grace_set_new(Quark *gr);

int copysetdata(Quark *psrc, Quark *pdest);

void project_postprocess(Quark *pr);
int project_get_viewport(const Quark *project, double *vx, double *vy);

char *object_types(OType type);

void move_object(Quark *q, VVector shift);
int object_place_at_vp(Quark *q, VPoint vp);

char *scale_types(ScaleType it);
ScaleType get_scale_type_by_name(const char *name);

char *get_format_types(FormatType f);
FormatType get_format_type_by_name(const char *name);

int graph_scroll(Quark *gr, int type);
int graph_zoom(Quark *gr, int type);

int arrange_graphs(Quark **graphs, int ngraphs,
                   int nrows, int ncols, int order, int snake,
                   double loff, double roff, double toff, double boff,
                   double vgap, double hgap,
                   int hpack, int vpack);

void autoscale_bysets(Quark **sets, int nsets, int autos_type);
int autoscale_graph(Quark *gr, int autos_type);
void autotick_graph_axes(Quark *q, int amask);

void move_legend(Quark *gr, const VVector *shift);

void rescale_viewport(Quark *pr, double ext_x, double ext_y);

#endif /* __CORE_UTILS_H_ */
