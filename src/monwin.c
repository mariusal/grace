/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2002 Grace Development Team
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
#include "parser.h"
#include "motifinc.h"
#include "protos.h"

extern XtAppContext app_con;

static void clear_results(void *data);
static void popup_on(int onoff, void *data);
static void auto_redraw_cb(int onoff, void *data);
static void auto_update_cb(int onoff, void *data);
static void create_wmon_frame(void *data);
static int save_logs_proc(char *filename, void *data);
static void cmd_cb(char *s, void *data);

typedef struct _console_ui
{
    Widget mon_frame;
    Widget monText;
    FSBStructure *save_logs_fsb;
    TextStructure *cmd;
    Storage *history;
    int eohistory;
    int popup_only_on_errors;
    int auto_redraw;
    int auto_update;
} console_ui;

static void command_hist_prev(Widget w, XEvent *e, String *par, Cardinal *npar)
{
    char *s;
    console_ui *ui = (console_ui *) GetUserData(w);
    if (!ui->eohistory) {
        storage_scroll(ui->history, -1, FALSE);
    }
    if (storage_get_data(ui->history, (void **) &s) == RETURN_SUCCESS) {
        SetTextString(ui->cmd, s);
    }
    ui->eohistory = FALSE;
}

static void command_hist_next(Widget w, XEvent *e, String *par, Cardinal *npar)
{
    char *s;
    console_ui *ui = GetUserData(w);
    if (storage_scroll(ui->history, +1, FALSE) == RETURN_SUCCESS) {
        storage_get_data(ui->history, (void **) &s);
    } else {
        ui->eohistory = TRUE;
        s = "";
    }
    SetTextString(ui->cmd, s);
}

static char command_hist_table[] = "\
    <Key>osfUp: command_hist_prev()\n\
    <Key>osfDown: command_hist_next()";

static XtActionsRec command_hist_actions[] = {
    {"command_hist_prev", command_hist_prev},
    {"command_hist_next", command_hist_next}
};

static void *wrap_str_copy(void *data)
{
    return copy_string(NULL, (char *) data);
}

/*
 * Create the mon Panel
 */
static void create_monitor_frame(int force, char *msg)
{
    static console_ui *ui = NULL;
    
    set_wait_cursor();

    if (ui == NULL) {
        Widget menubar, menupane, fr;

	ui = xmalloc(sizeof(console_ui));
        ui->mon_frame = CreateDialogForm(app_shell, "Console");
        ui->save_logs_fsb = NULL;
        ui->history = storage_new(xfree, wrap_str_copy, NULL);
        ui->eohistory = TRUE;
        ui->popup_only_on_errors = FALSE;
        ui->auto_redraw = FALSE;
        ui->auto_update = FALSE;

        menubar = CreateMenuBar(ui->mon_frame);
        ManageChild(menubar);
        AddDialogFormChild(ui->mon_frame, menubar);
        
        menupane = CreateMenu(menubar, "File", 'F', FALSE);
        CreateMenuButton(menupane, "Save...", 'S', create_wmon_frame, ui);
        CreateMenuSeparator(menupane);
        CreateMenuCloseButton(menupane, ui->mon_frame);
        
        menupane = CreateMenu(menubar, "Edit", 'E', FALSE);
        CreateMenuButton(menupane, "Clear", 'C', clear_results, ui);

        menupane = CreateMenu(menubar, "Options", 'O', FALSE);
        CreateMenuToggle(menupane, "Popup only on errors", 'e', popup_on, ui);
        CreateMenuSeparator(menupane);
        CreateMenuToggle(menupane, "Auto redraw", 'r', auto_redraw_cb, ui);
        CreateMenuToggle(menupane, "Auto update", 'p', auto_update_cb, ui);

        menupane = CreateMenu(menubar, "Help", 'H', TRUE);
        CreateMenuHelpButton(menupane, "On console", 'c',
            ui->mon_frame, "doc/UsersGuide.html#console");
	
        fr = CreateFrame(ui->mon_frame, NULL);
	ui->monText = CreateScrollTextItem2(fr, 0, "Messages:");
        XmTextSetString(ui->monText, "");
        XtVaSetValues(ui->monText, XmNeditable, False, NULL);
        AddDialogFormChild(ui->mon_frame, fr);

        fr = CreateFrame(ui->mon_frame, NULL);
        ui->cmd = CreateTextInput(fr, "Command:");
        SetUserData(ui->cmd->text, ui);
        AddTextInputCB(ui->cmd, cmd_cb, ui);
	XtOverrideTranslations(ui->cmd->text,
            XtParseTranslationTable(command_hist_table));
        XtAppAddActions(app_con,
            command_hist_actions, XtNumber(command_hist_actions));
        AddDialogFormChild(ui->mon_frame, fr);
        FixateDialogFormChild(fr);

        ManageChild(ui->mon_frame);
    }
    
    if (msg != NULL) {
        XmTextPosition pos;
        pos = XmTextGetLastPosition(ui->monText);
        XmTextInsert(ui->monText, pos, msg);
    }
    
    if (force || ui->popup_only_on_errors == FALSE) {
        RaiseWindow(GetParent(ui->mon_frame));
    }
    
    unset_wait_cursor();
}

static void popup_on(int onoff, void *data)
{
    console_ui *ui = (console_ui *) data;
    ui->popup_only_on_errors = onoff;
}

static void auto_redraw_cb(int onoff, void *data)
{
    console_ui *ui = (console_ui *) data;
    ui->auto_redraw = onoff;
}

static void auto_update_cb(int onoff, void *data)
{
    console_ui *ui = (console_ui *) data;
    ui->auto_update = onoff;
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

    if (ui->save_logs_fsb == NULL) {
        ui->save_logs_fsb = CreateFileSelectionBox(app_shell, "Save logs");
	AddFileSelectionBoxCB(ui->save_logs_fsb, save_logs_proc, ui);
        ManageChild(ui->save_logs_fsb->FSB);
    }
    
    RaiseWindow(ui->save_logs_fsb->dialog);

    unset_wait_cursor();
}

static int save_logs_proc(char *filename, void *data)
{
    console_ui *ui = (console_ui *) data;
    FILE *pp;

    pp = grace_openw(filename);

    if (pp == NULL) {
        return RETURN_FAILURE;
    } else {
        char *text = XmTextGetString(ui->monText);
        
        if (text) {
            fwrite(text, SIZEOF_CHAR, strlen(text), pp);
            XtFree(text);
        }
        
        grace_close(pp);
        
        return RETURN_SUCCESS;
    }
}

static void cmd_cb(char *s, void *data)
{
    console_ui *ui = (console_ui *) data;
    
    ui->eohistory = TRUE;
    
    if (!is_empty_string(s)) {
        scanner(s);
        
        if (ui->auto_redraw) {
            xdrawgraph();
        }

        if (ui->auto_update) {
            update_all();
        }
        
        storage_eod(ui->history);
        storage_add(ui->history, copy_string(NULL, s));
        storage_eod(ui->history);
        
        
        SetTextString(ui->cmd, "");
    }
}

void stufftextwin(char *msg)
{
    create_monitor_frame(FALSE, msg);
}

void errwin(char *msg)
{
    char *buf;
    
    buf = copy_string(NULL, "[Error] ");
    buf = concat_strings(buf, msg);
    buf = concat_strings(buf, "\n");
    
    create_monitor_frame(TRUE, buf);
    
    xfree(buf);
}

void create_monitor_frame_cb(void *data)
{
    create_monitor_frame(TRUE, NULL);
}
