/*
 * Grace - Graphics for Exploratory Data Analysis
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-95 Paul J Turner, Portland, OR
 * Copyright (c) 1996-98 GRACE Development Team
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
/* #include <netdb.h> */

#include "globals.h"
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
#include <X11/Shell.h>
#include <X11/keysym.h>

#include <Xm/Xm.h>


#if defined(MAXHOSTNAMELEN)
#  define GR_MAXHOSTNAMELEN MAXHOSTNAMELEN
#else
#  define GR_MAXHOSTNAMELEN 64
#endif

extern graph *g;

extern Widget legend_x_panel, legend_y_panel;	/* from symwin.c */
extern Widget timestamp_x_item, timestamp_y_item;	/* from miscwin.c */
extern Widget loclab;
extern Widget arealab;
extern Widget perimlab;
extern Widget locate_item;
extern Widget locate_point_item;
extern Widget stack_depth_item;
extern Widget curw_item;
extern XmStringCharSet charset;
extern XmString astring, pstring;
extern XmString sdstring, cystring;
static Arg al;



/* for pointer based set operations */
static int setno1, setno2, graphno1, graphno2, loc1, loc2;

/*
 * xlib objects for drawing
 */
extern Display *disp;
extern Window xwin;
extern GC gc;

extern int win_h, win_w;	/* declared in x11drv.c */

int rectflag = 0;		/* if an xor'ed rectangle is drawn with mouse */
int rubber_flag = 0;		/* set rubber band line */
int mbox_flag = 0;		/* moving box attached to cursor */
int mline_flag = 0;		/* moving line attached to cursor */

int go_locateflag = TRUE;	/* locator */

int add_setno;			/* set to add points - set in ptswin.c */
int add_at;			/* where to begin inserting points in the set */
int move_dir;			/* restriction on point movement */

extern int cset;
extern int digit_setno;
extern int track_set, track_point;   /* from ptswin.c */
extern int paint_skip;	/* defined in ptswin.c */
extern char locator_format[];

static char buf[256];

void set_action(int act);
void select_line(int x1, int y1, int x2, int y2);

void do_autoscale_set(int gno, int setno);
static void _device2world(int x, int y, double *wx, double *wy);

/* old stubs */
static double _stringextentx(double scale, char *s);
static double _stringextenty(double scale, char *s);
void _world2device(double wx, double wy, int *x, int *y);


/*
 * variables for the canvas event proc
 */
static int sx, sy;
static int old_x, old_y;
static int xs, ys;
int action_flag = 0;
static int setindex = 0;
static int setnumber = 0;

/*
 * variables for the text handling routine
 */
static int strx = 0, stry = 0;
static int drawx = 0, drawy = 0;
static char tmpstr[256] = "";
static int justflag = 0;
static double si = 0.0;
static double co = 1.0;

/* pointer to insertion point in tmpstr */
static char *tmpstrins;

/*
 * for region, area and perimeter computation
 */
#define MAX_AREA_POLY 200
int narea_pts = 0;
int region_pts = 0;
extern int regiontype;
int regionlinkto = 0;

double area_polyx[MAX_AREA_POLY];
double area_polyy[MAX_AREA_POLY];
int iax[MAX_AREA_POLY];
int iay[MAX_AREA_POLY];


/*
 * action callback
 */
void set_actioncb(Widget w, XtPointer client_data, XtPointer call_data)
{
    int func = (int) client_data;
    set_action(0);
    set_action(func);
}

/*
 * for the goto point feature
 */
void setpointer(int x, int y)
{
    XWarpPointer(disp, None, xwin, 0, None, win_w, win_h, x, y);
}

/*
 * locator on main_panel
 */
void getpoints(int x, int y)
{
    double wx, wy, xtmp, ytmp;
    double dsx = 0.0, dsy = 0.0;
    int newg;
    char buf[256];
    XmString string;
    GLocator locator;

    _device2world(x, y, &wx, &wy);
    get_graph_locator(get_cg(), &locator);
    if (locator.pointset) {
	dsx = locator.dsx;
	dsy = locator.dsy;
    }
    if (focus_policy == FOCUS_FOLLOWS) {
	if ((newg = iscontained(get_cg(), wx, wy)) != get_cg()) {
	    switch_current_graph(newg);
	    _device2world(x, y, &wx, &wy);
	}
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
        world2view(wx, wy, &xtmp, &ytmp);
        break;
    case 5:
        xtmp = x;
        ytmp = y;
        break;
    default:
        return;
    }
    sprintf(buf, locator_format, get_cg(), xtmp, ytmp);

    string = XmStringCreateLtoR(buf, charset);
    XtSetArg(al, XmNlabelString, string);
    XtSetValues(loclab, &al, 1);
    XmStringFree(string);
}

/*
 * for world stack
 */
void set_stack_message(void)
{
    if (stack_depth_item) {
	sprintf(buf, " SD:%1d ", graph_world_stack_size(get_cg()));
	XmStringFree(sdstring);
	sdstring = XmStringCreateLtoR(buf, charset);
	XtSetArg(al, XmNlabelString, sdstring);
	XtSetValues(stack_depth_item, &al, 1);
	sprintf(buf, " CW:%1d ", g[get_cg()].curw);
	XmStringFree(cystring);
	cystring = XmStringCreateLtoR(buf, charset);
	XtSetArg(al, XmNlabelString, cystring);
	XtSetValues(curw_item, &al, 1);
    }
}


/*
 * draw a cursor for text writing
 * TODO: fix the rotation problems (cursor doesn't track)
 */
void update_text_cursor(char *s, int x, int y, char *sp )
/* s -complete text string, sp - insertion point in s */
{
    int hgt, tx, xtx, ytx, xhgt, yhgt;
    char stmp[256];

    strcpy( stmp, s );
    if ((sp - s >= 255) || (sp - s < 0)) {
    	return;
    }
    stmp[sp-s] = '\0';
 
    hgt = _stringextenty(getcharsize(), "N") / 2;

    switch( string_just ) {
    case 0:                                                                     
        /* left just */
                tx = _stringextentx(getcharsize(), stmp);
                xtx = (int) tx *co;
        ytx = (int) tx *si;
                xhgt = (int) -hgt * si;
        yhgt = (int) hgt *co;   
        break;
    case 1:                                                                     
        /* right just */
        tx = _stringextentx(getcharsize(), s ) -
             _stringextentx(getcharsize(), stmp );
        xtx = -(int) tx *co;
        ytx = -(int) tx *si;
        xhgt = (int) -hgt * si;
        yhgt = (int) hgt *co;   
        break;
    case 2:                                                                     
        /* centered */
        tx = (double)_stringextentx(getcharsize(), stmp) - 
                 (double)_stringextentx(getcharsize(), s)/2.;
        xtx = (int) tx *co;
        ytx = (int) tx *si;
        xhgt = (int) -hgt * si;
        yhgt = (int) hgt *co;   
        break;
    default:                                                                   
        /* should never come here */
        xtx = ytx = xhgt = yhgt = 0;
        errmsg("Internal error, update_text_cursor called with wrong argument");
        }
 
     select_line(x + xtx + xhgt, win_h - (y + ytx + yhgt),
				 x + xtx - xhgt, win_h - (y + ytx - yhgt));
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

    sprintf(buf, "%s, %s, %s, %s", hbuf, DisplayString(disp), str, docname);
}

/*
 * set the action_flag to the desired action (actions are
 * defined in defines.h), if 0 then cleanup the results
 * from previous actions.
 */
void set_action(int act)
{
    char tmpbuf[128];
    if (action_flag == STR_LOC) {
	double wx, wy;

	update_text_cursor(tmpstr, strx, stry, tmpstrins);
	setcharsize(grdefaults.charsize);
	setfont(grdefaults.font);
	setcolor(grdefaults.color);
	setlinestyle(grdefaults.lines);
	setlinewidth(grdefaults.linew);
	if (tmpstr[0]) {
	    _device2world(strx, win_h - stry, &wx, &wy);
	    define_string(tmpstr, wx, wy);
	    tmpstr[0] = 0;
	}
    }
/*
 * indicate what's happening with a message in the left footer
 */
    switch (action_flag = act) {
    case DEL_OBJECT:
	set_cursor(3);
	echomsg("Delete object");
	break;
    case EDIT_OBJECT:
	set_cursor(0);
	echomsg("Click near a line, box, or string");
	break;
    case MOVE_OBJECT_1ST:
	set_cursor(4);
	echomsg("Pick object to move");
	break;
    case MOVE_OBJECT_2ND:
	echomsg("Place object");
	break;
    case COPY_OBJECT1ST:
	set_cursor(4);
	echomsg("Pick object to copy");
	break;
    case COPY_OBJECT2ND:
	echomsg("Place object");
	break;
    case MAKE_BOX_1ST:
	set_cursor(0);
	echomsg("First corner of box");
	break;
    case MAKE_BOX_2ND:
	echomsg("Second corner of box");
	break;
    case STR_LOC1ST:
	set_cursor(0);
	echomsg("Pick start of text line");
	break;
    case STR_LOC2ND:
	echomsg("Pick end of text line");
	break;
    case MAKE_LINE_1ST:
	set_cursor(0);
	echomsg("Pick beginning of line");
	break;
    case MAKE_LINE_2ND:
	echomsg("Pick end of line");
	break;
    case MAKE_ELLIP_1ST:
	set_cursor(0);
	echomsg("Pick beginning of bounding box for ellipse");
	break;
    case MAKE_ELLIP_2ND:
	echomsg("Pick opposite corner");
	break;
    case STR_EDIT:
	set_cursor(2);
	echomsg("Edit string");
	break;
    case STR_LOC:
	set_cursor(2);
	echomsg("Pick beginning of text");
	break;
    case FIND_POINT:
	set_cursor(1);
	echomsg("Find points");
	break;
    case TRACKER:
	set_cursor(1);
	echomsg("Tracker");
	break;
    case DEF_REGION:
	set_cursor(0);
	echomsg("Define region");
	break;
    case DEF_REGION1ST:
	set_cursor(0);
	echomsg("Pick first point for region");
	break;
    case DEF_REGION2ND:
	echomsg("Pick second point for region");
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
    case SEL_POINT:
	set_cursor(0);
	echomsg("Pick reference point");
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
    case ADD_POINT1ST:
	set_cursor(0);
	echomsg("Pick 1st control point");
	break;
    case ADD_POINT2ND:
	set_cursor(0);
	echomsg("Pick 2nd control point");
	break;
    case ADD_POINT3RD:
	set_cursor(0);
	echomsg("Pick 3rd control point");
	break;
    case PAINT_POINTS:
	set_cursor(0);
	echomsg("Paint points - hold left mouse button down and move");
	break;
    case AUTO_NEAREST:
	set_cursor(0);
	echomsg("Autoscale on nearest set - click near a point of the set to autoscale");
	break;
    case KILL_NEAREST:
	set_cursor(3);
	echomsg("Kill nearest set - click near a point of the set to kill");
	break;
    case COPY_NEAREST1ST:
	set_cursor(0);
	echomsg("Copy nearest set - click near a point of the set to copy");
	break;
    case COPY_NEAREST2ND:
	set_cursor(0);
	sprintf(tmpbuf, "Selected S%1d in graph %d, click in the graph to place the copy", setno1, graphno1);
	echomsg(tmpbuf);
	break;
    case MOVE_NEAREST1ST:
	set_cursor(4);
	echomsg("Move nearest set - click near a point of the set to move");
	break;
    case MOVE_NEAREST2ND:
	set_cursor(4);
	sprintf(tmpbuf, "Selected S%1d in graph %d, click in the graph to move the set", setno1, graphno1);
	echomsg(tmpbuf);
	break;
    case JOIN_NEAREST1ST:
	set_cursor(0);
	echomsg("Join 2 sets - click near a point of the first set");
	break;
    case JOIN_NEAREST2ND:
	set_cursor(0);
	echomsg("Join 2 sets - click near a point of the second set");
	break;
    case DELETE_NEAREST1ST:
	set_cursor(3);
	echomsg("Delete points in a set - click near a point of the start of the range to  delete");
	break;
    case DELETE_NEAREST2ND:
	set_cursor(3);
	echomsg("Delete points in a set - click near the end of the range to delete");
	break;
    case REVERSE_NEAREST:
	set_cursor(0);
	echomsg("Reverse order of nearest set - click near a point of the set to reverse");
	break;
    case DEACTIVATE_NEAREST:
	set_cursor(0);
	echomsg("Deactivate nearest set - click near a point of the set to deactivate");
	break;
    case LEG_LOC:
	set_cursor(0);
	echomsg("Place legend");
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
    case  VIEW_1ST:
	set_cursor(0);
	echomsg("Pick first corner of viewport");
	break;
    case  VIEW_2ND:
	echomsg("Pick second corner of viewport");
	break;
    case PLACE_TIMESTAMP:
	set_cursor(0);
	echomsg("Click at the location for the timestamp");
	break;
    case PICK_SET:
    case PICK_EXPR:
    case PICK_HISTO:
    case PICK_FOURIER:
    case PICK_RUNAVG:
    case PICK_REG:
    case PICK_SAMPLE:
    case PICK_PRUNE:
	set_cursor(0);
	echomsg("Click near a point in the set to select");
	break;
    case PICK_BREAK:
	set_cursor(0);
	echomsg("Click near a point in a set to use as the break point");
	break;
    case 0:
	set_cursor(-1);
	set_default_message(buf);
	set_left_footer(buf);

	if (rectflag) {
	    select_region(sx, sy, old_x, old_y);
	    rectflag = 0;
	}
	if (rubber_flag) {
	    select_line(sx, sy, old_x, old_y);
	    rubber_flag = 0;
	}
	if (mbox_flag) {
	    select_region(sx, sy, xs, ys);
	    mbox_flag = 0;
	}
	if (mline_flag) {
	    select_line(sx, sy, xs, ys);
	    mline_flag = 0;
	}
	break;
    }
}

