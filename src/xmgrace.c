/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2003 Grace Development Team
 * 
 * Maintained by Evgeny Stambulchik <fnevgeny@plasma-gate.weizmann.ac.il>
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

#include <Xbae/Matrix.h>

#if defined(HAVE_XPM_H)
#  include <xpm.h>
#else
#  if defined (HAVE_X11_XPM_H)
#    include <X11/xpm.h>
#  endif
#endif

#include "globals.h"
#include "bitmaps.h"
#include "utils.h"
#include "files.h"
#include "core_utils.h"
#include "plotone.h"
#include "events.h"
#include "protos.h"

#include "motifinc.h"


/* used globally */
XtAppContext app_con;
Widget app_shell;

Widget drawing_window;		/* container for drawing area */


/* used locally */
static Widget frleft, frtop, frbot;
static Widget windowbarw[3];
static Widget loclab;		/* locator label */
static Widget statlab;		/* status line at the bottom */

static Widget CreateMainMenuBar(Widget parent);
static void MenuCB(Widget but, void *data);
static void graph_scroll_proc(Widget but, void *data);
static void graph_zoom_proc(Widget but, void *data);
static void load_example(Widget but, void *data);

/*
 * action routines, to be used with translations
 */

/* This is for buggy Motif-2.1 that crashes with Ctrl+<Btn1Down> */
static void do_nothing_action(Widget w, XEvent *e, String *par, Cardinal *npar)
{
}

static XtActionsRec dummy_actions[] = {
    {"do_nothing", do_nothing_action}
};

static XtActionsRec canvas_actions[] = {
    {"autoscale",         autoscale_action        },     
    {"autoscale_on_near", autoscale_on_near_action},     
    {"draw_line",         draw_line_action        },     
    {"draw_box",          draw_box_action         },      
    {"draw_ellipse",      draw_ellipse_action     },  
    {"write_string",      write_string_action     },  
    {"delete_object",     delete_object_action    }, 
    {"refresh_hotlink",   refresh_hotlink_action  },
    {"set_viewport",      set_viewport_action     },  
    {"enable_zoom",       enable_zoom_action      }
};

static XtActionsRec list_select_actions[] = {
    {"list_selectall_action",       list_selectall_action      },
    {"list_unselectall_action",     list_unselectall_action    },
    {"list_invertselection_action", list_invertselection_action}
};

static XtActionsRec cstext_actions[] = {
    {"cstext_edit_action", cstext_edit_action}
};

static char canvas_table[] = "#override\n\
	Ctrl Alt <Key>l: draw_line()\n\
	Ctrl Alt <Key>b: draw_box()\n\
	Ctrl Alt <Key>e: draw_ellipse()\n\
	Ctrl Alt <Key>t: write_string()\n\
	Ctrl <Key>a: autoscale()\n\
	Ctrl <Key>d: delete_object()\n\
	Ctrl <Key>u: refresh_hotlink()\n\
	Ctrl <Key>v: set_viewport()\n\
	Ctrl <Key>z: enable_zoom()";

/*
 * establish resource stuff
 */
typedef struct {
    Boolean invert;
    Boolean allow_dc;
    Boolean auto_redraw;
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
    {"allowDoubleClick", "AllowDoubleClick", XtRBoolean, sizeof(Boolean),
     XtOffset(ApplicationDataPtr, allow_dc), XtRImmediate,
     (XtPointer) TRUE},
    {"allowRedraw", "AllowRedraw", XtRBoolean, sizeof(Boolean),
     XtOffset(ApplicationDataPtr, auto_redraw), XtRImmediate,
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
     (XtPointer) FALSE},
};

