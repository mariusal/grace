/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2001 Grace Development Team
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
#include "draw.h"
#include "graphs.h"

#define BAR_HORIZONTAL  0
#define BAR_VERTICAL    1

void drawgraph(Grace *grace);
void do_hardcopy(Grace *grace);

void plotone(Canvas *canvas, int gno);

void xyplot(Canvas *canvas, int gno);
void draw_polar_graph(Canvas *canvas, int gno);
void draw_smith_chart(Canvas *canvas, int gno);
void draw_pie_chart(Canvas *canvas, int gno);

void drawframe(Canvas *canvas, int gno);
void fillframe(Canvas *canvas, int gno);

void drawsetfill(Canvas *canvas, int gno, int setno, set *s,
                 int refn, double *refx, double *refy, double offset);
void drawsetline(Canvas *canvas, int gno, int setno, set *s,
                 int refn, double *refx, double *refy, double offset);
void drawsetbars(Canvas *canvas, int gno, int setno, set *s,
                 int refn, double *refx, double *refy, double offset);
void drawsetsyms(Canvas *canvas, int gno, int setno, set *s,
                 int refn, double *refx, double *refy, double offset);
void drawsetavalues(Canvas *canvas, int gno, int setno, set *s,
                 int refn, double *refx, double *refy, double offset);
void drawseterrbars(Canvas *canvas, int gno, int setno, set *s,
                 int refn, double *refx, double *refy, double offset);
void drawsethilo(Canvas *canvas, set *s);
void drawcirclexy(Canvas *canvas, set *s);
void drawsetvmap(Canvas *canvas, int gno, set *s);
void drawsetboxplot(Canvas *canvas, set *s);

void symplus(Canvas *canvas, const VPoint *vp, double s);
void symx(Canvas *canvas, const VPoint *vp, double s);
void symsplat(Canvas *canvas, const VPoint *vp, double s);

int drawxysym(Canvas *canvas, const VPoint *vp, double size, int symtype,
    const Pen *sympen, const Pen *symfillpen, char s);
void drawerrorbar(Canvas *canvas,
    const VPoint *vp1,const  VPoint *vp2, Errbar *eb);

void draw_region(Canvas *canvas, region *r);

void draw_objects(Canvas *canvas, int gno);

void draw_arrowhead(Canvas *canvas, const VPoint *vp1, const VPoint *vp2,
    const Arrow *arrowp, const Pen *pen, const Pen *fill);

void dolegend(Canvas *canvas, int gno);
void putlegends(Canvas *canvas,
    int gno, const VPoint *vp, double ldist, double sdist, double yskip);

void draw_titles(Canvas *canvas, int gno);

void draw_ref_point(Canvas *canvas, int gno);

void draw_regions(Canvas *canvas, int gno);

#endif /* __PLOTONE_H_ */
