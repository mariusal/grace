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

#include <config.h>

#include <stdlib.h>
#include <string.h>

#include "globals.h"
#include "utils.h"
#include "files.h"
#include "xprotos.h"

#include "motifinc.h"

#define NO_HELP "doc/nohelp.html"

#ifdef WITH_XMHTML
#  include <XmHTML/XmHTML.h>
void create_helper_frame(char *fname);
#endif

void HelpCB(Widget w, void *data)
{
    char *URL, *ha;
    int remote;

    ha = (char *) data;
    
    if (ha == NULL) {
        ha = NO_HELP;
    }
    
    if (strstr(ha, "http:") || strstr(ha, "ftp:") || strstr(ha, "mailto:")) {
        URL = copy_string(NULL, ha);
        remote = TRUE;
    } else {
        char *p, *pa;
        
        if (ha == strstr(ha, "file:")) {
            p = (ha + 5);
        } else {
            p = ha;
        }

        pa = strchr(p, '#');
        if (pa) {
            char *base = copy_string(NULL, p);
            base[pa - p] = '\0';
            URL = grace_path(gapp->grace, base);
            URL = concat_strings(URL, pa);
            xfree(base);
        } else {
            URL = grace_path(gapp->grace, p);
        }

        remote = FALSE;
    }
    
    if (remote || gapp->gui->force_external_viewer) {
        char *help_viewer, *command;
        int i, j, len, urllen, comlen;
        
        help_viewer = get_help_viewer(gapp);
        len = strlen(help_viewer);
        urllen = strlen(URL);
        for (i = 0, comlen = len; i < len - 1; i++) {
    	    if ((help_viewer[i] == '%') && (help_viewer[i + 1] == 's')){
    	        comlen += urllen - 2;
    	        i++;
    	    }
        }
        command = xmalloc((comlen + 1)*SIZEOF_CHAR);
        command[comlen] = '\0';
        for (i = 0, j = 0; i < len; i++) {
    	    if ((help_viewer[i] == '%') && (help_viewer[i + 1] == 's')){
    	        strcpy (&command[j], URL);
    	        j += urllen;
    	        i++;
    	    } else {
    	        command[j++] = help_viewer[i];
    	    }
        }
#ifdef VMS    
        system_spawn(command);
#else
        command = concat_strings(command, "&");    
        system_wrap(command);
#endif
        xfree(command);
    } else {
#ifdef WITH_XMHTML
        create_helper_frame(URL);
#endif
    }
    
    xfree(URL);
}

/*
 * say a few things about Grace
 */
static Widget about_frame;

void create_about_grtool(Widget but, void *data)
{
    set_wait_cursor();

    if (about_frame == NULL) {
        Widget wbut, fr, rc, about_panel;
        char buf[1024];

        about_frame = CreateDialog(app_shell, "About");

        about_panel = CreateVContainer(about_frame);
        FormAddVChild(about_frame, about_panel);

        fr = CreateFrame(about_panel, NULL);
        rc = CreateVContainer(fr);
        CreateLabel(rc, bi_version_string());

        fr = CreateFrame(about_panel, "Legal stuff");
        rc = CreateVContainer(fr);
        CreateLabel(rc, "Copyright (c) 1996-2007 Grace Development Team");
        CreateLabel(rc, "Portions Copyright (c) 1991-1995 Paul J Turner");
        CreateLabel(rc, "Maintained by Evgeny Stambulchik");
        CreateLabel(rc, "All rights reserved");
        CreateLabel(rc,
                "The program is distributed under the terms of the GNU General Public License");

#if defined(MOTIF_GUI) || defined(HAVE_LIBPDF)
        fr = CreateFrame(about_panel, "Third party copyrights");
        rc = CreateVContainer(fr);
#ifdef MOTIF_GUI
        CreateLabel(rc,
                "Tab widget, Copyright (c) 1997 Pralay Dakua");
        CreateLabel(rc, "Xbae widget,");
        CreateLabel(rc,
                "      Copyright (c) 1991, 1992 Bell Communications Research, Inc. (Bellcore)");
        CreateLabel(rc,
                "      Copyright (c) 1995-1999 Andrew Lister");
#endif
#ifdef HAVE_LIBPDF
        CreateLabel(rc, "PDFlib library, Copyright (c) 1997-2004 Thomas Merz and PDFlib GmbH");
#endif
#endif

        fr = CreateFrame(about_panel, "Build info");
        rc = CreateVContainer(fr);
        sprintf(buf, "Host: %s", bi_system());
        CreateLabel(rc, buf);
        sprintf(buf, "Time: %s", bi_date());
        CreateLabel(rc, buf);
        sprintf(buf, "GUI toolkit: %s ", bi_gui());
        CreateLabel(rc, buf);
#ifdef MOTIF_GUI
        sprintf(buf, "Xbae version: %s ", bi_gui_xbae());
        CreateLabel(rc, buf);
#endif
        sprintf(buf, "T1lib: %s ", bi_t1lib());
        CreateLabel(rc, buf);

        fr = CreateFrame(about_panel, "Home page");
        rc = CreateVContainer(fr);
        CreateLabel(rc, "http://plasma-gate.weizmann.ac.il/Grace/");

        CreateSeparator(about_panel);

        wbut = CreateButton(about_panel, "Close");
        AddButtonCB(wbut, destroy_dialog_cb, about_frame);

        WidgetManage(about_frame);
    }

    DialogRaise(about_frame);

    unset_wait_cursor();
}


