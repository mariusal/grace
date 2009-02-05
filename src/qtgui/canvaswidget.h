#ifndef __CANVASWIDGET_H_
#define __CANVASWIDGET_H_

#include <QWidget>

extern "C" {
#include <grace/grace.h>
#include "qtgrace.h"
}

class CanvasWidget : public QWidget
{
   Q_OBJECT

public:
  CanvasWidget(QWidget *parent = 0);
  void qtdrawgraph(const GProject*);

protected:
  void mousePressEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);

private:
  QPixmap *pixmap;
  void xdrawgrid();
  void paintEvent(QPaintEvent *event);

  Grace *grace;
  Canvas *canvas;
  GProject *gp;
  int hdevice;
  GrFILE *grf;
  //FILE *fpout;

};

#endif /* __CANVASWIDGET_H_ */
