/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
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
 * event handler
 *
 */

#include <config.h>

#include <stdlib.h>

#include "utils.h"
#include "core_utils.h"
#include "events.h"

#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/Intrinsic.h>
#include <X11/keysym.h>
#include <Xm/ScrollBar.h>
#include <Xm/RowColumn.h>

#include "motifinc.h"
#include "protos.h"

static void scroll_bar_pix(Widget bar, int pix)
{
    int value, slider_size, maxvalue;

    XtVaGetValues(bar,
        XmNvalue,      &value,
        XmNmaximum,    &maxvalue,
        XmNsliderSize, &slider_size,
        NULL);
    value += pix;
    if (value < 0) {
        value = 0;
    } else
    if (value > maxvalue - slider_size) {
        value = maxvalue - slider_size;
    }
    XmScrollBarSetValues(bar, value, 0, 0, 0, True);
}

static void scroll_pix(Widget w, int dx, int dy)
{
    Widget bar;
    
    if (dx && (bar = XtNameToWidget(w, "HorScrollBar"))) {
        scroll_bar_pix(bar, dx);
    }
    if (dy && (bar = XtNameToWidget(w, "VertScrollBar"))) {
        scroll_bar_pix(bar, dy);
    }
}
static void scroll(Widget w, int up, int horiz)
{
    int value, slider_size, increment, page_increment, maxvalue;
    Widget vbar;
    
    if (horiz) {
        vbar = XtNameToWidget(w, "HorScrollBar");
    } else {
        vbar = XtNameToWidget(w, "VertScrollBar");
    }
    
    if (!vbar) {
        return;
    }
    
    XmScrollBarGetValues(vbar, &value, &slider_size,
        &increment, &page_increment);
    if (up) {
        value -= increment;
        if (value < 0) {
            value = 0;
        }
    } else {
        XtVaGetValues(vbar, XmNmaximum, &maxvalue, NULL);
        value += increment;
        if (value > maxvalue - slider_size) {
            value = maxvalue - slider_size;
        }
    }
    XmScrollBarSetValues(vbar, value, 0, 0, 0, True);
}

typedef struct {
    VPoint vp;
    int include_graphs;
    Quark *q;
    int part;
    view bbox;
    int found;
} canvas_target;

static void target_consider(canvas_target *ct, Quark *q, int part,
    const view *v)
{
    if (is_vpoint_inside(v, &ct->vp, 0.0)) {
        ct->q = q;
        ct->part = part;
        ct->bbox = *v;
        ct->found = TRUE;
    }
}

static int target_hook(Quark *q, void *udata, QTraverseClosure *closure)
{
    canvas_target *ct = (canvas_target *) udata;
    view v;
    AText *at;
    DObject *o;
    double *x, *y;
    
    if (!quark_is_active(q)) {
        closure->descend = FALSE;
        return TRUE;
    }
    
    switch (quark_fid_get(q)) {
    case QFlavorFrame:
        {
            legend *l;

            frame_get_view(q, &v);
            target_consider(ct, q, 0, &v);

            if ((l = frame_get_legend(q)) && l->active) {
                target_consider(ct, q, 1, &l->bb);
            }
        }
        break;
    case QFlavorGraph:
        if (ct->include_graphs) {
            Grace *grace = grace_from_quark(q);
            Quark *cg = graph_get_current(grace->project);
            if (cg == q && graph_get_viewport(q, &v) == RETURN_SUCCESS) {
                VPoint vp;
                GLocator *locator;
                
                target_consider(ct, q, 0, &v);
                
                locator = graph_get_locator(cg);
                Wpoint2Vpoint(cg, &locator->origin, &vp);
                v.xv1 = v.xv2 = vp.x;
                v.yv1 = v.yv2 = vp.y;
                view_extend(&v, 0.01);
                target_consider(ct, q, 1, &v);
            }
        }
        break;
    case QFlavorAText:
        at = atext_get_data(q);
        {
            VPoint vp;
            target_consider(ct, q, 0, &at->bb);
            
            if (at->arrow_flag &&
                Apoint2Vpoint(q, &at->ap, &vp) == RETURN_SUCCESS) {
                double arrsize = MAX2(0.01*at->arrow.length, 0.005);
                v.xv1 = v.xv2 = vp.x;
                v.yv1 = v.yv2 = vp.y;
                view_extend(&v, arrsize);
                target_consider(ct, q, 1, &v);
            }
        }
        break;
    case QFlavorAxis:
        if (axis_get_bb(q, &v) == RETURN_SUCCESS) {
            target_consider(ct, q, 0, &v);
        }
        break;
    case QFlavorDObject:
        o = object_get_data(q);
        target_consider(ct, q, 0, &o->bb);
        break;
    case QFlavorSet:
        x = set_get_col(q, DATA_X);
        y = set_get_col(q, DATA_Y);
        if (x && y) {
            int i;
            WPoint wp;
            VPoint vp;
            set *p = set_get_data(q);
            double symsize = MAX2(0.01*p->sym.size, 0.005);
            for (i = 0; i < set_get_length(q); i++) {
                wp.x = x[i];
                wp.y = y[i];
                Wpoint2Vpoint(q, &wp, &vp);
                v.xv1 = v.xv2 = vp.x;
                v.yv1 = v.yv2 = vp.y;
                view_extend(&v, symsize);
                target_consider(ct, q, i, &v);
            }
        }
        break;
    }
    
    return TRUE;
}

