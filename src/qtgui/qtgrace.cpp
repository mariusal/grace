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

#include <QApplication>
#include <QAbstractButton>
#include <QMessageBox>
#include <QDialog>
#include <QGroupBox>
#include <QCheckBox>
#include <qtinc.h>

#include "mainwindow.h"

extern "C" {
  #include <globals.h>
  #include <bitmaps.h>
  #include "xprotos.h"
  Widget app_shell;
}

int main_cpp(int argc, char *argv[], GraceApp *gapp)
{
  QApplication app(argc, argv);
  MainWindow mainWin(gapp);
  app_shell = &mainWin;
  mainWin.show();
  return app.exec();
}

void startup_gui(GraceApp *gapp)
{
  char *ch[8] = {"qtgrace"};
  main_cpp(1, ch, gapp);
  exit(0);
}

int initialize_gui(int *argc, char **argv)
{
    X11Stuff *xstuff;
    MainWinUI *mwui;
//    Screen *screen;
//    ApplicationData rd;
//    String *allResources, *resolResources;
//    int lowres = FALSE;
//    unsigned int i, n_common, n_resol;
//    char *display_name = NULL;
//
    xstuff = (X11Stuff*) xmalloc(sizeof(X11Stuff));
    memset(xstuff, 0, sizeof(X11Stuff));
    gapp->gui->xstuff = xstuff;

    mwui = (MainWinUI*) xmalloc(sizeof(MainWinUI));
    memset(mwui, 0, sizeof(MainWinUI));
    gapp->gui->mwui = mwui;

//    
//    installXErrorHandler();
//    
//    /* Locale settings for GUI */
//    XtSetLanguageProc(NULL, NULL, NULL);
//    
//    XtToolkitInitialize();
//    app_con = XtCreateApplicationContext();
//    
//    /* Check if we're running in the low-resolution X */
//    for (i = 1; i < *argc - 1; i++) {
//        /* See if display name was specified in the args */
//        char *pattern = "-display";
//        if (strlen(argv[i]) > 1 && strstr(pattern, argv[i]) == pattern) {
//            display_name = argv[i + 1];
//        }
//    }
//    xstuff->disp = XOpenDisplay(display_name);
//    if (xstuff->disp == NULL) {
//	errmsg("Can't open display");
//        return RETURN_FAILURE;
//    }
//
//    screen = DefaultScreenOfDisplay(xstuff->disp);
//    if (HeightOfScreen(screen) < 740) {
//        lowres = TRUE;
//    }
//    
//    n_common = sizeof(fallbackResourcesCommon)/sizeof(String) - 1;
//    if (lowres) {
//        n_resol  = sizeof(fallbackResourcesLowRes)/sizeof(String) - 1;
//        resolResources = fallbackResourcesLowRes;
//    } else {
//        n_resol  = sizeof(fallbackResourcesHighRes)/sizeof(String) - 1;
//        resolResources = fallbackResourcesHighRes;
//    }
//    allResources = xmalloc((n_common + n_resol + 1)*sizeof(String));
//    for (i = 0; i < n_common; i++) {
//        allResources[i] = fallbackResourcesCommon[i];
//    }
//    for (i = 0; i < n_resol; i++) {
//        allResources[n_common + i] = resolResources[i];
//    }
//    allResources[n_common + n_resol] = NULL;
//    XtAppSetFallbackResources(app_con, allResources);
//    
//    XtDisplayInitialize(app_con, xstuff->disp, "xmgrace", "XmGrace", NULL, 0, argc, argv);
//
//    XtAppAddActions(app_con, dummy_actions, XtNumber(dummy_actions));
//    XtAppAddActions(app_con, canvas_actions, XtNumber(canvas_actions));
//    XtAppAddActions(app_con, list_select_actions, XtNumber(list_select_actions));
//    XtAppAddActions(app_con, cstext_actions, XtNumber(cstext_actions));
//
//    app_shell = XtAppCreateShell(NULL, "XmGrace", applicationShellWidgetClass,
//        xstuff->disp, NULL, 0);
//
//    if (is_motif_compatible() != TRUE) {
//        return RETURN_FAILURE;
//    }
//    
//    XtGetApplicationResources(app_shell, &rd,
//        resources, XtNumber(resources), NULL, 0);
//    
//    gapp->gui->invert = rd.invert;
//    gapp->gui->instant_update = rd.instantupdate;
//    gapp->gui->toolbar = rd.toolbar;
//    gapp->gui->statusbar = rd.statusbar;
//    gapp->gui->locbar = rd.locatorbar;
//
//    x11_init(gapp);
//
//    /* initialize cursors */
//    init_cursors(gapp->gui);

    return RETURN_SUCCESS;
}

