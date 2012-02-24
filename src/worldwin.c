/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2003 Grace Development Team
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

/* 
 * Contents:
 *     arrange graphs popup
 *     overlay graphs popup
 *     autoscaling popup
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>

#include "mbitmaps.h"

#include "globals.h"
#include "core_utils.h"
#include "utils.h"
#include "motifinc.h"
#include "xprotos.h"


static int define_arrange_proc(void *data);
static int define_autos_proc(void *data);

typedef struct _Arrange_ui {
    Widget top;
    StorageStructure *frames;
    SpinStructure *nrows;
    SpinStructure *ncols;
    OptionStructure *order;
    Widget snake;
    SpinStructure *toff;
    SpinStructure *loff;
    SpinStructure *roff;
    SpinStructure *boff;
    SpinStructure *hgap;
    SpinStructure *vgap;
    Widget hpack;
    Widget vpack;
    Widget add;
    Widget kill;
} Arrange_ui;


/*
 * Arrange frames popup routines
 */
static int define_arrange_proc(void *data)
{
    Arrange_ui *ui = (Arrange_ui *) data;
    int nframes;
    Quark **frames;
    int nrows, ncols, order, snake;
    int hpack, vpack, add, kill;
    double toff, loff, roff, boff, vgap, hgap;

    nrows = (int) GetSpinChoice(ui->nrows);
    ncols = (int) GetSpinChoice(ui->ncols);
    if (nrows < 1 || ncols < 1) {
	errmsg("# of rows and columns must be > 0");
	return RETURN_FAILURE;
    }
    
    nframes = GetStorageChoices(ui->frames, &frames);
    if (nframes == 0) {
        frames = NULL;
    }
    
    order = GetOptionChoice(ui->order);
    snake = GetToggleButtonState(ui->snake);
    
    toff = GetSpinChoice(ui->toff);
    loff = GetSpinChoice(ui->loff);
    roff = GetSpinChoice(ui->roff);
    boff = GetSpinChoice(ui->boff);

    hgap = GetSpinChoice(ui->hgap);
    vgap = GetSpinChoice(ui->vgap);
    
    add  = GetToggleButtonState(ui->add);
    kill = GetToggleButtonState(ui->kill);
    
    hpack = GetToggleButtonState(ui->hpack);
    vpack = GetToggleButtonState(ui->vpack);

    if (add && nframes < nrows*ncols) {
        Quark *project = gproject_get_top(gapp->gp);
        int i;
        frames = xrealloc(frames, nrows*ncols*sizeof(Quark *));
        for (i = number_of_frames(project); nframes < nrows*ncols; nframes++, i++) {
            frames[nframes] = frame_new(project);
        }
    }
    
    if (kill && nframes > nrows*ncols) {
        for (; nframes > nrows*ncols; nframes--) {
            quark_free(frames[nframes - 1]);
        }
    }
    
    arrange_frames(frames, nframes,
        nrows, ncols, order, snake,
        loff, roff, toff, boff, vgap, hgap,
        hpack, vpack);
    
    snapshot_and_update(gapp->gp, TRUE);
    
    SelectStorageChoices(ui->frames, nframes, frames);
    xfree(frames);
    
    return RETURN_SUCCESS;
}

void hpack_cb(Widget but, int onoff, void *data)
{
    Arrange_ui *ui = (Arrange_ui *) data;
    WidgetSetSensitive(ui->hgap->rc, !onoff);
}
void vpack_cb(Widget but, int onoff, void *data)
{
    Arrange_ui *ui = (Arrange_ui *) data;
    WidgetSetSensitive(ui->vgap->rc, !onoff);
}