#ifdef WITH_XMHTML
/*
 * Simplistic HTML viewer
 */

typedef struct _html_ui {
    Widget top;
    Widget html;
    TextStructure *location;
    Widget track;
    
    char *url;
    char *base;
    char *anchor;
    
    TextStructure *input;
    Widget case_sensitive;
    Widget find_backwards;
    
    XmHTMLTextFinder finder;
    char *last;
} html_ui;

static char *loadFile(char *URL)
{
    FILE *file;
    int size;
    char *content;

    /* open the given file */
    if ((file = gapp_openr(gapp, URL, SOURCE_DISK)) == NULL) {
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

    gapp_close(file);

    /* sanity */
    content[size] = '\0';

    return content;
}

static char *translateURL(char *url, char *base)
{
    char *fname;
    URLType type;
    
    if (url == NULL) {
        return NULL;
    }
    
    type = XmHTMLGetURLType(url);
    if (type != ANCHOR_FILE_LOCAL || url[0] == '/') {
        fname = copy_string(NULL, url);
    } else {
        char *p;
        fname = copy_string(NULL, base);
        p = strrchr(fname, '/');
        if (p) {
            p++;
            *p = '\0';
            fname = concat_strings(fname, url);
        } else {
            fname = copy_string(NULL, url);
        }
    }
    
    return fname;
}

static void anchorCB(Widget w, XtPointer client_data, XtPointer call_data)
{
    int id;
    XmHTMLAnchorPtr href_data = (XmHTMLAnchorPtr) call_data;
    html_ui *ui = (html_ui *) client_data;
    char *turl;
    
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
            
            ui->url = copy_string(ui->url, ui->base);
            ui->url = concat_strings(ui->url, href_data->href);
            TextSetString(ui->location, ui->url);
        }
        break;
    /* let HelpCB check all other types */
    default:
        turl = translateURL(href_data->href, ui->base);
        HelpCB(NULL, turl);
        xfree(turl);
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
        LabelSetString(ui->track, href_data->href);
    } else {
        /* a valid anchor, eg, moving away from an anchor */
        LabelSetString(ui->track, "");
    }
}

static int find_cb(void *data)
{
    char *s, *ptr;
    int case_sensitive, find_backwards;
    XmHTMLTextPosition start, end;
    html_ui *ui = (html_ui *) data;
    
    if (!ui->input) {
        return RETURN_FAILURE;
    }
    
    s = TextGetString(ui->input);

    if (string_is_empty(s)) {
        xfree(s);
        return RETURN_FAILURE;
    }
    
    if (ui->finder == NULL) {
        ui->finder = XmHTMLTextFinderCreate(ui->html);
        if (ui->finder == NULL) {
            errmsg("XmHTMLTextFinderCreate failed!");
            xfree(s);
            return RETURN_FAILURE;
        }
    }

    case_sensitive = ToggleButtonGetState(ui->case_sensitive);
    find_backwards = ToggleButtonGetState(ui->find_backwards);

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
            return find_cb(ui);
        } 
        break;
    case XmREG_MATCH:
        if (XmHTMLTextFindToPosition(ui->html, ui->finder, &start, &end)) {
            XmHTMLTextShowPosition(ui->html, start);
            XmHTMLTextSetHighlight(ui->html, start, end, XmHIGHLIGHT_SELECTED);
        }
        break;
    }

    xfree(ui->last);
    ui->last = s;
    
    return RETURN_SUCCESS;
}

static void create_find_dialog(Widget but, void *data)
{
    static Widget dialog = NULL;
    html_ui *ui = (html_ui *) data;

    if (!dialog) {
        Widget rc, rc2;
        
        dialog = CreateDialog(ui->html, "Find Dialog");
        
        rc = CreateVContainer(dialog);
        ui->input = CreateText(rc, "Find:");
        rc2 = CreateHContainer(rc);
        ui->case_sensitive = CreateToggleButton(rc2, "Case sensitive");
        ui->find_backwards = CreateToggleButton(rc2, "Find backwards (N/I)");

        CreateAACDialog(dialog, rc, find_cb, data);
        
        WidgetManage(dialog);
    }

    DialogRaise(dialog);
}

static void refresh_cb(Widget but, void *data)
{
    html_ui *ui = (html_ui *) data;
    XmHTMLRedisplay(ui->html);
}

