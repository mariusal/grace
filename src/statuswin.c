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
 * status popup
 *
 */

#include <config.h>

#include <stdio.h>

#include <Xm/Xm.h>
#include <Xm/BulletinB.h>
#include <Xm/DialogS.h>
#include <Xm/DrawingA.h>
#include <Xm/Frame.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/Separator.h>
#include <Xm/RowColumn.h>
#include <Xm/ScrolledW.h>

#include "globals.h"
#include "utils.h"
#include "graphs.h"
#include "graphutils.h"
#include "plotone.h"
#include "protos.h"
#include "motifinc.h"

/* Objects to display in status window */
#define STATUS_SETS     0
#define STATUS_REGIONS  1

#define SPAGESIZE 10

#define  STATUS_NULL 0
#define  STATUS_KILL 1
#define  STATUS_SOFTKILL 2
#define  STATUS_DEACTIVATE 3
#define  STATUS_REACTIVATE 4
#define  STATUS_COPY1ST 5
#define  STATUS_MOVE1ST 6
#define  STATUS_COPY2ND 7
#define  STATUS_MOVE2ND 8
#define  STATUS_PACK 9
#define  STATUS_AUTOSCALE 10
#define  STATUS_REVERSE 11
#define  STATUS_JOIN1ST 12
#define  STATUS_JOIN2ND 13

#define  STATUS_REGION_DEFINE 201
#define  STATUS_REGION_KILL 202
#define  STATUS_REGION_EXTRACT 203
#define  STATUS_REGION_EVAL 204
#define  STATUS_REGION_DEL 205

extern Display *disp;
extern int cset;        /* defined in symwin.c */

static int cur_statusitem = STATUS_SETS;

static int npages;

static char buf[256];

#define getdx(gno, setn)    getcol(gno, setn, 2)
#define getdy(gno, setn)    getcol(gno, setn, 3)

#define cg get_cg()

static Widget status_frame;
static Widget status_panel;
static Widget status_sw;
static Widget *select_status_item;
static Widget status_auto_redraw_tb;
static Dimension status_minwidth = 0;
static int curpage = 0;

static Widget header_w;
static Widget *labx;
static Widget *laby;

static Widget rc4, rc6;

static char header[256];

static XFontStruct *f;
static XmFontList xmf;

static void set_status_label(Widget w, char *s);
static void page_status_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void home_status_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void end_status_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void adjust_status_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void status_item_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void set_status_action(int cd);
static void status_op(Widget w, XtPointer client_data, XtPointer call_data);
static void toggle_status_auto_redraw(Widget w, XtPointer client_data,
    XtPointer call_data);

static void set_status_label(Widget w, char *buf)
{
    Arg al;
    XmString ls;
    ls = XmStringCreateLtoR(buf, charset);
    XtSetArg(al, XmNlabelString, ls);
    XtSetValues(w, &al, 1);
    XmStringFree(ls);
}

void update_status(int gno, int itemno)
{
    int i;

    update_set_lists(gno);
    if (status_frame) {
	set_status_label(header_w, header);
	switch (cur_statusitem) {
	case STATUS_SETS:
	    if (itemno < 0) {
		for (i = curpage * SPAGESIZE; i < SPAGESIZE * (curpage + 1); i++) {
		    if (i < number_of_sets(gno)) {
			update_set_status(gno, i);
		    } else {
			set_status_label(labx[i - curpage * SPAGESIZE],
				 "Not initialized");
			set_status_label(laby[i - curpage * SPAGESIZE],
					 " ");
		    }
		}
	    }
	    break;
	case STATUS_REGIONS:
	    if (itemno < 0) {
		for (i = 0; i < MAXREGION; i++) {
		    update_region_status(i);
		}
	    }
	    break;
	}
    }
}

void update_region_status(int rno)
{
    if (rno >= 0 && rno < MAXREGION) {
	if (status_frame && cur_statusitem == STATUS_REGIONS) {
	    sprintf(buf, "  %2d    %3s   %6s", rno, on_or_off(rg[rno].active),
		    region_types(rg[rno].type, 0));
	    set_status_label(labx[rno], buf);
	}
    }
}

