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

#include "defines.h"

#include <X11/Xlib.h>

#define CMAP_INSTALL_NEVER      0
#define CMAP_INSTALL_ALWAYS     1
#define CMAP_INSTALL_AUTO       2

int xlibinit(const Canvas *canvas);
void xlibredraw(Window window, int x, int y, int widht, int height);
int xlibinitgraphics(const Canvas *canvas);
void drawxlib(int x, int y, int mode);
void xlibupdatecmap(const Canvas *canvas);
void xlibinitcmap(const Canvas *canvas);

void xlibdrawpixel(const Canvas *canvas, const VPoint *vp);
void xlibdrawpolyline(const Canvas *canvas, const VPoint *vps, int n, int mode);
void xlibfillpolygon(const Canvas *canvas, const VPoint *vps, int nc);
void xlibdrawarc(const Canvas *canvas,
    const VPoint *vp1, const VPoint *vp2, int a1, int a2);
void xlibfillarc(const Canvas *canvas,
    const VPoint *vp1, const VPoint *vp2, int a1, int a2, int mode);
void xlibputpixmap(const Canvas *canvas,
    const VPoint *vp, int width, int height, char *databits,
    int pixmap_bpp, int bitmap_pad, int pixmap_type);

void xlibleavegraphics(const Canvas *canvas);
     
int xconvxlib(double x);
int yconvxlib(double y);
void xlibVPoint2dev(const VPoint *vp, int *x, int *y);
void xlibdev2VPoint(int x, int y, VPoint *vp);