String fallbackResourcesCommon[] = {
    "XMgrace.consoleDialog*text.columns: 72",
    "XMgrace.consoleDialog*text.rows: 5",
    "XMgrace*background: #e5e5e5",
    "XMgrace*foreground: #000000",
    "XMgrace*XbaeMatrix.oddRowBackground: #cccccc",
    "XMgrace*XbaeMatrix.evenRowBackground: #cfe7e7",
    "XMgrace*fontTable.evenRowBackground: #e5e5e5",
    "XMgrace*XmPushButton.background: #b0c4de",
    "XMgrace*XmMenuShell*XmPushButton.background: #e5e5e5",
    "XMgrace*penButton.background: #e5e5e5",
    "XMgrace*XmText*background: #cfe7e7",
    "XMgrace*XmToggleButton.selectColor: #ff0000",
    "XMgrace*XmToggleButton.fillOnSelect: true",
    "XMgrace*XmSeparator.margin: 0",
#ifdef WITH_XMHTML
    "XMgrace*XmHTML.background: #ffffff",
    "XMgrace*XmHTML.foreground: #000000",
    "XMgrace*XmHTML.width: 600",
    "XMgrace*XmHTML.height: 500",
#endif
    "XMgrace*mainWin.shadowThickness: 0",
    "XMgrace*mainWin.menuBar.shadowThickness: 1",
    "XMgrace*menuBar*tearOffModel: XmTEAR_OFF_ENABLED",
    "XMgrace*dragInitiatorProtocolStyle: XmDRAG_NONE",
    "XMgrace*dragReceiverProtocolStyle:  XmDRAG_NONE",
    "XMgrace*fileMenu.newButton.acceleratorText: Ctrl+N",
    "XMgrace*fileMenu.newButton.accelerator: Ctrl<Key>n",
    "XMgrace*fileMenu.openButton.acceleratorText: Ctrl+O",
    "XMgrace*fileMenu.openButton.accelerator: Ctrl<Key>o",
    "XMgrace*fileMenu.saveButton.acceleratorText: Ctrl+S",
    "XMgrace*fileMenu.saveButton.accelerator: Ctrl<Key>s",
    "XMgrace*fileMenu.exitButton.acceleratorText: Ctrl+Q",
    "XMgrace*fileMenu.exitButton.accelerator: Ctrl<Key>q",
    "XMgrace*fileMenu.printButton.acceleratorText: Ctrl+P",
    "XMgrace*fileMenu.printButton.accelerator: Ctrl<Key>p",
    "XMgrace*helpMenu.onContextButton.acceleratorText: Shift+F1",
    "XMgrace*helpMenu.onContextButton.accelerator: Shift<Key>F1",
    "XMgrace*pageZoomMenu.smallerButton.acceleratorText: Ctrl+-",
    "XMgrace*pageZoomMenu.smallerButton.accelerator: Ctrl<Key>minus",
    "XMgrace*pageZoomMenu.largerButton.acceleratorText: Ctrl++",
    "XMgrace*pageZoomMenu.largerButton.accelerator: Ctrl<Key>plus",
    "XMgrace*pageZoomMenu.originalSizeButton.acceleratorText: Ctrl+1",
    "XMgrace*pageZoomMenu.originalSizeButton.accelerator: Ctrl<Key>1",
    "XMgrace*saveLogsFSB*pattern: *.log",
    "XMgrace*openProjectFSB*pattern: *.*gr",
    "XMgrace*saveProjectFSB*pattern: *.xgr",
    "XMgrace*readSetsFSB*pattern: *.dat",
    "XMgrace*writeSetsFSB*pattern: *.dat",
    "XMgrace*readParametersFSB*pattern: *.par",
    "XMgrace*writeParametersFSB*pattern: *.par",
    "XMgrace*selectNetcdfFileFSB*pattern: *.nc",
    "XMgrace*selectHotLinkFileFSB*pattern: *.dat",
    "XMgrace*openFitParameterFileFSB*pattern: *.fit",
    "XMgrace*saveFitParameterFileFSB*pattern: *.fit",
    NULL
};

String fallbackResourcesHighRes[] = {
    "XMgrace*mainWin.width: 680",
    "XMgrace*mainWin.height: 700",
    "XMgrace*explorerDialog.form.width: 680",
    "XMgrace*explorerDialog.form.height: 600",
    "XMgrace*fontList:-*-helvetica-medium-r-normal-*-12-*-*-*-*-*-*-*",
    "XMgrace*tabFontList:-*-helvetica-medium-r-normal-*-12-*-*-*-*-*-*-*",
    "XMgrace.consoleDialog*text.fontList:-*-courier-medium-r-normal-*-12-*-*-*-*-*-*-*",
    "XMgrace*HContainer.marginHeight: 3",
    "XMgrace*VContainer.marginHeight: 3",
    NULL
};

String fallbackResourcesLowRes[] = {
    "XMgrace*mainWin.width: 530",
    "XMgrace*mainWin.height: 545",
    "XMgrace*explorerDialog.form.width: 530",
    "XMgrace*explorerDialog.form.height: 485",
    "XMgrace*fontList:-*-helvetica-medium-r-normal-*-8-*-*-*-*-*-*-*",
    "XMgrace*tabFontList:-*-helvetica-medium-r-normal-*-8-*-*-*-*-*-*-*",
    "XMgrace.consoleDialog*text.fontList:-*-courier-medium-r-normal-*-8-*-*-*-*-*-*-*",
    "XMgrace*HContainer.marginHeight: 1",
    "XMgrace*VContainer.marginHeight: 1",
    NULL
};

/*
 * main menubar
 */
