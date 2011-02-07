#ifndef __CANVASWIDGET_H_
#define __CANVASWIDGET_H_

#include <QWidget>

class CanvasWidget : public QWidget
{
   Q_OBJECT

public:
    CanvasWidget(QWidget *parent = 0);
    QImage *pixmap;

protected:
    bool event(QEvent *event);

private:
    void paintEvent(QPaintEvent *event);
};

#endif /* __CANVASWIDGET_H_ */