/*
 * update string drawn on the canvas
 */
void do_text_string(int op, int c)
/* 
 * op = operation:    0 -> delete 1 char
                      1 -> add 1 character
                      2 -> nothing
   c = character to add
 */
{
    drawx = strx;
    drawy = stry;

    if (tmpstrins==0) return; /* tmpstrins is undefined at start*/
    update_text_cursor(tmpstr, drawx, drawy, tmpstrins);
/*
 *     WriteString(_Wxy2Vpoint(drawx, drawy), string_rot, justflag, tmpstr);
 */
    switch (op) {
    case 0:
		if ((int) strlen(tmpstr) > 0) {
	    	memmove( tmpstrins-1, tmpstrins, strlen(tmpstrins)+1 );
            tmpstrins -= 1;
		}
		break;
    case 1:
        memmove( tmpstrins+1, tmpstrins, strlen(tmpstrins)+1 );
        *tmpstrins = c;
        tmpstrins += 1;
        break;
    case 2:
	break;
    }
/*
 *     WriteString(_Wxy2Vpoint(drawx, drawy), string_rot, justflag, tmpstr);
 */
    update_text_cursor(tmpstr, drawx, drawy, tmpstrins);
}

/*
 * adjust text insertion point 
 */
void update_tmpstrins( KeySym key )
{
    drawx = strx;
    drawy = stry;
/*
 *     WriteString(_Wxy2Vpoint(drawx, drawy), string_rot, justflag, tmpstr);
 */
    update_text_cursor(tmpstr, drawx, drawy, tmpstrins);
        switch ((int) key) {
                case XK_Left:
                        if( tmpstrins>tmpstr )
                                tmpstrins -= 1;
                        break;
                case XK_Right:
                        if( *tmpstrins != '\0' )
                                tmpstrins += 1;
                        break;
                default:
                		break;
        }                       
/*
 *     WriteString(_Wxy2Vpoint(drawx, drawy), string_rot, justflag, tmpstr);
 */
    update_text_cursor(tmpstr, drawx, drawy, tmpstrins);
}
 
void update_tmpstrins2( int num )
{
    char *stmp2;
 
    drawx = strx;
    drawy = stry;
/*
 *     WriteString(_Wxy2Vpoint(drawx, drawy), string_rot, justflag, tmpstr);
 */
    update_text_cursor(tmpstr, drawx, drawy, tmpstrins);
        switch (num) {
                case 1: /* ^e go to end of string */
                        if( tmpstrins>=tmpstr )
                          tmpstrins += strlen(tmpstrins);
                        break;
                case 2: /* ^a go to start of string */
                        if( tmpstrins>tmpstr )
                          tmpstrins -= strlen(tmpstr)-strlen(tmpstrins);
                        break;
                case 3: /* ^k delete to end of string */
                        if( tmpstrins>=tmpstr )
                          *tmpstrins = '\0';
                        break;
                case 4: /* ^u delete to start of string */
                        if( tmpstrins>tmpstr ) {
                          stmp2=tmpstrins;
                          stmp2 -= strlen(tmpstr)-strlen(tmpstrins);
                          strcpy(tmpstr,tmpstrins);
                          tmpstrins=stmp2;
 
                        }
                        break;
 
        }                       
/*
 *     WriteString(_Wxy2Vpoint(drawx, drawy), string_rot, justflag, tmpstr);
 */
    update_text_cursor(tmpstr, drawx, drawy, tmpstrins);
}
 


/*
 * canvas event proc
 */
