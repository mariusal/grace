/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2002 Grace Development Team
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
 *
 * Raster meta driver for Grace
 *
 */

#ifndef __XRSTDRV_H_
#define __XRSTDRV_H_

#include <xmi.h>
#define CANVAS_BACKEND_API
#include "grace/canvas.h"

typedef struct _Xrst_data {
    miGC *gc;
    miPaintedSet *paintedSet;
    miCanvas *mcanvas;

    int color;
    int bgcolor;
    int patno;
    int linewidth;
    int linestyle;
    int fillrule;
    int arcfillmode;
    int linecap;
    int linejoin;

    unsigned int height, width, page_scale;
    
    XrstDumpProc  dump;
    DevParserProc parser;
    DevSetupProc  setup;
    
    void *data;
} Xrst_data;


int xrst_initgraphics(const Canvas *canvas, void *data,
    const CanvasStats *cstats);

void xrst_drawpixel(const Canvas *canvas, void *data,
    const VPoint *vp);
void xrst_drawpolyline(const Canvas *canvas, void *data,
    const VPoint *vps, int n, int mode);
void xrst_fillpolygon(const Canvas *canvas, void *data,
    const VPoint *vps, int nc);
void xrst_drawarc(const Canvas *canvas, void *data,
    const VPoint *vp1, const VPoint *vp2, double a1, double a2);
void xrst_fillarc(const Canvas *canvas, void *data,
    const VPoint *vp1, const VPoint *vp2, double a1, double a2, int mode);
void xrst_putpixmap(const Canvas *canvas, void *data,
    const VPoint *vp, int width, int height, 
    char *databits, int pixmap_bpp, int bitmap_pad, int pixmap_type);
void xrst_puttext(const Canvas *canvas, void *data,
    const VPoint *vp, const char *s, int len, int font,
    const TextMatrix *tm, int underline, int overline, int kerning);

void xrst_leavegraphics(const Canvas *canvas, void *data,
    const CanvasStats *cstats);

#endif /* __XRSTDRV_H_ */
