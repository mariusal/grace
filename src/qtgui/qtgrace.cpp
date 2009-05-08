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

#include <QtGlobal>
#include <QApplication>
#include <QMainWindow>
#include <QToolBar>
#include <QGridLayout>
#include <QScrollArea>
#include <QStatusBar>
#include <QMenuBar>
#include <QMenu>
#include <QLabel>
#include <QBitmap>
#include <QAbstractButton>
#include <QMessageBox>
#include <QDialog>
#include <QGroupBox>
#include <QCheckBox>
#include <QPushButton>
#include <QSignalMapper>
#include <QComboBox>
#include <QPainter>
#include <QStandardItemModel>
#include <mainwindow.h>
#include <canvaswidget.h>
#include <qtinc.h>

extern "C" {
  #include <globals.h>
  #include "xprotos.h"
  #include <bitmaps.h>
  Widget app_shell;
}

#include <qtgrace.h>

static MainWindow *mainWin;
static CanvasWidget *canvasWidget;
static QApplication *app;
static ButtonCallback *buttonCallback = new ButtonCallback();
static QSignalMapper *signalMapper = new QSignalMapper(mainWin);


void set_wait_cursor()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
}

void unset_wait_cursor()
{
    QApplication::restoreOverrideCursor();
}

/*
 * build the GUI
 */
void startup_gui(GraceApp *gapp)
{
    qDebug("startup_gui start");

    MainWinUI *mwui = gapp->gui->mwui;

    mwui->loclab = mainWin->ui.locatorBar;

    mwui->statlab = CreateLabel(mainWin, NULL);
    mainWin->ui.statusBar->addWidget(mwui->statlab);

    //toolBar->setStyleSheet(QString::fromUtf8(" QToolButton {\n"
    //            "    border: 1px solid #8f8f91;\n"
    //            "    border-radius: 2px;\n"
    //            "    background-color: #b0c4de;\n"
    //            " }\n"
    //            "\n"
    //            " QToolButton:pressed {\n"
    //            "    background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,\n"
    //            "                                                           stop: 0 #dadbde, stop: 1 #f6f7fa);\n"
    //            " }"));

    CreateMainMenuBar(mainWin);
    CreateToolBar(mainWin->ui.toolBar);
/*
 * initialize some option menus
 */
//    init_option_menus();

/*
 * initialize the tool bars
 */
//    set_view_items();

    set_tracker_string(NULL);
    set_left_footer(NULL);

/*
 * set icon
 */
    mainWin->setWindowIcon(QPixmap(gapp_icon_xpm));

    gapp->gui->inwin = TRUE;

/*
 * set the title
 */
    update_app_title(gapp->gp);

    xdrawgraph(gapp->gp);

    QObject::connect(signalMapper, SIGNAL(mapped(QObject*)),
                     buttonCallback, SLOT(buttonClicked(QObject*))); 
    //XtAppMainLoop(app_con);
    mainWin->show();
    qDebug("startup_gui end");
    app->exec();
    exit(0);
}

int initialize_gui(int *argc, char **argv)
{
    qDebug("initialize_gui start");
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
    app = new QApplication(*argc, argv);
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
    x11_init(gapp);
//
//    /* initialize cursors */
//    init_cursors(gapp->gui);

    qDebug("initialize_gui end");
    return RETURN_SUCCESS;
}

