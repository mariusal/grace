/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2000 Grace Development Team
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
#include "files.h"
#include "protos.h"

#include <X11/cursorfont.h>

#include "motifinc.h"

#define NO_HELP "nohelp.html"

#ifdef WITH_LIBHELP
#  include <help.h>
#endif

#ifdef WITH_XMHTML
#  include <XmHTML/XmHTML.h>
void create_helper_frame(char *fname);
#endif

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

    if (strstr(ha, "http:") || strstr(ha, "ftp:") || strstr(ha, "mailto:")) {
        strcpy(URL, ha);
    } else {
#ifdef WITH_XMHTML
        sprintf(URL, "doc/%s", ha);
        create_helper_frame(URL);
        return;
#else
        sprintf(URL, "file://localhost%s/doc/%s", get_grace_home(), ha);
#endif
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
}

extern Display *disp;

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

void create_about_grtool(void *data)
{
    set_wait_cursor();
    
    if (about_frame == NULL) {
        Widget wbut, fr, rc, about_panel;
        char buf[1024];
        
	about_frame = CreateDialogForm(app_shell, "About");
	
        about_panel = CreateVContainer(about_frame);
        AddDialogFormChild(about_frame, about_panel);

	fr = CreateFrame(about_panel, NULL);
        rc = CreateVContainer(fr);
	CreateLabel(rc, bi_version_string());
#ifdef DEBUG
	CreateLabel(rc, "Debugging is enabled");
#endif

	fr = CreateFrame(about_panel, "Legal stuff");
        rc = CreateVContainer(fr);
	CreateLabel(rc, "Copyright (c) 1991-1995 Paul J Turner");
	CreateLabel(rc, "Copyright (c) 1996-2000 Grace Development Team");
	CreateLabel(rc, "Maintained by Evgeny Stambulchik");
	CreateLabel(rc, "All rights reserved");
	CreateLabel(rc,
            "The program is distributed under the terms of the GNU General Public License");

	fr = CreateFrame(about_panel, "Third party copyrights");
        rc = CreateVContainer(fr);
	CreateLabel(rc,
            "Tab widget, Copyright (c) 1997 Pralay Dakua");
	CreateLabel(rc, "Xbae widget,");
	CreateLabel(rc,
            "      Copyright (c) 1991, 1992 Bell Communications Research, Inc. (Bellcore)");
	CreateLabel(rc,
            "      Copyright (c) 1995-1999 Andrew Lister");
	CreateLabel(rc, "Raster driver based on the GD-1.3 library,");
	CreateLabel(rc,
            "      Portions copyright (c) 1994-1998 Cold Spring Harbor Laboratory");
	CreateLabel(rc,
            "      Portions copyright (c) 1996-1998 Boutell.Com, Inc");
#ifdef HAVE_LIBPDF
	CreateLabel(rc, "PDFlib library, Copyright (c) 1997-2000 Thomas Merz");
#endif

	fr = CreateFrame(about_panel, "Build info");
        rc = CreateVContainer(fr);
        sprintf(buf, "Host: %s", bi_system());
	CreateLabel(rc, buf);
	sprintf(buf, "Time: %s", bi_date());
	CreateLabel(rc, buf);
	sprintf(buf, "GUI toolkit: %s ", bi_gui());
	CreateLabel(rc, buf);
	sprintf(buf, "Xbae version: %s ", bi_gui_xbae());
	CreateLabel(rc, buf);
	sprintf(buf, "T1lib: %s ", bi_t1lib());
	CreateLabel(rc, buf);

	fr = CreateFrame(about_panel, "Home page");
        rc = CreateVContainer(fr);
	CreateLabel(rc, "http://plasma-gate.weizmann.ac.il/Grace/");
	
	CreateSeparator(about_panel);

	wbut = CreateButton(about_panel, "Close");
	AlignLabel(wbut, ALIGN_CENTER);
        AddButtonCB(wbut, destroy_dialog_cb, GetParent(about_frame));
        
        ManageChild(about_frame);
    }
    
    RaiseWindow(GetParent(about_frame));
    
    unset_wait_cursor();
}


#ifdef WITH_XMHTML
/*
 * Simplistic HTML viewer
 */

typedef struct _html_ui {
    Widget top;
    Widget html;
    TextStructure *track;
    TextStructure *input;
    Widget case_sensitive;
    Widget find_backwards;
    XmHTMLTextFinder finder;
    char *last;
} html_ui;

static char *loadFile(char *filename)
{
    FILE *file;
    int size;
    char *content;

    /* open the given file */
    if ((file = grace_openr(filename, SOURCE_DISK)) == NULL) {
	return NULL;
    }

    /* see how large this file is */
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    rewind(file);

    /* allocate a buffer large enough to contain the entire file */
    if ((content = xmalloc(size + 1)) == NULL) {
        errmsg("xmalloc failed");
        return NULL;
    }

    /* now read the contents of this file */
    if ((fread(content, 1, size, file)) != size) {
        errmsg("Warning: did not read entire file!");
    }

    grace_close(file);

    /* sanity */
    content[size] = '\0';

    return content;
}

