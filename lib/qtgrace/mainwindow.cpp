#include <QPainter>
#include <QFont>

#include "mainwindow.h"

#include "canvaswidget.h"

MainWindow::MainWindow(QMainWindow *parent) : QMainWindow(parent)
{
  ui.setupUi(this);
  new CanvasWidget(ui.canvasFrame);
}

void MainWindow::on_actionExit_triggered()
{
  close();
}

void MainWindow::drawGraph(const GProject *gp)
{
/*    Quark *project = gproject_get_top(gp);
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
        
        xstream.screen = DefaultScreenOfDisplay(xstuff->disp);
        xstream.pixmap = xstuff->bufpixmap;
        canvas_set_prstream(grace_get_canvas(gapp->grace), &xstream);

        select_device(grace_get_canvas(gapp->grace), gapp->rt->tdevice);
	gproject_render(gp);

        if (quark_is_active(gr)) {
            draw_focus(gr);
        }
        reset_crosshair(gapp->gui, FALSE);
        region_need_erasing = FALSE;

        x11_redraw(xstuff->xwin, 0, 0, xstuff->win_w, xstuff->win_h);

        XFlush(xstuff->disp);

	unset_wait_cursor();
    }*/
}
