/*
 * Grace - Graphics for Exploratory Data Analysis
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2004 Grace Development Team
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

/*
 *
 * Canvas events
 *
 */

#ifndef __EVENTS_H_
#define __EVENTS_H_

#include <config.h>

#include "graceapp.h"
#include "xprotos.h"

/* Event types */
#define MOUSE_MOVE    0
#define MOUSE_PRESS   1
#define MOUSE_RELEASE 2
#define KEY_PRESS     3
#define KEY_RELEASE   4

/* Mouse buttons */
#define NO_BUTTON         0x00
#define LEFT_BUTTON       0x01
#define RIGHT_BUTTON      0x02
#define MIDDLE_BUTTON     0x04
#define WHEEL_UP_BUTTON   0x08
#define WHEEL_DOWN_BUTTON 0x10

/* Keys */
#define KEY_NONE   0
#define KEY_ESCAPE 1
#define KEY_1      2
#define KEY_PLUS   3
#define KEY_MINUS  4
#define KEY_UP     5
#define KEY_DOWN   6
#define KEY_E      7

/* Modifiers */
#define NO_MODIFIER      0x00
#define SHIFT_MODIFIER   0x02
#define CONTROL_MODIFIER 0x04

typedef struct {
    int type;
    int x;
    int y;
    int button;
    int key;
    int modifiers;
    int time;
    void *udata;
} CanvasEvent;

/* add points at */
#define ADD_POINT_BEGINNING 0
#define ADD_POINT_END       1
#define ADD_POINT_NEAREST   2

/* move points */
#define MOVE_POINT_XY   0
#define MOVE_POINT_X    1
#define MOVE_POINT_Y    2

/*
 * double click detection interval (ms)
 */
#define CLICK_INT 400
#define CLICK_DIST 10

#define MAXPICKDIST 0.015      /* the maximum distance away from an object */
                               /* you may be when picking it (in viewport  */
                               /* coordinates)                             */  

/* selection type */
#define SELECTION_TYPE_NONE 0
#define SELECTION_TYPE_RECT 1
#define SELECTION_TYPE_VERT 2
#define SELECTION_TYPE_HORZ 3
/* #define SELECTION_TYPE_POLY 4 */

void canvas_event(CanvasEvent *event);
void set_action(GUI *gui, unsigned int npoints, int seltype,
    CanvasPointSink sink, void *data);
void track_point(Quark *pset, int *loc, int shift);
void update_locator_lab(Quark *cg, VPoint *vpp);
void do_select_area(void);
void do_select_peri(void);
void do_select_region(void);
Quark *next_graph_containing(Quark *cg, VPoint *vp);

void set_zoom_cb(Widget but, void *data);
void set_zoomx_cb(Widget but, void *data);
void set_zoomy_cb(Widget but, void *data);
void atext_add_proc(Widget but, void *data);

void update_point_locator(Quark *pset, int loc);
void get_tracking_props(int *setno, int *move_dir, int *add_at);

#endif /* __EVENTS_H_ */