int x11_get_pixelsize(const GUI *gui)
{
//    Screen *screen = DefaultScreenOfDisplay(gui->xstuff->disp);
//    int i, n;
//    XPixmapFormatValues *pmf;
//    int pixel_size = 0;

//    pmf = XListPixmapFormats(DisplayOfScreen(screen), &n);
//    if (pmf) {
//        for (i = 0; i < n; i++) {
//            if (pmf[i].depth == PlanesOfScreen(screen)) {
//                pixel_size = pmf[i].bits_per_pixel/8;
//                break;
//            }
//        }
//        XFree((char *) pmf);
//    }

//    return pixel_size;
    return 8;
}

// All these funtions are implemented in drv_ui.c

int attach_ps_drv_setup(Canvas *canvas, int device_id)
{
}

int attach_eps_drv_setup(Canvas *canvas, int device_id)
{
}

int attach_pnm_drv_setup(Canvas *canvas, int device_id)
{
}

int attach_jpg_drv_setup(Canvas *canvas, int device_id)
{
}

int attach_png_drv_setup(Canvas *canvas, int device_id)
{
}

// src/motifutils.c
void update_all(void)
{
}

// src/motifutils.c
int clean_graph_selectors(Quark *pr, int etype, void *data)
{
}

// src/motifutils.c
int clean_frame_selectors(Quark *pr, int etype, void *data)
{
}

// src/xutil.c
void xunregister_rti(const Input_buffer *ib)
{
}

// src/xutil.c
void xregister_rti(Input_buffer *ib)
{
}

// src/motifutils.c
void unlink_ssd_ui(Quark *q)
{
}

// src/monwin.c
void stufftextwin(char *msg)
{
}

// src/monwin.c
void errwin(const char *msg)
{
}

int yesnowin(char *msg, char *s1, char *s2, char *help_anchor)
{
    QMessageBox msgBox;
    msgBox.setWindowIcon(QPixmap(gapp_icon_xpm));
    msgBox.setWindowTitle("Grace: Warning");
    msgBox.setIcon(QMessageBox::Question);
    if (msg != NULL) {
        msgBox.setText(msg);
    } else {
        msgBox.setText("Warning");
    }

    if (help_anchor) {
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel | QMessageBox::Help);
    } else {    
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    }

    if (s1) {
        msgBox.button(QMessageBox::Ok)->setText(s1);
    }    

    if (s2) {
        msgBox.button(QMessageBox::Cancel)->setText(s2);
    }

    msgBox.setDefaultButton(QMessageBox::Cancel);
    msgBox.setEscapeButton(QMessageBox::Cancel);
    int yesno_retval = FALSE;
    int ret = msgBox.exec();
    switch (ret) {
        case QMessageBox::Ok:
            yesno_retval = TRUE;
            break;
        case QMessageBox::Cancel:
            yesno_retval = FALSE;
            break;
        case QMessageBox::Help:
            // HelpCB(help_anchor); // TODO: implement help viewer
            // help was clicked
            break;
        default:
            // should never be reached
            break;
    }

    return yesno_retval;
}