int x11_init(GraceApp *gapp)
{
    X11Stuff *xstuff = gapp->gui->xstuff;
//    XGCValues gc_val;
//    long mrsize;
//    int max_path_limit;
//    
//    xstuff->screennumber = DefaultScreen(xstuff->disp);
//    xstuff->root = RootWindow(xstuff->disp, xstuff->screennumber);
// 
//    xstuff->gc = DefaultGC(xstuff->disp, xstuff->screennumber);
//    
//    xstuff->depth = DisplayPlanes(xstuff->disp, xstuff->screennumber);
//
//    /* init colormap */
//    xstuff->cmap = DefaultColormap(xstuff->disp, xstuff->screennumber);
//    /* redefine colormap, if needed */
//    if (gapp->gui->install_cmap == CMAP_INSTALL_ALWAYS) {
//        xstuff->cmap = XCopyColormapAndFree(xstuff->disp, xstuff->cmap);
//        gapp->gui->private_cmap = TRUE;
//    }
//    
//    /* set GCs */
//    if (gapp->gui->invert) {
//        gc_val.function = GXinvert;
//    } else {
//        gc_val.function = GXxor;
//    }
//    gcxor = XCreateGC(xstuff->disp, xstuff->root, GCFunction, &gc_val);
//
//    /* XExtendedMaxRequestSize() appeared in X11R6 */
//#if XlibSpecificationRelease > 5
//    mrsize = XExtendedMaxRequestSize(xstuff->disp);
//#else
//    mrsize = 0;
//#endif
//    if (mrsize <= 0) {
//        mrsize = XMaxRequestSize(xstuff->disp);
//    }
//    max_path_limit = (mrsize - 3)/2;
//    if (max_path_limit < get_max_path_limit(grace_get_canvas(gapp->grace))) {
//        char buf[128];
//        sprintf(buf,
//            "Setting max drawing path length to %d (limited by the X server)",
//            max_path_limit);
//        errmsg(buf);
//        set_max_path_limit(grace_get_canvas(gapp->grace), max_path_limit);
//    }
//   
//    xstuff->dpi = rint(MM_PER_INCH*DisplayWidth(xstuff->disp, xstuff->screennumber)/
//        DisplayWidthMM(xstuff->disp, xstuff->screennumber));
    mainWin = new MainWindow();
    canvasWidget = mainWin->ui.canvasWidget;
    xstuff->dpi = canvasWidget->physicalDpiX();

    return RETURN_SUCCESS;
}

// FOR main.c
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

int attach_pdf_drv_setup(Canvas *canvas, int device_id)
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
    QMessageBox msgBox(mainWin);
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
    QDialog *w = new QDialog(mainWin);
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
//    QDialog *d = (QDialog* ) w;
//    XtManageChild(w);
//    XMapRaised(XtDisplay(w), XtWindow(w));
    w->show();
}

Widget CreateFrame(Widget parent, char *s)
{
    //Widget fr;

//    fr = XtVaCreateManagedWidget("frame", xmFrameWidgetClass, parent, NULL);
    QGroupBox *groupBox = new QGroupBox(parent);
    
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
    QLayout *layout = form->layout();

    if (layout == 0) {
        layout = new QVBoxLayout();
        form->setLayout(layout);
    }
    layout->addWidget(child);

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
    QLayout *layout = parent->layout();

    if (layout == 0) {
        layout = new QVBoxLayout();
        parent->setLayout(layout);
    }
    //layout->addWidget(child);
    //rc = XmCreateRowColumn(parent, "VContainer", NULL, 0);
    //XtManageChild(rc);

    return parent;
}

Widget CreateToggleButton(Widget parent, char *s)
{
//    return (XtVaCreateManagedWidget(s, xmToggleButtonWidgetClass, parent, NULL));
    QCheckBox *cb = new QCheckBox(s, parent);

    QLayout *layout = parent->layout();
    if (layout != 0) {
        layout->addWidget(cb);
    }

    return cb;
}


OptionStructure *CreateOptionChoice(Widget parent, char *labelstr,
    int ncols, int nchoices, OptionItem *items)
{
    OptionStructure *retval;

    retval = (OptionStructure*) xcalloc(1, sizeof(OptionStructure));
    if (!retval) {
        return NULL;
    }

    QComboBox *comboBox = new QComboBox();
    retval->pulldown = comboBox;

    for (int i = 0; i < nchoices; i++) {
        comboBox->addItem(items[i].label);
    }

    QLabel *label = new QLabel();
    label->setText(labelstr);

    QWidget *widget = new QWidget();

    QHBoxLayout *horizontalLayout = new QHBoxLayout(widget);
    horizontalLayout->addWidget(label);
    horizontalLayout->addWidget(retval->pulldown);

    QLayout *layout = parent->layout();
    if (layout != 0) {
        layout->addWidget(widget);
    }

    return retval;
}