void update_set_status(int gno, int setno)
{
    double x1, y1, x2, y2, xbar, ybar, xsd, ysd;
    int ix1, ix2;
    int iy1, iy2;
    char buf1[512], buf2[512];

    if (setno >= number_of_sets(gno)) {
	return;
    }
    update_set_lists(gno);
    if (setno >= curpage * SPAGESIZE && setno < (curpage + 1) * SPAGESIZE) {
	if (status_frame && cur_statusitem == STATUS_SETS && gno == cg) {
	    if (is_set_active(gno, setno)) {
		minmax(getx(gno, setno), getsetlength(gno, setno), &x1, &x2, &ix1, &ix2);
		minmax(gety(gno, setno), getsetlength(gno, setno), &y1, &y2, &iy1, &iy2);
		xbar = 0.0;
		ybar = 0.0;
		xsd = 0.0;
		ysd = 0.0;
		stasum(getx(gno, setno), getsetlength(gno, setno), &xbar, &xsd, 0);
		stasum(gety(gno, setno), getsetlength(gno, setno), &ybar, &ysd, 0);
		sprintf(buf1, " %2d   %7d %4s %7s | X   %11.5g %7d %11.5g %7d %11.5g %11.5g  %s",
			setno, getsetlength(gno, setno),
			on_or_off(is_set_active(gno, setno)),
			set_types(dataset_type(gno, setno)), x1, ix1, x2, ix2,
			xbar, xsd, getcomment(gno, setno));
		sprintf(buf2, "%26s | Y   %11.5g %7d %11.5g %7d %11.5g %11.5g",
			"", y1, iy1, y2, iy2, ybar, ysd);
	    } else if (is_set_hidden(gno, setno)) {
		sprintf(buf1, " %2d    De-activated (%s)", setno, getcomment(gno, setno));
		strcpy(buf2, " ");
	    } else {
		sprintf(buf1, " %2d    Undefined", setno);
		strcpy(buf2, " ");
	    }
	    set_status_label(labx[setno - curpage * SPAGESIZE], buf1);
	    set_status_label(laby[setno - curpage * SPAGESIZE], buf2);
	}
    }
}

void clear_status(void)
{
    int i;
    for (i = 0; i < SPAGESIZE; i++) {
	set_status_label(labx[i], " ");
	set_status_label(laby[i], " ");
    }
}

void update_status_popup(Widget w, XtPointer client_data, XtPointer call_data)
{
    if (status_frame) {
        update_status_auto_redraw();
    }
    update_set_lists(cg);
    status_item_proc((Widget) NULL, (XtPointer) NULL, (XtPointer) NULL);
/*
    update_status(cg, cur_statusitem, -1);
*/
}

static void page_status_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int dir = (int) client_data;
    if (dir == 1) {
	curpage = (curpage + 1) % npages;
    } else {
	curpage = curpage - 1;
	if (curpage < 0) {
	    curpage = npages - 1;
	}
    }
    update_status(cg, -1);
}

static void home_status_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    curpage = 0;
    update_status(cg, -1);
}

static void end_status_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    curpage = npages - 1;
    update_status(cg, -1);
}

static void adjust_status_proc(Widget w, XtPointer client_data, 
                               XtPointer call_data)
{
    Dimension status_width, tmp_width;
    Widget sw_vsb;
    int i;

    update_status_popup(NULL, NULL, NULL);
    XtVaGetValues (header_w, XmNwidth, &status_width, NULL);
    for (i = 0; i < SPAGESIZE; i++) {
        XtVaGetValues (labx[i], XmNwidth, &tmp_width, NULL);
        if (tmp_width > status_width) {
            status_width = tmp_width;
        }
        XtVaGetValues (laby[i], XmNwidth, &tmp_width, NULL);
        if (tmp_width > status_width) {
            status_width = tmp_width;
        }
    }
    sw_vsb = XtNameToWidget (status_sw, "VertScrollBar");
    if (sw_vsb != NULL) {
        XtVaGetValues (sw_vsb, XmNwidth, &tmp_width, NULL);
        status_width += tmp_width;
        XtVaGetValues (status_sw, XmNspacing, &tmp_width, NULL);
        status_width += tmp_width;
    }
    status_width += 10;
    if (status_width < status_minwidth) {
        status_width = status_minwidth;
    }
    XtVaSetValues (status_frame, XmNwidth, status_width, NULL);
}

