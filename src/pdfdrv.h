/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1996-2002 Grace Development Team
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

typedef enum {
    PDF_1_2,
    PDF_1_3,
    PDF_1_4
} PDFCompatibility;

typedef enum {
    COLORSPACE_GRAYSCALE,
    COLORSPACE_RGB,
    COLORSPACE_CMYK
} PDFColorSpace;

#define DEFAULT_COLORSPACE  COLORSPACE_RGB

int pdf_initgraphics(const Canvas *canvas, void *data,
    const CanvasStats *cstats);

void pdf_drawpixel(const Canvas *canvas, void *data, const VPoint *vp);
void pdf_drawpolyline(const Canvas *canvas, void *data,
    const VPoint *vps, int n, int mode);
void pdf_fillpolygon(const Canvas *canvas, void *data,
    const VPoint *vps, int nc);
void pdf_drawarc(const Canvas *canvas, void *data,
    const VPoint *vp1, const VPoint *vp2, double a1, double a2);
void pdf_fillarc(const Canvas *canvas, void *data,
    const VPoint *vp1, const VPoint *vp2, double a1, double a2, int mode);
void pdf_putpixmap(const Canvas *canvas, void *data,
    const VPoint *vp, int width, int height, char *databits,
    int pixmap_bpp, int bitmap_pad, int pixmap_type);
void pdf_puttext(const Canvas *canvas, void *data,
    const VPoint *vp, const char *s, int len, int font, const TextMatrix *tm,
    int underline, int overline, int kerning);

void pdf_leavegraphics(const Canvas *canvas, void *data,
    const CanvasStats *cstats);

int pdf_op_parser(const Canvas *canvas, void *data, const char *opstring);

#if defined(NONE_GUI)
#  define pdf_gui_setup NULL
#else
void pdf_gui_setup(const Canvas *canvas, void *data);
#endif
