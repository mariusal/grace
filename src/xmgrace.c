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

/* 
 *
 * Main Motif interface
 *
 */

#include <config.h>

#include <unistd.h>

#include <Xm/Xm.h>
#include <Xm/MainW.h>
#include <Xm/Form.h>
#include <Xm/RowColumn.h>
#include <Xm/ScrolledW.h>
#include <Xm/DrawingA.h>
#include <Xm/RepType.h>

#if defined(HAVE_XPM_H)
#  include <xpm.h>
#else
#  include <X11/xpm.h>
#endif

#ifdef WITH_EDITRES
#  include <X11/Xmu/Editres.h>
#endif

#include "globals.h"
#include "bitmaps.h"
#include "utils.h"
#include "files.h"
#include "core_utils.h"
#include "events.h"
#include "xprotos.h"

#include "motifinc.h"


/* used globally */
XtAppContext app_con;
Widget app_shell;

/*
 * establish resource stuff
 */
typedef struct {
    Boolean invert;
    Boolean toolbar;
    Boolean statusbar;
    Boolean locatorbar;
    Boolean instantupdate;
} ApplicationData, *ApplicationDataPtr;

static XtResource resources[] =
{
    {"invertDraw", "InvertDraw", XtRBoolean, sizeof(Boolean),
     XtOffset(ApplicationDataPtr, invert), XtRImmediate,
     (XtPointer) TRUE},
    {"toolBar", "ToolBar", XtRBoolean, sizeof(Boolean),
     XtOffset(ApplicationDataPtr, toolbar), XtRImmediate,
     (XtPointer) TRUE},
    {"statusBar", "StatusBar", XtRBoolean, sizeof(Boolean),
     XtOffset(ApplicationDataPtr, statusbar), XtRImmediate,
     (XtPointer) TRUE},
    {"locatorBar", "LocatorBar", XtRBoolean, sizeof(Boolean),
     XtOffset(ApplicationDataPtr, locatorbar), XtRImmediate,
     (XtPointer) TRUE},
    {"instantUpdate", "InstantUpdate", XtRBoolean, sizeof(Boolean),
     XtOffset(ApplicationDataPtr, instantupdate), XtRImmediate,
     (XtPointer) TRUE},
};

String fallbackResourcesCommon[] = {
    "XmGrace.consoleDialog*text.columns: 72",
    "XmGrace.consoleDialog*text.rows: 5",
    "XmGrace*background: #e5e5e5",
    "XmGrace*foreground: #000000",
    "XmGrace*XbaeMatrix.allowColumnResize: False",
    "XmGrace*XbaeMatrix.allowRowResize: False",
    "XmGrace*XbaeMatrix.oddRowBackground: #cccccc",
    "XmGrace*XbaeMatrix.evenRowBackground: #cfe7e7",
    "XmGrace*XbaeMatrix.textBackground: #cfe7e7",
    "XmGrace*XbaeMatrix.selectedBackground: #e5ffff",
    "XmGrace*XbaeMatrix.selectedForeground: #000000",
    "XmGrace*fontTable.evenRowBackground: #e5e5e5",
    "XmGrace*SSD.evenRowBackground: #ffffff",
    "XmGrace*SSD.buttonLabelBackground: #cccccc",
    "XmGrace*XmPushButton.background: #b0c4de",
    "XmGrace*XmMenuShell*XmPushButton.background: #e5e5e5",
    "XmGrace*penButton.background: #e5e5e5",
    "XmGrace*XmText*background: #cfe7e7",
    "XmGrace*XmToggleButton.selectColor: #ff0000",
    "XmGrace*XmToggleButton.fillOnSelect: true",
    "XmGrace*XmSeparator.margin: 0",
    "XmGrace*XmSash.HighlightColor: #b0c4de",
#ifdef WITH_XMHTML
    "XmGrace*XmHTML.background: #ffffff",
    "XmGrace*XmHTML.foreground: #000000",
    "XmGrace*XmHTML.width: 600",
    "XmGrace*XmHTML.height: 500",
#endif
    "XmGrace*enableThinThickness: True",
    "XmGrace*mainWin.shadowThickness: 0",
    "XmGrace*XmMenuShell*shadowThickness: 2",
    "XmGrace*menuBar*tearOffModel: XmTEAR_OFF_ENABLED",
    NULL
};