static void anchorCB(Widget w, XtPointer client_data, XtPointer call_data)
{
    int id;
    XmHTMLAnchorPtr href_data = (XmHTMLAnchorPtr) call_data;
    
    /* see if we have been called with a valid reason */
    if (href_data->reason != XmCR_ACTIVATE) {
        return;
    }

    switch (href_data->url_type) {
    /* a named anchor */
    case ANCHOR_JUMP:
        /* see if XmHTML knows this anchor */
        if ((id = XmHTMLAnchorGetId(w, href_data->href)) != -1) {
            /* and let XmHTML jump and mark as visited */
            href_data->doit = True;
            href_data->visited = True;
            return;
        }
        return;
        break;
    /* let HelpCB check all other types */
    default:
        HelpCB(href_data->href);
        break;
    }
}

static void trackCB(Widget w, XtPointer client_data, XtPointer call_data)
{
    XmHTMLAnchorPtr href_data = (XmHTMLAnchorPtr) call_data;
    html_ui *ui = (html_ui *) client_data;

    /* see if we have been called with a valid reason */
    if (href_data->reason != XmCR_HTML_ANCHORTRACK) {
        return;
    }

    if (href_data->href) {
        /* a valid anchor, eg, moving into an anchor */
        SetTextString(ui->track, href_data->href);
    } else {
        /* a valid anchor, eg, moving away from an anchor */
        SetTextString(ui->track, "");
    }
}

static int find_cb(void *data)
{
    char *s, *ptr;
    int case_sensitive, find_backwards;
    XmHTMLTextPosition start, end;
    html_ui *ui = (html_ui *) data;
    
    ptr = GetTextString(ui->input);

    if (!ptr || ptr[0] == '\0') {
        return RETURN_FAILURE;
    }
    
    if (ui->finder == NULL) {
        ui->finder = XmHTMLTextFinderCreate(ui->html);
        if (ui->finder == NULL) {
            errmsg("XmHTMLTextFinderCreate failed!");
            return RETURN_FAILURE;
        }
    }

    s = copy_string(NULL, ptr);

    case_sensitive = GetToggleButtonState(ui->case_sensitive);
    find_backwards = GetToggleButtonState(ui->find_backwards);

    /*****
    * The second arg represent regcomp flags, the default being
    * REG_EXTENDED. Using -1 for this arg instructs the finder to
    * keep the current flags. See man regcomp on possible values for
    * your system. The third arg specifies whether or not the search
    * should be done case-insensitive (True) or not (False). The last arg
    * specifies the search direction. Currently only forward (top to
    * bottom) is supported.
    *****/
    XmHTMLTextFinderSetPatternFlags(ui->finder,
        -1,
        case_sensitive ? False : True,
        find_backwards ? XmHTML_BACKWARD : XmHTML_FORWARD);
    
    if (ui->last == NULL || strcmp(ui->last, s)) {
        if(!XmHTMLTextFinderSetPattern(ui->finder, s)) {
            /* failure dialog */
            ptr = XmHTMLTextFinderGetErrorString(ui->finder);

            errmsg(ptr ? ptr : "(unknown error)");

            /* must free this */
            xfree(ptr);
            xfree(ui->last);
            ui->last = s;
            return RETURN_FAILURE;
        }
    }

    switch (XmHTMLTextFindString(ui->html, ui->finder)) {
    case XmREG_ERROR:
        ptr = XmHTMLTextFinderGetErrorString(ui->finder);
        errmsg(ptr ? ptr : "(unknown error)");
        xfree(ptr);
        break;
    case XmREG_NOMATCH:
        if (yesno("End of document reached; continue from beginning?",
                  NULL, NULL, NULL) == TRUE) {
            xfree(s);
            XCFREE(ui->last);
            return find_cb((void *) ui);
        } 
        break;
    case XmREG_MATCH:
        if (XmHTMLTextFindToPosition(ui->html, ui->finder, &start, &end)) {
            XmHTMLTextSetHighlight(ui->html, start, end, XmHIGHLIGHT_SELECTED);
            XmHTMLTextShowPosition(ui->html, start);
        }
        break;
    }

    xfree(ui->last);
    ui->last = s;
    
    return RETURN_SUCCESS;
}