void my_proc(Widget w, XtPointer data, XEvent * event)
{
    static int x, y, boxno, lineno, stringno, ellipno;
    static double wx1, wx2, wy1, wy2;
    static double wx, wy, dx, dy;
    static int ty, no, c;
    static KeySym keys;
    static XComposeStatus compose;
    int tmpcg = get_cg();
    
    WPoint wptmp;
    VPoint vptmp;
    view v;

/*
 * hot keys
 */
    switch (event->type) {
    case KeyPress:
	buf[0] = 0;
	XLookupString((XKeyEvent *) event, buf, 1, &keys, &compose);
	switch (c = buf[0]) {
	case 1:		/* ^A */
	    if (action_flag == STR_LOC) {
	      update_tmpstrins2( 2 );
	    } 
	    break;
	case 2:		/* ^B */
	    break;
	case 3:		/* ^C */
	    break;
	case 4:		/* ^D */
	    break;
	case 5:		/* ^E */
            if (action_flag == STR_LOC) {
                update_tmpstrins2( 1 );
            }
	    break;
	case 6:		/* ^F */
	    break;
	case 7:		/* ^G */
	    break;
	    /* stay off 8 (^H) - needed by text routines */
        case 11:                /* ^K */
            if (action_flag == STR_LOC) {
                update_tmpstrins2( 3 );
            }
            break;
 	case 12:		/* ^L */
	    break;
	case 14:		/* ^N */
	    break;
	case 16:		/* ^P */
	    break;
	case 18:		/* ^R */
	    break;
	case 19:		/* ^S */
	    break;
	case 20:		/* ^T */
	    break;
        case 21:                /* ^U */
            if (action_flag == STR_LOC) {
                update_tmpstrins2( 4 );
            }
            break;
	case 22:		/* ^V */
	    break;
	case 23:		/* ^W */
	    break;
	case 24:		/* ^X */
	    break;
	case 26:		/* ^Z */
	    break;
	case 8:
	case 127:
	    if (action_flag == STR_LOC) {
	        do_text_string(0, 0);
	    }
	    break;
	case '\r':
	case '\n':
	    if (action_flag == STR_LOC) {
		int itmp;

		update_text_cursor(tmpstr, drawx, drawy, tmpstrins);
		if (tmpstr[0]) {
		    _device2world(drawx, win_h - drawy, &wx, &wy);
		    define_string(tmpstr, wx, wy);
		}
		itmp = (int) (1.25 * _stringextenty(getcharsize(), "Ny"));
		strx = strx + si * itmp;
		stry = stry - co * itmp;
		tmpstr[0] = 0;
                tmpstrins=tmpstr;
		update_text_cursor(tmpstr, strx, stry, tmpstrins);
	    }
	    break;
	default:
	    if (action_flag == STR_LOC) {
        	if( c==0 )
            	update_tmpstrins( keys );
			else if (c >= 32 && c < 128) {
		    	do_text_string(1, c);
			}
	    }
	    break;
	}
	break;
    case ButtonPress:
	switch (event->xbutton.button) {
	case Button3:
	    getpoints(x, y);
	    if (action_flag == COMP_AREA || action_flag == COMP_PERIMETER) {
		if (narea_pts >= 3) {
		    int i;

		    for (i = 0; i < narea_pts; i++) {
			XDrawLine(disp, xwin, gc, iax[i], iay[i], iax[(i + 1) % narea_pts], iay[(i + 1) % narea_pts]);
		    }
		}
	    }
	    if (action_flag == DEF_REGION) {
		if (region_pts >= 3) {
		    int i;

		    for (i = 0; i < region_pts; i++) {
			XDrawLine(disp, xwin, gc, iax[i], iay[i], iax[(i + 1) % region_pts], iay[(i + 1) % region_pts]);
		    }
		    load_poly_region(nr, region_pts, area_polyx, area_polyy);
		    if (regionlinkto) {
			int i;

			for (i = 0; i < number_of_graphs() ; i++) {
			    rg[nr].linkto[i] = TRUE;
			}
		    } else {
			int i;

			for (i = 0; i < number_of_graphs() ; i++) {
			    rg[nr].linkto[i] = FALSE;
			}
			rg[nr].linkto[get_cg()] = TRUE;
		    }
		}
	    }
	    narea_pts = 0;
	    region_pts = 0;
	    set_action(0);
	    break;
	case Button1:
	    if (action_flag == 0) {
		if (focus_policy == FOCUS_CLICK) {
		    int newg;

		    _device2world(x, y, &wx, &wy);
		    if ( !focus_clicked(get_cg(),x,y) && 
		         (newg = nextcontained(get_cg(), wx, wy)) != get_cg() &&
		          !double_click((XButtonEvent *) event)) {
			switch_current_graph(newg);
			_device2world(x, y, &wx, &wy);
		    }
		}
	    }
	    c = go_locateflag;
	    go_locateflag = TRUE;
	    getpoints(x, y);
	    go_locateflag = c;
	    {
		if (!action_flag && allow_dc && double_click((XButtonEvent *) event)) {
		    if (tmpcg == get_cg()) {	/* don't allow a change of focus */
			int setno, loc;

			_device2world(x, y, &wx, &wy);
			wptmp.x = wx;
			wptmp.y = wy;
			vptmp = Wpoint2Vpoint(wptmp);
			if( focus_clicked( get_cg(), x, y ) ){
				double xm, ym;
				
				get_corner_clicked( get_cg(), x, y, &xm, &ym );
				if( xm == g[get_cg()].v.xv1 ){		
					xm = g[get_cg()].v.xv2;
				} else {
					xm = g[get_cg()].v.xv1;
				}
				if( ym == g[get_cg()].v.yv1 ){		
					ym = g[get_cg()].v.yv2;
				} else {
					ym = g[get_cg()].v.yv1;
				}
				vptmp.x = xm;
				vptmp.y = ym;
				xlibVPoint2dev(vptmp, &sx, &sy );
				set_action(  VIEW_2ND );
				rectflag = 1;
				select_region(sx, sy, x, y );
				break;	
			} else if (is_validVPoint(vptmp)) {
			    findpoint(get_cg(), wx, wy, &wx, &wy, &setno, &loc);
			    if (setno != -1) {
					cset = setno;
					set_wait_cursor();
					define_symbols_popup(NULL, NULL, NULL);
					unset_wait_cursor();
			    } else {
					set_wait_cursor();
					create_file_popup(NULL, NULL, NULL);
					unset_wait_cursor();
/* annoying error message
					errwin("No set!");
*/
			    }
			} else {
			    if (wx < g[get_cg()].w.xg1) {
					curaxis = 1;
					create_axes_dialog(NULL, NULL, NULL);
			    } else if (wy < g[get_cg()].w.yg1) {
					curaxis = 0;
					create_axes_dialog(NULL, NULL, NULL);
			    } else if (wy > g[get_cg()].w.yg2) {
					create_graphapp_frame(NULL, NULL, NULL);
			    } else if (wx > g[get_cg()].w.xg2) {
					create_graphapp_frame(NULL, NULL, NULL);
			    }
			}
		    }
		}
	    }
	    switch (action_flag) {
	    case DEL_OBJECT:	/* delete a box or a line */
		set_action(0);
		_device2world(x, y, &wx, &wy);
		find_item(get_cg(), wx, wy, &ty, &no);
		if (ty >= 0) {
		    switch (ty) {
		    case OBJECT_BOX:
			kill_box(no);
			break;
		    case OBJECT_ELLIPSE:
			kill_ellipse(no);
			break;
		    case OBJECT_LINE:
			kill_line(no);
			break;
		    case OBJECT_STRING:
			kill_string(no);
			break;
		    }
		    set_action(DEL_OBJECT);
		}
		break;
/*
 * select an object to move
 */
	    case MOVE_OBJECT_1ST:
		set_action(MOVE_OBJECT_2ND);
		_device2world(x, y, &wx, &wy);
		find_item(get_cg(), wx, wy, &ty, &no);
		if (ty < 0) {
			set_action(MOVE_OBJECT_1ST);
		} else {
		    switch (ty) {
		    case OBJECT_BOX:
			if (boxes[no].loctype == COORD_VIEW) {
			    sx = (int) (win_w * boxes[no].x1);
			    sy = (int) (win_h - win_h * boxes[no].y1);
			    xs = (int) (win_w * boxes[no].x2);
			    ys = (int) (win_h - win_h * boxes[no].y2);
			} else {
			    _world2device(boxes[no].x1, boxes[no].y1, &sx, &sy);
			    _world2device(boxes[no].x2, boxes[no].y2, &xs, &ys);
			}
			select_region(sx, sy, xs, ys);
			mbox_flag = 1;
			break;
			case OBJECT_ELLIPSE:
				if (ellip[no].loctype == COORD_VIEW) {
			    	sx = (int) (win_w * ellip[no].x1);
			    	sy = (int) (win_h - win_h * ellip[no].y1);
			    	xs = (int) (win_w * ellip[no].x2);
			    	ys = (int) (win_h - win_h * ellip[no].y2);
				} else {
			    	_world2device(ellip[no].x1, ellip[no].y1, &sx, &sy);
			    	_world2device(ellip[no].x2, ellip[no].y2, &xs, &ys);
				}
				select_region(sx, sy, xs, ys);
				mbox_flag = 1;
				break;
		    case OBJECT_LINE:
				if (lines[no].loctype == COORD_VIEW) {
			    	sx = (int) (win_w * lines[no].x1);
			    	sy = (int) (win_h - win_h * lines[no].y1);
			    	xs = (int) (win_w * lines[no].x2);
			    	ys = (int) (win_h - win_h * lines[no].y2);
				} else {
			    	_world2device(lines[no].x1, lines[no].y1, &sx, &sy);
			    	_world2device(lines[no].x2, lines[no].y2, &xs, &ys);
				}
				select_line(sx, sy, xs, ys);
				mline_flag = 1;
				break;
		    case OBJECT_STRING:
				xs = _stringextentx(getcharsize(), pstr[no].s);
				ys = _stringextenty(getcharsize(), pstr[no].s);
				if (pstr[no].loctype == COORD_VIEW) {
			    	sx = (int) (win_w * pstr[no].x);
			    	sy = (int) (win_h - win_h * pstr[no].y);
				} else {
			    	_world2device(pstr[no].x, pstr[no].y, &sx, &sy);
				}
 				xs = sx + xs;
				ys = sy + ys;
				mbox_flag = 1;
				select_region(sx, sy, xs, ys);
				break;
		    }
		}

		break;
/*
 * box has been selected and new position found
 */
	    case MOVE_OBJECT_2ND:
		dx = sx - x;
		dy = sy - y;

		set_action(0);
		sx = x;
		sy = y;
		xs = xs - dx;
		ys = ys - dy;
		_device2world(sx, sy, &wx1, &wy1);
		_device2world(xs, ys, &wx2, &wy2);
		switch (ty) {
		case OBJECT_BOX:
		    if (boxes[no].loctype == COORD_VIEW) {
			wx1 = xy_xconv(wx1);
			wy1 = xy_yconv(wy1);
			wx2 = xy_xconv(wx2);
			wy2 = xy_yconv(wy2);
		    } else {
/*
 * 			boxes[no].gno = get_cg();
 */
		    }
		    boxes[no].x1 = wx1;
		    boxes[no].x2 = wx2;
		    boxes[no].y1 = wy1;
		    boxes[no].y2 = wy2;
		    break;
		case OBJECT_ELLIPSE:
		    if (ellip[no].loctype == COORD_VIEW) {
			wx1 = xy_xconv(wx1);
			wy1 = xy_yconv(wy1);
			wx2 = xy_xconv(wx2);
			wy2 = xy_yconv(wy2);
		    } else {
/*
 * 			ellip[no].gno = get_cg();
 */
		    }
		    break;
		case OBJECT_LINE:
		    if (lines[no].loctype == COORD_VIEW) {
			wx1 = xy_xconv(wx1);
			wy1 = xy_yconv(wy1);
			wx2 = xy_xconv(wx2);
			wy2 = xy_yconv(wy2);
		    } else {
/*
 * 			lines[no].gno = get_cg();
 */
		    }
		    lines[no].x1 = wx1;
		    lines[no].x2 = wx2;
		    lines[no].y1 = wy1;
		    lines[no].y2 = wy2;
		    break;
		case OBJECT_STRING:
		    wy = (double)_stringextenty(getcharsize(), pstr[no].s)/
														(double)win_h;
		    if (pstr[no].loctype == COORD_VIEW) {
				wx1 = xy_xconv(wx1);
				wy1 = xy_yconv(wy1) - wy/2.;
		    } else {
				world2view( wx1, wy1, &wx2, &wy2 );
				wy2 -= wy/2.;
				view2world( wx2, wy2, &wx1, &wy1 );
		    }
		    pstr[no].x = wx1;
		    pstr[no].y = wy1;
		    break;
		}
		set_action(MOVE_OBJECT_1ST);
		break;
/*
 * select a box, ellipse or a line to copy
 */
	    case COPY_OBJECT1ST:
		set_action(COPY_OBJECT2ND);
		_device2world(x, y, &wx, &wy);
		find_item(get_cg(), wx, wy, &ty, &no);
		if (ty < 0) {
/*
 * 		    set_action(0);
 */
		set_action(COPY_OBJECT1ST);
		} else {
		    switch (ty) {
		    case OBJECT_BOX:
			if (boxes[no].loctype == COORD_VIEW) {
			    sx = (int) (win_w * boxes[no].x1);
			    sy = (int) (win_h - win_h * boxes[no].y1);
			    xs = (int) (win_w * boxes[no].x2);
			    ys = (int) (win_h - win_h * boxes[no].y2);
			} else {
			    _world2device(boxes[no].x1, boxes[no].y1, &sx, &sy);
			    _world2device(boxes[no].x2, boxes[no].y2, &xs, &ys);
			}
			select_region(sx, sy, xs, ys);
			mbox_flag = 1;
			break;
		    case OBJECT_ELLIPSE:
			if (ellip[no].loctype == COORD_VIEW) {
			    sx = (int) (win_w * ellip[no].x1);
			    sy = (int) (win_h - win_h * ellip[no].y1);
			    xs = (int) (win_w * ellip[no].x2);
			    ys = (int) (win_h - win_h * ellip[no].y2);
			} else {
			    _world2device(ellip[no].x1, ellip[no].y1, &sx, &sy);
			    _world2device(ellip[no].x2, ellip[no].y2, &xs, &ys);
			}
			select_region(sx, sy, xs, ys);
			mbox_flag = 1;
			break;
		    case OBJECT_LINE:
			if (lines[no].loctype == COORD_VIEW) {
			    sx = (int) (win_w * lines[no].x1);
			    sy = (int) (win_h - win_h * lines[no].y1);
			    xs = (int) (win_w * lines[no].x2);
			    ys = (int) (win_h - win_h * lines[no].y2);
			} else {
			    _world2device(lines[no].x1, lines[no].y1, &sx, &sy);
			    _world2device(lines[no].x2, lines[no].y2, &xs, &ys);
			}
			select_line(sx, sy, xs, ys);
			mline_flag = 1;
			break;
		    case OBJECT_STRING:
			xs = _stringextentx(getcharsize(), pstr[no].s);
			ys = _stringextenty(getcharsize(), pstr[no].s);
			if (pstr[no].loctype == COORD_VIEW) {
			    sx = (int) (win_w * pstr[no].x);
			    sy = (int) (win_h - win_h * pstr[no].y);
			} else {
			    _world2device(pstr[no].x, pstr[no].y, &sx, &sy);
			}
			xs = sx + xs;
			ys = sy + ys;
			mbox_flag = 1;
			select_region(sx, sy, xs, ys);
			break;
		    }
		}

		break;
/*
 * box has been selected and new position found
 */
	    case COPY_OBJECT2ND:
		dx = sx - x;
		dy = sy - y;

		set_action(0);
		sx = x;
		sy = y;
		xs = xs - dx;
		ys = ys - dy;
		_device2world(sx, sy, &wx1, &wy1);
		_device2world(xs, ys, &wx2, &wy2);
		switch (ty) {
		case OBJECT_BOX:
		    if ((boxno = next_box()) >= 0) {
			copy_object(ty, no, boxno);
			if (boxes[no].loctype == COORD_VIEW) {
			    wx1 = xy_xconv(wx1);
			    wy1 = xy_yconv(wy1);
			    wx2 = xy_xconv(wx2);
			    wy2 = xy_yconv(wy2);
			} else {
			    boxes[boxno].gno = get_cg();
			}
			boxes[boxno].x1 = wx1;
			boxes[boxno].x2 = wx2;
			boxes[boxno].y1 = wy1;
			boxes[boxno].y2 = wy2;
		    }
		    break;
		case OBJECT_ELLIPSE:
		    if ((ellipno = next_ellipse()) >= 0) {
			copy_object(ty, no, ellipno);
			if (ellip[no].loctype == COORD_VIEW) {
			    wx1 = xy_xconv(wx1);
			    wy1 = xy_yconv(wy1);
			    wx2 = xy_xconv(wx2);
			    wy2 = xy_yconv(wy2);
			} else {
			    ellip[ellipno].gno = get_cg();
			}
			ellip[ellipno].x1 = wx1;
			ellip[ellipno].x2 = wx2;
			ellip[ellipno].y1 = wy1;
			ellip[ellipno].y2 = wy2;
		    }
		    break;
		case OBJECT_LINE:
		    if ((lineno = next_line()) >= 0) {
			copy_object(ty, no, lineno);
			if (lines[no].loctype == COORD_VIEW) {
			    wx1 = xy_xconv(wx1);
			    wy1 = xy_yconv(wy1);
			    wx2 = xy_xconv(wx2);
			    wy2 = xy_yconv(wy2);
			} else {
			    lines[lineno].gno = get_cg();
			}
			lines[lineno].x1 = wx1;
			lines[lineno].x2 = wx2;
			lines[lineno].y1 = wy1;
			lines[lineno].y2 = wy2;
		    }
		    break;
		case OBJECT_STRING:
		    if ((stringno = next_string()) >= 0) {
			copy_object(ty, no, stringno);
			if (pstr[no].loctype == COORD_VIEW) {
			    wx1 = xy_xconv(wx1);
			    wy1 = xy_yconv(wy1);
			} else {
			    pstr[stringno].gno = get_cg();
			}
			pstr[stringno].x = wx1;
			pstr[stringno].y = wy1;
		    }
		    break;
		}
		set_action(COPY_OBJECT1ST);
		break;
/*
 * select a box or a line to move
 */
	    case EDIT_OBJECT:
		set_action(EDIT_OBJECT);
		_device2world(x, y, &wx, &wy);
		find_item(get_cg(), wx, wy, &ty, &no);
		if (ty < 0) {
/*
 * 		    set_action(0);
 */
		} else {
		    switch (ty) {
		    case OBJECT_BOX:
			box_edit_popup(no);
			break;
		    case OBJECT_ELLIPSE:
			ellipse_edit_popup(no);
			break;
		    case OBJECT_LINE:
			line_edit_popup(no);
			break;
		    case OBJECT_STRING:
			string_edit_popup(no);
			break;
		    }
		}

		break;
/*
 * make a new box, select first corner
 */
	    case MAKE_BOX_1ST:
		set_action(MAKE_BOX_2ND);
		rectflag = 1;
		sx = x;
		sy = y;
		select_region(sx, sy, x, y);
		break;
/*
 * make a new box, select opposite corner
 */
	    case MAKE_BOX_2ND:
		set_action(0);
		if ((boxno = next_box()) >= 0) {
		    _device2world(sx, sy, &wx1, &wy1);
		    _device2world(x, y, &wx2, &wy2);
		    if (box_loctype == COORD_VIEW) {
			wx1 = xy_xconv(wx1);
			wy1 = xy_yconv(wy1);
			wx2 = xy_xconv(wx2);
			wy2 = xy_yconv(wy2);
		    } else {
			boxes[boxno].gno = get_cg();
		    }
		    boxes[boxno].loctype = box_loctype;
		    boxes[boxno].x1 = wx1;
		    boxes[boxno].x2 = wx2;
		    boxes[boxno].y1 = wy1;
		    boxes[boxno].y2 = wy2;
		    boxes[boxno].color = box_color;
		    boxes[boxno].linew = box_linew;
		    boxes[boxno].lines = box_lines;
		    boxes[boxno].fillcolor = box_fillcolor;
		    boxes[boxno].fillpattern = box_fillpat;
		    set_action(MAKE_BOX_1ST);
		}
		break;
/*
 * make an ellipse, select first corner
 */
	    case MAKE_ELLIP_1ST:
		set_action(MAKE_ELLIP_2ND);
		rectflag = 1;
		sx = x;
		sy = y;
		select_region(sx, sy, x, y);
		break;
/*
 * make a new ellipse, select opposite corner
 */
	    case MAKE_ELLIP_2ND:
		set_action(0);
		if ((ellipno = next_ellipse()) >= 0) {
		    _device2world(sx, sy, &wx1, &wy1);
		    _device2world(x, y, &wx2, &wy2);
 		    ellip[ellipno] = defellip;
		    if (ellip[ellipno].loctype == COORD_VIEW) {
				wx1 = xy_xconv(wx1);
				wy1 = xy_yconv(wy1);
				wx2 = xy_xconv(wx2);
				wy2 = xy_yconv(wy2);
		    } else {
				ellip[ellipno].gno = get_cg();
		    }
		    ellip[ellipno].x1 = wx1;
		    ellip[ellipno].x2 = wx2;
		    ellip[ellipno].y1 = wy1;
		    ellip[ellipno].y2 = wy2;
                    
                    set_action(MAKE_ELLIP_1ST);
		}
		break;
/*
 * locate angled string
 */
	    case STR_LOC1ST:
		set_action(STR_LOC2ND);
		rubber_flag = 1;
		sx = x;
		sy = y;
		select_line(sx, sy, x, y);
		break;
	    case STR_LOC2ND:
		_device2world(sx, sy, &wx1, &wy1);
		_device2world(x, y, &wx2, &wy2);
		wx1 = xy_xconv(wx1);
		wy1 = xy_yconv(wy1);
		wx2 = xy_xconv(wx2);
		wy2 = xy_yconv(wy2);
		string_rot = (int) ((atan2((wy2 - wy1) * win_h, (wx2 - wx1) * win_w) * 180.0 / M_PI) + 360.0) % 360;
		updatestrings();
		set_action(0);
		set_action(STR_LOC);
		break;
/*
 * make a new line, select start point
 */
	    case MAKE_LINE_1ST:
		sx = x;
		sy = y;
		set_action(MAKE_LINE_2ND);
		rubber_flag = 1;
		select_line(sx, sy, x, y);
		break;
/*
 * make a new line, select end point
 */
	    case MAKE_LINE_2ND:
		set_action(0);
		if ((lineno = next_line()) >= 0) {
		    _device2world(sx, sy, &wx1, &wy1);
		    _device2world(x, y, &wx2, &wy2);
		    if (line_loctype == COORD_VIEW) {
			wx1 = xy_xconv(wx1);
			wy1 = xy_yconv(wy1);
			wx2 = xy_xconv(wx2);
			wy2 = xy_yconv(wy2);
		    } else {
			lines[lineno].gno = get_cg();
		    }
		    lines[lineno].loctype = line_loctype;
		    lines[lineno].x1 = wx1;
		    lines[lineno].x2 = wx2;
		    lines[lineno].y1 = wy1;
		    lines[lineno].y2 = wy2;
		    lines[lineno].color = line_color;
		    lines[lineno].lines = line_lines;
		    lines[lineno].linew = line_linew;
		    lines[lineno].arrow = line_arrow;
		    lines[lineno].asize = line_asize;
		    lines[lineno].atype = line_atype;
		    set_action(MAKE_LINE_1ST);
		}
		break;
/*
 * Edit an existing string
 */
	    case STR_EDIT:
		_device2world(x, y, &wx, &wy);
		find_item(get_cg(), wx, wy, &ty, &no);
		if ((ty >= 0) && (ty == OBJECT_STRING)) {
		    int ilenx, ileny;

		    wx1 = pstr[no].x;
		    wy1 = pstr[no].y;
		    if (pstr[no].loctype == COORD_VIEW) {	/* in viewport coords */
			view2world(wx1, wy1, &wx2, &wy2);
			wx1 = wx2;
			wy1 = wy2;
		    }
		    _world2device(wx1, wy1, &strx, &stry);
		    drawx = strx;
		    drawy = stry;
		    tmpstrins = strcpy(tmpstr, pstr[no].s) + strlen(pstr[no].s);
		    setcharsize(pstr[no].charsize);
		    setfont(pstr[no].font);
		    setcolor(pstr[no].color);
		    string_just = pstr[no].just;
		    justflag = string_just;
		    string_size = pstr[no].charsize;
		    string_font = pstr[no].font;
		    string_color = pstr[no].color;
		    string_rot = pstr[no].rot;
		    string_loctype = pstr[no].loctype;
		    updatestrings();
		    kill_string(no);
		    si = sin(M_PI / 180.0 * string_rot) *
			((double) win_w) / ((double) win_h);
		    co = cos(M_PI / 180.0 * string_rot);

		    ilenx = _stringextentx(getcharsize(), tmpstr);
		    ileny = _stringextenty(getcharsize(), tmpstr);

		    switch (justflag) {
		    case 1:
/*
			strx = drawx + co * ilenx - si * ileny;
			stry = drawy + si * ilenx + co * ileny;
*/
			break;
		    case 2:
/*
			strx = drawx + (co * ilenx - si * ileny) / 2;
			stry = drawy + (si * ilenx + co * ileny) / 2;
*/
			break;
		    }
		    update_text_cursor(tmpstr, drawx, drawy, tmpstrins);
		    do_text_string(2, 0);
		    action_flag = STR_LOC;
		} else {
		    set_action(0);
		}
		break;
/*
 * locate a string on the canvas
 */
	    case STR_LOC:
		if (tmpstr[0]) {
		    _device2world(strx, win_h - stry, &wx, &wy);
		    define_string(tmpstr, wx, wy);
		}
		strx = x;
		stry = win_h - y;
		drawx = strx;
		drawy = stry;
		tmpstr[0] = 0;
		tmpstrins = tmpstr;
		define_string_defaults(NULL, NULL, NULL);
		justflag = string_just;
		setcharsize(string_size);
		setfont(string_font);
		setcolor(string_color);
		setlinewidth(string_linew);
		si = sin(M_PI / 180.0 * string_rot) *
		    ((double) win_w) / ((double) win_h);
		co = cos(M_PI / 180.0 * string_rot);
		update_text_cursor(tmpstr, strx, stry, tmpstrins);
		break;
/*
 * Kill the set nearest the pointer
 */
	    case KILL_NEAREST:
		{
		    int setno, loc;

		    _device2world(x, y, &wx, &wy);
		    findpoint(get_cg(), wx, wy, &wx, &wy, &setno, &loc);
		    if (setno != -1) {
			if (verify_action) {
			    char tmpbuf[128];

			    sprintf(tmpbuf, "Kill S%1d?", setno);
			    if (yesno(tmpbuf, NULL, NULL, NULL)) {
				do_kill(get_cg(), setno, 0);
				xdrawgraph();
			    }
			} else {
			    do_kill(get_cg(), setno, 0);
			    xdrawgraph();
			}
			set_action(KILL_NEAREST);
		    } else {
			errwin("Found no set, cancelling kill");
			set_action(0);
		    }
		}
		break;
/*
 * Deactivate the set nearest the pointer
 */
	    case DEACTIVATE_NEAREST:
		{
		    int setno, loc;

		    _device2world(x, y, &wx, &wy);
		    findpoint(get_cg(), wx, wy, &wx, &wy, &setno, &loc);
		    if (setno != -1) {
			if (verify_action) {
			    char tmpbuf[128];

			    sprintf(tmpbuf, "Deactivate S%1d?", setno);
			    if (yesno(tmpbuf, NULL, NULL, NULL)) {
				do_hideset(get_cg(), setno);
				xdrawgraph();
			    }
			} else {
			    do_hideset(get_cg(), setno);
			    xdrawgraph();
			}
			set_action(DEACTIVATE_NEAREST);
		    } else {
			errwin("Found no set, cancelling deactivate");
			set_action(0);
		    }
		}
		break;
/*
 * Copy the set nearest the pointer
 */
	    case COPY_NEAREST1ST:
		{
		    _device2world(x, y, &wx, &wy);
		    findpoint(get_cg(), wx, wy, &wx, &wy, &setno1, &loc1);
		    graphno1 = get_cg();	/* in case focus changes */
		    if (setno1 != -1) {
			set_action(COPY_NEAREST2ND);
		    } else {
			errwin("Found no set, cancelling copy");
			set_action(0);
		    }
		}
		break;
	    case COPY_NEAREST2ND:
		{
		    _device2world(x, y, &wx, &wy);
		    graphno2 = iscontained(get_cg(), wx, wy);
		    if (graphno2 != -1) {
			if (verify_action) {
			    char tmpbuf[128];

			    sprintf(tmpbuf, "Copy S%1d in graph %d to next set in graph %d?", setno1, graphno1, graphno2);
			    if (yesno(tmpbuf, NULL, NULL, NULL)) {
				do_copy(setno1, graphno1, SET_SELECT_NEXT, graphno2 + 1);
			    }
			} else {
			    do_copy(setno1, graphno1, SET_SELECT_NEXT, graphno2 + 1);
			}
			set_action(COPY_NEAREST1ST);
		    } else {
			errwin("Found no graph, cancelling copy");
			set_action(0);
		    }
		}
		break;
/*
 * Move the set nearest the pointer
 */
	    case MOVE_NEAREST1ST:
		{
		    _device2world(x, y, &wx, &wy);
		    findpoint(get_cg(), wx, wy, &wx, &wy, &setno1, &loc1);
		    graphno1 = get_cg();	/* in case focus changes */
		    if (setno1 != -1) {
			set_action(MOVE_NEAREST2ND);
		    } else {
			errwin("Found no set, cancelling move");
			set_action(0);
		    }
		}
		break;
	    case MOVE_NEAREST2ND:
		{
		    _device2world(x, y, &wx, &wy);
		    graphno2 = iscontained(get_cg(), wx, wy);
		    if (graphno2 != -1 && graphno2+1 != graphno1 ) {
				if (verify_action) {
			   	 	char tmpbuf[128];
			   		sprintf(tmpbuf, "Move S%1d in graph %d to next set in graph %d?", setno1, graphno1, graphno2);
			    	if (yesno(tmpbuf, NULL, NULL, NULL)) {
						do_move(setno1, graphno1, SET_SELECT_NEXT, graphno2+1);
			   		}
				} else {
			    	do_move(setno1, graphno1, SET_SELECT_NEXT, graphno2 + 1);
				}
				set_action(MOVE_NEAREST1ST);
		    } else if( graphno2 == -1 ) {
				errwin("Found no graph, cancelling move");
				set_action(0);
		    }
		}
		break;
/*
 * Break a set at a point
 */
	    case PICK_BREAK:
		{
		    int setno, loc;

		    _device2world(x, y, &wx, &wy);
		    findpoint(get_cg(), wx, wy, &wx, &wy, &setno, &loc);
		    if (setno != -1) {
			if (verify_action) {
			    char tmpbuf[128];

			    sprintf(tmpbuf, "Reverse S%1d?", setno);
			    if (yesno(tmpbuf, NULL, NULL, NULL)) {
				do_breakset(get_cg(), setno, loc);
			    }
			} else {
			    do_breakset(get_cg(), setno, loc);
			}
		    } else {
			errwin("Found no set, cancelling break");
		    }
		    set_action(0);
		}
		break;
/*
 * Reverse the set nearest the pointer
 */
	    case REVERSE_NEAREST:
		{
		    int setno, loc;

		    _device2world(x, y, &wx, &wy);
		    findpoint(get_cg(), wx, wy, &wx, &wy, &setno, &loc);
		    if (setno != -1) {
			if (verify_action) {
			    char tmpbuf[128];

			    sprintf(tmpbuf, "Reverse S%1d?", setno);
			    if (yesno(tmpbuf, NULL, NULL, NULL)) {
				do_reverse_sets(setno);
				xdrawgraph();
			    }
			} else {
			    do_reverse_sets(setno);
			    xdrawgraph();
			}
		    } else {
			errwin("Found no set, cancelling reverse");
		    }
		    set_action(0);
		}
		break;
/*
 * Join two sets
 */
	    case JOIN_NEAREST1ST:
		{
		    _device2world(x, y, &wx, &wy);
		    findpoint(get_cg(), wx, wy, &wx, &wy, &setno1, &loc1);
		    graphno1 = get_cg();	/* in case focus changes */
		    if (setno1 != -1) {
			set_action(JOIN_NEAREST2ND);
		    } else {
			errwin("Found no set, cancelling join");
			set_action(0);
		    }
		}
		break;
	    case JOIN_NEAREST2ND:
		{
		    int setno, loc;

		    _device2world(x, y, &wx, &wy);
		    graphno2 = iscontained(get_cg(), wx, wy);
		    findpoint(graphno2, wx, wy, &wx, &wy, &setno, &loc);
		    if (setno1 == setno && graphno1 == graphno2) {
			errwin("Can't join the same set");
		    } else {
			if (verify_action) {
			    char tmpbuf[128];

			    sprintf(tmpbuf, "Join S%1d in graph %d to the end of S%1d in graph %d?", setno1, graphno1, setno, graphno2);
			    if (yesno(tmpbuf, NULL, NULL, NULL)) {
				do_join_sets(graphno1, setno1, graphno2, setno);
				xdrawgraph();
			    }
			} else {
			    do_join_sets(graphno1, setno1, graphno2, setno);
			    xdrawgraph();
			}
		    }
		    set_action(JOIN_NEAREST1ST);
		}
		break;
/*
 * Delete range in a set
 */
	    case DELETE_NEAREST1ST:
		{
		    _device2world(x, y, &wx, &wy);
		    findpoint(get_cg(), wx, wy, &wx, &wy, &setno1, &loc1);
		    graphno1 = get_cg();	/* in case focus changes */
		    if (setno1 != -1) {
			set_action(DELETE_NEAREST2ND);
		    } else {
			errwin("Found no set, cancelling delete");
			set_action(0);
		    }
		}
		break;
	    case DELETE_NEAREST2ND:
		{
		    _device2world(x, y, &wx, &wy);
		    graphno2 = iscontained(get_cg(), wx, wy);
		    if (graphno1 != graphno2) {
			errwin("Can't perform operation across 2 graphs");
			set_action(0);
		    } else if (graphno2 != -1) {
			findpoint(graphno2, wx, wy, &wx, &wy, &setno2, &loc2);
			if (graphno1 != graphno2) {
			    errwin("Points found are not in the same graph");
			    set_action(0);
			} else if (setno1 != setno2) {
			    errwin("Points found are not in the same set");
			    set_action(0);
			} else {
			    if (loc2 < loc1) {
				iswap(&loc1, &loc2);
			    }
			    if (verify_action) {
				char tmpbuf[128];

				sprintf(tmpbuf, "In S%1d, delete points %d through %d?", setno1, loc1, loc2);
				if (yesno(tmpbuf, NULL, NULL, NULL)) {
				    do_drop_points(setno1, loc1 - 1, loc2 - 1);
				}
			    } else {
				do_drop_points(setno1, loc1 - 1, loc2 - 1);
			    }
			    set_action(DELETE_NEAREST1ST);
			}
		    } else {
			errwin("Found no graph, cancelling delete");
			set_action(0);
		    }
		}
		break;
	    case AUTO_NEAREST:
		{
		    _device2world(x, y, &wx, &wy);
		    findpoint(get_cg(), wx, wy, &wx, &wy, &setno1, &loc1);
		    if (setno1 != -1) {
			do_autoscale_set(get_cg(), setno1);
		    } else {
			errwin("Found no set, cancelling autoscale");
		    }
		    set_action(0);
		}
		break;
/*
 * find a point in a set
 */
	    case FIND_POINT:
		{
		    int setno, loc;

		    _device2world(x, y, &wx, &wy);
		    findpoint(get_cg(), wx, wy, &wx, &wy, &setno, &loc);
		    if (setno != -1) {
			sprintf(buf, "Set %d, loc %d, (%f, %f)", setno, loc, wx, wy);
			xv_setstr(locate_point_item, buf);
			set_action(FIND_POINT);
		    } else {
			errwin("Found no set, cancelling find");
			set_action(0);
		    }
		}
		break;
/*
 * Tracker
 */
	    case TRACKER:
		{
		    int xtmp, ytmp;
		    double *xx, *yy;

		    if (track_set == SET_SELECT_NEAREST) {	
			_device2world(x, y, &wx, &wy);
			findpoint(get_cg(), wx, wy, &wx, &wy, &track_set, &track_point);
		    } else {	/* set selected, find nearest point */
			if (track_point == -1) {
			    _device2world(x, y, &wx, &wy);
			    findpoint_inset(get_cg(), track_set, wx, wy, &track_point);
			}
		    }
		    track_point--;
		    if (track_point < 0) {
			track_point = getsetlength(get_cg(), track_set) - 1;
		    } else if (track_point >= getsetlength(get_cg(), track_set)) {
			track_point = 0;
		    }
		    xx = getx(get_cg(), track_set);
		    yy = gety(get_cg(), track_set);
			if (inbounds(get_cg(), xx[track_point], yy[track_point])) {
			    _world2device(xx[track_point], yy[track_point], &xtmp, &ytmp);
			    setpointer(xtmp, ytmp);
			    sprintf(buf, "Set %d, loc %d, (%f, %f)", track_set, track_point + 1,
				    xx[track_point], yy[track_point]);
			} else {
			    sprintf(buf, "OUTSIDE - Set %d, loc %d, (%f, %f)", track_set, track_point + 1,
				    xx[track_point], yy[track_point]);
			}
			xv_setstr(locate_point_item, buf);
		    set_action(TRACKER);
		}
		break;
/*
 * define a polygonal region
 */
	    case DEF_REGION:
		_device2world(x, y, &area_polyx[region_pts], &area_polyy[region_pts]);
		iax[region_pts] = x;
		iay[region_pts] = y;
		region_pts++;
		rubber_flag = 1;
		sx = x;
		sy = y;
		select_line(sx, sy, x, y);
		set_action(DEF_REGION);
		break;
/*
 * define a region by a line, type left, right, above, below
 */
	    case DEF_REGION1ST:
		sx = x;
		sy = y;
		set_action(MAKE_LINE_2ND);
		rubber_flag = 1;
		select_line(sx, sy, x, y);
		set_action(DEF_REGION2ND);
		break;
	    case DEF_REGION2ND:
		set_action(0);
		_device2world(sx, sy, &wx1, &wy1);
		_device2world(x, y, &wx2, &wy2);
		rg[nr].active = TRUE;
		rg[nr].type = regiontype;
		if (regionlinkto) {
		    int i;

		    for (i = 0; i < number_of_graphs() ; i++) {
			rg[nr].linkto[i] = TRUE;
		    }
		} else {
		    int i;

		    for (i = 0; i < number_of_graphs() ; i++) {
			rg[nr].linkto[i] = FALSE;
		    }
		    rg[nr].linkto[get_cg()] = TRUE;
		}
		rg[nr].type = regiontype;
		rg[nr].x1 = wx1;
		rg[nr].x2 = wx2;
		rg[nr].y1 = wy1;
		rg[nr].y2 = wy2;
		draw_region(nr);
		break;

/*
 * Compute the area of a polygon
 */
	    case COMP_AREA:
		{
		    double area;

		    _device2world(x, y, &area_polyx[narea_pts], &area_polyy[narea_pts]);
		    iax[narea_pts] = x;
		    iay[narea_pts] = y;
		    narea_pts++;
		    if (narea_pts <= 2) {
			area = 0.0;
		    } else {
			area = comp_area(narea_pts, area_polyx, area_polyy);
		    }
		    sprintf(buf, "[%f]", fabs(area));
		    XmStringFree(astring);
		    astring = XmStringCreateLtoR(buf, charset);
		    XtSetArg(al, XmNlabelString, astring);
		    XtSetValues(arealab, &al, 1);
		    rubber_flag = 1;
		    sx = x;
		    sy = y;
		    select_line(sx, sy, x, y);
		    set_action(COMP_AREA);
		}
		break;
	    case COMP_PERIMETER:
		{
		    double area;

		    _device2world(x, y, &area_polyx[narea_pts], &area_polyy[narea_pts]);
		    iax[narea_pts] = x;
		    iay[narea_pts] = y;
		    narea_pts++;
		    if (narea_pts <= 1) {
			area = 0.0;
		    } else {
			area = comp_perimeter(narea_pts, area_polyx, area_polyy);
		    }
		    sprintf(buf, "[%f]", fabs(area));

		    XmStringFree(pstring);
		    pstring = XmStringCreateLtoR(buf, charset);
		    XtSetArg(al, XmNlabelString, pstring);
		    XtSetValues(perimlab, &al, 1);
		    rubber_flag = 1;
		    sx = x;
		    sy = y;
		    select_line(sx, sy, x, y);
		    set_action(COMP_PERIMETER);
		}
		break;
/*
 * select a reference point for the locator in main_panel
 */
	    case SEL_POINT:
		_device2world(x, y, &wx, &wy);
		g[get_cg()].locator.pointset = TRUE;
		g[get_cg()].locator.dsx = wx;
		g[get_cg()].locator.dsy = wy;
		draw_ref_point(get_cg());
		update_locator_items(get_cg());
		set_action(0);
		break;
/*
 * delete a point in a set
 */
	    case DEL_POINT:
		{
		    int setno, loc;

		    _device2world(x, y, &wx, &wy);
			if( track_set==SET_SELECT_NEAREST )
		    	findpoint(get_cg(), wx, wy, &wx, &wy, &setno, &loc);
			else {
				findpoint_inset( get_cg(), track_set, wx, wy, &loc );
				get_point( get_cg(), track_set, loc, &wx, &wy );
				setno = track_set;
			}
		    if (setno == -1) {
			sprintf(buf, "No sets found");
			xv_setstr(locate_point_item, buf);
			set_action(0);
		    } else {
			sprintf(buf, "Set %d, loc %d, (%f, %f)", setno, loc, wx, wy);
			xv_setstr(locate_point_item, buf);
			if (setno >= 0) {
			    del_point(get_cg(), setno, loc);
			    update_set_status(get_cg(), setno);
			}
			set_action(DEL_POINT);
		    }
		}
		break;
/*
 * move a point in a set
 */
	    case MOVE_POINT1ST:
		_device2world(x, y, &wx, &wy);
		if( track_set==SET_SELECT_NEAREST )
		    findpoint(get_cg(), wx, wy, &wx, &wy, &setnumber, &setindex);
		else {
			findpoint_inset( get_cg(), track_set, wx, wy, &setindex );
			get_point( get_cg(), track_set, setindex, &wx, &wy );
			setnumber = track_set;
		}
		sprintf(buf, "Set %d, loc %d, (%14g, %14g)", setnumber, setindex, wx, wy);
		xv_setstr(locate_point_item, buf);
		if (setnumber >= 0) {
		    _world2device(wx, wy, &sx, &sy);
		    rubber_flag = 1;
		    select_line(sx, sy, sx, sy);
		    set_action(MOVE_POINT2ND);
		} else {
		    set_action(0);
		}
		break;
	    case MOVE_POINT2ND:
		_device2world(x, y, &wx, &wy);
		get_point(get_cg(), setnumber, setindex - 1, &wx1, &wy1);
		switch (move_dir) {
		case 0:
		    set_point(get_cg(), setnumber, setindex - 1, wx, wy);
		    break;
		case 1:
		    set_point(get_cg(), setnumber, setindex - 1, wx, wy1);
		    break;
		case 2:
		    set_point(get_cg(), setnumber, setindex - 1, wx1, wy);
		    break;
		}
		sprintf(buf, "Set %d, loc %d, (%14g, %14g)", setnumber, setindex, wx, wy);
		xv_setstr(locate_point_item, buf);
		update_set_status(get_cg(), setnumber);
		set_action(0);
		set_action(MOVE_POINT1ST);
		xdrawgraph();
		break;
/*
 * add a point to a set
 */
	    case ADD_POINT:
		{
		    int ind;

		    _device2world(x, y, &wx, &wy);
		    if (add_setno >= 0) {
			switch (add_at) {
			case 0:/* at end */
			    ind = getsetlength(get_cg(), add_setno);
			    add_point(get_cg(), add_setno, wx, wy, 0.0, 0.0, SET_XY);
			    sprintf(buf, "Set %d, loc %d, (%f, %f)", add_setno, ind + 1, wx, wy);
			    break;
			case 1:/* at beginning */
			    ind = 1;
			    add_point_at(get_cg(), add_setno, 0, FALSE, wx, wy, 0.0, 0.0, SET_XY);
			    sprintf(buf, "Set %d, loc %d, (%f, %f)", add_setno, ind + 1, wx, wy);
			    break;
			case 2:/* after nearest point */
			    findpoint_inset(get_cg(), add_setno, wx, wy, &ind);
			    if (ind >= 1) {
				add_point_at(get_cg(), add_setno, ind - 1, TRUE, wx, wy, 0.0, 0.0, SET_XY);
				sprintf(buf, "Added to Set %d, after loc %d, (%f, %f)",
					add_setno, ind, wx, wy);
			    }
			    break;
			case 3:/* before nearest point */
			    findpoint_inset(get_cg(), add_setno, wx, wy, &ind);
			    if (ind >= 1) {
				add_point_at(get_cg(), add_setno, ind - 1, FALSE, wx, wy, 0.0, 0.0, SET_XY);
				sprintf(buf, "Added to Set %d, before loc %d, (%f, %f)",
					add_setno, ind, wx, wy);
			    }
			    break;
			}
			xv_setstr(locate_point_item, buf);
			XDrawLine(disp, xwin, gc, x - 5, y - 5, x + 5, y + 5);
			XDrawLine(disp, xwin, gc, x - 5, y + 5, x + 5, y - 5);
			update_set_status(get_cg(), add_setno);
			set_action(ADD_POINT);
		    } else {
			set_action(0);
		    }
		}
		break;
	    case ADD_POINT1ST:
		{
		    int ind;

		    _device2world(x, y, &wx, &wy);
		    if (digit_setno >= 0) {
			ind = getsetlength(get_cg(), digit_setno);
			add_point(get_cg(), digit_setno, wx, wy, 0.0, 0.0, SET_XY);
			sprintf(buf, "Set %d, loc %d, (%f, %f)", digit_setno, ind + 1, wx, wy);
			xv_setstr(locate_point_item, buf);
			XDrawLine(disp, xwin, gc, x - 5, y - 5, x + 5, y + 5);
			XDrawLine(disp, xwin, gc, x - 5, y + 5, x + 5, y - 5);
			update_set_status(get_cg(), digit_setno);
			set_action(ADD_POINT2ND);
		    } else {
			set_action(0);
		    }
		}
		break;
	    case ADD_POINT2ND:
		{
		    int ind;

		    _device2world(x, y, &wx, &wy);
		    if (digit_setno >= 0) {
			ind = getsetlength(get_cg(), digit_setno);
			add_point(get_cg(), digit_setno, wx, wy, 0.0, 0.0, SET_XY);
			sprintf(buf, "Set %d, loc %d, (%f, %f)", digit_setno, ind + 1, wx, wy);
			xv_setstr(locate_point_item, buf);
			XDrawLine(disp, xwin, gc, x - 5, y - 5, x + 5, y + 5);
			XDrawLine(disp, xwin, gc, x - 5, y + 5, x + 5, y - 5);
			update_set_status(get_cg(), digit_setno);
			set_action(ADD_POINT3RD);
		    } else {
			set_action(0);
		    }
		}
		break;
	    case ADD_POINT3RD:
		{
		    int ind;

		    _device2world(x, y, &wx, &wy);
		    if (digit_setno >= 0) {
			ind = getsetlength(get_cg(), digit_setno);
			add_point(get_cg(), digit_setno, wx, wy, 0.0, 0.0, SET_XY);
			sprintf(buf, "Set %d, loc %d, (%f, %f)", digit_setno, ind + 1, wx, wy);
			xv_setstr(locate_point_item, buf);
			XDrawLine(disp, xwin, gc, x - 5, y - 5, x + 5, y + 5);
			XDrawLine(disp, xwin, gc, x - 5, y + 5, x + 5, y - 5);
			update_set_status(get_cg(), digit_setno);
			add_setno = digit_setno;
			add_at = 0;
			set_action(ADD_POINT);
		    } else {
			set_action(0);
		    }
		}
		break;
/*
 * compute distance, dy/dx, angle
 */
	    case DISLINE1ST:
		_device2world(x, y, &wx1, &wy1);
		set_action(DISLINE2ND);
		rubber_flag = 1;
		sx = x;
		sy = y;
		select_line(sx, sy, x, y);
		break;
	    case DISLINE2ND:
		_device2world(x, y, &wx2, &wy2);
		sprintf(buf, "(%f, %f, %f, %f degrees)", hypot((wx2 - wx1), (wy2 - wy1)),
			wx2 - wx1, wy2 - wy1, 180.0 / M_PI * atan2(wy2 - wy1, wx2 - wx1));
		xv_setstr(locate_point_item, buf);
		set_action(0);
		set_action(DISLINE1ST);
		break;
/*
 * place the timestamp
 */
	    case PLACE_TIMESTAMP:
		_device2world(x, y, &wx, &wy);
		if (timestamp.loctype == COORD_VIEW) {
		    wx = xy_xconv(wx);
		    wy = xy_yconv(wy);
		}
		if (timestamp_x_item) {
		    sprintf(buf, "%g", wx);
		    xv_setstr(timestamp_x_item, buf);
		    sprintf(buf, "%g", wy);
		    xv_setstr(timestamp_y_item, buf);
		}
		timestamp.x = wx;
		timestamp.y = wy;
		set_action(0);
		xdrawgraph();
		break;
/*
 * pick compute ops
 */
	    case PICK_EXPR:
	    case PICK_RUNAVG:
	    case PICK_REG:
	    case PICK_FOURIER:
	    case PICK_HISTO:
	    case PICK_SAMPLE:
	    case PICK_PRUNE:
	    case PICK_DIFF:
	    case PICK_INT:
	    case PICK_SPLINE:
	    case PICK_SEASONAL:
		_device2world(x, y, &wx, &wy);
		findpoint(get_cg(), wx, wy, &wx, &wy, &setnumber, &setindex);
		if (setnumber >= 0) {
		    execute_pick_compute(get_cg(), setnumber, action_flag);
		    set_action(action_flag);
		} else {
		    set_action(0);
		}
		break;
/*
 * locate the graph legend
 */
	    case LEG_LOC:
		_device2world(x, y, &wx, &wy);
		if (g[get_cg()].l.loctype == COORD_VIEW) {
		    wx = xy_xconv(wx);
		    wy = xy_yconv(wy);
		}
		g[get_cg()].l.legx = wx;
		g[get_cg()].l.legy = wy;
		set_action(0);
		break;
/*
 * set one corner of zoom
 */
	    case ZOOM_1ST:
	    case ZOOMX_1ST:
	    case ZOOMY_1ST:
		switch (action_flag) {
		case ZOOM_1ST:
		    set_action(ZOOM_2ND);
		    break;
		case ZOOMX_1ST:
		    set_action(ZOOMX_2ND);
		    break;
		case ZOOMY_1ST:
		    set_action(ZOOMY_2ND);
		    break;
		}
		rectflag = 1;
		sx = x;
		sy = y;
		select_region(x, y, x, y);
		break;
/*
 * set opposing corner of zoom
 */
	    case ZOOM_2ND:
		set_action(0);
		select_region(sx, sy, old_x, old_y);
		_device2world(sx, sy, &wx1, &wy1);
		_device2world(old_x, old_y, &wx2, &wy2);
		if (sx == old_x || sy == old_y) {
		    errwin("Zoomed rectangle is zero along X or Y, zoom cancelled");
		} else {
		    if (wx1 > wx2) {
			fswap(&wx1, &wx2);
		    }
		    if (wy1 > wy2) {
			fswap(&wy1, &wy2);
		    }
		    newworld(get_cg(), linked_zoom, -1, wx1, wy1, wx2, wy2);
		    xdrawgraph();
		}
		break;
	    case ZOOMX_2ND:
		set_action(0);
		select_region(sx, sy, old_x, old_y);
		_device2world(sx, sy, &wx1, &wy1);
		_device2world(old_x, old_y, &wx2, &wy2);
		if (sx == old_x) {
		    errwin("Zoomed rectangle is zero along X, zoom cancelled");
		} else {
		    if (wx1 > wx2) {
			fswap(&wx1, &wx2);
		    }
		    newworld(get_cg(), linked_zoom, 0, wx1, wy1, wx2, wy2);
		    xdrawgraph();
		}
		break;
	    case ZOOMY_2ND:
		set_action(0);
		select_region(sx, sy, old_x, old_y);
		_device2world(sx, sy, &wx1, &wy1);
		_device2world(old_x, old_y, &wx2, &wy2);
		if (sy == old_y) {
		    errwin("Zoomed rectangle is zero along Y, zoom cancelled");
		} else {
		    if (wy1 > wy2) {
			fswap(&wy1, &wy2);
		    }
		    newworld(get_cg(), linked_zoom, 1, wx1, wy1, wx2, wy2);
		    xdrawgraph();
		}
		break;
/*
 * set one corner of viewport
 */
	    case  VIEW_1ST:
		set_action(VIEW_2ND);
		rectflag = 1;
		sx = x;
		sy = y;
		select_region(x, y, x, y);
		break;
/*
 * set opposing corner of viewport
 */
	    case  VIEW_2ND:
		{
		    double vx1, vx2, vy1, vy2;

		    set_action(0);
		    select_region(sx, sy, old_x, old_y);
		    if (sx == old_x || sy == old_y) {
			errwin("Viewport size incorrect, not changed");
		    } else {
			_device2world(sx, sy, &wx1, &wy1);
			_device2world(old_x, old_y, &wx2, &wy2);
			world2view(wx1, wy1, &vx1, &vy1);
			world2view(wx2, wy2, &vx2, &vy2);
			if (vx1 > vx2) {
			    fswap(&vx1, &vx2);
			}
			if (vy1 > vy2) {
			    fswap(&vy1, &vy2);
			}
			v.xv1 = vx1;
			v.xv2 = vx2;
			v.yv1 = vy1;
			v.yv2 = vy2;
                        set_graph_viewport(get_cg(), v);
			update_view(get_cg());
			xdrawgraph();
		    }
		}
		break;
	    }
	    break;
	case Button2:
	    getpoints(x, y);
	    switch (action_flag) {
	    case TRACKER:
		{
		    int xtmp, ytmp;
		    double *xx, *yy;

		    if (track_set == SET_SELECT_NEAREST) {	
			_device2world(x, y, &wx, &wy);
			findpoint(get_cg(), wx, wy, &wx, &wy, &track_set, &track_point);
			track_point-=2;
		    } else {	/* set selected, find nearest point */
			if (track_point == -1) {
			    _device2world(x, y, &wx, &wy);
			    findpoint_inset(get_cg(), track_set, wx, wy, &track_point);
			    track_point-=2;
			}
		    }
		    track_point++;
			if (track_point < 0) {
			    track_point = getsetlength(get_cg(), track_set) - 1;
		    } else if (track_point >= getsetlength(get_cg(), track_set)) {
			track_point = 0;
			}
		    xx = getx(get_cg(), track_set);
		    yy = gety(get_cg(), track_set);
		    if (inbounds(get_cg(), xx[track_point], yy[track_point])) {
			_world2device(xx[track_point], yy[track_point], &xtmp, &ytmp);
			    setpointer(xtmp, ytmp);
			    sprintf(buf, "Set %d, loc %d, (%f, %f)", track_set, track_point + 1,
				xx[track_point], yy[track_point]);
			} else {
			    sprintf(buf, "OUTSIDE - Set %d, loc %d, (%f, %f)", track_set, track_point + 1,
				xx[track_point], yy[track_point]);
			}
			xv_setstr(locate_point_item, buf);
		    set_action(TRACKER);
		}
		break;
	    }

	    break;
	}
	break;
    case MotionNotify:
	x = event->xmotion.x;
	y = event->xmotion.y;
/* cross hair cursor function */
	motion((XMotionEvent *) event);
/* allows painting of points */
	if (event->xmotion.state & Button1MotionMask) {
	    switch (action_flag) {
	    case PAINT_POINTS:
		{
		    int ind;
		    static int count = 0;

		    if (paint_skip < 0) {	/* initialize count */
			count = 0;
			paint_skip = -paint_skip + 1;
		    }
		    _device2world(x, y, &wx, &wy);
		    if (add_setno >= 0) {
			if (paint_skip == 0 || (count % paint_skip == 0)) {
			    ind = getsetlength(get_cg(), add_setno);
			    add_point(get_cg(), add_setno, wx, wy, 0.0, 0.0, SET_XY);
			    sprintf(buf, "Set %d, loc %d, (%f, %f)", add_setno, ind + 1, wx, wy);
			    xv_setstr(locate_point_item, buf);
			    XDrawLine(disp, xwin, gc, x - 5, y - 5, x + 5, y + 5);
			    XDrawLine(disp, xwin, gc, x - 5, y + 5, x + 5, y - 5);
			    update_set_status(get_cg(), add_setno);
			    set_action(PAINT_POINTS);
			}
			count++;
		    } else {
			set_action(0);
		    }
		}
		break;
	    }
	}
	getpoints(x, y);
	break;
    default:
	break;
    }
/*
 * some mouse tracking stuff
 */
    switch (action_flag) {
    case MOVE_OBJECT_2ND:
    case COPY_OBJECT2ND:
	dx = sx - x;
	dy = sy - y;

	switch (ty) {
	case OBJECT_BOX:
	case OBJECT_ELLIPSE:
	    select_region(sx, sy, xs, ys);
	    sx = x;
	    sy = y;
	    xs = xs - dx;
	    ys = ys - dy;
	    select_region(sx, sy, xs, ys);
	    break;
	case OBJECT_LINE:
	    select_line(sx, sy, xs, ys);
	    sx = x;
	    sy = y;
	    xs = xs - dx;
	    ys = ys - dy;
	    select_line(sx, sy, xs, ys);
	    break;
	case OBJECT_STRING:
	    select_region(sx, sy, xs, ys);
	    sx = x;
	    sy = y;
	    xs = xs - dx;
	    ys = ys - dy;
	    select_region(sx, sy, xs, ys);
	    break;
	}
	break;
    case STR_LOC:
	break;
    case LEG_LOC:
	break;
    }
    if (rectflag) {
	select_region(sx, sy, old_x, old_y);
	select_region(sx, sy, x, y);
    }
    if (rubber_flag) {
	select_line(sx, sy, old_x, old_y);
	select_line(sx, sy, x, y);
    }
    old_x = x;
    old_y = y;
}

