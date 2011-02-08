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

#include <string.h>

#include "motifinc.h"
#include "xprotos.h"
#include "globals.h"

static void scroll_bar_pix(Widget bar, int pix)
{
    int value, slider_size, maxvalue, increment;

    GetScrollBarValues(bar, &value, &maxvalue, &slider_size, &increment);
    value += pix;
    if (value < 0) {
        value = 0;
    } else
    if (value > maxvalue - slider_size) {
        value = maxvalue - slider_size;
    }
    SetScrollBarValue(bar, value);
}

static void scroll_pix(Widget w, int dx, int dy)
{
    Widget bar;
    
    if (dx && (bar = GetHorizontalScrollBar(w))) {
        scroll_bar_pix(bar, dx);
    }
    if (dy && (bar = GetVerticalScrollBar(w))) {
        scroll_bar_pix(bar, dy);
    }
}
static void scroll(Widget w, int up, int horiz)
{
    int value, slider_size, increment, maxvalue;
    Widget vbar;
    
    if (horiz) {
        vbar = GetHorizontalScrollBar(w);
    } else {
        vbar = GetVerticalScrollBar(w);
    }
    
    if (!vbar) {
        return;
    }
    
    GetScrollBarValues(vbar, &value, &maxvalue, &slider_size, &increment);
    if (up) {
        value -= increment;
        if (value < 0) {
            value = 0;
        }
    } else {
        value += increment;
        if (value > maxvalue - slider_size) {
            value = maxvalue - slider_size;
        }
    }
    SetScrollBarValue(vbar, value);
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
            GraceApp *gapp = gapp_from_quark(q);
            Quark *cg = graph_get_current(gproject_get_top(gapp->gp));
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

static int find_target(GProject *gp, canvas_target *ct)
{
    ct->found = FALSE;
    quark_traverse(gproject_get_top(gp), target_hook, ct);

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

static Widget popup = NULL, poplab, drop_pt_bt, as_set_bt, atext_bt;
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
#define AUTOSCALE_BY_SET_CB  9

static void popup_any_cb(canvas_target *ct, int type)
{
    Quark *q = ct->q;
    
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
            del_point(q, ct->part);
        }
        break;
    case AUTOSCALE_BY_SET_CB:
        if (quark_fid_get(q) == QFlavorSet) {
            autoscale_bysets(&q, 1, AUTOSCALE_XY);
        }
        break;
    }
    
    snapshot_and_update(gapp->gp, TRUE);
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

    snapshot_and_update(gapp->gp, TRUE);

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
    
    snapshot_and_update(gapp->gp, TRUE);
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

    snapshot_and_update(gapp->gp, TRUE);
}

