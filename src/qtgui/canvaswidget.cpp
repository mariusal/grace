#include <QApplication>
#include <QPainter>
#include <QFont>
#include <QMouseEvent>
extern "C" {
    #include <core_utils.h>
}
#include "mainwindow.h"
#include "canvaswidget.h"

typedef struct {
    double x;
    double y;
} XPoint;

typedef int (*CanvasPointSink) (
    unsigned int npoints,
    const VPoint *vps,
    void *data
);

struct _QtStuff {
//    Display *disp;
    int screennumber;

//    Window root;
//    Window xwin;

//    Widget canvas;

//   GC gc;
    int depth;
//    Colormap cmap;

    double dpi;

//    Pixmap bufpixmap;

    double win_h;
    double win_w;
    double win_scale;

    /* cursors */
//    Cursor wait_cursor;
//    Cursor line_cursor;
//    Cursor find_cursor;
//    Cursor move_cursor;
//    Cursor text_cursor;
//    Cursor kill_cursor;
//    Cursor drag_cursor;
    int cur_cursor;

    /* coords of focus markers*/
    double f_x1, f_y1, f_x2, f_y2;
    view f_v;

    unsigned int npoints;
    XPoint *xps;

    unsigned int npoints_requested;
    int collect_points;

    CanvasPointSink point_sink;
    void *sink_data;
    int sel_type;
};

CanvasWidget::CanvasWidget(QWidget *parent) :
    QWidget(parent), pixmap(0)
{
    xstuff = new QtStuff;
    region_need_erasing = FALSE;
    setMouseTracking(true);
}

void CanvasWidget::setMainWindow(MainWindow *mainWindow)
{
    this->mainWindow = mainWindow;
}

void CanvasWidget::setGraceApp(GraceApp *gapp)
{
    this->gapp = gapp;
}

void CanvasWidget::setLocatorBar(QLabel *locatorBar)
{
    this->locatorBar = locatorBar;
}

void CanvasWidget::paintEvent(QPaintEvent *)
{
    if (pixmap != 0) {
        QPainter painter(this);
        painter.drawImage(0, 0, *pixmap);
        // TODO: draw just what you see
        //void QPainter::drawImage ( int x, int y, const QImage & image, int sx = 0, int sy = 0, int sw = -1, int sh = -1, Qt::ImageConversionFlags flags = Qt::AutoColor )
    }
}

void set_wait_cursor()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
}

void unset_wait_cursor()
{
    QApplication::restoreOverrideCursor();
}

void CanvasWidget::xdrawgrid()
{
    int i, j;
    double step;
    double x, y;

    double w = width();
    double h = height();

    QPainter painter(pixmap);

    QPen pen;
    pen.setColor(Qt::black);
    pen.setWidth(1);
    pen.setStyle(Qt::SolidLine);
    pen.setCapStyle(Qt::FlatCap);
    pen.setJoinStyle(Qt::MiterJoin);
    painter.setPen(pen);

    QBrush brush;
    brush.setColor(Qt::white);
    painter.setBrush(brush);

    painter.drawRect(rect());

    step = MIN2(w, h)/10;
    for (i = 0; i < w/step; i++) {
        for (j = 0; j < h/step; j++) {
            x = i*step;
            y = h - j*step;
            painter.drawPoint(QPointF(x, y));
        }
    }
}

void CanvasWidget::qt_VPoint2dev(const VPoint *vp, double *x, double *y)
{
    *x = xstuff->win_scale * vp->x;
    *y = xstuff->win_h - xstuff->win_scale * vp->y;
}

/*
 * x11_dev2VPoint - given (x,y) in screen coordinates, return the
 * viewport coordinates
 */
void CanvasWidget::qt_dev2VPoint(double x, double y, VPoint *vp)
{
    if (xstuff->win_scale == 0) {
        vp->x = 0.0;
        vp->y = 0.0;
    } else {
        vp->x = x / xstuff->win_scale;
        vp->y = (xstuff->win_h - y) / xstuff->win_scale;
    }
}