String fallbackResourcesHighRes[] = {
    "XmGrace*mainWin.width: 780",
    "XmGrace*mainWin.height: 660",
    "XmGrace*explorerDialog.form.width: 650",
    "XmGrace*explorerDialog.form.height: 600",
    "XmGrace*fontList:-*-helvetica-medium-r-normal-*-12-*-*-*-*-*-*-*",
    "XmGrace.consoleDialog*text.fontList:-*-courier-medium-r-normal-*-12-*-*-*-*-*-*-*",
    "XmGrace*ListTree.font:-*-helvetica-medium-r-normal-*-12-*-*-*-*-*-*-*",
    "XmGrace*HContainer.marginHeight: 3",
    "XmGrace*VContainer.marginHeight: 3",
    NULL
};

String fallbackResourcesLowRes[] = {
    "XmGrace*mainWin.width: 600",
    "XmGrace*mainWin.height: 520",
    "XmGrace*explorerDialog.form.width: 530",
    "XmGrace*explorerDialog.form.height: 485",
    "XmGrace*fontList:-*-helvetica-medium-r-normal-*-8-*-*-*-*-*-*-*",
    "XmGrace.consoleDialog*text.fontList:-*-courier-medium-r-normal-*-8-*-*-*-*-*-*-*",
    "XmGrace*ListTree.font:-*-helvetica-medium-r-normal-*-8-*-*-*-*-*-*-*",
    "XmGrace*HContainer.marginHeight: 1",
    "XmGrace*VContainer.marginHeight: 1",
    NULL
};

#ifdef HAVE__XMVERSIONSTRING
extern const char _XmVersionString[];
#endif

static int is_motif_compatible(void)
{
    char buf[128];
    int bd_lesstif;
#ifdef HAVE__XMVERSIONSTRING
    char *s;
    int rt_lesstif;
#endif

    /* First, check for compatible version */
    if (xmUseVersion < XmVersion) {
        sprintf(buf,
            "Run-time Motif library is older than the build, %d < %d",
            xmUseVersion, XmVersion);
        errmsg(buf);
        return FALSE;
    }
    
    bd_lesstif = (strstr(bi_gui(), "Motif") == NULL);

#ifdef HAVE__XMVERSIONSTRING
    /* Then, check whether we are in the Motif/LessTif binary compatibility
       mode */
    /* strcpy is dangerous since the sizeof(_XmVersionString) may be different
       at run time! 13 chars should be safe, though, and enough to distinguish
       between Motif and LessTif :) */
    strncpy(buf, _XmVersionString, 13);
    buf[13] = '\0';
    rt_lesstif = (strstr(buf, "Motif") == NULL);
    if (bd_lesstif != rt_lesstif) {
        sprintf(buf, "The software was built with %s, but is running with %s!",
            bd_lesstif ? "LessTif":"Motif", rt_lesstif ? "LessTif":"Motif");
        errmsg(buf);
        errmsg("We don't support binary Motif/LessTif compatibility.");
        errmsg("Use a semistatic binary or compile Grace yourself!");
        return FALSE;
    }
    
    /* Finally, if LessTif is used, check for a reasonably new release */
    if (rt_lesstif) {
        s = strstr(_XmVersionString, "Version");
        if (s == NULL || (strcmp(s, "Version 0.90.0") < 0)) {
            errmsg("An old version of LessTif, please upgrade to 0.90.0 at least");
        }
    }
#endif

#if XbaeVersion >= 40800
    /* Now we should compare whether Xbae was built against the
       runtime version of M*tif/LessTif. */
    strncpy(buf, XbaeGetXmVersionTxt(), 13);
    buf[13] = '\0';
    rt_lesstif = (strstr(buf, "Motif") == NULL);
    if (bd_lesstif != rt_lesstif) {
        sprintf(buf, "libXbae was built with %s, but is running with %s!",
            bd_lesstif ? "LessTif":"Motif", rt_lesstif ? "LessTif":"Motif");
        errmsg(buf);
        errmsg("Use a semistatic binary or compile Grace/libXbae yourself!");
        return FALSE;
    }
    /* Now we check for consistency of the used M*tif version */
    if (XbaeGetXmVersionNum() != XmVersion) {
        sprintf(buf, "libXbae was built with Motif/LessTif %i, but is running with %i!",
            XbaeGetXmVersionNum(), XmVersion);
        errmsg(buf);
        errmsg("Use a semistatic binary or compile Grace/libXbae yourself!");
        return FALSE;
    }
#endif /* XbaeVersion > 40800 */

    return TRUE;
}