void create_arrange_frame(Widget but, void *data)
{
    static Arrange_ui *ui = NULL;
    set_wait_cursor();

    if (ui == NULL) {
        Widget arrange_panel, fr, gr, rc;
        BitmapOptionItem opitems[8] = {
            {0               | 0              | 0             , m_hv_lr_tb_bits},
            {0               | 0              | GA_ORDER_V_INV, m_hv_lr_bt_bits},
            {0               | GA_ORDER_H_INV | 0             , m_hv_rl_tb_bits},
            {0               | GA_ORDER_H_INV | GA_ORDER_V_INV, m_hv_rl_bt_bits},
            {GA_ORDER_HV_INV | 0              | 0             , m_vh_lr_tb_bits},
            {GA_ORDER_HV_INV | 0              | GA_ORDER_V_INV, m_vh_lr_bt_bits},
            {GA_ORDER_HV_INV | GA_ORDER_H_INV | 0             , m_vh_rl_tb_bits},
            {GA_ORDER_HV_INV | GA_ORDER_H_INV | GA_ORDER_V_INV, m_vh_rl_bt_bits}
        };
        
        ui = xmalloc(sizeof(Arrange_ui));
    
	ui->top = CreateDialogForm(app_shell, "Arrange frames");
        AddHelpCB(ui->top, "doc/UsersGuide.html#arrange-frames");

	arrange_panel = CreateVContainer(ui->top);
        
	fr = CreateFrame(arrange_panel, NULL);
        rc = CreateVContainer(fr);
        ui->frames = CreateFrameChoice(rc,
            "Arrange frames:", LIST_TYPE_MULTIPLE);
        ui->add = CreateToggleButton(rc,
            "Add frames as needed to fill the matrix");
        ui->kill = CreateToggleButton(rc, "Kill extra frames");

        fr = CreateFrame(arrange_panel, "Matrix");
        gr = CreateGrid(fr, 4, 1);
        ui->ncols = CreateSpinChoice(gr,
            "Cols:", 2, SPIN_TYPE_INT, (double) 1, (double) 99, (double) 1);
        PlaceGridChild(gr, ui->ncols->rc, 0, 0);
        ui->nrows = CreateSpinChoice(gr,
            "Rows:", 2, SPIN_TYPE_INT, (double) 1, (double) 99, (double) 1);
        PlaceGridChild(gr, ui->nrows->rc, 1, 0);
        ui->order = CreateBitmapOptionChoice(gr,
            "Order:", 2, 8, MBITMAP_WIDTH, MBITMAP_HEIGHT, opitems);
        PlaceGridChild(gr, ui->order->menu, 2, 0);
        rc = CreateHContainer(gr);
        ui->snake = CreateToggleButton(rc, "Snake fill");
        PlaceGridChild(gr, rc, 3, 0);

	fr = CreateFrame(arrange_panel, "Page offsets");
        gr = CreateGrid(fr, 3, 3);
        ui->toff = CreateSpinChoice(gr, "", 4, SPIN_TYPE_FLOAT, 0.0, 1.0, 0.05);
        PlaceGridChild(gr, ui->toff->rc, 1, 0);
        ui->loff = CreateSpinChoice(gr, "", 4, SPIN_TYPE_FLOAT, 0.0, 1.0, 0.05);
        PlaceGridChild(gr, ui->loff->rc, 0, 1);
        ui->roff = CreateSpinChoice(gr, "", 4, SPIN_TYPE_FLOAT, 0.0, 1.0, 0.05);
        PlaceGridChild(gr, ui->roff->rc, 2, 1);
        ui->boff = CreateSpinChoice(gr, "", 4, SPIN_TYPE_FLOAT, 0.0, 1.0, 0.05);
        PlaceGridChild(gr, ui->boff->rc, 1, 2);

	fr = CreateFrame(arrange_panel, "Spacing");
        gr = CreateGrid(fr, 2, 1);
        rc = CreateHContainer(gr);
        ui->hgap = CreateSpinChoice(rc,
            "Hgap/width", 3, SPIN_TYPE_FLOAT, 0.0, 9.0, 0.1);
        ui->hpack = CreateToggleButton(rc, "Pack");
        AddToggleButtonCB(ui->hpack, hpack_cb, ui);
        PlaceGridChild(gr, rc, 0, 0);
        rc = CreateHContainer(gr);
        ui->vgap = CreateSpinChoice(rc,
            "Vgap/height", 3, SPIN_TYPE_FLOAT, 0.0, 9.0, 0.1);
        ui->vpack = CreateToggleButton(rc, "Pack");
        AddToggleButtonCB(ui->vpack, vpack_cb, ui);
        PlaceGridChild(gr, rc, 1, 0);
        
	CreateAACDialog(ui->top, arrange_panel, define_arrange_proc, ui);
        
        SetSpinChoice(ui->nrows, (double) 1);
        SetSpinChoice(ui->ncols, (double) 1);
        
        SetSpinChoice(ui->toff, GA_OFFSET_DEFAULT);
        SetSpinChoice(ui->loff, GA_OFFSET_DEFAULT);
        SetSpinChoice(ui->roff, GA_OFFSET_DEFAULT);
        SetSpinChoice(ui->boff, GA_OFFSET_DEFAULT);

        SetSpinChoice(ui->hgap, GA_GAP_DEFAULT);
        SetSpinChoice(ui->vgap, GA_GAP_DEFAULT);
        
        SetToggleButtonState(ui->add, TRUE);
    }

    DialogRaise(ui->top);
    
    unset_wait_cursor();
}

/*
 * autoscale popup
 */
typedef struct _Auto_ui {
    Widget top;
    GraphSetStructure *sel;
    OptionStructure *astype;
} Auto_ui;

static int define_autos_proc(void *data)
{
    int astype, nsets;
    Auto_ui *ui = (Auto_ui *) data;
    Quark **sets;
    
    nsets = GetStorageChoices(ui->sel->set_sel, &sets);
    if (nsets <= 0) {
        errmsg("No sets selected");
        return RETURN_FAILURE;
    }

    astype = GetOptionChoice(ui->astype);
    
    autoscale_bysets(sets, nsets, astype);
    
    xfree(sets);
    
    snapshot_and_update(gapp->gp, TRUE);
    
    return RETURN_SUCCESS;
}

void create_autos_frame(Widget but, void *data)
{
    static Auto_ui *aui = NULL;

    set_wait_cursor();
    
    if (aui == NULL) {
	Widget rc;
        
        aui = xmalloc(sizeof(Auto_ui));
        
        aui->top = CreateDialogForm(app_shell, "Autoscale graphs");

	rc = CreateVContainer(aui->top);
        aui->sel = CreateGraphSetSelector(rc, NULL, LIST_TYPE_MULTIPLE);
        aui->astype = CreateASChoice(rc, "Autoscale type:");

	CreateAACDialog(aui->top, rc, define_autos_proc, aui);
    }
    
    DialogRaise(aui->top);
    unset_wait_cursor();
}