//static char *label_to_resname(const char *s, const char *suffix)
//{
//    char *retval, *rs;
//    int capitalize = FALSE;
//
//    retval = copy_string(NULL, s);
//    rs = retval;
//    while (*s) {
//        if (isalnum(*s)) {
//            if (capitalize == TRUE) {
//                *rs = toupper(*s);
//                capitalize = FALSE;
//            } else {
//                *rs = tolower(*s);
//            }
//            rs++;
//        } else {
//            capitalize = TRUE;
//        }
//        s++;
//    }
//    *rs = '\0';
//    if (suffix != NULL) {
//        retval = concat_strings(retval, suffix);
//    }
//    return retval;
//}

Widget CreateDialogForm(Widget parent, const char *s)
{
    //X11Stuff *xstuff = gapp->gui->xstuff;
    //QWidget dialog, w;
    char *bufp;
//    int standalone;

//    if (parent == NULL) {
//        standalone = TRUE;
//        parent = XtAppCreateShell("XMgapp", "XMgapp",
//            topLevelShellWidgetClass, xstuff->disp,
//            NULL, 0);
//    } else {
//        standalone = FALSE;
//    }
//    bufp = label_to_resname(s, "Dialog");
//    dialog = XmCreateDialogShell(parent, bufp, NULL, 0);
//    xfree(bufp);

//    if (standalone) {
//        RegisterEditRes(dialog);
//    }

//    handle_close(dialog);

    bufp = copy_string(NULL, "Grace: ");
    bufp = concat_strings(bufp, s);
//    XtVaSetValues(dialog,
//        XmNtitle, bufp,
//        NULL);
//    xfree(bufp);

//    w = XmCreateForm(dialog, "form", NULL, 0);
    QDialog *w = new QDialog();
    w->setWindowTitle(bufp);
    xfree(bufp);

    return w;
}

Widget GetParent(Widget w)
{
    if (w) {
        //return (XtParent(w));
        return w;
    } else {
        errmsg("Internal error: GetParent() called with NULL widget");
        return NULL;
    }
}

/*
 * Manage and raise
 */
void RaiseWindow(Widget w)
{
    QDialog *d = (QDialog* ) w;
//    XtManageChild(w);
//    XMapRaised(XtDisplay(w), XtWindow(w));
    d->show();
}

Widget CreateFrame(Widget parent, char *s)
{
    //Widget fr;

//    fr = XtVaCreateManagedWidget("frame", xmFrameWidgetClass, parent, NULL);
    QGroupBox *groupBox = new QGroupBox((QWidget*) parent);
    
    if (s != NULL) {
        groupBox->setTitle(s);
//        XtVaCreateManagedWidget(s, xmLabelGadgetClass, fr,
//                                XmNchildType, XmFRAME_TITLE_CHILD,
//                                NULL);
    }

    return groupBox;
}

void AddDialogFormChild(Widget form, Widget child)
{
    QWidget *f = (QWidget*) form;
    QWidget *c = (QWidget*) child;
    
    QLayout *layout = f->layout();

    if (layout == 0) {
        layout = new QVBoxLayout();
        f->setLayout(layout);
    }
    layout->addWidget(c);

//    Widget last_widget;
//
//    last_widget = GetUserData(form);
//    if (last_widget) {
//        XtVaSetValues(child,
//            XmNtopAttachment, XmATTACH_WIDGET,
//            XmNtopWidget, last_widget,
//            NULL);
//        XtVaSetValues(last_widget,
//            XmNbottomAttachment, XmATTACH_NONE,
//            NULL);
//    } else {
//        XtVaSetValues(child,
//            XmNtopAttachment, XmATTACH_FORM,
//            NULL);
//    }
//    XtVaSetValues(child,
//        XmNleftAttachment, XmATTACH_FORM,
//        XmNrightAttachment, XmATTACH_FORM,
//        XmNbottomAttachment, XmATTACH_FORM,
//        NULL);
//    SetUserData(form, child);
}

Widget CreateVContainer(Widget parent)
{
    Widget rc;

    rc = new QVBoxLayout((QWidget*)parent);
    //rc = XmCreateRowColumn(parent, "VContainer", NULL, 0);
    //XtManageChild(rc);

    return rc;
}