/*
 * draw the graph focus indicators
 */
void CanvasWidget::draw_focus(Quark *gr)
{
    double ix1, iy1, ix2, iy2;
    view v;
    VPoint vp;
    GUI *gui = gui_from_quark(gr);

    if (gui->draw_focus_flag == TRUE) {
        graph_get_viewport(gr, &v);
        vp.x = v.xv1;
        vp.y = v.yv1;
        qt_VPoint2dev(&vp, &ix1, &iy1);
        vp.x = v.xv2;
        vp.y = v.yv2;
        qt_VPoint2dev(&vp, &ix2, &iy2);
        aux_XFillRectangle(gui, ix1 - 5, iy1 - 5, 10, 10);
        aux_XFillRectangle(gui, ix1 - 5, iy2 - 5, 10, 10);
        aux_XFillRectangle(gui, ix2 - 5, iy2 - 5, 10, 10);
        aux_XFillRectangle(gui, ix2 - 5, iy1 - 5, 10, 10);

        xstuff->f_x1 = ix1;
        xstuff->f_x2 = ix2;
        xstuff->f_y1 = iy1;
        xstuff->f_y2 = iy2;

        xstuff->f_v  = v;
    }
}

void CanvasWidget::qtdrawgraph(const GProject *gp)
{
    Quark *project = gproject_get_top(gp);
    GraceApp *gapp = gapp_from_quark(project);

    if (gapp && gapp->gui->inwin) {
        Quark *gr = graph_get_current(project);
        Device_entry *d = get_device_props(grace_get_canvas(gapp->grace),  gapp->rt->tdevice);
        Page_geometry *pg = &d->pg;
        float dpi = gapp->gui->zoom * physicalDpiX();

        set_wait_cursor();

        if (dpi != pg->dpi) {
            int wpp, hpp;
            project_get_page_dimensions(project, &wpp, &hpp);

            pg->width  = (unsigned long) (wpp*dpi/72);
            pg->height = (unsigned long) (hpp*dpi/72);
            pg->dpi = dpi;
        }

        resize_drawables(pg->width, pg->height);

        xdrawgrid();

        canvas_set_prstream(grace_get_canvas(gapp->grace), pixmap);

        select_device(grace_get_canvas(gapp->grace), gapp->rt->tdevice);
        gproject_render(gp);

        if (quark_is_active(gr)) {
            draw_focus(gr);
        }
        //reset_crosshair(gapp->gui, FALSE);
        region_need_erasing = FALSE;

        update();
        //x11_redraw(xstuff->xwin, 0, 0, xstuff->win_w, xstuff->win_h);

        //XFlush(xstuff->disp);

        unset_wait_cursor();
    }
}

void CanvasWidget::resize_drawables(unsigned int w, unsigned int h)
{
    if (w == 0 || h == 0) {
        return;
    }

    // TODO: what image format to use?
    if (pixmap == 0) {
        pixmap = new QImage(w, h, QImage::Format_ARGB4444_Premultiplied);
    } if (xstuff->win_w != w || xstuff->win_h != h) {
        delete pixmap;
        pixmap = new QImage(w, h, QImage::Format_ARGB4444_Premultiplied);
    }

    if (pixmap == 0) {
        errmsg("Can't allocate buffer pixmap");
        xstuff->win_w = 0;
        xstuff->win_h = 0;
    } else {
        xstuff->win_w = w;
        xstuff->win_h = h;
    }

    xstuff->win_scale = MIN2(xstuff->win_w, xstuff->win_h);

    if (!gui_is_page_free(gapp->gui)) {
        //SetDimensions(xstuff->canvas, xstuff->win_w, xstuff->win_h);
        setMinimumSize(w, h);
    }
}

void CanvasWidget::set_tracker_string(char *s)
{
    if (s == NULL) {
        locatorBar->setText("[Out of frame]");
    } else {
        locatorBar->setText(s);
    }
}