static void status_item_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int cd;
    update_set_lists(cg);
    if (status_frame) {
	cd = GetChoice(select_status_item);

	switch (cd) {
	case 0:
	    npages = number_of_sets(cg) / SPAGESIZE;
	    if (npages * SPAGESIZE != number_of_sets(cg)) {
		npages++;
	    }
            if (npages == 0) {
                npages++;
            }
	    curpage = 0;
	    cur_statusitem = STATUS_SETS;
            strcpy(header, " set#     n   stat    type | X/Y         min      ");
            strcat(header, "at         max      at        mean      stddev  comment");
	    XtUnmanageChild(rc6);
	    XtManageChild(rc4);
	    break;
	case 1:
	    npages = MAXREGION / SPAGESIZE;
	    if (npages * SPAGESIZE != MAXREGION) {
		npages++;
	    }
	    curpage = 0;
	    cur_statusitem = STATUS_REGIONS;
	    clear_status();
	    sprintf(header, " Region # Active  Type");
	    XtUnmanageChild(rc4);
	    XtManageChild(rc6);
	    set_status_action(STATUS_NULL);
	    break;
	}
	set_status_label(header_w, header);
	update_status(cg, -1);
    }
}

/*
 * write the status to the results file
 */
void update_stuff_status(void)
{
    int i, j;
    double x1, y1, x2, y2, xbar, ybar, xsd, ysd;

    strcpy(buf, "\nStatus\n");
    stufftext(buf, STUFF_START);
    for (j = 0; j < number_of_graphs(); j++) {
	if (is_graph_active(j)) {
	    if (j == cg) {
		sprintf(buf, "\nStatus of sets for graph %d (current)\n", cg);
	    } else {
		sprintf(buf, "\nStatus of sets for graph %d\n", j);
	    }
	    stufftext(buf, STUFF_TEXT);
	    sprintf(buf, " set#     n   X/Y          min         max        mean      stddev  comment\n");
	    stufftext(buf, STUFF_TEXT);
	    for (i = 0; i < number_of_sets(j); i++) {
		if (is_set_active(j, i)) {
		    getsetminmax(j, i, &x1, &x2, &y1, &y2);
		    stasum(getx(j, i), getsetlength(j, i), &xbar, &xsd, 0);
		    stasum(gety(j, i), getsetlength(j, i), &ybar, &ysd, 0);
		    sprintf(buf, " %3d  %7d  X   %11.5g %11.5g %11.5g %11.5g  %s\n",
			    i, getsetlength(j, i),
			    x1, x2, xbar, xsd, getcomment(j, i));
		    stufftext(buf, STUFF_TEXT);
		    sprintf(buf, "               Y   %11.5g %11.5g %11.5g %11.5g\n", y1, y2, ybar, ysd);
		    stufftext(buf, STUFF_TEXT);
		}
	    }
	}
    }
    strcpy(buf, "\n");
    stufftext(buf, STUFF_STOP);
}

static int status_curset;
static int status_curop;
static int status_set1;
static int status_set2;
static int status_g1;
static int status_g2;

static XmString infostring;
static Widget infolab;

static Window rcwin;

