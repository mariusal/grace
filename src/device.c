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

#include <config.h>

#include <stdlib.h>
#include <string.h>

#include "cmath.h"
#include "defines.h"
#include "graphutils.h"
#include "utils.h"
#include "draw.h"

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
    return &canvas->curdevice->pg;
}

int set_page_dimensions(Grace *grace, int wpp, int hpp, int rescale)
{
    int i;
    Canvas *canvas = grace->rt->canvas;
    
    if (wpp <= 0 || hpp <= 0) {
        return RETURN_FAILURE;
    } else {
        int wpp_old, hpp_old;
	wpp_old = grace->project->page_wpp;
	hpp_old = grace->project->page_hpp;
        
        grace->project->page_wpp = wpp;
	grace->project->page_hpp = hpp;
        if (rescale) {
            if (hpp*wpp_old - wpp*hpp_old != 0) {
                /* aspect ratio changed */
                double ext_x, ext_y;
                double old_aspectr, new_aspectr;
                
                old_aspectr = (double) wpp_old/hpp_old;
                new_aspectr = (double) wpp/hpp;
                if (old_aspectr >= 1.0 && new_aspectr >= 1.0) {
                    ext_x = new_aspectr/old_aspectr;
                    ext_y = 1.0;
                } else if (old_aspectr <= 1.0 && new_aspectr <= 1.0) {
                    ext_x = 1.0;
                    ext_y = old_aspectr/new_aspectr;
                } else if (old_aspectr >= 1.0 && new_aspectr <= 1.0) {
                    ext_x = 1.0/old_aspectr;
                    ext_y = 1.0/new_aspectr;
                } else {
                    ext_x = new_aspectr;
                    ext_y = old_aspectr;
                }

                rescale_viewport(grace->project, ext_x, ext_y);
            } 
        }
        for (i = 0; i < canvas->ndevices; i++) {
            canvas->device_table[i]->pg.width =
                (unsigned long) (wpp*(canvas->device_table[i]->pg.dpi/72));
            canvas->device_table[i]->pg.height =
                (unsigned long) (hpp*(canvas->device_table[i]->pg.dpi/72));
        }
        return RETURN_SUCCESS;
    }
}

int get_device_page_dimensions(const Canvas *canvas,
    int dindex, int *wpp, int *hpp)
{
    if (dindex >= canvas->ndevices || dindex < 0) {
        return RETURN_FAILURE;
    } else {
	*wpp = canvas->device_table[dindex]->pg.width*72/canvas->device_table[dindex]->pg.dpi;
	*hpp = canvas->device_table[dindex]->pg.height*72/canvas->device_table[dindex]->pg.dpi;
        return RETURN_SUCCESS;
    }
}

int register_device(Canvas *canvas, Device_entry *device)
{
    int dindex;
    
    canvas->ndevices++;
    dindex = canvas->ndevices - 1;
    canvas->device_table = xrealloc(canvas->device_table,
        canvas->ndevices*SIZEOF_VOID_P);

    canvas->device_table[dindex] = device;
    
    return dindex;
}

int select_device(Canvas *canvas, int dindex)
{
    if (dindex >= canvas->ndevices || dindex < 0) {
        return RETURN_FAILURE;
    } else {
        canvas->curdevice = canvas->device_table[dindex];
	return RETURN_SUCCESS;
    }
}

/*
 * set the current print device
 */
int set_printer(Grace *grace, int device)
{
    Canvas *canvas = grace->rt->canvas;
    if (device >= canvas->ndevices || device < 0 ||
        canvas->device_table[device]->type == DEVICE_TERM) {
        return RETURN_FAILURE;
    } else {
        grace->rt->hdevice = device;
	if (canvas->device_table[device]->type != DEVICE_PRINT) {
            set_ptofile(grace, TRUE);
        }
        return RETURN_SUCCESS;
    }
}

int set_printer_by_name(Grace *grace, const char *dname)
{
    int device;
    
    device = get_device_by_name(grace->rt->canvas, dname);
    
    return set_printer(grace, device);
}

int get_device_by_name(const Canvas *canvas, const char *dname)
{
    int i;
    
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

void *get_curdevice_data(const Canvas *canvas)
{
    return canvas->curdevice->data;
}

int parse_device_options(Canvas *canvas, int dindex, char *options)
{
    char *p, *oldp, opstring[64];
    int n;
        
    if (dindex >= canvas->ndevices || dindex < 0 || 
            canvas->device_table[dindex]->parser == NULL) {
        return RETURN_FAILURE;
    } else {
        oldp = options;
        while ((p = strchr(oldp, ',')) != NULL) {
	    n = MIN2((p - oldp), 64 - 1);
            strncpy(opstring, oldp, n);
            opstring[n] = '\0';
            if (canvas->device_table[dindex]->parser(canvas, opstring) !=
                RETURN_SUCCESS) {
                return RETURN_FAILURE;
            }
            oldp = p + 1;
        }
        return canvas->device_table[dindex]->parser(canvas, oldp);
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

/*
 * flag to indicate destination of hardcopy output,
 * ptofile = 0 means print to printer, otherwise print to file
 */

void set_ptofile(Grace *grace, int flag)
{
    grace->rt->ptofile = flag;
}

int get_ptofile(const Grace *grace)
{
    return grace->rt->ptofile;
}
