/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 *
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 *
 * Copyright (c) 2012 Grace Development Team
 *
 * Maintained by Evgeny Stambulchik
 *
 *
 *                           All Rights Reserved
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* Qt widgets */

#include <QKeyEvent>
#include <QWidget>

extern "C" {
#include <widgets.h>
#include "events.h"
}


class KeyCB : public QObject {
    Q_OBJECT
public:
    KeyCB(QObject *parent) :
            QObject(parent)
    {
    }

    Key_CBData *cbdata;

protected:
    bool eventFilter(QObject *obj, QEvent *event)
    {
        if (event->type() != QEvent::KeyPress) {
            return QObject::eventFilter(obj, event);
        }

        KeyEvent kevent;
        kevent.key = KEY_NONE;
        kevent.w = cbdata->w;
        kevent.anydata = cbdata->anydata;

        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        switch (keyEvent->key()) {
        case Qt::Key_Up: /* Up */
            kevent.key = KEY_UP;
            break;
        case Qt::Key_Down: /* Down */
            kevent.key = KEY_DOWN;
            break;
        default:
            return QObject::eventFilter(obj, event);
        }

        cbdata->cbproc(&kevent);

        return false;
    }
};

void AddWidgetKeyPressCB(Widget w, Key_CBProc cbproc, void *anydata)
{
    Key_CBData *cbdata;

    cbdata = (Key_CBData *) xmalloc(sizeof(Key_CBData));
    cbdata->w = w;
    cbdata->cbproc = cbproc;
    cbdata->anydata = anydata;

    KeyCB *keyCB = new KeyCB(w);
    keyCB->cbdata = cbdata;
    w->installEventFilter(keyCB);
}

#include "qt.moc"