int initialize_gui(int *argc, char **argv)
{
    X11Stuff *xstuff;
    MainWinUI *mwui;
    Screen *screen;
    ApplicationData rd;
    String *allResources, *resolResources;
    int lowres = FALSE;
    unsigned int i, n_common, n_resol;
    char *display_name = NULL;

    xstuff = xmalloc(sizeof(X11Stuff));
    memset(xstuff, 0, sizeof(X11Stuff));
    gapp->gui->xstuff = xstuff;

    mwui = xmalloc(sizeof(MainWinUI));
    memset(mwui, 0, sizeof(MainWinUI));
    gapp->gui->mwui = mwui;
    
    installXErrorHandler();
    
    /* Locale settings for GUI */
    if (!is_locale_utf8()) {
        XtSetLanguageProc(NULL, NULL, NULL);
    }
    
    XtToolkitInitialize();
    app_con = XtCreateApplicationContext();
    
    /* Check if we're running in the low-resolution X */
    for (i = 1; i < *argc - 1; i++) {
        /* See if display name was specified in the args */
        char *pattern = "-display";
        if (strlen(argv[i]) > 1 && strstr(pattern, argv[i]) == pattern) {
            display_name = argv[i + 1];
        }
    }
    xstuff->disp = XOpenDisplay(display_name);
    if (xstuff->disp == NULL) {
	errmsg("Can't open display");
        return RETURN_FAILURE;
    }

    screen = DefaultScreenOfDisplay(xstuff->disp);
    if (HeightOfScreen(screen) < 740) {
        lowres = TRUE;
    }
    
    n_common = sizeof(fallbackResourcesCommon)/sizeof(String) - 1;
    if (lowres) {
        n_resol  = sizeof(fallbackResourcesLowRes)/sizeof(String) - 1;
        resolResources = fallbackResourcesLowRes;
    } else {
        n_resol  = sizeof(fallbackResourcesHighRes)/sizeof(String) - 1;
        resolResources = fallbackResourcesHighRes;
    }
    allResources = xmalloc((n_common + n_resol + 1)*sizeof(String));
    for (i = 0; i < n_common; i++) {
        allResources[i] = fallbackResourcesCommon[i];
    }
    for (i = 0; i < n_resol; i++) {
        allResources[n_common + i] = resolResources[i];
    }
    allResources[n_common + n_resol] = NULL;
    XtAppSetFallbackResources(app_con, allResources);
    
    XtDisplayInitialize(app_con, xstuff->disp, "xmgrace", "XmGrace", NULL, 0, argc, argv);

    /* Widget translations etc */
    InitWidgets();
    
    app_shell = XtAppCreateShell(NULL, "XmGrace", applicationShellWidgetClass,
        xstuff->disp, NULL, 0);

    if (is_motif_compatible() != TRUE) {
        return RETURN_FAILURE;
    }
    
    XtGetApplicationResources(app_shell, &rd,
        resources, XtNumber(resources), NULL, 0);
    
    gapp->gui->invert = rd.invert;
    gapp->gui->instant_update = rd.instantupdate;
    gapp->gui->toolbar = rd.toolbar;
    gapp->gui->statusbar = rd.statusbar;
    gapp->gui->locbar = rd.locatorbar;

    x11_init(gapp);

    /* initialize cursors */
    init_cursors(gapp->gui);

    return RETURN_SUCCESS;
}