/* #define MENU_HELP	200 */
#define MENU_EXIT	201
#define MENU_NEW	203
#define MENU_OPEN	204
#define MENU_SAVE	205
#define MENU_SAVEAS	206
#define MENU_REVERT	207
#define MENU_PRINT	208

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
    Screen *screen;
    ApplicationData rd;
    String *allResources, *resolResources;
    int lowres = FALSE;
    unsigned int i, n_common, n_resol;
    char *display_name = NULL;

    xstuff = xmalloc(sizeof(X11Stuff));
    memset(xstuff, 0, sizeof(X11Stuff));
    grace->gui->xstuff = xstuff;
    
    installXErrorHandler();
    
    /* Locale settings for GUI */
    XtSetLanguageProc(NULL, NULL, NULL);
    
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
    
    XtDisplayInitialize(app_con, xstuff->disp, "xmgrace", "XMgrace", NULL, 0, argc, argv);

    XtAppAddActions(app_con, dummy_actions, XtNumber(dummy_actions));
    XtAppAddActions(app_con, canvas_actions, XtNumber(canvas_actions));
    XtAppAddActions(app_con, list_select_actions, XtNumber(list_select_actions));
    XtAppAddActions(app_con, cstext_actions, XtNumber(cstext_actions));

    app_shell = XtAppCreateShell(NULL, "XMgrace", applicationShellWidgetClass,
        xstuff->disp, NULL, 0);

    if (is_motif_compatible() != TRUE) {
        return RETURN_FAILURE;
    }
    
    XtGetApplicationResources(app_shell, &rd,
        resources, XtNumber(resources), NULL, 0);
    
    grace->gui->invert = rd.invert;
    grace->gui->allow_dc = rd.allow_dc;
    grace->gui->auto_redraw = rd.auto_redraw;
    grace->gui->instant_update = rd.instantupdate;
    grace->gui->toolbar = rd.toolbar;
    grace->gui->statusbar = rd.statusbar;
    grace->gui->locbar = rd.locatorbar;

    x11_init(grace);

    return RETURN_SUCCESS;
}

static void do_drawgraph(Widget but, void *data)
{
    xdrawgraph(grace->project, TRUE);
}


static void MenuCB(Widget but, void *data)
{
    char *s;
    
    switch ((int) data) {
    case MENU_EXIT:
	bailout(grace);
	break;
    case MENU_NEW:
	new_project(grace, NULL);

        xdrawgraph(grace->project, FALSE);
	break;
    case MENU_OPEN:
	create_openproject_popup();
	break;
    case MENU_SAVE:
	if (strcmp (project_get_docname(grace->project), NONAME) != 0) {
	    set_wait_cursor();
	    
	    save_project(grace->project, project_get_docname(grace->project));
	    
	    unset_wait_cursor();
	} else {
	    create_saveproject_popup();
	}
	break;
    case MENU_SAVEAS:
	create_saveproject_popup();
	break;
    case MENU_REVERT:
	set_wait_cursor();
	s = copy_string(NULL, project_get_docname(grace->project));
	if (strcmp (s, NONAME) != 0) {
            load_project(grace, s);
        } else {
	    new_project(grace, NULL);
        }
        xfree(s);
        xdrawgraph(grace->project, FALSE);
	unset_wait_cursor();
	break;
    case MENU_PRINT:
	set_wait_cursor();
	do_hardcopy(grace->project);
	unset_wait_cursor();
	break;
    default:
	break;
    }
}

/*
 * service the autoscale buttons on the main panel
 */
static void autoscale_proc(Widget but, void *data)
{
    Quark *cg = graph_get_current(grace->project);
    
    if (autoscale_graph(cg, (int) data) == RETURN_SUCCESS) {
	update_ticks();
        xdrawgraph(grace->project, FALSE);
    } else {
	errmsg("Can't autoscale (no active sets?)");
    }
}

static void autoon_proc(Widget but, void *data)
{
    set_action(0);
    set_action(AUTO_NEAREST);
}

/*
 * service the autoticks button on the main panel
 */
static void autoticks_proc(Widget but, void *data)
{
    autotick_graph_axes(graph_get_current(grace->project), AXIS_MASK_XY);
    update_ticks();
    xdrawgraph(grace->project, FALSE);
}

/*
 * set the message in the left footer
 */
void set_left_footer(char *s)
{
    if (s == NULL) {
        char hbuf[64];
        char buf[GR_MAXPATHLEN + 100];
        gethostname(hbuf, 63);
        sprintf(buf, "%s, %s, %s, %d%%",
            hbuf, display_name(grace->gui), project_get_docname(grace->project),
            (int) rint(100*grace->gui->zoom));
        SetLabel(statlab, buf);
    } else {
        SetLabel(statlab, s);
    }
    XmUpdateDisplay(statlab);
}

void set_tracker_string(char *s)
{
    if (s == NULL) {
        SetLabel(loclab, "[Out of frame]");
    } else {
        SetLabel(loclab, s);
    }
}

/*
 * clear the locator reference point
 */
static void do_clear_point(Widget but, void *data)
{
    GLocator *locator;
    
    locator = graph_get_locator(graph_get_current(grace->project));
    locator->pointset = FALSE;
    xdrawgraph(grace->project, FALSE);
}

/*
 * set visibility of the toolbars
 */
