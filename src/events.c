/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1996-2003 Grace Development Team
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
 * event handler
 *
 */

#include <config.h>

#include <stdlib.h>

#include "utils.h"
#include "graphs.h"
#include "events.h"

#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/Intrinsic.h>
#include <X11/keysym.h>

#include "x11drv.h"
#include "motifinc.h"
#include "protos.h"

extern Widget loclab;

/*
 * set format string for locator
 */
static char *typestr[6] = {"X, Y",
                           "DX, DY",
			   "DIST",
			   "Phi, Rho",
			   "VX, VY",
                           "SX, SY"};


int cursortype = 0;


void canvas_event_proc(Widget w, XtPointer data, XEvent *event, Boolean *cont)
{
    int x, y;                /* pointer coordinates */
    VPoint vp;
    KeySym keybuf;
    Grace *grace = (Grace *) data;
    Quark *cg = graph_get_current(grace->project);
    
    XMotionEvent *xme;
    XButtonEvent *xbe;
    XKeyEvent    *xke;
    
    static Time lastc_time = 0;  /* time of last mouse click */
    static int lastc_x, lastc_y; /* coords of last mouse click */
    int dbl_click;

    int undo_point = FALSE;
    int abort_action = FALSE;
    int collect_points = FALSE;
    int npoints = 0, npoints_requested = 0;
    
    x = event->xmotion.x;
    y = event->xmotion.y;
    
    switch (event->type) {
    case MotionNotify:
	xme = (XMotionEvent *) event;
	if (cursortype || xme->state & ShiftMask) {
            crosshair_motion(x, y);
        }
	xlibdev2VPoint(x, y, &vp);

        if (grace->gui->focus_policy == FOCUS_FOLLOWS) {
            cg = next_graph_containing(cg, &vp);
        }
        
        update_locator_lab(cg, &vp);
        break;
    case ButtonPress:
	x = event->xbutton.x;
	y = event->xbutton.y;
	xlibdev2VPoint(x, y, &vp);

	switch (event->xbutton.button) {
	case Button1:
            xbe = (XButtonEvent *) event;
            /* first, determine if it's a double click */
            if (xbe->time - lastc_time < CLICK_INT &&
                abs(x - lastc_x) < CLICK_DIST      &&
                abs(y - lastc_y) < CLICK_DIST) {
                dbl_click = TRUE;
            } else {
                dbl_click = FALSE;
            }
            lastc_time = xbe->time;
            lastc_x = x;
            lastc_y = y;

            if (!dbl_click) {
                if (grace->gui->focus_policy == FOCUS_CLICK) {
                    cg = next_graph_containing(cg, &vp);
                }
                update_locator_lab(cg, &vp);
            } else {
                fprintf(stderr, "DblClick\n");
            }
            
            if (collect_points) {
                /* add_point(x, y) */
                npoints++;
            }
            
            break;
	case Button2:
            fprintf(stderr, "Button2\n");
            break;
	case Button3:
            undo_point = TRUE;
            if (npoints) {
                npoints--;
            }
            if (npoints == 0) {
                abort_action = TRUE;
            }
            break;
	case Button4:
            fprintf(stderr, "Button4\n");
            break;
	case Button5:
            fprintf(stderr, "Button5\n");
            break;
	default:
            break;
        }
        break;
    case KeyPress:
	xke = (XKeyEvent *) event;
        keybuf = XLookupKeysym(xke, 0);
        if (keybuf == XK_Escape) { /* Esc */
            fprintf(stderr, "Esc\n");
            abort_action = TRUE;
            return;
        }
        break;
    case KeyRelease:
	xke = (XKeyEvent *) event;
        keybuf = XLookupKeysym(xke, 0);
        if (cursortype == 0 &&
            (keybuf == XK_Shift_L || keybuf == XK_Shift_R)) { /* Shift */
            reset_crosshair(TRUE);
        }
        break;
    default:
	break;
    }
    
    if (abort_action) {
        /* abort action */
        /* xfree(points) */
        npoints = 0;
        /* return RETURN_FAILURE to caller */
    } else
    if (undo_point) {
        /* previous action */
    } else
    if (npoints == npoints_requested) {
        /* return points to caller */
    }
}

static int hook(Quark *q, void *udata, QTraverseClosure *closure)
{
    if (q->fid == QFlavorGraph) {
        Quark *pr = get_parent_project(q);
        VPoint *vp = (VPoint *) udata;
        view v;
        
        closure->descend = FALSE;
        
	if (is_graph_hidden(q)        == FALSE &&
            get_graph_viewport(q, &v) == RETURN_SUCCESS &&
            is_vpoint_inside(&v, vp, 0.0) == TRUE &&
            graph_get_current(pr) != q) {
            switch_current_graph(q);
            return FALSE;
        }
    } else
    if (q->fid == QFlavorFrame && !frame_is_active(q)) {
        closure->descend = FALSE;
    }

    return TRUE;
}

/*
 * Given the graph quark, find the (non-hidden) graph that contains
 * the VPoint.
 */
