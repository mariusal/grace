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
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);

private:
    void paintEvent(QPaintEvent *event);
};

#endif /* __CANVASWIDGET_H_ */