static void set_view_items(void)
{
    if (grace->gui->statusbar) {
	SetToggleButtonState(windowbarw[1], TRUE);
	ManageChild(frbot);
	XtVaSetValues(drawing_window,
		      XmNbottomAttachment, XmATTACH_WIDGET,
		      XmNbottomWidget, frbot,
		      NULL);
	if (grace->gui->toolbar) {
	    XtVaSetValues(frleft,
			  XmNbottomAttachment, XmATTACH_WIDGET,
			  XmNbottomWidget, frbot,
			  NULL);
	}
    } else {
	SetToggleButtonState(windowbarw[1], FALSE);
	XtVaSetValues(drawing_window,
		      XmNbottomAttachment, XmATTACH_FORM,
		      NULL);
	UnmanageChild(frbot);
	if (grace->gui->toolbar) {
	    XtVaSetValues(frleft,
			  XmNbottomAttachment, XmATTACH_FORM,
			  NULL);
	}
    }
    if (grace->gui->toolbar) {
	SetToggleButtonState(windowbarw[2], TRUE);
	ManageChild(frleft);
	if (grace->gui->statusbar) {
	    XtVaSetValues(frleft,
			  XmNbottomAttachment, XmATTACH_WIDGET,
			  XmNbottomWidget, frbot,
			  NULL);
	}
	if (grace->gui->locbar) {
	    XtVaSetValues(frleft,
			  XmNtopAttachment, XmATTACH_WIDGET,
			  XmNtopWidget, frtop,
			  NULL);
	}
	XtVaSetValues(drawing_window,
		      XmNleftAttachment, XmATTACH_WIDGET,
		      XmNleftWidget, frleft,
		      NULL);
    } else {
	SetToggleButtonState(windowbarw[2], FALSE);
	UnmanageChild(frleft);
	XtVaSetValues(drawing_window,
		      XmNleftAttachment, XmATTACH_FORM,
		      NULL);
    }
    if (grace->gui->locbar) {
	SetToggleButtonState(windowbarw[0], TRUE);
	ManageChild(frtop);
	XtVaSetValues(drawing_window,
		      XmNtopAttachment, XmATTACH_WIDGET,
		      XmNtopWidget, frtop,
		      NULL);
	if (grace->gui->toolbar) {
	    XtVaSetValues(frleft,
			  XmNtopAttachment, XmATTACH_WIDGET,
			  XmNtopWidget, frtop,
			  NULL);
	}
    } else {
	SetToggleButtonState(windowbarw[0], FALSE);
	UnmanageChild(frtop);
	XtVaSetValues(drawing_window,
		      XmNtopAttachment, XmATTACH_FORM,
		      NULL);
	if (grace->gui->toolbar) {
	    XtVaSetValues(frleft,
			  XmNtopAttachment, XmATTACH_FORM,
			  NULL);
	}
    }
}

/*
 * service routines for the View pulldown
 */
static void set_statusbar(Widget but, int onoff, void *data)
{
    grace->gui->statusbar = onoff;
    set_view_items();
}

static void set_toolbar(Widget but, int onoff, void *data)
{
    grace->gui->toolbar = onoff;
    set_view_items();
}

static void set_locbar(Widget but, int onoff, void *data)
{
    grace->gui->locbar = onoff;
    set_view_items();
}

static void zoom_in_cb(Widget but, void *data)
{
    Grace *grace = (Grace *) data;
    page_zoom_inout(grace, +1);
}

static void zoom_out_cb(Widget but, void *data)
{
    Grace *grace = (Grace *) data;
    page_zoom_inout(grace, -1);
}

static void zoom_1_cb(Widget but, void *data)
{
    Grace *grace = (Grace *) data;
    page_zoom_inout(grace, 0);
}

/*
 * create the main menubar
 */
