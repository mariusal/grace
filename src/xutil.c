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

#include <config.h>

#include <stdlib.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/cursorfont.h>

#include "defines.h"
#include "globals.h"

#include "utils.h"
#include "graphs.h"
#include "graphutils.h"
#include "plotone.h"
#include "device.h"

#include "x11drv.h"

#include "protos.h"

extern Window root, xwin;
extern Display *disp;
extern Widget app_shell;
extern XtAppContext app_con;
extern GC gc, gcxor;
extern int depth;

static Pixmap bufpixmap = (Pixmap) NULL;

extern int win_h, win_w;	/* declared in x11drv.c */

extern int inpipe;
extern char batchfile[];

/*
 * cursors
 */

static Cursor wait_cursor;
static Cursor line_cursor;
static Cursor find_cursor;
static Cursor move_cursor;
static Cursor text_cursor;
static Cursor kill_cursor;
static int cur_cursor = -1;
static int waitcursoron = FALSE;

int cursortype = 0;
static int cursor_oldx = -1, cursor_oldy = -1;

void DefineDialogCursor(Cursor c);
void UndefineDialogCursor();

void set_wait_cursor()
{
    if (disp == NULL) {
        return;
    }
    
    DefineDialogCursor(wait_cursor);
    waitcursoron = TRUE;
}

void unset_wait_cursor()
{
    if (disp == NULL) {
        return;
    }
    
    UndefineDialogCursor();
    if (cur_cursor >= 0) {
        set_cursor(cur_cursor);
    }
    waitcursoron = FALSE;
}

void set_cursor(int c)
{
    if (disp == NULL) {
        return;
    }

    XUndefineCursor(disp, xwin);
    cur_cursor = -1;
    switch (c) {
    case 0:
        XDefineCursor(disp, xwin, line_cursor);
        cur_cursor = 0;
        break;
    case 1:
        XDefineCursor(disp, xwin, find_cursor);
        cur_cursor = 1;
        break;
    case 2:
        XDefineCursor(disp, xwin, text_cursor);
        cur_cursor = 2;
        break;
    case 3:
        XDefineCursor(disp, xwin, kill_cursor);
        cur_cursor = 3;
        break;
    case 4:
        XDefineCursor(disp, xwin, move_cursor);
        cur_cursor = 4;
        break;
    }
    XFlush(disp);
}

void set_window_cursor(Window xwin, int c)
{
    XUndefineCursor(disp, xwin);
    switch (c) {
    case 0:
        XDefineCursor(disp, xwin, line_cursor);
        break;
    case 1:
        XDefineCursor(disp, xwin, find_cursor);
        break;
    case 2:
        XDefineCursor(disp, xwin, text_cursor);
        break;
    case 3:
        XDefineCursor(disp, xwin, kill_cursor);
        break;
    case 4:
        XDefineCursor(disp, xwin, move_cursor);
        break;
    case 5:
        XDefineCursor(disp, xwin, wait_cursor);
        break;
    }
    XFlush(disp);
}

void init_cursors(void)
{
    wait_cursor = XCreateFontCursor(disp, XC_watch);
    line_cursor = XCreateFontCursor(disp, XC_crosshair);
    find_cursor = XCreateFontCursor(disp, XC_hand2);
    text_cursor = XCreateFontCursor(disp, XC_xterm);
    kill_cursor = XCreateFontCursor(disp, XC_pirate);
    move_cursor = XCreateFontCursor(disp, XC_fleur);
    cur_cursor = -1;
}


/*
 * put a string in the title bar
 */
void set_title(char *ts)
{
    if (ts == NULL) {
        return;
    }
    
    XtVaSetValues(app_shell, XtNtitle, ts, NULL);
}

/*
 * draw the graph focus indicators
 */
void draw_focus(int gno)
{
    int ix1, iy1, ix2, iy2;
    view v;
    VPoint vp;
    
    if (draw_focus_flag == TRUE) {
	get_graph_viewport(gno, &v);
        vp.x = v.xv1;
        vp.y = v.yv1;
        xlibVPoint2dev(vp, &ix1, &iy1);
        vp.x = v.xv2;
        vp.y = v.yv2;
        xlibVPoint2dev(vp, &ix2, &iy2);
	if (bufpixmap != (Pixmap) NULL) {
            XFillRectangle(disp, bufpixmap, gcxor, ix1 - 5, iy1 - 5, 10, 10);
	    XFillRectangle(disp, bufpixmap, gcxor, ix1 - 5, iy2 - 5, 10, 10);
	    XFillRectangle(disp, bufpixmap, gcxor, ix2 - 5, iy2 - 5, 10, 10);
	    XFillRectangle(disp, bufpixmap, gcxor, ix2 - 5, iy1 - 5, 10, 10);
        }
        XFillRectangle(disp, xwin, gcxor, ix1 - 5, iy1 - 5, 10, 10);
	XFillRectangle(disp, xwin, gcxor, ix1 - 5, iy2 - 5, 10, 10);
	XFillRectangle(disp, xwin, gcxor, ix2 - 5, iy2 - 5, 10, 10);
	XFillRectangle(disp, xwin, gcxor, ix2 - 5, iy1 - 5, 10, 10);
    }
}

