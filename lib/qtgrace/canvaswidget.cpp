#include <QPainter>
#include <QFont>

#include "canvaswidget.h"

CanvasWidget::CanvasWidget(QWidget *parent) : QWidget(parent)
{
}

void CanvasWidget::paintEvent(QPaintEvent *)
{
   QPainter painter(this);
   painter.setPen(Qt::blue);
   painter.setFont(QFont("Arial", 30));
   painter.drawText(rect(), Qt::AlignCenter, "Qt");
}