/*
 * switch on the area calculator
 */
void do_select_area(void)
{
    narea_pts = 0;
    set_action(0);
    set_action(COMP_AREA);
}

/*
 * switch on the perimeter calculator
 */
void do_select_peri(void)
{
    narea_pts = 0;
    set_action(0);
    set_action(COMP_PERIMETER);
}

/*
 * define a region
 */
void do_select_region(void)
{
    region_pts = 0;
    set_action(0);
    set_action(DEF_REGION);
}

/*
 * double click detection
 */
#define CLICKINT 400

int double_click(XButtonEvent * e)
{
    static Time lastc = 0;

    if (e->time - lastc < CLICKINT) {
	return TRUE;
    }
    lastc = e->time;
    return FALSE;
}

/*
 * Determine if a graph corner was double clicked
 * return 1 if clicked, 0 o.w.
 */
int focus_clicked(int cg, int x, int y )
{
	int xv1, yv1, xv2, yv2;
	VPoint vptmp;
	
	vptmp.x = g[get_cg()].v.xv1;
	vptmp.y = g[get_cg()].v.yv1;
	xlibVPoint2dev(vptmp, &xv1, &yv1 );
	vptmp.x = g[get_cg()].v.xv2;
	vptmp.y = g[get_cg()].v.yv2;
	xlibVPoint2dev(vptmp, &xv2, &yv2 );
	
	if ( (fabs((float)(yv1 - y)) <= 5 || fabs((float)(yv2 - y)) <= 5) && 
	     (fabs((float)(xv1 - x)) <= 5 || fabs((float)(xv2 - x)) <= 5) ) {
		return 1;
	} else {
		return 0;
	}
}