static Widget CreateMainMenuBar(Widget parent)
{
    Widget menubar;
    Widget menupane, submenupane, sub2menupane;
    static char buf[128];

    menubar = CreateMenuBar(parent);

    /* File menu */
    menupane = CreateMenu(menubar, "File", 'F', FALSE);

    CreateMenuButton(menupane, "New", 'N', MenuCB, (void *) MENU_NEW);
    CreateMenuButton(menupane, "Open...", 'O', MenuCB, (void *) MENU_OPEN);
    CreateMenuButton(menupane, "Save", 'S', MenuCB, (void *) MENU_SAVE);
    CreateMenuButton(menupane, "Save as...", 'a', MenuCB, (void *) MENU_SAVEAS);
    CreateMenuButton(menupane, "Revert to saved", 'v', MenuCB, (void *) MENU_REVERT);

    CreateMenuSeparator(menupane);

    CreateMenuButton(menupane, "Print setup...", 't', create_printer_setup, &grace->rt->hdevice);
    CreateMenuButton(menupane, "Print", 'P', MenuCB, (void *) MENU_PRINT);
    CreateMenuSeparator(menupane);
    CreateMenuButton(menupane, "Exit", 'x', MenuCB, (void *) MENU_EXIT);

    /* Edit menu */
    menupane = CreateMenu(menubar, "Edit", 'E', FALSE);

    CreateMenuButton(menupane, "Data sets...", 'D', create_datasetprop_popup, NULL);

    CreateMenuSeparator(menupane);
    CreateMenuButton(menupane, "Arrange graphs...", 'r', create_arrange_frame, NULL);
    CreateMenuButton(menupane, "Autoscale graphs...", 'A', create_autos_frame, NULL);
    CreateMenuSeparator(menupane);

    CreateMenuButton(menupane, "Set locator fixed point", 'f', set_actioncb, (void *) SEL_POINT);
    CreateMenuButton(menupane, "Clear locator fixed point", 'C', do_clear_point, NULL);
    
    CreateMenuSeparator(menupane);

    CreateMenuButton(menupane, "Preferences...", 'r', create_props_frame, NULL);

    /* Data menu */
    menupane = CreateMenu(menubar, "Data", 'D', FALSE);

    CreateMenuButton(menupane, "Data set operations...", 'o', create_datasetop_popup, NULL);

    submenupane = CreateMenu(menupane, "Transformations", 'T', FALSE);
    CreateMenuButton(submenupane, "Evaluate expression...", 'E', create_eval_frame, NULL);
    CreateMenuSeparator(submenupane);
    CreateMenuButton(submenupane, "Histograms...", 'H', create_histo_frame, NULL);
    CreateMenuButton(submenupane, "Fourier transforms...", 'u', create_fourier_frame, NULL);
    CreateMenuSeparator(submenupane);
    CreateMenuButton(submenupane, "Running properties...", 's', create_run_frame, NULL);
    CreateMenuButton(submenupane, "Differences/derivatives...", 'D', create_diff_frame, NULL);
    CreateMenuButton(submenupane, "Integration...", 'I', create_int_frame, NULL);
    CreateMenuSeparator(submenupane);
    CreateMenuButton(submenupane, "Interpolation/splines...", 't', create_interp_frame, NULL);
    CreateMenuButton(submenupane, "Non-linear curve fitting...", 'N', create_nonl_frame, NULL);
    CreateMenuSeparator(submenupane);
    CreateMenuButton(submenupane, "Correlation/covariance...", 'C', create_xcor_frame, NULL);
    CreateMenuButton(submenupane, "Linear convolution...", 'v', create_lconv_frame, NULL);
    CreateMenuSeparator(submenupane);
    CreateMenuButton(submenupane, "Sample points...", 'm', create_samp_frame, NULL);
    CreateMenuButton(submenupane, "Prune data...", 'P', create_prune_frame, NULL);

    CreateMenuButton(menupane, "Feature extraction...", 'x', create_featext_frame, NULL);
    CreateMenuButton(menupane, "Cumulative properties...", 'C', create_cumulative_frame, NULL);

    CreateMenuSeparator(menupane);
    submenupane = CreateMenu(menupane, "Import", 'I', FALSE);
    CreateMenuButton(submenupane, "ASCII...", 'A', create_file_popup, NULL);
#ifdef HAVE_NETCDF
    CreateMenuButton(submenupane, "NetCDF...", 'N', create_netcdfs_popup, NULL);
#endif
   
    submenupane = CreateMenu(menupane, "Export", 'E', FALSE);
    CreateMenuButton(submenupane, "ASCII...", 'A', create_write_popup, NULL);

    /* View menu */
    menupane = CreateMenu(menubar, "View", 'V', FALSE);

    submenupane = CreateMenu(menupane, "Show/Hide", 'w', FALSE);
    windowbarw[0] = CreateMenuToggle(submenupane, "Locator bar", 'L', set_locbar, NULL);
    windowbarw[1] = CreateMenuToggle(submenupane, "Status bar", 'S', set_statusbar, NULL);
    windowbarw[2] = CreateMenuToggle(submenupane, "Tool bar", 'T', set_toolbar, NULL);

    if (!gui_is_page_free(grace->gui)) {
        submenupane = CreateMenu(menupane, "Page zoom", 'z', FALSE);
        CreateMenuButton(submenupane, "Smaller", 'S', zoom_out_cb, grace);
        CreateMenuButton(submenupane, "Larger", 'L', zoom_in_cb, grace);
        CreateMenuSeparator(submenupane);
        CreateMenuButton(submenupane, "Original size", 'O', zoom_1_cb, grace);
    }

    CreateMenuSeparator(menupane);

    CreateMenuButton(menupane, "Page setup...", 'P', create_printer_setup, &grace->rt->tdevice);

    CreateMenuSeparator(menupane);

    CreateMenuButton(menupane, "Redraw", 'R', do_drawgraph, NULL);

    CreateMenuSeparator(menupane);

    CreateMenuButton(menupane, "Update all", 'U', update_all_cb, NULL);

    /* Window menu */
    menupane = CreateMenu(menubar, "Tools", 'T', FALSE);
   
    CreateMenuButton(menupane, "Explorer", 'E', define_explorer_popup, grace->gui);
    CreateMenuButton(menupane, "Console", 'C', create_monitor_frame_cb, NULL);
#if 0
    CreateMenuButton(menupane, "Point explorer", 'P', create_points_frame, NULL);
#endif
    CreateMenuButton(menupane, "Font tool", 'F', create_fonttool_cb, NULL);
/*
 *     CreateMenuButton(menupane, "Area/perimeter...", 'A', create_area_frame, NULL);
 */
    

    /* Help menu */
    menupane = CreateMenu(menubar, "Help", 'H', TRUE);

    CreateMenuButton(menupane, "User's Guide", 'G', HelpCB, "doc/UsersGuide.html");
    CreateMenuButton(menupane, "Tutorial", 'T', HelpCB, "doc/Tutorial.html");
    CreateMenuButton(menupane, "FAQ", 'Q', HelpCB, "doc/FAQ.html");
    CreateMenuButton(menupane, "Changes", 'C', HelpCB, "doc/CHANGES.html");

    CreateMenuSeparator(menupane);
 
    submenupane = CreateMenu(menupane, "Examples", 'E', FALSE);
    sub2menupane = CreateMenu(submenupane, "General intro", 'i', FALSE);
    CreateMenuButton(sub2menupane, "Explain", '\0', load_example, "explain.agr");
    CreateMenuButton(sub2menupane, "Properties", '\0', load_example, "props.agr");
    CreateMenuButton(sub2menupane, "Axes", '\0',load_example, "axes.agr");
    CreateMenuButton(sub2menupane, "Fonts", '\0', load_example, "tfonts.agr");
    CreateMenuButton(sub2menupane, "Arrows", '\0', load_example, "arrows.agr");
    CreateMenuButton(sub2menupane, "Symbols and lines", '\0', load_example, "symslines.agr");
    CreateMenuButton(sub2menupane, "Fills", '\0', load_example, "fills.agr");
    CreateMenuButton(sub2menupane, "World stack", '\0', load_example, "tstack.agr");
    CreateMenuButton(sub2menupane, "Inset graphs", '\0', load_example, "tinset.agr");
    CreateMenuButton(sub2menupane, "Many graphs", '\0', load_example, "manygraphs.agr");

    sub2menupane = CreateMenu(submenupane, "XY graphs", 'g', FALSE);
    CreateMenuButton(sub2menupane, "Log scale", '\0', load_example, "tlog.agr");
    CreateMenuButton(sub2menupane, "Log2 scale", '\0', load_example, "log2.agr");
    CreateMenuButton(sub2menupane, "Logit scale", '\0', load_example, "logit.agr");
    CreateMenuButton(sub2menupane, "Reciprocal scale", '\0', load_example, "reciprocal.agr");
    CreateMenuButton(sub2menupane, "Error bars", '\0', load_example, "terr.agr");
    CreateMenuButton(sub2menupane, "Date/time axis formats", '\0', load_example, "times.agr");
    CreateMenuButton(sub2menupane, "Australia map", '\0', load_example, "au.agr");
    CreateMenuButton(sub2menupane, "A CO2 analysis", '\0', load_example, "co2.agr");
    CreateMenuButton(sub2menupane, "Motif statistics", '\0', load_example, "motif.agr");
    CreateMenuButton(sub2menupane, "Spectrum", '\0', load_example, "spectrum.agr");

    sub2menupane = CreateMenu(submenupane, "XY charts", 'c', FALSE);
    CreateMenuButton(sub2menupane, "Bar chart", '\0', load_example, "bar.agr");
    CreateMenuButton(sub2menupane, "Stacked bar", '\0', load_example, "stackedb.agr");
    CreateMenuButton(sub2menupane, "Bar chart with error bars", '\0', load_example, "chartebar.agr");
    CreateMenuButton(sub2menupane, "Different charts", '\0', load_example, "charts.agr");

    sub2menupane = CreateMenu(submenupane, "Polar graphs", 'P', FALSE);
    CreateMenuButton(sub2menupane, "Polar graph", '\0', load_example, "polar.agr");

    sub2menupane = CreateMenu(submenupane, "Pie charts", 'i', FALSE);
    CreateMenuButton(sub2menupane, "Pie chart", '\0', load_example, "pie.agr");

    sub2menupane = CreateMenu(submenupane, "Special set presentations", 'S', FALSE);
    CreateMenuButton(sub2menupane, "HILO", '\0', load_example, "hilo.agr");
    CreateMenuButton(sub2menupane, "XY Radius", '\0', load_example, "txyr.agr");
    CreateMenuButton(sub2menupane, "XYZ", '\0', load_example, "xyz.agr");
    CreateMenuButton(sub2menupane, "Box plot", '\0', load_example, "boxplot.agr");
    CreateMenuButton(sub2menupane, "Vector map", '\0', load_example, "vmap.agr");
    CreateMenuButton(sub2menupane, "XY Size", '\0', load_example, "xysize.agr");
    CreateMenuButton(sub2menupane, "XY Color", '\0', load_example, "xycolor.agr");

    sub2menupane = CreateMenu(submenupane, "Type setting", 'T', FALSE);
    CreateMenuButton(sub2menupane, "Simple", '\0', load_example, "test2.agr");
    CreateMenuButton(sub2menupane, "Text transforms", '\0', load_example, "txttrans.agr");
    CreateMenuButton(sub2menupane, "Advanced", '\0', load_example, "typeset.agr");

    sub2menupane = CreateMenu(submenupane, "Calculus", 'u', FALSE);
    CreateMenuButton(sub2menupane, "Non-linear fit", '\0', load_example, "logistic.agr");
 
    CreateMenuSeparator(menupane);

    sprintf(buf,
        "http://plasma-gate.weizmann.ac.il/Grace/comments.phtml?version_id=%ld",
        bi_version_id());
    CreateMenuButton(menupane, "Comments", 'm', HelpCB, buf);
    CreateMenuSeparator(menupane);
    CreateMenuButton(menupane, "License terms", 'L', HelpCB, "doc/GPL.html");
    CreateMenuButton(menupane, "About...", 'A', create_about_grtool, NULL);

    return (menubar);
}


