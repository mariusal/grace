/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
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

#include <config.h>

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <X11/Xlib.h>
#include <X11/cursorfont.h>

#include "defines.h"
#include "globals.h"

#include "utils.h"
#include "files.h"
#include "core_utils.h"
#include "plotone.h"

#include "motifinc.h"
#include "protos.h"

extern Widget app_shell;
extern XtAppContext app_con;

extern Input_buffer *ib_tbl;
extern int ib_tblsize;

static GC gcxor;

static void resize_drawables(unsigned int w, unsigned int h);

long x11_allocate_color(GUI *gui, const RGB *rgb)
{
    X11Stuff *xstuff = gui->xstuff;
    XColor xc;
    
    xc.pixel = 0;
    xc.flags = DoRed | DoGreen | DoBlue;
    
    xc.red   = rgb->red   << (16 - GRACE_BPP);
    xc.green = rgb->green << (16 - GRACE_BPP);
    xc.blue  = rgb->blue  << (16 - GRACE_BPP);

    if (XAllocColor(xstuff->disp, xstuff->cmap, &xc)) {
        return xc.pixel;
    } else
    if (gui->install_cmap != CMAP_INSTALL_NEVER && 
        gui->private_cmap == FALSE) {
        xstuff->cmap = XCopyColormapAndFree(xstuff->disp, xstuff->cmap);
        gui->private_cmap = TRUE;
        
        /* try to allocate the same color in the private colormap */
        return x11_allocate_color(gui, rgb);
    } else {
        return -1;
    }
}

void set_wait_cursor(void)
{
    if (grace->gui->xstuff->disp == NULL) {
        return;
    }
    
    DefineDialogCursor(grace->gui->xstuff->wait_cursor);
}

void unset_wait_cursor(void)
{
    X11Stuff *xstuff = grace->gui->xstuff;
    if (xstuff->disp == NULL) {
        return;
    }
    
    UndefineDialogCursor();
    if (xstuff->cur_cursor >= 0) {
        set_cursor(grace->gui, xstuff->cur_cursor);
    }
}

void set_cursor(GUI *gui, int c)
{
    X11Stuff *xstuff = gui->xstuff;
    if (xstuff->disp == NULL || xstuff->cur_cursor == c) {
        return;
    }

    XUndefineCursor(xstuff->disp, xstuff->xwin);
    xstuff->cur_cursor = c;
    switch (c) {
    case 0:
        XDefineCursor(xstuff->disp, xstuff->xwin, xstuff->line_cursor);
        break;
    case 1:
        XDefineCursor(xstuff->disp, xstuff->xwin, xstuff->find_cursor);
        break;
    case 2:
        XDefineCursor(xstuff->disp, xstuff->xwin, xstuff->text_cursor);
        break;
    case 3:
        XDefineCursor(xstuff->disp, xstuff->xwin, xstuff->kill_cursor);
        break;
    case 4:
        XDefineCursor(xstuff->disp, xstuff->xwin, xstuff->move_cursor);
        break;
    case 5:
        XDefineCursor(xstuff->disp, xstuff->xwin, xstuff->drag_cursor);
        break;
    default:
        xstuff->cur_cursor = -1;
        break;
    }
    XFlush(xstuff->disp);
}

void init_cursors(GUI *gui)
{
    X11Stuff *xstuff = gui->xstuff;

    xstuff->wait_cursor = XCreateFontCursor(xstuff->disp, XC_watch);
    xstuff->line_cursor = XCreateFontCursor(xstuff->disp, XC_crosshair);
    xstuff->find_cursor = XCreateFontCursor(xstuff->disp, XC_dotbox);
    xstuff->text_cursor = XCreateFontCursor(xstuff->disp, XC_xterm);
    xstuff->kill_cursor = XCreateFontCursor(xstuff->disp, XC_pirate);
    xstuff->move_cursor = XCreateFontCursor(xstuff->disp, XC_fleur);
    xstuff->drag_cursor = XCreateFontCursor(xstuff->disp, XC_hand2);
    
    xstuff->cur_cursor = -1;
}


/*
 * put a string in the title bar
 */
