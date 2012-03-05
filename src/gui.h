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

/* GUI independent functions */

#ifndef __GUI_H_
#define __GUI_H_

#include "widgets.h"

/* Text */
TextStructure *CreateText(Widget parent, char *s);
TextStructure *CreateText2(Widget parent, char *s, int len);
TextStructure *CreateScrolledText(Widget parent, char *s, int nrows);
TextStructure *CreateCSText(Widget parent, char *s);
TextStructure *CreateScrolledCSText(Widget parent, char *s, int nrows);
char *TextGetString(TextStructure *cst);
void TextSetString(TextStructure *cst, char *s);
int xv_evalexpr(TextStructure *cst, double *);
int xv_evalexpri(TextStructure *cst, int *);

typedef void (*Text_CBProc)(
    TextStructure *cst,
    char *,              /* text string */
    void *               /* data the application registered */
);
void AddTextActivateCB(TextStructure *cst, Text_CBProc cbproc, void *data);

typedef int (*TextValidate_CBProc)(
        char **text,
        void *data
);
void AddTextValidateCB(TextStructure *cst, TextValidate_CBProc cbproc, void *anydata);

/* Button */
void AddButtonCB(Widget w, Button_CBProc cbproc, void *data);

/* ToggleButton */
void AddToggleButtonCB(Widget w, TB_CBProc cbproc, void *anydata);

#endif /* __GUI_H_ */
