/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-95 Paul J Turner, Portland, OR
 * Copyright (c) 1996-99 Grace Development Team
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
#  define RST_FORMAT_JPG  3
#ifdef HAVE_LIBJPEG
#endif

#  define DEFAULT_RASTER_FORMAT RST_FORMAT_GIF

/* PNM sub-formats */
#define PNM_FORMAT_PBM  0
#define PNM_FORMAT_PGM  1
#define PNM_FORMAT_PPM  2

#define DEFAULT_PNM_FORMAT PNM_FORMAT_PPM

#ifdef HAVE_LIBJPEG
#  define JPEG_DCT_IFAST  0
#  define JPEG_DCT_ISLOW  1
#  define JPEG_DCT_FLOAT  2

#define JPEG_DCT_DEFAULT    JPEG_DCT_ISLOW
#endif

#define INTENSITY(r, g, b) ((299*r + 587*g + 114*b)/1000)

void rst_drawpixel(VPoint vp);
void rst_drawpolyline(VPoint *vps, int n, int mode);
void rst_fillpolygon(VPoint *vps, int nc);
void rst_drawarc(VPoint vp1, VPoint vp2, int a1, int a2);
void rst_fillarc(VPoint vp1, VPoint vp2, int a1, int a2);
void rst_putpixmap(VPoint vp, int width, int height, 
     char *databits, int pixmap_bpp, int bitmap_pad, int pixmap_type);
void rst_leavegraphics(void);

int gdinitgraphics(void);
int gifinitgraphics(void);
int pnminitgraphics(void);
#ifdef HAVE_LIBJPEG
int jpginitgraphics(void);
#endif

int gif_op_parser(char *opstring);
int pnm_op_parser(char *opstring);
#ifdef HAVE_LIBJPEG
int jpg_op_parser(char *opstring);
#endif

#if defined(NONE_GUI)
#  define gif_gui_setup NULL
#  define pnm_gui_setup NULL
#  ifdef HAVE_LIBJPEG
#    define jpg_gui_setup NULL
#  endif
#else
void gif_gui_setup(void);
void pnm_gui_setup(void);
#  ifdef HAVE_LIBJPEG
void jpg_gui_setup(void);
#endif
#endif
