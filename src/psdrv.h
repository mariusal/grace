/*
 * Grace - Graphics for Exploratory Data Analysis
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-95 Paul J Turner, Portland, OR
 * Copyright (c) 1996-98 GRACE Development Team
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

#define PS_FORMAT   0
#define EPS_FORMAT  1
/*
 * #define EPSI_FORMAT  2
 */

#define DEFAULT_PS_FORMAT PS_FORMAT

int psprintinitgraphics(void);
int epsinitgraphics(void);

void ps_drawpixel(VPoint vp);
void ps_drawpolyline(VPoint *vps, int n, int mode);
void ps_fillpolygon(VPoint *vps, int nc);
void ps_drawarc(VPoint vp1, VPoint vp2, int a1, int a2);
void ps_fillarc(VPoint vp1, VPoint vp2, int a1, int a2);
void ps_putpixmap(VPoint vp, int width, int height, 
     char *databits, int pixmap_bpp, int bitmap_pad, int pixmap_type);
void ps_puttext(VPoint start, VPoint end, double size, 
                                            CompositeString *cstring);

void ps_leavegraphics(void);
int ps_op_parser(char *opstring);

#if defined(NONE_GUI)
#  define ps_gui_setup NULL
#else
void ps_gui_setup(void);
#endif