static int find_target(Quark *pr, canvas_target *ct)
{
    ct->found = FALSE;
    quark_traverse(pr, target_hook, ct);

    return ct->found ? RETURN_SUCCESS:RETURN_FAILURE;
}

static void move_target(canvas_target *ct, const VPoint *vp)
{
    VVector vshift;
    
    vshift.x = vp->x - ct->vp.x;
    vshift.y = vp->y - ct->vp.y;
    
    switch (quark_fid_get(ct->q)) {
    case QFlavorFrame:
        switch (ct->part) {
        case 0:
            frame_shift(ct->q, &vshift);
            break;
        case 1:
            frame_legend_shift(ct->q, &vshift);
            break;
        }
        break;
    case QFlavorAText:
        switch (ct->part) {
        case 0:
            atext_shift(ct->q, &vshift);
            break;
        case 1:
            atext_at_shift(ct->q, &vshift);
            break;
        }
        break;
    case QFlavorAxis:
        axis_shift(ct->q, &vshift);
        break;
    case QFlavorDObject:
        object_shift(ct->q, &vshift);
        break;
    case QFlavorSet:
        set_point_shift(ct->q, ct->part, &vshift);
        break;
    }
}

static Widget popup = NULL, poplab, drop_pt_bt, as_set_bt, edit_menu, atext_bt;
static Widget set_locator_bt, clear_locator_bt;
static Widget bring_to_front_bt, move_up_bt, move_down_bt, send_to_back_bt;

#define EDIT_CB              0
#define HIDE_CB              1
#define DELETE_CB            2
#define DUPLICATE_CB         3
#define BRING_TO_FRONT_CB    4
#define SEND_TO_BACK_CB      5
#define MOVE_UP_CB           6
#define MOVE_DOWN_CB         7
#define DROP_POINT_CB        8
#define EDITDATA_S_CB        9
#define EDITDATA_E_CB       10
#define AUTOSCALE_BY_SET_CB 11

static void popup_any_cb(canvas_target *ct, int type)
{
    Quark *q = ct->q;
    Quark *pr = get_parent_project(q);
    
    switch (type) {
    case EDIT_CB:
        raise_explorer(gui_from_quark(q), q);
        return;
        break;
    case HIDE_CB:
        quark_set_active(q, FALSE);
        break;
    case DELETE_CB:
        quark_free(q);
        break;
    case DUPLICATE_CB:
        quark_copy(q);
        break;
    case BRING_TO_FRONT_CB:
        quark_push(q, TRUE);
        break;
    case SEND_TO_BACK_CB:
        quark_push(q, FALSE);
        break;
    case MOVE_UP_CB:
        quark_move(q, TRUE);
        break;
    case MOVE_DOWN_CB:
        quark_move(q, FALSE);
        break;
    case DROP_POINT_CB:
        if (quark_fid_get(q) == QFlavorSet && ct->part >= 0) {
            droppoints(q, ct->part, ct->part);
        }
        break;
    case AUTOSCALE_BY_SET_CB:
        if (quark_fid_get(q) == QFlavorSet) {
            autoscale_bysets(&q, 1, AUTOSCALE_XY);
        }
        break;
    case EDITDATA_S_CB:
        create_ss_frame(q);
        break;
    case EDITDATA_E_CB:
        do_ext_editor(q);
        break;
    }
    
    xdrawgraph(pr, TRUE);
    update_all();
}