OptionStructure *CreateOptionChoiceVA(Widget parent, char *labelstr, ...)
{
    OptionStructure *retval;
    int nchoices = 0;
    OptionItem *oi = NULL;
    va_list var;
    char *s;
    int value;

    va_start(var, labelstr);
    while ((s = va_arg(var, char *)) != NULL) {
        value = va_arg(var, int);
        nchoices++;
        oi = (OptionItem*) xrealloc(oi, nchoices*sizeof(OptionItem));
        oi[nchoices - 1].value = value;
        oi[nchoices - 1].label = copy_string(NULL, s);
    }
    va_end(var);

    retval = CreateOptionChoice(parent, labelstr, 1, nchoices, oi);

    while (nchoices) {
        nchoices--;
        xfree(oi[nchoices].label);
    }
    xfree(oi);

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

Widget CreateCommandButtons(Widget parent, int n, Widget * buts, char **l)
{
    QWidget *frame = (QWidget *) parent;
    int i;
//    Widget form;
//    Dimension h;
//
    QHBoxLayout *horizontalLayout;
    QPushButton *pushButton;

    horizontalLayout = new QHBoxLayout(frame);

//    form = XtVaCreateWidget("form", xmFormWidgetClass, parent,
//                            XmNfractionBase, n,
//                            NULL);
//
    for (i = 0; i < n; i++) {
        pushButton = new QPushButton(l[i], frame);
        horizontalLayout->addWidget(pushButton);
        buts[i] = pushButton;
//        buts[i] = XtVaCreateManagedWidget(l[i],
//                                          xmPushButtonWidgetClass, form,
//                                          XmNtopAttachment, XmATTACH_FORM,
//                                          XmNbottomAttachment, XmATTACH_FORM,
//                                          XmNleftAttachment, XmATTACH_POSITION,
//                                          XmNleftPosition, i,
//                                          XmNrightAttachment, XmATTACH_POSITION,
//                                          XmNrightPosition, i + 1,
//                                          XmNdefaultButtonShadowThickness, 1,
//                                          XmNshowAsDefault, (i == 0) ? True : False,
//                                          NULL);
    }
//    XtManageChild(form);
//    XtVaGetValues(buts[0], XmNheight, &h, NULL);
//    XtVaSetValues(form, XmNpaneMaximum, h, XmNpaneMinimum, h, NULL);
//
//    return form;
}

void AddButtonCB(Widget button, Button_CBProc cbproc, void *data)
{
    Button_CBdata *cbdata;

    cbdata = (Button_CBdata*) xmalloc(sizeof(Button_CBdata));
    cbdata->but = button;
    cbdata->anydata = data;
    cbdata->cbproc = cbproc;

    Callback_Data *callbackData = new Callback_Data();
    callbackData->cbdata = cbdata;
    signalMapper->setMapping(button, callbackData);

    // TODO: remove this HACK, one connect for QPushButton other for QAction
    QObject::connect(button, SIGNAL(clicked()), signalMapper, SLOT (map()));
    QObject::connect(button, SIGNAL(triggered()), signalMapper, SLOT (map()));
    
//    XtAddCallback(button,
//        XmNactivateCallback, button_int_cb_proc, (XtPointer) cbdata);
}

typedef struct {
    Widget form;
    int close;
    AACDialog_CBProc cbproc;
    void *anydata;
} AACDialog_CBdata;

void aacdialog_int_cb_proc(Widget but, void *data)
{
    AACDialog_CBdata *cbdata;
    int retval;

    set_wait_cursor();

    cbdata = (AACDialog_CBdata *) data;

    retval = cbdata->cbproc(cbdata->anydata);

    if (cbdata->close && retval == RETURN_SUCCESS) {
//        XtUnmanageChild(XtParent(cbdata->form));
    }

    unset_wait_cursor();
}

WidgetList CreateAACDialog(Widget form,
    Widget container, AACDialog_CBProc cbproc, void *data)
{
    
    //Widget fr;
    QFrame *fr;
    WidgetList aacbut;
    AACDialog_CBdata *cbdata_accept, *cbdata_apply;
    char *aaclab[3] = {"Apply", "Accept", "Close"};

    //aacbut = (WidgetList) XtMalloc(3*sizeof(Widget));
    aacbut = (WidgetList) xmalloc(3*sizeof(Widget));

//    fr = CreateFrame(form, NULL);
//    XtVaSetValues(fr,
//        XmNtopAttachment, XmATTACH_NONE,
//        XmNleftAttachment, XmATTACH_FORM,
//        XmNrightAttachment, XmATTACH_FORM,
//        XmNbottomAttachment, XmATTACH_FORM,
//        NULL);
    fr = new QFrame(form);
    fr->setFrameShape(QFrame::StyledPanel);
    fr->setFrameShadow(QFrame::Raised);
    CreateCommandButtons(fr, 3, aacbut, aaclab);

    AddDialogFormChild(form, container);
    AddDialogFormChild(form, fr);
//    XtVaSetValues(container,
//        XmNbottomAttachment, XmATTACH_WIDGET,
//        XmNbottomWidget, fr,
//        NULL);
//
//    XtVaSetValues(form, XmNcancelButton, aacbut[2], NULL);
//
    cbdata_accept = (AACDialog_CBdata*) xmalloc(sizeof(AACDialog_CBdata));
    cbdata_accept->form    = form;
    cbdata_accept->anydata = data;
    cbdata_accept->cbproc  = cbproc;
    cbdata_accept->close   = TRUE;

    cbdata_apply  = (AACDialog_CBdata*) xmalloc(sizeof(AACDialog_CBdata));
    cbdata_apply->form     = form;
    cbdata_apply->anydata  = data;
    cbdata_apply->cbproc   = cbproc;
    cbdata_apply->close    = FALSE;

    AddButtonCB(aacbut[0], aacdialog_int_cb_proc, cbdata_apply);
    AddButtonCB(aacbut[1], aacdialog_int_cb_proc, cbdata_accept);
//    AddButtonCB(aacbut[2], destroy_dialog_cb, XtParent(form));
    QObject::connect(aacbut[2], SIGNAL(clicked()), form, SLOT(close()));
//
//    XtManageChild(container);
//    XtManageChild(form);
//
    return aacbut;
}

void SetToggleButtonState(Widget w, int value)
{
    if (w == NULL) {
        return;
    }
    QCheckBox *cb = (QCheckBox*)(w);
    cb->setChecked(value ? true : false);

    return;
}

void SetOptionChoice(OptionStructure *opt, int value)
{
    QComboBox *comboBox = (QComboBox*) opt->pulldown;
    comboBox->setCurrentIndex(value);
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
	QCheckBox *cb = (QCheckBox*)(w);
	return cb->isChecked();
//        return XmToggleButtonGetState(w);
    }
}