Widget CreateToggleButton(Widget parent, char *s)
{
//    return (XtVaCreateManagedWidget(s, xmToggleButtonWidgetClass, parent, NULL));
    QCheckBox *cb = new QCheckBox(s);
    QLayout* l = dynamic_cast<QLayout*>(parent);
    if (l) {
        l->addWidget(cb);
    }
    return cb;
}

OptionStructure *CreateOptionChoice(Widget parent, char *labelstr,
    int ncols, int nchoices, OptionItem *items)
{
//    Arg args[2];
//    XmString str;
    OptionStructure *retval;

    retval = (OptionStructure*) xcalloc(1, sizeof(OptionStructure));
    if (!retval) {
        return NULL;
    }
//
//    XtSetArg(args[0], XmNpacking, XmPACK_COLUMN);
//    retval->pulldown = XmCreatePulldownMenu(parent, "pulldownMenu", args, 1);
//
//    retval->ncols = ncols;
//
//    UpdateOptionChoice(retval, nchoices, items);
//
//    str = XmStringCreateLocalized(labelstr);
//    XtSetArg(args[0], XmNlabelString, str);
//    XtSetArg(args[1], XmNsubMenuId, retval->pulldown);
//
//    retval->menu = XmCreateOptionMenu(parent, "optionMenu", args, 2);
//
//    XmStringFree(str);
//
//    XtManageChild(retval->menu);

    return retval;
}

OptionStructure *CreateOptionChoiceVA(Widget parent, char *labelstr, ...)
{
    OptionStructure *retval;
    int nchoices = 0;
    OptionItem *oi = NULL;
//    va_list var;
//    char *s;
//    int value;
//
//    va_start(var, labelstr);
//    while ((s = va_arg(var, char *)) != NULL) {
//        value = va_arg(var, int);
//        nchoices++;
//        oi = xrealloc(oi, nchoices*sizeof(OptionItem));
//        oi[nchoices - 1].value = value;
//        oi[nchoices - 1].label = copy_string(NULL, s);
//    }
//    va_end(var);
//
    retval = CreateOptionChoice(parent, labelstr, 1, nchoices, oi);
//
//    while (nchoices) {
//        nchoices--;
//        xfree(oi[nchoices].label);
//    }
//    xfree(oi);
//
    return retval;
}

SpinStructure *CreateSpinChoice(Widget parent, char *s, int len,
                        int type, double min, double max, double incr)
{
    SpinStructure *retval;
//    Widget fr, form;
//    XmString str;
//
//    if (min >= max) {
//        errmsg("min >= max in CreateSpinChoice()!");
//        return NULL;
//    }
//
    retval = (SpinStructure*) xmalloc(sizeof(SpinStructure));
//
//    retval->type = type;
//    retval->min = min;
//    retval->max = max;
//    retval->incr = incr;
//
//    retval->rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, parent,
//        XmNorientation, XmHORIZONTAL,
//        NULL);
//    str = XmStringCreateLocalized(s);
//    XtVaCreateManagedWidget("label", xmLabelWidgetClass, retval->rc,
//        XmNlabelString, str,
//        NULL);
//    XmStringFree(str);
//    fr = XtVaCreateWidget("fr", xmFrameWidgetClass, retval->rc,
//        XmNshadowType, XmSHADOW_ETCHED_OUT,
//        NULL);
//    form = XtVaCreateWidget("form", xmFormWidgetClass, fr,
//        NULL);
//    retval->text = XtVaCreateWidget("text", xmTextWidgetClass, form,
//        XmNtraversalOn, True,
//        XmNcolumns, len,
//        NULL);
//
//    XtAddEventHandler(retval->text, ButtonPressMask, False, spin_updown, retval);
//
//    retval->arrow_up = XtVaCreateWidget("form", xmArrowButtonGadgetClass, form,
//        XmNarrowDirection, XmARROW_UP,
//        NULL);
//    XtAddCallback(retval->arrow_up, XmNactivateCallback,
//        spin_arrow_cb, (XtPointer) retval);
//    retval->arrow_down = XtVaCreateWidget("form", xmArrowButtonGadgetClass, form,
//        XmNarrowDirection, XmARROW_DOWN,
//        NULL);
//    XtAddCallback(retval->arrow_down, XmNactivateCallback,
//        spin_arrow_cb, (XtPointer) retval);
//    XtVaSetValues(retval->text,
//        XmNtopAttachment, XmATTACH_FORM,
//        XmNleftAttachment, XmATTACH_FORM,
//        XmNbottomAttachment, XmATTACH_FORM,
//        XmNrightAttachment, XmATTACH_NONE,
//        NULL);
//    XtVaSetValues(retval->arrow_down,
//        XmNtopAttachment, XmATTACH_FORM,
//        XmNbottomAttachment, XmATTACH_FORM,
//        XmNleftAttachment, XmATTACH_WIDGET,
//        XmNleftWidget, retval->text,
//        XmNrightAttachment, XmATTACH_NONE,
//        NULL);
//    XtVaSetValues(retval->arrow_up,
//        XmNtopAttachment, XmATTACH_FORM,
//        XmNbottomAttachment, XmATTACH_FORM,
//        XmNrightAttachment, XmATTACH_FORM,
//        XmNleftAttachment, XmATTACH_WIDGET,
//        XmNleftWidget, retval->arrow_down,
//        NULL);
//
//    XtManageChild(retval->text);
//    XtManageChild(retval->arrow_up);
//    XtManageChild(retval->arrow_down);
//    XtManageChild(form);
//    XtManageChild(fr);
//    XtManageChild(retval->rc);
//
    return retval;
}

