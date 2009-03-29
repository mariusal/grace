/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
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

#include "globals.h"
#include "utils.h"
#include "xprotos.h"

static void resize_drawables(unsigned int w, unsigned int h);

/*
 * put a string in the title bar
 */
void update_app_title(const GProject *gp)
{
    Quark *pr = gproject_get_top(gp);
    GUI *gui = gui_from_quark(pr);
    static char *ts_save = NULL;
    char *ts, *docname;
    static int dstate_save = 0;
    int dstate;
    
    if (!pr || !gui->inwin) {
        return;
    }
    
    dstate = quark_dirtystate_get(pr);
    docname = gproject_get_docname(gp);
    if (!docname) {
        docname = NONAME;
    }
    ts = mybasename(docname);
    if (ts_save == NULL || !strings_are_equal(ts_save, ts) ||
        dstate != dstate_save) {
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
        set_title(buf1, buf2);
        xfree(buf1);
        xfree(buf2);
    }
}

void page_zoom_inout(GraceApp *gapp, int inout)
{
    if (!gui_is_page_free(gapp->gui)) {
        if (inout > 0) {
            gapp->gui->zoom *= ZOOM_STEP;
        } else
        if (inout < 0) {
            gapp->gui->zoom /= ZOOM_STEP;
        } else {
            gapp->gui->zoom = 1.0;
        }
        xdrawgraph(gapp->gp);
        set_left_footer(NULL);
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

void sync_canvas_size(GraceApp *gapp)
{
    X11Stuff *xstuff = gapp->gui->xstuff;
    unsigned int w, h;

    Device_entry *d = get_device_props(grace_get_canvas(gapp->grace), gapp->rt->tdevice);

    GetDimensions(xstuff->canvas, &w, &h);

    set_page_dimensions(gapp, w*72.0/d->pg.dpi, h*72.0/d->pg.dpi, TRUE);
}


/* 
 * redraw all
 */
void xdrawgraph(const GProject *gp)
{
    Quark *project = gproject_get_top(gp);
    GraceApp *gapp = gapp_from_quark(project);
    
    if (gapp && gapp->gui->inwin) {
        X11Stuff *xstuff = gapp->gui->xstuff;
        Quark *gr = graph_get_current(project);
        Device_entry *d = get_device_props(grace_get_canvas(gapp->grace), gapp->rt->tdevice);
        Page_geometry *pg = &d->pg;
        float dpi = gapp->gui->zoom*xstuff->dpi;
        X11stream xstream;
        
        set_wait_cursor();

        if (dpi != pg->dpi) {
            int wpp, hpp;
            project_get_page_dimensions(project, &wpp, &hpp);

            pg->width  = (unsigned long) (wpp*dpi/72);
            pg->height = (unsigned long) (hpp*dpi/72);
            pg->dpi = dpi;
        }
        
        resize_drawables(pg->width, pg->height);
        
        xdrawgrid(xstuff);
        
        init_xstream(&xstream);
        canvas_set_prstream(grace_get_canvas(gapp->grace), &xstream);

        select_device(grace_get_canvas(gapp->grace), gapp->rt->tdevice);
        gproject_render(gp);

        if (quark_is_active(gr)) {
            draw_focus(gr);
        }
        reset_crosshair(gapp->gui, FALSE);
        region_need_erasing = FALSE;

        x11_redraw_all();
#ifndef QT_GUI
        XFlush(xstuff->disp);
#endif
        unset_wait_cursor();
    }
}

static void resize_drawables(unsigned int w, unsigned int h)
{
    X11Stuff *xstuff = gapp->gui->xstuff;
    
    if (w == 0 || h == 0) {
        return;
    }
    
    if (xstuff->bufpixmap == (Pixmap) NULL) {
        create_pixmap(w, h);
    } else if (xstuff->win_w != w || xstuff->win_h != h) {
        recreate_pixmap(w, h);
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
    
    if (!gui_is_page_free(gapp->gui)) {
        SetDimensions(xstuff->canvas, xstuff->win_w, xstuff->win_h);
    }
}

static int x11_convx(double x)
{
    X11Stuff *xstuff = gapp->gui->xstuff;
    return ((int) rint(xstuff->win_scale * x));
}

static int x11_convy(double y)
{
    X11Stuff *xstuff = gapp->gui->xstuff;
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
    X11Stuff *xstuff = gapp->gui->xstuff;
    if (xstuff->win_scale == 0) {
        vp->x = (double) 0.0;
        vp->y = (double) 0.0;
    } else {
        vp->x = (double) x / xstuff->win_scale;
        vp->y = (double) (xstuff->win_h - y) / xstuff->win_scale;
    }
}

/*
 * for the goto point feature
 */
void setpointer(VPoint vp)
{
    X11Stuff *xstuff = gapp->gui->xstuff;
    short x, y;
    
    x11_VPoint2dev(&vp, &x, &y);
    
    /* Make sure we remain inside the DA widget dimensions */
    x = MAX2(x, 0);
    x = MIN2(x, xstuff->win_w);
    y = MAX2(y, 0);
    y = MIN2(y, xstuff->win_h);
    
    move_pointer(x, y);
}

/*
 * set the message in the left footer
 */
void set_left_footer(char *s)
{
    Widget statlab = gapp->gui->mwui->statlab;

    if (s == NULL) {
        char hbuf[64], buf[GR_MAXPATHLEN + 100], *prname;
        gethostname(hbuf, 63);
        prname = gproject_get_docname(gapp->gp);
        if (prname) {
            sprintf(buf, "%s, %s, %s, %d%%", hbuf, display_name(gapp->gui),
                prname, (int) rint(100*gapp->gui->zoom));
        } else {
            sprintf(buf, "%s, %s", hbuf, display_name(gapp->gui));
        }
        SetLabel(statlab, buf);
    } else {
        SetLabel(statlab, s);
    }
#ifndef QT_GUI
    XmUpdateDisplay(statlab);
#endif
}

void set_tracker_string(char *s)
{
    Widget loclab = gapp->gui->mwui->loclab;
    
    if (s == NULL) {
        SetLabel(loclab, "[Out of frame]");
    } else {
        SetLabel(loclab, s);
    }
}