/*
 * set visibility of the toolbars
 */
void set_view_items(void)
{
    MainWinUI *mwui = gapp->gui->mwui;
    
    if (gapp->gui->statusbar) {
	SetToggleButtonState(mwui->windowbarw[1], TRUE);
	WidgetManage(mwui->frbot);
	XtVaSetValues(mwui->drawing_window,
		      XmNbottomAttachment, XmATTACH_WIDGET,
		      XmNbottomWidget, mwui->frbot,
		      NULL);
	if (gapp->gui->toolbar) {
	    XtVaSetValues(mwui->frleft,
			  XmNbottomAttachment, XmATTACH_WIDGET,
			  XmNbottomWidget, mwui->frbot,
			  NULL);
	}
    } else {
	SetToggleButtonState(mwui->windowbarw[1], FALSE);
	XtVaSetValues(mwui->drawing_window,
		      XmNbottomAttachment, XmATTACH_FORM,
		      NULL);
	WidgetUnmanage(mwui->frbot);
	if (gapp->gui->toolbar) {
	    XtVaSetValues(mwui->frleft,
			  XmNbottomAttachment, XmATTACH_FORM,
			  NULL);
	}
    }
    if (gapp->gui->toolbar) {
	SetToggleButtonState(mwui->windowbarw[2], TRUE);
	WidgetManage(mwui->frleft);
	if (gapp->gui->statusbar) {
	    XtVaSetValues(mwui->frleft,
			  XmNbottomAttachment, XmATTACH_WIDGET,
			  XmNbottomWidget, mwui->frbot,
			  NULL);
	}
	if (gapp->gui->locbar) {
	    XtVaSetValues(mwui->frleft,
			  XmNtopAttachment, XmATTACH_WIDGET,
			  XmNtopWidget, mwui->frtop,
			  NULL);
	}
	XtVaSetValues(mwui->drawing_window,
		      XmNleftAttachment, XmATTACH_WIDGET,
		      XmNleftWidget, mwui->frleft,
		      NULL);
    } else {
	SetToggleButtonState(mwui->windowbarw[2], FALSE);
	WidgetUnmanage(mwui->frleft);
	XtVaSetValues(mwui->drawing_window,
		      XmNleftAttachment, XmATTACH_FORM,
		      NULL);
    }
    if (gapp->gui->locbar) {
	SetToggleButtonState(mwui->windowbarw[0], TRUE);
	WidgetManage(mwui->frtop);
	XtVaSetValues(mwui->drawing_window,
		      XmNtopAttachment, XmATTACH_WIDGET,
		      XmNtopWidget, mwui->frtop,
		      NULL);
	if (gapp->gui->toolbar) {
	    XtVaSetValues(mwui->frleft,
			  XmNtopAttachment, XmATTACH_WIDGET,
			  XmNtopWidget, mwui->frtop,
			  NULL);
	}
    } else {
	SetToggleButtonState(mwui->windowbarw[0], FALSE);
	WidgetUnmanage(mwui->frtop);
	XtVaSetValues(mwui->drawing_window,
		      XmNtopAttachment, XmATTACH_FORM,
		      NULL);
	if (gapp->gui->toolbar) {
	    XtVaSetValues(mwui->frleft,
			  XmNtopAttachment, XmATTACH_FORM,
			  NULL);
	}
    }
}