/*
 * rubber band line
 */
void select_line(int x1, int y1, int x2, int y2)
{
    XDrawLine(disp, xwin, gcxor, x1, y1, x2, y2);
}


/*
 * draw an xor'ed box
 */
void select_region(int x1, int y1, int x2, int y2)
{
    int dx = x2 - x1;
    int dy = y2 - y1;

    if (dx < 0) {
	iswap(&x1, &x2);
	dx = -dx;
    }
    if (dy < 0) {
	iswap(&y1, &y2);
	dy = -dy;
    }
    XDrawRectangle(disp, xwin, gcxor, x1, y1, dx, dy);
}


/*
 * draw a crosshair cursor
 */
void motion(XMotionEvent * e)
{
    if (e->type != MotionNotify || cursortype == 0) {
	return;
    }
    /* Erase the previous crosshair */
    XDrawLine(disp, xwin, gcxor, 0, cursor_oldy, win_w, cursor_oldy);
    XDrawLine(disp, xwin, gcxor, cursor_oldx, 0, cursor_oldx, win_h);

    /* Draw the new crosshair */
    cursor_oldx = e->x;
    cursor_oldy = e->y;
    XDrawLine(disp, xwin, gcxor, 0, cursor_oldy, win_w, cursor_oldy);
    XDrawLine(disp, xwin, gcxor, cursor_oldx, 0, cursor_oldx, win_h);
}


/*
 * expose/resize proc
 */
void expose_resize(Widget w, XtPointer client_data,
                        XmDrawingAreaCallbackStruct *cbs)
{
    Dimension ww, wh;
    Page_geometry pg;
    static int inc = 0;

#if defined(DEBUG)
    if (debuglevel == 7) {
	printf("Call to expose_resize(); reason == %d\n", cbs->reason);
    }
#endif
    
    /* HACK */
    if (xwin == 0) {
        return;
    }
    
    if (!inc) {
	inwin = TRUE;
	inc++;
        
        if (page_layout == PAGE_FREE) {
            get_canvas_size(&ww, &wh);
            pg = get_page_geometry();
            pg.width = (long) ww;
            pg.height = (long) wh;
            set_page_geometry(pg);
        }
        
        drawgraph();
	
	if (batchfile[0]) {
            getparms(batchfile);
	}
	
	if (inpipe == TRUE) {
	    getdata(get_cg(), "STDIN", SOURCE_STDIN, SET_XY);
	    inpipe = FALSE;
            drawgraph();
	}
        
        return;
    }
    
    if (cbs->reason == XmCR_EXPOSE) {
  	xlibredraw(cbs->window, cbs->event->xexpose.x,
                                cbs->event->xexpose.y,
                                cbs->event->xexpose.width,
                                cbs->event->xexpose.height);
        return;
    }
    
    
    if (page_layout == PAGE_FREE) {
        get_canvas_size(&ww, &wh);
        pg = get_page_geometry();
        pg.width = (long) ww;
        pg.height = (long) wh;
        set_page_geometry(pg);

        drawgraph();
    }
}

/* 
 * draw all active graphs, when graphs are drawn, draw the focus markers
 */
void xdrawgraph(void)
{
    if (inwin && (auto_redraw)) {
	if (cursortype) {
	    cursor_oldx = cursor_oldy = -1;
	}
	set_wait_cursor();
	drawgraph();
	unset_wait_cursor();
    }
}


void xlibredraw(Window window, int x, int y, int width, int height)
{
    if (inwin == TRUE && bufpixmap != (Pixmap) NULL) {
        XCopyArea(disp, bufpixmap, window, gc, x, y, width, height, x, y);
    }
}

Pixmap resize_bufpixmap(unsigned int w, unsigned int h)
{
    static unsigned int pixmap_w = 0, pixmap_h = 0;
    
    if (w == 0 || h == 0) {
        return (bufpixmap);
    }
    
    if (bufpixmap == (Pixmap) NULL) {
        bufpixmap = XCreatePixmap(disp, root, w, h, depth);
    } else if (pixmap_w != w || pixmap_h != h) {
        XFreePixmap(disp, bufpixmap);
        bufpixmap = XCreatePixmap(disp, root, w, h, depth);
    }
    
    if (bufpixmap == (Pixmap) NULL) {
        errmsg("Can't allocate buffer pixmap");
        pixmap_w = 0;
        pixmap_h = 0;
        return (xwin);
    } else {
        pixmap_w = w;
        pixmap_h = h;
        return (bufpixmap);
    }
}

void switch_current_graph(int gno)
{
    int saveg = get_cg();
    
    if (is_graph_hidden(gno) == FALSE) {
        select_graph(gno);
        draw_focus(saveg);
        draw_focus(gno);
        make_format(gno);
        update_all();
        set_graph_selectors(gno);
    }
}

/**********************************************************************
 * XtFlush - Flushes all Xt events.
 **********************************************************************/
/*
 * void XtFlush(void)
 * {
 *     while (XtAppPending(app_con) & XtIMXEvent) {
 *      XtAppProcessEvent(app_con, XtIMXEvent);
 *      XFlush(XtDisplay(app_shell));
 *     }
 * }
 */