static void create_find_dialog(void *data)
{
    static Widget dialog = NULL;
    html_ui *ui = (html_ui *) data;

    if (!dialog) {
        Widget rc, rc2;
        
        dialog = CreateDialogForm(ui->html, "Find Dialog");
        
        rc = CreateVContainer(dialog);
        ui->input = CreateTextInput(rc, "Find:");
        rc2 = CreateHContainer(rc);
        ui->case_sensitive = CreateToggleButton(rc2, "Case sensitive");
        ui->find_backwards = CreateToggleButton(rc2, "Find backwards (N/I)");

        CreateAACDialog(dialog, rc, find_cb, data);
        
        ManageChild(dialog);
    }

    RaiseWindow(GetParent(dialog));
}

static XmImageInfo *loadImage(Widget w,
    String url, Dimension width, Dimension height, XtPointer client_data)
{
    char *fname;
    XmImageInfo *image;
    
    if (url == NULL) {
        return NULL;
    }
    
    if (url[0] == '/') {
        fname = url;
    } else {
        char *buf, *pname, *p;
        XtVaGetValues(w, XmNuserData, &pname, NULL);
        buf = copy_string(NULL, pname);
        p = strrchr(buf, '/');
        if (p) {
            p++;
            *p = '\0';
            buf = concat_strings(buf, url);
            fname = grace_path(buf);
        } else {
            fname = url;
        }
        xfree(buf);
    }
    
    image = XmHTMLImageDefaultProc(w, fname, NULL, 0);
    
    return image;
}

void create_helper_frame(char *fname)
{
    static html_ui *ui = NULL;
    char *content, *contentp, *pname;
    
    set_wait_cursor();
    
    if (ui == NULL) {
        Widget fr1, fr2, menubar, menupane;
        
	ui = xmalloc(sizeof(html_ui));
        ui->finder = NULL;
        ui->last = NULL;
        
        ui->top = CreateDialogForm(NULL, "Gracilla");
	
        menubar = CreateMenuBar(ui->top);
        ManageChild(menubar);
        AddDialogFormChild(ui->top, menubar);
        
        menupane = CreateMenu(menubar, "File", 'F', FALSE);
        CreateMenuButton(menupane, "Close", 'C', destroy_dialog_cb, GetParent(ui->top));
        
        menupane = CreateMenu(menubar, "Edit", 'E', FALSE);
        CreateMenuButton(menupane, "Find", 'F', create_find_dialog, ui);

        menupane = CreateMenu(menubar, "Help", 'H', TRUE);
        CreateMenuButton(menupane, "User's Guide", 'G', HelpCB, "UsersGuide.html");
        CreateMenuButton(menupane, "Tutorial", 'T', HelpCB, "Tutorial.html");
        CreateMenuButton(menupane, "FAQ", 'Q', HelpCB, "FAQ.html");
        CreateMenuButton(menupane, "Changes", 'C', HelpCB, "CHANGES.html");
        CreateMenuSeparator(menupane);
        CreateMenuButton(menupane, "License terms", 'L', HelpCB, (void *) "GPL.html");
        
	fr1 = CreateFrame(ui->top, NULL);
        AddDialogFormChild(ui->top, fr1);
        ui->html = XtVaCreateManagedWidget("html",
	   xmHTMLWidgetClass, fr1,
           XmNimageProc, loadImage,
           XmNenableBadHTMLWarnings, XmHTML_NONE,
           XmNanchorButtons, False,
	   XmNmarginWidth, 20,
	   XmNmarginHeight, 20,
	   NULL);

	XtAddCallback(ui->html, XmNactivateCallback, anchorCB, NULL);
        XtAddCallback(ui->html, XmNanchorTrackCallback, trackCB, ui);

	fr2 = CreateFrame(ui->top, NULL);
        AddDialogFormChild(ui->top, fr2);
        ui->track = CreateTextInput(fr2, "Link:");
        XtVaSetValues(ui->track->text, XmNeditable, False, NULL);
        SetTextString(ui->track, "Welcome to Gracilla!");
        
        XtVaSetValues(fr1,
            XmNbottomAttachment, XmATTACH_WIDGET,
            XmNbottomWidget, fr2,
            NULL);
        XtVaSetValues(fr2,
            XmNtopAttachment, XmATTACH_NONE,
            NULL);
	
        ManageChild(ui->top);
    }
    
    if (ui->finder) {
        XmHTMLTextFinderDestroy(ui->finder);
        ui->finder = NULL;
        ui->last = NULL;
    }
    
    content = loadFile(fname);
    if (content == NULL) {
	contentp = "<html><body>Could not read given file</body></html>";
    } else {
        contentp = content;
    }

    XtVaGetValues(ui->html, XmNuserData, &pname, NULL);
    pname = copy_string(pname, fname);
    XtVaSetValues(ui->html, XmNuserData, pname, NULL);

    XmHTMLTextSetString(ui->html, contentp);
    xfree(content);

    RaiseWindow(GetParent(ui->top));
    
    unset_wait_cursor();
}
#endif /* WITH_XMHTML */
