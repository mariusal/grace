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
 * event handler
 *
 */

#include <config.h>
#include <cmath.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#if defined(HAVE_SYS_PARAM_H)
#  include <sys/param.h>
#endif	
#include <time.h>

#include "globals.h"
#include "events.h"
#include "utils.h"
#include "graphs.h"
#include "draw.h"
#include "graphutils.h"
#include "t1fonts.h"
#include "x11drv.h"
#include "plotone.h"
#include "protos.h"

#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/Intrinsic.h>
#include <X11/keysym.h>

#include "motifinc.h"

#if defined(MAXHOSTNAMELEN)
#  define GR_MAXHOSTNAMELEN MAXHOSTNAMELEN
#else
#  define GR_MAXHOSTNAMELEN 64
#endif


extern Widget loclab;
extern Widget arealab;
extern Widget perimlab;
extern Widget locate_point_item;
extern Widget stack_depth_item;
extern Widget curw_item;

int cursortype = 0;

static VPoint anchor_vp = {0.0, 0.0};
static int x, y;               /* pointer coordinates */
static int anchor_x = 0;
static int anchor_y = 0;
static view bb;

int go_locateflag = TRUE;	/* locator */

int add_setno;			/* set to add points - set in ptswin.c */
int add_at;			/* where to begin inserting points in the set */
int move_dir;			/* restriction on point movement */

extern char locator_format[];

static char buf[256];

static int action_flag = 0;

extern int regiontype;
extern int regionlinkto;

/*
 * for region, area and perimeter computation
 */
#define MAX_POLY_POINTS 200
static int region_pts = 0;
static int iax[MAX_POLY_POINTS];
static int iay[MAX_POLY_POINTS];
static WPoint region_wps[MAX_POLY_POINTS];


void anchor_point(int curx, int cury, VPoint curvp)
{
     anchor_vp = curvp;
     anchor_x = curx;
     anchor_y = cury;
}

