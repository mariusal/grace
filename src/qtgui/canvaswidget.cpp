#include <QApplication>
#include <QPainter>
#include <QFont>
#include <QMouseEvent>
#include "mainwindow.h"
#include "canvaswidget.h"

/* selection type */
#define SELECTION_TYPE_NONE 0
#define SELECTION_TYPE_RECT 1
#define SELECTION_TYPE_VERT 2
#define SELECTION_TYPE_HORZ 3
/* #define SELECTION_TYPE_POLY 4 */

CanvasWidget* CanvasWidget::instance = 0;

CanvasWidget::CanvasWidget(QWidget *parent) :
    QWidget(parent), pixmap(0)
{
    xstuff = gapp->gui->xstuff;
    xstuff->collect_points = FALSE;
    region_need_erasing = FALSE;

    setMouseTracking(true);
    setFocusPolicy(Qt::ClickFocus);
}

void CanvasWidget::setStatic()
{
    instance = this;
}

void CanvasWidget::setMainWindow(MainWindow *mainWindow)
{
    this->mainWindow = mainWindow;
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

    /* 8 bits per color channel (i.e., 256^3 colors) */
    /* are defined in CANVAS_BPCC canvas.h file. */
    /* Use alpha channel to be able to use QPainter::setCompositionMode(CompositionMode mode)*/
    /* Image composition using alpha blending are faster using premultiplied ARGB32 than with plain ARGB32 */
    if (pixmap == 0) {
        pixmap = new QImage(w, h, QImage::Format_ARGB32_Premultiplied);
    } if (xstuff->win_w != w || xstuff->win_h != h) {
        delete pixmap;
        pixmap = new QImage(w, h, QImage::Format_ARGB32_Premultiplied);
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

void switch_current_graph(Quark *gr)
{
    if (quark_is_active(gr)) {
        GraceApp *gapp = gapp_from_quark(gr);
        Quark *cg = graph_get_current(gproject_get_top(gapp->gp));
        CanvasWidget *cw = CanvasWidget::instance;

        select_graph(gr);
        cw->draw_focus(cg);
        cw->draw_focus(gr);
        cw->mainWindow->update_all();
        //graph_set_selectors(gr);
        cw->update_locator_lab(cg, NULL);
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

/*
 * slide an xor'ed bbox shifted by shift_*, (optionally erasing previous one)
 */
void CanvasWidget::slide_region(GUI *gui, view bb, double shift_x, double shift_y, int erase)
{
    double x1, x2, y1, y2;
    VPoint vp;

    vp.x = bb.xv1;
    vp.y = bb.yv1;
    qt_VPoint2dev(&vp, &x1, &y1);
    x1 += shift_x;
    y1 += shift_y;

    vp.x = bb.xv2;
    vp.y = bb.yv2;
    qt_VPoint2dev(&vp, &x2, &y2);
    x2 += shift_x;
    y2 += shift_y;

    select_region(gui, x1, y1, x2, y2, erase);
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

void CanvasWidget::select_vregion(GUI *gui, double x1, double x2, int erase)
{
    select_region(gui, x1, xstuff->f_y1, x2, xstuff->f_y2, erase);
}

void CanvasWidget::select_hregion(GUI *gui, double y1, double y2, int erase)
{
    select_region(gui, xstuff->f_x1, y1, xstuff->f_x2, y2, erase);
}

void CanvasWidget::aux_XDrawRectangle(GUI *gui, double x, double y, double width, double height)
{
    QPainter painter(pixmap);
    painter.setPen(QPen(Qt::white));
    painter.setBrush(QBrush(Qt::NoBrush));
    painter.setCompositionMode(QPainter::CompositionMode_Exclusion);
    painter.drawRect(QRectF(x, y, width, height));
    repaint(); // TODO: repaint just drawn rectangle
}

void CanvasWidget::aux_XFillRectangle(GUI *gui, double x, double y, double width, double height)
{
    QPainter painter(pixmap);
    painter.setPen(QPen(Qt::NoPen));
    painter.setBrush(QBrush(Qt::white));
    painter.setCompositionMode(QPainter::CompositionMode_Exclusion);
    painter.drawRect(QRectF(x, y, width, height));
}

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

    //raise_explorer(gapp->gui, q);

    return RETURN_SUCCESS;
}

void CanvasWidget::set_action(GUI *gui, unsigned int npoints, int seltype,
    CanvasPointSink sink, void *data)
{
    xstuff->npoints = 0;
    xstuff->npoints_requested = npoints;
    xstuff->point_sink = sink;
    xstuff->sink_data  = data;
    xstuff->sel_type = seltype;

    xstuff->collect_points = TRUE;

//    XmProcessTraversal(xstuff->canvas, XmTRAVERSE_CURRENT);
}

void CanvasWidget::actionZoom()
{
    setCursor(Qt::CrossCursor);
    set_action(gapp->gui, 2, SELECTION_TYPE_RECT, zoom_sink, gapp);
}

void CanvasWidget::actionZoomY()
{
    setCursor(Qt::CrossCursor);
    set_action(gapp->gui, 2, SELECTION_TYPE_HORZ, zoomy_sink, gapp);
}

void CanvasWidget::actionZoomX()
{
    setCursor(Qt::CrossCursor);
    set_action(gapp->gui, 2, SELECTION_TYPE_VERT, zoomx_sink, gapp);
}

void CanvasWidget::actionAddText()
{
    setCursor(Qt::IBeamCursor);
    set_action(gapp->gui, 1, SELECTION_TYPE_NONE, atext_sink, gapp);
}

void CanvasWidget::completeAction(double x, double y)
{
    if (abort_action && xstuff->collect_points) {
        errmsg("complete action clear selection");
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
        //set_cursor(gapp->gui, -1);
        setCursor(Qt::ArrowCursor);
        mainWindow->set_left_footer(NULL);
    } else
    if (undo_point) {
        /* previous action */
        errmsg("complete action undo");
    } else
    if (xstuff->npoints_requested &&
        xstuff->npoints == xstuff->npoints_requested) {
        errmsg("complete action points requested");
        int ret;
        unsigned int i;
        VPoint *vps = (VPoint*) xmalloc(xstuff->npoints*sizeof(VPoint));
        for (i = 0; i < xstuff->npoints; i++) {
            XPoint xp = xstuff->xps[i];
            qt_dev2VPoint(xp.x, xp.y, &vps[i]);
        }
        /* return points to caller */
        ret = xstuff->point_sink(xstuff->npoints, vps, xstuff->sink_data);
        if (ret != RETURN_SUCCESS) {
            //XBell(xstuff->disp, 50);
        }

        xfree(vps);

        xstuff->npoints_requested = 0;
        xstuff->collect_points = FALSE;
        xstuff->npoints = 0;
        //set_cursor(gapp->gui, -1);
        setCursor(Qt::ArrowCursor);

        mainWindow->snapshot_and_update(gapp->gp, TRUE);
    }
}

void CanvasWidget::mousePressEvent(QMouseEvent *event)
{
    QPointF point = event->posF();
    double x = point.x();
    double y = point.y();

    VPoint vp;
    Quark *cg = graph_get_current(gproject_get_top(gapp->gp));
    undo_point = FALSE;
    abort_action = FALSE;

    qt_dev2VPoint(x, y, &vp);

    switch (event->button()) {
    case Qt::LeftButton:
        if (event->modifiers() & Qt::ControlModifier) {
            errmsg("press left and control");
            ct.vp = vp;
            ct.include_graphs = FALSE;
            if (on_focus) {
                resize_region(gapp->gui, xstuff->f_v, on_focus, 0, 0, FALSE);
            } else if (find_target(gapp->gp, &ct) == RETURN_SUCCESS) {
                slide_region(gapp->gui, ct.bbox, 0, 0, FALSE);
            }
        } else {
            if (xstuff->collect_points) {
                XPoint xp;
                xp.x = x;
                xp.y = y;
                xstuff->npoints++;
                xstuff->xps = (XPoint*) xrealloc(xstuff->xps, xstuff->npoints * sizeof(XPoint));
                xstuff->xps[xstuff->npoints - 1] = xp;
                select_region(gapp->gui, x, y, x, y, FALSE);
            } else if (gapp->gui->focus_policy == FOCUS_CLICK) {
                cg = next_graph_containing(cg, &vp);
            }
            update_locator_lab(cg, &vp);
        }

        last_b1down_x = x;
        last_b1down_y = y;

        if (!xstuff->collect_points) {
            //set_cursor(gapp->gui, 5);
            setCursor(Qt::ClosedHandCursor);
        }

        break;

    case Qt::MidButton:
        fprintf(stderr, "Button2\n");
        break;

    default:
        break;
}

}

void CanvasWidget::mouseMoveEvent(QMouseEvent *event)
{
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
        errmsg("move collect points");
            switch (xstuff->sel_type) {
            case SELECTION_TYPE_RECT:
                select_region(gapp->gui, x, y, last_b1down_x, last_b1down_y, TRUE);
                break;
            case SELECTION_TYPE_VERT:
                select_vregion(gapp->gui, x, last_b1down_x, TRUE);
                break;
            case SELECTION_TYPE_HORZ:
                select_hregion(gapp->gui, y, last_b1down_y, TRUE);
                break;
            }
        } else
        if (event->buttons() & Qt::LeftButton) {
            if (event->modifiers() & Qt::ControlModifier) {
                errmsg("move left and control");
                if (on_focus) {
                    resize_region(gapp->gui, xstuff->f_v, on_focus,
                        x - last_b1down_x, y - last_b1down_y, TRUE);
                } else
                if (ct.found) {
                    slide_region(gapp->gui, ct.bbox,
                        x - last_b1down_x, y - last_b1down_y, TRUE);
                }
            } else {
                errmsg("move scroll");
                mainWindow->scroll_pix(last_b1down_x - x, last_b1down_y - y);
            }
        } else {
            //errmsg("move");
            if (gapp->gui->focus_policy == FOCUS_FOLLOWS) {
                cg = next_graph_containing(cg, &vp);
            }

            if (event->modifiers() & Qt::ControlModifier) {
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
                    setCursor(Qt::SizeAllCursor);
                } else {
                    //set_cursor(gapp->gui, -1);
                    setCursor(Qt::ArrowCursor);
                }
            }
        }

        update_locator_lab(cg, &vp);

        last_mouse_move_x = x;
        last_mouse_move_y = y;
}

void CanvasWidget::mouseReleaseEvent(QMouseEvent *event)
{
    QPointF point = event->posF();
    double x = point.x();
    double y = point.y();

    VPoint vp;

    switch (event->button()) {
    case Qt::LeftButton:
        if (event->modifiers() & Qt::ControlModifier) {
            qt_dev2VPoint(x, y, &vp);
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
            } else if (ct.found) {
                slide_region(gapp->gui, ct.bbox, x - last_b1down_x, y
                        - last_b1down_y, FALSE);

                move_target(&ct, &vp);
            }
            ct.found = FALSE;

            mainWindow->snapshot_and_update(gapp->gp, TRUE);
        }
        if (!xstuff->collect_points) {
            //set_cursor(gapp->gui, -1);
            setCursor(Qt::ArrowCursor);
        }
        break;
    default:
        break;
    }

    completeAction(x, y);
}

void CanvasWidget::contextMenuEvent(QContextMenuEvent *event)
{
    QPointF point = QPointF(event->pos());
    errmsg("right button");
    if (xstuff->collect_points) {
        undo_point = TRUE;
        if (xstuff->npoints) {
            xstuff->npoints--;
        }
        if (xstuff->npoints == 0) {
            abort_action = TRUE;
        }
    }// else {
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

    completeAction(point.x(), point.y());
}

void CanvasWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
     //            ct.vp = vp;
     //            ct.include_graphs = (xbe->state & ControlMask) ? FALSE:TRUE;
     //            if (find_target(gapp->gp, &ct) == RETURN_SUCCESS) {
     //                raise_explorer(gapp->gui, ct.q);
     //                ct.found = FALSE;
     //            }
     //        }

}

void CanvasWidget::wheelEvent(QWheelEvent *event)
{
    QWidget::wheelEvent(event);
//case Button4:
//        scroll(drawing_window, TRUE, xbe->state & ControlMask);
//        break;
//case Button5:
//        scroll(drawing_window, FALSE, xbe->state & ControlMask);
//        break;
}

void CanvasWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() & Qt::Key_Escape) {
        abort_action = TRUE;
    }

    completeAction(0, 0);
}

void CanvasWidget::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() & Qt::Key_Control) {
        if (on_focus) {
            //set_cursor(gapp->gui, -1);
            setCursor(Qt::ArrowCursor);
        } else if (ct.found) {
            slide_region(gapp->gui, ct.bbox, last_mouse_move_x - last_b1down_x, last_mouse_move_y - last_b1down_y, FALSE);
            ct.found = FALSE;
        }
    }
}