void canvas_event(CanvasEvent *event)
{
    int x, y;                /* pointer coordinates */
    VPoint vp;
    Quark *cg = graph_get_current(gproject_get_top(gapp->gp));
    X11Stuff *xstuff = gapp->gui->xstuff;
    Widget drawing_window = gapp->gui->mwui->drawing_window;
    
    static unsigned long lastc_time = 0;  /* time of last mouse click */
    static int lastc_x, lastc_y; /* coords of last mouse click */
    static int last_b1down_x, last_b1down_y;   /* coords of last event */
    int dbl_click;

    int undo_point = FALSE;
    int abort_action = FALSE;
    
    static canvas_target ct;
    static int on_focus;
    
    x = event->x;
    y = event->y;
    
    switch (event->type) {
    case MOUSE_MOVE:
	if (gapp->gui->crosshair_cursor) {
            crosshair_motion(gapp->gui, x, y);
        }

        x11_dev2VPoint(x, y, &vp);

	if (xstuff->collect_points && xstuff->npoints) {
            switch (xstuff->sel_type) {
            case SELECTION_TYPE_RECT:
                select_region(gapp->gui,
                    x, y, last_b1down_x, last_b1down_y, TRUE);
                break;
            case SELECTION_TYPE_VERT:
                select_vregion(gapp->gui, x, last_b1down_x, TRUE);
                break;
            case SELECTION_TYPE_HORZ:
                select_hregion(gapp->gui, y, last_b1down_y, TRUE);
                break;
            }
        } else
        if (event->button & LEFT_BUTTON) {
            if (event->modifiers & CONTROL_MODIFIER) {
                if (on_focus) {
                    resize_region(gapp->gui, xstuff->f_v, on_focus,
                        x - last_b1down_x, y - last_b1down_y, TRUE);
                } else
                if (ct.found) {
                    slide_region(gapp->gui, ct.bbox,
                        x - last_b1down_x, y - last_b1down_y, TRUE);
                }
            } else {
                scroll_pix(drawing_window, last_b1down_x - x, last_b1down_y - y);
            }
        } else {
            if (gapp->gui->focus_policy == FOCUS_FOLLOWS) {
                cg = next_graph_containing(cg, &vp);
            }
            
            if (event->modifiers & CONTROL_MODIFIER) {
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
                    set_cursor(gapp->gui, 4);
                } else {
                    set_cursor(gapp->gui, -1);
                }
            }
        }

        update_locator_lab(cg, &vp);
        
        break;
    case MOUSE_PRESS:
        x = event->x;
        y = event->y;
	x11_dev2VPoint(x, y, &vp);

        switch (event->button) {
        case LEFT_BUTTON:
            /* first, determine if it's a double click */
            if (event->time - lastc_time < CLICK_INT &&
                abs(x - lastc_x) < CLICK_DIST      &&
                abs(y - lastc_y) < CLICK_DIST) {
                dbl_click = TRUE;
            } else {
                dbl_click = FALSE;
            }
            lastc_time = event->time;
            lastc_x = x;
            lastc_y = y;

            if (!dbl_click) {
                if (event->modifiers & CONTROL_MODIFIER) {
                    ct.vp = vp;
                    ct.include_graphs = FALSE;
                    if (on_focus) {
                        resize_region(gapp->gui, xstuff->f_v, on_focus,
                            0, 0, FALSE);
                    } else
                    if (find_target(gapp->gp, &ct) == RETURN_SUCCESS) {
                        slide_region(gapp->gui, ct.bbox, 0, 0, FALSE);
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
                        select_region(gapp->gui, x, y, x, y, FALSE);
                    } else
                    if (gapp->gui->focus_policy == FOCUS_CLICK) {
                        cg = next_graph_containing(cg, &vp);
                    }
                    update_locator_lab(cg, &vp);
                }
            } else {
                ct.vp = vp;
                ct.include_graphs = (event->modifiers & CONTROL_MODIFIER) ? FALSE:TRUE;
                if (find_target(gapp->gp, &ct) == RETURN_SUCCESS) {
                    raise_explorer(gapp->gui, ct.q);
                    ct.found = FALSE;
                }
            }
            
            last_b1down_x = x;
            last_b1down_y = y;
            
            if (!xstuff->collect_points) {
                set_cursor(gapp->gui, 5);
            }
            
            break;
        case MIDDLE_BUTTON:
            fprintf(stderr, "Button2\n");
            break;
        case RIGHT_BUTTON:
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
                ct.include_graphs = (event->modifiers & CONTROL_MODIFIER) ? FALSE:TRUE;
                if (find_target(gapp->gp, &ct) == RETURN_SUCCESS) {
                    char *s;
                    ct.found = FALSE;
                    
                    if (!popup) {
                        popup = CreatePopupMenu(gapp->gui->xstuff->canvas);
                        
                        poplab = CreateMenuLabel(popup, "");
                        
                        CreateMenuSeparator(popup);

                        CreateMenuButton(popup,
                            "Properties...", '\0', edit_cb, &ct);
                        
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
                    } else {
                        UnmanageChild(as_set_bt);
                    }
                    if (quark_fid_get(ct.q) == QFlavorSet && ct.part >= 0) {
                        ManageChild(drop_pt_bt);
                    } else {
                        UnmanageChild(drop_pt_bt);
                    }
                    
                    ShowMenu(popup, event->udata);
                }
            }
            break;
        case WHEEL_UP_BUTTON:
            scroll(drawing_window, TRUE, event->modifiers & CONTROL_MODIFIER);
            break;
        case WHEEL_DOWN_BUTTON:
            scroll(drawing_window, FALSE, event->modifiers & CONTROL_MODIFIER);
            break;
	default:
            break;
        }
        break;
    case MOUSE_RELEASE:
        switch (event->button) {
        case LEFT_BUTTON:
            if (event->modifiers & CONTROL_MODIFIER) {
                x11_dev2VPoint(x, y, &vp);
                if (on_focus) {
                    view v;
                    Quark *fr = get_parent_frame(graph_get_current(gproject_get_top(gapp->gp)));
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
                    slide_region(gapp->gui, ct.bbox, x - last_b1down_x, y - last_b1down_y, FALSE);

                    move_target(&ct, &vp);
                }
                ct.found = FALSE;

                snapshot_and_update(gapp->gp, TRUE);
            }
            if (!xstuff->collect_points) {
                set_cursor(gapp->gui, -1);
            }
            break;
        }
        break;
    case KEY_PRESS:
        switch (event->key) {
        case KEY_ESCAPE: /* Esc */
            abort_action = TRUE;
            break;
        case KEY_PLUS: /* "Grey" plus */
            if (event->modifiers & CONTROL_MODIFIER) {
                page_zoom_inout(gapp, +1);
            }
            break;
        case KEY_MINUS: /* "Grey" minus */
            if (event->modifiers & CONTROL_MODIFIER) {
                page_zoom_inout(gapp, -1);
            }
            break;
        case KEY_1:
            if (event->modifiers & CONTROL_MODIFIER) {
                page_zoom_inout(gapp, 0);
            }
            break;
        }
        break;
    case KEY_RELEASE:
        if (event->modifiers & CONTROL_MODIFIER) {
            if (on_focus) {
                set_cursor(gapp->gui, -1);
            } else
            if (ct.found) {
                slide_region(gapp->gui, ct.bbox, x - last_b1down_x, y - last_b1down_y, FALSE);
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
            select_region(gapp->gui,
                x, y, last_b1down_x, last_b1down_y, FALSE);
            break;
        case SELECTION_TYPE_VERT:
            select_vregion(gapp->gui, x, last_b1down_x, FALSE);
            break;
        case SELECTION_TYPE_HORZ:
            select_hregion(gapp->gui, y, last_b1down_y, FALSE);
            break;
        }
        /* abort action */
        xstuff->npoints = 0;
        xstuff->collect_points = FALSE;
        set_cursor(gapp->gui, -1);
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
            Beep();
        }
        
        xfree(vps);

        xstuff->npoints_requested = 0;
        xstuff->collect_points = FALSE;
        xstuff->npoints = 0;
        set_cursor(gapp->gui, -1);

        snapshot_and_update(gapp->gp, TRUE);
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
            &locator->fx, xtmp, LFORMAT_TYPE_PLAIN);
        strcpy(bufx, s);
        s = create_fstring(get_parent_project(cg),
            &locator->fy, ytmp, LFORMAT_TYPE_PLAIN);
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
        GraceApp *gapp = gapp_from_quark(gr);
        Quark *cg = graph_get_current(gproject_get_top(gapp->gp));
        
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
    GraceApp *gapp = (GraceApp *) data;
    world w;
    Quark *cg = graph_get_current(gproject_get_top(gapp->gp));
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
    GraceApp *gapp = (GraceApp *) data;
    world w;
    Quark *cg = graph_get_current(gproject_get_top(gapp->gp));
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
    GraceApp *gapp = (GraceApp *) data;
    world w;
    Quark *cg = graph_get_current(gproject_get_top(gapp->gp));
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
    GraceApp *gapp = (GraceApp *) data;
    Quark *cg = graph_get_current(gproject_get_top(gapp->gp)), *q;
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
        q = atext_new(gproject_get_top(gapp->gp));
        ap.x = vps[0].x; ap.y = vps[0].y;
        atext_set_ap(q, &ap);
    }
    
    raise_explorer(gapp->gui, q);

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

