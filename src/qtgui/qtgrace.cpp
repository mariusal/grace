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
#include <QAbstractButton>
#include <QMessageBox>
#include <QDialog>
#include <QGroupBox>
#include <QCheckBox>
#include <QPushButton>
#include <QSignalMapper>
#include <QComboBox>
#include <qtinc.h>

#include "mainwindow.h"

extern "C" {
  #include <globals.h>
  #include "xprotos.h"
  #include <bitmaps.h>
  Widget app_shell;
}

#include <qtgrace.h>

MainWindow *mainWin;
QApplication *app;

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
//    MainWinUI *mwui = gapp->gui->mwui;
//    X11Stuff *xstuff = gapp->gui->xstuff;
//    Widget main_frame, form, menu_bar, bt, rcleft;
//    Pixmap icon, shape;
//
///* 
// * Allow users to change tear off menus with X resources
// */
//    XmRepTypeInstallTearOffModelConverter();
//    
//    RegisterEditRes(app_shell);
//
///*
// * We handle important WM events ourselves
// */
//    handle_close(app_shell);
//    
//    XtVaSetValues(app_shell, XmNcolormap, xstuff->cmap, NULL);
//    
///*
// * build the UI here
// */
//    main_frame = XtVaCreateManagedWidget("mainWin",
//        xmMainWindowWidgetClass, app_shell, NULL);
//
//    menu_bar = CreateMainMenuBar(main_frame);
//    ManageChild(menu_bar);
//
//    form = XmCreateForm(main_frame, "form", NULL, 0);
//
//    mwui->frleft = CreateFrame(form, NULL);
//    rcleft = XtVaCreateManagedWidget("toolBar", xmRowColumnWidgetClass,
//                                     mwui->frleft,
//				     XmNorientation, XmVERTICAL,
//				     XmNpacking, XmPACK_TIGHT,
//				     XmNspacing, 0,
//				     XmNentryBorder, 0,
//				     XmNmarginWidth, 0,
//				     XmNmarginHeight, 0,
//				     NULL);
//
//    mwui->frtop = CreateFrame(form, NULL);
//    mwui->loclab = CreateLabel(mwui->frtop, NULL);
//    
//    mwui->frbot = CreateFrame(form, NULL);
//    mwui->statlab = CreateLabel(mwui->frbot, NULL);
//
//    if (!gui_is_page_free(gapp->gui)) {
//        mwui->drawing_window = XtVaCreateManagedWidget("drawing_window",
//				     xmScrolledWindowWidgetClass, form,
//				     XmNscrollingPolicy, XmAUTOMATIC,
//				     XmNvisualPolicy, XmVARIABLE,
//				     NULL);
//        xstuff->canvas = XtVaCreateManagedWidget("canvas",
//                                     xmDrawingAreaWidgetClass,
//                                     mwui->drawing_window,
//				     NULL);
//    } else {
//        xstuff->canvas = XtVaCreateManagedWidget("canvas",
//                                     xmDrawingAreaWidgetClass, form,
//				     NULL);
//        mwui->drawing_window = xstuff->canvas;
//    }
//    
//    XtAddCallback(xstuff->canvas, XmNexposeCallback, expose_resize, gapp);
//    XtAddCallback(xstuff->canvas, XmNresizeCallback, expose_resize, gapp);
//
//    XtAddEventHandler(xstuff->canvas,
//                      ButtonPressMask
//                      | ButtonReleaseMask
//		      | PointerMotionMask
//		      | KeyPressMask
//		      | KeyReleaseMask
//		      | ColormapChangeMask,
//		      False,
//		      canvas_event_proc, gapp);
//		      
//    XtOverrideTranslations(xstuff->canvas, XtParseTranslationTable(canvas_table));
//    
//    AddHelpCB(xstuff->canvas, "doc/UsersGuide.html#canvas");
//
//    XtVaSetValues(mwui->frtop,
//		  XmNtopAttachment, XmATTACH_FORM,
//		  XmNleftAttachment, XmATTACH_FORM,
//		  XmNrightAttachment, XmATTACH_FORM,
//		  NULL);
//    XtVaSetValues(mwui->frbot,
//		  XmNbottomAttachment, XmATTACH_FORM,
//		  XmNrightAttachment, XmATTACH_FORM,
//		  XmNleftAttachment, XmATTACH_FORM,
//		  NULL);
//    XtVaSetValues(mwui->frleft,
//		  XmNtopAttachment, XmATTACH_WIDGET,
//		  XmNtopWidget, mwui->frtop,
//		  XmNbottomAttachment, XmATTACH_WIDGET,
//		  XmNbottomWidget, mwui->frbot,
//		  XmNleftAttachment, XmATTACH_FORM,
//		  NULL);
//    XtVaSetValues(mwui->drawing_window,
//		  XmNtopAttachment, XmATTACH_WIDGET,
//		  XmNtopWidget, mwui->frtop,
//		  XmNbottomAttachment, XmATTACH_WIDGET,
//		  XmNbottomWidget, mwui->frbot,
//		  XmNleftAttachment, XmATTACH_WIDGET,
//		  XmNleftWidget, mwui->frleft,
//		  XmNrightAttachment, XmATTACH_FORM,
//		  NULL);
//
//    ManageChild(form);
//
//    XmMainWindowSetAreas(main_frame, menu_bar, NULL, NULL, NULL, form);
//
//    /* redraw */
//    bt = CreateBitmapButton(rcleft, 16, 16, redraw_bits);
//    AddButtonCB(bt, do_drawgraph, NULL);
//    
//    CreateSeparator(rcleft);
//
//    /* zoom */
//    bt = CreateBitmapButton(rcleft, 16, 16, zoom_bits);
//    AddButtonCB(bt, set_zoom_cb, (void *) gapp);
//    bt = CreateBitmapButton(rcleft, 16, 16, zoom_x_bits);
//    AddButtonCB(bt, set_zoomx_cb, (void *) gapp);
//    bt = CreateBitmapButton(rcleft, 16, 16, zoom_y_bits);
//    AddButtonCB(bt, set_zoomy_cb, (void *) gapp);
//
//    CreateSeparator(rcleft);
//
//    /* autoscale */
//    bt = CreateBitmapButton(rcleft, 16, 16, auto_bits);
//    AddButtonCB(bt, autoscale_xy_cb, (void *) gapp);
//    bt = CreateBitmapButton(rcleft, 16, 16, auto_x_bits);
//    AddButtonCB(bt, autoscale_x_cb, (void *) gapp);
//    bt = CreateBitmapButton(rcleft, 16, 16, auto_y_bits);
//    AddButtonCB(bt, autoscale_y_cb, (void *) gapp);
//    bt = CreateBitmapButton(rcleft, 16, 16, auto_tick_bits);
//    AddButtonCB(bt, autoticks_cb, NULL);
//
//    CreateSeparator(rcleft);
//
//    /* scrolling buttons */
//    bt = CreateBitmapButton(rcleft, 16, 16, left_bits);
//    AddButtonCB(bt, graph_scroll_left_cb, (void *) gapp);
//    bt = CreateBitmapButton(rcleft, 16, 16, right_bits);
//    AddButtonCB(bt, graph_scroll_right_cb, (void *) gapp);
//    bt = CreateBitmapButton(rcleft, 16, 16, up_bits);
//    AddButtonCB(bt, graph_scroll_up_cb, (void *) gapp);
//    bt = CreateBitmapButton(rcleft, 16, 16, down_bits);
//    AddButtonCB(bt, graph_scroll_down_cb, (void *) gapp);
//
//    CreateSeparator(rcleft);
//
//    /* expand/shrink */
//    bt = CreateBitmapButton(rcleft, 16, 16, expand_bits);
//    AddButtonCB(bt, graph_zoom_in_cb, (void *) gapp);
//    bt = CreateBitmapButton(rcleft, 16, 16, shrink_bits);
//    AddButtonCB(bt, graph_zoom_out_cb, (void *) gapp);
//
//    CreateSeparator(rcleft);
//
//    bt = CreateBitmapButton(rcleft, 16, 16, atext_bits);
//    AddButtonCB(bt, atext_add_proc, (void *) gapp);
//
//    CreateSeparator(rcleft);
//    CreateSeparator(rcleft);
//
//    /* exit */
//    bt = CreateBitmapButton(rcleft, 16, 16, exit_bits);
//    AddButtonCB(bt, exit_cb, gapp);
//
/*
 * initialize some option menus
 */