static void edit_cb(Widget but, void *udata)
{
    popup_any_cb((canvas_target *) udata, EDIT_CB);
}

static void hide_cb(Widget but, void *udata)
{
    popup_any_cb((canvas_target *) udata, HIDE_CB);
}

static void delete_cb(Widget but, void *udata)
{
    if (yesno("Really delete this item?", NULL, NULL, NULL)) {
        popup_any_cb((canvas_target *) udata, DELETE_CB);
    }
}

static void duplicate_cb(Widget but, void *udata)
{
    popup_any_cb((canvas_target *) udata, DUPLICATE_CB);
}

static void bring_to_front_cb(Widget but, void *udata)
{
    popup_any_cb((canvas_target *) udata, BRING_TO_FRONT_CB);
}

static void send_to_back_cb(Widget but, void *udata)
{
    popup_any_cb((canvas_target *) udata, SEND_TO_BACK_CB);
}

static void move_up_cb(Widget but, void *udata)
{
    popup_any_cb((canvas_target *) udata, MOVE_UP_CB);
}

static void move_down_cb(Widget but, void *udata)
{
    popup_any_cb((canvas_target *) udata, MOVE_DOWN_CB);
}

static void autoscale_cb(Widget but, void *udata)
{
    popup_any_cb((canvas_target *) udata, AUTOSCALE_BY_SET_CB);
}

static void s_editS_cb(Widget but, void *udata)
{
    popup_any_cb((canvas_target *) udata, EDITDATA_S_CB);
}

static void s_editE_cb(Widget but, void *udata)
{
    popup_any_cb((canvas_target *) udata, EDITDATA_E_CB);
}

static void drop_point_cb(Widget but, void *udata)
{
    popup_any_cb((canvas_target *) udata, DROP_POINT_CB);
}

static void atext_cb(Widget but, void *udata)
{
    canvas_target *ct = (canvas_target *) udata;
    APoint ap;
    Quark *q;

    q = atext_new(ct->q);
    Vpoint2Apoint(q, &ct->vp, &ap);
    atext_set_ap(q, &ap);
    atext_set_pointer(q, TRUE);

    update_all();

    raise_explorer(gui_from_quark(ct->q), q);
}

/*
 * clear the locator reference point
 */
static void do_clear_point(Widget but, void *udata)
{
    canvas_target *ct = (canvas_target *) udata;
    GLocator *locator;
    
    locator = graph_get_locator(ct->q);
    locator->pointset = FALSE;
    quark_dirtystate_set(ct->q, TRUE);
    
    update_all();

    xdrawgraph(get_parent_project(ct->q), FALSE);
}

/*
 * set the locator reference point
 */
static void set_locator_cb(Widget but, void *udata)
{
    canvas_target *ct = (canvas_target *) udata;
    GLocator *locator;
    WPoint wp;
    
    Vpoint2Wpoint(ct->q, &ct->vp, &wp);

    locator = graph_get_locator(ct->q);
    locator->origin = wp;
    locator->pointset = TRUE;
    quark_dirtystate_set(ct->q, TRUE);

    update_all();

    xdrawgraph(get_parent_project(ct->q), FALSE);
}