VPoint xlibdev2VPoint(int x, int y);

void get_corner_clicked( int cg, int x, int y, double *xc, double *yc )
{
	double xv, yv;
	VPoint vp;
	
	vp = xlibdev2VPoint(x, y);
	xv = vp.x;
	yv = vp.y;
	if( fabs( g[get_cg()].v.xv1 - xv ) < fabs( g[get_cg()].v.xv2 - xv ) )
		*xc = g[get_cg()].v.xv1;
	else
		*xc = g[get_cg()].v.xv2;	
	if( fabs( g[get_cg()].v.yv1 - yv ) < fabs( g[get_cg()].v.yv2 - yv ) )
		*yc = g[get_cg()].v.yv1;
	else
		*yc = g[get_cg()].v.yv2;
}

/*
 * _world2device - given world coordinates (wx,wy) return the device coordinates,
 *              for the display only
 */
void _world2device(double wx, double wy, int *x, int *y)
{
    VPoint vp;
    WPoint wp;
    
    wp.x = wx;
    wp.y = wy;
    
    vp = Wpoint2Vpoint(wp);
    xlibVPoint2dev(vp, x, y);
}

/*
 * _device2world - given (x,y) in screen coordinates, return the world coordinates
 *             in (wx,wy) used for the display only
 */
