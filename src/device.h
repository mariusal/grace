/*
 * Grace - Graphics for Exploratory Data Analysis
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

/* 
 *
 *  declarations for devices
 *
 */
#ifndef __DEVICE_H_
#define __DEVICE_H_

#include "t1fonts.h"

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

/* Standard formats */
#define PAGE_FORMAT_CUSTOM    0
#define PAGE_FORMAT_USLETTER  1
#define PAGE_FORMAT_A4        2

#define FONTSRC_BITMAP  0
#define FONTSRC_DEVICE  1

typedef struct {
    unsigned long width;
    unsigned long height;
    float dpi_x;
    float dpi_y;
} Page_geometry;

typedef struct {
    int type;
    char *name;		    /* name of device */
    int (*init)(void);	    /* function to initialize device */
    int (*parser)(char *);  /* function to parse device-specific commands */
    void (*setup)(void);    /* function (GUI interface) to setup device */
    char *fext;		    /* filename extension */
    int devfonts;           /* Bitmap fonts or provided by device */
    int fontaa;             /* Font antialiasing */
    Page_geometry pg;       /* device defaults */
} Device_entry;

/* device exit */
extern void (*devleavegraphics) (void);

/* device pixel routine */
extern void (*devdrawpixel) (VPoint vp);  
/* device polyline routine */
extern void (*devdrawpolyline) (VPoint *vps, int n, int mode);  
/* device polygon routine */
extern void (*devfillpolygon) (VPoint *vps, int nc);   
/* device arc routine */
extern void (*devdrawarc) (VPoint vp1, VPoint vp2, int a1, int a2);	
/* device fill arc routine */
extern void (*devfillarc) (VPoint vp1, VPoint vp2, int a1, int a2);	
/* device pixmap drawing */
extern void (*devputpixmap) (VPoint vp, int width, int height, char *databits,
                               int pixmap_bpp, int bitmap_pad, int pixmap_type);
extern void (*devputtext) (VPoint start, VPoint end, double size, 
                                            CompositeString *cstring);

/* update color map */
extern void (*devupdatecmap)(void);	


int register_device(Device_entry device);
int select_device(int dindex);
int initgraphics (void);

Device_entry get_device_props(int device);
Device_entry get_curdevice_props(void);

void set_device_props(int device, Device_entry dev);
void set_curdevice_props(Device_entry dev);

int set_page_geometry(Page_geometry pg);
Page_geometry get_page_geometry(void);

int get_device_by_name(char *dname);

int parse_device_options(int dindex, char *options);

void set_printer(int device);
int set_printer_by_name(char *dname);

int number_of_devices(void);

void get_page_viewport(double *vx, double *vy);

int terminal_device(void);

/* some useful macros */
#define page_dpi_x     ((get_page_geometry()).dpi_x)
#define page_dpi_y     ((get_page_geometry()).dpi_y)

#define page_width     ((get_page_geometry()).width)
#define page_height    ((get_page_geometry()).height)

#define page_width_in  ((double) page_width/page_dpi_x)
#define page_height_in ((double) page_height/page_dpi_y)

#define page_width_mm  (MM_PER_INCH*page_width_in)
#define page_height_mm (MM_PER_INCH*page_height_in)

#define page_width_cm  (CM_PER_INCH*page_width_in)
#define page_height_cm (CM_PER_INCH*page_height_in)

#endif /* __DEVICE_H_ */
