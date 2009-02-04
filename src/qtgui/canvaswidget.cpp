#include <QApplication>
#include <QPainter>
#include <QFont>

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

void CanvasWidget::qtdrawgraph(const GProject *gp)
{
    Quark *project = gproject_get_top(gp);
    GraceApp *gapp = gapp_from_quark(project);

    if (gapp && gapp->gui->inwin) {
	//X11Stuff *xstuff = gapp->gui->xstuff;
	//Quark *gr = graph_get_current(project);
	Project *pr = project_get_data(project);
	Quark *gr = pr->cg;
	Device_entry *d = get_device_props(grace_get_canvas(gapp->grace), gapp->rt->tdevice);
	Page_geometry *pg = &d->pg;
	//float dpi = gapp->gui->zoom*xstuff->dpi;
	float dpi = gapp->gui->zoom*physicalDpiX();
	//X11stream xstream;

	//set_wait_cursor();
	QApplication::setOverrideCursor(Qt::WaitCursor);

	if (dpi != pg->dpi) {
	    int wpp, hpp;
	    project_get_page_dimensions(project, &wpp, &hpp);

	    pg->width  = (unsigned long) (wpp*dpi/72);
	    pg->height = (unsigned long) (hpp*dpi/72);
	    pg->dpi = dpi;
	}

	//resize_drawables(pg->width, pg->height);
	setMinimumSize(pg->width, pg->height);

	//xdrawgrid(xstuff);

	//xstream.screen = DefaultScreenOfDisplay(xstuff->disp);
	//xstream.pixmap = xstuff->bufpixmap;
	if (pixmap != 0) {
	    delete pixmap;
	}
	pixmap = new QPixmap(pg->width, pg->height);
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

	//unset_wait_cursor();
	QApplication::restoreOverrideCursor();
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