Quark *next_graph_containing(Quark *q, VPoint *vp)
{
    Quark *pr = get_parent_project(q);

    quark_traverse(pr, hook, vp);

    return graph_get_current(pr);
}

/*
 * locator on main_panel
 */
void update_locator_lab(Quark *cg, VPoint *vpp)
{
    static VPoint vp = {0.0, 0.0};
    view v;
    double wx, wy, xtmp, ytmp;
    int x, y;
    double dsx = 0.0, dsy = 0.0;
    char buf[256], bufx[64], bufy[64], *s;
    GLocator *locator;
    
    if (vpp != NULL) {
        vp = *vpp;
    }

    if (is_graph_hidden(cg)) {
        SetLabel(loclab, "[No graphs]");
        return;
    }
    
    get_graph_viewport(cg, &v);
    if (!is_vpoint_inside(&v, &vp, 0.0)) {
        SetLabel(loclab, "[Out of frame]");
        return;
    }
    
    view2world(vp.x, vp.y, &wx, &wy);
    
    locator = get_graph_locator(cg);
    
    if (locator->pointset) {
	dsx = locator->dsx;
	dsy = locator->dsy;
    }
    
    switch (locator->pt_type) {
    case 0:
        if (get_graph_type(cg) == GRAPH_POLAR) {
            polar2xy(wx, wy, &xtmp, &ytmp);
        } else {
            xtmp = wx;
            ytmp = wy;
        }
        break;
    case 1:
        xtmp = wx - dsx;
        ytmp = wy - dsy;
        break;
    case 2:
        if (get_graph_type(cg) == GRAPH_POLAR) {
            polar2xy(wx, wy, &xtmp, &ytmp);
        } else {
            xtmp = wx;
            ytmp = wy;
        }
        xtmp = hypot(dsx - xtmp, dsy - ytmp);
        ytmp = 0.0;
        break;
    case 3:
        if (dsx - wx != 0.0 || dsy - wy != 0.0) {
            xy2polar(dsx - wx, dsy - wy, &xtmp, &ytmp);
        } else {
            xtmp = 0.0;
            ytmp = 0.0;
        }
        break;
    case 4:
        xtmp = vp.x;
        ytmp = vp.y;
        break;
    case 5:
        xlibVPoint2dev(&vp, &x, &y);
        xtmp = x;
        ytmp = y;
        break;
    default:
        return;
    }
    s = create_fstring(get_parent_project(cg), locator->fx, locator->px, xtmp, LFORMAT_TYPE_PLAIN);
    strcpy(bufx, s);
    s = create_fstring(get_parent_project(cg), locator->fy, locator->py, ytmp, LFORMAT_TYPE_PLAIN);
    strcpy(bufy, s);
    sprintf(buf, "%s: %s = [%s, %s]", QIDSTR(cg), typestr[locator->pt_type], bufx, bufy);

    SetLabel(loclab, buf);
}

void switch_current_graph(Quark *gr)
{
    
    if (is_graph_hidden(gr) == FALSE) {
        Grace *grace = grace_from_quark(gr);
        Quark *cg = graph_get_current(grace->project);
        
        select_graph(gr);
        draw_focus(cg);
        draw_focus(gr);
        update_all();
        set_graph_selectors(gr);
        update_locator_lab(cg, NULL);
    }
}

/*
 * action callback
 */
void set_actioncb(Widget but, void *data)
{
}

/*
 * set the action_flag to the desired action (actions are
 * defined in defines.h), if 0 then cleanup the results
 * from previous actions.
 */
void set_action(CanvasAction act)
{
}

/* -------------------------------------------------------------- */
/* canvas_actions */
void autoscale_action(Widget w, XEvent *e, String *p, Cardinal *c)
{
}

void autoscale_on_near_action(Widget w, XEvent *e, String *p, Cardinal *c)
{
}

void draw_line_action(Widget w, XEvent *e, String *p, Cardinal *c)
{
}

void draw_box_action(Widget w, XEvent *e, String *p, Cardinal *c)
{
}

void draw_ellipse_action(Widget w, XEvent *e, String *p, Cardinal *c)
{
}

void write_string_action(Widget w, XEvent *e, String *p, Cardinal *c)
{
}


void delete_object_action(Widget w, XEvent *e, String *p, Cardinal *c)
{
}

void place_legend_action(Widget w, XEvent *e, String *p, Cardinal *c)
{
}

void move_object_action(Widget w, XEvent *e, String *p, Cardinal *c)
{
}

void refresh_hotlink_action(Widget w, XEvent *e, String *p, Cardinal *c)
{
    do_hotupdate_proc(NULL);
}

void set_viewport_action(Widget w, XEvent *e, String *p, Cardinal *c)
{
}

void enable_zoom_action(Widget w, XEvent *e, String *p, Cardinal *c)
{
}

/*
 * update the sets in the current graph
 */
void do_hotupdate_proc(void *data)
{
}

/*
 * switch on the area calculator
 */
void do_select_area(void)
{
}

/*
 * switch on the perimeter calculator
 */
void do_select_peri(void)
{
}

/*
 * define a (polygon) region
 */
void do_select_region(void)
{
}

