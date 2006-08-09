/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2005 Grace Development Team
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

Quark *graph_next(Quark *project);

Quark *graph_get_current(const Quark *project);

int islogx(Quark *gr);
int islogy(Quark *gr);

int islogitx(Quark *gr);
int islogity(Quark *gr);

int is_log_axis(const Quark *q);
int is_logit_axis(const Quark *q);

int number_of_frames(Quark *project);

int number_of_graphs(Quark *project);
int select_graph(Quark *g);

#define getx(p) set_get_col(p, DATA_X)
#define gety(p) set_get_col(p, DATA_Y)

int set_set_colors(Quark *p, unsigned int color);
Quark *gapp_set_new(Quark *gr);

int copysetdata(Quark *psrc, Quark *pdest);

int kill_ssd_cb(Quark *q, int etype, void *data);
Quark *gapp_ssd_new(Quark *parent);

int project_get_viewport(const Quark *project, double *vx, double *vy);

void move_object(Quark *q, VVector shift);
int object_place_at_vp(Quark *q, VPoint vp);

int graph_scroll(Quark *gr, int type);
int graph_zoom(Quark *gr, int type);

int arrange_frames(Quark **graphs, int ngraphs,
                   int nrows, int ncols, int order, int snake,
                   double loff, double roff, double toff, double boff,
                   double vgap, double hgap,
                   int hpack, int vpack);

void autoscale_bysets(Quark **sets, int nsets, int autos_type);
int autoscale_graph(Quark *gr, int autos_type);
void autotick_graph_axes(Quark *q, int amask);

void move_legend(Quark *gr, const VVector *shift);

void rescale_viewport(Quark *pr, double ext_x, double ext_y);

/* set_utils.c */
int getsetminmax(Quark **sets, int nsets, 
    double *xmin, double *xmax, double *ymin, double *ymax);
int getsetminmax_c(Quark **sets, int nsets,
    double *xmin, double *xmax, double *ymin, double *ymax, int ivec);
int set_point(Quark *pset, int seti, const WPoint *wp);
int get_point(Quark *pset, int seti, WPoint *wp);
int get_datapoint(Quark *pset, int ind, int *ncols, Datapoint *dpoint);
int set_point_shift(Quark *pset, int seti, const VVector *vshift);

void del_point(Quark *pset, int pt);

#endif /* __CORE_UTILS_H_ */
