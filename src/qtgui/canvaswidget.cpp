#include <QPainter>
#include <QPaintEvent>
#include "mainwindow.h"
#include "canvaswidget.h"
extern "C" {
#include "events.h"
}

CanvasWidget::CanvasWidget(QWidget *parent) :
    QWidget(parent), pixmap(0)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::ClickFocus);
}

void CanvasWidget::paintEvent(QPaintEvent *event)
{
    QRect r = event->rect();
    if (pixmap != 0) {
        QPainter painter(this);
        painter.drawImage(r, *pixmap, r);
    }
}


void CanvasWidget::mouseMoveEvent(QMouseEvent *event)
{

}

void CanvasWidget::mousePressEvent(QMouseEvent *event)
{

}

void CanvasWidget::mouseReleaseEvent(QMouseEvent *event)
{

}

void CanvasWidget::contextMenuEvent(QContextMenuEvent *event)
{

}

void CanvasWidget::mouseDoubleClickEvent(QMouseEvent *event)
{

}

void CanvasWidget::wheelEvent(QWheelEvent *event)
{

}

void CanvasWidget::keyPressEvent(QKeyEvent *event)
{

}

void CanvasWidget::keyReleaseEvent(QKeyEvent *event)
{

}
