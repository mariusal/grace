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

#include <config.h>

#include <stdlib.h>
#include <string.h>

#include "grace/baseP.h"
#include "grace/canvasP.h"

Device_entry *device_new(const char *name, int type, int twopass,
    void *data, DevFreeDataProc freedata)
{
    Device_entry *d;
    
    d = xmalloc(sizeof(Device_entry));
    if (d) {
        memset(d, 0, sizeof(Device_entry));
        d->pg.dpi      = 72.0;
        d->color_trans = COLOR_TRANS_NONE;
        
        d->type     = type;
        d->twopass  = twopass;
        d->name     = copy_string(NULL, name);
        d->data     = data;
        d->freedata = freedata;
    }
    
    return d;
}

void device_free(Device_entry *d)
{
    if (d) {
        xfree(d->name);
        xfree(d->fext);
        if (d->freedata) {
            d->freedata(d->data);
        }
        xfree(d);
    }
}

int device_set_procs(Device_entry *d,
    DevInitProc          initgraphics,
    DevLeaveGraphicsProc leavegraphics,
    DevParserProc        parser,
    DevSetupProc         setup,
    DevUpdateCmapProc    updatecmap,
    DevDrawPixelProc     drawpixel,
    DevDrawPolyLineProc  drawpolyline,
    DevFillPolygonProc   fillpolygon,
    DevDrawArcProc       drawarc,
    DevFillArcProc       fillarc,
    DevPutPixmapProc     putpixmap,
    DevPutTextProc       puttext)
{
    d->initgraphics  = initgraphics;
    d->leavegraphics = leavegraphics;
    d->parser        = parser;
    d->setup         = setup;
    d->updatecmap    = updatecmap;
    d->drawpixel     = drawpixel;
    d->drawpolyline  = drawpolyline;
    d->fillpolygon   = fillpolygon;
    d->drawarc       = drawarc;
    d->fillarc       = fillarc;
    d->putpixmap     = putpixmap;
    d->puttext       = puttext;
    
    if (d->puttext) {
        d->fontrast = FONT_RASTER_DEVICE;
    } else {
        d->fontrast = FONT_RASTER_AA_LOW;
    }
    
    return RETURN_SUCCESS;
}

int device_set_dpi(Device_entry *d, float dpi, int resize)
{
    Page_geometry *pg = &d->pg;
    
    if (dpi <= 0.0) {
        return RETURN_FAILURE;
    }
    
    if (resize && pg->dpi) {
        float rf = dpi/pg->dpi;
        pg->width  *= rf;
        pg->height *= rf;
    }
    
    pg->dpi = dpi;
    
    return RETURN_SUCCESS;
}

int device_set_fext(Device_entry *d, const char *fext)
{
    d->fext = copy_string(d->fext, fext);
    
    return RETURN_SUCCESS;
}

int device_set_autocrop(Device_entry *d, int autocrop)
{
    d->autocrop = autocrop;
    
    return RETURN_SUCCESS;
}

int device_set_fontrast(Device_entry *d, FontRaster fontrast)
{
    if (!d->puttext && fontrast == FONT_RASTER_DEVICE) {
        return RETURN_FAILURE;
    } else {
        d->fontrast = fontrast;
        return RETURN_SUCCESS;
    }
}

int is_valid_page_geometry(const Page_geometry *pg)
{
    if (pg->width  > 0 &&
	pg->height > 0 &&
        pg->dpi > 0.0) {
	return TRUE;
    } else {
        return FALSE;
    }
}

