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

/* TODO: */
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

    WidgetManage(retval->form);

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
    FormAddVChild(retval->form, retval->text);

    WidgetManage(retval->form);

    return retval;
}


static void cstext_edit_action(void *anydata)
{
    TextStructure *cst = (TextStructure *) anydata;
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

char *TextGetString(TextStructure *cst)
{
    return TextEditGetString(cst->text);
}

void TextSetString(TextStructure *cst, char *s)
{
    cst->locked = TRUE;
    TextEditSetString(cst->text, s);
    TextSetCursorPos(cst, s ? strlen(s):0);
    cst->locked = FALSE;
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

static void text_int_cb_proc(Widget_CBData *wcbdata)
{
    char *s;
    Text_CBdata *cbdata = (Text_CBdata *) wcbdata->anydata;

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

    AddWidgetCB(cst->text, "activate", text_int_cb_proc, cbdata);
}

typedef struct {
    TextStructure *cst;
    TextValidate_CBProc cbproc;
    void *anydata;
} TextValidate_CBData;

static void text_int_validate_cb_proc(Widget_CBData *wcbdata)
{
    TextValidate_CBData *cbdata = (TextValidate_CBData *) wcbdata->anydata;
    TextValidate_CD *cdata = (TextValidate_CD *) wcbdata->calldata;

    if (cbdata->cst->locked) return;

    if (!cbdata->cbproc(cdata->text, cbdata->anydata)) {
        cdata->allow_change = FALSE;
    }
}

void AddTextValidateCB(TextStructure *cst, TextValidate_CBProc cbproc, void *anydata)
{
    TextValidate_CBData *cbdata;

    cbdata = (TextValidate_CBData *) xmalloc(sizeof(TextValidate_CBData));
    cbdata->cst = cst;
    cbdata->cbproc = cbproc;
    cbdata->anydata = anydata;
    cst->locked = FALSE;

    AddWidgetCB(cst->text, "modifyVerify", text_int_validate_cb_proc, cbdata);
}

/* Button */
typedef struct {
    Widget w;
    Button_CBProc cbproc;
    void *anydata;
} Button_CBdata;

static void button_int_cb_proc(Widget_CBData *wcbdata)
{
    Button_CBdata *cbdata = (Button_CBdata *) wcbdata->anydata;

    cbdata->cbproc(cbdata->w, cbdata->anydata);
}

void AddButtonCB(Widget w, Button_CBProc cbproc, void *data)
{
    Button_CBdata *cbdata;

    cbdata = xmalloc(sizeof(Button_CBdata));
    cbdata->w = w;
    cbdata->anydata = data;
    cbdata->cbproc = cbproc;

    AddWidgetCB(w, "activate", button_int_cb_proc, cbdata);
}

/* ToggleButton */
typedef struct {
    Widget w;
    TB_CBProc cbproc;
    void *anydata;
} TB_CBdata;

static void tb_int_cb_proc(Widget_CBData *wcbdata)
{
    int onoff;

    TB_CBdata *cbdata = (TB_CBdata *) wcbdata->anydata;

    onoff = ToggleButtonGetState(cbdata->w);
    cbdata->cbproc(cbdata->w, onoff, cbdata->anydata);
}

void AddToggleButtonCB(Widget w, TB_CBProc cbproc, void *anydata)
{
    TB_CBdata *cbdata;

    cbdata = xmalloc(sizeof(TB_CBdata));
    cbdata->w = w;
    cbdata->cbproc = cbproc;
    cbdata->anydata = anydata;

    AddWidgetCB(w, "valueChanged", tb_int_cb_proc, cbdata);
}

/* SpinChoice */
static void spin_arrow_cb(Widget_CBData *wcbdata)
{
    SpinStructure *spinp;
    double value, incr;

    spinp = (SpinStructure *) wcbdata->anydata;
    value = SpinChoiceGetValue(spinp);
    incr = spinp->incr;

    if (wcbdata->w == spinp->arrow_up) {
        incr =  spinp->incr;
    } else if (wcbdata->w == spinp->arrow_down) {
        incr = -spinp->incr;
    } else {
        errmsg("Wrong call to spin_arrow_cb()");
        return;
    }
    value += incr;
    SpinChoiceSetValue(spinp, value);
    WidgetSetFocus(spinp->text);
}

static void spin_up(void *anydata)
{
    SpinStructure *spinp = (SpinStructure *) anydata;
    double value;

    value = SpinChoiceGetValue(spinp) + spinp->incr;
    SpinChoiceSetValue(spinp, value);
}

static void spin_down(void *anydata)
{
    SpinStructure *spinp = (SpinStructure *) anydata;
    double value;

    value = SpinChoiceGetValue(spinp) - spinp->incr;
    SpinChoiceSetValue(spinp, value);
}

SpinStructure *CreateSpinChoice(Widget parent, char *s, int len,
                        int type, double min, double max, double incr)
{
    SpinStructure *retval;
    Widget fr, form;

    if (min >= max) {
        errmsg("min >= max in CreateSpinChoice()!");
        return NULL;
    }

    retval = xmalloc(sizeof(SpinStructure));

    retval->type = type;
    retval->min = min;
    retval->max = max;
    retval->incr = incr;

    retval->rc = CreateHContainer(parent);

    CreateLabel(retval->rc, s);
    fr = CreateFrame(retval->rc, NULL);

    form = CreateForm(fr);

    retval->text = CreateLineTextEdit(form, len);
    FormAddHChild(form, retval->text);

    AddWidgetButtonPressCB(retval->text, WHEEL_UP_BUTTON, spin_up, retval);
    AddWidgetButtonPressCB(retval->text, WHEEL_DOWN_BUTTON, spin_down, retval);

    retval->arrow_down = CreateArrowButton(form, ARROW_DOWN);
    AddWidgetCB(retval->arrow_down, "activate", spin_arrow_cb, retval);
    FormAddHChild(form, retval->arrow_down);

    retval->arrow_up = CreateArrowButton(form, ARROW_UP);
    AddWidgetCB(retval->arrow_up, "activate", spin_arrow_cb, retval);
    FormAddHChild(form, retval->arrow_up);

    WidgetManage(form);

    return retval;
}

void SpinChoiceSetValue(SpinStructure *spinp, double value)
{
    char buf[64];

    if (value < spinp->min) {
        Beep();
        value = spinp->min;
    } else if (value > spinp->max) {
        Beep();
        value = spinp->max;
    }

    if (spinp->type == SPIN_TYPE_FLOAT) {
        sprintf(buf, "%g", value);
    } else {
        sprintf(buf, "%d", (int) rint(value));
    }
    TextEditSetString(spinp->text, buf);
}

double SpinChoiceGetValue(SpinStructure *spinp)
{
    double retval;
    char *s;

    s = TextEditGetString(spinp->text);

    graal_eval_expr(grace_get_graal(gapp->grace),
                    s, &retval,
                    gproject_get_top(gapp->gp));

    xfree(s);

    if (retval < spinp->min) {
        errmsg("Input value below min limit in GetSpinChoice()");
        retval = spinp->min;
        SpinChoiceSetValue(spinp, retval);
    } else if (retval > spinp->max) {
        errmsg("Input value above max limit in GetSpinChoice()");
        retval = spinp->max;
        SpinChoiceSetValue(spinp, retval);
    }

    if (spinp->type == SPIN_TYPE_INT) {
        return rint(retval);
    } else {
        return retval;
    }
}

typedef struct {
    SpinStructure *spin;
    Spin_CBProc cbproc;
    void *anydata;
    Timer_CBdata *tcbdata;
} Spin_CBdata;

static void sp_double_cb_proc(Widget_CBData *wcbdata)
{
    Spin_CBdata *cbdata = (Spin_CBdata *) wcbdata->anydata;

    cbdata->cbproc(cbdata->spin, SpinChoiceGetValue(cbdata->spin), cbdata->anydata);
}

static void sp_timer_proc(void *anydata)
{
    Spin_CBdata *cbdata = (Spin_CBdata *) anydata;

    cbdata->cbproc(cbdata->spin, SpinChoiceGetValue(cbdata->spin), cbdata->anydata);
}

static void sp_ev_proc(void *anydata)
{
    Spin_CBdata *cbdata = (Spin_CBdata *) anydata;

    TimerStart(cbdata->tcbdata);
}

void AddSpinChoiceCB(SpinStructure *spinp, Spin_CBProc cbproc, void *anydata)
{
    Spin_CBdata *cbdata;

    cbdata = xmalloc(sizeof(Spin_CBdata));

    cbdata->spin = spinp;
    cbdata->cbproc = cbproc;
    cbdata->anydata = anydata;
    cbdata->tcbdata = CreateTimer(250 /* 0.25 second */, sp_timer_proc, cbdata);

    AddWidgetCB(spinp->text, "activate", sp_double_cb_proc, cbdata);
    AddWidgetCB(spinp->arrow_up, "activate", sp_double_cb_proc, cbdata);
    AddWidgetCB(spinp->arrow_down, "activate", sp_double_cb_proc, cbdata);

    AddWidgetButtonPressCB(spinp->text, WHEEL_UP_BUTTON, sp_ev_proc, cbdata);
    AddWidgetButtonPressCB(spinp->text, WHEEL_DOWN_BUTTON, sp_ev_proc, cbdata);
}
