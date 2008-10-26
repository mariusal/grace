#ifndef __CANVASWIDGET_H_
#define __CANVASWIDGET_H_

#include <QWidget>
#include <QPicture>

class CanvasWidget : public QWidget
{
   Q_OBJECT

public:
  CanvasWidget(QWidget *parent = 0);

private:
  QPicture qtstream;
  void paintEvent(QPaintEvent *event);
};

#endif /* __CANVASWIDGET_H_ */