void my_proc(Widget parent, XtPointer data, XEvent *event)
{
    int i;
    char keybuf;
    KeySym keys;
    XComposeStatus compose;
    static Time lastc = 0;  /* time of last mouse click */
    Boolean dbl_click;

    WPoint wp, wp_new;
    VPoint vp;
    VVector shift;
    view v;
    int cg, newg, setno, loc;
    static int track_gno = -1;
    static int track_setno;
    static int track_loc;
    static int type, id;   /* for objects */
    int axisno;
    char buf[100];
    Datapoint dpoint;
    GLocator locator;
    
    cg = get_cg();
    
    switch (event->type) {
    case MotionNotify:
	x = event->xmotion.x;
	y = event->xmotion.y;
	if (cursortype != 0) {
            crosshair_motion(x, y);
        }
	vp = xlibdev2VPoint(x, y);
        getpoints(vp);

        switch (action_flag) {
        case 0:
            if (focus_policy == FOCUS_FOLLOWS) {
                if ((newg = next_graph_containing(-1, vp)) != cg) {
                    switch_current_graph(newg);
                }
            }
            break;
        case VIEW_2ND:
        case ZOOM_2ND:
        case ZOOMX_2ND:
        case ZOOMY_2ND:
        case MAKE_BOX_2ND:
        case MAKE_ELLIP_2ND:
	    select_region(x, y, anchor_x, anchor_y, 1);
            break;
        case MOVE_OBJECT_2ND:
        case COPY_OBJECT2ND:
        case PLACE_LEGEND_2ND:
        case PLACE_TIMESTAMP_2ND:
            slide_region(bb, x - anchor_x, y - anchor_y, 1);
            break;
        case MAKE_LINE_2ND:
        case DEF_REGION2ND:
	    select_line(x, y, anchor_x, anchor_y, 1);
            break;
        case DEF_REGION:
	    if (region_pts > 0) {
                select_line(x, y, anchor_x, anchor_y, 1);
            }
            break;
        case MOVE_POINT2ND:
	    switch (move_dir) {
	    case 0:
	        select_line(x, y, anchor_x, anchor_y, 1);
                break;
	    case 1:
	        select_line(x, anchor_y, anchor_x, anchor_y, 1);
	        break;
	    case 2:
	        select_line(anchor_x, y, anchor_x, anchor_y, 1);
	        break;
	    }
            break;
        default:
            return;
            break;
        }

        break;
    case ButtonPress:
	x = event->xbutton.x;
	y = event->xbutton.y;
	vp = xlibdev2VPoint(x, y);
	getpoints(vp);
	switch (event->xbutton.button) {
	case Button1:
            /* first, determine if it's double click */
            if (((XButtonEvent *) event)->time - lastc < CLICKINT) {
                dbl_click = True;
            } else {
                dbl_click = False;
            }
            lastc = ((XButtonEvent *) event)->time;
            
            switch (action_flag) {
            case 0:
                if (dbl_click == True && allow_dc == TRUE) {
                    if (focus_clicked(cg, vp, &anchor_vp) == TRUE) {
                        xlibVPoint2dev(anchor_vp, &anchor_x, &anchor_y);
                        set_action(VIEW_2ND);
	                select_region(x, y, anchor_x, anchor_y, 0);
                    } else if (find_point(cg, vp, &setno, &loc) == GRACE_EXIT_SUCCESS) {
                        define_symbols_popup(parent, (XtPointer) setno, NULL);
                    } else if (axis_clicked(cg, vp, &axisno) == TRUE) {
                        create_axes_dialog(axisno);
                    } else if (title_clicked(cg, vp) == TRUE) {
                        create_graphapp_frame(cg);
                    } else if (legend_clicked(cg, vp, &bb) == TRUE) {
                        create_graphapp_frame(cg);
                    } else if (find_item(cg, vp, &bb, &type, &id) == GRACE_EXIT_SUCCESS) {
                        object_edit_popup(type, id);
                    } else if (timestamp_clicked(vp, &bb) == TRUE) {
                        create_plot_frame();
                    }
                } else {
                    if (focus_policy == FOCUS_CLICK) {
                        if ((newg = next_graph_containing(cg, vp)) != cg) {
                            switch_current_graph(newg);
                        }
                    }
                }
                break;
            case VIEW_2ND:
                set_action(DO_NOTHING);
		v.xv1 = MIN2(vp.x, anchor_vp.x);
		v.yv1 = MIN2(vp.y, anchor_vp.y);
		v.xv2 = MAX2(vp.x, anchor_vp.x);
		v.yv2 = MAX2(vp.y, anchor_vp.y);
                set_graph_viewport(cg, v);
		xdrawgraph();
                break;
            case ZOOM_1ST:
                anchor_point(x, y, vp);
                set_action(ZOOM_2ND);
	        select_region(x, y, anchor_x, anchor_y, 0);
                break;
            case ZOOMX_1ST:
                anchor_point(x, y, vp);
                set_action(ZOOMX_2ND);
	        select_region(x, y, anchor_x, anchor_y, 0);
                break;
            case ZOOMY_1ST:
                anchor_point(x, y, vp);
                set_action(ZOOMY_2ND);
	        select_region(x, y, anchor_x, anchor_y, 0);
                break;
            case VIEW_1ST:
                anchor_point(x, y, vp);
                set_action(VIEW_2ND);
	        select_region(x, y, anchor_x, anchor_y, 0);
                break;
            case ZOOM_2ND:
                set_action(DO_NOTHING);
		newworld(cg, ALL_AXES, vp, anchor_vp);
                break;
            case ZOOMX_2ND:
                set_action(DO_NOTHING);
		newworld(cg, ALL_X_AXES, vp, anchor_vp);
                break;
            case ZOOMY_2ND:
                set_action(DO_NOTHING);
		newworld(cg, ALL_Y_AXES, vp, anchor_vp);
                break;
            case EDIT_OBJECT:
                if (find_item(cg, vp, &bb, &type, &id) == GRACE_EXIT_SUCCESS) {
                    object_edit_popup(type, id);
                }
                break;
            case DEL_OBJECT:
                if (find_item(cg, vp, &bb, &type, &id) == GRACE_EXIT_SUCCESS) {
                    if (yesno("Kill the object?", NULL, NULL, NULL) == TRUE) {
                        kill_object(type, id);
                        xdrawgraph();
                    }
                }
                break;
            case MOVE_OBJECT_1ST:
                if (find_item(cg, vp, &bb, &type, &id) == GRACE_EXIT_SUCCESS) {
                    anchor_point(x, y, vp);
	            slide_region(bb, x - anchor_x, y - anchor_y, 0);
                    set_action(MOVE_OBJECT_2ND);
                }
                break;
            case MOVE_OBJECT_2ND:
                shift.x = vp.x - anchor_vp.x;
                shift.y = vp.y - anchor_vp.y;
                move_object(type, id, shift);
                xdrawgraph();
                set_action(MOVE_OBJECT_1ST);
                break;
            case COPY_OBJECT1ST:
                if (find_item(cg, vp, &bb, &type, &id) == GRACE_EXIT_SUCCESS) {
                    anchor_point(x, y, vp);
	            slide_region(bb, x - anchor_x, y - anchor_y, 0);
                    set_action(COPY_OBJECT2ND);
                }
                break;
            case COPY_OBJECT2ND:
                shift.x = vp.x - anchor_vp.x;
                shift.y = vp.y - anchor_vp.y;
                id = duplicate_object(type, id);
                move_object(type, id, shift);
                xdrawgraph();
                set_action(COPY_OBJECT1ST);
                break;
            case STR_LOC:
                id = next_string();
                init_string(id, vp);
                object_edit_popup(OBJECT_STRING, id);
                break;
            case MAKE_LINE_1ST:
                anchor_point(x, y, vp);
	        select_line(x, y, anchor_x, anchor_y, 0);
                set_action(MAKE_LINE_2ND);
                break;
            case MAKE_LINE_2ND:
	        select_line(x, y, anchor_x, anchor_y, 0);
                id = next_line();
                init_line(id, vp, anchor_vp);
                xdrawgraph();
                set_action(MAKE_LINE_1ST);
                break;
            case MAKE_BOX_1ST:
                anchor_point(x, y, vp);
	        select_region(x, y, anchor_x, anchor_y, 0);
                set_action(MAKE_BOX_2ND);
                break;
            case MAKE_BOX_2ND:
	        select_region(x, y, anchor_x, anchor_y, 0);
                id = next_box();
                init_box(id, vp, anchor_vp);
                xdrawgraph();
                set_action(MAKE_BOX_1ST);
                break;
            case MAKE_ELLIP_1ST:
                anchor_point(x, y, vp);
	        select_region(x, y, anchor_x, anchor_y, 0);
                set_action(MAKE_ELLIP_2ND);
                break;
            case MAKE_ELLIP_2ND:
	        select_region(x, y, anchor_x, anchor_y, 0);
                id = next_ellipse();
                init_ellipse(id, vp, anchor_vp);
                xdrawgraph();
                set_action(MAKE_ELLIP_1ST);
                break;
            case AUTO_NEAREST:
                if (find_point(cg, vp, &setno, &loc) == GRACE_EXIT_SUCCESS) {
                    autoscale_byset(cg, setno, AUTOSCALE_XY);
                    update_ticks(cg);
                    xdrawgraph();
                    set_action(DO_NOTHING);
                }
                break;
            case TRACKER:
                if (track_gno == cg) {
                    track_point(cg, track_setno, &track_loc, -1);
                }
                break;
            case DEL_POINT:
                if (find_point(cg, vp, &setno, &loc) == GRACE_EXIT_SUCCESS) {
		    del_point(cg, setno, loc);
		    update_set_lists(cg);
                    xdrawgraph();
                }
                break;
            case MOVE_POINT1ST:
                if (find_point(cg, vp, &track_setno, &track_loc) == GRACE_EXIT_SUCCESS) {
                    anchor_point(x, y, vp);
                    get_point(cg, track_setno, track_loc, &wp);
                    sprintf(buf, "G%d.S%d, loc %d, (%f, %f)",
                        cg, track_setno, track_loc, wp.x, wp.y);
		    xv_setstr(locate_point_item, buf);
	            select_line(x, y, anchor_x, anchor_y, 0);
		    set_action(MOVE_POINT2ND);
                }
                break;
            case MOVE_POINT2ND:
                if (is_valid_setno(cg, track_setno)) {
                    get_point(cg, track_setno, track_loc, &wp);
                    view2world(vp.x, vp.y, &wp_new.x, &wp_new.y);

		    switch (move_dir) {
		    case 0:
		        wp = wp_new;
                        break;
		    case 1:
		        wp.x = wp_new.x;
		        break;
		    case 2:
		        wp.y = wp_new.y;
		        break;
		    }

                    set_point(cg, track_setno, track_loc, wp);
                    
                    sprintf(buf, "G%d.S%d, loc %d, (%f, %f)",
                        cg, track_setno, track_loc, wp.x, wp.y);
		    xv_setstr(locate_point_item, buf);
		    update_set_lists(cg);
                    xdrawgraph();
		    set_action(MOVE_POINT1ST);
                }
                break;
            case ADD_POINT:
		view2world(vp.x, vp.y, &wp.x, &wp.y);
                zero_datapoint(&dpoint);
                dpoint.ex[0] = wp.x;
                dpoint.ex[1] = wp.y;
                switch (add_at) {
		case 0: /* at the end */
		    loc = getsetlength(cg, add_setno);
		    break;
		case 1: /* at the beginning */
		    loc = 0;
		    break;
		case 2: /* between nearest points */
		    loc = find_insert_location(cg, add_setno, vp);
		    break;
		}
		add_point_at(cg, add_setno, loc, &dpoint);
		sprintf(buf, "Set %d, loc %d, (%f, %f)", add_setno, loc, wp.x, wp.y);
		update_set_lists(cg);
                xdrawgraph();
                break;
            case PLACE_LEGEND_1ST:
                if (legend_clicked(cg, vp, &bb) == TRUE) {
                    anchor_point(x, y, vp);
	            slide_region(bb, x - anchor_x, y - anchor_y, 0);
                    set_action(PLACE_LEGEND_2ND);
                }
                break;
            case PLACE_LEGEND_2ND:
                shift.x = vp.x - anchor_vp.x;
                shift.y = vp.y - anchor_vp.y;
                move_legend(cg, shift);
                xdrawgraph();
                set_action(PLACE_LEGEND_1ST);
                break;
            case PLACE_TIMESTAMP_1ST:
                if (timestamp_clicked(vp, &bb) == TRUE) {
                    anchor_point(x, y, vp);
	            slide_region(bb, x - anchor_x, y - anchor_y, 0);
                    set_action(PLACE_TIMESTAMP_2ND);
                }
                break;
            case PLACE_TIMESTAMP_2ND:
                shift.x = vp.x - anchor_vp.x;
                shift.y = vp.y - anchor_vp.y;
                move_timestamp(shift);
                xdrawgraph();
                set_action(PLACE_TIMESTAMP_1ST);
                break;
	    case SEL_POINT:
		get_graph_locator(cg, &locator);
		view2world(vp.x, vp.y, &locator.dsx, &locator.dsy);
                locator.pointset = TRUE;
		set_graph_locator(cg, &locator);
		update_locator_items(cg);
                xdrawgraph();
		set_action(DO_NOTHING);
		break;
	    case DEF_REGION1ST:
		anchor_point(x, y, vp);
                select_line(x, y, anchor_x, anchor_y, 0);
		set_action(DEF_REGION2ND);
		break;
	    case DEF_REGION2ND:
		set_action(DO_NOTHING);
                select_line(x, y, anchor_x, anchor_y, 0);
		rg[nr].active = TRUE;
		rg[nr].type = regiontype;
		if (regionlinkto) {
		    for (i = 0; i < number_of_graphs() ; i++) {
			rg[nr].linkto[i] = TRUE;
		    }
		} else {
		    for (i = 0; i < number_of_graphs() ; i++) {
			rg[nr].linkto[i] = FALSE;
		    }
		    rg[nr].linkto[cg] = TRUE;
		}
		rg[nr].type = regiontype;
		view2world(anchor_vp.x, anchor_vp.y, &rg[nr].x1, &rg[nr].y1);
		view2world(vp.x, vp.y, &rg[nr].x2, &rg[nr].y2);
                xdrawgraph();
		break;
	    case DEF_REGION:
                anchor_point(x, y, vp);
		iax[region_pts] = x;
		iay[region_pts] = y;
                view2world(vp.x, vp.y,
                    &region_wps[region_pts].x, &region_wps[region_pts].y);
		if (region_pts < MAX_POLY_POINTS) {
                    region_pts++;
                } else {
                    errmsg("Too many points in polygon!");
                }
                select_line(x, y, anchor_x, anchor_y, 0);
		break;
            default:
                break;
            }
            break;
	case Button2:
            switch (action_flag) {
            case TRACKER:
                if (find_point(cg, vp, &track_setno, &track_loc) == GRACE_EXIT_SUCCESS) {
                    track_gno = cg;
                    track_point(cg, track_setno, &track_loc, 0);
                } else {
                    xv_setstr(locate_point_item, "");
                    track_gno = -1;
                }
                break;
            default:
                break;
            }
            break;
	case Button3:
            switch (action_flag) {
            case 0:
/*
 *                 find_item(cg, vp, &anchor_vp, &type, &id);
 *                 sprintf(buf, "type = %d, id = %d", type, id);
 *                 set_left_footer(buf);
 */
                break;
            case TRACKER:
                if (track_gno == cg) {
                    track_point(cg, track_setno, &track_loc, +1);
                }
                break;
            case DEF_REGION:
		/* end region definition */
                select_line(x, y, iax[0], iay[0], 0);
		load_poly_region(nr, region_pts, region_wps);
		if (regionlinkto) {
		    for (i = 0; i < number_of_graphs() ; i++) {
		        rg[nr].linkto[i] = TRUE;
		    }
		} else {
		    for (i = 0; i < number_of_graphs() ; i++) {
		        rg[nr].linkto[i] = FALSE;
		    }
		    rg[nr].linkto[cg] = TRUE;
		}
                set_action(DO_NOTHING);
                xdrawgraph();
                break;
            default:
                set_action(DO_NOTHING);
                break;
            }
            return;
	default: /* TODO: wheel mice */
            break;
        }
	break;
    case KeyPress:
	XLookupString((XKeyEvent *) event, &keybuf, 1, &keys, &compose);
        if (keybuf == 27) { /* Esc */
            set_action(DO_NOTHING);
            return;
        }
        break;
    default:
	break;
    }
}




