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

#include <Xbae/Matrix.h>

#if defined(HAVE_XPM_H)
#  include <xpm.h>
#else
#  include <X11/xpm.h>
#endif

#include "globals.h"
#include "bitmaps.h"
#include "utils.h"
#include "files.h"
#include "core_utils.h"
#include "events.h"
#include "protos.h"

#include "motifinc.h"


/* used globally */
XtAppContext app_con;
Widget app_shell;

static Widget CreateMainMenuBar(Widget parent);
static void graph_scroll_left_cb(Widget but, void *data);
static void graph_scroll_right_cb(Widget but, void *data);
static void graph_scroll_up_cb(Widget but, void *data);
static void graph_scroll_down_cb(Widget but, void *data);
static void graph_zoom_in_cb(Widget but, void *data);
static void graph_zoom_out_cb(Widget but, void *data);
static void load_example_cb(Widget but, void *data);

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
    {"refresh_hotlink",   refresh_hotlink_action  },
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
	Ctrl <Key>a: autoscale()\n\
	Ctrl <Key>u: refresh_hotlink()\n\
	Ctrl <Key>z: enable_zoom()";

/*
 * establish resource stuff
 */
typedef struct {
    Boolean invert;
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
     (XtPointer) TRUE},
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
    "XMgrace*XmSash.HighlightColor: #b0c4de",
#ifdef WITH_XMHTML
    "XMgrace*XmHTML.background: #ffffff",
    "XMgrace*XmHTML.foreground: #000000",
    "XMgrace*XmHTML.width: 600",
    "XMgrace*XmHTML.height: 500",
#endif
    "XMgrace*mainWin.shadowThickness: 0",
    "XMgrace*mainWin.menuBar.shadowThickness: 1",
    "XMgrace*toolBar*highlightThickness: 1",
    "XMgrace*toolBar*shadowThickness: 1",
    "XMgrace*menuBar*tearOffModel: XmTEAR_OFF_ENABLED",
    "XMgrace*fileMenu.newButton.acceleratorText: Ctrl+N",
    "XMgrace*fileMenu.newButton.accelerator: Ctrl<Key>n",
    "XMgrace*fileMenu.openButton.acceleratorText: Ctrl+O",
    "XMgrace*fileMenu.openButton.accelerator: Ctrl<Key>o",
    "XMgrace*fileMenu.saveButton.acceleratorText: Ctrl+S",
    "XMgrace*fileMenu.saveButton.accelerator: Ctrl<Key>s",
    "XMgrace*fileMenu.exitButton.acceleratorText: Ctrl+Q",
    "XMgrace*fileMenu.exitButton.accelerator: Ctrl<Key>q",
    "XMgrace*fileMenu.printSetupButton.acceleratorText: Ctrl+P",
    "XMgrace*fileMenu.printSetupButton.accelerator: Ctrl<Key>p",
    "XMgrace*fileMenu.printButton.acceleratorText: Ctrl+Alt+P",
    "XMgrace*fileMenu.printButton.accelerator: Ctrl Alt<Key>p",
    "XMgrace*editMenu.undoButton.acceleratorText: Ctrl+Z",
    "XMgrace*editMenu.undoButton.accelerator: Ctrl<Key>z",
    "XMgrace*editMenu.redoButton.acceleratorText: Ctrl+Shift+Z",
    "XMgrace*editMenu.redoButton.accelerator: Ctrl Shift<Key>z",
    "XMgrace*editMenu.explorerButton.acceleratorText: Ctrl+E",
    "XMgrace*editMenu.explorerButton.accelerator: Ctrl<Key>e",
    "XMgrace*helpMenu.onContextButton.acceleratorText: Shift+F1",
    "XMgrace*helpMenu.onContextButton.accelerator: Shift<Key>F1",
    "XMgrace*pageZoomMenu.smallerButton.acceleratorText: Ctrl+-",
    "XMgrace*pageZoomMenu.smallerButton.accelerator: Ctrl<Key>minus",
    "XMgrace*pageZoomMenu.largerButton.acceleratorText: Ctrl++",
    "XMgrace*pageZoomMenu.largerButton.accelerator: Ctrl<Key>plus",
    "XMgrace*pageZoomMenu.originalSizeButton.acceleratorText: Ctrl+1",
    "XMgrace*pageZoomMenu.originalSizeButton.accelerator: Ctrl<Key>1",
    "XMgrace*viewMenu.redrawButton.acceleratorText: Ctrl+L",
    "XMgrace*viewMenu.redrawButton.accelerator: Ctrl<Key>l",
    "XMgrace*findButton.acceleratorText: Ctrl+F",
    "XMgrace*findButton.accelerator: Ctrl<Key>f",
    "XMgrace*findAgainButton.acceleratorText: Ctrl+G",
    "XMgrace*findAgainButton.accelerator: Ctrl<Key>g",
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
    "XMgrace*mainWin.width: 780",
    "XMgrace*mainWin.height: 660",
    "XMgrace*explorerDialog.form.width: 650",
    "XMgrace*explorerDialog.form.height: 600",
    "XMgrace*fontList:-*-helvetica-medium-r-normal-*-12-*-*-*-*-*-*-*",
    "XMgrace.consoleDialog*text.fontList:-*-courier-medium-r-normal-*-12-*-*-*-*-*-*-*",
    "XMgrace*ListTree.font:-*-helvetica-medium-r-normal-*-12-*-*-*-*-*-*-*",
    "XMgrace*HContainer.marginHeight: 3",
    "XMgrace*VContainer.marginHeight: 3",
    NULL
};

