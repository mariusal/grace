/*
 * Grace - Graphics for Exploratory Data Analysis
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-95 Paul J Turner, Portland, OR
 * Copyright (c) 1996-98 GRACE Development Team
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

#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/keysym.h>
#include <X11/StringDefs.h>

#ifdef WITH_EDITRES
#  include <X11/Xmu/Editres.h>
#endif

#include <Xm/Xm.h>
#include <Xm/ArrowB.h>
#include <Xm/CascadeB.h>
#include <Xm/DialogS.h>
#include <Xm/DrawingA.h>
#include <Xm/BulletinB.h>
#include <Xm/FileSB.h>
#include <Xm/Frame.h>
#include <Xm/Form.h>
#include <Xm/MainW.h>
#include <Xm/MessageB.h>
#include <Xm/Protocols.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/Label.h>
#include <Xm/RowColumn.h>
#include <Xm/SelectioB.h>
#include <Xm/ToggleB.h>
#include <Xm/Separator.h>
#include <Xm/ScrolledW.h>
#if XmVersion >= 1002
#  include <Xm/RepType.h>
#endif

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
#include "device.h"
#include "x11drv.h"
#include "graphs.h"
#include "graphutils.h"
#include "plotone.h"
#include "protos.h"

#include "motifinc.h"

/*
 * used to set up XmStrings
 * Seems to be some problems under AIX, the #ifdef is supposed to
 * take care of the problem.
 */
#ifdef XmFONTLIST_DEFAULT_TAG
XmStringCharSet charset = (XmStringCharSet) XmFONTLIST_DEFAULT_TAG;
#else
XmStringCharSet charset = (XmStringCharSet) XmSTRING_DEFAULT_CHARSET;
#endif


/* used globally */
XtAppContext app_con;
Widget app_shell;

static Widget canvas;

static Widget drawing_window;		/* container for drawing area */


Widget loclab;			/* locator label */
Widget statlab;			/* status line at the bottom */
Widget stack_depth_item;	/* stack depth item on the main panel */
Widget curw_item;		/* current world stack item on the main panel */
XmString sdstring;		/* string for stack depth */
XmString cystring;		/* string for stack cycle */
XmString statstring;		/* string for pointer status */

Display *disp;
Window xwin;

extern Window root;
extern GC gc;
extern int screennumber;
extern int depth;
extern Colormap cmap;
extern unsigned long xvlibcolors[];

/* used locally */
static Widget main_frame;
static Widget menu_bar;
static Widget frleft, frtop, frbot;	/* dialogs along canvas edge */
static Widget form;		/* form for mainwindow */

static void MenuCB(Widget w, XtPointer client_data, XtPointer call_data);
static Widget CreateMainMenuBar(Widget parent);
static void init_pm(Pixel fg, Pixel bg);

extern int action_flag;

/*
 * for buttons on front panel
 */

static Pixmap zoompm, shrinkpm, expandpm, autopm;
static Pixmap uppm, leftpm, downpm, rightpm;

static int toolbar_visible = 1;
static int statusbar_visible = 1;
static int locbar_visible = 1;

static Widget windowbarw[3];

/* action routines */
void autoscale( Widget, XKeyEvent *, String *, Cardinal * );
void autoscale_on_near( Widget, XKeyEvent *, String *, Cardinal * );
void draw_box_action( Widget, XKeyEvent *, String *, Cardinal * );
void delete_object( Widget, XKeyEvent *, String *, Cardinal * );
void place_legend( Widget, XKeyEvent *, String *, Cardinal * );
void move_object( Widget, XKeyEvent *, String *, Cardinal * );
void draw_line_action( Widget, XKeyEvent *, String *, Cardinal * );
void refresh_hotlink( Widget, XKeyEvent *, String *, Cardinal * );
void set_viewport( Widget, XKeyEvent *, String *, Cardinal * );
void write_string( Widget, XKeyEvent *, String *, Cardinal * );
void exit_abruptly( Widget, XKeyEvent *, String *, Cardinal * );
void enable_zoom( Widget, XKeyEvent *, String *, Cardinal * );
static void graph_scroll_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void graph_zoom_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void world_stack_proc(Widget w, XtPointer client_data, XtPointer call_data);

#define WSTACK_PUSH         0
#define WSTACK_POP          1
#define WSTACK_CYCLE        2
#define WSTACK_PUSH_ZOOM    3

/*
 * establish action routines
 */
static XtActionsRec canvas_actions[] = {
	{ "autoscale", (XtActionProc) autoscale },	
	{ "autoscale_on_near", (XtActionProc) autoscale_on_near },	
	{ "draw_box_action", (XtActionProc) draw_box_action },	
	{ "delete_object", (XtActionProc) delete_object },	
	{ "place_legend", (XtActionProc) place_legend },	
	{ "move_object", (XtActionProc) move_object },	
	{ "draw_line_action", (XtActionProc) draw_line_action },	
	{ "refresh_hotlink", (XtActionProc) refresh_hotlink },
	{ "set_viewport", (XtActionProc) set_viewport },	
	{ "write_string", (XtActionProc) write_string },	
	{ "exit_abruptly", (XtActionProc) exit_abruptly },	
	{ "enable_zoom", (XtActionProc) enable_zoom }
};

static XtActionsRec graph_select_actions[] = {
    {"list_choice_selectall",   list_choice_selectall},
    {"list_choice_unselectall", list_choice_unselectall}
/*
 *     {"graph_choice_delete", (XtActionProc) graph_choice_delete}
 */
};

static char canvas_table[] = "#override\n\
	Ctrl <Key>A: autoscale()\n\
	Ctrl <Key>B: draw_box_action()\n\
	Ctrl <Key>D: delete_object()\n\
	Ctrl <Key>L: place_legend()\n\
	Ctrl <Key>N: move_object()\n\
	Ctrl <Key>P: draw_line_action()\n\
	Ctrl <Key>U: refresh_hotlink()\n\
	Ctrl <Key>V: set_viewport()\n\
	Ctrl <Key>W: write_string()\n\
	Ctrl <Key>X: exit_abruptly()\n\
	Ctrl <Key>Z: enable_zoom()";

/*
 * establish resource stuff
 */
typedef struct {
    Boolean invert;
    Boolean allow_dc;
    Boolean verify_action;
    Boolean auto_redraw;
    Boolean logwindow;
    Boolean toolbar;
    Boolean statusbar;
    Boolean locatorbar;
}
ApplicationData, *ApplicationDataPtr;