void set_title(const Quark *pr)
{
    GUI *gui = gui_from_quark(pr);
    static char *ts_save = NULL;
    char *ts;
    static int dstate_save = 0;
    int dstate = quark_dirtystate_get(pr);
    
    
    if (!pr || !gui->inwin) {
        return;
    }
    
    dstate = quark_dirtystate_get(pr);
    ts = mybasename(project_get_docname(pr));
    if (ts_save == NULL || strcmp(ts_save, ts) != 0 || dstate != dstate_save) {
        char *buf1, *buf2;
        ts_save = copy_string(ts_save, ts);
        dstate_save = dstate;
        buf1 = copy_string(NULL, "Grace: ");
        buf1 = concat_strings(buf1, ts);
        buf2 = copy_string(NULL, ts);
        if (dstate) {
            buf2 = concat_strings(buf2, "*");
            buf1 = concat_strings(buf1, " (modified)");
        }
        XtVaSetValues(app_shell, XtNtitle, buf1, XtNiconName, buf2, NULL);
        xfree(buf1);
        xfree(buf2);
    }
}

void page_zoom_inout(Grace *grace, int inout)
{
    if (!gui_is_page_free(grace->gui)) {
        if (inout > 0) {
            grace->gui->zoom *= ZOOM_STEP;
        } else
        if (inout < 0) {
            grace->gui->zoom /= ZOOM_STEP;
        } else {
            grace->gui->zoom = 1.0;
        }
        xdrawgraph(grace->project, TRUE);
        set_left_footer(NULL);
    }
}

/*
 *  Auxiliary routines for simultaneous drawing on display and pixmap
 */
static void aux_XDrawLine(GUI *gui, int x1, int y1, int x2, int y2)
{
    X11Stuff *xstuff = gui->xstuff;
    XDrawLine(xstuff->disp, xstuff->xwin, gcxor, x1, y1, x2, y2);
    if (xstuff->bufpixmap != (Pixmap) NULL) {
        XDrawLine(xstuff->disp, xstuff->bufpixmap, gcxor, x1, y1, x2, y2);
    }
}

static void aux_XDrawRectangle(GUI *gui, int x1, int y1, int x2, int y2)
{
    X11Stuff *xstuff = gui->xstuff;
    XDrawRectangle(xstuff->disp, xstuff->xwin, gcxor, x1, y1, x2, y2);
    if (xstuff->bufpixmap != (Pixmap) NULL) {
        XDrawRectangle(xstuff->disp, xstuff->bufpixmap, gcxor, x1, y1, x2, y2);
    }
}

static void aux_XFillRectangle(GUI *gui, int x, int y, unsigned int width, unsigned int height)
{
    X11Stuff *xstuff = gui->xstuff;
    XFillRectangle(xstuff->disp, xstuff->xwin, gcxor, x, y, width, height);
    if (xstuff->bufpixmap != (Pixmap) NULL) {
        XFillRectangle(xstuff->disp, xstuff->bufpixmap, gcxor, x, y, width, height);
    }
}


/*
 * draw the graph focus indicators
 */
void draw_focus(Quark *gr)
{
    short ix1, iy1, ix2, iy2;
    view v;
    VPoint vp;
    GUI *gui = gui_from_quark(gr);
    
    if (gui->draw_focus_flag == TRUE) {
        graph_get_viewport(gr, &v);
        vp.x = v.xv1;
        vp.y = v.yv1;
        x11_VPoint2dev(&vp, &ix1, &iy1);
        vp.x = v.xv2;
        vp.y = v.yv2;
        x11_VPoint2dev(&vp, &ix2, &iy2);
        aux_XFillRectangle(gui, ix1 - 5, iy1 - 5, 10, 10);
        aux_XFillRectangle(gui, ix1 - 5, iy2 - 5, 10, 10);
        aux_XFillRectangle(gui, ix2 - 5, iy2 - 5, 10, 10);
        aux_XFillRectangle(gui, ix2 - 5, iy1 - 5, 10, 10);
        
        gui->xstuff->f_x1 = ix1;
        gui->xstuff->f_x2 = ix2;
        gui->xstuff->f_y1 = iy1;
        gui->xstuff->f_y2 = iy2;
        
        gui->xstuff->f_v  = v;
    }
}

/*
 * rubber band line (optionally erasing previous one)
 */
void select_line(GUI *gui, int x1, int y1, int x2, int y2, int erase)
{
    static int x1_old, y1_old, x2_old, y2_old;

    if (erase) {
        aux_XDrawLine(gui, x1_old, y1_old, x2_old, y2_old);
    }
    x1_old = x1;
    y1_old = y1;
    x2_old = x2;
    y2_old = y2;
    aux_XDrawLine(gui, x1, y1, x2, y2);
}

static int region_need_erasing = FALSE;
/*
 * draw an xor'ed box (optionally erasing previous one)
 */