String fallbackResourcesLowRes[] = {
    "XMgrace*mainWin.width: 600",
    "XMgrace*mainWin.height: 520",
    "XMgrace*explorerDialog.form.width: 530",
    "XMgrace*explorerDialog.form.height: 485",
    "XMgrace*fontList:-*-helvetica-medium-r-normal-*-8-*-*-*-*-*-*-*",
    "XMgrace.consoleDialog*text.fontList:-*-courier-medium-r-normal-*-8-*-*-*-*-*-*-*",
    "XMgrace*ListTree.font:-*-helvetica-medium-r-normal-*-8-*-*-*-*-*-*-*",
    "XMgrace*HContainer.marginHeight: 1",
    "XMgrace*VContainer.marginHeight: 1",
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
    grace->gui->xstuff = xstuff;

    mwui = xmalloc(sizeof(MainWinUI));
    memset(mwui, 0, sizeof(MainWinUI));
    grace->gui->mwui = mwui;
    
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
    grace->gui->auto_redraw = rd.auto_redraw;
    grace->gui->instant_update = rd.instantupdate;
    grace->gui->toolbar = rd.toolbar;
    grace->gui->statusbar = rd.statusbar;
    grace->gui->locbar = rd.locatorbar;

    x11_init(grace);

    /* initialize cursors */
    init_cursors(grace->gui);

    return RETURN_SUCCESS;
}

static void do_drawgraph(Widget but, void *data)
{
    xdrawgraph(grace->project, TRUE);
}


/*
 * service the autoscale buttons on the main panel
 */
static void autoscale_proc(Grace *grace, int type)
{
    Quark *cg = graph_get_current(grace->project);
    
    if (autoscale_graph(cg, type) == RETURN_SUCCESS) {
        xdrawgraph(grace->project, FALSE);
    } else {
	errmsg("Can't autoscale (no active sets?)");
    }
}

static void autoscale_xy_cb(Widget but, void *data)
{
    autoscale_proc((Grace *) data, AUTOSCALE_XY);
}

static void autoscale_x_cb(Widget but, void *data)
{
    autoscale_proc((Grace *) data, AUTOSCALE_X);
}

static void autoscale_y_cb(Widget but, void *data)
{
    autoscale_proc((Grace *) data, AUTOSCALE_Y);
}

/*
 * service the autoticks button on the main panel
 */
static void autoticks_cb(Widget but, void *data)
{
    autotick_graph_axes(graph_get_current(grace->project), AXIS_MASK_XY);
    xdrawgraph(grace->project, FALSE);
}

/*
 * set the message in the left footer
 */
