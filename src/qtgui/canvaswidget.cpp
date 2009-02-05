#include <QApplication>
#include <QPainter>
#include <QFont>
extern "C" {
    #include <core_utils.h>
}
#include "canvaswidget.h"

CanvasWidget::CanvasWidget(QWidget *parent) :
    QWidget(parent), pixmap(0)
{
}

void CanvasWidget::paintEvent(QPaintEvent *)
{
    if (pixmap != 0) {
	QPainter painter(this);
	painter.drawPixmap(0, 0, *pixmap);
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

        setMinimumSize(pg->width, pg->height);

        if (pixmap != 0) {
            delete pixmap;
        }
        pixmap = new QPixmap(pg->width, pg->height);

        xdrawgrid();

        canvas_set_prstream(grace_get_canvas(gapp->grace), pixmap);

        select_device(grace_get_canvas(gapp->grace), gapp->rt->tdevice);
        gproject_render(gp);

        if (quark_is_active(gr)) {
            //draw_focus(gr);
        }
        //reset_crosshair(gapp->gui, FALSE);
        //region_need_erasing = FALSE;

        update();
        //x11_redraw(xstuff->xwin, 0, 0, xstuff->win_w, xstuff->win_h);

        //XFlush(xstuff->disp);

        unset_wait_cursor();
    }
}

void CanvasWidget::mousePressEvent(QMouseEvent *event)
{
}

void CanvasWidget::mouseMoveEvent(QMouseEvent *event)
{
}

void CanvasWidget::mouseReleaseEvent(QMouseEvent *event)
{
}