void select_region(GUI *gui, int x1, int y1, int x2, int y2, int erase)
{
    static int x1_old, y1_old, dx_old, dy_old;
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
    if (erase && region_need_erasing) {
        aux_XDrawRectangle(gui, x1_old, y1_old, dx_old, dy_old);
    }
    x1_old = x1;
    y1_old = y1;
    dx_old = dx;
    dy_old = dy;
    aux_XDrawRectangle(gui, x1, y1, dx, dy);
    region_need_erasing = TRUE;
}

void select_vregion(GUI *gui, int x1, int x2, int erase)
{
    X11Stuff *xstuff = gui->xstuff;
    
    select_region(gui, x1, xstuff->f_y1, x2, xstuff->f_y2, erase);
}

void select_hregion(GUI *gui, int y1, int y2, int erase)
{
    X11Stuff *xstuff = gui->xstuff;
    
    select_region(gui, xstuff->f_x1, y1, xstuff->f_x2, y2, erase);
}

/*
 * slide an xor'ed bbox shifted by shift_*, (optionally erasing previous one)
 */
void slide_region(GUI *gui, view bb, int shift_x, int shift_y, int erase)
{
    short x1, x2, y1, y2;
    VPoint vp;

    vp.x = bb.xv1;
    vp.y = bb.yv1;
    x11_VPoint2dev(&vp, &x1, &y1);
    x1 += shift_x;
    y1 += shift_y;
    
    vp.x = bb.xv2;
    vp.y = bb.yv2;
    x11_VPoint2dev(&vp, &x2, &y2);
    x2 += shift_x;
    y2 += shift_y;
    
    select_region(gui, x1, y1, x2, y2, erase);
}

void resize_region(GUI *gui, view bb, int on_focus,
    int shift_x, int shift_y, int erase)
{
    short x1, x2, y1, y2;
    VPoint vp;

    vp.x = bb.xv1;
    vp.y = bb.yv1;
    x11_VPoint2dev(&vp, &x1, &y1);
    vp.x = bb.xv2;
    vp.y = bb.yv2;
    x11_VPoint2dev(&vp, &x2, &y2);

    switch (on_focus) {
    case 1:
        x1 += shift_x;
        y1 += shift_y;
        break;
    case 2:
        x1 += shift_x;
        y2 += shift_y;
        break;
    case 3:
        x2 += shift_x;
        y2 += shift_y;
        break;
    case 4:
        x2 += shift_x;
        y1 += shift_y;
        break;
    default:
        return;
    }
    
    select_region(gui, x1, y1, x2, y2, erase);
}

static int crosshair_erase = FALSE;
static int cursor_oldx, cursor_oldy;


void reset_crosshair(GUI *gui, int clear)
{
    X11Stuff *xstuff = gui->xstuff;
    crosshair_erase = FALSE;
    if (clear) {
        aux_XDrawLine(gui, xstuff->f_x1, cursor_oldy, xstuff->f_x2, cursor_oldy);
        aux_XDrawLine(gui, cursor_oldx, xstuff->f_y1, cursor_oldx, xstuff->f_y2);
    }
}

/*
 * draw a crosshair cursor
 */
void crosshair_motion(GUI *gui, int x, int y)
{
    X11Stuff *xstuff = gui->xstuff;
    
    /* Erase the previous crosshair */
    if (crosshair_erase == TRUE) {
        aux_XDrawLine(gui, xstuff->f_x1, cursor_oldy, xstuff->f_x2, cursor_oldy);
        aux_XDrawLine(gui, cursor_oldx, xstuff->f_y1, cursor_oldx, xstuff->f_y2);
    }

    if (x < xstuff->f_x1 || x > xstuff->f_x2 ||
        y < xstuff->f_y2 || y > xstuff->f_y1) {
        crosshair_erase = FALSE;
        return;
    }
    
    /* Draw the new crosshair */
    aux_XDrawLine(gui, xstuff->f_x1, y, xstuff->f_x2, y);
    aux_XDrawLine(gui, x, xstuff->f_y1, x, xstuff->f_y2);
    crosshair_erase = TRUE;
    cursor_oldx = x;
    cursor_oldy = y;
}

void sync_canvas_size(Grace *grace)
{
    X11Stuff *xstuff = grace->gui->xstuff;
    unsigned int w, h;

    Device_entry *d = get_device_props(grace->rt->canvas, grace->rt->tdevice);

    GetDimensions(xstuff->canvas, &w, &h);

    set_page_dimensions(grace, w*72.0/d->pg.dpi, h*72.0/d->pg.dpi, TRUE);
}