void canvas_event_proc(Widget w, XtPointer data, XEvent *event, Boolean *cont)
{
    int x, y;                /* pointer coordinates */
    VPoint vp;
    KeySym keybuf;
    Grace *grace = (Grace *) data;
    Quark *cg = graph_get_current(grace->project);
    X11Stuff *xstuff = grace->gui->xstuff;
    Widget drawing_window = grace->gui->mwui->drawing_window;
    
    XMotionEvent *xme;
    XButtonEvent *xbe;
    XKeyEvent    *xke;
    
    static Time lastc_time = 0;  /* time of last mouse click */
    static int lastc_x, lastc_y; /* coords of last mouse click */
    static int last_b1down_x, last_b1down_y;   /* coords of last event */
    int dbl_click;

    int undo_point = FALSE;
    int abort_action = FALSE;
    
    static canvas_target ct;
    static int on_focus;
    
    x = event->xmotion.x;
    y = event->xmotion.y;
    
    switch (event->type) {
    case MotionNotify:
	xme = (XMotionEvent *) event;
	if (grace->gui->crosshair_cursor) {
            crosshair_motion(grace->gui, x, y);
        }

        x11_dev2VPoint(x, y, &vp);

	if (xstuff->collect_points && xstuff->npoints) {
            switch (xstuff->sel_type) {
            case SELECTION_TYPE_RECT:
                select_region(grace->gui,
                    x, y, last_b1down_x, last_b1down_y, TRUE);
                break;
            case SELECTION_TYPE_VERT:
                select_vregion(grace->gui, x, last_b1down_x, TRUE);
                break;
            case SELECTION_TYPE_HORZ:
                select_hregion(grace->gui, y, last_b1down_y, TRUE);
                break;
            }
        } else
        if (xme->state & Button1Mask) {
            if (xme->state & ControlMask) {
                if (on_focus) {
                    resize_region(grace->gui, xstuff->f_v, on_focus,
                        x - last_b1down_x, y - last_b1down_y, TRUE);
                } else
                if (ct.found) {
                    slide_region(grace->gui, ct.bbox,
                        x - last_b1down_x, y - last_b1down_y, TRUE);
                }
            } else {
                scroll_pix(drawing_window, last_b1down_x - x, last_b1down_y - y);
            }
        } else {
            if (grace->gui->focus_policy == FOCUS_FOLLOWS) {
                cg = next_graph_containing(cg, &vp);
            }
            
            if (xme->state & ControlMask) {
                if (abs(x - xstuff->f_x1) <= 5 &&
                    abs(y - xstuff->f_y1) <= 5) {
                    on_focus = 1;
                } else
                if (abs(x - xstuff->f_x1) <= 5 &&
                    abs(y - xstuff->f_y2) <= 5) {
                    on_focus = 2;
                } else
                if (abs(x - xstuff->f_x2) <= 5 &&
                    abs(y - xstuff->f_y2) <= 5) {
                    on_focus = 3;
                } else
                if (abs(x - xstuff->f_x2) <= 5 &&
                    abs(y - xstuff->f_y1) <= 5) {
                    on_focus = 4;
                } else {
                    on_focus = 0;
                }
                if (on_focus) {
                    set_cursor(grace->gui, 4);
                } else {
                    set_cursor(grace->gui, -1);
                }
            }
        }

        update_locator_lab(cg, &vp);
        
        break;
    case ButtonPress:
        xbe = (XButtonEvent *) event;
	x = event->xbutton.x;
	y = event->xbutton.y;
	x11_dev2VPoint(x, y, &vp);

	switch (event->xbutton.button) {
	case Button1:
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
                if (xbe->state & ControlMask) {
                    ct.vp = vp;
                    ct.include_graphs = FALSE;
                    if (on_focus) {
                        resize_region(grace->gui, xstuff->f_v, on_focus,
                            0, 0, FALSE);
                    } else
                    if (find_target(grace->project, &ct) == RETURN_SUCCESS) {
                        slide_region(grace->gui, ct.bbox, 0, 0, FALSE);
                    }
                } else {
                    if (xstuff->collect_points) {
                        XPoint xp;
                        xp.x = x;
                        xp.y = y;
                        xstuff->npoints++;
                        xstuff->xps =
                            xrealloc(xstuff->xps, xstuff->npoints*sizeof(XPoint));
                            xstuff->xps[xstuff->npoints - 1] = xp;
                        select_region(grace->gui, x, y, x, y, FALSE);
                    } else
                    if (grace->gui->focus_policy == FOCUS_CLICK) {
                        cg = next_graph_containing(cg, &vp);
                    }
                    update_locator_lab(cg, &vp);
                }
            } else {
                ct.vp = vp;
                ct.include_graphs = (xbe->state & ControlMask) ? FALSE:TRUE;
                if (find_target(grace->project, &ct) == RETURN_SUCCESS) {
                    raise_explorer(grace->gui, ct.q);
                    ct.found = FALSE;
                }
            }
            
            last_b1down_x = x;
            last_b1down_y = y;
            
            if (!xstuff->collect_points) {
                set_cursor(grace->gui, 5);
            }
            
            break;
	case Button2:
            fprintf(stderr, "Button2\n");
            break;
	case Button3:
            if (xstuff->collect_points) {
                undo_point = TRUE;
                if (xstuff->npoints) {
                    xstuff->npoints--;
                }
                if (xstuff->npoints == 0) {
                    abort_action = TRUE;
                }
            } else {
                ct.vp = vp;
                ct.include_graphs = (xbe->state & ControlMask) ? FALSE:TRUE;
                if (find_target(grace->project, &ct) == RETURN_SUCCESS) {
                    char *s;
                    ct.found = FALSE;
                    
                    if (!popup) {
                        popup = XmCreatePopupMenu(grace->gui->xstuff->canvas,
                            "popupMenu", NULL, 0);
                        
                        poplab = CreateMenuLabel(popup, "");
                        
                        CreateMenuSeparator(popup);

                        CreateMenuButton(popup,
                            "Properties", '\0', edit_cb, &ct);
                        
                        CreateMenuSeparator(popup);
                        
                        CreateMenuButton(popup, "Hide", '\0', hide_cb, &ct);

                        CreateMenuSeparator(popup);
                        
                        CreateMenuButton(popup,
                            "Delete", '\0', delete_cb, &ct);
                        CreateMenuButton(popup,
                            "Duplicate", '\0', duplicate_cb, &ct);

                        CreateMenuSeparator(popup);

                        bring_to_front_bt = CreateMenuButton(popup,
                            "Bring to front", '\0', bring_to_front_cb, &ct);
                        move_up_bt = CreateMenuButton(popup,
                            "Move up", '\0', move_up_cb, &ct);
                        move_down_bt = CreateMenuButton(popup,
                            "Move down", '\0', move_down_cb, &ct);
                        send_to_back_bt = CreateMenuButton(popup,
                            "Send to back", '\0', send_to_back_cb, &ct);
                        
                        CreateMenuSeparator(popup);

                        as_set_bt = CreateMenuButton(popup,
                            "Autoscale by this set", '\0', autoscale_cb, &ct);

                        edit_menu = CreateMenu(popup, "Edit data", '\0', FALSE);
                        CreateMenuButton(edit_menu, "In spreadsheet", '\0',
    	                    s_editS_cb, &ct);
                        CreateMenuButton(edit_menu, "In text editor", '\0',
    	                    s_editE_cb, &ct);

                        atext_bt = CreateMenuButton(popup,
                            "Annotate this point", '\0', atext_cb, &ct);

                        CreateMenuSeparator(popup);

                        drop_pt_bt = CreateMenuButton(popup,
                            "Drop this point", '\0', drop_point_cb, &ct);

                        set_locator_bt = CreateMenuButton(popup,
                            "Set locator fixed point", '\0', set_locator_cb, &ct);
                        clear_locator_bt = CreateMenuButton(popup,
                            "Clear locator fixed point", '\0', do_clear_point, &ct);
                    }
                    s = q_labeling(ct.q);
                    SetLabel(poplab, s);
                    xfree(s);
                    if (quark_is_last_child(ct.q)) {
                        SetSensitive(bring_to_front_bt, FALSE);
                        SetSensitive(move_up_bt, FALSE);
                    } else {
                        SetSensitive(bring_to_front_bt, TRUE);
                        SetSensitive(move_up_bt, TRUE);
                    }
                    if (quark_is_first_child(ct.q)) {
                        SetSensitive(send_to_back_bt, FALSE);
                        SetSensitive(move_down_bt, FALSE);
                    } else {
                        SetSensitive(send_to_back_bt, TRUE);
                        SetSensitive(move_down_bt, TRUE);
                    }
                    
                    if ((quark_fid_get(ct.q) == QFlavorFrame && ct.part == 0) ||
                        (quark_fid_get(ct.q) == QFlavorGraph && ct.part == 0)) {
                        ManageChild(atext_bt);
                    } else {
                        UnmanageChild(atext_bt);
                    }
                    if (quark_fid_get(ct.q) == QFlavorGraph && ct.part != 1) {
                        ManageChild(set_locator_bt);
                    } else {
                        UnmanageChild(set_locator_bt);
                    }
                    if (quark_fid_get(ct.q) == QFlavorGraph && ct.part == 1) {
                        ManageChild(clear_locator_bt);
                    } else {
                        UnmanageChild(clear_locator_bt);
                    }
                    
                    if (quark_fid_get(ct.q) == QFlavorSet) {
                        ManageChild(as_set_bt);
                        ManageMenu(edit_menu);
                    } else {
                        UnmanageChild(as_set_bt);
                        UnmanageMenu(edit_menu);
                    }
                    if (quark_fid_get(ct.q) == QFlavorSet && ct.part >= 0) {
                        ManageChild(drop_pt_bt);
                    } else {
                        UnmanageChild(drop_pt_bt);
                    }
                    
                    XmMenuPosition(popup, xbe);
                    XtManageChild(popup);
                }
            }
            break;
	case Button4:
            scroll(drawing_window, TRUE, xbe->state & ControlMask);
            break;
	case Button5:
            scroll(drawing_window, FALSE, xbe->state & ControlMask);
            break;
	default:
            break;
        }
        break;
    case ButtonRelease:
        xbe = (XButtonEvent *) event;
	switch (event->xbutton.button) {
	case Button1:
            if (xbe->state & ControlMask) {
                x11_dev2VPoint(x, y, &vp);
                if (on_focus) {
                    view v;
                    Quark *fr = get_parent_frame(graph_get_current(grace->project));
                    frame_get_view(fr, &v);
                    switch (on_focus) {
                    case 1:
                        v.xv1 = vp.x;
                        v.yv1 = vp.y;
                        break;
                    case 2:
                        v.xv1 = vp.x;
                        v.yv2 = vp.y;
                        break;
                    case 3:
                        v.xv2 = vp.x;
                        v.yv2 = vp.y;
                        break;
                    case 4:
                        v.xv2 = vp.x;
                        v.yv1 = vp.y;
                        break;
                    }
                    frame_set_view(fr, &v);
                } else
                if (ct.found) {
                    slide_region(grace->gui, ct.bbox, x - last_b1down_x, y - last_b1down_y, FALSE);

                    move_target(&ct, &vp);
                }
                ct.found = FALSE;

                xdrawgraph(grace->project, TRUE);
                update_all();
            }
            if (!xstuff->collect_points) {
                set_cursor(grace->gui, -1);
            }
            break;
        }
        break;
    case KeyPress:
	xke = (XKeyEvent *) event;
        keybuf = XLookupKeysym(xke, 0);
        switch (keybuf) {
        case XK_Escape: /* Esc */
            abort_action = TRUE;
            break;
        case XK_KP_Add: /* "Grey" plus */
            if (xke->state & ControlMask) {
                page_zoom_inout(grace, +1);
            }
            break;
        case XK_KP_Subtract: /* "Grey" minus */
            if (xke->state & ControlMask) {
                page_zoom_inout(grace, -1);
            }
            break;
        case XK_1:
            if (xke->state & ControlMask) {
                page_zoom_inout(grace, 0);
            }
            break;
        }
        break;
    case KeyRelease:
	xke = (XKeyEvent *) event;
        keybuf = XLookupKeysym(xke, 0);
        if (xke->state & ControlMask) {
            if (on_focus) {
                set_cursor(grace->gui, -1);
            } else
            if (ct.found) {
                slide_region(grace->gui, ct.bbox, x - last_b1down_x, y - last_b1down_y, FALSE);
                ct.found = FALSE;
            }
        }
        break;
    default:
	break;
    }
    
    if (abort_action && xstuff->collect_points) {
        /* clear selection */
        switch (xstuff->sel_type) {
        case SELECTION_TYPE_RECT:
            select_region(grace->gui,
                x, y, last_b1down_x, last_b1down_y, FALSE);
            break;
        case SELECTION_TYPE_VERT:
            select_vregion(grace->gui, x, last_b1down_x, FALSE);
            break;
        case SELECTION_TYPE_HORZ:
            select_hregion(grace->gui, y, last_b1down_y, FALSE);
            break;
        }
        /* abort action */
        xstuff->npoints = 0;
        xstuff->collect_points = FALSE;
        set_cursor(grace->gui, -1);
        set_left_footer(NULL);
    } else
    if (undo_point) {
        /* previous action */
    } else
    if (xstuff->npoints_requested &&
        xstuff->npoints == xstuff->npoints_requested) {
        int ret;
        unsigned int i;
        VPoint *vps = xmalloc(xstuff->npoints*sizeof(VPoint));
        for (i = 0; i < xstuff->npoints; i++) {
            XPoint xp = xstuff->xps[i];
            x11_dev2VPoint(xp.x, xp.y, &vps[i]);
        }
        /* return points to caller */
        ret = xstuff->point_sink(xstuff->npoints, vps, xstuff->sink_data);
        if (ret != RETURN_SUCCESS) {
            XBell(xstuff->disp, 50);
        }
        
        xfree(vps);

        xstuff->npoints_requested = 0;
        xstuff->collect_points = FALSE;
        xstuff->npoints = 0;
        set_cursor(grace->gui, -1);

        xdrawgraph(grace->project, TRUE);
        update_all();
    }
}

