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

/* 
 *
 *  declarations for devices
 *
 */
#ifndef __DEVICE_H_
#define __DEVICE_H_

#include "grace.h"
#include "draw.h"

/* default dimensions of the canvas */
#define DEFAULT_PAGE_WIDTH  600
#define DEFAULT_PAGE_HEIGHT 600

#define MM_PER_INCH	25.4
#define CM_PER_INCH	(MM_PER_INCH/10)

/* hardcopy or terminal device */
/* device output can be redirected to file/printer(both) */
#define DEVICE_TERM	0
#define DEVICE_FILE	1
#define DEVICE_PRINT	2

/* Page orientation */
#define PAGE_ORIENT_LANDSCAPE  0
#define PAGE_ORIENT_PORTRAIT   1

int register_device(Canvas *canvas, Device_entry *device);
int select_device(Canvas *canvas, int dindex);
int initgraphics(Canvas *canvas);

Device_entry *get_device_props(const Canvas *canvas, int device);
Device_entry *get_curdevice_props(const Canvas *canvas);

char *get_device_name(const Canvas *canvas, int device);

void *get_curdevice_data(const Canvas *canvas);

int is_valid_page_geometry(const Page_geometry *pg);
int set_page_geometry(Canvas *canvas, const Page_geometry *pg);
Page_geometry *get_page_geometry(const Canvas *canvas);

int set_page_dimensions(Grace *grace, int wpp, int hpp, int rescale);
int set_printer(Grace *grace, int device);
int set_printer_by_name(Grace *grace, const char *dname);
void set_ptofile(Grace *grace, int flag);
int get_ptofile(const Grace *grace);

int get_device_page_dimensions(const Canvas *canvas,
    int dindex, int *wpp, int *hpp);

int get_device_by_name(const Canvas *canvas, const char *dname);

int parse_device_options(Canvas *canvas, int dindex, char *options);

int number_of_devices(const Canvas *canvas);

void get_page_viewport(const Canvas *canvas, double *vx, double *vy);

int terminal_device(const Canvas *canvas);

PageFormat get_page_format(const Canvas *canvas, int device);

/* some useful macros */
#define page_dpi(canvas)       ((get_page_geometry(canvas))->dpi)

#define page_width(canvas)     ((get_page_geometry(canvas))->width)
#define page_height(canvas)    ((get_page_geometry(canvas))->height)

#define page_width_in(canvas)  ((double) page_width(canvas)/page_dpi(canvas))
#define page_height_in(canvas) ((double) page_height(canvas)/page_dpi(canvas))

#define page_width_mm(canvas)  (MM_PER_INCH*page_width_in(canvas))
#define page_height_mm(canvas) (MM_PER_INCH*page_height_in(canvas))

#define page_width_cm(canvas)  (CM_PER_INCH*page_width_in(canvas))
#define page_height_cm(canvas) (CM_PER_INCH*page_height_in(canvas))

#define page_width_pp(canvas)  (72*page_width_in(canvas))
#define page_height_pp(canvas) (72*page_height_in(canvas))

#endif /* __DEVICE_H_ */