static void _device2world(int x, int y, double *wx, double *wy)
{
    VPoint vp;

    vp = xlibdev2VPoint(x, y);
    view2world(vp.x, vp.y, wx, wy);
}


/*
 * Given the graph gno, find the graph that contains
 * (wx, wy). Used for setting the graph focus.
 */
int iscontained(int gno, double wx, double wy)
{
    int i;
    double x1, y1, x2, y2;
    double x, y;

    
    world2view(wx, wy, &x, &y);

    for (i = 0; i < number_of_graphs() ; i++) {
	if (is_graph_hidden(i) == FALSE) {
	    x1 = g[i].v.xv1;
	    x2 = g[i].v.xv2;
	    y1 = g[i].v.yv1;
	    y2 = g[i].v.yv2;
	    if (is_graph_xinvert(i)) {
		fswap(&x1, &x2);
	    }
	    if (is_graph_yinvert(i)) {
		fswap(&y1, &y2);
	    }
	    if ((x1 <= x && x2 >= x) && (y1 <= y && y2 >= y)) {
		return i;
	    }
	}
    }
    return gno;
}


/*
 * Given the graph gno, find the next graph that contains
 * (wx, wy). Used for setting the graph focus.
 */
int nextcontained(int gno, double wx, double wy)
{
    int i, j;
    double x1, y1, x2, y2;
    double x, y;

    world2view(wx, wy, &x, &y);
    
    for (j = 0; j < number_of_graphs() ; j++) {
	i = (j + gno + 1) % number_of_graphs() ;
	if (is_graph_hidden(i) == FALSE) {
	    x1 = g[i].v.xv1;
	    x2 = g[i].v.xv2;
	    y1 = g[i].v.yv1;
	    y2 = g[i].v.yv2;
	    if (is_graph_xinvert(i)) {
		fswap(&x1, &x2);
	    }
	    if (is_graph_yinvert(i)) {
		fswap(&y1, &y2);
	    }
	    if ((x1 <= x && x2 >= x) && (y1 <= y && y2 >= y)) {
		return i;
	    }
	}
    }
    return gno;
}