//    init_option_menus();

/*
 * initialize the tool bars
 */
//    set_view_items();

    mainWin->canvasWidget->set_tracker_string(NULL);
    mainWin->set_left_footer(NULL);

/*
 * set icon
 */
    mainWin->setWindowIcon(QPixmap(gapp_icon_xpm));

//    XpmCreatePixmapFromData(xstuff->disp, xstuff->root,
//        gapp_icon_xpm, &icon, &shape, NULL);
//    XtVaSetValues(app_shell, XtNiconPixmap, icon, XtNiconMask, shape, NULL);

//    XtRealizeWidget(app_shell);
    
//    XmProcessTraversal(xstuff->canvas, XmTRAVERSE_CURRENT);
    
//    xstuff->xwin = XtWindow(xstuff->canvas);
    gapp->gui->inwin = TRUE;

/*
 * set the title
 */
    mainWin->update_app_title(gapp->gp);

    mainWin->canvasWidget->qtdrawgraph(gapp->gp);

    //XtAppMainLoop(app_con);
    ((QMainWindow*) app_shell)->show();
    app->exec();
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
    app = new QApplication(*argc, argv);
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
    mainWin = new MainWindow(gapp);
    app_shell = mainWin;
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
    QMessageBox msgBox((MainWindow*) app_shell);
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
    QDialog *w = new QDialog((MainWindow*) parent);
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

