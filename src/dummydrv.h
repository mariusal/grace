/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1996-2002 Grace Development Team
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

#include "defines.h"

int dummy_initgraphics(const Canvas *canvas, void *data,
    const CanvasStats *cstats);

void dummy_drawpixel(const Canvas *canvas, void *data, const VPoint *vp);
void dummy_drawpolyline(const Canvas *canvas, void *data,
    const VPoint *vps, int n, int mode);
void dummy_fillpolygon(const Canvas *canvas, void *data,
    const VPoint *vps, int nc);
void dummy_drawarc(const Canvas *canvas, void *data,
    const VPoint *vp1, const VPoint *vp2, double a1, double a2);
void dummy_fillarc(const Canvas *canvas, void *data,
    const VPoint *vp1, const VPoint *vp2, double a1, double a2, int mode);
void dummy_putpixmap(const Canvas *canvas, void *data,
    const VPoint *vp, const CPixmap *pm);
void dummy_puttext(const Canvas *canvas, void *data,
    const VPoint *vp, const char *s, int len, int font,
    const TextMatrix *tm, int underline, int overline, int kerning);

void dummy_leavegraphics(const Canvas *canvas, void *data,
    const CanvasStats *cstats);