/*
 * locator on main_panel
 */
void CanvasWidget::update_locator_lab(Quark *cg, VPoint *vpp)
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

typedef struct {
    VPoint vp;
    int include_graphs;
    Quark *q;
    int part;
    view bbox;
    int found;
} canvas_target;

/* selection type */
#define SELECTION_TYPE_NONE 0
#define SELECTION_TYPE_RECT 1
#define SELECTION_TYPE_VERT 2
#define SELECTION_TYPE_HORZ 3
/* #define SELECTION_TYPE_POLY 4 */

void switch_current_graph(Quark *gr)
{
    if (quark_is_active(gr)) {
        GraceApp *gapp = gapp_from_quark(gr);
        Quark *cg = graph_get_current(gproject_get_top(gapp->gp));

        select_graph(gr);
        //draw_focus(cg);
        //draw_focus(gr);
        //mainWindow->update_all();
        //graph_set_selectors(gr);
        //update_locator_lab(cg, NULL);
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
Quark* CanvasWidget::next_graph_containing(Quark *q, VPoint *vp)
{
    Quark *pr = get_parent_project(q);

    quark_traverse(pr, hook, vp);

    return graph_get_current(pr);
}

void CanvasWidget::mouseMoveEvent(QMouseEvent *event)
{
    static canvas_target ct;
    static int on_focus;
    Quark *cg = graph_get_current(gproject_get_top(gapp->gp));

    QPointF point = event->posF();
    double x = point.x();
    double y = point.y();

    VPoint vp;

    qt_dev2VPoint(x, y, &vp);

    if (gapp->gui->crosshair_cursor) {
        //crosshair_motion(gapp->gui, x, y);
    }

    qt_dev2VPoint(x, y, &vp);

    if (xstuff->collect_points && xstuff->npoints) {
        errmsg("collect_points");
            switch (xstuff->sel_type) {
            case SELECTION_TYPE_RECT:
                //select_region(gapp->gui, x, y, last_b1down_x, last_b1down_y, TRUE);
                break;
            case SELECTION_TYPE_VERT:
                //select_vregion(gapp->gui, x, last_b1down_x, TRUE);
                break;
            case SELECTION_TYPE_HORZ:
                //select_hregion(gapp->gui, y, last_b1down_y, TRUE);
                break;
            }
        } else
        if (event->modifiers() && Qt::LeftButton) {
            errmsg("veikia left");
            if (event->modifiers() && Qt::ControlModifier) {
                errmsg("veikia left ir ctrl");
                if (on_focus) {
                    resize_region(gapp->gui, xstuff->f_v, on_focus,
                        x - last_b1down_x, y - last_b1down_y, TRUE);
                } else
                if (ct.found) {
                    //slide_region(gapp->gui, ct.bbox,
                    //    x - last_b1down_x, y - last_b1down_y, TRUE);
                }
            } else {
                //scroll_pix(drawing_window, last_b1down_x - x, last_b1down_y - y);
            }
        } else {
            if (gapp->gui->focus_policy == FOCUS_FOLLOWS) {
                cg = next_graph_containing(cg, &vp);
            }

            if (event->modifiers() && Qt::ControlModifier) {
                if (fabs(x - xstuff->f_x1) <= 5 &&
                    fabs(y - xstuff->f_y1) <= 5) {
                    on_focus = 1;
                } else
                if (fabs(x - xstuff->f_x1) <= 5 &&
                    fabs(y - xstuff->f_y2) <= 5) {
                    on_focus = 2;
                } else
                if (fabs(x - xstuff->f_x2) <= 5 &&
                    fabs(y - xstuff->f_y2) <= 5) {
                    on_focus = 3;
                } else
                if (fabs(x - xstuff->f_x2) <= 5 &&
                    fabs(y - xstuff->f_y1) <= 5) {
                    on_focus = 4;
                } else {
                    on_focus = 0;
                }
                if (on_focus) {
                    //set_cursor(gapp->gui, 4);
                } else {
                    //set_cursor(gapp->gui, -1);
                }
            }
        }

        update_locator_lab(cg, &vp);
}

void CanvasWidget::resize_region(GUI *gui, view bb, int on_focus,
    double shift_x, double shift_y, int erase)
{
    double x1, x2, y1, y2;
    VPoint vp;

    vp.x = bb.xv1;
    vp.y = bb.yv1;
    qt_VPoint2dev(&vp, &x1, &y1);
    vp.x = bb.xv2;
    vp.y = bb.yv2;
    qt_VPoint2dev(&vp, &x2, &y2);

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

/*
 * draw an xor'ed box (optionally erasing previous one)
 */
void CanvasWidget::select_region(GUI *gui, double x1, double y1, double x2, double y2, int erase)
{
    static double x1_old, y1_old, dx_old, dy_old;
    double dx = x2 - x1;
    double dy = y2 - y1;

    if (dx < 0) {
    fswap(&x1, &x2);
    dx = -dx;
    }
    if (dy < 0) {
    fswap(&y1, &y2);
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

void CanvasWidget::aux_XDrawRectangle(GUI *gui, double x1, double y1, double x2, double y2)
{
    QPainter painter(pixmap);
    painter.setPen(QPen(Qt::black));
    painter.setBrush(QBrush(Qt::NoBrush));
    painter.setCompositionMode(QPainter::CompositionMode_Xor);
    painter.drawRect(QRectF(QPointF(x1, y1), QPointF(x2, y2)));
    //X11Stuff *xstuff = gui->xstuff;
    //XDrawRectangle(xstuff->disp, xstuff->xwin, gcxor, x1, y1, x2, y2);
    //if (xstuff->bufpixmap != (Pixmap) NULL) {
    //    XDrawRectangle(xstuff->disp, xstuff->bufpixmap, gcxor, x1, y1, x2, y2);
    //}
}

void CanvasWidget::aux_XFillRectangle(GUI *gui, double x, double y, double width, double height)
{
    QPainter painter(pixmap);
    painter.setPen(QPen(Qt::NoPen));
    painter.setBrush(QBrush(Qt::black));
    painter.setCompositionMode(QPainter::CompositionMode_Xor);
    painter.drawRect(QRectF(x, y, width, height));
    //X11Stuff *xstuff = gui->xstuff;
    //XFillRectangle(xstuff->disp, xstuff->xwin, gcxor, x, y, width, height);
    //if (xstuff->bufpixmap != (Pixmap) NULL) {
    //    XFillRectangle(xstuff->disp, xstuff->bufpixmap, gcxor, x, y, width, height);
    //}
}


void CanvasWidget::mousePressEvent(QMouseEvent *event)
{
    QPointF point = event->posF();
    double x = point.x();
    double y = point.y();

    VPoint vp;

    qt_dev2VPoint(x, y, &vp);
//    xbe = (XButtonEvent *) event;
//x = event->xbutton.x;
//y = event->xbutton.y;
//x11_dev2VPoint(x, y, &vp);
//
//case Button1:
//        /* first, determine if it's a double click */
//        if (xbe->time - lastc_time < CLICK_INT &&
//            abs(x - lastc_x) < CLICK_DIST      &&
//            abs(y - lastc_y) < CLICK_DIST) {
//            dbl_click = TRUE;
//        } else {
//            dbl_click = FALSE;
//        }
//        lastc_time = xbe->time;
//        lastc_x = x;
//        lastc_y = y;
//
//        if (!dbl_click) {
//            if (xbe->state & ControlMask) {
//                ct.vp = vp;
//                ct.include_graphs = FALSE;
//                if (on_focus) {
//                    resize_region(gapp->gui, xstuff->f_v, on_focus,
//                        0, 0, FALSE);
//                } else
//                if (find_target(gapp->gp, &ct) == RETURN_SUCCESS) {
//                    slide_region(gapp->gui, ct.bbox, 0, 0, FALSE);
//                }
//            } else {
//                if (xstuff->collect_points) {
//                    XPoint xp;
//                    xp.x = x;
//                    xp.y = y;
//                    xstuff->npoints++;
//                    xstuff->xps =
//                        xrealloc(xstuff->xps, xstuff->npoints*sizeof(XPoint));
//                        xstuff->xps[xstuff->npoints - 1] = xp;
//                    select_region(gapp->gui, x, y, x, y, FALSE);
//                } else
//                if (gapp->gui->focus_policy == FOCUS_CLICK) {
//                    cg = next_graph_containing(cg, &vp);
//                }
//                update_locator_lab(cg, &vp);
//            }
//        } else {
//            ct.vp = vp;
//            ct.include_graphs = (xbe->state & ControlMask) ? FALSE:TRUE;
//            if (find_target(gapp->gp, &ct) == RETURN_SUCCESS) {
//                raise_explorer(gapp->gui, ct.q);
//                ct.found = FALSE;
//            }
//        }
//
        last_b1down_x = x;
        last_b1down_y = y;
//
//        if (!xstuff->collect_points) {
//            set_cursor(gapp->gui, 5);
//        }
//
//        break;
//case Button2:
//        fprintf(stderr, "Button2\n");
//        break;
//case Button3:
//        if (xstuff->collect_points) {
//            undo_point = TRUE;
//            if (xstuff->npoints) {
//                xstuff->npoints--;
//            }
//            if (xstuff->npoints == 0) {
//                abort_action = TRUE;
//            }
//        } else {
//            ct.vp = vp;
//            ct.include_graphs = (xbe->state & ControlMask) ? FALSE:TRUE;
//            if (find_target(gapp->gp, &ct) == RETURN_SUCCESS) {
//                char *s;
//                ct.found = FALSE;
//
//                if (!popup) {
//                    popup = XmCreatePopupMenu(gapp->gui->xstuff->canvas,
//                        "popupMenu", NULL, 0);
//
//                    poplab = CreateMenuLabel(popup, "");
//
//                    CreateMenuSeparator(popup);
//
//                    CreateMenuButton(popup,
//                        "Properties...", '\0', edit_cb, &ct);
//
//                    CreateMenuSeparator(popup);
//
//                    CreateMenuButton(popup, "Hide", '\0', hide_cb, &ct);
//
//                    CreateMenuSeparator(popup);
//
//                    CreateMenuButton(popup,
//                        "Delete", '\0', delete_cb, &ct);
//                    CreateMenuButton(popup,
//                        "Duplicate", '\0', duplicate_cb, &ct);
//
//                    CreateMenuSeparator(popup);
//
//                    bring_to_front_bt = CreateMenuButton(popup,
//                        "Bring to front", '\0', bring_to_front_cb, &ct);
//                    move_up_bt = CreateMenuButton(popup,
//                        "Move up", '\0', move_up_cb, &ct);
//                    move_down_bt = CreateMenuButton(popup,
//                        "Move down", '\0', move_down_cb, &ct);
//                    send_to_back_bt = CreateMenuButton(popup,
//                        "Send to back", '\0', send_to_back_cb, &ct);
//
//                    CreateMenuSeparator(popup);
//
//                    as_set_bt = CreateMenuButton(popup,
//                        "Autoscale by this set", '\0', autoscale_cb, &ct);
//
//                    atext_bt = CreateMenuButton(popup,
//                        "Annotate this point", '\0', atext_cb, &ct);
//
//                    CreateMenuSeparator(popup);
//
//                    drop_pt_bt = CreateMenuButton(popup,
//                        "Drop this point", '\0', drop_point_cb, &ct);
//
//                    set_locator_bt = CreateMenuButton(popup,
//                        "Set locator fixed point", '\0', set_locator_cb, &ct);
//                    clear_locator_bt = CreateMenuButton(popup,
//                        "Clear locator fixed point", '\0', do_clear_point, &ct);
//                }
//                s = q_labeling(ct.q);
//                SetLabel(poplab, s);
//                xfree(s);
//                if (quark_is_last_child(ct.q)) {
//                    SetSensitive(bring_to_front_bt, FALSE);
//                    SetSensitive(move_up_bt, FALSE);
//                } else {
//                    SetSensitive(bring_to_front_bt, TRUE);
//                    SetSensitive(move_up_bt, TRUE);
//                }
//                if (quark_is_first_child(ct.q)) {
//                    SetSensitive(send_to_back_bt, FALSE);
//                    SetSensitive(move_down_bt, FALSE);
//                } else {
//                    SetSensitive(send_to_back_bt, TRUE);
//                    SetSensitive(move_down_bt, TRUE);
//                }
//
//                if ((quark_fid_get(ct.q) == QFlavorFrame && ct.part == 0) ||
//                    (quark_fid_get(ct.q) == QFlavorGraph && ct.part == 0)) {
//                    ManageChild(atext_bt);
//                } else {
//                    UnmanageChild(atext_bt);
//                }
//                if (quark_fid_get(ct.q) == QFlavorGraph && ct.part != 1) {
//                    ManageChild(set_locator_bt);
//                } else {
//                    UnmanageChild(set_locator_bt);
//                }
//                if (quark_fid_get(ct.q) == QFlavorGraph && ct.part == 1) {
//                    ManageChild(clear_locator_bt);
//                } else {
//                    UnmanageChild(clear_locator_bt);
//                }
//
//                if (quark_fid_get(ct.q) == QFlavorSet) {
//                    ManageChild(as_set_bt);
//                } else {
//                    UnmanageChild(as_set_bt);
//                }
//                if (quark_fid_get(ct.q) == QFlavorSet && ct.part >= 0) {
//                    ManageChild(drop_pt_bt);
//                } else {
//                    UnmanageChild(drop_pt_bt);
//                }
//
//                XmMenuPosition(popup, xbe);
//                XtManageChild(popup);
//            }
//        }
}

void CanvasWidget::mouseReleaseEvent(QMouseEvent *event)
{
//    xbe = (XButtonEvent *) event;
//switch (event->xbutton.button) {
//case Button1:
//        if (xbe->state & ControlMask) {
//            x11_dev2VPoint(x, y, &vp);
//            if (on_focus) {
//                view v;
//                Quark *fr = get_parent_frame(graph_get_current(gproject_get_top(gapp->gp)));
//                frame_get_view(fr, &v);
//                switch (on_focus) {
//                case 1:
//                    v.xv1 = vp.x;
//                    v.yv1 = vp.y;
//                    break;
//                case 2:
//                    v.xv1 = vp.x;
//                    v.yv2 = vp.y;
//                    break;
//                case 3:
//                    v.xv2 = vp.x;
//                    v.yv2 = vp.y;
//                    break;
//                case 4:
//                    v.xv2 = vp.x;
//                    v.yv1 = vp.y;
//                    break;
//                }
//                frame_set_view(fr, &v);
//            } else
//            if (ct.found) {
//                slide_region(gapp->gui, ct.bbox, x - last_b1down_x, y - last_b1down_y, FALSE);
//
//                move_target(&ct, &vp);
//            }
//            ct.found = FALSE;
//
//            snapshot_and_update(gapp->gp, TRUE);
//        }
//        if (!xstuff->collect_points) {
//            set_cursor(gapp->gui, -1);
//        }
//        break;
//    }
//
}

void CanvasWidget::wheelEvent(QWheelEvent *event)
{
    event->ignore();
//case Button4:
//        scroll(drawing_window, TRUE, xbe->state & ControlMask);
//        break;
//case Button5:
//        scroll(drawing_window, FALSE, xbe->state & ControlMask);
//        break;
}

void CanvasWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->modifiers() && Qt::Key_Escape) {
        abort_action = TRUE;
    }
}

void CanvasWidget::keyReleaseEvent(QKeyEvent *event)
{
//    case KeyRelease:
//    xke = (XKeyEvent *) event;
//        keybuf = XLookupKeysym(xke, 0);
//        if (xke->state & ControlMask) {
//            if (on_focus) {
//                set_cursor(gapp->gui, -1);
//            } else
//            if (ct.found) {
//                slide_region(gapp->gui, ct.bbox, x - last_b1down_x, y - last_b1down_y, FALSE);
//                ct.found = FALSE;
//            }
//        }
//        break;
//
}

bool CanvasWidget::event(QEvent *event)
//void canvas_event_proc(Widget w, XtPointer data, XEvent *event, Boolean *cont)
{
    bool return_value = QWidget::event(event);
//    int x, y;                /* pointer coordinates */
//    VPoint vp;
//    KeySym keybuf;
//    //GraceApp *gapp = (GraceApp *) data;
//    Quark *cg = graph_get_current(gproject_get_top(gapp->gp));
//    X11Stuff *xstuff = gapp->gui->xstuff;
//    Widget drawing_window = gapp->gui->mwui->drawing_window;
//
//    XMotionEvent *xme;
//    XButtonEvent *xbe;
//    XKeyEvent    *xke;
//
//    static Time lastc_time = 0;  /* time of last mouse click */
//    static int lastc_x, lastc_y; /* coords of last mouse click */
//    static int last_b1down_x, last_b1down_y;   /* coords of last event */
//    int dbl_click;
//
    undo_point = FALSE;
    abort_action = FALSE;
//
//    static canvas_target ct;
//    static int on_focus;
//
//    x = event->xmotion.x;
//    y = event->xmotion.y;
//
//    switch (event->type) {
//    case MotionNotify:
//
//        break;
//    case ButtonPress:
//
//
//    switch (event->xbutton.button) {
//
//    default:
//            break;
//        }
//        break;
//    case ButtonRelease:
//        break;
//    default:
//    break;
//    }
//
    if (abort_action && xstuff->collect_points) {
        /* clear selection */
//        switch (xstuff->sel_type) {
//        case SELECTION_TYPE_RECT:
//            select_region(gapp->gui,
//                x, y, last_b1down_x, last_b1down_y, FALSE);
//            break;
//        case SELECTION_TYPE_VERT:
//            select_vregion(gapp->gui, x, last_b1down_x, FALSE);
//            break;
//        case SELECTION_TYPE_HORZ:
//            select_hregion(gapp->gui, y, last_b1down_y, FALSE);
//            break;
//        }
        /* abort action */
        xstuff->npoints = 0;
        xstuff->collect_points = FALSE;
//        set_cursor(gapp->gui, -1);
//        set_left_footer(NULL);
    } else
    if (undo_point) {
        /* previous action */
    } else
    if (xstuff->npoints_requested &&
        xstuff->npoints == xstuff->npoints_requested) {
//        int ret;
//        unsigned int i;
//        VPoint *vps = xmalloc(xstuff->npoints*sizeof(VPoint));
//        for (i = 0; i < xstuff->npoints; i++) {
//            XPoint xp = xstuff->xps[i];
//            x11_dev2VPoint(xp.x, xp.y, &vps[i]);
//        }
//        /* return points to caller */
//        ret = xstuff->point_sink(xstuff->npoints, vps, xstuff->sink_data);
//        if (ret != RETURN_SUCCESS) {
//            XBell(xstuff->disp, 50);
//        }
//
//        xfree(vps);
//
        xstuff->npoints_requested = 0;
        xstuff->collect_points = FALSE;
        xstuff->npoints = 0;
//        set_cursor(gapp->gui, -1);
//
//        snapshot_and_update(gapp->gp, TRUE);
    }
    return return_value;
}