#ifndef QT_GUI
    XmProcessTraversal(xstuff->canvas, XmTRAVERSE_CURRENT);
#endif
}

/* -------------------------------------------------------------- */
/* canvas_actions */
void set_zoom_cb(Widget but, void *data)
{
    GraceApp *gapp = (GraceApp *) data;
    set_cursor(gapp->gui, 0);
    set_action(gapp->gui, 2, SELECTION_TYPE_RECT, zoom_sink, gapp);
}

void set_zoomx_cb(Widget but, void *data)
{
    GraceApp *gapp = (GraceApp *) data;
    set_cursor(gapp->gui, 0);
    set_action(gapp->gui, 2, SELECTION_TYPE_VERT, zoomx_sink, gapp);
}

void set_zoomy_cb(Widget but, void *data)
{
    GraceApp *gapp = (GraceApp *) data;
    set_cursor(gapp->gui, 0);
    set_action(gapp->gui, 2, SELECTION_TYPE_HORZ, zoomy_sink, gapp);
}


void atext_add_proc(Widget but, void *data)
{
    GraceApp *gapp = (GraceApp *) data;
    set_cursor(gapp->gui, 2);
    set_action(gapp->gui, 1, SELECTION_TYPE_NONE, atext_sink, gapp);
    set_left_footer("Select an anchor point");
}