static XtResource resources[] =
{
    {"invertDraw", "InvertDraw", XtRBoolean, sizeof(Boolean),
     XtOffset(ApplicationDataPtr, invert), XtRImmediate,
     (XtPointer) FALSE},
    {"allowDoubleClick", "AllowDoubleClick", XtRBoolean, sizeof(Boolean),
     XtOffset(ApplicationDataPtr, allow_dc), XtRImmediate,
     (XtPointer) TRUE},
    {"verifyAction", "VerifyAction", XtRBoolean, sizeof(Boolean),
     XtOffset(ApplicationDataPtr, verify_action), XtRImmediate,
     (XtPointer) FALSE},
    {"allowRedraw", "AllowRedraw", XtRBoolean, sizeof(Boolean),
     XtOffset(ApplicationDataPtr, auto_redraw), XtRImmediate,
     (XtPointer) TRUE},
    {"logWindow", "LogWindow", XtRBoolean, sizeof(Boolean),
     XtOffset(ApplicationDataPtr, logwindow), XtRImmediate,
     (XtPointer) FALSE},
    {"toolBar", "ToolBar", XtRBoolean, sizeof(Boolean),
     XtOffset(ApplicationDataPtr, toolbar), XtRImmediate,
     (XtPointer) TRUE},
    {"statusBar", "StatusBar", XtRBoolean, sizeof(Boolean),
     XtOffset(ApplicationDataPtr, statusbar), XtRImmediate,
     (XtPointer) TRUE},
    {"locatorBar", "LocatorBar", XtRBoolean, sizeof(Boolean),
     XtOffset(ApplicationDataPtr, locatorbar), XtRImmediate,
     (XtPointer) TRUE}
};

String fallbackResources[] = {
    "XMgrace*fontList:-adobe-helvetica-bold-r-normal-*-14-*-*-*-*-*-*-*",
    "XMgrace*tabFontList:-adobe-helvetica-bold-r-normal-*-14-*-*-*-*-*-*-*",
    "XMgrace*background: #e5e5e5",
    "XMgrace*foreground: #000000",
    "XMgrace*XmPushButton.background: #b0c4de",
    "XMgrace*XmMenuShell*XmPushButton.background: #e5e5e5",
    "XMgrace*XmText*background: #cfe7e7",
    "XMgrace*XmToggleButton.selectColor: #ff0000",
    "*XmToggleButton.fillOnSelect: true",
    "*XmSeparator.margin: 0",
#if ((XmVersion >= 1002) || defined (LesstifVersion) && (LesstifVersion >= 1000))
/*
 * Lesstif-0.80 sometimes crashes with tear-off menus; let's hope version 1.0
 * will fix it :-)
 */
    "*fileMenu.tearOffModel: XmTEAR_OFF_ENABLED",
    "*readMenu.tearOffModel: XmTEAR_OFF_ENABLED",
    "*writeMenu.tearOffModel: XmTEAR_OFF_ENABLED",
    "*dataMenu.tearOffModel: XmTEAR_OFF_ENABLED",
    "*transformationsMenu.tearOffModel: XmTEAR_OFF_ENABLED",
    "*setOperationsMenu.tearOffModel: XmTEAR_OFF_ENABLED",
    "*regionOperationsMenu.tearOffModel: XmTEAR_OFF_ENABLED",
    "*graphOperationsMenu.tearOffModel: XmTEAR_OFF_ENABLED",
    "*plotMenu.tearOffModel: XmTEAR_OFF_ENABLED",
    "*optionsMenu.tearOffModel: XmTEAR_OFF_ENABLED",
    "*pageMenu.tearOffModel: XmTEAR_OFF_ENABLED",
    "*viewMenu.tearOffModel: XmTEAR_OFF_ENABLED",
    "*helpMenu.tearOffModel: XmTEAR_OFF_ENABLED",
#endif
    "*dragInitiatorProtocolStyle: XmDRAG_NONE",
    "*dragReceiverProtocolStyle:  XmDRAG_NONE",
    "*fileMenu.open.acceleratorText: Ctrl+O",
    "*fileMenu.open.accelerator: Ctrl<Key>o",
    "*fileMenu.save.acceleratorText: Ctrl+S",
    "*fileMenu.save.accelerator: Ctrl<Key>s",
    "*fileMenu.exit.acceleratorText: Ctrl+Q",
    "*fileMenu.exit.accelerator: Ctrl<Key>q",
    "*fileMenu.print.acceleratorText: Ctrl+P",
    "*fileMenu.print.accelerator: Ctrl<Key>p",
    "*helpMenu.onContext.acceleratorText: Shift+F1",
    "*helpMenu.onContext.accelerator: Shift<Key>F1",
    NULL
};

/*
 * main menubar
 */
/* #define MENU_HELP	200 */
#define MENU_EXIT	201
#define MENU_CLEAR	202
/* #define MENU_NEW	203 */
#define MENU_OPEN	204
#define MENU_SAVE	205
#define MENU_SAVEAS	206
#define MENU_PRINT	207


void xlibprocess_args(int *argc, char **argv)
{
    ApplicationData rd;

    app_shell = XtVaAppInitialize(&app_con, "XMgrace", NULL, 0, argc, argv, 
    	fallbackResources, NULL);
    
    XtGetApplicationResources(app_shell, &rd, resources,
  			    XtNumber(resources), NULL, 0);
    
    invert = rd.invert;
    allow_dc = rd.allow_dc;
    verify_action = rd.verify_action;
    logwindow = rd.logwindow;
    auto_redraw = rd.auto_redraw;
    toolbar_visible = rd.toolbar;
    statusbar_visible = rd.statusbar;
    locbar_visible = rd.locatorbar;

    XtAppAddActions(app_con, canvas_actions, XtNumber(canvas_actions));
    XtAppAddActions(app_con, graph_select_actions, XtNumber(graph_select_actions));
}

static void do_drawgraph(Widget w, XtPointer client_data, XtPointer call_data)
{
    drawgraph();
}


static void MenuCB(Widget w, XtPointer client_data, XtPointer call_data)
{
    switch ((int) client_data) {
    case MENU_EXIT:
	bailout();
	break;
    case MENU_CLEAR:
	wipeout();
	set_graph_active(0, TRUE);
	switch_current_graph(0);
	set_title(mybasename(docname));
	update_all();
	drawgraph();
	break;
    case MENU_OPEN:
	create_openproject_popup();
	break;
    case MENU_SAVE:
	if (strcmp (docname, NONAME) != 0) {
	    set_wait_cursor();
	    
	    if (save_project(docname) == GRACE_EXIT_SUCCESS) {
	        clear_dirtystate();
	    }
	    
	    unset_wait_cursor();
	} else {
	    create_saveproject_popup();
	}
	break;
    case MENU_SAVEAS:
	create_saveproject_popup();
	break;
    case MENU_PRINT:
	set_wait_cursor();
	do_hardcopy();
	unset_wait_cursor();
	break;
    default:
	break;
    }
}

/*
 * service the autoscale buttons on the main panel
 */
static void autoscale_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int cg = get_cg();
    
    if (activeset(cg)) {
	autoscale_graph(cg, (int) client_data);
	drawgraph();
    } else {
	errwin("No active sets!");
    }
}

void autoon_proc(void)
{
    set_action(0);
    set_action(AUTO_NEAREST);
}

/*
 * service the autoticks button on the main panel
 */
void autoticks_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    autotick_axis(get_cg(), ALL_AXES);
    update_ticks(get_cg());
    drawgraph();
}

/*
 * set the message in the left footer
 */
void set_left_footer(char *s)
{
    Arg al;

    XmStringFree(statstring);
    statstring = XmStringCreateLtoR(s, charset);
    XtSetArg(al, XmNlabelString, statstring);
    XtSetValues(statlab, &al, 1);
    if (logwindow) {
	log_results(s);
    }
    XmUpdateDisplay(statlab);
}


/*
 * clear the locator reference point
 */
