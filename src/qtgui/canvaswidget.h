#ifndef __CANVASWIDGET_H_
#define __CANVASWIDGET_H_

#include <QWidget>
#include <QLabel>

extern "C" {
#include <grace/grace.h>
#include "qtgrace.h"
}

typedef struct _QtStuff QtStuff;

class MainWindow;

class CanvasWidget : public QWidget
{
   Q_OBJECT

public:
  CanvasWidget(QWidget *parent = 0);
  void setMainWindow(MainWindow *mainWindow);
  void setGraceApp(GraceApp *gapp);
  void setLocatorBar(QLabel *locatorBar);
  void qtdrawgraph(const GProject*);
  void set_tracker_string(char *s);
  void update_locator_lab(Quark *cg, VPoint *vpp);

protected:
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    bool event(QEvent *event);

private:
    MainWindow *mainWindow;
    GraceApp *gapp;
    QLabel *locatorBar;
    QImage *pixmap;
    QtStuff *xstuff;

    int region_need_erasing;
    double last_b1down_x, last_b1down_y;   /* coords of last event */
    int undo_point;
    int abort_action;

    void xdrawgrid();
    void resize_drawables(unsigned int w, unsigned int h);
    void qt_VPoint2dev(const VPoint *vp, double *x, double *y);
    void qt_dev2VPoint(double x, double y, VPoint *vp);
    void draw_focus(Quark *gr);
    Quark* next_graph_containing(Quark *q, VPoint *vp);
    void paintEvent(QPaintEvent *event);

    void resize_region(GUI *gui, view bb, int on_focus,
            double shift_x, double shift_y, int erase);
    void select_region(GUI *gui, double x1, double y1, double x2, double y2, int erase);
    void aux_XDrawRectangle(GUI *gui, double x1, double y1, double x2, double y2);
    void aux_XFillRectangle(GUI *gui, double x, double y, double width, double height);

    Grace *grace;
    Canvas *canvas;
    GProject *gp;
    int hdevice;
    GrFILE *grf;
    //FILE *fpout;
};

#endif /* __CANVASWIDGET_H_ */
