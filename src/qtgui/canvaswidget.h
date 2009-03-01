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

class MainWindow;

class CanvasWidget : public QWidget
{
   Q_OBJECT

public:
    CanvasWidget(QWidget *parent = 0);
    static CanvasWidget *instance;
    MainWindow *mainWindow;
    void setStatic();
    void setMainWindow(MainWindow *mainWindow);
    void setLocatorBar(QLabel *locatorBar);
    void qtdrawgraph(const GProject*);
    void set_tracker_string(char *s);
    void update_locator_lab(Quark *cg, VPoint *vpp);
    void draw_focus(Quark *gr);
    void set_action(GUI *gui, unsigned int npoints, int seltype,
        CanvasPointSink sink, void *data);

    void actionZoom();
    void actionZoomY();
    void actionZoomX();
    void actionAddText();


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
    QLabel *locatorBar;
    QImage *pixmap;
    X11Stuff *xstuff;

    int region_need_erasing;
    double last_b1down_x, last_b1down_y;   /* coords of last event */
    double last_mouse_move_x, last_mouse_move_y;
    int undo_point;
    int abort_action;
    canvas_target ct;
    int on_focus;

    void xdrawgrid();
    void resize_drawables(unsigned int w, unsigned int h);
    void qt_VPoint2dev(const VPoint *vp, double *x, double *y);
    void qt_dev2VPoint(double x, double y, VPoint *vp);
    Quark* next_graph_containing(Quark *q, VPoint *vp);
    void paintEvent(QPaintEvent *event);

    void slide_region(GUI *gui, view bb, double shift_x, double shift_y, int erase);
    void resize_region(GUI *gui, view bb, int on_focus, double shift_x, double shift_y, int erase);
    void select_region(GUI *gui, double x1, double y1, double x2, double y2, int erase);
    void select_vregion(GUI *gui, double x1, double x2, int erase);
    void select_hregion(GUI *gui, double y1, double y2, int erase);
    void aux_XDrawRectangle(GUI *gui, double x, double y, double width, double height);
    void aux_XFillRectangle(GUI *gui, double x, double y, double width, double height);
    void completeAction(double x, double y);

    Grace *grace;
    Canvas *canvas;
    GProject *gp;
    int hdevice;
    GrFILE *grf;
    //FILE *fpout;
};

#endif /* __CANVASWIDGET_H_ */