void do_clear_point(Widget w, XtPointer client_data, XtPointer call_data)
{
    GLocator locator;
    
    get_graph_locator(get_cg(), &locator);
    locator.pointset = FALSE;
    set_graph_locator(get_cg(), &locator);
/*
 *     g[cg].pt_type = 0;
 *     g[cg].dsx = g[cg].dsy = 0.0;
 */
}

/*
 * set visibility of the toolbars
 */
static void set_view_items(void)
{
    if (statusbar_visible) {
	XmToggleButtonSetState(windowbarw[1], True, False);
	XtManageChild(frbot);
	XtVaSetValues(drawing_window,
		      XmNbottomAttachment, XmATTACH_WIDGET,
		      XmNbottomWidget, frbot,
		      NULL);
	if (toolbar_visible) {
	    XtVaSetValues(frleft,
			  XmNbottomAttachment, XmATTACH_WIDGET,
			  XmNbottomWidget, frbot,
			  NULL);
	}
    } else {
	XmToggleButtonSetState(windowbarw[1], False, False);
	XtVaSetValues(drawing_window,
		      XmNbottomAttachment, XmATTACH_FORM,
		      NULL);
	XtUnmanageChild(frbot);
	if (toolbar_visible) {
	    XtVaSetValues(frleft,
			  XmNbottomAttachment, XmATTACH_FORM,
			  NULL);
	}
    }
    if (toolbar_visible) {
	XmToggleButtonSetState(windowbarw[2], True, False);
	XtManageChild(frleft);
	if (statusbar_visible) {
	    XtVaSetValues(frleft,
			  XmNbottomAttachment, XmATTACH_WIDGET,
			  XmNbottomWidget, frbot,
			  NULL);
	}
	if (locbar_visible) {
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
	XmToggleButtonSetState(windowbarw[2], False, False);
	XtUnmanageChild(frleft);
	XtVaSetValues(drawing_window,
		      XmNleftAttachment, XmATTACH_FORM,
		      NULL);
    }
    if (locbar_visible) {
	XmToggleButtonSetState(windowbarw[0], True, False);
	XtManageChild(frtop);
	XtVaSetValues(drawing_window,
		      XmNtopAttachment, XmATTACH_WIDGET,
		      XmNtopWidget, frtop,
		      NULL);
	if (toolbar_visible) {
	    XtVaSetValues(frleft,
			  XmNtopAttachment, XmATTACH_WIDGET,
			  XmNtopWidget, frtop,
			  NULL);
	}
    } else {
	XmToggleButtonSetState(windowbarw[0], False, False);
	XtUnmanageChild(frtop);
	XtVaSetValues(drawing_window,
		      XmNtopAttachment, XmATTACH_FORM,
		      NULL);
	if (toolbar_visible) {
	    XtVaSetValues(frleft,
			  XmNtopAttachment, XmATTACH_FORM,
			  NULL);
	}
    }
}

/*
 * service routines for the View pulldown
 */
void set_statusbar(Widget w, XtPointer client_data, XtPointer call_data)
{
    if (XmToggleButtonGetState(w)) {
	statusbar_visible = 1;
    } else {
	statusbar_visible = 0;
    }
    set_view_items();
}

void set_toolbar(Widget w, XtPointer client_data, XtPointer call_data)
{
    if (XmToggleButtonGetState(w)) {
	toolbar_visible = 1;
    } else {
	toolbar_visible = 0;
    }
    set_view_items();
}

void set_locbar(Widget w, XtPointer client_data, XtPointer call_data)
{
    if (XmToggleButtonGetState(w)) {
	locbar_visible = 1;
    } else {
	locbar_visible = 0;
    }
    set_view_items();
}

/*
 * create the main menubar
 */
static Widget CreateMainMenuBar(Widget parent)
{
    Widget menubar;
    Widget menupane, submenupane;

    Widget cascade;

    menubar = CreateMenuBar(parent, "menuBar", "main.html#menubar");

/*
 * File menu
 */
    menupane = CreateMenu(menubar, "fileMenu", "File", 'F', NULL, NULL);

    CreateMenuButton(menupane, "open", "Open...", 'O',
    	(XtCallbackProc) MenuCB, (XtPointer) MENU_OPEN, "file.html#open");

    CreateMenuButton(menupane, "save", "Save", 'S',
    	(XtCallbackProc) MenuCB, (XtPointer) MENU_SAVE, "file.html#save");

    CreateMenuButton(menupane, "saveAs", "Save as...", 'a',
    	(XtCallbackProc) MenuCB, (XtPointer) MENU_SAVEAS, "file.html#saveas");

    CreateMenuButton(menupane, "describe", "Describe...", 'D',
    	(XtCallbackProc) create_describe_popup, NULL, "file.html#describe");

    CreateMenuSeparator(menupane);

/*
 * Read submenu
 */

    submenupane = CreateMenu(menupane, "readMenu", "Read", 'R', NULL, NULL);

    CreateMenuButton(submenupane, "sets", "Sets...", 'S',
    	(XtCallbackProc) create_file_popup, (XtPointer) NULL, "file.html#readsets");

#ifdef HAVE_MFHDF
    CreateMenuButton(submenupane, "netCDF", "NetCDF/HDF...", 'N',
    	(XtCallbackProc) create_netcdfs_popup, (XtPointer) NULL, "file.html#readnetcdf");
#else

#ifdef HAVE_NETCDF
    CreateMenuButton(submenupane, "netCDF", "NetCDF...", 'N', 
    	(XtCallbackProc) create_netcdfs_popup, (XtPointer) NULL, "file.html#readnetcdf");
#endif

#endif
    CreateMenuButton(submenupane, "parameters", "Parameters...", 'P',
    	(XtCallbackProc) create_rparams_popup, (XtPointer) NULL, "file.html#readpars");

    CreateMenuButton(submenupane, "blockData", "Block data...", 'B',
    	(XtCallbackProc) create_block_popup, (XtPointer) NULL, "file.html#readblock");

/*
 *     CreateMenuButton(submenupane, "image", "Image...", 'I',
 *     	(XtCallbackProc) create_image_frame, (XtPointer) NULL, 0);
 */
   
/*
 * Write submenu
 */  
    submenupane = CreateMenu(menupane, "writeMenu", "Write", 'W', NULL, NULL);

    CreateMenuButton(submenupane, "sets", "Sets...", 'S',
    	(XtCallbackProc) create_write_popup, (XtPointer) NULL, "file.html#writesets");

    CreateMenuButton(submenupane, "parameters", "Parameters...", 'P', 
    	(XtCallbackProc) create_wparam_frame, (XtPointer) NULL, "file.html#writeparams");


    CreateMenuSeparator(menupane);

    CreateMenuButton(menupane, "clearAll", "Clear all", 'C',
    	(XtCallbackProc) MenuCB, (XtPointer) MENU_CLEAR, "file.html#clearall");

    CreateMenuSeparator(menupane);

    CreateMenuButton(menupane, "print", "Print", 'P',
    	(XtCallbackProc) MenuCB, (XtPointer) MENU_PRINT, "file.html#print");

    CreateMenuButton(menupane, "deviceSetup", "Device setup...", 't',
    	(XtCallbackProc) create_printer_setup, (XtPointer) NULL, "file.html#printersetup");


    CreateMenuSeparator(menupane);

    CreateMenuButton(menupane, "exit", "Exit", 'x',
    	(XtCallbackProc) MenuCB, (XtPointer) MENU_EXIT, "file.html#exit");


/*
 * Data menu
 */
    menupane = CreateMenu(menubar, "dataMenu", "Data", 'D', NULL, NULL);

    CreateMenuButton(menupane, "results", "Results...", 'R',
    	(XtCallbackProc) create_monitor_frame, (XtPointer) NULL, 0);
    
    CreateMenuButton(menupane, "commands", "Commands...", 'C',
    	(XtCallbackProc) open_command, (XtPointer) NULL, 0);

    CreateMenuSeparator(menupane);


    CreateMenuButton(menupane, "dataSetProperties", "Data set properties...", 'o',
    	    (XtCallbackProc) create_change_popup, (XtPointer) NULL, 0);

    submenupane = CreateMenu(menupane, "dataSetOperationsMenu", "Data set operations", 't', NULL, NULL);

    CreateMenuButton(submenupane, "Sort", "Sort...", 'o',
    	    (XtCallbackProc) create_sort_popup, (XtPointer) NULL, 0);
    CreateMenuButton(submenupane, "reverse", "Reverse...", 'v',
    	    (XtCallbackProc) create_reverse_popup, (XtPointer) NULL, 0);

    CreateMenuSeparator(submenupane);

    CreateMenuButton(submenupane, "dropPoints", "Drop points...", 'n',
    	    (XtCallbackProc) create_drop_popup, (XtPointer) NULL, 0);
    CreateMenuButton(submenupane, "pruneData", "Prune data...", 'P',
    	    (XtCallbackProc) create_prune_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "geometricTransforms", "Geometric transforms...", 'G',
    	    (XtCallbackProc) create_geom_frame, (XtPointer) NULL, 0);
    	    
    submenupane = CreateMenu(menupane, "transformationsMenu", "Transformations", 'T', NULL, NULL);

    CreateMenuButton(submenupane, "join", "Join...", 'J',
    	    (XtCallbackProc) create_join_popup, (XtPointer) NULL, 0);
    CreateMenuButton(submenupane, "split", "Split...", 'S',
    	    (XtCallbackProc) create_split_popup, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "evaluateExpression", "Evaluate expression...", 'E',
    	    (XtCallbackProc) create_eval_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "histograms", "Histograms...", 'H',
    	    (XtCallbackProc) create_histo_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "fourierTransforms", "Fourier transforms...", 'u',
    	    (XtCallbackProc) create_fourier_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "regression", "Regression...", 'R',
    	    (XtCallbackProc) create_reg_frame, (XtPointer) NULL, 0);
    CreateMenuButton(submenupane, "nonLinearFit", "Non-linear curve fitting...", 'N',
    	    (XtCallbackProc) create_nonl_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "runningAverages", "Running averages...", 'a',
    	    (XtCallbackProc) create_run_frame, (XtPointer) NULL, 0);
    CreateMenuButton(submenupane, "differences", "Differences...", 'D',
    	    (XtCallbackProc) create_diff_frame, (XtPointer) NULL, 0);
    CreateMenuButton(submenupane, "seasonalDifferences", "Seasonal differences...", 'o',
    	    (XtCallbackProc) create_seasonal_frame, (XtPointer) NULL, 0);
    CreateMenuButton(submenupane, "integration", "Integration...", 'I',
    	    (XtCallbackProc) create_int_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "correlation", "Cross/auto correlation...", 'C',
    	    (XtCallbackProc) create_xcor_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "interpolation", "Interpolation...", 't',
    	    (XtCallbackProc) create_interp_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "splines", "Splines...", 'S',
    	    (XtCallbackProc) create_spline_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "samplePoints", "Sample points...", 'm',
    	    (XtCallbackProc) create_samp_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "digitalFilter", "Digital filter...", 'f',
    	    (XtCallbackProc) create_digf_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "linearConvolution", "Linear convolution...", 'v',
    	    (XtCallbackProc) create_lconv_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "featureExtraction", "Feature extraction...", 'x',
    	    (XtCallbackProc) create_featext_frame, (XtPointer) NULL, 0);

    CreateMenuSeparator(menupane);

    CreateMenuButton(menupane, "pointOperations", "Point operations...", 'P',
    	(XtCallbackProc) create_points_frame, (XtPointer) NULL, 0);


    submenupane = CreateMenu(menupane, "regionOperationsMenu", 
    				"Region operations", 'i', NULL, NULL);

    CreateMenuButton(submenupane, "status", "Status...", 'S',
    	(XtCallbackProc) define_status_popup, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "define", "Define...", 'D',
    	    (XtCallbackProc) create_define_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "clear", "Clear...", 'C',
    	    (XtCallbackProc) create_clear_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "evaluate", "Evaluate...", 'E',
    	    (XtCallbackProc) create_evalregion_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "extractPoints", "Extract points...", 'p',
    	    (XtCallbackProc) create_extract_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "extractSets", "Extract sets...", 's',
    	    (XtCallbackProc) create_extractsets_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "deletePoints", "Delete points...", 'o',
    	    (XtCallbackProc) create_delete_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "killSets", "Kill sets...", 'K',
    	    (XtCallbackProc) create_deletesets_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "reportOn", "Report on...", 'R',
    	    (XtCallbackProc) create_reporton_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "areaPerimeter", "Area/perimeter...", 'A',
    	    (XtCallbackProc) create_area_frame, (XtPointer) NULL, 0);


    CreateMenuButton(menupane, "hotLinks", "Hot links...", 'l',
    	(XtCallbackProc) create_hotlinks_popup, (XtPointer) NULL, 0);



/* Plot menu */
    menupane = CreateMenu(menubar, "plotMenu", "Plot", 'P', NULL, NULL);

    CreateMenuButton(menupane, "plotAppearance", "Plot appearance...", 'p',
    	(XtCallbackProc) create_plot_frame, (XtPointer) NULL, 0);

    CreateMenuSeparator(menupane);

    CreateMenuButton(menupane, "graphAppearance", "Graph appearance...", 'G',
    	(XtCallbackProc) create_graphapp_frame, (XtPointer) NULL, 0);
    CreateMenuButton(menupane, "arrangeGraphs", "Arrange graphs...", 'r',
    	    (XtCallbackProc) create_arrange_frame, (XtPointer) NULL, 0);
    CreateMenuButton(menupane, "overlayGraphs", "Overlay graphs...", 'O',
    	    (XtCallbackProc) create_overlay_frame, (XtPointer) NULL, 0);
    CreateMenuButton(menupane, "autoscale", "Autoscale graphs...", 'A',
    	(XtCallbackProc) create_autos_frame, (XtPointer) NULL, 0);

    CreateMenuSeparator(menupane);

    CreateMenuButton(menupane, "setAppearance", "Set appearance...", 'S',
    	(XtCallbackProc) define_symbols_popup, (XtPointer) -1, 0);
    CreateMenuButton(menupane, "setOperations", "Set operations...", 'o',
    	    (XtCallbackProc) create_swap_popup, (XtPointer) NULL, 0);

    CreateMenuSeparator(menupane);

    CreateMenuButton(menupane, "axisProperties", "Axis properties...", 'x',
    	(XtCallbackProc) create_axes_dialog, (XtPointer) NULL, 0);

    CreateMenuSeparator(menupane);

    CreateMenuButton(menupane, "drawingObjects", "Drawing objects...", 'o',
    	(XtCallbackProc) define_objects_popup, (XtPointer) NULL, 0);

    CreateMenuSeparator(menupane);

    CreateMenuButton(menupane, "fontTool", "Font tool", 'F',
       (XtCallbackProc) create_fonttool, (XtPointer) NULL, 0);

/* Options menu */
    menupane = CreateMenu(menubar, "optionsMenu", "Options", 'O', NULL, NULL);
   

    submenupane = CreateMenu(menupane, "viewMenu", "View", 'V', NULL, NULL);

    windowbarw[0] = CreateMenuToggle(submenupane, "locatorBar", "Locator bar", 'L',
	    (XtCallbackProc) set_locbar, (XtPointer) &frtop, NULL);
    windowbarw[1] = CreateMenuToggle(submenupane, "statusBar", "Status bar", 'S',
	    (XtCallbackProc) set_statusbar, (XtPointer) &frbot, NULL);
    windowbarw[2] = CreateMenuToggle(submenupane, "toolBar", "Tool bar", 'T',
	    (XtCallbackProc) set_toolbar, (XtPointer) &frleft, NULL);

    CreateMenuSeparator(submenupane);

    CreateMenuButton(submenupane, "setLocatorFixedPoint", "Set locator fixed point", 'f',
    	(XtCallbackProc) set_actioncb, (XtPointer) SEL_POINT, 0);

    CreateMenuButton(submenupane, "clearLocatorFixedPoint", "Clear locator fixed point", 'C',
    	(XtCallbackProc) do_clear_point, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "locatorProps", "Locator props...", 'p',
    	(XtCallbackProc) create_locator_frame, (XtPointer) NULL, 0);
    
    CreateMenuButton(menupane, "workingDirectory", "Working directory...", 'W',
    	(XtCallbackProc) create_workingdir_popup, (XtPointer) NULL, 0);

    CreateMenuButton(menupane, "misc", "Misc...", 'M',
    	(XtCallbackProc) create_props_frame, (XtPointer) NULL, 0);


/* help menu */

    menupane = CreateMenu(menubar, "helpMenu", "Help", 'H', &cascade, NULL);
    XtVaSetValues(menubar, XmNmenuHelpWidget, cascade, NULL);

    CreateMenuButton(menupane, "onContext", "On context", 'x',
    	(XtCallbackProc) ContextHelpCB, (XtPointer) NULL, 0);

    CreateMenuButton(menupane, "migrationGuide", "Migration Guide", 'M',
    	(XtCallbackProc) HelpCB, (XtPointer) "MIGRATION.html", 0);

    CreateMenuButton(menupane, "userGiude", "User Guide", 'G',
    	(XtCallbackProc) HelpCB, (XtPointer) "guide.html", 0);
    
    CreateMenuButton(menupane, "faq", "FAQ", 'Q',
    	(XtCallbackProc) HelpCB, (XtPointer) "FAQ.html", 0);

    CreateMenuButton(menupane, "changes", "Changes", 'C',
    	(XtCallbackProc) HelpCB, (XtPointer) "CHANGES.html", 0);

    CreateMenuButton(menupane, "comments", "Comments", 'm',
    	(XtCallbackProc) HelpCB, (XtPointer) "http://plasma-gate.weizmann.ac.il/Grace/comments.html", 0);
   	    
    CreateMenuSeparator(menupane);

    CreateMenuButton(menupane, "licenseTerms", "License terms", 'L',
    	(XtCallbackProc) HelpCB, (XtPointer) "GPL.html", 0);

    CreateMenuButton(menupane, "about", "About...", 'A',
    	(XtCallbackProc) create_about_grtool, (XtPointer) NULL, 0);


    return (menubar);
}


/*
 * initialize the GUI
 */
void initialize_screen()
{
    Widget bt, rc3, rcleft, rctop, formbot;
    Pixmap icon, shape;
    Atom WM_DELETE_WINDOW;
    Pixel fg, bg;
    XmString string;		/* string for current location */
    char buf[256];

/* 
 * Allow users to change tear off menus with X resources
 */
#if (XmVersion >= 1002)
    XmRepTypeInstallTearOffModelConverter();
#endif

#ifdef WITH_EDITRES    
    XtAddEventHandler(app_shell, (EventMask) 0, True,
    			_XEditResCheckMessages, NULL);
#endif

/*
 *     XtAddEventHandler(app_shell, StructureNotifyMask, False,
 *         		     (XtEventHandler) resize, NULL);
 */

    savewidget(app_shell);
    
    disp = XtDisplay(app_shell);
    if (disp == NULL) {
	errmsg("xmgrace: can't open display, exiting...");
	exit(1);
    }
    
    xlibinit();

/*
 * We handle important WM events ourselves
 */
    WM_DELETE_WINDOW = XmInternAtom(disp, "WM_DELETE_WINDOW", False);
    XmAddWMProtocolCallback(app_shell, WM_DELETE_WINDOW, 
    	(XtCallbackProc) MenuCB, (XtPointer) MENU_EXIT);
    XtVaSetValues(app_shell, XmNdeleteResponse, XmDO_NOTHING, NULL);
    
/*
 * build the UI here
 */
    main_frame = XtVaCreateManagedWidget("main", xmMainWindowWidgetClass, app_shell,
					 XmNshadowThickness, 0,
					 XmNwidth, 680,
					 XmNheight, 700,
					 NULL);

    XtAddCallback(main_frame, XmNhelpCallback, (XtCallbackProc) HelpCB, (XtPointer) "index.html");

    menu_bar = CreateMainMenuBar(main_frame);
    XtManageChild(menu_bar);

    form = XmCreateForm(main_frame, "form", NULL, 0);

    frleft = XtVaCreateManagedWidget("fr", xmFrameWidgetClass, form,
				     NULL);
    rcleft = XtVaCreateManagedWidget("toolBar", xmRowColumnWidgetClass, frleft,
				     XmNorientation, XmVERTICAL,
				     XmNpacking, XmPACK_TIGHT,
				     XmNspacing, 0,
				     XmNentryBorder, 0,
				     XmNmarginWidth, 0,
				     XmNmarginHeight, 0,
				     NULL);
    XtAddCallback(rcleft, XmNhelpCallback, (XtCallbackProc) HelpCB, (XtPointer) "main.html#toolbar");

    frtop = XtVaCreateManagedWidget("frtop", xmFrameWidgetClass, form,
				    NULL);
    rctop = XtVaCreateManagedWidget("rctop", xmRowColumnWidgetClass, frtop,
				    XmNorientation, XmHORIZONTAL,
				    XmNpacking, XmPACK_TIGHT,
				    XmNspacing, 0,
				    XmNentryBorder, 0,
				    XmNmarginWidth, 0,
				    XmNmarginHeight, 0,
				    NULL);

    frbot = XtVaCreateManagedWidget("frbot", xmFrameWidgetClass, form, NULL);
    XtManageChild(frbot);
    /* formbot = XmCreateForm(frbot, "form", NULL, 0); */
    formbot = XmCreateRowColumn(frbot, "rc", NULL, 0);
    set_default_message(buf);
    statstring = XmStringCreateLtoR(buf, charset);
    statlab = XtVaCreateManagedWidget("statlab", xmLabelWidgetClass, formbot,
				      XmNlabelString, statstring,
				      XmNalignment, XmALIGNMENT_BEGINNING,
				      XmNrecomputeSize, True,
				      NULL);
    XtAddCallback(statlab, XmNhelpCallback, (XtCallbackProc) HelpCB, (XtPointer) "main.html#statbar");

    string = XmStringCreateLtoR("G0:[X, Y] =                                           ",
				charset);
    loclab = XtVaCreateManagedWidget("label Locate", xmLabelWidgetClass, rctop,
				     XmNlabelString, string,
				     XmNalignment, XmALIGNMENT_END,
				     XmNrecomputeSize, True,
				     NULL);
    XmStringFree(string);
    XtAddCallback(loclab, XmNhelpCallback, (XtCallbackProc) HelpCB, (XtPointer) "main.html#locbar");

    XtManageChild(formbot);
    
    if (page_layout == PAGE_FIXED) {

        drawing_window = XtVaCreateManagedWidget("drawing_window",
				     xmScrolledWindowWidgetClass, form,
				     XmNnavigationType, XmEXCLUSIVE_TAB_GROUP,
				     XmNscrollingPolicy, XmAUTOMATIC,
				     XmNvisualPolicy, XmVARIABLE,
				     NULL);
        canvas = XtVaCreateManagedWidget("canvas",
                                     xmDrawingAreaWidgetClass, drawing_window,
				     XmNwidth, (Dimension) DEFAULT_PAGE_WIDTH,
				     XmNheight, (Dimension) DEFAULT_PAGE_HEIGHT,
				     XmNresizePolicy, XmRESIZE_ANY,
                                     XmNbackground,
				     xvlibcolors[0],
				     NULL);
    } else {
        canvas = XtVaCreateManagedWidget("canvas",
                                     xmDrawingAreaWidgetClass, form,
				     XmNwidth, (Dimension) DEFAULT_PAGE_WIDTH,
				     XmNheight, (Dimension) DEFAULT_PAGE_HEIGHT,
				     XmNresizePolicy, XmRESIZE_ANY,
				     XmNnavigationType, XmEXCLUSIVE_TAB_GROUP,
                                     XmNbackground,
				     xvlibcolors[0],
				     NULL);
        drawing_window = canvas;
    }
    
    XtAddCallback(canvas, XmNexposeCallback,
                            (XtCallbackProc) expose_resize, NULL);
    XtAddCallback(canvas, XmNresizeCallback,
                            (XtCallbackProc) expose_resize, NULL);
    XtAddCallback(canvas, XmNhelpCallback,
                    (XtCallbackProc) HelpCB, (XtPointer) "main.html#canvas");

    XtAddEventHandler(canvas, EnterWindowMask
		      | LeaveWindowMask
		      | ButtonPressMask
		      | PointerMotionMask
		      | KeyPressMask
		      | ColormapChangeMask,
		      False,
		      (XtEventHandler) my_proc, NULL);
		      
    XtOverrideTranslations(canvas, XtParseTranslationTable(canvas_table));
		      

    XtVaSetValues(frleft,
		  XmNtopAttachment, XmATTACH_WIDGET,
		  XmNtopWidget, frtop,
		  XmNbottomAttachment, XmATTACH_WIDGET,
		  XmNbottomWidget, frbot,
		  XmNleftAttachment, XmATTACH_FORM,
		  NULL);
    XtVaSetValues(frtop,
		  XmNtopAttachment, XmATTACH_FORM,
		  XmNleftAttachment, XmATTACH_FORM,
		  XmNrightAttachment, XmATTACH_FORM,
		  NULL);
    XtVaSetValues(drawing_window,
		  XmNtopAttachment, XmATTACH_WIDGET,
		  XmNtopWidget, frtop,
		  XmNbottomAttachment, XmATTACH_WIDGET,
		  XmNbottomWidget, frbot,
		  XmNrightAttachment, XmATTACH_FORM,
		  XmNleftAttachment, XmATTACH_WIDGET,
		  XmNleftWidget, frleft,
		  NULL);
    XtVaSetValues(frbot,
		  XmNbottomAttachment, XmATTACH_FORM,
		  XmNrightAttachment, XmATTACH_FORM,
		  XmNleftAttachment, XmATTACH_FORM,
		  NULL);

    XtManageChild(form);

    XmMainWindowSetAreas(main_frame, menu_bar, NULL, NULL, NULL, form);


    bt = XtVaCreateManagedWidget("Draw", xmPushButtonWidgetClass, rcleft,
				 NULL);
    XtAddCallback(bt, XmNactivateCallback,
                        (XtCallbackProc) do_drawgraph, (XtPointer) NULL);
    XtAddCallback(bt, XmNhelpCallback,
                        (XtCallbackProc) HelpCB, (XtPointer) "main.html#draw");

/*
 * initialize pixmaps for buttons on front
 */
/*
 * We need it to get right (same) background color for pixmaps.
 * There should be more clever way of doing that, of course.
 */    
    XtVaGetValues(bt,
		  XmNforeground, &fg,
		  XmNbackground, &bg,
		  NULL);
    init_pm(fg, bg);

/* zoom and autoscale */
    rc3 = XtVaCreateManagedWidget("rc", xmRowColumnWidgetClass, rcleft,
				  XmNorientation, XmHORIZONTAL,
				  XmNpacking, XmPACK_TIGHT,
				  XmNspacing, 0,
				  XmNentryBorder, 0,
				  XmNmarginWidth, 0,
				  XmNmarginHeight, 0,
				  NULL);
    bt = XtVaCreateManagedWidget("Zoom", xmPushButtonWidgetClass, rc3,
				 NULL);
    XtVaSetValues(bt,
		  XmNlabelType, XmPIXMAP,
		  XmNlabelPixmap, zoompm,
		  NULL);
    XtAddCallback(bt, XmNactivateCallback, (XtCallbackProc) set_actioncb, (XtPointer) ZOOM_1ST);
    XtAddCallback(bt, XmNhelpCallback, (XtCallbackProc) HelpCB, (XtPointer) "main.html#zoom");

    bt = XtVaCreateManagedWidget("AS", xmPushButtonWidgetClass, rc3,
				 NULL);
    XtVaSetValues(bt,
		  XmNlabelType, XmPIXMAP,
		  XmNlabelPixmap, autopm,
		  NULL);
    XtAddCallback(bt, XmNactivateCallback,
                                    autoscale_proc, (XtPointer) AUTOSCALE_XY);
    XtAddCallback(bt, XmNhelpCallback,
                         (XtCallbackProc) HelpCB, (XtPointer) "main.html#as");

/* expand/shrink */
    rc3 = XtVaCreateManagedWidget("rc", xmRowColumnWidgetClass, rcleft,
				  XmNorientation, XmHORIZONTAL,
				  XmNpacking, XmPACK_TIGHT,
				  XmNspacing, 0,
				  XmNentryBorder, 0,
				  XmNmarginWidth, 0,
				  XmNmarginHeight, 0,
				  NULL);
    bt = XtVaCreateManagedWidget("Z", xmPushButtonWidgetClass, rc3,
				 NULL);
    XtVaSetValues(bt,
		  XmNlabelType, XmPIXMAP,
		  XmNlabelPixmap, expandpm,
		  NULL);
    XtAddCallback(bt, XmNactivateCallback,
                    (XtCallbackProc) graph_zoom_proc, (XtPointer) GZOOM_SHRINK);
    XtAddCallback(bt, XmNhelpCallback, (XtCallbackProc) HelpCB, (XtPointer) "main.html#shrink");

    bt = XtVaCreateManagedWidget("z", xmPushButtonWidgetClass, rc3,
				 NULL);
    XtVaSetValues(bt,
		  XmNlabelType, XmPIXMAP,
		  XmNlabelPixmap, shrinkpm,
		  NULL);
    XtAddCallback(bt, XmNactivateCallback,
                (XtCallbackProc) graph_zoom_proc, (XtPointer) GZOOM_EXPAND);
    XtAddCallback(bt, XmNhelpCallback, (XtCallbackProc) HelpCB, (XtPointer) "main.html#expand");

/*
 * scrolling buttons
 */
    rc3 = XtVaCreateManagedWidget("rc", xmRowColumnWidgetClass, rcleft,
				  XmNorientation, XmHORIZONTAL,
				  XmNpacking, XmPACK_TIGHT,
				  XmNspacing, 0,
				  XmNentryBorder, 0,
				  XmNmarginWidth, 0,
				  XmNmarginHeight, 0,
				  NULL);
    bt = XtVaCreateManagedWidget("Left", xmPushButtonWidgetClass, rc3,
				 NULL);
    XtVaSetValues(bt,
		  XmNlabelType, XmPIXMAP,
		  XmNlabelPixmap, leftpm,
		  NULL);
    XtAddCallback(bt, XmNactivateCallback, 
                (XtCallbackProc) graph_scroll_proc, (XtPointer) GSCROLL_LEFT);
    XtAddCallback(bt, XmNhelpCallback, (XtCallbackProc) HelpCB, (XtPointer) "main.html#left");

    bt = XtVaCreateManagedWidget("Right", xmPushButtonWidgetClass, rc3,
				 NULL);
    XtVaSetValues(bt,
		  XmNlabelType, XmPIXMAP,
		  XmNlabelPixmap, rightpm,
		  NULL);
    XtAddCallback(bt, XmNactivateCallback,
                        (XtCallbackProc) graph_scroll_proc, (XtPointer) GSCROLL_RIGHT);
    XtAddCallback(bt, XmNhelpCallback, (XtCallbackProc) HelpCB, (XtPointer) "main.html#right");

    rc3 = XtVaCreateManagedWidget("rc", xmRowColumnWidgetClass, rcleft,
				  XmNorientation, XmHORIZONTAL,
				  XmNpacking, XmPACK_TIGHT,
				  XmNspacing, 0,
				  XmNentryBorder, 0,
				  XmNmarginWidth, 0,
				  XmNmarginHeight, 0,
				  NULL);

    bt = XtVaCreateManagedWidget("Down", xmPushButtonWidgetClass, rc3,
				 NULL);
    XtVaSetValues(bt,
		  XmNlabelType, XmPIXMAP,
		  XmNlabelPixmap, downpm,
		  NULL);
    XtAddCallback(bt, XmNactivateCallback,
                (XtCallbackProc) graph_scroll_proc, (XtPointer) GSCROLL_DOWN);
    XtAddCallback(bt, XmNhelpCallback, (XtCallbackProc) HelpCB, (XtPointer) "main.html#down");
    
    bt = XtVaCreateManagedWidget("Up", xmPushButtonWidgetClass, rc3,
				 NULL);
    XtVaSetValues(bt,
		  XmNlabelType, XmPIXMAP,
		  XmNlabelPixmap, uppm,
		  NULL);
    XtAddCallback(bt, XmNactivateCallback,
                (XtCallbackProc) graph_scroll_proc, (XtPointer) GSCROLL_UP);
    XtAddCallback(bt, XmNhelpCallback, (XtCallbackProc) HelpCB, (XtPointer) "main.html#up");

    XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, rcleft,
			    NULL);


    bt = XtVaCreateManagedWidget("AutoT", xmPushButtonWidgetClass, rcleft,
				 NULL);
    XtAddCallback(bt, XmNactivateCallback, (XtCallbackProc) autoticks_proc, NULL);
    XtAddCallback(bt, XmNhelpCallback, (XtCallbackProc) HelpCB, (XtPointer) "main.html#autoticks");

    bt = XtVaCreateManagedWidget("AutoO", xmPushButtonWidgetClass, rcleft,
				 NULL);
    XtAddCallback(bt, XmNactivateCallback, (XtCallbackProc) autoon_proc, NULL);
    XtAddCallback(bt, XmNhelpCallback, (XtCallbackProc) HelpCB, (XtPointer) "main.html#autoon");

    rc3 = XtVaCreateManagedWidget("rc", xmRowColumnWidgetClass, rcleft,
				  XmNorientation, XmHORIZONTAL,
				  XmNpacking, XmPACK_TIGHT,
				  XmNspacing, 0,
				  XmNentryBorder, 0,
				  XmNmarginWidth, 0,
				  XmNmarginHeight, 0,
				  NULL);
    bt = XtVaCreateManagedWidget("ZX", xmPushButtonWidgetClass, rc3,
				 NULL);
    XtAddCallback(bt, XmNactivateCallback, (XtCallbackProc) set_actioncb, (XtPointer) ZOOMX_1ST);
    XtAddCallback(bt, XmNhelpCallback, (XtCallbackProc) HelpCB, (XtPointer) "main.html#zoomx");

    bt = XtVaCreateManagedWidget("ZY", xmPushButtonWidgetClass, rc3,
				 NULL);
    XtAddCallback(bt, XmNactivateCallback, (XtCallbackProc) set_actioncb, (XtPointer) ZOOMY_1ST);
    XtAddCallback(bt, XmNhelpCallback, (XtCallbackProc) HelpCB, (XtPointer) "main.html#zoomy");

    rc3 = XtVaCreateManagedWidget("rc", xmRowColumnWidgetClass, rcleft,
				  XmNorientation, XmHORIZONTAL,
				  XmNpacking, XmPACK_TIGHT,
				  XmNspacing, 0,
				  XmNentryBorder, 0,
				  XmNmarginWidth, 0,
				  XmNmarginHeight, 0,
				  NULL);
    bt = XtVaCreateManagedWidget("AX", xmPushButtonWidgetClass, rc3,
				 NULL);
    XtAddCallback(bt, XmNactivateCallback,
                                     autoscale_proc, (XtPointer) AUTOSCALE_X);
    XtAddCallback(bt, XmNhelpCallback,
                      (XtCallbackProc) HelpCB, (XtPointer) "main.html#autox");

    bt = XtVaCreateManagedWidget("AY", xmPushButtonWidgetClass, rc3,
				 NULL);
    XtAddCallback(bt, XmNactivateCallback,
                                     autoscale_proc, (XtPointer) AUTOSCALE_Y);
    XtAddCallback(bt, XmNhelpCallback,
                      (XtCallbackProc) HelpCB, (XtPointer) "main.html#autoy");

    rc3 = XtVaCreateManagedWidget("rc", xmRowColumnWidgetClass, rcleft,
				  XmNorientation, XmHORIZONTAL,
				  XmNpacking, XmPACK_TIGHT,
				  XmNspacing, 0,
				  XmNentryBorder, 0,
				  XmNmarginWidth, 0,
				  XmNmarginHeight, 0,
				  NULL);
    bt = XtVaCreateManagedWidget("PZ", xmPushButtonWidgetClass, rc3,
				 NULL);
    XtAddCallback(bt, XmNactivateCallback, 
                            world_stack_proc, (XtPointer) WSTACK_PUSH_ZOOM);
    XtAddCallback(bt, XmNhelpCallback,
                        (XtCallbackProc) HelpCB, (XtPointer) "main.html#pz");

    bt = XtVaCreateManagedWidget("Pu", xmPushButtonWidgetClass, rc3,
				 NULL);
    XtAddCallback(bt, XmNactivateCallback, 
                                 world_stack_proc, (XtPointer) WSTACK_PUSH);
    XtAddCallback(bt, XmNhelpCallback,
                        (XtCallbackProc) HelpCB, (XtPointer) "main.html#pu");

    rc3 = XtVaCreateManagedWidget("rc", xmRowColumnWidgetClass, rcleft,
				  XmNorientation, XmHORIZONTAL,
				  XmNpacking, XmPACK_TIGHT,
				  XmNspacing, 0,
				  XmNentryBorder, 0,
				  XmNmarginWidth, 0,
				  XmNmarginHeight, 0,
				  NULL);
    bt = XtVaCreateManagedWidget("Po", xmPushButtonWidgetClass, rc3,
				 NULL);
    XtAddCallback(bt, XmNactivateCallback, 
                                world_stack_proc, (XtPointer) WSTACK_POP);
    XtAddCallback(bt, XmNhelpCallback,
                        (XtCallbackProc) HelpCB, (XtPointer) "main.html#po");

    bt = XtVaCreateManagedWidget("Cy", xmPushButtonWidgetClass, rc3,
				 NULL);
    XtAddCallback(bt, XmNactivateCallback, 
                                world_stack_proc, (XtPointer) WSTACK_CYCLE);
    XtAddCallback(bt, XmNhelpCallback,
                        (XtCallbackProc) HelpCB, (XtPointer) "main.html#cy");

    sdstring = XmStringCreateLtoR("SD:1 ", charset);
    stack_depth_item = XtVaCreateManagedWidget("stackdepth",
                                               xmLabelWidgetClass, rcleft,
					       XmNlabelString, sdstring,
					       NULL);
    XtAddCallback(stack_depth_item, XmNhelpCallback,
                        (XtCallbackProc) HelpCB, (XtPointer) "main.html#sd");

    cystring = XmStringCreateLtoR("CW:0 ", charset);
    curw_item = XtVaCreateManagedWidget("curworld", xmLabelWidgetClass, rcleft,
					XmNlabelString, cystring,
					NULL);
    XtAddCallback(curw_item, XmNhelpCallback,
                        (XtCallbackProc) HelpCB, (XtPointer) "main.html#cw");

    bt = XtVaCreateManagedWidget("Exit", xmPushButtonWidgetClass, rcleft,
				 NULL);

    XtAddCallback(bt, XmNactivateCallback,
                            (XtCallbackProc) MenuCB, (XtPointer) MENU_EXIT);
    XtAddCallback(bt, XmNhelpCallback,
                        (XtCallbackProc) HelpCB, (XtPointer) "main.html#exit");

/*
 * initialize cursors
 */
    init_cursors();

/*
 * initialize some option menus
 */
    init_option_menus();

/*
 * initialize the tool bars
 */
    set_view_items();

/*
 * set icon
 */
#if defined(HAVE_XPM)
    XpmCreatePixmapFromData(disp, root,
			    grace_icon_xpm, &icon, &shape, NULL);
#else
    icon = XCreatePixmapFromBitmapData(disp, root, 
                    (char *) grace_icon_bits, grace_icon_width, grace_icon_height,
		                                                 fg, bg, depth);
    shape = icon;
#endif
    XtVaSetValues(app_shell, XtNiconPixmap, icon, XtNiconMask, shape, NULL);

    XtRealizeWidget(app_shell);
    xwin = XtWindow(canvas);
    
/*
 * set the title
 */
    set_title(mybasename(docname));

/*
 * If logging is on, initialize
 */
    inwin = 1;
    log_results("Startup");
    inwin = 0;
}

