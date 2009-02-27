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

#include "mainwindow.h"

extern "C" {
  #include <bitmaps.h>
  #include "xprotos.h"
}

int main_cpp(int argc, char *argv[], GraceApp *gapp)
{
  QApplication app(argc, argv);
  MainWindow mainWin(gapp);
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
//    X11Stuff *xstuff;
//    MainWinUI *mwui;
//    Screen *screen;
//    ApplicationData rd;
//    String *allResources, *resolResources;
//    int lowres = FALSE;
//    unsigned int i, n_common, n_resol;
//    char *display_name = NULL;
//
//    xstuff = xmalloc(sizeof(X11Stuff));
//    memset(xstuff, 0, sizeof(X11Stuff));
//    gapp->gui->xstuff = xstuff;
//
//    mwui = xmalloc(sizeof(MainWinUI));
//    memset(mwui, 0, sizeof(MainWinUI));
//    gapp->gui->mwui = mwui;
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