void do_autoscale_set(int gno, int setno)
{
    if (is_set_active(gno, setno)) {
	autoscale_byset(gno, setno, AUTOSCALE_XY);
        drawgraph();
    }
}

/*
 * for zooms, lz => linked to all active graphs
 *
 */
void newworld(int gno, int lz, int axes, double wx1, double wy1, double wx2, double wy2)
{
    int i, ming, maxg;
    if (lz) {
	ming = 0;
	maxg = number_of_graphs() ;
    } else {
	ming = gno;
	maxg = gno;
    }
    for (i = ming; i <= maxg; i++) {
	if (is_graph_active(i)) {
	    switch (axes) {
	    case AXIS_TYPE_ANY:
		g[i].w.xg1 = wx1;
		g[i].w.xg2 = wx2;
		g[i].w.yg1 = wy1;
		g[i].w.yg2 = wy2;
		autotick_axis(i, ALL_AXES);
                break;
	    case AXIS_TYPE_X:
		g[i].w.xg1 = wx1;
		g[i].w.xg2 = wx2;
                autotick_axis(i, ALL_X_AXES);
		break;
	    case AXIS_TYPE_Y:
		g[i].w.yg1 = wy1;
		g[i].w.yg2 = wy2;
                autotick_axis(i, ALL_Y_AXES);
		break;
	    }
	}
    }
}