void do_main_winloop(void)
{
    XtAppMainLoop(app_con);
}

/*
 * initialize pixmaps for buttons on front
 */
static void init_pm(Pixel fg, Pixel bg)
{
    zoompm = XCreatePixmapFromBitmapData(disp, root, 
                                (char *) zoom_bits, 16, 16, fg, bg, depth);
    autopm = XCreatePixmapFromBitmapData(disp, root, 
                                (char *) auto_bits, 16, 16, fg, bg, depth);
    shrinkpm = XCreatePixmapFromBitmapData(disp, root, 
                                (char *) shrink_bits, 16, 16, fg, bg, depth);
    expandpm = XCreatePixmapFromBitmapData(disp, root, 
                                (char *) expand_bits, 16, 16, fg, bg, depth);
    rightpm = XCreatePixmapFromBitmapData(disp, root, 
                                (char *) right_bits, 16, 16, fg, bg, depth);
    leftpm = XCreatePixmapFromBitmapData(disp, root, 
                                (char *) left_bits, 16, 16, fg, bg, depth);
    uppm = XCreatePixmapFromBitmapData(disp, root, 
                                (char *) up_bits, 16, 16, fg, bg, depth);
    downpm = XCreatePixmapFromBitmapData(disp, root, 
                                (char *) down_bits, 16, 16, fg, bg, depth);
}

