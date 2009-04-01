#ifndef __CANVASWIDGET_H_
#define __CANVASWIDGET_H_

#include <QWidget>
#include <QLabel>

extern "C" {
#include <globals.h>
#include <grace/grace.h>
#include <graceapp.h>
#include <core_utils.h>
#include <xprotos.h>
}

typedef struct {
    VPoint vp;
    int include_graphs;
    Quark *q;
    int part;
    view bbox;
    int found;
} canvas_target;

class CanvasWidget : public QWidget
{
   Q_OBJECT

public:
    CanvasWidget(QWidget *parent = 0);
    void set_action(GUI *gui, unsigned int npoints, int seltype,
        CanvasPointSink sink, void *data);

    QImage *pixmap;


protected:
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);

private:
    X11Stuff *xstuff;

    int region_need_erasing;
    int last_b1down_x, last_b1down_y;   /* coords of last event */
    int last_mouse_move_x, last_mouse_move_y;
    int undo_point;
    int abort_action;
    canvas_target ct;
    int on_focus;

    void qt_VPoint2dev(const VPoint *vp, double *x, double *y);
    void qt_dev2VPoint(double x, double y, VPoint *vp);
    Quark* next_graph_containing(Quark *q, VPoint *vp);
    void paintEvent(QPaintEvent *event);

    void completeAction(int x, int y);

    Grace *grace;
    Canvas *canvas;
    GProject *gp;
    int hdevice;
    GrFILE *grf;
    //FILE *fpout;
};

#endif /* __CANVASWIDGET_H_ */