Widget CreateScale(Widget parent, char *s, int min, int max, int delta)
{
//    Widget w;
//    XmString str;
//
//    str = XmStringCreateLocalized(s);
//
//    w = XtVaCreateManagedWidget("scroll",
//        xmScaleWidgetClass, parent,
//        XmNtitleString, str,
//        XmNminimum, min,
//        XmNmaximum, max,
//        XmNscaleMultiple, delta,
//        XmNvalue, 0,
//        XmNshowValue, True,
//        XmNprocessingDirection, XmMAX_ON_RIGHT,
//        XmNorientation, XmHORIZONTAL,
//#if XmVersion >= 2000
//        XmNsliderMark, XmROUND_MARK,
//#endif
//        NULL);
//
//    XmStringFree(str);
//
//    return w;
    return parent;
}

WidgetList CreateAACDialog(Widget form,
    Widget container, AACDialog_CBProc cbproc, void *data)
{
//    Widget fr;
//    WidgetList aacbut;
//    AACDialog_CBdata *cbdata_accept, *cbdata_apply;
//    char *aaclab[3] = {"Apply", "Accept", "Close"};
//
//    aacbut = (WidgetList) XtMalloc(3*sizeof(Widget));
//
//    fr = CreateFrame(form, NULL);
//    XtVaSetValues(fr,
//        XmNtopAttachment, XmATTACH_NONE,
//        XmNleftAttachment, XmATTACH_FORM,
//        XmNrightAttachment, XmATTACH_FORM,
//        XmNbottomAttachment, XmATTACH_FORM,
//        NULL);
//    CreateCommandButtons(fr, 3, aacbut, aaclab);
//
    AddDialogFormChild(form, container);
//    XtVaSetValues(container,
//        XmNbottomAttachment, XmATTACH_WIDGET,
//        XmNbottomWidget, fr,
//        NULL);
//
//    XtVaSetValues(form, XmNcancelButton, aacbut[2], NULL);
//
//    cbdata_accept = xmalloc(sizeof(AACDialog_CBdata));
//    cbdata_accept->form    = form;
//    cbdata_accept->anydata = data;
//    cbdata_accept->cbproc  = cbproc;
//    cbdata_accept->close   = TRUE;
//
//    cbdata_apply  = xmalloc(sizeof(AACDialog_CBdata));
//    cbdata_apply->form     = form;
//    cbdata_apply->anydata  = data;
//    cbdata_apply->cbproc   = cbproc;
//    cbdata_apply->close    = FALSE;
//
//    AddButtonCB(aacbut[0], aacdialog_int_cb_proc, cbdata_apply);
//    AddButtonCB(aacbut[1], aacdialog_int_cb_proc, cbdata_accept);
//    AddButtonCB(aacbut[2], destroy_dialog_cb, XtParent(form));
//
//    XtManageChild(container);
//    XtManageChild(form);
//
//    return aacbut;
    //return form;
}

