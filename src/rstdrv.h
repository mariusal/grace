/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1996-99 Grace Development Team
 * Copyright (c) 1991-95 Paul J Turner, Portland, OR
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

#define RST_FORMAT_GD   0
#define RST_FORMAT_GIF  1
#define RST_FORMAT_PNM  2

#define DEFAULT_RASTER_FORMAT RST_FORMAT_GIF

/* PNM sub-formats */
#define PNM_FORMAT_PBM  0
#define PNM_FORMAT_PGM  1
#define PNM_FORMAT_PPM  2

#define DEFAULT_PNM_FORMAT PNM_FORMAT_PPM

#if defined(NONE_GUI)
#  define gif_gui_setup NULL
#  define pnm_gui_setup NULL
#else
void gif_gui_setup(void);
void pnm_gui_setup(void);
#endif

void rst_drawpixel(VPoint vp);
void rst_drawpolyline(VPoint *vps, int n, int mode);
void rst_fillpolygon(VPoint *vps, int nc);
void rst_drawarc(VPoint vp1, VPoint vp2, int a1, int a2);
void rst_fillarc(VPoint vp1, VPoint vp2, int a1, int a2);
void rst_putpixmap(VPoint vp, int width, int height, 
     char *databits, int pixmap_bpp, int bitmap_pad, int pixmap_type);
void rst_leavegraphics(void);

int gifinitgraphics(void);
int gdinitgraphics(void);
int pnminitgraphics(void);