/*
 * action callback
 */
void set_actioncb(Widget w, XtPointer client_data, XtPointer call_data)
{
    int func = (int) client_data;
    set_action(DO_NOTHING);
    set_action(func);
}


/*
 * set the action_flag to the desired action (actions are
 * defined in defines.h), if 0 then cleanup the results
 * from previous actions.
 */
void set_action(CanvasAction act)
{
    int i;
/*
 * indicate what's happening with a message in the left footer
 */
    switch (act) {
    case 0:
        switch (action_flag) {
        case ZOOM_2ND:
        case ZOOMX_2ND:
        case ZOOMY_2ND:
        case VIEW_2ND:
            select_region(x, y, anchor_x, anchor_y, 0);
            break;
        case MOVE_OBJECT_2ND:
        case COPY_OBJECT2ND:
        case PLACE_LEGEND_2ND:
        case PLACE_TIMESTAMP_2ND:
            slide_region(bb, x - anchor_x, y - anchor_y, 0);
            break;
        case MAKE_LINE_2ND:
        case DEF_REGION2ND:
	    select_line(x, y, anchor_x, anchor_y, 0);
            break;
        case DEF_REGION:
	    select_line(x, y, anchor_x, anchor_y, 0);
	    for (i = 0; i < region_pts - 1; i++) {
                select_line(iax[i + 1], iay[i + 1], iax[i], iay[i], 0); 
            }
            break;
        case MOVE_POINT2ND:
	    switch (move_dir) {
	    case 0:
	        select_line(x, y, anchor_x, anchor_y, 0);
                break;
	    case 1:
	        select_line(x, anchor_y, anchor_x, anchor_y, 0);
	        break;
	    case 2:
	        select_line(anchor_x, y, anchor_x, anchor_y, 0);
	        break;
	    }
            break;
        default:
            break;
        }

	set_cursor(-1);
	set_default_message(buf);
	set_left_footer(buf);
	break;
    case ZOOM_1ST:
	set_cursor(0);
	echomsg("Pick first corner for zoom");
	break;
    case ZOOM_2ND:
	echomsg("Pick second corner for zoom");
	break;
    case ZOOMX_1ST:
	set_cursor(0);
	echomsg("Pick first point for zoom along X-axis");
	break;
    case ZOOMX_2ND:
	echomsg("Pick second point for zoom along X-axis");
	break;
    case ZOOMY_1ST:
	set_cursor(0);
	echomsg("Pick first point for zoom along Y-axis");
	break;
    case ZOOMY_2ND:
	echomsg("Pick second point for zoom along Y-axis");
	break;
    case VIEW_1ST:
	set_cursor(0);
	echomsg("Pick first corner of viewport");
	break;
    case VIEW_2ND:
	echomsg("Pick second corner of viewport");
	break;
    case EDIT_OBJECT:
	set_cursor(1);
	echomsg("Pick object to edit");
	break;
    case DEL_OBJECT:
	set_cursor(3);
	echomsg("Delete object");
	break;
    case MOVE_OBJECT_1ST:
	set_cursor(1);
	echomsg("Pick object to move");
	break;
    case COPY_OBJECT1ST:
	set_cursor(1);
	echomsg("Pick object to copy");
	break;
    case MOVE_OBJECT_2ND:
    case COPY_OBJECT2ND:
	set_cursor(4);
	echomsg("Place object");
	break;
    case STR_LOC:
	set_cursor(2);
	echomsg("Pick beginning of text");
	break;
    case MAKE_LINE_1ST:
	set_cursor(0);
	echomsg("Pick beginning of line");
	break;
    case MAKE_LINE_2ND:
	echomsg("Pick end of line");
	break;
    case MAKE_BOX_1ST:
	set_cursor(0);
	echomsg("First corner of box");
	break;
    case MAKE_BOX_2ND:
	echomsg("Second corner of box");
	break;
    case MAKE_ELLIP_1ST:
	set_cursor(0);
	echomsg("Pick beginning of bounding box for ellipse");
	break;
    case MAKE_ELLIP_2ND:
	echomsg("Pick opposite corner");
	break;
    case AUTO_NEAREST:
	set_cursor(0);
	echomsg("Autoscale on nearest set - click near a point of the set to autoscale");
	break;
    case TRACKER:
	set_cursor(1);
	echomsg("Tracker");
	break;
    case DEL_POINT:
	set_cursor(3);
	echomsg("Delete point");
	break;
    case MOVE_POINT1ST:
	set_cursor(4);
	echomsg("Pick point to move");
	break;
    case MOVE_POINT2ND:
	echomsg("Pick final location");
	break;
    case ADD_POINT:
	set_cursor(0);
	echomsg("Add point");
	break;
    case PLACE_LEGEND_1ST:
	set_cursor(1);
	echomsg("Pick legend");
	break;
    case PLACE_LEGEND_2ND:
	set_cursor(4);
	echomsg("Move legend");
	break;
    case PLACE_TIMESTAMP_1ST:
	set_cursor(1);
	echomsg("Pick timestamp");
	break;
    case PLACE_TIMESTAMP_2ND:
	set_cursor(4);
	echomsg("Place timestamp");
	break;
    case SEL_POINT:
	set_cursor(0);
	echomsg("Pick reference point");
	break;
    case DEF_REGION1ST:
	set_cursor(0);
	echomsg("Pick first point for region");
	break;
    case DEF_REGION2ND:
	echomsg("Pick second point for region");
	break;
    case DEF_REGION:
	set_cursor(0);
	echomsg("Define region");
	break;

    case COMP_AREA:
	set_cursor(0);
	echomsg("Compute area");
	break;
    case COMP_PERIMETER:
	set_cursor(0);
	echomsg("Compute perimeter");
	break;
    case DISLINE1ST:
	set_cursor(0);
	echomsg("Pick start of line for distance computation");
	break;
    case DISLINE2ND:
	set_cursor(0);
	echomsg("Pick ending point");
	break;
    }

    action_flag = act;
}