void get_canvas_size(Dimension *w, Dimension *h)
{
    XtVaGetValues(canvas,
                  XmNwidth, w,
                  XmNheight, h,
                  NULL);
}

void set_canvas_size(Dimension w, Dimension h)
{
    XtVaSetValues(canvas,
                  XmNwidth, w,
                  XmNheight, h,
                  NULL);
}


void set_pagelayout(int layout)
{
    if (page_layout == layout) {
        return;
    }
    
    if (inwin) {
        errmsg("Can not change layout after initialization of GUI");
        return;
    } else {
        page_layout = layout;
    }
    
/*
 *     if (layout == PAGE_FREE) {
 *         XtVaSetValues(scrollw,
 *         	      XmNscrollingPolicy, XmAPPLICATION_DEFINED,
 * 		      XmNvisualPolicy, XmVARIABLE,
 *                       NULL);
 *     } else {
 *         XtVaSetValues(scrollw,
 *         	      XmNscrollingPolicy, XmAUTOMATIC,
 * 		      XmNvisualPolicy, XmCONSTANT,
 *                       NULL);
 *     }
 */
}

static void graph_scroll_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    graph_scroll((int) client_data);
    drawgraph();
}

static void graph_zoom_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    graph_zoom((int) client_data);
    drawgraph();
}

static void world_stack_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    switch ((int) client_data) {
    case WSTACK_PUSH_ZOOM:
        push_and_zoom();
        break;
    case WSTACK_PUSH:
        push_world();
        break;
    case WSTACK_POP:
        pop_world();
        break;
    case WSTACK_CYCLE:
        cycle_world_stack();
        break;
    default:
        return;
    }
    update_all();
    drawgraph();
}