void set_left_footer(char *s)
{
    Widget statlab = grace->gui->mwui->statlab;

    if (s == NULL) {
        char hbuf[64], buf[GR_MAXPATHLEN + 100], *prname;
        gethostname(hbuf, 63);
        prname = project_get_docname(grace->project);
        if (prname) {
            sprintf(buf, "%s, %s, %s, %d%%", hbuf, display_name(grace->gui),
                prname, (int) rint(100*grace->gui->zoom));
        } else {
            sprintf(buf, "%s, %s", hbuf, display_name(grace->gui));
        }
        SetLabel(statlab, buf);
    } else {
        SetLabel(statlab, s);
    }
    XmUpdateDisplay(statlab);
}

void set_tracker_string(char *s)
{
    Widget loclab = grace->gui->mwui->loclab;
    
    if (s == NULL) {
        SetLabel(loclab, "[Out of frame]");
    } else {
        SetLabel(loclab, s);
    }
}

/*
 * set visibility of the toolbars
 */
static void set_view_items(void)
{
    MainWinUI *mwui = grace->gui->mwui;
    
    if (grace->gui->statusbar) {
	SetToggleButtonState(mwui->windowbarw[1], TRUE);
	ManageChild(mwui->frbot);
	XtVaSetValues(mwui->drawing_window,
		      XmNbottomAttachment, XmATTACH_WIDGET,
		      XmNbottomWidget, mwui->frbot,
		      NULL);
	if (grace->gui->toolbar) {
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
	UnmanageChild(mwui->frbot);
	if (grace->gui->toolbar) {
	    XtVaSetValues(mwui->frleft,
			  XmNbottomAttachment, XmATTACH_FORM,
			  NULL);
	}
    }
    if (grace->gui->toolbar) {
	SetToggleButtonState(mwui->windowbarw[2], TRUE);
	ManageChild(mwui->frleft);
	if (grace->gui->statusbar) {
	    XtVaSetValues(mwui->frleft,
			  XmNbottomAttachment, XmATTACH_WIDGET,
			  XmNbottomWidget, mwui->frbot,
			  NULL);
	}
	if (grace->gui->locbar) {
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
	UnmanageChild(mwui->frleft);
	XtVaSetValues(mwui->drawing_window,
		      XmNleftAttachment, XmATTACH_FORM,
		      NULL);
    }
    if (grace->gui->locbar) {
	SetToggleButtonState(mwui->windowbarw[0], TRUE);
	ManageChild(mwui->frtop);
	XtVaSetValues(mwui->drawing_window,
		      XmNtopAttachment, XmATTACH_WIDGET,
		      XmNtopWidget, mwui->frtop,
		      NULL);
	if (grace->gui->toolbar) {
	    XtVaSetValues(mwui->frleft,
			  XmNtopAttachment, XmATTACH_WIDGET,
			  XmNtopWidget, mwui->frtop,
			  NULL);
	}
    } else {
	SetToggleButtonState(mwui->windowbarw[0], FALSE);
	UnmanageChild(mwui->frtop);
	XtVaSetValues(mwui->drawing_window,
		      XmNtopAttachment, XmATTACH_FORM,
		      NULL);
	if (grace->gui->toolbar) {
	    XtVaSetValues(mwui->frleft,
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

static void new_cb(Widget but, void *data)
{
    Grace *grace = (Grace *) data;
    new_project(grace, NULL);
    xdrawgraph(grace->project, FALSE);
}


static void exit_cb(Widget but, void *data)
{
    Grace *grace = (Grace *) data;
    bailout(grace);
}

static void open_cb(Widget but, void *data)
{
    create_openproject_popup();
}

static void save_cb(Widget but, void *data)
{
    Grace *grace = (Grace *) data;
    if (strcmp (project_get_docname(grace->project), NONAME) != 0) {
	set_wait_cursor();

	save_project(grace->project, project_get_docname(grace->project));
        update_all();

	unset_wait_cursor();
    } else {
	create_saveproject_popup();
    }
}

static void save_as_cb(Widget but, void *data)
{
    create_saveproject_popup();
}

static void revert_cb(Widget but, void *data)
{
    char *s;
    Grace *grace = (Grace *) data;

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
}

static void print_cb(Widget but, void *data)
{
    Grace *grace = (Grace *) data;
    set_wait_cursor();
    do_hardcopy(grace->project);
    unset_wait_cursor();
}

static void undo_stats(AMem *amem)
{
    printf("undo = %d, redo = %d\n",
        amem_get_undo_count(amem), amem_get_redo_count(amem));
}

static void undo_cb(Widget but, void *data)
{
    Grace *grace = (Grace *) data;
    AMem *amem = quark_get_amem(grace->project);
    
    amem_undo(amem);
    
    xdrawgraph(grace->project, FALSE);
    update_all();
    
    undo_stats(amem);
}

static void redo_cb(Widget but, void *data)
{
    Grace *grace = (Grace *) data;
    AMem *amem = quark_get_amem(grace->project);
    
    amem_redo(amem);
    
    xdrawgraph(grace->project, FALSE);
    update_all();
    
    undo_stats(amem);
}


/*
 * create the main menubar
 */
static Widget CreateMainMenuBar(Widget parent)
{
    MainWinUI *mwui = grace->gui->mwui;
    Widget menubar;
    Widget menupane, submenupane, sub2menupane;
    static char buf[128];

    menubar = CreateMenuBar(parent);

    /* File menu */
    menupane = CreateMenu(menubar, "File", 'F', FALSE);

    CreateMenuButton(menupane, "New", 'N', new_cb, grace);
    CreateMenuButton(menupane, "Open...", 'O', open_cb, grace);
    CreateMenuButton(menupane, "Save", 'S', save_cb, grace);
    CreateMenuButton(menupane, "Save as...", 'a', save_as_cb, grace);
    CreateMenuButton(menupane, "Revert to saved", 'v', revert_cb, grace);

    CreateMenuSeparator(menupane);

    CreateMenuButton(menupane, "Print setup...", 't', create_printer_setup, &grace->rt->hdevice);
    CreateMenuButton(menupane, "Print", 'P', print_cb, grace);
    CreateMenuSeparator(menupane);
    CreateMenuButton(menupane, "Exit", 'x', exit_cb, grace);

    /* Edit menu */
    menupane = CreateMenu(menubar, "Edit", 'E', FALSE);

    mwui->undo_button = CreateMenuButton(menupane, "Undo", 'U', undo_cb, grace);
    mwui->redo_button = CreateMenuButton(menupane, "Redo", 'R', redo_cb, grace);

    CreateMenuSeparator(menupane);

    CreateMenuButton(menupane, "Explorer...", 'E', define_explorer_popup, grace->gui);

    CreateMenuSeparator(menupane);

    CreateMenuButton(menupane, "Arrange frames...", 'f', create_arrange_frame, NULL);
    CreateMenuButton(menupane, "Autoscale graphs...", 'A', create_autos_frame, NULL);

    CreateMenuSeparator(menupane);

    CreateMenuButton(menupane, "Preferences...", 'P', create_props_frame, NULL);

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
    mwui->windowbarw[0] = CreateMenuToggle(submenupane, "Locator bar", 'L', set_locbar, NULL);
    mwui->windowbarw[1] = CreateMenuToggle(submenupane, "Status bar", 'S', set_statusbar, NULL);
    mwui->windowbarw[2] = CreateMenuToggle(submenupane, "Tool bar", 'T', set_toolbar, NULL);

    if (!gui_is_page_free(grace->gui)) {
        submenupane = CreateMenu(menupane, "Page zoom", 'z', FALSE);
        CreateMenuButton(submenupane, "Smaller", 'S', zoom_out_cb, grace);
        CreateMenuButton(submenupane, "Larger", 'L', zoom_in_cb, grace);
        CreateMenuSeparator(submenupane);
        CreateMenuButton(submenupane, "Original size", 'O', zoom_1_cb, grace);
    }

    CreateMenuSeparator(menupane);

    CreateMenuButton(menupane, "Page rendering...", 'P', create_printer_setup, &grace->rt->tdevice);

    CreateMenuSeparator(menupane);

    CreateMenuButton(menupane, "Redraw", 'R', do_drawgraph, NULL);

    CreateMenuSeparator(menupane);

    CreateMenuButton(menupane, "Update all", 'U', update_all_cb, NULL);

    /* Window menu */
    menupane = CreateMenu(menubar, "Tools", 'T', FALSE);
   
    CreateMenuButton(menupane, "Console", 'C', create_monitor_frame_cb, NULL);
#if 0
    CreateMenuButton(menupane, "Point explorer", 'P', create_points_frame, NULL);
#endif
    CreateMenuButton(menupane, "Font tool", 'F', create_fonttool_cb, NULL);
/*
 *     CreateMenuButton(menupane, "Area/perimeter...", 'A', create_area_frame, NULL);
 */
    CreateMenuButton(menupane, "Dataset statistics", 'D', create_datasetprop_popup, NULL);

    /* Help menu */
    menupane = CreateMenu(menubar, "Help", 'H', TRUE);

    CreateMenuButton(menupane, "User's Guide", 'G', HelpCB, "doc/UsersGuide.html");
    CreateMenuButton(menupane, "FAQ", 'Q', HelpCB, "doc/FAQ.html");
    CreateMenuButton(menupane, "Changes", 'C', HelpCB, "doc/NEWS.html");

    CreateMenuSeparator(menupane);
 
    submenupane = CreateMenu(menupane, "Examples", 'E', FALSE);
    sub2menupane = CreateMenu(submenupane, "New 5.99 samples", 'N', FALSE);
    CreateMenuButton(sub2menupane, "Diode", '\0', load_example_cb, "diode.xgr");

    sub2menupane = CreateMenu(submenupane, "General intro", 'i', FALSE);
    CreateMenuButton(sub2menupane, "Explain", '\0', load_example_cb, "explain.agr");
    CreateMenuButton(sub2menupane, "Properties", '\0', load_example_cb, "props.agr");
    CreateMenuButton(sub2menupane, "Axes", '\0',load_example_cb, "axes.agr");
    CreateMenuButton(sub2menupane, "Fonts", '\0', load_example_cb, "tfonts.agr");
    CreateMenuButton(sub2menupane, "Arrows", '\0', load_example_cb, "arrows.agr");
    CreateMenuButton(sub2menupane, "Symbols and lines", '\0', load_example_cb, "symslines.agr");
    CreateMenuButton(sub2menupane, "Fills", '\0', load_example_cb, "fills.agr");
    CreateMenuButton(sub2menupane, "Inset graphs", '\0', load_example_cb, "tinset.agr");
    CreateMenuButton(sub2menupane, "Many graphs", '\0', load_example_cb, "manygraphs.agr");

    sub2menupane = CreateMenu(submenupane, "XY graphs", 'g', FALSE);
    CreateMenuButton(sub2menupane, "Log scale", '\0', load_example_cb, "tlog.agr");
    CreateMenuButton(sub2menupane, "Log2 scale", '\0', load_example_cb, "log2.agr");
    CreateMenuButton(sub2menupane, "Logit scale", '\0', load_example_cb, "logit.agr");
    CreateMenuButton(sub2menupane, "Reciprocal scale", '\0', load_example_cb, "reciprocal.agr");
    CreateMenuButton(sub2menupane, "Error bars", '\0', load_example_cb, "terr.agr");
    CreateMenuButton(sub2menupane, "Date/time axis formats", '\0', load_example_cb, "times.agr");
    CreateMenuButton(sub2menupane, "Australia map", '\0', load_example_cb, "au.agr");
    CreateMenuButton(sub2menupane, "A CO2 analysis", '\0', load_example_cb, "co2.agr");
    CreateMenuButton(sub2menupane, "Motif statistics", '\0', load_example_cb, "motif.agr");
    CreateMenuButton(sub2menupane, "Spectrum", '\0', load_example_cb, "spectrum.agr");

    sub2menupane = CreateMenu(submenupane, "XY charts", 'c', FALSE);
    CreateMenuButton(sub2menupane, "Bar chart", '\0', load_example_cb, "bar.agr");
    CreateMenuButton(sub2menupane, "Stacked bar", '\0', load_example_cb, "stackedb.agr");
    CreateMenuButton(sub2menupane, "Bar chart with error bars", '\0', load_example_cb, "chartebar.agr");
    CreateMenuButton(sub2menupane, "Different charts", '\0', load_example_cb, "charts.agr");

    sub2menupane = CreateMenu(submenupane, "Polar graphs", 'P', FALSE);
    CreateMenuButton(sub2menupane, "Polar graph", '\0', load_example_cb, "polar.agr");

    sub2menupane = CreateMenu(submenupane, "Pie charts", 'i', FALSE);
    CreateMenuButton(sub2menupane, "Pie chart", '\0', load_example_cb, "pie.agr");

    sub2menupane = CreateMenu(submenupane, "Special set presentations", 'S', FALSE);
    CreateMenuButton(sub2menupane, "HILO", '\0', load_example_cb, "hilo.agr");
    CreateMenuButton(sub2menupane, "XY Radius", '\0', load_example_cb, "txyr.agr");
    CreateMenuButton(sub2menupane, "XYZ", '\0', load_example_cb, "xyz.agr");
    CreateMenuButton(sub2menupane, "Box plot", '\0', load_example_cb, "boxplot.agr");
    CreateMenuButton(sub2menupane, "Vector map", '\0', load_example_cb, "vmap.agr");
    CreateMenuButton(sub2menupane, "XY Size", '\0', load_example_cb, "xysize.agr");
    CreateMenuButton(sub2menupane, "XY Color", '\0', load_example_cb, "xycolor.agr");

    sub2menupane = CreateMenu(submenupane, "Type setting", 'T', FALSE);
    CreateMenuButton(sub2menupane, "Simple", '\0', load_example_cb, "test2.agr");
    CreateMenuButton(sub2menupane, "Text transforms", '\0', load_example_cb, "txttrans.agr");
    CreateMenuButton(sub2menupane, "Advanced", '\0', load_example_cb, "typeset.agr");

#if 0
    sub2menupane = CreateMenu(submenupane, "Calculus", 'u', FALSE);
    CreateMenuButton(sub2menupane, "Non-linear fit", '\0', load_example_cb, "logistic.agr");
#endif
    CreateMenuSeparator(menupane);
    submenupane = CreateMenu(menupane, "Web support", 'W', FALSE);

    CreateMenuButton(submenupane, "Home page", 'H', HelpCB,
        "http://plasma-gate.weizmann.ac.il/Grace/");
    CreateMenuButton(submenupane, "Forums", 'F', HelpCB,
        "http://plasma-gate.weizmann.ac.il/Grace/phpbb/");
    sprintf(buf,
        "http://plasma-gate.weizmann.ac.il/Grace/report.php?version_id=%ld",
        bi_version_id());
    CreateMenuButton(submenupane, "Report an issue", 'R', HelpCB, buf);
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
    MainWinUI *mwui = grace->gui->mwui;
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

    if (!gui_is_page_free(grace->gui)) {
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

    ManageChild(form);

    XmMainWindowSetAreas(main_frame, menu_bar, NULL, NULL, NULL, form);

    /* redraw */
    bt = CreateBitmapButton(rcleft, 16, 16, redraw_bits);
    AddButtonCB(bt, do_drawgraph, NULL);
    
    CreateSeparator(rcleft);

    /* zoom */
    bt = CreateBitmapButton(rcleft, 16, 16, zoom_bits);
    AddButtonCB(bt, set_zoom_cb, (void *) grace);
    bt = CreateBitmapButton(rcleft, 16, 16, zoom_x_bits);
    AddButtonCB(bt, set_zoomx_cb, (void *) grace);
    bt = CreateBitmapButton(rcleft, 16, 16, zoom_y_bits);
    AddButtonCB(bt, set_zoomy_cb, (void *) grace);

    CreateSeparator(rcleft);

    /* autoscale */
    bt = CreateBitmapButton(rcleft, 16, 16, auto_bits);
    AddButtonCB(bt, autoscale_xy_cb, (void *) grace);
    bt = CreateBitmapButton(rcleft, 16, 16, auto_x_bits);
    AddButtonCB(bt, autoscale_x_cb, (void *) grace);
    bt = CreateBitmapButton(rcleft, 16, 16, auto_y_bits);
    AddButtonCB(bt, autoscale_y_cb, (void *) grace);
    bt = CreateBitmapButton(rcleft, 16, 16, auto_tick_bits);
    AddButtonCB(bt, autoticks_cb, NULL);

    CreateSeparator(rcleft);

    /* scrolling buttons */
    bt = CreateBitmapButton(rcleft, 16, 16, left_bits);
    AddButtonCB(bt, graph_scroll_left_cb, (void *) grace);
    bt = CreateBitmapButton(rcleft, 16, 16, right_bits);
    AddButtonCB(bt, graph_scroll_right_cb, (void *) grace);
    bt = CreateBitmapButton(rcleft, 16, 16, up_bits);
    AddButtonCB(bt, graph_scroll_up_cb, (void *) grace);
    bt = CreateBitmapButton(rcleft, 16, 16, down_bits);
    AddButtonCB(bt, graph_scroll_down_cb, (void *) grace);

    CreateSeparator(rcleft);

    /* expand/shrink */
    bt = CreateBitmapButton(rcleft, 16, 16, expand_bits);
    AddButtonCB(bt, graph_zoom_in_cb, (void *) grace);
    bt = CreateBitmapButton(rcleft, 16, 16, shrink_bits);
    AddButtonCB(bt, graph_zoom_out_cb, (void *) grace);

    CreateSeparator(rcleft);

    bt = CreateBitmapButton(rcleft, 16, 16, atext_bits);
    AddButtonCB(bt, atext_add_proc, (void *) grace);

    CreateSeparator(rcleft);
    CreateSeparator(rcleft);

    /* exit */
    bt = CreateBitmapButton(rcleft, 16, 16, exit_bits);
    AddButtonCB(bt, exit_cb, grace);

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
        grace_icon_xpm, &icon, &shape, NULL);
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
    if (quark_fid_get(q) == QFlavorGraph) {
        int *type = (int *) udata;
        closure->descend = FALSE;
        graph_scroll(q, *type);
    }

    return TRUE;
}

static void graph_scroll_proc(Grace *grace, int type)
{
    Quark *cg, *f;
    
    cg = graph_get_current(grace->project);
    f = get_parent_frame(cg);
    
    quark_traverse(f, scroll_hook, &type);
    
    snapshot_and_update(grace->project, TRUE);
}

static void graph_scroll_left_cb(Widget but, void *data)
{
    graph_scroll_proc((Grace *) data, GSCROLL_LEFT);
}

static void graph_scroll_right_cb(Widget but, void *data)
{
    graph_scroll_proc((Grace *) data, GSCROLL_RIGHT);
}

static void graph_scroll_up_cb(Widget but, void *data)
{
    graph_scroll_proc((Grace *) data, GSCROLL_UP);
}

static void graph_scroll_down_cb(Widget but, void *data)
{
    graph_scroll_proc((Grace *) data, GSCROLL_DOWN);
}

static int zoom_hook(Quark *q, void *udata, QTraverseClosure *closure)
{
    if (quark_fid_get(q) == QFlavorGraph) {
        int *type = (int *) udata;
        closure->descend = FALSE;
        graph_zoom(q, *type);
    }

    return TRUE;
}

static void graph_zoom_proc(Grace *grace, int type)
{
    Quark *cg, *f;
    
    cg = graph_get_current(grace->project);
    f = get_parent_frame(cg);
    
    quark_traverse(f, zoom_hook, &type);
    
    snapshot_and_update(grace->project, TRUE);
}

static void graph_zoom_in_cb(Widget but, void *data)
{
    graph_zoom_proc((Grace *) data, GZOOM_EXPAND);
}

static void graph_zoom_out_cb(Widget but, void *data)
{
    graph_zoom_proc((Grace *) data, GZOOM_SHRINK);
}

static void load_example_cb(Widget but, void *data)
{
    char *s, buf[128];
    
    set_wait_cursor();
    
    s = (char *) data;
    sprintf(buf, "examples/%s", s);
    load_project_file(grace, buf, FALSE);

    xdrawgraph(grace->project, FALSE);

    unset_wait_cursor();
}