void autoscale(Widget w, XKeyEvent *e, String *p, Cardinal *c)
{
    if (action_flag != STR_LOC) {
        autoscale_graph(get_cg(), AUTOSCALE_XY);
    }
}

void autoscale_on_near( Widget w, XKeyEvent *e, String *p, Cardinal *c )
{
	set_action(0);
	set_action(AUTO_NEAREST);	
}

void draw_box_action( Widget w, XKeyEvent *e, String *p, Cardinal *c )
{
	if( action_flag != STR_LOC ) {
		set_action(0);
		set_action(MAKE_BOX_1ST);	
	}
}

void delete_object( Widget w, XKeyEvent *e, String *p, Cardinal *c )
{
	if( action_flag != STR_LOC ) {
		set_action(0);
		set_action(DEL_OBJECT);	
	}
}

void place_legend( Widget w, XKeyEvent *e, String *p, Cardinal *c )
{
	if( action_flag != STR_LOC )
		legend_loc_proc(NULL, NULL, NULL);	
}

void move_object( Widget w, XKeyEvent *e, String *p, Cardinal *c )
{
	if( action_flag != STR_LOC ) {
		set_action(0);
		set_action(MOVE_OBJECT_1ST);
	}
}

void draw_line_action( Widget w, XKeyEvent *e, String *p, Cardinal *c )
{
	if( action_flag != STR_LOC ){
		set_action(0);
		set_action(MAKE_LINE_1ST);
	}
}

void refresh_hotlink( Widget w, XKeyEvent *e, String *p, Cardinal *c )
{
	if( action_flag != STR_LOC )
		do_hotupdate_proc( (Widget)NULL, (XtPointer)NULL, (XtPointer)NULL );
}

void set_viewport( Widget w, XKeyEvent *e, String *p, Cardinal *c )
{
	if( action_flag != STR_LOC ){
    	set_action(0);
    	set_action(VIEW_1ST);	
	}
}

void write_string( Widget w, XKeyEvent *e, String *p, Cardinal *c )
{
    if( action_flag != STR_LOC ) {
    	set_action(0);
    	set_action(STR_LOC);	
    }
}

void exit_abruptly( Widget w, XKeyEvent *e, String *p, Cardinal *c )
{
    bailout();	
}

void enable_zoom( Widget w, XKeyEvent *e, String *p, Cardinal *c )
{
    if( action_flag != STR_LOC ){
        set_action(0);
        set_action(ZOOM_1ST);	
    }
}


/*
 * activate the legend location flag
 */
void legend_loc_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    set_action(0);
    set_action(LEG_LOC);
    set_dirtystate();
}

/*
 * world stack operations
 */
void push_and_zoom(void)
{
    push_world();
#ifndef NONE_GUI
    set_action(0);
    set_action(ZOOM_1ST);
#endif
}


static double _stringextentx(double scale, char *s)
{
    return (0.015 * scale * strlen(s));
}

static double _stringextenty(double scale, char *s)
{
    return (0.025 * scale);
}