static XmImageInfo *loadImage(Widget w,
    String url, Dimension width, Dimension height, XtPointer client_data)
{
    char *fname;
    XmImageInfo *image;
    html_ui *ui = (html_ui *) client_data;
    
    fname = translateURL(url, ui->base);
    if (fname == NULL) {
        return NULL;
    }
    
    image = XmHTMLImageDefaultProc(w, fname, NULL, 0);
    
    xfree(fname);
    
    return image;
}

void location_cb(TextStructure *cst, char *s, void *data)
{
    HelpCB(NULL, s);
}

static void find_again_cb(Widget but, void *data)
{
    find_cb(data);
}

void create_helper_frame(char *URL)
{
    static html_ui *ui = NULL;
    char *content;
    
    set_wait_cursor();
    
    if (ui == NULL) {
        Widget fr1, fr2, menubar, menupane, rc;
        
	ui = xmalloc(sizeof(html_ui));
        memset(ui, 0, sizeof(html_ui));
        
        ui->top = CreateDialog(app_shell, "Gracilla");
	
        menubar = CreateMenuBar(ui->top);
        WidgetManage(menubar);
        FormAddVChild(ui->top, menubar);
        
        menupane = CreateMenu(menubar, "File", 'F', FALSE);
        CreateMenuButtonA(menupane, "Close", 'C', "<Key>Escape", "Esc", destroy_dialog_cb, ui->top);
        
        menupane = CreateMenu(menubar, "Edit", 'E', FALSE);
        CreateMenuButtonA(menupane, "Find", 'F', "Ctrl<Key>f", "Ctrl+F", create_find_dialog, ui);
        CreateMenuButtonA(menupane, "Find again", 'g', "Ctrl<Key>g", "Ctrl+G", find_again_cb, ui);

        menupane = CreateMenu(menubar, "View", 'V', FALSE);
        CreateMenuButton(menupane, "Refresh", 'R', refresh_cb, ui);

        menupane = CreateMenu(menubar, "Help", 'H', TRUE);
        CreateMenuButton(menupane, "User's Guide", 'G', HelpCB, "doc/UsersGuide.html");
        CreateMenuButton(menupane, "FAQ", 'Q', HelpCB, "doc/FAQ.html");
        CreateMenuButton(menupane, "Changes", 'C', HelpCB, "doc/NEWS.html");
        CreateMenuSeparator(menupane);
        CreateMenuButton(menupane, "License terms", 'L', HelpCB, "doc/GPL.html");

        ui->location = CreateText(ui->top, "Location:");
        AddTextActivateCB(ui->location, location_cb, ui->location);
        FormAddVChild(ui->top, ui->location->form);
        
	fr1 = CreateFrame(ui->top, NULL);
	FormAddVChild(ui->top, fr1);
        ui->html = XtVaCreateManagedWidget("html",
            xmHTMLWidgetClass, fr1,
            XmNimageProc, loadImage,
            XmNclientData, (XtPointer) ui,
            XmNenableBadHTMLWarnings, XmHTML_NONE,
            XmNanchorButtons, False,
            XmNmarginWidth, 20,
            XmNmarginHeight, 20,
            NULL);

	XtAddCallback(ui->html, XmNactivateCallback, anchorCB, ui);
        XtAddCallback(ui->html, XmNanchorTrackCallback, trackCB, ui);

	fr2 = CreateFrame(ui->top, NULL);
	FormAddVChild(ui->top, fr2);
        rc = CreateVContainer(fr2);
        ui->track = CreateLabel(rc, "Welcome to Gracilla!");
        
        XtVaSetValues(fr1,
            XmNbottomAttachment, XmATTACH_WIDGET,
            XmNbottomWidget, fr2,
            NULL);
        XtVaSetValues(fr2,
            XmNtopAttachment, XmATTACH_NONE,
            NULL);
	
        WidgetManage(ui->top);
        
        XtVaSetValues(rc, XmNresizeHeight, False, NULL);
        
        DialogRaise(ui->top);
    }
    
    ui->url  = copy_string(ui->url, URL);
    ui->base = copy_string(ui->base, URL);
    if (ui->url) {
        char *p;
        
        p = strchr(ui->url, '#');
        if (p) {
            ui->base[p - ui->url] = '\0';
            ui->anchor = copy_string(ui->anchor, p);
        } else {
            XCFREE(ui->anchor);
        }
    }
    
    TextSetString(ui->location, ui->url);

    if (ui->finder) {
        XmHTMLTextFinderDestroy(ui->finder);
        ui->finder = NULL;
        ui->last = NULL;
    }
    
    content = loadFile(ui->base);
    if (content != NULL) {
        XmHTMLTextSetString(ui->html, content);
        if (ui->anchor) {
            int id = XmHTMLAnchorGetId(ui->html, ui->anchor);
            if (id != -1) {
                XmHTMLAnchorScrollToId(ui->html, id);
            }
        } else {
            XmHTMLTextScrollToLine(ui->html, 0);
        }
        xfree(content);

        DialogRaise(ui->top);
    }
    
    unset_wait_cursor();
}
#endif /* WITH_XMHTML */