static void set_status_action(int cd)
{
    char buf[256];
    status_curop = cd;
    switch (cd) {
    case STATUS_KILL:
    case STATUS_SOFTKILL:
	sprintf(buf, "Click on the set index number to kill the set");
	set_window_cursor(rcwin, 3);
	break;
    case STATUS_DEACTIVATE:
	sprintf(buf, "Click on the set index number to hide the set");
	set_window_cursor(rcwin, 0);
	break;
    case STATUS_REACTIVATE:
	sprintf(buf, "Click on the set index number to show the set");
	set_window_cursor(rcwin, 0);
	break;
    case STATUS_COPY1ST:
	sprintf(buf, "Click on the set index number to copy from");
	set_window_cursor(rcwin, 0);
	break;
    case STATUS_MOVE1ST:
	sprintf(buf, "Click on the set index number to move from");
	set_window_cursor(rcwin, 4);
	break;
    case STATUS_COPY2ND:
	sprintf(buf, "Copy set %d in graph %d to...", status_set1, status_g1);
	set_window_cursor(rcwin, 0);
	break;
    case STATUS_MOVE2ND:
	sprintf(buf, "Move set %d in graph %d to...", status_set1, status_g1);
	set_window_cursor(rcwin, 4);
	break;
    case STATUS_REVERSE:
	sprintf(buf, "Click on the set index number to reverse");
	set_window_cursor(rcwin, 0);
	break;
    case STATUS_JOIN1ST:
	sprintf(buf, "Click on the set index number to join to");
	set_window_cursor(rcwin, 0);
	break;
    case STATUS_JOIN2ND:
	sprintf(buf, "Join set %d in graph %d to...", status_set1, status_g1);
	set_window_cursor(rcwin, 0);
	break;
    case STATUS_AUTOSCALE:
	sprintf(buf, "Click on the set index number to autoscale");
	set_window_cursor(rcwin, 0);
	break;
/* regions */
    case STATUS_REGION_DEFINE:
	sprintf(buf, "Click on a region number to define");
	set_window_cursor(rcwin, 0);
	break;
    case STATUS_REGION_KILL:
	sprintf(buf, "Click on a region number to kill");
	set_window_cursor(rcwin, 3);
	break;
    case STATUS_REGION_EXTRACT:
	sprintf(buf, "Click on a region number to extract points to the next available set");
	set_window_cursor(rcwin, 0);
	break;
    case STATUS_REGION_EVAL:
	sprintf(buf, "Click on a region number to evaluate");
	set_window_cursor(rcwin, 0);
	break;
    case STATUS_REGION_DEL:
	sprintf(buf, "Click on a region number in which to kill all points");
	set_window_cursor(rcwin, 3);
	break;
    case STATUS_NULL:
	sprintf(buf, "Idle...");
	set_window_cursor(rcwin, -1);
	break;
    }
    if (infostring) {
	XmStringFree(infostring);
    }
    infostring = XmStringCreateLtoR(buf, charset);
    XtVaSetValues(infolab, XmNlabelString, infostring, NULL);
}