static void canvas_event_proc(Widget w, XtPointer data, XEvent *event, Boolean *cont)
{
    CanvasEvent cevent;
    KeySym keybuf;
    XMotionEvent *xme;
    XButtonEvent *xbe;
    XKeyEvent    *xke;

    cevent.modifiers = NO_MODIFIER;
    cevent.button    = NO_BUTTON;
    cevent.key       = KEY_NONE;
    cevent.x = event->xmotion.x;
    cevent.y = event->xmotion.y;

    switch (event->type) {
    case MotionNotify:
        cevent.type = MOUSE_MOVE;
        xme = (XMotionEvent *) event;
        if (xme->state & Button1Mask) {
            cevent.button = cevent.button ^ LEFT_BUTTON;
        }
        if (xme->state & ControlMask) {
            cevent.modifiers = cevent.modifiers ^ CONTROL_MODIFIER;
        }
        break;
    case ButtonPress:
        cevent.type = MOUSE_PRESS;
        xbe = (XButtonEvent *) event;
        cevent.udata = xbe;
        cevent.x = event->xbutton.x;
        cevent.y = event->xbutton.y;
        switch (event->xbutton.button) {
        case Button1:
            cevent.button = cevent.button ^ LEFT_BUTTON;
            cevent.time = xbe->time;
            break;
        case Button2:
            cevent.button = cevent.button ^ MIDDLE_BUTTON;
            break;
        case Button3:
            cevent.button = cevent.button ^ RIGHT_BUTTON;
            break;
        case Button4:
            cevent.button = cevent.button ^ WHEEL_UP_BUTTON;
            break;
        case Button5:
            cevent.button = cevent.button ^ WHEEL_DOWN_BUTTON;
            break;
        }
        if (xbe->state & ControlMask) {
            cevent.modifiers = cevent.modifiers ^ CONTROL_MODIFIER;
        }
        break;
    case ButtonRelease:
        cevent.type = MOUSE_RELEASE;
        xbe = (XButtonEvent *) event;
        switch (event->xbutton.button) {
        case Button1:
            cevent.button = cevent.button ^ LEFT_BUTTON;
            break;
        }
        if (xbe->state & ControlMask) {
            cevent.modifiers = cevent.modifiers ^ CONTROL_MODIFIER;
        }
        break;
    case KeyPress:
        cevent.type = KEY_PRESS;
        xke = (XKeyEvent *) event;
        keybuf = XLookupKeysym(xke, 0);
        switch (keybuf) {
        case XK_Escape: /* Esc */
            cevent.key = KEY_ESCAPE;
            break;
        case XK_KP_Add: /* "Grey" plus */
            cevent.key = KEY_PLUS;
            break;
        case XK_KP_Subtract: /* "Grey" minus */
            cevent.key = KEY_MINUS;
            break;
        case XK_1:
            cevent.key = KEY_1;
            break;
        }
        if (xke->state & ControlMask) {
            cevent.modifiers = cevent.modifiers ^ CONTROL_MODIFIER;
        }
        break;
    case KeyRelease:
        cevent.type = KEY_RELEASE;
        xke = (XKeyEvent *) event;
        if (xke->state & ControlMask) {
            cevent.modifiers = cevent.modifiers ^ CONTROL_MODIFIER;
        }
        break;
    default:
        break;
    }

    canvas_event(&cevent);
}

static void close_windowCB(Widget_CBData *wcbdata)
{
    bailout(wcbdata->anydata);
}

/*
 * build the GUI
 */
