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
#include "utils.h"
#include "buildinfo.h"
#include "protos.h"

#include <X11/cursorfont.h>
#include <Xm/DialogS.h>
#include <Xm/RowColumn.h>

#include "motifinc.h"

#define NO_HELP "nohelp.html"

#ifdef WITH_LIBHELP
#  include <help.h>
#endif

extern Display *disp;
extern Widget app_shell;


void HelpCB(void *data)
{
    char URL[256];
    char *help_viewer, *ha;
#ifndef WITH_LIBHELP    
    int i=0, j=0;
    char command[1024];
    int len;
#endif /* WITH_LIBHELP */

    ha = (char *) data;
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
        get_help(app_shell, (XtPointer) URL, NULL);
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

void ContextHelpCB(void *data)
{
    Widget whelp;
    Cursor cursor;
    
    cursor = XCreateFontCursor(disp, XC_question_arrow);
    whelp = XmTrackingLocate(app_shell, cursor, False);
    if (whelp != NULL) {
        errmsg("Not implemented yet");
    }
    XFreeCursor(disp, cursor);
}

/*
 * say a few things about Grace
 */
static Widget about_frame;
static Widget about_panel;

void create_about_grtool(void *data)
{
    Widget wbut;
    char buf[1024];

    set_wait_cursor();
    
    if (about_frame == NULL) {
	about_frame = XmCreateDialogShell(app_shell, "About", NULL, 0);
	handle_close(about_frame);
	about_panel = XmCreateRowColumn(about_frame, "about_rc", NULL, 0);

	CreateLabel(about_panel, bi_version_string());

	CreateSeparator(about_panel);

	CreateLabel(about_panel, "Copyright (c) 1991-95 Paul J Turner");
	CreateLabel(about_panel, "Copyright (c) 1996-99 Grace Development Team");
	CreateLabel(about_panel, "All rights reserved");
	CreateLabel(about_panel,
            "The program is distributed under the terms of the GNU General Public License");

	CreateSeparator(about_panel);

	CreateLabel(about_panel,
            "The home of Grace is http://plasma-gate.weizmann.ac.il/Grace/");

	CreateSeparator(about_panel);

	CreateLabel(about_panel,
            "Contains Tab widget, Copyright (c) 1997 Pralay Dakua");
	CreateLabel(about_panel, "May contain Xbae widget,");
	CreateLabel(about_panel,
            "      Copyright (c) 1991, 1992 Bell Communications Research, Inc. (Bellcore)");
	CreateLabel(about_panel,
            "      Copyright (c) 1995-97 Andrew Lister");
	CreateLabel(about_panel, "Raster driver based on the GD-1.3 library,");
	CreateLabel(about_panel,
            "      Portions copyright (c) 1994-98 Cold Spring Harbor Laboratory");
	CreateLabel(about_panel,
            "      Portions copyright (c) 1996-98 Boutell.Com, Inc");
#ifdef HAVE_LIBPDF
	CreateLabel(about_panel,
            "May contain PDFlib library, Copyright (c) 1997-99 Thomas Metz");
#endif

	CreateSeparator(about_panel);

	sprintf(buf, "Built on: %s", BI_SYSTEM);
	CreateLabel(about_panel, buf);
	sprintf(buf, "Build time: %s", BI_DATE);
	CreateLabel(about_panel, buf);
	sprintf(buf, "GUI toolkit: %s ", BI_GUI);
	CreateLabel(about_panel, buf);
	sprintf(buf, "T1lib: %s ", BI_T1LIB);
	CreateLabel(about_panel, buf);
#ifdef DEBUG
	CreateLabel(about_panel, "Debugging is enabled");
#endif
	
	CreateSeparator(about_panel);

	wbut = CreateButton(about_panel, "Close");
	AlignLabel(wbut, ALIGN_CENTER);
        XtAddCallback(wbut, XmNactivateCallback, destroy_dialog, (XtPointer) about_frame);

	XtManageChild(about_panel);
    }
    XtRaise(about_frame);
    unset_wait_cursor();
}
