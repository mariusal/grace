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

/*
 * Dummy (Null) driver for Grace
 */

#include <stdio.h>
#include <stdlib.h>
 
#define CANVAS_BACKEND_API
#include "grace/canvas.h"
#include "devlist.h"
#include "dummydrv.h"

int register_dummy_drv(Canvas *canvas)
{
    Device_entry *d;

    d = device_new("Dummy", DEVICE_TERM, FALSE, NULL, NULL);
    if (!d) {
        return -1;
    }
    
    device_set_procs(d,
        dummy_initgraphics,
        dummy_leavegraphics,
        NULL,
        NULL,
        NULL,
        dummy_drawpixel,
        dummy_drawpolyline,
        dummy_fillpolygon,
        dummy_drawarc,
        dummy_fillarc,
        dummy_putpixmap,
        dummy_puttext);
    
    return register_device(canvas, d);
}

int dummy_initgraphics(const Canvas *canvas, void *data, const CanvasStats *cstats)
{
    return RETURN_SUCCESS;
}

void dummy_drawpixel(const Canvas *canvas, void *data, const VPoint *vp){}
void dummy_drawpolyline(const Canvas *canvas, void *data,
    const VPoint *vps, int n, int mode){}
void dummy_fillpolygon(const Canvas *canvas, void *data,
    const VPoint *vps, int nc){}
void dummy_drawarc(const Canvas *canvas, void *data,
    const VPoint *vp1, const VPoint *vp2, double a1, double a2){}
void dummy_fillarc(const Canvas *canvas, void *data,
    const VPoint *vp1, const VPoint *vp2, double a1, double a2, int mode){}
void dummy_putpixmap(const Canvas *canvas, void *data,
    const VPoint *vp, int width, int height, 
    char *databits, int pixmap_bpp, int bitmap_pad, int pixmap_type){}
void dummy_puttext(const Canvas *canvas, void *data,
    const VPoint *vp, const char *s, int len, int font,
    const TextMatrix *tm, int underline, int overline, int kerning){}

void dummy_leavegraphics(const Canvas *canvas, void *data,
    const CanvasStats *cstats){}