void select_set(Widget w, XtPointer calld, XEvent * e)
{
    int cd = (int) calld;
    if (e->type != ButtonPress) {
	return;
    }
    switch (e->xbutton.button) {
    case Button3:
	set_status_action(STATUS_NULL);
	return;
	break;
    case Button2:
	set_status_action(STATUS_NULL);
	return;
	break;
    }

    if (cur_statusitem == STATUS_SETS) {
	if (cd + curpage * SPAGESIZE >= number_of_sets(cg)) {
	    set_status_action(STATUS_NULL);
	    errwin("Not that many sets\n");
	    return;
	}
	status_curset = cd + curpage * SPAGESIZE;
	cd = status_curset;
	if (status_curop == STATUS_NULL && double_click((XButtonEvent *) e)) {
	    cset = cd;
	    set_window_cursor(rcwin, 5);
	    define_symbols_popup(NULL, NULL, NULL);
	    set_window_cursor(rcwin, -1);
	    return;
	}
	switch (status_curop) {
	case STATUS_KILL:
	    do_showset(cg, cd);
	    do_kill(cg, cd, 0);
	    if (status_auto_redraw) {
		drawgraph();
	    }
	    set_status_action(STATUS_KILL);
	    break;
	case STATUS_SOFTKILL:
	    do_showset(cg, cd);
	    do_kill(cg, cd, 1);
	    if (status_auto_redraw) {
		drawgraph();
	    }
	    set_status_action(STATUS_SOFTKILL);
	    break;
	case STATUS_DEACTIVATE:
	    if (is_set_active(cg, cd)) {
		do_hideset(cg, cd);
		if (status_auto_redraw) {
		    drawgraph();
		}
		set_status_action(STATUS_DEACTIVATE);
	    } else {
		errwin("Set not active, Hide requires an active set");
		set_status_action(STATUS_NULL);
	    }
	    break;
	case STATUS_REACTIVATE:
	    do_showset(cg, cd);
	    if (status_auto_redraw) {
		drawgraph();
	    }
	    set_status_action(STATUS_REACTIVATE);
	    break;
	case STATUS_COPY1ST:
	    status_set1 = cd;
	    status_g1 = cg;
	    if (is_set_active(cg, cd)) {
		set_status_action(STATUS_COPY2ND);
	    } else {
		errwin("Set not active, Copy requires an active set");
		set_status_action(STATUS_NULL);
	    }
	    break;
	case STATUS_MOVE1ST:
	    status_set1 = cd;
	    status_g1 = cg;
	    if (is_set_active(cg, cd)) {
		set_status_action(STATUS_MOVE2ND);
	    } else {
		errwin("Set not active, Move requires an active set");
		set_status_action(STATUS_NULL);
	    }
	    break;
	case STATUS_COPY2ND:
	    status_set2 = cd;
	    status_g2 = cg;
	    do_copyset(status_g1, status_set1, status_g2, status_set2);
	    if (status_auto_redraw) {
		drawgraph();
	    }
	    set_status_action(STATUS_COPY1ST);
	    break;
	case STATUS_MOVE2ND:
	    status_set2 = cd;
	    status_g2 = cg;
	    do_moveset(status_g1, status_set1, status_g2, status_set2);
	    if (status_auto_redraw) {
		drawgraph();
	    }
	    set_status_action(STATUS_MOVE1ST);
	    break;
	case STATUS_REVERSE:
	    if (is_set_active(cg, cd)) {
		do_reverse_sets(cd);
		if (status_auto_redraw) {
		    drawgraph();
		}
		set_status_action(STATUS_REVERSE);
	    } else {
		errwin("Set not active, Reverse operates on active sets");
		set_status_action(STATUS_NULL);
	    }
	    break;
	case STATUS_JOIN1ST:
	    status_set1 = cd;
	    status_g1 = cg;
	    if (is_set_active(cg, cd)) {
		set_status_action(STATUS_JOIN2ND);
	    } else {
		errwin("Set not active, Join operates on active sets");
		set_status_action(STATUS_NULL);
	    }
	    break;
	case STATUS_JOIN2ND:
	    status_set2 = cd;
	    status_g2 = cg;
	    if (is_set_active(cg, cd)) {
		if ((status_g1 == status_g2) && (status_set1 == status_set2)) {
		    errwin("Can't join set to itself, use copy then join");
		    set_status_action(STATUS_NULL);
		} else {
		    do_join_sets(status_g1, status_set1, status_g2, status_set2);
		    if (status_auto_redraw) {
			drawgraph();
		    }
		    set_status_action(STATUS_JOIN1ST);
		}
	    } else {
		errwin("Set not active, Join operates on active sets");
		set_status_action(STATUS_NULL);
	    }
	    break;
	case STATUS_AUTOSCALE:
	    if (is_set_active(cg, cd)) {
		autoscale_byset(cg, cd, AUTOSCALE_XY);
		if (status_auto_redraw) {
		    drawgraph();
		}
	    } else {
		errwin("Set not active, Auto operates on active sets");
		set_status_action(STATUS_NULL);
	    }
	    break;
	default:
	    set_status_action(STATUS_NULL);
	    break;
	}
    } else if (cur_statusitem == STATUS_REGIONS) {
	status_curop = STATUS_NULL;
	set_window_cursor(rcwin, -1);
	switch (status_curop) {
	case STATUS_REGION_DEFINE:
	    set_status_action(STATUS_REGION_DEFINE);
	    break;
	case STATUS_REGION_KILL:
	    set_status_action(STATUS_REGION_KILL);
	    break;
	case STATUS_REGION_EXTRACT:
	    set_status_action(STATUS_REGION_EXTRACT);
	    break;
	case STATUS_REGION_EVAL:
	    set_status_action(STATUS_REGION_EVAL);
	    break;
	case STATUS_REGION_DEL:
	    set_status_action(STATUS_REGION_DEL);
	    break;
	}
    }
}

