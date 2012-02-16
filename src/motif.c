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

/* Motif widgets */

#include "widgets.h"

#include "events.h"

static void keyCB(Widget w, XtPointer client_data, XEvent *event, Boolean *cont)
{
    XKeyEvent *xke;
    KeySym keybuf;
    KeyEvent kevent;
    Key_CBData *cbdata = (Key_CBData *) client_data;

    if (event->type != KeyPress) return;

    kevent.key = KEY_NONE;
    kevent.w = w;
    kevent.anydata = cbdata->anydata;

    xke = (XKeyEvent *) event;
    keybuf = XLookupKeysym(xke, 0);

    switch (keybuf) {
    case XK_Up: /* Up */
        kevent.key = KEY_UP;
        break;
    case XK_Down: /* Down */
        kevent.key = KEY_DOWN;
        break;
    }

    cbdata->cbproc(&kevent);
}

void AddWidgetKeyPressCB(Widget w, Key_CBProc cbproc, void *anydata)
{
    Key_CBData *cbdata;

    cbdata = (Key_CBData *) xmalloc(sizeof(Key_CBData));
    cbdata->w = w;
    cbdata->cbproc = cbproc;
    cbdata->anydata = anydata;

    XtAddEventHandler(w, KeyPressMask, False, keyCB, cbdata);
}