void startup_gui(GraceApp *gapp)
{
    MainWinUI *mwui = gapp->gui->mwui;
    X11Stuff *xstuff = gapp->gui->xstuff;
    Widget main_frame, form, menu_bar, rcleft;
    Pixmap icon, shape;

/* 
 * Allow users to change tear off menus with X resources
 */
    XmRepTypeInstallTearOffModelConverter();
    
#ifdef WITH_EDITRES
    XtAddEventHandler(app_shell, (EventMask) 0, True, _XEditResCheckMessages, NULL);
#endif

/*
 * We handle important WM events ourselves
 */
    AddWindowCloseCB(app_shell, close_windowCB, gapp);
    
    XtVaSetValues(app_shell, XmNcolormap, xstuff->cmap, NULL);
    
/*
 * build the UI here
 */
    main_frame = XtVaCreateManagedWidget("mainWin",
        xmMainWindowWidgetClass, app_shell, NULL);

    menu_bar = CreateMainMenuBar(main_frame);
    WidgetManage(menu_bar);

    form = CreateForm(main_frame);

    mwui->frleft = CreateFrame(form, NULL);
    rcleft = XtVaCreateManagedWidget("toolBar", xmRowColumnWidgetClass,
                                     mwui->frleft,
				     XmNorientation, XmVERTICAL,
				     XmNpacking, XmPACK_TIGHT,
				     XmNspacing, 0,
				     XmNentryBorder, 0,
				     XmNmarginWidth, 0,
				     XmNmarginHeight, 0,
				     NULL);

    mwui->frtop = CreateFrame(form, NULL);
    mwui->loclab = CreateLabel(mwui->frtop, NULL);
    
    mwui->frbot = CreateFrame(form, NULL);
    mwui->statlab = CreateLabel(mwui->frbot, NULL);

    if (!gui_is_page_free(gapp->gui)) {
        mwui->drawing_window = XtVaCreateManagedWidget("drawing_window",
				     xmScrolledWindowWidgetClass, form,
				     XmNscrollingPolicy, XmAUTOMATIC,
				     XmNvisualPolicy, XmVARIABLE,
				     NULL);
        xstuff->canvas = XtVaCreateManagedWidget("canvas",
                                     xmDrawingAreaWidgetClass,
                                     mwui->drawing_window,
				     NULL);
    } else {
        xstuff->canvas = XtVaCreateManagedWidget("canvas",
                                     xmDrawingAreaWidgetClass, form,
				     NULL);
        mwui->drawing_window = xstuff->canvas;
    }
    
    XtAddCallback(xstuff->canvas, XmNexposeCallback, expose_resize, gapp);
    XtAddCallback(xstuff->canvas, XmNresizeCallback, expose_resize, gapp);

    XtAddEventHandler(xstuff->canvas,
                      ButtonPressMask
                      | ButtonReleaseMask
		      | PointerMotionMask
		      | KeyPressMask
		      | KeyReleaseMask
		      | ColormapChangeMask,
		      False,
		      canvas_event_proc, gapp);
		      
    AddHelpCB(xstuff->canvas, "doc/UsersGuide.html#canvas");

    XtVaSetValues(mwui->frtop,
		  XmNtopAttachment, XmATTACH_FORM,
		  XmNleftAttachment, XmATTACH_FORM,
		  XmNrightAttachment, XmATTACH_FORM,
		  NULL);
    XtVaSetValues(mwui->frbot,
		  XmNbottomAttachment, XmATTACH_FORM,
		  XmNrightAttachment, XmATTACH_FORM,
		  XmNleftAttachment, XmATTACH_FORM,
		  NULL);
    XtVaSetValues(mwui->frleft,
		  XmNtopAttachment, XmATTACH_WIDGET,
		  XmNtopWidget, mwui->frtop,
		  XmNbottomAttachment, XmATTACH_WIDGET,
		  XmNbottomWidget, mwui->frbot,
		  XmNleftAttachment, XmATTACH_FORM,
		  NULL);
    XtVaSetValues(mwui->drawing_window,
		  XmNtopAttachment, XmATTACH_WIDGET,
		  XmNtopWidget, mwui->frtop,
		  XmNbottomAttachment, XmATTACH_WIDGET,
		  XmNbottomWidget, mwui->frbot,
		  XmNleftAttachment, XmATTACH_WIDGET,
		  XmNleftWidget, mwui->frleft,
		  XmNrightAttachment, XmATTACH_FORM,
		  NULL);

    WidgetManage(form);

    XmMainWindowSetAreas(main_frame, menu_bar, NULL, NULL, NULL, form);

    CreateToolBar(rcleft);
/*
 * initialize some option menus
 */
    init_option_menus();

/*
 * initialize the tool bars
 */
    set_view_items();

    set_tracker_string(NULL);
    set_left_footer(NULL);

/*
 * set icon
 */
    XpmCreatePixmapFromData(xstuff->disp, xstuff->root,
        gapp_icon_xpm, &icon, &shape, NULL);
    XtVaSetValues(app_shell, XtNiconPixmap, icon, XtNiconMask, shape, NULL);

    XtRealizeWidget(app_shell);
    
    XmProcessTraversal(xstuff->canvas, XmTRAVERSE_CURRENT);
    
    xstuff->xwin = XtWindow(xstuff->canvas);
    gapp->gui->inwin = TRUE;

/*
 * set the title
 */
    update_app_title(gapp->gp);

    XtAppMainLoop(app_con);
}

