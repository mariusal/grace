/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-95 Paul J Turner, Portland, OR
 * Copyright (c) 1996-99 Grace Development Team
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
#include "graphs.h"
#include "utils.h"
#include "protos.h"

#include <X11/cursorfont.h>
#include <Xm/ToggleB.h>
#include "motifinc.h"
#include "buildinfo.h"

#define NO_HELP "nohelp.html"

#ifdef WITH_LIBHELP
#  include <help.h>
#endif

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
    system_wrap(command);
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
    Widget wbut, rc;
    char buf[1024];

    set_wait_cursor();
    if (about_frame == NULL) {
	about_frame = XmCreateDialogShell(app_shell, "About", NULL, 0);
	handle_close(about_frame);
	about_panel = XmCreateRowColumn(about_frame, "about_rc", NULL, 0);

	sprintf(buf, "%s", bi_version_string());
	XtVaCreateManagedWidget(buf, xmLabelWidgetClass, about_panel, NULL);
	CreateSeparator(about_panel);

	sprintf(buf, "Copyright (c) 1991-95 Paul J Turner");
	XtVaCreateManagedWidget(buf, xmLabelWidgetClass, about_panel, NULL);
	sprintf(buf, "Copyright (c) 1996-99 Grace Development Team");
	XtVaCreateManagedWidget(buf, xmLabelWidgetClass, about_panel, NULL);
	sprintf(buf, "All rights reserved");
	XtVaCreateManagedWidget(buf, xmLabelWidgetClass, about_panel, NULL);
	sprintf(buf, "The program is distributed under the terms of the GNU General Public License");
	XtVaCreateManagedWidget(buf, xmLabelWidgetClass, about_panel, NULL);
        
	CreateSeparator(about_panel);

	sprintf(buf, "The home of Grace is http://plasma-gate.weizmann.ac.il/Grace/");
	XtVaCreateManagedWidget(buf, xmLabelWidgetClass, about_panel, NULL);
        
	CreateSeparator(about_panel);

	sprintf(buf, "Contains Tab widget, Copyright (c) 1997 Pralay Dakua");
	XtVaCreateManagedWidget(buf, xmLabelWidgetClass, about_panel, NULL);
	sprintf(buf, "May contain Xbae widget,");
	XtVaCreateManagedWidget(buf, xmLabelWidgetClass, about_panel, NULL);
	sprintf(buf, "      Copyright (c) 1991, 1992 Bell Communications Research, Inc. (Bellcore)");
	XtVaCreateManagedWidget(buf, xmLabelWidgetClass, about_panel, NULL);
	sprintf(buf, "      Copyright (c) 1995-97 Andrew Lister");
	XtVaCreateManagedWidget(buf, xmLabelWidgetClass, about_panel, NULL);
#ifdef HAVE_LIBPDF
	sprintf(buf, "May contain PDFlib library, Copyright (c) 1997-98 Thomas Metz");
	XtVaCreateManagedWidget(buf, xmLabelWidgetClass, about_panel, NULL);
#endif
#ifdef HAVE_LIBGD
	sprintf(buf, "May contain GD library,");
	XtVaCreateManagedWidget(buf, xmLabelWidgetClass, about_panel, NULL);
	sprintf(buf, "      Portions copyright (c) 1994-98 Cold Spring Harbor Laboratory");
	XtVaCreateManagedWidget(buf, xmLabelWidgetClass, about_panel, NULL);
	sprintf(buf, "      Portions copyright (c) 1996-98 Boutell.Com, Inc");
	XtVaCreateManagedWidget(buf, xmLabelWidgetClass, about_panel, NULL);
	sprintf(buf, "      GIF decompression code copyright (c) 1990, 91, 93 David Koblas");
	XtVaCreateManagedWidget(buf, xmLabelWidgetClass, about_panel, NULL);
	sprintf(buf, "      Non-LZW-based GIF compression code copyright (c) 1998");
	XtVaCreateManagedWidget(buf, xmLabelWidgetClass, about_panel, NULL);
	sprintf(buf, "      Hutchison Avenue Software Corporation");
	XtVaCreateManagedWidget(buf, xmLabelWidgetClass, about_panel, NULL);
#endif

	CreateSeparator(about_panel);

	sprintf(buf, "Built on: %s", BI_SYSTEM);
	XtVaCreateManagedWidget(buf, xmLabelWidgetClass, about_panel, NULL);
	sprintf(buf, "Build time: %s", BI_DATE);
	XtVaCreateManagedWidget(buf, xmLabelWidgetClass, about_panel, NULL);
	sprintf(buf, "GUI toolkit: %s ", BI_GUI);
	XtVaCreateManagedWidget(buf, xmLabelWidgetClass, about_panel, NULL);
	sprintf(buf, "T1lib: %s ", BI_T1LIB);
	XtVaCreateManagedWidget(buf, xmLabelWidgetClass, about_panel, NULL);
#ifdef DEBUG
	sprintf(buf, "Debugging is enabled");
	XtVaCreateManagedWidget(buf, xmLabelWidgetClass, about_panel, NULL);
#endif
	
	CreateSeparator(about_panel);

	sprintf(buf, "Max number of lines = %d", maxlines);
	XtVaCreateManagedWidget(buf, xmLabelWidgetClass, about_panel, NULL);
	sprintf(buf, "Max number of boxes = %d", maxboxes);
	XtVaCreateManagedWidget(buf, xmLabelWidgetClass, about_panel, NULL);
	sprintf(buf, "Max number of ellipses = %d", maxellipses);
	XtVaCreateManagedWidget(buf, xmLabelWidgetClass, about_panel, NULL);
	sprintf(buf, "Max number of strings = %d", maxstr);
	XtVaCreateManagedWidget(buf, xmLabelWidgetClass, about_panel, NULL);
        
	CreateSeparator(about_panel);

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