/*
 * build the GUI
 */
void startup_gui(Grace *grace)
{
    X11Stuff *xstuff = grace->gui->xstuff;
    Widget main_frame, form, menu_bar, bt, rcleft;
    Pixmap icon, shape;

/* 
 * Allow users to change tear off menus with X resources
 */
    XmRepTypeInstallTearOffModelConverter();
    
    RegisterEditRes(app_shell);

/*
 * We handle important WM events ourselves
 */
    handle_close(app_shell);
    
    XtVaSetValues(app_shell, XmNcolormap, xstuff->cmap, NULL);
    
/*
 * build the UI here
 */
    main_frame = XtVaCreateManagedWidget("mainWin",
        xmMainWindowWidgetClass, app_shell, NULL);

    menu_bar = CreateMainMenuBar(main_frame);
    ManageChild(menu_bar);

    form = XmCreateForm(main_frame, "form", NULL, 0);

    frleft = CreateFrame(form, NULL);
    rcleft = XtVaCreateManagedWidget("toolBar", xmRowColumnWidgetClass, frleft,
				     XmNorientation, XmVERTICAL,
				     XmNpacking, XmPACK_TIGHT,
				     XmNspacing, 0,
				     XmNentryBorder, 0,
				     XmNmarginWidth, 0,
				     XmNmarginHeight, 0,
				     NULL);

    frtop = CreateFrame(form, NULL);
    loclab = CreateLabel(frtop, NULL);
    
    frbot = CreateFrame(form, NULL);
    statlab = CreateLabel(frbot, NULL);

    if (!gui_is_page_free(grace->gui)) {
        drawing_window = XtVaCreateManagedWidget("drawing_window",
				     xmScrolledWindowWidgetClass, form,
				     XmNscrollingPolicy, XmAUTOMATIC,
				     XmNvisualPolicy, XmVARIABLE,
				     NULL);
        xstuff->canvas = XtVaCreateManagedWidget("canvas",
                                     xmDrawingAreaWidgetClass, drawing_window,
				     NULL);
    } else {
        xstuff->canvas = XtVaCreateManagedWidget("canvas",
                                     xmDrawingAreaWidgetClass, form,
				     NULL);
        drawing_window = xstuff->canvas;
    }
    
    XtAddCallback(xstuff->canvas, XmNexposeCallback, expose_resize, grace);
    XtAddCallback(xstuff->canvas, XmNresizeCallback, expose_resize, grace);

    XtAddEventHandler(xstuff->canvas,
                      ButtonPressMask
                      | ButtonReleaseMask
		      | PointerMotionMask
		      | KeyPressMask
		      | KeyReleaseMask
		      | ColormapChangeMask,
		      False,
		      canvas_event_proc, grace);
		      
    XtOverrideTranslations(xstuff->canvas, XtParseTranslationTable(canvas_table));
    
    AddHelpCB(xstuff->canvas, "doc/UsersGuide.html#canvas");

    XtVaSetValues(frtop,
		  XmNtopAttachment, XmATTACH_FORM,
		  XmNleftAttachment, XmATTACH_FORM,
		  XmNrightAttachment, XmATTACH_FORM,
		  NULL);
    XtVaSetValues(frbot,
		  XmNbottomAttachment, XmATTACH_FORM,
		  XmNrightAttachment, XmATTACH_FORM,
		  XmNleftAttachment, XmATTACH_FORM,
		  NULL);
    XtVaSetValues(frleft,
		  XmNtopAttachment, XmATTACH_WIDGET,
		  XmNtopWidget, frtop,
		  XmNbottomAttachment, XmATTACH_WIDGET,
		  XmNbottomWidget, frbot,
		  XmNleftAttachment, XmATTACH_FORM,
		  NULL);
    XtVaSetValues(drawing_window,
		  XmNtopAttachment, XmATTACH_WIDGET,
		  XmNtopWidget, frtop,
		  XmNbottomAttachment, XmATTACH_WIDGET,
		  XmNbottomWidget, frbot,
		  XmNleftAttachment, XmATTACH_WIDGET,
		  XmNleftWidget, frleft,
		  XmNrightAttachment, XmATTACH_FORM,
		  NULL);

    ManageChild(form);

    XmMainWindowSetAreas(main_frame, menu_bar, NULL, NULL, NULL, form);

    /* redraw */
    bt = CreateBitmapButton(rcleft, 16, 16, redraw_bits);
    AddButtonCB(bt, do_drawgraph, NULL);
    
    CreateSeparator(rcleft);

    /* zoom */
    bt = CreateBitmapButton(rcleft, 16, 16, zoom_bits);
    AddButtonCB(bt, set_actioncb, (void *) ZOOM_1ST);
    bt = CreateBitmapButton(rcleft, 16, 16, zoom_x_bits);
    AddButtonCB(bt, set_actioncb, (void *) ZOOMX_1ST);
    bt = CreateBitmapButton(rcleft, 16, 16, zoom_y_bits);
    AddButtonCB(bt, set_actioncb, (void *) ZOOMY_1ST);

    CreateSeparator(rcleft);

    /* autoscale */
    bt = CreateBitmapButton(rcleft, 16, 16, auto_bits);
    AddButtonCB(bt, autoscale_proc, (void *) AUTOSCALE_XY);
    bt = CreateBitmapButton(rcleft, 16, 16, auto_x_bits);
    AddButtonCB(bt, autoscale_proc, (void *) AUTOSCALE_X);
    bt = CreateBitmapButton(rcleft, 16, 16, auto_y_bits);
    AddButtonCB(bt, autoscale_proc, (void *) AUTOSCALE_Y);
    bt = CreateBitmapButton(rcleft, 16, 16, auto_on_bits);
    AddButtonCB(bt, autoon_proc, NULL);
    bt = CreateBitmapButton(rcleft, 16, 16, auto_tick_bits);
    AddButtonCB(bt, autoticks_proc, NULL);

    CreateSeparator(rcleft);

    /* scrolling buttons */
    bt = CreateBitmapButton(rcleft, 16, 16, left_bits);
    AddButtonCB(bt, graph_scroll_proc, (void *) GSCROLL_LEFT);
    bt = CreateBitmapButton(rcleft, 16, 16, right_bits);
    AddButtonCB(bt, graph_scroll_proc, (void *) GSCROLL_RIGHT);
    bt = CreateBitmapButton(rcleft, 16, 16, up_bits);
    AddButtonCB(bt, graph_scroll_proc, (void *) GSCROLL_UP);
    bt = CreateBitmapButton(rcleft, 16, 16, down_bits);
    AddButtonCB(bt, graph_scroll_proc, (void *) GSCROLL_DOWN);

    CreateSeparator(rcleft);

    /* expand/shrink */
    bt = CreateBitmapButton(rcleft, 16, 16, expand_bits);
    AddButtonCB(bt, graph_zoom_proc, (void *) GZOOM_EXPAND);
    bt = CreateBitmapButton(rcleft, 16, 16, shrink_bits);
    AddButtonCB(bt, graph_zoom_proc, (void *) GZOOM_SHRINK);

    CreateSeparator(rcleft);
    CreateSeparator(rcleft);

    /* exit */
    bt = CreateBitmapButton(rcleft, 16, 16, exit_bits);
    AddButtonCB(bt, MenuCB, (void *) MENU_EXIT);

/*
 * initialize cursors
 */
    init_cursors(grace->gui);

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
#if defined(HAVE_XPM)
    XpmCreatePixmapFromData(xstuff->disp, xstuff->root,
        grace_icon_xpm, &icon, &shape, NULL);
#else
    icon = XCreateBitmapFromData(xstuff->disp, xstuff->root,
        (char *) grace_icon_bits, grace_icon_width, grace_icon_height);
    shape = XCreateBitmapFromData(xstuff->disp, xstuff->root,
        (char *) grace_mask_bits, grace_icon_width, grace_icon_height);
#endif
    XtVaSetValues(app_shell, XtNiconPixmap, icon, XtNiconMask, shape, NULL);

    XtRealizeWidget(app_shell);
    
    XmProcessTraversal(xstuff->canvas, XmTRAVERSE_CURRENT);
    
    xstuff->xwin = XtWindow(xstuff->canvas);
    grace->gui->inwin = TRUE;

/*
 * set the title
 */
    update_app_title(grace->project);

    XtAppMainLoop(app_con);
}

