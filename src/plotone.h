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

#include "defines.h"
#include "grace/canvas.h"
#include "graphs.h"

void drawgraph(Grace *grace);
void do_hardcopy(Grace *grace);

void draw_graph(Canvas *canvas, Quark *gr);

void xyplot(Canvas *canvas, Quark *gr);
void draw_polar_graph(Canvas *canvas, Quark *gr);
void draw_smith_chart(Canvas *canvas, Quark *gr);
void draw_pie_chart(Canvas *canvas, Quark *gr);

void drawframe(Canvas *canvas, Quark *q);
void fillframe(Canvas *canvas, Quark *q);

void drawsetfill(Canvas *canvas, Quark *pset,
                 int refn, double *refx, double *refy, double offset);
void drawsetline(Canvas *canvas, Quark *pset,
                 int refn, double *refx, double *refy, double offset);
void drawsetbars(Canvas *canvas, Quark *pset,
                 int refn, double *refx, double *refy, double offset);
void drawsetsyms(Canvas *canvas, Quark *pset,
                 int refn, double *refx, double *refy, double offset);
void drawsetavalues(Canvas *canvas, Quark *pset,
                 int refn, double *refx, double *refy, double offset);
void drawseterrbars(Canvas *canvas, Quark *pset,
                 int refn, double *refx, double *refy, double offset);
void drawsethilo(Canvas *canvas, Quark *pset);
void drawcirclexy(Canvas *canvas, Quark *pset);
void drawsetvmap(Canvas *canvas, Quark *pset);
void drawsetboxplot(Canvas *canvas, Quark *pset);

void symplus(Canvas *canvas, const VPoint *vp, double s);
void symx(Canvas *canvas, const VPoint *vp, double s);
void symsplat(Canvas *canvas, const VPoint *vp, double s);

int drawxysym(Canvas *canvas, const VPoint *vp, const Symbol *sym);
void drawerrorbar(Canvas *canvas,
    const VPoint *vp1,const  VPoint *vp2, Errbar *eb);

void draw_region(Canvas *canvas, region *r);

void draw_object(Canvas *canvas, Quark *q);

void draw_arrowhead(Canvas *canvas, const VPoint *vp1, const VPoint *vp2,
    const Arrow *arrowp, const Pen *pen, const Pen *fill);

void dolegend(Canvas *canvas, Quark *gr);
void putlegends(Canvas *canvas, Quark *gr, const VPoint *vp, double maxsymsize);

void draw_titles(Canvas *canvas, Quark *gr);

void draw_ref_point(Canvas *canvas, Quark *gr);

void draw_regions(Canvas *canvas, Quark *gr);

#endif /* __PLOTONE_H_ */