int set_page_geometry(Canvas *canvas, const Page_geometry *pg)
{
    if (is_valid_page_geometry(pg) == TRUE) {
        canvas->curdevice->pg = *pg;
	return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

Page_geometry *get_page_geometry(const Canvas *canvas)
{
    if (canvas && canvas->curdevice) {
        return &canvas->curdevice->pg;
    } else {
        return NULL;
    }
}

int get_device_page_dimensions(const Canvas *canvas,
    unsigned int dindex, int *wpp, int *hpp)
{
    if (dindex >= canvas->ndevices) {
        return RETURN_FAILURE;
    } else {
	*wpp = canvas->device_table[dindex]->pg.width*72/canvas->device_table[dindex]->pg.dpi;
	*hpp = canvas->device_table[dindex]->pg.height*72/canvas->device_table[dindex]->pg.dpi;
        return RETURN_SUCCESS;
    }
}

int register_device(Canvas *canvas, Device_entry *d)
{
    int dindex;
    
    canvas->ndevices++;
    dindex = canvas->ndevices - 1;
    canvas->device_table = xrealloc(canvas->device_table,
        canvas->ndevices*SIZEOF_VOID_P);

    canvas->device_table[dindex] = d;
    
    return dindex;
}

int select_device(Canvas *canvas, unsigned int dindex)
{
    if (dindex >= canvas->ndevices) {
        return RETURN_FAILURE;
    } else {
        canvas->curdevice = canvas->device_table[dindex];
	return RETURN_SUCCESS;
    }
}

int get_device_by_name(const Canvas *canvas, const char *dname)
{
    unsigned int i;
    
    i = 0;
    while (i < canvas->ndevices) {
        if (strncmp(canvas->device_table[i]->name, dname, strlen(dname)) == 0) {
            break;
        } else {
            i++;
        }
    }
    if (i >= canvas->ndevices) {
        return -1;
    } else {
	return i;
    }
}

Device_entry *get_device_props(const Canvas *canvas, int device)
{
    return canvas->device_table[device];
}

Device_entry *get_curdevice_props(const Canvas *canvas)
{
    return canvas->curdevice;
}

char *get_device_name(const Canvas *canvas, int device)
{
    return canvas->device_table[device]->name;
}

int parse_device_options(Canvas *canvas, unsigned int dindex, char *options)
{
    char *p, *oldp, opstring[64];
    int n;
        
    if (dindex >= canvas->ndevices || 
        canvas->device_table[dindex]->parser == NULL) {
        return RETURN_FAILURE;
    } else {
        Device_entry *dev = canvas->device_table[dindex];
        oldp = options;
        while ((p = strchr(oldp, ',')) != NULL) {
	    n = MIN2((p - oldp), 64 - 1);
            strncpy(opstring, oldp, n);
            opstring[n] = '\0';
            if (dev->parser(canvas, dev->data, opstring) !=
                RETURN_SUCCESS) {
                return RETURN_FAILURE;
            }
            oldp = p + 1;
        }
        return dev->parser(canvas, dev->data, oldp);
    }
}

int number_of_devices(const Canvas *canvas)
{
    return (canvas->ndevices);
}

void get_page_viewport(const Canvas *canvas, double *vx, double *vy)
{
    *vx = canvas->curdevice->pg.width/canvas->curdevice->pg.dpi;
    *vy = canvas->curdevice->pg.height/canvas->curdevice->pg.dpi;
    if (*vx < *vy) {
        *vy /= *vx;
        *vx = 1.0;
    } else {
        *vx /= *vy;
        *vy = 1.0;
    }
}

int terminal_device(const Canvas *canvas)
{
    if (canvas->curdevice->type == DEVICE_TERM) {
        return TRUE;
    } else {
        return FALSE;
    }
}

PageFormat get_page_format(const Canvas *canvas, int device)
{
    Page_geometry pg;
    int width_pp, height_pp;
    
    pg = canvas->device_table[device]->pg;
    width_pp  = (int) rint((double) 72*pg.width/pg.dpi);
    height_pp = (int) rint((double) 72*pg.height/pg.dpi);
    
    if ((width_pp == 612 && height_pp == 792) ||
        (height_pp == 612 && width_pp == 792)) {
        return PAGE_FORMAT_USLETTER;
    } else if ((width_pp == 595 && height_pp == 842) ||
               (height_pp == 595 && width_pp == 842)) {
        return PAGE_FORMAT_A4;
    } else {
        return PAGE_FORMAT_CUSTOM;
    }
}
