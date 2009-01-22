#include <QPainter>
#include <QFont>

#include "canvaswidget.h"
#include <iostream>
using namespace std;


CanvasWidget::CanvasWidget(QWidget *parent) : QWidget(parent)
{
/*  static const QPointF points[3] = {
    QPointF(10.0, 80.0),
    QPointF(20.0, 10.0),
    QPointF(80.0, 30.0),
  };

  QPainter painter(&qtstream);
  painter.drawPolyline(points, 3);*/

  //printf("Hello world\n");
//  canvas_set_prstream(grace_get_canvas(m_gapp->grace), &qtstream);
  //select_device(grace_get_canvas(m_gapp->grace), m_gapp->rt->tdevice);
  //gproject_render(gp);
  
  //printf("Hello world2\n");
    pixmap = 0;
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
	canvas_set_prstream(grace_get_canvas(gapp->grace), &qtstream);

	select_device(grace_get_canvas(gapp->grace), gapp->rt->tdevice);
	gproject_render(gp);

	if (pixmap != 0) {
	    delete pixmap;
	}
	pixmap = new QPixmap(pg->width, pg->height);
	QPainter p(pixmap);
	qtstream.play(&p);

	if (quark_is_active(gr)) {
	    //draw_focus(gr);
	}
	//reset_crosshair(gapp->gui, FALSE);
	//region_need_erasing = FALSE;

	update();
	//x11_redraw(xstuff->xwin, 0, 0, xstuff->win_w, xstuff->win_h);

	//XFlush(xstuff->disp);

	//unset_wait_cursor();
    }
}

void CanvasWidget::draw(QString fileName)
{
  /* Global init. Needs to be done only once. */ 
//  grace_init(); 
     
  /* Allocate Grace object */
  grace = grace_new("");
  if (!grace) {
    exit(1);
  }

  /* Get the Canvas object */ 
  canvas = grace_get_canvas(grace); 
  if (!canvas) { 
      exit(1); 
  } 

  /* Register a canvas device (here - the PS device) */ 
  hdevice = register_qt_drv(canvas);

  Device_entry *d;
  d = get_device_props(canvas, hdevice);
  d->fontrast = FONT_RASTER_AA_SMART;

  QByteArray bytes = fileName.toAscii();
  const char *ptr = bytes.data();

  /* Open input stream from a project file */
  grf = grfile_openr(ptr);
  if (!grf) {
    errmsg("Can't open input for reading");
    exit(1);
  }

  /* Parse & load the project */
  gp = gproject_load(grace, grf, AMEM_MODEL_SIMPLE);
  if (!gp) {
    errmsg("Failed parsing project");
    exit(1);
  }

  /* Free the stream */
  grfile_free(grf);

  /* Sync device dimensions with the plot page size */
  grace_sync_canvas_devices(gp);
  setMinimumSize(d->pg.width, d->pg.height);


  /* Assign the output stream */
  //canvas_set_prstream(grace_get_canvas(gapp->grace), &xstream);
  //canvas_set_prstream(grace_get_canvas(grace), fpout);

  canvas_set_prstream(canvas, &qtstream);
  select_device(canvas, hdevice);
  gproject_render(gp);
  
 /* Free the GProject object */
//  gproject_free(gp);

  /* Free the Grace object (or, it could be re-used for other projects) */
//  grace_free(grace);

  update();
}