static void status_op(Widget w, XtPointer client_data, XtPointer call_data)
{
    int cd = (int) client_data;
    if (cd == STATUS_PACK) {
	status_curop = STATUS_NULL;
	set_window_cursor(rcwin, -1);
	set_wait_cursor();
	packsets(cg);
	unset_wait_cursor();
	set_status_action(STATUS_NULL);
    } else {
	set_status_action(cd);
    }
}

static void toggle_status_auto_redraw(Widget w, XtPointer client_data,
    XtPointer call_data)
{
    status_auto_redraw = (int) XmToggleButtonGetState(w);
}

void update_status_auto_redraw(void)
{
    if (status_auto_redraw_tb) {
	XmToggleButtonSetState(status_auto_redraw_tb,
	    status_auto_redraw == TRUE, False);
    }
}


void define_status_popup(Widget w, XtPointer client_data, XtPointer call_data)
{
    int i;
    XmString header_string;
    Dimension status_width, tmp_width;
    Widget wbut, rc, rc3, fr1, fr2, sw_vsb;
    set_wait_cursor();
    if (status_frame == NULL) {
	npages = number_of_sets(cg) / SPAGESIZE;
	if (npages * SPAGESIZE != number_of_sets(cg)) {
	    npages++;
	}
	status_frame = XmCreateDialogShell(app_shell, "Status", NULL, 0);
	handle_close(status_frame);

	f = (XFontStruct *) XLoadQueryFont(disp, "fixed");
	xmf = XmFontListCreate(f, charset);

	status_panel = XmCreateForm(status_frame, "form", NULL, 0);

	status_sw = XtVaCreateManagedWidget("sw",
			       xmScrolledWindowWidgetClass, status_panel,
				     XmNscrollingPolicy, XmAUTOMATIC,
				     XmNheight, 300,
				     NULL);
	rc3 = XmCreateRowColumn(status_sw, "rc3", NULL, 0);
	header_w = XtVaCreateManagedWidget("header", xmLabelWidgetClass, rc3,
				     XmNalignment, XmALIGNMENT_BEGINNING,
					   XmNfontList, xmf,
					   XmNrecomputeSize, True,
					   NULL);
	labx = (Widget *)malloc( SPAGESIZE*sizeof(Widget) );
	laby = (Widget *)malloc( SPAGESIZE*sizeof(Widget) );
	for (i = 0; i < SPAGESIZE; i++) {
	    labx[i] = XtVaCreateManagedWidget("X", xmLabelWidgetClass, rc3,
				     XmNalignment, XmALIGNMENT_BEGINNING,
					      XmNfontList, xmf,
					      XmNrecomputeSize, True,
					      NULL);
	    laby[i] = XtVaCreateManagedWidget("Y", xmLabelWidgetClass, rc3,
				     XmNalignment, XmALIGNMENT_BEGINNING,
					      XmNfontList, xmf,
					      XmNrecomputeSize, True,
					      NULL);
	    XtAddEventHandler(labx[i], ButtonPressMask, False,
			      (XtEventHandler) select_set, (XtPointer) i);
	    XtAddEventHandler(laby[i], ButtonPressMask, False,
			      (XtEventHandler) select_set, (XtPointer) i);
	}
	XtManageChild(rc3);
	XtVaSetValues(status_sw,
		      XmNworkWindow, rc3,
		      NULL);

	fr1 = XmCreateFrame(status_panel, "fr1", NULL, 0);
	rc = XmCreateRowColumn(fr1, "rc", NULL, 0);
	infolab = XtVaCreateManagedWidget("Idle...", xmLabelWidgetClass, rc,
				     XmNalignment, XmALIGNMENT_BEGINNING,
					  XmNrecomputeSize, True,
					  NULL);
	rc4 = XmCreateRowColumn(rc, "rc", NULL, 0);
	XtVaSetValues(rc4, XmNorientation, XmHORIZONTAL, NULL);
	wbut = XtVaCreateManagedWidget("Kill", xmPushButtonWidgetClass, rc4,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) status_op, (XtPointer) STATUS_KILL);
	wbut = XtVaCreateManagedWidget("Soft kill", xmPushButtonWidgetClass,
                                       rc4, NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) status_op, (XtPointer) STATUS_SOFTKILL);
	wbut = XtVaCreateManagedWidget("Hide", xmPushButtonWidgetClass, rc4,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) status_op, (XtPointer) STATUS_DEACTIVATE);
	wbut = XtVaCreateManagedWidget("Show", xmPushButtonWidgetClass, rc4,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) status_op, (XtPointer) STATUS_REACTIVATE);
	wbut = XtVaCreateManagedWidget("Copy", xmPushButtonWidgetClass, rc4,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) status_op, (XtPointer) STATUS_COPY1ST);
	wbut = XtVaCreateManagedWidget("Move", xmPushButtonWidgetClass, rc4,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) status_op, (XtPointer) STATUS_MOVE1ST);
	wbut = XtVaCreateManagedWidget("Auto", xmPushButtonWidgetClass, rc4,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) status_op, (XtPointer) STATUS_AUTOSCALE);
	wbut = XtVaCreateManagedWidget("Reverse", xmPushButtonWidgetClass, rc4,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) status_op, (XtPointer) STATUS_REVERSE);
	wbut = XtVaCreateManagedWidget("Join", xmPushButtonWidgetClass, rc4,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) status_op, (XtPointer) STATUS_JOIN1ST);
	wbut = XtVaCreateManagedWidget("Pack", xmPushButtonWidgetClass, rc4,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) status_op, (XtPointer) STATUS_PACK);
	wbut = XtVaCreateManagedWidget("Cancel", xmPushButtonWidgetClass, rc4,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) status_op, (XtPointer) STATUS_NULL);
        status_auto_redraw_tb = XtVaCreateManagedWidget(
	    "Auto redraw", xmToggleButtonWidgetClass, rc4, NULL);
	XtAddCallback(status_auto_redraw_tb, XmNvalueChangedCallback,
	    (XtCallbackProc) toggle_status_auto_redraw, (XtPointer) NULL);

	XtManageChild(rc4);
	XtManageChild(rc);
	XtManageChild(fr1);

	rc6 = XmCreateRowColumn(rc, "rc", NULL, 0);

	XtVaSetValues(rc6, XmNorientation, XmHORIZONTAL, NULL);
	wbut = XtVaCreateManagedWidget("Define", xmPushButtonWidgetClass, rc6,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback,
	   (XtCallbackProc) status_op, (XtPointer) STATUS_REGION_DEFINE);

	wbut = XtVaCreateManagedWidget("Kill", xmPushButtonWidgetClass, rc6,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback,
	     (XtCallbackProc) status_op, (XtPointer) STATUS_REGION_KILL);

	wbut = XtVaCreateManagedWidget("Extract", xmPushButtonWidgetClass, rc6,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback,
	  (XtCallbackProc) status_op, (XtPointer) STATUS_REGION_EXTRACT);

	wbut = XtVaCreateManagedWidget("Evaluate", xmPushButtonWidgetClass, rc6,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback,
	     (XtCallbackProc) status_op, (XtPointer) STATUS_REGION_EVAL);

	wbut = XtVaCreateManagedWidget("Delete pts", xmPushButtonWidgetClass, rc6,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback,
	      (XtCallbackProc) status_op, (XtPointer) STATUS_REGION_DEL);

	wbut = XtVaCreateManagedWidget("Report", xmPushButtonWidgetClass, rc6,
				       NULL);

	wbut = XtVaCreateManagedWidget("Cancel", xmPushButtonWidgetClass, rc6,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) status_op, (XtPointer) STATUS_NULL);

	fr2 = XmCreateFrame(status_panel, "fr2", NULL, 0);
	rc = XmCreateRowColumn(fr2, "rc", NULL, 0);
	XtVaSetValues(rc, XmNorientation, XmHORIZONTAL, NULL);

	wbut = XtVaCreateManagedWidget("Close", xmPushButtonWidgetClass, rc,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) destroy_dialog, status_frame);

	wbut = XtVaCreateManagedWidget("Update", xmPushButtonWidgetClass, rc,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) update_status_popup, (XtPointer) 0);

	wbut = XtVaCreateManagedWidget("Write", xmPushButtonWidgetClass, rc,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) update_stuff_status, (XtPointer) 0);

	wbut = XtVaCreateManagedWidget("Page+", xmPushButtonWidgetClass, rc,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) page_status_proc, (XtPointer) 1);

	wbut = XtVaCreateManagedWidget("Page-", xmPushButtonWidgetClass, rc,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) page_status_proc, (XtPointer) 0);

	wbut = XtVaCreateManagedWidget("Home", xmPushButtonWidgetClass, rc,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) home_status_proc, (XtPointer) 0);

	wbut = XtVaCreateManagedWidget("End", xmPushButtonWidgetClass, rc,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) end_status_proc, (XtPointer) 0);
	wbut = XtVaCreateManagedWidget("Adjust", xmPushButtonWidgetClass, rc,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) adjust_status_proc, (XtPointer) 0);

	select_status_item = CreatePanelChoice(rc, "Display: ",
					       3,
					       "Sets", "Regions",
					       NULL,
					       0);
	XtAddCallback(select_status_item[2],
		      XmNactivateCallback, (XtCallbackProc) status_item_proc, (XtPointer) 0);
	XtAddCallback(select_status_item[3],
		      XmNactivateCallback, (XtCallbackProc) status_item_proc, (XtPointer) 1);
	wbut = XtVaCreateManagedWidget("Help", xmPushButtonWidgetClass, rc, 
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) HelpCB, (XtPointer) "data.html#status");

	XtManageChild(rc);
	XtManageChild(fr2);

	XtVaSetValues(status_sw,
		      XmNtopAttachment, XmATTACH_FORM,
		      XmNleftAttachment, XmATTACH_FORM,
		      XmNrightAttachment, XmATTACH_FORM,
		      XmNbottomAttachment, XmATTACH_WIDGET,
		      XmNbottomWidget, fr1,
		      NULL);
	XtVaSetValues(fr1,
		      XmNleftAttachment, XmATTACH_FORM,
		      XmNrightAttachment, XmATTACH_FORM,
		      XmNbottomAttachment, XmATTACH_WIDGET,
		      XmNbottomWidget, fr2,
		      NULL);
	XtVaSetValues(fr2,
		      XmNleftAttachment, XmATTACH_FORM,
		      XmNrightAttachment, XmATTACH_FORM,
		      XmNbottomAttachment, XmATTACH_FORM,
		      NULL);
	XtManageChild(status_panel);

	rcwin = XtWindow(rc3);
        update_status_popup(NULL, NULL, NULL);
        XtVaGetValues (status_sw, XmNwidth, &status_minwidth, NULL);
        XtVaGetValues (header_w, XmNlabelString, &header_string, NULL);
        status_width = XmStringWidth (xmf, header_string);
        sw_vsb = XtNameToWidget (status_sw, "VertScrollBar");
        if (sw_vsb != NULL) {
            XtVaGetValues (sw_vsb, XmNwidth, &tmp_width, NULL);
            status_width += tmp_width;
            XtVaGetValues (status_sw, XmNspacing, &tmp_width, NULL);
            status_width += tmp_width;
        }
        status_width += 25;
        if (status_width < status_minwidth) {
            status_width = status_minwidth;
        }
        XtVaSetValues (status_frame, XmNwidth, status_width, NULL);
    }
    XtRaise(status_frame);
    update_status_popup(NULL, NULL, NULL);
    unset_wait_cursor();
}