/*
 * expose/resize proc
 */
void expose_resize(Widget w, XtPointer client_data, XtPointer call_data)
{
    Grace *grace = (Grace *) client_data;
    static int inc = FALSE;
    XmDrawingAreaCallbackStruct *cbs = (XmDrawingAreaCallbackStruct *) call_data;

#if defined(DEBUG)
    if (get_debuglevel(grace) == 7) {
	printf("Call to expose_resize(); reason == %d\n", cbs->reason);
    }
#endif
    
    /* HACK */
    if (!grace->gui->inwin) {
        return;
    }
    
    if (!inc) {
	inc = TRUE;
        
        update_all();
        xdrawgraph(grace->project, TRUE);

        return;
    }
    
    if (cbs->reason == XmCR_EXPOSE) {
  	x11_redraw(cbs->window,
            cbs->event->xexpose.x,
            cbs->event->xexpose.y,
            cbs->event->xexpose.width,
            cbs->event->xexpose.height);
    } else {
        if (gui_is_page_free(grace->gui)) {
            sync_canvas_size(grace);
            xdrawgraph(grace->project, TRUE);
        }
    }
}

/* 
 * redraw all
 */
void xdrawgraph(const Quark *q, int force)
{
    Quark *project = get_parent_project(q);
    Grace *grace = grace_from_quark(q);
    X11Stuff *xstuff = grace->gui->xstuff;
    
    if (grace && grace->gui->inwin && (force || grace->gui->auto_redraw)) {
        Quark *gr = graph_get_current(project);
        Device_entry *d = get_device_props(grace->rt->canvas, grace->rt->tdevice);
        
        set_wait_cursor();

        device_set_dpi(d, grace->gui->zoom*xstuff->dpi, TRUE);
        
        resize_drawables(d->pg.width, d->pg.height);

        select_device(grace->rt->canvas, grace->rt->tdevice);
	drawgraph(project);

        if (graph_is_active(gr)) {
            draw_focus(gr);
        }
        reset_crosshair(grace->gui, FALSE);
        region_need_erasing = FALSE;

        x11_redraw(xstuff->xwin, 0, 0, xstuff->win_w, xstuff->win_h);

        XFlush(xstuff->disp);

	unset_wait_cursor();
    }
}


void x11_redraw(Window window, int x, int y, int width, int height)
{
    X11Stuff *xstuff = grace->gui->xstuff;
    if (grace->gui->inwin == TRUE && xstuff->bufpixmap != (Pixmap) NULL) {
        XCopyArea(xstuff->disp, xstuff->bufpixmap, window, xstuff->gc, x, y, width, height, x, y);
    }
}

static void resize_drawables(unsigned int w, unsigned int h)
{
    X11Stuff *xstuff = grace->gui->xstuff;
    
    if (w == 0 || h == 0) {
        return;
    }
    
    if (xstuff->bufpixmap == (Pixmap) NULL) {
        xstuff->bufpixmap = XCreatePixmap(xstuff->disp, xstuff->root, w, h, xstuff->depth);
    } else if (xstuff->win_w != w || xstuff->win_h != h) {
        XFreePixmap(xstuff->disp, xstuff->bufpixmap);
        xstuff->bufpixmap = XCreatePixmap(xstuff->disp, xstuff->root, w, h, xstuff->depth);
    }
    
    if (xstuff->bufpixmap == (Pixmap) NULL) {
        errmsg("Can't allocate buffer pixmap");
        xstuff->win_w = 0;
        xstuff->win_h = 0;
    } else {
        xstuff->win_w = w;
        xstuff->win_h = h;
    }

    xstuff->win_scale = MIN2(xstuff->win_w, xstuff->win_h);
    
    if (!gui_is_page_free(grace->gui)) {
        SetDimensions(xstuff->canvas, xstuff->win_w, xstuff->win_h);
    }
}

static void xmonitor_rti(XtPointer ib, int *ptrFd, XtInputId *ptrId)
{
    set_wait_cursor();
    
    monitor_input(grace, (Input_buffer *) ib, 1, 1);
    
    unset_wait_cursor();
}

void xunregister_rti(XtInputId id)
{
    if (grace->gui->inwin) {
        /* the screen has been initialized : we can remove the buffer */
        XtRemoveInput(id);
    }
}