static int hook(Quark *q, void *udata, QTraverseClosure *closure)
{
    if (quark_fid_get(q) == QFlavorGraph) {
        Quark *pr = get_parent_project(q);
        VPoint *vp = (VPoint *) udata;
        view v;
        
        closure->descend = FALSE;
        
	if (quark_is_active(q)        == TRUE &&
            graph_get_viewport(q, &v) == RETURN_SUCCESS &&
            is_vpoint_inside(&v, vp, 0.0) == TRUE &&
            graph_get_current(pr) != q) {
            switch_current_graph(q);
            return FALSE;
        }
    } else
    if (quark_fid_get(q) == QFlavorFrame && !quark_is_active(q)) {
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
    GLocator *locator;
    char buf[256];
    
    if (vpp != NULL) {
        vp = *vpp;
    }

    if (quark_is_active(cg) == TRUE                  &&
        graph_get_viewport(cg, &v) == RETURN_SUCCESS &&
        is_vpoint_inside(&v, &vp, 0.0) == TRUE       &&
        (locator = graph_get_locator(cg)) != NULL    &&
        locator->type != GLOCATOR_TYPE_NONE) {
        char bufx[64], bufy[64], *s, *prefix, *sx, *sy;
        WPoint wp;
        double wx, wy, xtmp, ytmp;

        Vpoint2Wpoint(cg, &vp, &wp);
        wx = wp.x;
        wy = wp.y;

        if (locator->pointset) {
	    wx -= locator->origin.x;
	    wy -= locator->origin.y;
            prefix = "d";
        } else {
            prefix = "";
        }

        switch (locator->type) {
        case GLOCATOR_TYPE_XY:
            xtmp = wx;
            ytmp = wy;
            sx = "X";
            sy = "Y";
            break;
        case GLOCATOR_TYPE_POLAR:
            xy2polar(wx, wy, &xtmp, &ytmp);
            sx = "Phi";
            sy = "Rho";
            break;
        default:
            return;
        }
        s = create_fstring(get_parent_project(cg),
            locator->fx, locator->px, xtmp, LFORMAT_TYPE_PLAIN);
        strcpy(bufx, s);
        s = create_fstring(get_parent_project(cg),
            locator->fy, locator->py, ytmp, LFORMAT_TYPE_PLAIN);
        strcpy(bufy, s);

        sprintf(buf, "%s: %s%s, %s%s = (%s, %s)", QIDSTR(cg),
            prefix, sx, prefix, sy, bufx, bufy);
    } else {
        sprintf(buf, "VX, VY = (%.4f, %.4f)", vp.x, vp.y);
    }
    
    set_tracker_string(buf);
}

void switch_current_graph(Quark *gr)
{
    
    if (quark_is_active(gr)) {
        Grace *grace = grace_from_quark(gr);
        Quark *cg = graph_get_current(grace->project);
        
        select_graph(gr);
        draw_focus(cg);
        draw_focus(gr);
        update_all();
        graph_set_selectors(gr);
        update_locator_lab(cg, NULL);
    }
}

static int zoom_sink(unsigned int npoints, const VPoint *vps, void *data)
{
    Grace *grace = (Grace *) data;
    world w;
    Quark *cg = graph_get_current(grace->project);
    WPoint wp;
    
    if (!cg || npoints != 2) {
        return RETURN_FAILURE;
    }
    
    Vpoint2Wpoint(cg, &vps[0], &wp);
    w.xg1 = wp.x;
    w.yg1 = wp.y;
    Vpoint2Wpoint(cg, &vps[1], &wp);
    w.xg2 = wp.x;
    w.yg2 = wp.y;
    
    if (w.xg1 > w.xg2) {
        fswap(&w.xg1, &w.xg2);
    }
    if (w.yg1 > w.yg2) {
        fswap(&w.yg1, &w.yg2);
    }
    
    return graph_set_world(cg, &w);
}

static int zoomx_sink(unsigned int npoints, const VPoint *vps, void *data)
{
    Grace *grace = (Grace *) data;
    world w;
    Quark *cg = graph_get_current(grace->project);
    WPoint wp;
    
    if (!cg || npoints != 2) {
        return RETURN_FAILURE;
    }
    
    graph_get_world(cg, &w);
    
    Vpoint2Wpoint(cg, &vps[0], &wp);
    w.xg1 = wp.x;
    Vpoint2Wpoint(cg, &vps[1], &wp);
    w.xg2 = wp.x;
    
    if (w.xg1 > w.xg2) {
        fswap(&w.xg1, &w.xg2);
    }
    
    return graph_set_world(cg, &w);
}

static int zoomy_sink(unsigned int npoints, const VPoint *vps, void *data)
{
    Grace *grace = (Grace *) data;
    world w;
    Quark *cg = graph_get_current(grace->project);
    WPoint wp;
    
    if (!cg || npoints != 2) {
        return RETURN_FAILURE;
    }
    
    graph_get_world(cg, &w);
    
    Vpoint2Wpoint(cg, &vps[0], &wp);
    w.yg1 = wp.y;
    Vpoint2Wpoint(cg, &vps[1], &wp);
    w.yg2 = wp.y;
    
    if (w.yg1 > w.yg2) {
        fswap(&w.yg1, &w.yg2);
    }
    
    return graph_set_world(cg, &w);
}

static int atext_sink(unsigned int npoints, const VPoint *vps, void *data)
{
    Grace *grace = (Grace *) data;
    Quark *cg = graph_get_current(grace->project), *q;
    WPoint wp;
    APoint ap;

    if (!cg || npoints != 1) {
        return RETURN_FAILURE;
    }
    
    if (Vpoint2Wpoint(cg, &vps[0], &wp) == RETURN_SUCCESS &&
        is_validWPoint(cg, &wp) == TRUE) {
        q = atext_new(cg);
        ap.x = wp.x; ap.y = wp.y;
        atext_set_ap(q, &ap);
    } else {
        q = atext_new(grace->project);
        ap.x = vps[0].x; ap.y = vps[0].y;
        atext_set_ap(q, &ap);
    }
    
    update_all();
    
    raise_explorer(grace->gui, q);

    return RETURN_SUCCESS;
}

void set_action(GUI *gui, unsigned int npoints, int seltype,
    CanvasPointSink sink, void *data)
{
    X11Stuff *xstuff = gui->xstuff;
    
    xstuff->npoints = 0;
    xstuff->npoints_requested = npoints;
    xstuff->point_sink = sink;
    xstuff->sink_data  = data;
    xstuff->sel_type = seltype;
    
    xstuff->collect_points = TRUE;

    XmProcessTraversal(xstuff->canvas, XmTRAVERSE_CURRENT);
}

/* -------------------------------------------------------------- */
/* canvas_actions */
void set_zoom_cb(Widget but, void *data)
{
    Grace *grace = (Grace *) data;
    set_cursor(grace->gui, 0);
    set_action(grace->gui, 2, SELECTION_TYPE_RECT, zoom_sink, grace);
}

void set_zoomx_cb(Widget but, void *data)
{
    Grace *grace = (Grace *) data;
    set_cursor(grace->gui, 0);
    set_action(grace->gui, 2, SELECTION_TYPE_VERT, zoomx_sink, grace);
}

void set_zoomy_cb(Widget but, void *data)
{
    Grace *grace = (Grace *) data;
    set_cursor(grace->gui, 0);
    set_action(grace->gui, 2, SELECTION_TYPE_HORZ, zoomy_sink, grace);
}


void atext_add_proc(Widget but, void *data)
{
    Grace *grace = (Grace *) data;
    set_cursor(grace->gui, 2);
    set_action(grace->gui, 1, SELECTION_TYPE_NONE, atext_sink, grace);
    set_left_footer("Select an anchor point");
}

void autoscale_action(Widget w, XEvent *e, String *p, Cardinal *c)
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


void refresh_hotlink_action(Widget w, XEvent *e, String *p, Cardinal *c)
{
    do_hotupdate_proc(NULL);
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
 * define a (polygon) region
 */
void do_select_region(void)
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

