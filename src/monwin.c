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

/*
 *
 * Console
 *
 */

#include <config.h>

#include <stdio.h>

#include <Xm/Xm.h>
#include <Xm/Text.h>

#include "utils.h"
#include "files.h"
#include "motifinc.h"
#include "protos.h"


static void clear_results(void *data);
static void create_wmon_frame(void *data);
static int wmon_apply_notify_proc(void *data);

typedef struct _console_ui
{
    Widget mon_frame;
    Widget monText;
    Widget wmon_frame;
    Widget wmon_text_item;
} console_ui;

/*
 * Create the mon Panel
 */
void create_monitor_frame(void *data)
{
    static console_ui *ui = NULL;
    char *msg = (char *) data;
    
    set_wait_cursor();

    if (ui == NULL) {
        Widget menubar, menupane, wbut, fr;

	ui = xmalloc(sizeof(console_ui));
        ui->mon_frame = CreateDialogForm(app_shell, "Console");
        ui->wmon_frame = NULL;

        menubar = CreateMenuBar(ui->mon_frame);
        ManageChild(menubar);
        AddDialogFormChild(ui->mon_frame, menubar);
        
        menupane = CreateMenu(menubar, "File", 'F', FALSE);
        CreateMenuButton(menupane, "Save...", 'S', create_wmon_frame, ui);
        CreateMenuSeparator(menupane);
        wbut = CreateMenuButton(menupane,
            "Close", 'C', destroy_dialog_cb, GetParent(ui->mon_frame));
        XtVaSetValues(ui->mon_frame, XmNcancelButton, wbut, NULL);
        
        menupane = CreateMenu(menubar, "Edit", 'E', FALSE);
        CreateMenuButton(menupane, "Clear", 'C', clear_results, ui);

        menupane = CreateMenu(menubar, "Help", 'H', TRUE);
        CreateMenuButton(menupane, "On console", 'c', HelpCB, NULL);
	
        fr = CreateFrame(ui->mon_frame, NULL);
        
	ui->monText = CreateScrollTextItem2(fr, 5, "Messages:");
        XmTextSetString(ui->monText, "");
        XtVaSetValues(ui->monText,
            XmNeditable, False,
            NULL);

        AddDialogFormChild(ui->mon_frame, fr);
        ManageChild(ui->mon_frame);
    }
    
    if (msg != NULL) {
        XmTextPosition pos;
        pos = XmTextGetLastPosition(ui->monText);
        XmTextInsert(ui->monText, pos, msg);
    }
    
    RaiseWindow(GetParent(ui->mon_frame));
    
    unset_wait_cursor();
}

static void clear_results(void *data)
{
    console_ui *ui = (console_ui *) data;
    XmTextSetString(ui->monText, "");
}

/*
 * Create the wmon Frame and the wmon Panel
 */
static void create_wmon_frame(void *data)
{
    console_ui *ui = (console_ui *) data;
    
    if (!ui) {
        return;
    }
    
    set_wait_cursor();
    
    if (ui->wmon_frame == NULL) {
        Widget wmon_panel;
	
        ui->wmon_frame = CreateDialogForm(app_shell, "Save logs");
	wmon_panel = CreateVContainer(ui->wmon_frame);

	ui->wmon_text_item = CreateTextItem2(wmon_panel, 30, "Save to file: ");

	CreateAACDialog(ui->wmon_frame,
            wmon_panel, wmon_apply_notify_proc, ui);
    }
    
    RaiseWindow(GetParent(ui->wmon_frame));
    unset_wait_cursor();
}

static int wmon_apply_notify_proc(void *data)
{
    console_ui *ui = (console_ui *) data;
    int len;
    char *s, *text;
    FILE *pp;

    s = xv_getstr(ui->wmon_text_item);
    pp = grace_openw(s);

    if (pp == NULL) {
        return RETURN_FAILURE;
    } else {
        text = XmTextGetString(ui->monText);
        len = XmTextGetLastPosition(ui->monText);
        
        fwrite(text, SIZEOF_CHAR, len, pp);
        
        grace_close(pp);
        XtFree(text);
        
        return RETURN_SUCCESS;
    }
}


void stufftextwin(char *msg)
{
    create_monitor_frame(msg);
}

void errwin(char *msg)
{
    char *buf;
    
    buf = copy_string(NULL, "[Error] ");
    buf = concat_strings(buf, msg);
    buf = concat_strings(buf, "\n");
    
    stufftextwin(buf);
    
    xfree(buf);
}