static int scroll_hook(Quark *q, void *udata, QTraverseClosure *closure)
{
    if (q->fid == QFlavorGraph) {
        int *type = (int *) udata;
        closure->descend = FALSE;
        graph_scroll(q, *type);
    }

    return TRUE;
}

static void graph_scroll_proc(Widget but, void *data)
{
    Quark *cg, *f;
    int type = (int) data;
    
    cg = graph_get_current(grace->project);
    f = get_parent_frame(cg);
    
    quark_traverse(f, scroll_hook, &type);
    
    xdrawgraph(grace->project, FALSE);
}

static int zoom_hook(Quark *q, void *udata, QTraverseClosure *closure)
{
    if (q->fid == QFlavorGraph) {
        int *type = (int *) udata;
        closure->descend = FALSE;
        graph_zoom(q, *type);
    }

    return TRUE;
}

static void graph_zoom_proc(Widget but, void *data)
{
    Quark *cg, *f;
    int type = (int) data;
    
    cg = graph_get_current(grace->project);
    f = get_parent_frame(cg);
    
    quark_traverse(f, zoom_hook, &type);
    
    xdrawgraph(grace->project, FALSE);
}

static void load_example(Widget but, void *data)
{
    char *s, buf[128];
    
    set_wait_cursor();
    
    s = (char *) data;
    sprintf(buf, "examples/%s", s);
    load_project_file(grace, buf, FALSE);

    xdrawgraph(grace->project, FALSE);

    unset_wait_cursor();
}
