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
 * Edit block data Panel
 *
 *
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <Xm/Xm.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/RowColumn.h>

#include "globals.h"
#include "graphs.h"
#include "plotone.h"
#include "utils.h"
#include "files.h"
#include "motifinc.h"
#include "protos.h"

static char ncolsbuf[128];

static int block_curtype = SET_XY;

static Widget eblock_frame;
static Widget eblock_panel;

/*
 * Panel item declarations
 */
static OptionStructure *eblock_choice_items[MAX_SET_COLS];
static Widget eblock_ncols_item;
static OptionStructure *eblock_type_choice_item;
static ListStructure *eblock_graph_choice_item;

/*
 * Event and Notify proc declarations
 */
static void eblock_type_notify_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void eblock_accept_notify_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void update_eblock(int gno);

/*
 * Create the block data panel
 */
void create_eblock_frame(int gno)
{
    int i;
    char buf[32];
    Widget rc, buts[2];

    if (blockncols == 0) {
	errwin("Need to read block data first");
	return;
    }
    set_wait_cursor();
    if (eblock_frame == NULL) {
        OptionItem blockitem;
	char *label1[2];
	label1[0] = "Accept";
	label1[1] = "Close";
        blockitem.value = 0;
        blockitem.label = "Index";
	eblock_frame = XmCreateDialogShell(app_shell, "Edit block data", NULL, 0);
	handle_close(eblock_frame);
	eblock_panel = XmCreateRowColumn(eblock_frame, "eblock_rc", NULL, 0);

	eblock_ncols_item = XtVaCreateManagedWidget("tmp", xmLabelWidgetClass, eblock_panel,
						    NULL);

        rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, eblock_panel, NULL);

	eblock_type_choice_item = CreateSetTypeChoice(rc, "Set type:");
        AddOptionChoiceCB(eblock_type_choice_item, eblock_type_notify_proc);

	for (i = 0; i < MAX_SET_COLS; i++) {
            sprintf(buf, "%s from column:", dataset_colname(i));
            eblock_choice_items[i] = CreateOptionChoice(rc, buf, 3, 1, &blockitem);
        }

	XtManageChild(rc);

	eblock_graph_choice_item = CreateGraphChoice(eblock_panel,
                                    "Load to a new set in graph:", LIST_TYPE_SINGLE);

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, eblock_panel, NULL);

	CreateCommandButtons(eblock_panel, 2, buts, label1);
	XtAddCallback(buts[0], XmNactivateCallback,
		 (XtCallbackProc) eblock_accept_notify_proc, (XtPointer) 0);
	XtAddCallback(buts[1], XmNactivateCallback,
		 (XtCallbackProc) destroy_dialog, (XtPointer) eblock_frame);

	XtManageChild(eblock_panel);
    }
    XtRaise(eblock_frame);
    update_eblock(gno);
    unset_wait_cursor();
}

/*
 * Notify and event procs
 */

static void update_eblock(int gno)
{
    static old_blockncols = 0;
    int i, ncols;
    char buf[16];
    OptionItem *blockitems;
    
    if (eblock_frame == NULL) {
	return;
    }
    if (blockncols == 0) {
	errmsg("Need to read block data first");
	return;
    }
    if (is_valid_gno(gno)) {
        SelectListChoice(eblock_graph_choice_item, gno);
    }
    sprintf(ncolsbuf, "%d columns of length %d", blockncols, blocklen);
    SetLabel(eblock_ncols_item, ncolsbuf);
    if (blockncols != old_blockncols) {
        blockitems = malloc((blockncols + 1)*sizeof(OptionItem));
        for (i = 0; i < blockncols + 1; i++) {
            blockitems[i].value = i;
            if (i == 0) {
                strcpy(buf, "Index");
            } else {
                sprintf(buf, "%d", i);
            }
            blockitems[i].label = copy_string(NULL, buf);
        }
        for (i = 0; i < MAX_SET_COLS; i++) {
            UpdateOptionChoice(eblock_choice_items[i],
                blockncols + 1, blockitems);
        }
        for (i = 0; i < blockncols + 1; i++) {
            free(blockitems[i].label);
        }
        free(blockitems);
        old_blockncols = blockncols;
    }

    ncols = settype_cols(block_curtype);
    for (i = 0; i < MAX_SET_COLS; i++) {
        XtSetSensitive(eblock_choice_items[i]->rc, (i < ncols));
    }
}

static void eblock_type_notify_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int cd = (int) client_data;

    block_curtype = cd;

    update_eblock(-1);
}

static void eblock_accept_notify_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int i, gno;
    int cs[MAX_SET_COLS];
    char blockcols[32];

    for (i = 0; i < settype_cols(block_curtype); i++) {
        cs[i] = GetOptionChoice(eblock_choice_items[i]);
        if (i == 0) {
            sprintf(blockcols, "%d", cs[i]);
        } else {
            sprintf(blockcols, "%s:%d", blockcols, cs[i]);
        }
    }
    
    if (GetSingleListChoice(eblock_graph_choice_item, &gno)
        != GRACE_EXIT_SUCCESS) {
        errmsg("Please select a single graph");
    } else {
        create_set_fromblock(gno, block_curtype, blockcols);
        update_status_popup(NULL, NULL, NULL);
        update_all();
        drawgraph();
    }
}
