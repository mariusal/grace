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
 * monitor Panel
 *
 */

#include <config.h>

#include <stdio.h>

#include <Xm/Xm.h>
#include <Xm/Text.h>

#include "globals.h"
#include "utils.h"
#include "files.h"
#include "motifinc.h"
#include "protos.h"

static Widget mon_frame, monText;
static Widget wmon_text_item;
static Widget wmon_frame;

static void clear_results(void *data);
static void create_wmon_frame(void *data);
static int wmon_apply_notify_proc(void *data);

/*
 * Create the mon Panel
 */
void create_monitor_frame(void *data)
{
    set_wait_cursor();
    if (mon_frame == NULL) {
        Widget wbut, rc, fr;
        int ac;
        Arg args[5];

	mon_frame = CreateDialogForm(app_shell, "Results");
	
        fr = CreateFrame(mon_frame, NULL);
        AddDialogFormChild(mon_frame, fr);
        
        ac = 0;
        XtSetArg(args[ac], XmNeditMode, XmMULTI_LINE_EDIT); ac++;
        XtSetArg(args[ac], XmNrows, 10); ac++;
        XtSetArg(args[ac], XmNcolumns, 80); ac++;
        XtSetArg(args[ac], XmNwordWrap, True); ac++;
        XtSetArg(args[ac], XmNeditable, False); ac++;
	monText = XmCreateScrolledText(fr, "monText", args, ac);
        ManageChild(monText);

	rc = CreateHContainer(mon_frame);
        AddDialogFormChild(mon_frame, rc);

	wbut = CreateButton(rc, "Save...");
	AddButtonCB(wbut, create_wmon_frame, NULL);
	wbut = CreateButton(rc, "Clear");
	AddButtonCB(wbut, clear_results, NULL);
	wbut = CreateButton(rc, "Close");
	AddButtonCB(wbut, destroy_dialog_cb, GetParent(mon_frame));
	wbut = CreateButton(rc, "Help");
	AddButtonCB(wbut, HelpCB, NULL);

	ManageChild(mon_frame);
    }
    
    RaiseWindow(GetParent(mon_frame));
    unset_wait_cursor();
}

static void clear_results(void *data)
{
    XmTextSetString(monText, "");
}

void stufftextwin(char *s)
{
    XmTextPosition pos;

    create_monitor_frame(NULL);
    
    pos = XmTextGetLastPosition(monText);
    XmTextInsert(monText, pos, s);
}

/*
 * Create the wmon Frame and the wmon Panel
 */
static void create_wmon_frame(void *data)
{
    set_wait_cursor();
    
    if (wmon_frame == NULL) {
        Widget wmon_panel;
	
        wmon_frame = CreateDialogForm(app_shell, "Save logs");
	wmon_panel = CreateVContainer(wmon_frame);

	wmon_text_item = CreateTextItem2(wmon_panel, 20, "Save to file: ");
	CreateSeparator(wmon_panel);

	CreateAACDialog(wmon_frame, wmon_panel, wmon_apply_notify_proc, NULL);
        
        ManageChild(wmon_frame);
    }
    
    RaiseWindow(GetParent(wmon_frame));
    unset_wait_cursor();
}

static int wmon_apply_notify_proc(void *data)
{
    int len;
    char s[256];
    char *text;
    FILE *pp;

    strcpy(s, xv_getstr(wmon_text_item));
    pp = grace_openw(s);

    if (pp == NULL) {
        return RETURN_FAILURE;
    } else {
        text = XmTextGetString(monText);
        len = XmTextGetLastPosition(monText);
        
        fwrite(text, SIZEOF_CHAR, len, pp);
        
        grace_close(pp);
        XtFree(text);
        
        return RETURN_SUCCESS;
    }
}