void xregister_rti(Input_buffer *ib)
{
    if (grace->gui->inwin) {
        /* the screen has been initialized : we can register the buffer */
        ib->id = (unsigned long) XtAppAddInput(app_con,
                                               ib->fd,
                                               (XtPointer) XtInputReadMask,
                                               xmonitor_rti,
                                               (XtPointer) ib);
    }
}

/*
 * for the goto point feature
 */
void setpointer(VPoint vp)
{
    X11Stuff *xstuff = grace->gui->xstuff;
    short x, y;
    
    x11_VPoint2dev(&vp, &x, &y);
    
    /* Make sure we remain inside the DA widget dimensions */
    x = MAX2(x, 0);
    x = MIN2(x, xstuff->win_w);
    y = MAX2(y, 0);
    y = MIN2(y, xstuff->win_h);
    
    XWarpPointer(xstuff->disp, None, xstuff->xwin, 0, 0, 0, 0, x, y);
}

char *display_name(GUI *gui)
{
    return DisplayString(gui->xstuff->disp);
}

#define BUFSIZE 1024
static int HandleXError(Display *dpy, XErrorEvent *event)
{
    char buffer[BUFSIZE], mesg[BUFSIZE], *output;

    char *mtype = "XlibMessage";

    XGetErrorDatabaseText(dpy, mtype, "XError", "X Error", buffer, BUFSIZE);
    output = copy_string(NULL, buffer);
    output = concat_strings(output, ":");
    
    XGetErrorText(dpy, event->error_code, buffer, BUFSIZE);
    output = concat_strings(output, buffer);
    output = concat_strings(output, "\n");
    
    XGetErrorDatabaseText(dpy, mtype, "MajorCode", "Request Major code %d", mesg,
                          BUFSIZE);
    sprintf(buffer, mesg, event->request_code);
    output = concat_strings(output, buffer);

    if (event->request_code < 128) {
        char number[32];
        sprintf(number, "%d", event->request_code);
        XGetErrorDatabaseText(dpy, "XRequest", number, "", mesg, BUFSIZE);
        sprintf(buffer, " (%s)\n", mesg);
        output = concat_strings(output, buffer);
    }

    switch (event->error_code) {
    case (BadWindow):
    case (BadPixmap):
    case (BadCursor):
    case (BadFont):
    case (BadDrawable):
    case (BadColor):
    case (BadGC):
    case (BadIDChoice):
    case (BadValue):
    case (BadAtom):
        if (event->error_code == BadValue) {
           XGetErrorDatabaseText(dpy, mtype, "Value", "Value 0x%x", mesg,
                                 BUFSIZE);
        } else if (event->error_code == BadAtom) {
           XGetErrorDatabaseText(dpy, mtype, "AtomID", "AtomID 0x%x", mesg,
                                 BUFSIZE);
        } else {
           XGetErrorDatabaseText(dpy, mtype, "ResourceID", "ResourceID 0x%x",
                                 mesg, BUFSIZE);
        }
        output = concat_strings(output, "  ");
        sprintf(buffer, mesg, event->resourceid);
        output = concat_strings(output, buffer);
        output = concat_strings(output, "\n");
        break;
    } /* switch() */

    XGetErrorDatabaseText(dpy, mtype, "ErrorSerial", "Error Serial #%d", mesg,
                           BUFSIZE);
    sprintf(buffer, mesg, event->serial);
    output = concat_strings(output, "  ");
    output = concat_strings(output, buffer);
    output = concat_strings(output, ".");
    
    emergency_exit(grace, TRUE, output);
    xfree(output);
    
    /* return value is ignored anyway */
    return 0;
}

/*
 * Interrupt handler for X IO errors
 */
static int HandleXIOError(Display *d)
{
    char msg[BUFSIZE];      
    if (errno == EPIPE) {
        sprintf(msg, "X connection to %s broken (server error - EPIPE).",
               DisplayString(d));
    } else {
        sprintf(msg, "Fatal IO error on X server %s.", DisplayString(d));
    }

    emergency_exit(grace, FALSE, msg);
    
    /* Ideally, we don't reach this anyway ... */
    return 1;
}

void installXErrorHandler(void)
{
    XSetErrorHandler(HandleXError);
    XSetIOErrorHandler(HandleXIOError);
}