int GetOptionChoice(OptionStructure *opt)
{
    QComboBox *comboBox = (QComboBox*) opt->pulldown;
    return comboBox->currentIndex();
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

void set_title(char *title, char *icon_name)
{
    mainWin->setWindowTitle(title);
}

/*
 *  Auxiliary routines for simultaneous drawing on display and pixmap
 */
void aux_XDrawLine(GUI *gui, int x1, int y1, int x2, int y2)
{
//    X11Stuff *xstuff = gui->xstuff;
//    XDrawLine(xstuff->disp, xstuff->xwin, gcxor, x1, y1, x2, y2);
//    if (xstuff->bufpixmap != (Pixmap) NULL) {
//        XDrawLine(xstuff->disp, xstuff->bufpixmap, gcxor, x1, y1, x2, y2);
//    }
}

void aux_XDrawRectangle(GUI *gui, int x1, int y1, int x2, int y2)
{
    QImage *pixmap = (QImage*) gui->xstuff->bufpixmap;

    QPainter painter(pixmap);
    painter.setPen(QPen(Qt::white));
    painter.setBrush(QBrush(Qt::NoBrush));
    painter.setCompositionMode(QPainter::CompositionMode_Exclusion);
    painter.drawRect(QRectF(x1, y1, x2, y2));
    canvasWidget->repaint(); // TODO: repaint just drawn rectangle
//    XDrawRectangle(xstuff->disp, xstuff->xwin, gcxor, x1, y1, x2, y2);
//    if (xstuff->bufpixmap != (Pixmap) NULL) {
//        XDrawRectangle(xstuff->disp, xstuff->bufpixmap, gcxor, x1, y1, x2, y2);
//    }
}

void aux_XFillRectangle(GUI *gui, int x, int y, unsigned int width, unsigned int height)
{
    QImage *pixmap = (QImage*) gui->xstuff->bufpixmap;

    QPainter painter(pixmap);
    painter.setPen(QPen(Qt::NoPen));
    painter.setBrush(QBrush(Qt::white));
    painter.setCompositionMode(QPainter::CompositionMode_Exclusion);
    painter.drawRect(QRectF(x, y, width, height));
//    XFillRectangle(xstuff->disp, xstuff->xwin, gcxor, x, y, width, height);
//    if (xstuff->bufpixmap != (Pixmap) NULL) {
//        XFillRectangle(xstuff->disp, xstuff->bufpixmap, gcxor, x, y, width, height);
//    }
}

void SetDimensions(Widget w, unsigned int width, unsigned int height)
{
    canvasWidget->setMinimumSize(width, height);
//    XtVaSetValues(w,
//        XmNwidth, (Dimension) width,
//        XmNheight, (Dimension) height,
//        NULL);
}

void GetDimensions(Widget w, unsigned int *width, unsigned int *height)
{
    unsigned int ww, wh;

    ww = canvasWidget->width();
    wh = canvasWidget->height();

    *width  = (unsigned int) ww;
    *height = (unsigned int) wh;

//    Dimension ww, wh;
//
//    XtVaGetValues(w,
//        XmNwidth, &ww,
//        XmNheight, &wh,
//        NULL);
//
//    *width  = (unsigned int) ww;
//    *height = (unsigned int) wh;
}

void init_xstream(X11stream *xstream)
{
    X11Stuff *xstuff = gapp->gui->xstuff;

//    xstream->screen = DefaultScreenOfDisplay(xstuff->disp);
    xstream->pixmap = xstuff->bufpixmap;
}

void create_pixmap(unsigned int w, unsigned int h)
{
    qDebug("create_pixmap");
    X11Stuff *xstuff = gapp->gui->xstuff;
//
//    xstuff->bufpixmap = XCreatePixmap(xstuff->disp, xstuff->root, w, h, xstuff->depth);
    /* 8 bits per color channel (i.e., 256^3 colors) */
    /* are defined in CANVAS_BPCC canvas.h file. */
    /* Use alpha channel to be able to use QPainter::setCompositionMode(CompositionMode mode)*/
    /* Image composition using alpha blending are faster using premultiplied ARGB32 than with plain ARGB32 */
    xstuff->bufpixmap = new QImage(w, h, QImage::Format_ARGB32_Premultiplied);
}

void recreate_pixmap(unsigned int w, unsigned int h)
{
    qDebug("recreate_pixmap");
    X11Stuff *xstuff = gapp->gui->xstuff;
//
//    XFreePixmap(xstuff->disp, xstuff->bufpixmap);
    delete (QImage*)xstuff->bufpixmap;
    create_pixmap(w, h);
}

void xdrawgrid(X11Stuff *xstuff)
{
    int i, j;
    double step;
    double x, y;

    double w = canvasWidget->width();
    double h = canvasWidget->height();

    QImage *pixmap = (QImage*) xstuff->bufpixmap;

    QPainter painter(pixmap);

    QPen pen;
    pen.setColor(Qt::black);
    pen.setWidth(1);
    pen.setStyle(Qt::SolidLine);
    pen.setCapStyle(Qt::FlatCap);
    pen.setJoinStyle(Qt::MiterJoin);
    painter.setPen(pen);

    QBrush brush;
    brush.setColor(Qt::white);
    painter.setBrush(brush);

    painter.drawRect(canvasWidget->rect());

    step = MIN2(w, h)/10;
    for (i = 0; i < w/step; i++) {
        for (j = 0; j < h/step; j++) {
            x = i*step;
            y = h - j*step;
            painter.drawPoint(QPointF(x, y));
        }
    }
}

void x11_redraw_all()
{
    X11Stuff *xstuff = gapp->gui->xstuff;

    canvasWidget->pixmap = (QImage*) xstuff->bufpixmap;
    canvasWidget->update();
//    X11Stuff *xstuff = gapp->gui->xstuff;
//    if (gapp->gui->inwin == TRUE && xstuff->bufpixmap != (Pixmap) NULL) {
//        XCopyArea(xstuff->disp, xstuff->bufpixmap, window, xstuff->gc, x, y, width, height, x, y);
//    }
}

void move_pointer(short x, short y)
{
 //   X11Stuff *xstuff = gapp->gui->xstuff;

//    XWarpPointer(xstuff->disp, None, xstuff->xwin, 0, 0, 0, 0, x, y);
}

Widget CreateLabel(Widget parent, char *s)
{
    qDebug("CreateLabel");
    QWidget *p = (QWidget*) parent;
    Widget label;
    
    label = new QLabel(s ? s:"", p);

    return label;
}

void SetLabel(Widget w, char *s)
{
    //qDebug("SetLabel %s", s);
    QLabel *label = (QLabel*) w;

    label->setText(s);
}

char *display_name(GUI *gui)
{
    //return DisplayString(gui->xstuff->disp);
    return "0:0";
}

// TODO: remove this function, use global one
void snapshot_and_update(GProject *gp, int all)
{
    Quark *pr = gproject_get_top(gp);
    //GUI *gui = gui_from_quark(pr);
    //AMem *amem;

    if (!pr) {
        return;
    }

    //amem = quark_get_amem(pr);
    //amem_snapshot(amem);

    xdrawgraph(gp);

    if (all) {
        update_all();
    } else {
        //update_undo_buttons(gp);
        //update_explorer(gui->eui, FALSE);
        update_app_title(gp);
    }
}

// TODO: remove this function, use global one
void update_all(void)
{
    if (!gapp->gui->inwin) {
        return;
    }

    if (gui_is_page_free(gapp->gui)) {
        sync_canvas_size(gapp);
    }

    //update_ssd_selectors(gproject_get_top(gapp->gp));
    //update_frame_selectors(gproject_get_top(gapp->gp));
    //update_graph_selectors(gproject_get_top(gapp->gp));
    //update_set_selectors(NULL);

    if (gapp->gui->need_colorsel_update == TRUE) {
        //init_xvlibcolors();
        //update_color_selectors();
        gapp->gui->need_colorsel_update = FALSE;
    }

    if (gapp->gui->need_fontsel_update == TRUE) {
        //update_font_selectors();
        gapp->gui->need_fontsel_update = FALSE;
    }

    //update_undo_buttons(gapp->gp);
    //update_props_items();
    //update_explorer(gapp->gui->eui, TRUE);
    set_left_footer(NULL);
    update_app_title(gapp->gp);
    canvasWidget->update();
}

void set_cursor(GUI *gui, int c)
{
    X11Stuff *xstuff = gui->xstuff;
//    if (xstuff->disp == NULL || xstuff->cur_cursor == c) {
    if (xstuff->cur_cursor == c) {
        return;
    }

//    XUndefineCursor(xstuff->disp, xstuff->xwin);
    xstuff->cur_cursor = c;
    switch (c) {
    case 0:
//        XDefineCursor(xstuff->disp, xstuff->xwin, xstuff->line_cursor);
        break;
    case 1:
//        XDefineCursor(xstuff->disp, xstuff->xwin, xstuff->find_cursor);
        break;
    case 2:
//        XDefineCursor(xstuff->disp, xstuff->xwin, xstuff->text_cursor);
        break;
    case 3:
//        XDefineCursor(xstuff->disp, xstuff->xwin, xstuff->kill_cursor);
        break;
    case 4:
//        XDefineCursor(xstuff->disp, xstuff->xwin, xstuff->move_cursor);
        break;
    case 5:
//        XDefineCursor(xstuff->disp, xstuff->xwin, xstuff->drag_cursor);
        break;
    default:
        xstuff->cur_cursor = -1;
        break;
    }
//    XFlush(xstuff->disp);
}

// TODO: remove this function, use global one
void graph_set_selectors(Quark *gr)
{
    int i;
    
//    for (i = 0; i < ngraph_selectors; i++) {
//        SelectStorageChoice(graph_selectors[i], gr);
//    }
}

// TODO: remove this function, use global one
void raise_explorer(GUI *gui, Quark *q)
{
}

Widget CreateMenuBar(Widget parent)
{
    QMenuBar *menuBar = new QMenuBar(parent);
    mainWin->setMenuBar(menuBar);
    return menuBar;
}

Widget CreateMenu(Widget parent, char *label, char mnemonic, int help)
{
    QMenuBar *menuBar = (QMenuBar*)parent;

    QMenu *menu = new QMenu(menuBar);
    menu->setTitle(label);

    menuBar->addAction(menu->menuAction());

    return menu;
}

Widget CreateMenuButton(Widget parent, char *label, char mnemonic,
	Button_CBProc cb, void *data)
{
    QMenu *menu = (QMenu*)parent;

    QAction *action = new QAction(parent);
    action->setText(label);

    AddButtonCB((QWidget*)action, cb, data);

    menu->addAction(action);

    return (QWidget*)action;
}

Widget CreateSeparator(Widget parent)
{
    QToolBar *toolBar = (QToolBar*) parent;

    QAction *action = toolBar->addSeparator();

    return (QWidget*)action;
}

Widget CreateMenuSeparator(Widget parent)
{
    QMenu *menu = (QMenu*)parent;

    QAction *action = menu->addSeparator();

    return (QWidget*)action;
}

void create_file_popup(Widget but, void *data)
{
}

void create_netcdfs_popup(Widget but, void *data)
{
}

void create_saveproject_popup(void)
{
}

void define_explorer_popup(Widget but, void *data)
{
}
void create_openproject_popup(void)
{
}

void create_write_popup(Widget but, void *data)
{
}


void create_printer_setup(Widget but, void *data)
{
}


void create_eval_frame(Widget but, void *data)
{
}

void create_load_frame(Widget but, void *data)
{
}

void create_histo_frame(Widget but, void *data)
{
}

void create_fourier_frame(Widget but, void *data)
{
}

void create_run_frame(Widget but, void *data)
{
}

void create_reg_frame(Widget but, void *data)
{
}

void create_diff_frame(Widget but, void *data)
{
}

void create_interp_frame(Widget but, void *data)
{
}

void create_int_frame(Widget but, void *data)
{
}

void create_xcor_frame(Widget but, void *data)
{
}

void create_samp_frame(Widget but, void *data)
{
}

void create_prune_frame(Widget but, void *data)
{
}

void create_lconv_frame(Widget but, void *data)
{
}

void create_leval_frame(Widget but, void *data)
{
}


void create_points_frame(Widget but, void *data)
{
}


void create_arrange_frame(Widget but, void *data)
{
}

void create_autos_frame(Widget but, void *data)
{
}

void create_nonl_frame(Widget but, void *data)
{
}


void create_fonttool(TextStructure *cstext_parent)
{
}

void create_fonttool_cb(Widget but, void *data)
{
}


void create_datasetprop_popup(Widget but, void *data)
{
}

void create_datasetop_popup(Widget but, void *data)
{
}


void create_featext_frame(Widget but, void *data)
{
}

void create_cumulative_frame(Widget but, void *data)
{
}

Widget CreateMenuToggle(Widget parent, char *label, char mnemonic,
	TB_CBProc cb, void *data)
{
}

void create_monitor_frame_cb(Widget but, void *data)
{
}

void update_all_cb(Widget but, void *data)
{
}

void HelpCB(Widget w, void *data)
{
}

//void create_props_frame(Widget but, void *data)
//{
//}

void create_about_grtool(Widget but, void *data)
{
}

Widget CreateBitmapButton(Widget parent,
    int width, int height, const unsigned char *bits)
{
    QToolBar *toolBar = (QToolBar*) parent;

    QAction *action = new QAction(parent);

    action->setIcon(QIcon(QBitmap::fromData(QSize(width, height),
                    bits,
                    QImage::Format_MonoLSB)));
    toolBar->addAction(action);

    return (QWidget*)action;
}

void set_view_items(void)
{
}


