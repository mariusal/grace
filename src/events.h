/*
 * Grace - Graphics for Exploratory Data Analysis
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2004 Grace Development Team
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
 * Canvas events
 *
 */

#ifndef __EVENTS_H_
#define __EVENTS_H_

#include <config.h>

#include "grace.h"
#include "xprotos.h"

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>


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

void canvas_event_proc(Widget w, XtPointer data, XEvent *event, Boolean *cont);
void anchor_point(int curx, int cury, VPoint curvp);
void set_action(GUI *gui, unsigned int npoints, int seltype,
    CanvasPointSink sink, void *data);
void track_point(Quark *pset, int *loc, int shift);
void update_locator_lab(Quark *cg, VPoint *vpp);
void set_stack_message(void);
void do_select_area(void);
void do_select_peri(void);
void do_dist_proc(void);
void do_select_region(void);
Quark *next_graph_containing(Quark *cg, VPoint *vp);
int graph_clicked(Quark *gr, VPoint vp);
int focus_clicked(Quark *cg, VPoint vp, VPoint *avp);
int legend_clicked(Quark *gr, VPoint vp, view *bb);
int axis_clicked(Quark *gr, VPoint vp, int *axisno);
int title_clicked(Quark *gr, VPoint vp);
int find_insert_location(Quark *pset, VPoint vp);
int find_point(Quark *gr, VPoint vp, Quark **pset, int *loc);
void newworld(Quark *gr, int axes, VPoint vp1, VPoint vp2);
void push_and_zoom(void);

void set_zoom_cb(Widget but, void *data);
void set_zoomx_cb(Widget but, void *data);
void set_zoomy_cb(Widget but, void *data);
void set_locator_cb(Widget but, void *data);

/* action routines */
void enable_zoom_action( Widget, XEvent *, String *, Cardinal * );
void autoscale_action( Widget, XEvent *, String *, Cardinal * );
void draw_line_action( Widget, XEvent *, String *, Cardinal * );
void draw_box_action( Widget, XEvent *, String *, Cardinal * );
void draw_ellipse_action( Widget, XEvent *, String *, Cardinal * );
void write_string_action( Widget, XEvent *, String *, Cardinal * );
void refresh_hotlink_action( Widget, XEvent *, String *, Cardinal * );

void update_point_locator(Quark *pset, int loc);
void get_tracking_props(int *setno, int *move_dir, int *add_at);

#endif /* __EVENTS_H_ */
