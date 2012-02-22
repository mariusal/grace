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

/* GUI */

#include "gui.h"

#include "events.h"

#include "globals.h"

/* Text */
TextStructure *CreateText(Widget parent, char *s)
{
    return CreateText2(parent, s, 0);
}

TextStructure *CreateText2(Widget parent, char *s, int len)
{
    TextStructure *retval;

    retval = xmalloc(sizeof(TextStructure));
    retval->form = CreateForm(parent);

    retval->label = CreateLabel(retval->form, s);
    FormAddHChild(retval->form, retval->label);

    retval->text = CreateLineTextEdit(retval->form, len);
    FormAddHChild(retval->form, retval->text);

    retval->multiline = FALSE;

    return retval;
}

/*
 * create a multiline editable window
 * parent = parent widget
 * nrows  = number of lines in the window
 * s      = label for window
 */
TextStructure *CreateScrolledText(Widget parent, char *s, int nrows)
{
    TextStructure *retval;

    retval = xmalloc(sizeof(TextStructure));
    retval->form = CreateForm(parent);

    retval->label = CreateLabel(retval->form, s);
    FormAddVChild(retval->form, retval->label);

    retval->text = CreateMultiLineTextEdit(retval->form, nrows);
    FormAddVChild(retval->form, WidgetGetParent(retval->text));

    retval->multiline = TRUE;

    return retval;
}


static void cstext_edit_action(KeyEvent *event)
{
    TextStructure *cst = (TextStructure *) event->anydata;
    create_fonttool(cst);
}

TextStructure *CreateCSText(Widget parent, char *s)
{
    TextStructure *retval;

    retval = CreateText(parent, s);
    AddWidgetKeyPressCB2(retval->text, CONTROL_MODIFIER, KEY_E, cstext_edit_action, retval);

    return retval;
}

TextStructure *CreateScrolledCSText(Widget parent, char *s, int nrows)
{
    TextStructure *retval;

    retval = CreateScrolledText(parent, s, nrows);
    AddWidgetKeyPressCB2(retval->text, CONTROL_MODIFIER, KEY_E, cstext_edit_action, retval);

    return retval;
}

/*
 * xv_evalexpr - take a text field and pass it to the parser to evaluate
 */
int xv_evalexpr(TextStructure *cst, double *answer)
{
    int retval;
    char *s;

    s = TextGetString(cst);

    retval = graal_eval_expr(grace_get_graal(gapp->grace),
        s, answer, gproject_get_top(gapp->gp));

    xfree(s);

    return retval;
}

/*
 * xv_evalexpri - as xv_evalexpr, but for integers
 */
int xv_evalexpri(TextStructure *cst, int *answer)
{
    int retval;
    double buf;

    retval = xv_evalexpr(cst, &buf);

    *answer = rint(buf);

    return retval;
}

typedef struct {
    TextStructure *cst;
    Text_CBProc cbproc;
    void *anydata;
} Text_CBdata;

static void text_int_cb_proc(KeyEvent *event)
{
    char *s;
    Text_CBdata *cbdata = (Text_CBdata *) event->anydata;

    s = TextGetString(cbdata->cst);
    cbdata->cbproc(cbdata->cst, s, cbdata->anydata);
    xfree(s);
}

void AddTextActivateCB(TextStructure *cst, Text_CBProc cbproc, void *data)
{
    Text_CBdata *cbdata;

    cbdata = xmalloc(sizeof(Text_CBdata));
    cbdata->cst = cst;
    cbdata->anydata = data;
    cbdata->cbproc = cbproc;

    if (cst->multiline) {
        AddWidgetKeyPressCB2(cst->text, CONTROL_MODIFIER, KEY_RETURN, text_int_cb_proc, cbdata);
    } else {
        AddWidgetKeyPressCB(cst->text, KEY_RETURN, text_int_cb_proc, cbdata);
    }
}

