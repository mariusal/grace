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

#include <config.h>

#include <stdlib.h>
#include <string.h>

#include "globals.h"
#include "patchlevel.h"
#include "graphs.h"
#include "utils.h"
#include "protos.h"

#include <X11/cursorfont.h>
#include <Xm/ToggleB.h>
#include "motifinc.h"

#define NO_HELP "nohelp.html"

#ifdef WITH_LIBHELP
#  include <help.h>
#endif

extern char gui_version[];

extern Display *disp;
extern Widget app_shell;


void HelpCB(Widget w, XtPointer client_data, XtPointer call_data)
{
    char URL[256];
    char *help_viewer, *ha;
#ifndef WITH_LIBHELP    
    int i=0, j=0;
    char command[1024];
    int len;
#endif /* WITH_LIBHELP */

    ha = (char *) client_data;
    if (ha == NULL) {
        ha = NO_HELP;
    }
    
    set_wait_cursor();
    
#ifdef WITH_LIBHELP
    if (strstr(ha, "http:")) {
        char buf[256];
        strcpy(URL, ha);
        sprintf(buf, "The remote URL, %s, can't be accessed with xmhelp", URL);
        errmsg(buf);
    } else {
        /* xmhelp doesn't like "file://localhost/" prefix */
        sprintf(URL, "file:%s/doc/%s", get_grace_home(), ha);
        get_help(w, (XtPointer) URL, ha);
    }
#else /* usual HTML browser */

    if (strstr(ha, "http:")) {
        strcpy(URL, ha);
    } else {
        sprintf(URL, "file://localhost%s/doc/%s", get_grace_home(), ha);
    }
    
    help_viewer = get_help_viewer();
    len = strlen(help_viewer);
    for (i = 0; i < len - 1; i++) {
    	if ((help_viewer[i] == '%') && (help_viewer[i+1] == 's')){
    	    strcpy (&command[j], URL);
    	    j += strlen(URL);
    	    i++;
    	} else {
    	    command[j++] = help_viewer[i];
    	}
    }      
#ifdef VMS    
    system_spawn(command);
#else
    strcat(command, "&");    
    system(command);
#endif

#endif /* WITH_LIBHELP */

    unset_wait_cursor();
}

void ContextHelpCB(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget whelp;
    XmAnyCallbackStruct *cb_struct;
    Cursor cursor;
    
    cb_struct = call_data;
    cursor = XCreateFontCursor(disp, XC_question_arrow);
    whelp = XmTrackingLocate(app_shell, cursor, False);
    if (whelp != NULL) {
        cb_struct->reason = XmCR_HELP;
        XtCallCallbacks(whelp, XmNhelpCallback, cb_struct);
    }
    XFreeCursor(disp, cursor);
}

/*
 * say a few things about Grace
 */
static Widget about_frame;
static Widget about_panel;

void create_about_grtool(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget wbut, rc, sep;
    char buf[1024];

    set_wait_cursor();
    if (about_frame == NULL) {
	about_frame = XmCreateDialogShell(app_shell, "About", NULL, 0);
	handle_close(about_frame);
	about_panel = XmCreateRowColumn(about_frame, "about_rc", NULL, 0);

	sprintf(buf, "Grace-%d.%d.%d %s, GUI: %s",
                       MAJOR_REV, MINOR_REV, PATCHLEVEL, BETA_VER, gui_version);
	XtVaCreateManagedWidget(buf, xmLabelWidgetClass, about_panel, NULL);
        
	sep = XmCreateSeparator(about_panel, "sep", NULL, 0);
        XtManageChild(sep);

	sprintf(buf, "Max scratch array length = %d", MAXARR);
	XtVaCreateManagedWidget(buf, xmLabelWidgetClass, about_panel, NULL);
	sprintf(buf, "Max number of lines = %d", maxlines);
	XtVaCreateManagedWidget(buf, xmLabelWidgetClass, about_panel, NULL);
	sprintf(buf, "Max number of boxes = %d", maxboxes);
	XtVaCreateManagedWidget(buf, xmLabelWidgetClass, about_panel, NULL);
	sprintf(buf, "Max number of ellipses = %d", maxellipses);
	XtVaCreateManagedWidget(buf, xmLabelWidgetClass, about_panel, NULL);
	sprintf(buf, "Max number of strings = %d", maxstr);
	XtVaCreateManagedWidget(buf, xmLabelWidgetClass, about_panel, NULL);
        
	sep = XmCreateSeparator(about_panel, "sep", NULL, 0);
        XtManageChild(sep);

	sprintf(buf, "The home of Grace is http://plasma-gate.weizmann.ac.il/Grace/");
	XtVaCreateManagedWidget(buf, xmLabelWidgetClass, about_panel, NULL);
        
	sep = XmCreateSeparator(about_panel, "sep", NULL, 0);
        XtManageChild(sep);

	sprintf(buf, "(C) Copyright 1991-1995 Paul J Turner");
	XtVaCreateManagedWidget(buf, xmLabelWidgetClass, about_panel, NULL);
	sprintf(buf, "(C) Copyright 1996-1998 Grace Development Team");
	XtVaCreateManagedWidget(buf, xmLabelWidgetClass, about_panel, NULL);
	sprintf(buf, "All Rights Reserved");
	XtVaCreateManagedWidget(buf, xmLabelWidgetClass, about_panel, NULL);
	sprintf(buf, "The program is distributed under the terms of the GNU General Public License");
	XtVaCreateManagedWidget(buf, xmLabelWidgetClass, about_panel, NULL);
        
	sep = XmCreateSeparator(about_panel, "sep", NULL, 0);
        XtManageChild(sep);

	rc = XmCreateRowColumn(about_panel, "rc", NULL, 0);
	XtVaSetValues(rc, XmNorientation, XmHORIZONTAL, NULL);
	wbut = XtVaCreateManagedWidget("Close", xmPushButtonWidgetClass, rc,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) about_frame);
	XtManageChild(rc);

	XtManageChild(about_panel);
    }
    XtRaise(about_frame);
    unset_wait_cursor();
}
