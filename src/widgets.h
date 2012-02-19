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

/* Widgets */

#ifndef __WIDGETS_H_
#define __WIDGETS_H_

#include <config.h>

#ifdef MOTIF_GUI
/* for Widget */
#include <X11/Intrinsic.h>
#endif /* MOTIF_GUI */

#ifdef QT_GUI
#include "qtgui/qtinc.h"
#endif /* QT_GUI */

void ManageChild(Widget w);
void UnmanageChild(Widget w);
int IsManaged(Widget w);
void *GetUserData(Widget w);
void SetUserData(Widget w, void *udata);
void SetSensitive(Widget w, int onoff);

typedef struct {
    int key;

    Widget w;
    void *anydata;
} KeyEvent;

typedef void (*Key_CBProc)(KeyEvent *event);
typedef struct {
    Widget w;
    Key_CBProc cbproc;
    void *anydata;
} Key_CBData;

void AddWidgetKeyPressCB(Widget w, Key_CBProc cbproc, void *anydata);

Widget CreateVContainer(Widget parent);
Widget CreateHContainer(Widget parent);

Widget CreateForm(Widget parent);
Widget CreateDialogForm(Widget parent, const char *s);
void SetDialogFormResizable(Widget form, int onoff);
void AddDialogFormChild(Widget form, Widget child);
void FixateDialogFormChild(Widget w);

Widget CreateLabel(Widget parent, char *s);
void SetLabel(Widget w, char *s);
void AlignLabel(Widget w, int alignment);

/* Text item CB procedure */
typedef void (*TItem_CBProc)(
    Widget ti,
    char *,              /* text string */
    void *               /* data the application registered */
);

Widget CreateTextItem(Widget parent, int len, char *s);
void AddTextItemCB(Widget ti, TItem_CBProc cbproc, void *data);

#endif /* __WIDGETS_H_ */