int x11_init(Grace *grace)
{
    X11Stuff *xstuff = grace->gui->xstuff;
    XGCValues gc_val;
    long mrsize;
    int max_path_limit;
    
    xstuff->screennumber = DefaultScreen(xstuff->disp);
    xstuff->root = RootWindow(xstuff->disp, xstuff->screennumber);
 
    xstuff->gc = DefaultGC(xstuff->disp, xstuff->screennumber);
    
    xstuff->depth = DisplayPlanes(xstuff->disp, xstuff->screennumber);

    /* init colormap */
    xstuff->cmap = DefaultColormap(xstuff->disp, xstuff->screennumber);
    /* redefine colormap, if needed */
    if (grace->gui->install_cmap == CMAP_INSTALL_ALWAYS) {
        xstuff->cmap = XCopyColormapAndFree(xstuff->disp, xstuff->cmap);
        grace->gui->private_cmap = TRUE;
    }
    
    /* set GCs */
    if (grace->gui->invert) {
        gc_val.function = GXinvert;
    } else {
        gc_val.function = GXxor;
    }
    gcxor = XCreateGC(xstuff->disp, xstuff->root, GCFunction, &gc_val);

    /* XExtendedMaxRequestSize() appeared in X11R6 */
#if XlibSpecificationRelease > 5
    mrsize = XExtendedMaxRequestSize(xstuff->disp);
#else
    mrsize = 0;
#endif
    if (mrsize <= 0) {
        mrsize = XMaxRequestSize(xstuff->disp);
    }
    max_path_limit = (mrsize - 3)/2;
    if (max_path_limit < get_max_path_limit(grace->rt->canvas)) {
        char buf[128];
        sprintf(buf,
            "Setting max drawing path length to %d (limited by the X server)",
            max_path_limit);
        errmsg(buf);
        set_max_path_limit(grace->rt->canvas, max_path_limit);
    }
    
    xstuff->dpi = rint(MM_PER_INCH*DisplayWidth(xstuff->disp, xstuff->screennumber)/
        DisplayWidthMM(xstuff->disp, xstuff->screennumber));

    return RETURN_SUCCESS;
}


static int x11_convx(double x)
{
    X11Stuff *xstuff = grace->gui->xstuff;
    return ((int) rint(xstuff->win_scale * x));
}

static int x11_convy(double y)
{
    X11Stuff *xstuff = grace->gui->xstuff;
    return ((int) rint(xstuff->win_h - xstuff->win_scale * y));
}

void x11_VPoint2dev(const VPoint *vp, short *x, short *y)
{
    *x = x11_convx(vp->x);
    *y = x11_convy(vp->y);
}

/*
 * x11_dev2VPoint - given (x,y) in screen coordinates, return the 
 * viewport coordinates
 */
void x11_dev2VPoint(short x, short y, VPoint *vp)
{
    X11Stuff *xstuff = grace->gui->xstuff;
    if (xstuff->win_scale == 0) {
        vp->x = (double) 0.0;
        vp->y = (double) 0.0;
    } else {
        vp->x = (double) x / xstuff->win_scale;
        vp->y = (double) (xstuff->win_h - y) / xstuff->win_scale;
    }
}

Pixmap char_to_pixmap(Widget w, int font, char c, int csize)
{
    X11Stuff *xstuff = grace->gui->xstuff;
    CPixmap *pm;
    Pixmap pixmap = 0;
    int height, width, hshift, vshift;
    float fsize = 0.8*(float)csize;
    
    pm = canvas_raster_char(font, c, fsize, &vshift, &hshift);
       
    if (pm != NULL && pm->bits != NULL) {
        long bg, fg;
        Pixmap ptmp;
        
        vshift = csize - vshift - 4;
        height = pm->height;
        width = pm->width;
        
        ptmp = XCreateBitmapFromData(xstuff->disp, xstuff->root,
                    pm->bits, width, height);
        pixmap = XCreatePixmap(xstuff->disp, xstuff->root,
            csize, csize, xstuff->depth);
        
        XtVaGetValues(w, XmNbackground, &bg, XmNforeground, &fg, NULL);
        XSetForeground(xstuff->disp, xstuff->gc, bg);
        XFillRectangle(xstuff->disp, pixmap, xstuff->gc, 0, 0, csize, csize);
        
        XSetBackground(xstuff->disp, xstuff->gc, bg);
        XSetForeground(xstuff->disp, xstuff->gc, fg);
        XCopyPlane(xstuff->disp, ptmp, pixmap, xstuff->gc, 0, 0,
            width, height, hshift, vshift, 1);
        
        XFreePixmap(xstuff->disp, ptmp);
        canvas_cpixmap_free(pm);
    }
    
    return pixmap;
}
