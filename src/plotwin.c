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
 * Plot properties
 *
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>

#include "globals.h"
#include "draw.h"
#include "utils.h"
#include "protos.h"
#include "motifinc.h"

static Widget plot_frame;

/*
 * Panel item declarations
 */
static OptionStructure *bg_color_item;
static Widget bg_fill_item;

static Widget timestamp_active_item;
static OptionStructure *timestamp_font_item;
static Widget timestamp_size_item;
static Widget timestamp_rotate_item;
static OptionStructure *timestamp_color_item;
Widget timestamp_x_item;
Widget timestamp_y_item;
static Widget instantupdate_item;

static int plot_define_notify_proc(void *data);
static void update_plot_items(void);


static void oc_plot_cb(int a, void *data)
{
    plot_define_notify_proc(data);
}
static void scale_plot_cb(int a, void *data)
{
    plot_define_notify_proc(data);
}
static void tb_plot_cb(int a, void *data)
{
    plot_define_notify_proc(data);
}
static void text_plot_cb(void *data)
{
    plot_define_notify_proc(data);
}

void create_plot_frame_cb(void *data)
{
    create_plot_frame();
}

void create_plot_frame(void)
{
    set_wait_cursor();
    
    if (plot_frame == NULL) {
        Widget panel, fr, rc, menubar, menupane;
    
        plot_frame = CreateDialogForm(app_shell, "Plot appearance");

        menubar = CreateMenuBar(plot_frame);
        AddDialogFormChild(plot_frame, menubar);
        ManageChild(menubar);

        menupane = CreateMenu(menubar, "File", 'F', FALSE);
        CreateMenuCloseButton(menupane, plot_frame);

        menupane = CreateMenu(menubar, "Options", 'O', FALSE);
        instantupdate_item = CreateMenuToggle(menupane, "Instantaneous update",
                            'u', NULL, NULL);
        SetToggleButtonState(instantupdate_item, grace->gui->instant_update);

        menupane = CreateMenu(menubar, "Help", 'H', TRUE);
        CreateMenuHelpButton(menupane, "On plot appearance", 'p',
            plot_frame, "doc/UsersGuide.html#plot-appearance");

        panel = CreateVContainer(plot_frame);

	fr = CreateFrame(panel, "Page background");
        rc = CreateHContainer(fr);
        bg_color_item = CreateColorChoice(rc, "Color:");
        AddOptionChoiceCB(bg_color_item, oc_plot_cb, bg_color_item);
	bg_fill_item = CreateToggleButton(rc, "Fill");
        AddToggleButtonCB(bg_fill_item, tb_plot_cb, bg_fill_item);

	fr = CreateFrame(panel, "Time stamp");
        rc = CreateVContainer(fr);

	timestamp_active_item = CreateToggleButton(rc, "Enable");
        AddToggleButtonCB(timestamp_active_item, tb_plot_cb, timestamp_active_item);
	timestamp_font_item = CreateFontChoice(rc, "Font:");
        AddOptionChoiceCB(timestamp_font_item, oc_plot_cb, timestamp_font_item);
	timestamp_color_item = CreateColorChoice(rc, "Color:");
        AddOptionChoiceCB(timestamp_color_item, oc_plot_cb, timestamp_color_item);
	timestamp_size_item = CreateCharSizeChoice(rc, "Character size");
        AddScaleCB(timestamp_size_item, scale_plot_cb, timestamp_size_item);
	timestamp_rotate_item = CreateAngleChoice(rc, "Angle");
        AddScaleCB(timestamp_rotate_item, scale_plot_cb, timestamp_rotate_item);
	timestamp_x_item = CreateTextItem2(rc, 10, "Timestamp X:");
        AddTextItemCB(timestamp_x_item, text_plot_cb, timestamp_x_item);
	timestamp_y_item = CreateTextItem2(rc, 10, "Timestamp Y:");
        AddTextItemCB(timestamp_y_item, text_plot_cb, timestamp_y_item);

	CreateAACDialog(plot_frame, panel, plot_define_notify_proc, NULL);
    }
    update_plot_items();
    
    RaiseWindow(GetParent(plot_frame));
    unset_wait_cursor();
}

static void update_plot_items(void)
{
    char buf[32];

    if (plot_frame) {
	plotstr timestamp = grace->project->timestamp;
        SetOptionChoice(bg_color_item, getbgcolor());
	SetToggleButtonState(bg_fill_item, getbgfill());

	SetToggleButtonState(timestamp_active_item, timestamp.active);
	SetOptionChoice(timestamp_font_item, timestamp.font);
	SetOptionChoice(timestamp_color_item, timestamp.color);

	SetCharSizeChoice(timestamp_size_item, timestamp.charsize);

	SetAngleChoice(timestamp_rotate_item, (int) timestamp.angle);

	sprintf(buf, "%g", timestamp.offset.x);
	xv_setstr(timestamp_x_item, buf);
	sprintf(buf, "%g", timestamp.offset.y);
	xv_setstr(timestamp_y_item, buf);
    }
}

static int plot_define_notify_proc(void *data)
{
    plotstr *timestamp = &grace->project->timestamp;
    
    if (!GetToggleButtonState(instantupdate_item) && data != NULL) {
        return RETURN_SUCCESS;
    }
    
    if (data == bg_color_item || data == NULL) {
        setbgcolor(GetOptionChoice(bg_color_item));
    }
    if (data == bg_fill_item || data == NULL) {
        setbgfill(GetToggleButtonState(bg_fill_item));
    }
    if (data == timestamp_active_item || data == NULL) {
        timestamp->active = GetToggleButtonState(timestamp_active_item);
    }
    if (data == timestamp_font_item || data == NULL) {
        timestamp->font = GetOptionChoice(timestamp_font_item);
    }
    if (data == timestamp_color_item || data == NULL) {
        timestamp->color = GetOptionChoice(timestamp_color_item);
    }
    
    if (data == timestamp_size_item || data == NULL) {
        timestamp->charsize = GetCharSizeChoice(timestamp_size_item);
    }
    
    if (data == timestamp_rotate_item || data == NULL) {
        timestamp->angle = (double) GetAngleChoice(timestamp_rotate_item);
    }
    
    if (data == timestamp_x_item || data == NULL) {
        xv_evalexpr(timestamp_x_item, &timestamp->offset.x);
    }
    if (data == timestamp_y_item || data == NULL) {
        xv_evalexpr(timestamp_y_item, &timestamp->offset.y);
    }
    
    set_dirtystate();
    xdrawgraph();
    
    return RETURN_SUCCESS;
}
