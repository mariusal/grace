/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 *
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 *
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2005 Grace Development Team
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

#ifndef __QTINC_H_
#define __QTINC_H_

#ifdef __cplusplus
#   include <QObject>
#   ifdef WIN32
        typedef union _XEvent { int type; } XEvent;
#   endif
#else
    typedef struct QObject QObject;
    typedef union _XEvent { int type; } XEvent;
#endif
typedef QObject *Widget;
typedef int WidgetList;
typedef int Boolean;
typedef int String;
typedef int Cardinal;
typedef int XmString;
typedef int XtPointer;
typedef int Cursor;

#endif /* __QTINC_H_ */