void UpdateOptionChoice(OptionStructure *optp, int nchoices, OptionItem *items)
{
//    int i, nold, ncols, nw;
//    Widget *wlist;
//    
//    nold = optp->nchoices;
//
//    if (optp->ncols == 0) {
//        ncols = 1;
//    } else {
//        ncols = optp->ncols;
//    }
//    
//    /* Don't create too tall pulldowns */
//    if (nchoices > MAX_PULLDOWN_LENGTH*ncols) {
//        ncols = (nchoices + MAX_PULLDOWN_LENGTH - 1)/MAX_PULLDOWN_LENGTH;
//    }
//    
//    XtVaSetValues(optp->pulldown, XmNnumColumns, ncols, NULL);
//
//    nw = nold - nchoices;
//    if (nw > 0) {
//        /* Unmanage extra items before destroying to speed the things up */
//        wlist = xmalloc(nw*sizeof(Widget));
//        for (i = nchoices; i < nold; i++) {
//            wlist[i - nchoices] = optp->options[i].widget;
//        }
//        XtUnmanageChildren(wlist, nw);
//        xfree(wlist);
//        
//        for (i = nchoices; i < nold; i++) {
//            XtDestroyWidget(optp->options[i].widget);
//        }
//    }
//
//    optp->options = xrealloc(optp->options, nchoices*sizeof(OptionWidgetItem));
//    optp->nchoices = nchoices;
//
//    for (i = nold; i < nchoices; i++) {
//        unsigned int j;
//        optp->options[i].widget = 
//                  XmCreatePushButton(optp->pulldown, "button", NULL, 0);
//        for (j = 0; j < optp->cbnum; j++) {
//            OC_CBdata *cbdata = optp->cblist[j];
//            XtAddCallback(optp->options[i].widget, XmNactivateCallback, 
//                                    oc_int_cb_proc, (XtPointer) cbdata);
//        }
//    }
//    
//    for (i = 0; i < nchoices; i++) {
//	optp->options[i].value = items[i].value;
//	if (items[i].label != NULL) {
//            XmString str, ostr;
//            XtVaGetValues(optp->options[i].widget, XmNlabelString, &ostr, NULL);
//            str = XmStringCreateLocalized(items[i].label);
//            if (XmStringCompare(str, ostr) != True) {
//                XtVaSetValues(optp->options[i].widget, XmNlabelString, str, NULL);
//            }
//            XmStringFree(str);
//        }
//    }
//    
//    nw = nchoices - nold;
//    if (nw > 0) {
//        wlist = xmalloc(nw*sizeof(Widget));
//        for (i = nold; i < nchoices; i++) {
//            wlist[i - nold] = optp->options[i].widget;
//        }
//        XtManageChildren(wlist, nw);
//        xfree(wlist);
//    }
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

//    XtSetArg(args[0], XmNpacking, XmPACK_COLUMN);
//    retval->pulldown = XmCreatePulldownMenu(parent, "pulldownMenu", args, 1);
    retval->ncols = ncols;

    UpdateOptionChoice(retval, nchoices, items);

    QLabel *label = new QLabel();
    label->setText(labelstr);

    QComboBox *comboBox = new QComboBox();

    QHBoxLayout *horizontalLayout = new QHBoxLayout();
    horizontalLayout->addWidget(label);
    horizontalLayout->addWidget(comboBox);

    QBoxLayout* l = dynamic_cast<QBoxLayout*>(parent);
    if (l) {
        l->addLayout(horizontalLayout);
    }

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

ButtonCallback *buttonCallback = new ButtonCallback();
QSignalMapper *signalMapper = new QSignalMapper(mainWin);

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
    QObject::connect(button, SIGNAL(clicked()), signalMapper, SLOT (map()));
    
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
    aacbut = (WidgetList) xmalloc(sizeof(Widget));

//    fr = CreateFrame(form, NULL);
//    XtVaSetValues(fr,
//        XmNtopAttachment, XmATTACH_NONE,
//        XmNleftAttachment, XmATTACH_FORM,
//        XmNrightAttachment, XmATTACH_FORM,
//        XmNbottomAttachment, XmATTACH_FORM,
//        NULL);
    fr = new QFrame((QWidget*) form);
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

    QObject::connect(signalMapper, SIGNAL(mapped(QObject*)),
                     buttonCallback, SLOT(buttonClicked(QObject*))); 
    AddButtonCB(aacbut[0], aacdialog_int_cb_proc, cbdata_apply);
    AddButtonCB(aacbut[1], aacdialog_int_cb_proc, cbdata_accept);
//    AddButtonCB(aacbut[2], destroy_dialog_cb, XtParent(form));
    QObject::connect(aacbut[2], SIGNAL(clicked()), form, SLOT(close()));
//
//    XtManageChild(container);
//    XtManageChild(form);
//
//    return aacbut;
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
	QCheckBox *cb = (QCheckBox*)(w);
	return cb->isChecked();
//        return XmToggleButtonGetState(w);
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

void set_title(char *title, char *icon_name)
{
}

/*
 *  Auxiliary routines for simultaneous drawing on display and pixmap
 */
void aux_XDrawLine(GUI *gui, int x1, int y1, int x2, int y2)
{
    X11Stuff *xstuff = gui->xstuff;
//    XDrawLine(xstuff->disp, xstuff->xwin, gcxor, x1, y1, x2, y2);
//    if (xstuff->bufpixmap != (Pixmap) NULL) {
//        XDrawLine(xstuff->disp, xstuff->bufpixmap, gcxor, x1, y1, x2, y2);
//    }
}

void aux_XDrawRectangle(GUI *gui, int x1, int y1, int x2, int y2)
{
    X11Stuff *xstuff = gui->xstuff;
//    XDrawRectangle(xstuff->disp, xstuff->xwin, gcxor, x1, y1, x2, y2);
//    if (xstuff->bufpixmap != (Pixmap) NULL) {
//        XDrawRectangle(xstuff->disp, xstuff->bufpixmap, gcxor, x1, y1, x2, y2);
//    }
}

void aux_XFillRectangle(GUI *gui, int x, int y, unsigned int width, unsigned int height)
{
    X11Stuff *xstuff = gui->xstuff;
//    XFillRectangle(xstuff->disp, xstuff->xwin, gcxor, x, y, width, height);
//    if (xstuff->bufpixmap != (Pixmap) NULL) {
//        XFillRectangle(xstuff->disp, xstuff->bufpixmap, gcxor, x, y, width, height);
//    }
}

void SetDimensions(Widget w, unsigned int width, unsigned int height)
{
//    XtVaSetValues(w,
//        XmNwidth, (Dimension) width,
//        XmNheight, (Dimension) height,
//        NULL);
}

void GetDimensions(Widget w, unsigned int *width, unsigned int *height)
{
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
//    xstream->screen = DefaultScreenOfDisplay(xstuff->disp);
//    xstream->pixmap = xstuff->bufpixmap;
}

void create_pixmap(unsigned int w, unsigned int h)
{
//    X11Stuff *xstuff = gapp->gui->xstuff;
//
//    xstuff->bufpixmap = XCreatePixmap(xstuff->disp, xstuff->root, w, h, xstuff->depth);
}

void recreate_pixmap(unsigned int w, unsigned int h)
{
//    X11Stuff *xstuff = gapp->gui->xstuff;
//
//    XFreePixmap(xstuff->disp, xstuff->bufpixmap);
//    create_pixmap(w, h);
}

void xdrawgrid(X11Stuff *xstuff)
{
}

void x11_redraw_all()
{
//    X11Stuff *xstuff = gapp->gui->xstuff;
//    if (gapp->gui->inwin == TRUE && xstuff->bufpixmap != (Pixmap) NULL) {
//        XCopyArea(xstuff->disp, xstuff->bufpixmap, window, xstuff->gc, x, y, width, height, x, y);
//    }
}

void set_left_footer(char *s)
{
}

void move_pointer(short x, short y)
{
 //   X11Stuff *xstuff = gapp->gui->xstuff;

//    XWarpPointer(xstuff->disp, None, xstuff->xwin, 0, 0, 0, 0, x, y);
}
