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

#define PS_FORMAT   0
#define EPS_FORMAT  1
/* #define EPSI_FORMAT  2 */

typedef enum {
    COLORSPACE_GRAYSCALE,
    COLORSPACE_RGB,
    COLORSPACE_CMYK
} PSColorSpace;

typedef struct {
    int embed;
    char *name;
} PSFont;

#define DEFAULT_COLORSPACE  COLORSPACE_RGB

#define MEDIA_FEED_AUTO    0  
#define MEDIA_FEED_MATCH   1
#define MEDIA_FEED_MANUAL  2

#define DOCDATA_7BIT    0  
#define DOCDATA_8BIT    1  
#define DOCDATA_BINARY  2  

#define FONT_EMBED_NONE    0
#define FONT_EMBED_BUT13   1
#define FONT_EMBED_BUT35   2
#define FONT_EMBED_ALL     3

#define MAX_PS_LINELEN   70

int ps_initgraphics(const Canvas *canvas, void *data,
    const CanvasStats *cstats);

void ps_drawpixel(const Canvas *canvas, void *data, const VPoint *vp);
void ps_drawpolyline(const Canvas *canvas, void *data,
    const VPoint *vps, int n, int mode);
void ps_fillpolygon(const Canvas *canvas, void *data,
    const VPoint *vps, int nc);
void ps_drawarc(const Canvas *canvas, void *data,
    const VPoint *vp1, const VPoint *vp2, double a1, double a2);
void ps_fillarc(const Canvas *canvas, void *data,
    const VPoint *vp1, const VPoint *vp2, double a1, double a2, int mode);
void ps_putpixmap(const Canvas *canvas, void *data,
    const VPoint *vp, const CPixmap *pm);
void ps_puttext(const Canvas *canvas, void *data,
    const VPoint *vp, const char *s, int len, int font, const TextMatrix *tm,
    int underline, int overline, int kerning);

void ps_leavegraphics(const Canvas *canvas, void *data,
    const CanvasStats *cstats);

int ps_op_parser(const Canvas *canvas, void *data, const char *opstring);

#if defined(NONE_GUI)
#  define ps_gui_setup NULL
#else
void ps_gui_setup(const Canvas *canvas, void *data);
#endif