void track_point(int gno, int setno, int *loc, int shift)
{
    int len;
    double *xtmp, *ytmp;
    WPoint wp;
    VPoint vp;
    
    if ((len = getsetlength(gno, setno)) > 0) {
        *loc += shift;
        if (*loc < 0) {
            *loc += len;
        } else {
            *loc = *loc % len;
        }
        xtmp = getx(gno, setno);
        ytmp = gety(gno, setno);
        wp.x = xtmp[*loc];
        wp.y = ytmp[*loc];
       
        vp = Wpoint2Vpoint(wp);
        setpointer(vp);

        sprintf(buf, "G%d.S%d, loc %d, (%f, %f)", gno, setno, *loc, wp.x, wp.y);
        xv_setstr(locate_point_item, buf);
    }
}



/*
 * locator on main_panel
 */
void getpoints(VPoint vp)
{
    double wx, wy, xtmp, ytmp;
    int x, y;
    double dsx = 0.0, dsy = 0.0;
    char buf[256];
    GLocator locator;

    view2world(vp.x, vp.y, &wx, &wy);
    get_graph_locator(get_cg(), &locator);
    if (locator.pointset) {
	dsx = locator.dsx;
	dsy = locator.dsy;
    }
    
    if (go_locateflag == FALSE) {
	return;
    }

    switch (locator.pt_type) {
    case 0:
        if (get_graph_type(get_cg()) == GRAPH_POLAR) {
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
        if (get_graph_type(get_cg()) == GRAPH_POLAR) {
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
            xtmp = hypot(dsx - wx, dsy - wy);
            ytmp = 180.0 + 180.0 / M_PI * atan2(dsy - wy, dsx - wx);
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
        xlibVPoint2dev(vp, &x, &y);
        xtmp = x;
        ytmp = y;
        break;
    default:
        return;
    }
    sprintf(buf, locator_format, get_cg(), xtmp, ytmp);

    SetLabel(loclab, buf);
}

/*
 * for world stack
 */
void set_stack_message(void)
{
    if (stack_depth_item) {
	sprintf(buf, " SD:%1d ", graph_world_stack_size(get_cg()));
	SetLabel(stack_depth_item, buf);
        sprintf(buf, " CW:%1d ", get_world_stack_current(get_cg()));
	SetLabel(curw_item, buf);
    }
}


void set_default_message(char *buf)
{
    char *str, hbuf[GR_MAXHOSTNAMELEN+1];
    struct tm tm;
    time_t time_value;
    (void) time(&time_value);
    tm = *localtime(&time_value);
    str = asctime(&tm);
    str[strlen(str) - 1] = 0;

    (void) gethostname(hbuf, GR_MAXHOSTNAMELEN);

    sprintf(buf, "%s, %s, %s, %s", hbuf, display_name(), str, docname);
}


/*
 * switch on the area calculator
 */
void do_select_area(void)
{
    set_action(DO_NOTHING);
    set_action(COMP_AREA);
}

/*
 * switch on the perimeter calculator
 */
void do_select_peri(void)
{
    set_action(DO_NOTHING);
    set_action(COMP_PERIMETER);
}

void do_dist_proc(void)
{
    set_action(DO_NOTHING);
    set_action(DISLINE1ST);
}


/*
 * define a (polygon) region
 */
void do_select_region(void)
{
    region_pts = 0;
    set_action(DO_NOTHING);
    set_action(DEF_REGION);
}

/*
 * Given the graph gno, find the graph that contains
 * (wx, wy). Used for setting the graph focus.
 */
int next_graph_containing(int cg, VPoint vp)
{
    int i, j, ng, gno = -1;
    view v;

    ng = number_of_graphs();

    if (is_valid_gno(cg) == FALSE) {
        cg = -1;
    }

    for (i = 0; i < ng ; i++) {
	j = (i + cg + 1) % ng;
	if (is_graph_hidden(j)        == FALSE &&
            get_graph_viewport(j, &v) == GRACE_EXIT_SUCCESS &&
            is_vpoint_inside(v, vp, MAXPICKDIST)   == TRUE) {
	    
            gno = j;
            break;
	}
    }

    return gno;
}

int legend_clicked(int gno, VPoint vp, view *bb)
{
    legend l;

    if (is_valid_gno(gno)) {
        get_graph_legend(gno, &l);
	if (l.active && is_vpoint_inside(l.bb, vp, MAXPICKDIST)) {
	    *bb = l.bb;
            return TRUE;
	} else {
            return FALSE;
        }
    } else {
        return FALSE;
    }
}

int timestamp_clicked(VPoint vp, view *bb)
{
    if (timestamp.active && is_vpoint_inside(timestamp.bb, vp, MAXPICKDIST)) {
        *bb = timestamp.bb;
        return TRUE;
    } else {
        return FALSE;
    }
}

int focus_clicked(int cg, VPoint vp, VPoint *avp)
{
    view v;
    
    if (get_graph_viewport(cg, &v) != GRACE_EXIT_SUCCESS) {
        return FALSE;
    }

    if (fabs(vp.x - v.xv1) < MAXPICKDIST && fabs(vp.y - v.yv1) < MAXPICKDIST) {
        avp->x = v.xv2;
        avp->y = v.yv2;
        return TRUE;
    } else if (fabs(vp.x - v.xv1) < MAXPICKDIST && fabs(vp.y - v.yv2) < MAXPICKDIST) {
        avp->x = v.xv2;
        avp->y = v.yv1;
        return TRUE;
    } else if (fabs(vp.x - v.xv2) < MAXPICKDIST && fabs(vp.y - v.yv1) < MAXPICKDIST) {
        avp->x = v.xv1;
        avp->y = v.yv2;
        return TRUE;
    } else if (fabs(vp.x - v.xv2) < MAXPICKDIST && fabs(vp.y - v.yv2) < MAXPICKDIST) {
        avp->x = v.xv1;
        avp->y = v.yv1;
        return TRUE;
    } else {
        return FALSE;
    }
}

int axis_clicked(int gno, VPoint vp, int *axisno)
{
    view v;
    
    /* TODO: check for offsets, zero axes, polar graphs */
    if (is_valid_gno(gno) == FALSE) {
        return FALSE;
    } else {
        get_graph_viewport(gno, &v);
        if (vp.x >= v.xv1 && vp.x <= v.xv2 &&
            (fabs(vp.y - v.yv1) < MAXPICKDIST ||
             fabs(vp.y - v.yv2) < MAXPICKDIST)) {
            *axisno = X_AXIS;
            return TRUE;
        } else if (vp.y >= v.yv1 && vp.y <= v.yv2 &&
            (fabs(vp.x - v.xv1) < MAXPICKDIST ||
             fabs(vp.x - v.xv2) < MAXPICKDIST)) {
            *axisno = Y_AXIS;
            return TRUE;
        } else {
            return FALSE;
        }
    }
}

int title_clicked(int gno, VPoint vp)
{
    view v;
    
    /* a rude check; TODO: use right offsets */
    if (is_valid_gno(gno) == FALSE) {
        return FALSE;
    } else {
        get_graph_viewport(gno, &v);
        if (vp.x >= v.xv1 && vp.x <= v.xv2 &&
            vp.y > v.yv2 && vp.y < v.yv2 + 0.1) {
            return TRUE;
        } else {
            return FALSE;
        }
    }
}

/*
 * locate a point and the set the point is in
 */
int find_point(int gno, VPoint vp, int *setno, int *loc)
{
    int i, j;
    double *xtmp, *ytmp;
    WPoint wptmp;
    VPoint vptmp;
    double dist, mindist = MAXPICKDIST;

    *setno = -1;
    if (is_valid_gno(gno) != TRUE) {
        return GRACE_EXIT_FAILURE;
    }
    
    for (i = 0; i < number_of_sets(gno); i++) {
	if (is_set_hidden(gno, i) == FALSE) {
	    xtmp = getx(gno, i);
	    ytmp = gety(gno, i);
	    for (j = 0; j < getsetlength(gno, i); j++) {
		wptmp.x = xtmp[j];
		wptmp.y = ytmp[j];
                vptmp = Wpoint2Vpoint(wptmp);
                
                dist = MAX2(fabs(vp.x - vptmp.x), fabs(vp.y - vptmp.y));
                if (dist < mindist) {
		    *setno = i;
		    *loc = j;
                    mindist = dist;
		}
	    }
	}
    }
    
    if (*setno == -1) {
        return GRACE_EXIT_FAILURE;
    } else {
        return GRACE_EXIT_SUCCESS;
    }
}

int find_insert_location(int gno, int setno, VPoint vp)
{
    int j, loc = -1;
    double *xtmp, *ytmp;
    WPoint wptmp;
    VPoint vp1, vp2;
    double dist, mindist = 1.0;
    
    if (is_valid_setno(gno, setno) == TRUE) {
        if (is_set_hidden(gno, setno) == FALSE) {
            xtmp = getx(gno, setno);
            ytmp = gety(gno, setno);
            for (j = 0; j < getsetlength(gno, setno) - 1; j++) {
                wptmp.x = xtmp[j];
                wptmp.y = ytmp[j];
                vp1 = Wpoint2Vpoint(wptmp);
                wptmp.x = xtmp[j + 1];
                wptmp.y = ytmp[j + 1];
                vp2 = Wpoint2Vpoint(wptmp);
 
                dist = hypot(vp.x - vp1.x, vp.y - vp1.y) +
                       hypot(vp.x - vp2.x, vp.y - vp2.y);
                if (dist < mindist) {
                    loc = j + 1;
                    mindist = dist;
                }
            }
        }
    }
    
    return loc;
}


/*
 * find object containing vp inside its bb
 */
int find_item(int gno, VPoint vp, view *bb, int *type, int *id)
{
    int i;

    *type = OBJECT_NONE;
    for (i = 0; i < maxboxes; i++) {
	if (isactive_box(i)) {
            get_object_bb(OBJECT_BOX, i, bb);
	    if (is_vpoint_inside(*bb, vp, MAXPICKDIST)) {
		*type = OBJECT_BOX;
		*id = i;
	    }
	}
    }
    for (i = 0; i < maxboxes; i++) {
	if (isactive_ellipse(i)) {
            get_object_bb(OBJECT_ELLIPSE, i, bb);
	    if (is_vpoint_inside(*bb, vp, MAXPICKDIST)) {
		*type = OBJECT_ELLIPSE;
		*id = i;
	    }
	}
    }
    for (i = 0; i < maxlines; i++) {
	if (isactive_line(i)) {
            get_object_bb(OBJECT_LINE, i, bb);
	    if (is_vpoint_inside(*bb, vp, MAXPICKDIST)) {
		*type = OBJECT_LINE;
		*id = i;
	    }
	}
    }
    for (i = 0; i < maxstr; i++) {
	if (isactive_string(i)) {
            get_object_bb(OBJECT_STRING, i, bb);
	    if (is_vpoint_inside(*bb, vp, MAXPICKDIST)) {
		*type = OBJECT_STRING;
		*id = i;
	    }
	}
    }
    
    if (*type == OBJECT_NONE) {
        return GRACE_EXIT_FAILURE;
    } else {
        get_object_bb(*type, *id, bb);
        return GRACE_EXIT_SUCCESS;
    }
}


/*
 * for zooms
 *
 */
void newworld(int gno, int axes, VPoint vp1, VPoint vp2)
{
    world w, wtmp;

    if (vp1.x == vp2.x || vp1.y == vp2.y) {
        errmsg("Zoomed rectangle is zero along X or Y, zoom cancelled");
        return;
    }

    view2world(vp1.x, vp1.y, &w.xg1, &w.yg1);
    view2world(vp2.x, vp2.y, &w.xg2, &w.yg2);
    if (w.xg1 > w.xg2) {
        fswap(&w.xg1, &w.xg2);
    }
    if (w.yg1 > w.yg2) {
        fswap(&w.yg1, &w.yg2);
    }

    if (is_graph_active(gno)) {
        get_graph_world(gno, &wtmp);
        switch (axes) {
        case ALL_AXES:
            wtmp.xg1 = w.xg1;
            wtmp.xg2 = w.xg2;
            wtmp.yg1 = w.yg1;
            wtmp.yg2 = w.yg2;
            break;
        case ALL_X_AXES:
            wtmp.xg1 = w.xg1;
            wtmp.xg2 = w.xg2;
            break;
        case ALL_Y_AXES:
            wtmp.yg1 = w.yg1;
            wtmp.yg2 = w.yg2;
            break;
        default:
            return;
            break;
        }
        set_graph_world(gno, wtmp);
        autotick_axis(gno, axes);
        xdrawgraph();
    }
}



/* -------------------------------------------------------------- */
/* canvas_actions */
void autoscale_action(Widget w, XKeyEvent *e, String *p, Cardinal *c)
{
    int cg = get_cg();
    
    autoscale_graph(cg, AUTOSCALE_XY);
    update_ticks(cg);
}

void autoscale_on_near_action( Widget w, XKeyEvent *e, String *p, Cardinal *c )
{
    set_action(DO_NOTHING);
    set_action(AUTO_NEAREST);       
}

void draw_box_action( Widget w, XKeyEvent *e, String *p, Cardinal *c )
{
    set_action(DO_NOTHING);
    set_action(MAKE_BOX_1ST);       
}

void delete_object_action( Widget w, XKeyEvent *e, String *p, Cardinal *c )
{
    set_action(DO_NOTHING);
    set_action(DEL_OBJECT); 
}

void place_legend_action( Widget w, XKeyEvent *e, String *p, Cardinal *c )
{
    set_action(DO_NOTHING);
    set_action(PLACE_LEGEND_1ST);
}

void place_timestamp_action( Widget w, XKeyEvent *e, String *p, Cardinal *c )
{
    set_action(DO_NOTHING);
    set_action(PLACE_TIMESTAMP_1ST);
}

void move_object_action( Widget w, XKeyEvent *e, String *p, Cardinal *c )
{
    set_action(DO_NOTHING);
    set_action(MOVE_OBJECT_1ST);
}

void draw_line_action( Widget w, XKeyEvent *e, String *p, Cardinal *c )
{
    set_action(DO_NOTHING);
    set_action(MAKE_LINE_1ST);
}

void refresh_hotlink_action( Widget w, XKeyEvent *e, String *p, Cardinal *c )
{
    do_hotupdate_proc( (Widget)NULL, (XtPointer)NULL, (XtPointer)NULL );
}

void set_viewport_action( Widget w, XKeyEvent *e, String *p, Cardinal *c )
{
    set_action(DO_NOTHING);
    set_action(VIEW_1ST);
}

void write_string_action( Widget w, XKeyEvent *e, String *p, Cardinal *c )
{
    set_action(DO_NOTHING);
    set_action(STR_LOC);
}

void exit_abruptly_action( Widget w, XKeyEvent *e, String *p, Cardinal *c )
{
    bailout();	
}

void enable_zoom_action( Widget w, XKeyEvent *e, String *p, Cardinal *c )
{
    set_action(DO_NOTHING);
    set_action(ZOOM_1ST);
}


/*
 * world stack operations
 */
void push_and_zoom(void)
{
    push_world();
    set_action(DO_NOTHING);
    set_action(ZOOM_1ST);
}
