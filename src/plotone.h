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

/*
 * plotone.h
 */

#ifndef __PLOTONE_H_
#define __PLOTONE_H_

#include "grace.h"

typedef struct {
    Canvas *canvas;

    int refn;
    double *refx, *refy;
    double offset, epsilon;
    
    int ndsets;
    
    int first_pass, last_pass;
} plot_rt_t;

void drawgraph(const Quark *project);
void do_hardcopy(const Quark *project);

int draw_graph(Quark *gr, plot_rt_t *plot_rt);
void draw_set(Quark *pset, plot_rt_t *plot_rt);

void draw_frame(Quark *q, plot_rt_t *plot_rt);
void fillframe(Canvas *canvas, Quark *q);

void drawsetfill(Quark *pset, plot_rt_t *plot_rt);
void drawsetline(Quark *pset, plot_rt_t *plot_rt);
void drawsetbars(Quark *pset, plot_rt_t *plot_rt);
void drawsetsyms(Quark *pset, plot_rt_t *plot_rt);
void drawsetavalues(Quark *pset, plot_rt_t *plot_rt);
void drawseterrbars(Quark *pset, plot_rt_t *plot_rt);
void drawsethilo(Quark *pset, plot_rt_t *plot_rt);
void drawcirclexy(Quark *pset, plot_rt_t *plot_rt);
void drawsetvmap(Quark *pset, plot_rt_t *plot_rt);
void drawsetboxplot(Quark *pset, plot_rt_t *plot_rt);
void draw_pie_chart_set(Quark *pset, plot_rt_t *plot_rt);

void symplus(Canvas *canvas, const VPoint *vp, double s);
void symx(Canvas *canvas, const VPoint *vp, double s);
void symsplat(Canvas *canvas, const VPoint *vp, double s);

int drawxysym(Canvas *canvas, const VPoint *vp, const Symbol *sym);
void drawerrorbar(Canvas *canvas,
    const VPoint *vp1,const  VPoint *vp2, Errbar *eb);

void draw_region(Canvas *canvas, Quark *q);

void draw_axis(Canvas *canvas, Quark *q);
void draw_object(Canvas *canvas, Quark *q);

void draw_arrowhead(Canvas *canvas, const VPoint *vp1, const VPoint *vp2,
    const Arrow *arrowp, const Pen *pen, const Pen *fill);

void draw_legends(Quark *q, plot_rt_t *plot_rt);

void draw_ref_point(Canvas *canvas, Quark *gr);

#endif /* __PLOTONE_H_ */
