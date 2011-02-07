#include <QPainter>
#include <QPaintEvent>
#include <QDateTime>
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


bool CanvasWidget::event(QEvent *event)
{
    CanvasEvent cevent;
    cevent.modifiers = NO_MODIFIER;
    cevent.button    = NO_BUTTON;

    QMouseEvent *xbe;
    QWheelEvent *wheelEvent;
    QKeyEvent *xke;

    switch (event->type()) {
    case QEvent::MouseMove:
        cevent.type = MOUSE_MOVE;
        xbe = (QMouseEvent*) event;
        cevent.x = xbe->x();
        cevent.y = xbe->y();
        if (xbe->buttons() & Qt::LeftButton) {
            cevent.button = cevent.button ^ LEFT_BUTTON;
        }
        if (xbe->modifiers() & Qt::ControlModifier) {
            cevent.modifiers = cevent.modifiers ^ CONTROL_MODIFIER;
        }
        //printf("%s x=%d, y=%d\n", "mouse move", cevent.x, cevent.y);
        break;
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseButtonPress:
        cevent.type = MOUSE_PRESS;
        xbe = (QMouseEvent*) event;
        cevent.udata = xbe;
        cevent.x = xbe->x();
        cevent.y = xbe->y();
        switch (xbe->button()) {
        case Qt::LeftButton:
            cevent.button = cevent.button ^ LEFT_BUTTON;
            cevent.time = QDateTime::currentMSecsSinceEpoch();
            break;
        case Qt::MiddleButton:
            cevent.button = cevent.button ^ MIDDLE_BUTTON;
            break;
        case Qt::RightButton:
            cevent.button = cevent.button ^ RIGHT_BUTTON;
            break;
        }
        if (xbe->modifiers() & Qt::ControlModifier) {
            cevent.modifiers = cevent.modifiers ^ CONTROL_MODIFIER;
        }
        printf("%s x=%d, y=%d\n", "button press", cevent.x, cevent.y);
        break;
    case QEvent::MouseButtonRelease:
        cevent.type = MOUSE_RELEASE;
        xbe = (QMouseEvent*) event;
        cevent.x = xbe->x();
        cevent.y = xbe->y();
        switch (xbe->button()) {
        case Qt::LeftButton:
            cevent.button = cevent.button ^ LEFT_BUTTON;
            break;
        }
        if (xbe->modifiers() & Qt::ControlModifier) {
            cevent.modifiers = cevent.modifiers ^ CONTROL_MODIFIER;
        }
        printf("%s x=%d, y=%d\n", "button release", cevent.x, cevent.y);
        break;
    case QEvent::Wheel:
        cevent.type = MOUSE_PRESS;
        wheelEvent = (QWheelEvent*) event;
        cevent.udata = wheelEvent;
        cevent.x = wheelEvent->x();
        cevent.y = wheelEvent->y();
        if (wheelEvent->delta() > 0) {
            printf("%s\n", "wheel up");
            cevent.button = cevent.button ^ WHEEL_UP_BUTTON;
        } else {
            printf("%s\n", "wheel down");
            cevent.button = cevent.button ^ WHEEL_DOWN_BUTTON;
        }
        if (wheelEvent->modifiers() & Qt::ControlModifier) {
            cevent.modifiers = cevent.modifiers ^ CONTROL_MODIFIER;
        }
        break;
    case QEvent::KeyPress:
        cevent.type = KEY_PRESS;
        xke = (QKeyEvent*) event;
        switch (xke->key()) {
        case Qt::Key_Escape: /* Esc */
            cevent.key = KEY_ESCAPE;
            break;
        case Qt::Key_Plus: /* "Grey" plus */
            cevent.key = KEY_PLUS;
            break;
        case Qt::Key_Minus: /* "Grey" minus */
            cevent.key = KEY_MINUS;
            break;
        case Qt::Key_1:
            cevent.key = KEY_1;
            break;
        }
        if (xke->modifiers() & Qt::ControlModifier) {
            cevent.modifiers = cevent.modifiers ^ CONTROL_MODIFIER;
        }
        break;
    case QEvent::KeyRelease:
        cevent.type = KEY_RELEASE;
        xke = (QKeyEvent*) event;
        if (xke->modifiers() & Qt::ControlModifier) {
            cevent.modifiers = cevent.modifiers ^ CONTROL_MODIFIER;
        }
        break;
    default:
        return QWidget::event(event);
    }

    canvas_event(&cevent);

    return true;
}
