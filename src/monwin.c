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
 * monitor Panel
 *
 */

#include <config.h>

#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/Text.h>
#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>

#include "globals.h"
#include "utils.h"
#include "files.h"
#include "motifinc.h"
#include "protos.h"

extern Display *disp;

static Widget mon_frame, mon_panel, text_w;
static Widget wmon_text_item;
static Widget mon_log_item;
static Widget wmon_frame;

static void clear_results(Widget w, XtPointer client_data, XtPointer call_data);
static void log_resultsCB(Widget w, XtPointer client_data, XtPointer call_data);
static void create_wmon_frame(Widget w, XtPointer client_data, XtPointer call_data);
static void wmon_apply_notify_proc(Widget w, XtPointer client_data, XtPointer call_data);

/*
 * Create the mon Panel
 */
void create_monitor_frame(void *data)
{
    set_wait_cursor();
    if (mon_frame == NULL) {
        Widget wbut, rc, fr;

	mon_frame = XmCreateDialogShell(app_shell, "Results", NULL, 0);
	handle_close(mon_frame);
	mon_panel = XmCreateForm(mon_frame, "mon_form", NULL, 0);
	fr = CreateFrame(mon_panel, NULL);
	text_w = XmCreateScrolledText(fr, "text_w", NULL, 0);
	SetFixedFont(text_w);
	XtVaSetValues(text_w,
		      XmNrows, 10,
		      XmNcolumns, 80,
		      XmNeditMode, XmMULTI_LINE_EDIT,
		      XmNwordWrap, True,
		      NULL);
        XtManageChild(text_w);

	rc = XmCreateRowColumn(mon_panel, "rc", NULL, 0);
	XtVaSetValues(rc, XmNorientation, XmHORIZONTAL, NULL);
	wbut = XtVaCreateManagedWidget("Save...", xmPushButtonWidgetClass, rc,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, create_wmon_frame, (XtPointer) NULL);
	wbut = XtVaCreateManagedWidget("Clear", xmPushButtonWidgetClass, rc,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, clear_results, (XtPointer) NULL);
	mon_log_item = XtVaCreateManagedWidget("Log", xmToggleButtonWidgetClass, rc,
				       NULL);
	XtAddCallback(mon_log_item, XmNvalueChangedCallback, log_resultsCB, (XtPointer) NULL);
	wbut = XtVaCreateManagedWidget("Close", xmPushButtonWidgetClass, rc,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, destroy_dialog, (XtPointer) mon_frame);
	wbut = XtVaCreateManagedWidget("Help", xmPushButtonWidgetClass, rc,
				       NULL);
	AddButtonCB(wbut, HelpCB, NULL);

	XtManageChild(rc);

	XtVaSetValues(fr,
		      XmNtopAttachment, XmATTACH_FORM,
		      XmNleftAttachment, XmATTACH_FORM,
		      XmNrightAttachment, XmATTACH_FORM,
		      XmNbottomAttachment, XmATTACH_WIDGET,
		      XmNbottomWidget, rc,
		      NULL);
	XtVaSetValues(rc,
		      XmNleftAttachment, XmATTACH_FORM,
		      XmNrightAttachment, XmATTACH_FORM,
		      XmNbottomAttachment, XmATTACH_FORM,
		      NULL);

	XtManageChild(mon_panel);
    }
    XmToggleButtonSetState(mon_log_item, logwindow ? True : False, False);
    XtRaise(mon_frame);
    unset_wait_cursor();
}

static void clear_results(Widget w, XtPointer client_data, XtPointer call_data)
{
    XmTextSetString(text_w, "");
}

static void log_resultsCB(Widget w, XtPointer client_data, XtPointer call_data)
{
    logwindow = XmToggleButtonGetState(mon_log_item);
}

/*

#define STUFF_TEXT 0
#define STUFF_START 1
#define STUFF_STOP 2

 * sp = 0, just put text
 * sp = 1, place text at end initialize savepos (for sp = 2)
 * sp = 2, place text at end and go to the beginning of the sequence
 */
void stufftextwin(char *s, int sp)
{
    static XmTextPosition pos = 0; /* total character count */
    static XmTextPosition savepos = 0; /* for this sequence */

    if (inwin) {
	create_monitor_frame(NULL);
	if (sp == STUFF_START) {
	    pos = XmTextGetLastPosition(text_w);
	    savepos = pos;
	    XmTextSetTopCharacter(text_w, savepos);
	}
	XmTextInsert(text_w, pos, s);
	pos += strlen(s);
	if (sp == STUFF_STOP) {
	    XmTextSetTopCharacter(text_w, savepos);
	    savepos = pos;
	}
    } else {
	printf(s);
    }
    if (resfp != NULL) {	/* results file opened in main.c */
	fprintf(resfp, s);
    }
}

/*
 * Create the wmon Frame and the wmon Panel
 */
static void create_wmon_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget wmon_panel;
    Widget buts[2];

    unset_wait_cursor();
    if (wmon_frame == NULL) {
	char *label1[2];
	label1[0] = "Accept";
	label1[1] = "Close";
	wmon_frame = XmCreateDialogShell(app_shell, "Save", NULL, 0);
	handle_close(wmon_frame);
	wmon_panel = XmCreateRowColumn(wmon_frame, "wmon_rc", NULL, 0);

	wmon_text_item = CreateTextItem2(wmon_panel, 20, "Save to file: ");
	CreateSeparator(wmon_panel);

	CreateCommandButtons(wmon_panel, 2, buts, label1);
	XtAddCallback(buts[0], XmNactivateCallback,
		    wmon_apply_notify_proc, (XtPointer) 0);
	XtAddCallback(buts[1], XmNactivateCallback,
		   destroy_dialog, (XtPointer) wmon_frame);

	XtManageChild(wmon_panel);
    }
    XtRaise(wmon_frame);
    unset_wait_cursor();
}

static void wmon_apply_notify_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int len;
    char s[256];
    char *text;
    FILE *pp;

    strcpy(s, xv_getstr(wmon_text_item));
    pp = grace_openw(s);

    if (pp != NULL) {
        text = XmTextGetString(text_w);
        len = XmTextGetLastPosition(text_w);
        fwrite(text, SIZEOF_CHAR, len, pp);
        grace_close(pp);
        XtFree(text);
    }
}