void SetToggleButtonState(Widget w, int value)
{
    if (w == NULL) {
        return;
    }
//    XmToggleButtonSetState(w, value ? True:False, False);

    return;
}

void SetOptionChoice(OptionStructure *opt, int value)
{
    int i;
//   Arg a;

    if (opt->options == NULL || opt->nchoices <= 0) {
        return;
    }

    for (i = 0; i < opt->nchoices; i++) {
        if (opt->options[i].value == value) {
//            XtSetArg(a, XmNmenuHistory, opt->options[i].widget);
//            XtSetValues(opt->menu, &a, 1);
            return;
        }
    }

    errmsg("Value not found in SetOptionChoice()");
}

void SetSpinChoice(SpinStructure *spinp, double value)
{
//    X11Stuff *xstuff = gapp->gui->xstuff;
//    char buf[64];
//
//    if (value < spinp->min) {
//        XBell(xstuff->disp, 50);
//        value = spinp->min;
//    } else if (value > spinp->max) {
//        XBell(xstuff->disp, 50);
//        value = spinp->max;
//    }
//
//    if (spinp->type == SPIN_TYPE_FLOAT) {
//        sprintf(buf, "%g", value);
//    } else {
//        sprintf(buf, "%d", (int) rint(value));
//    }
//    XmTextSetString(spinp->text, buf);
}

void SetScaleValue(Widget w, int value)
{
//    XtVaSetValues(w, XmNvalue, value, NULL);
}

int GetToggleButtonState(Widget w)
{
    if (!w) {
        errmsg("Internal error: GetToggleButtonState() called with NULL widget");
        return 0;
    } else {
//        return XmToggleButtonGetState(w);
        return 0;
    }
}

int GetOptionChoice(OptionStructure *opt)
{
//    Arg a;
    Widget warg;
    int i;

    if (opt->options == NULL || opt->nchoices <= 0) {
        errmsg("Internal error in GetOptionChoice()");
        return 0;
    }

//    XtSetArg(a, XmNmenuHistory, &warg);
//    XtGetValues(opt->menu, &a, 1);

    for (i = 0; i < opt->nchoices; i++) {
        if (opt->options[i].widget == warg) {
            return(opt->options[i].value);
        }
    }
    errmsg("Internal error in GetOptionChoice()");
    return 0;
}

/*
 * xv_evalexpr - take a text field and pass it to the parser to evaluate
 */
int xv_evalexpr(Widget w, double *answer)
{
    int retval;
//    char *s;
//
//    s = XmTextGetString(w);
//
//    retval = graal_eval_expr(grace_get_graal(gapp->grace),
//        s, answer, gproject_get_top(gapp->gp));
//
//    XtFree(s);

    return retval;
}

double GetSpinChoice(SpinStructure *spinp)
{
    double retval;

    xv_evalexpr(spinp->text, &retval);
    if (retval < spinp->min) {
        errmsg("Input value below min limit in GetSpinChoice()");
        retval = spinp->min;
        SetSpinChoice(spinp, retval);
    } else if (retval > spinp->max) {
        errmsg("Input value above max limit in GetSpinChoice()");
        retval = spinp->max;
        SetSpinChoice(spinp, retval);
    }

    if (spinp->type == SPIN_TYPE_INT) {
        return rint(retval);
    } else {
        return retval;
    }
}

int GetScaleValue(Widget w)
{
    int value;
//    XtVaGetValues(w, XmNvalue, &value, NULL);
    return value;
}

/*
 * redraw all
 */
void xdrawgraph(const GProject *gp)
{
}
