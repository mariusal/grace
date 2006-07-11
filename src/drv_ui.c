/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1996-2006 Grace Development Team
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
 *  UI setup for print drivers
 */

#define CANVAS_BACKEND_API
#include "xprotos.h"


typedef struct {
    PS_data *ps;
    
    Widget frame;
    Widget level2;
    OptionStructure *docdata;
    OptionStructure *fonts;
    OptionStructure *colorspace;
    Widget printable;
    SpinStructure *offset_x;
    SpinStructure *offset_y;
    OptionStructure *feed;
    Widget hwres;
} PS_UI_data;

static void update_ps_setup_frame(PS_UI_data *psdata);
static int set_ps_setup_proc(void *data);

static void colorspace_cb(Widget but, int onoff, void *data)
{
    OptionStructure *opt = (OptionStructure *) data;
    
    OptionItem colorspace_ops[3] = {
        {PS_COLORSPACE_GRAYSCALE, "Grayscale"},
        {PS_COLORSPACE_RGB,       "RGB"      },
        {PS_COLORSPACE_CMYK,      "CMYK"     }
    };
    
    if (onoff) {
        UpdateOptionChoice(opt, 3, colorspace_ops);
    } else {
        UpdateOptionChoice(opt, 2, colorspace_ops);
    }
}

static void ps_gui_setup(const Canvas *canvas, void *data)
{
    PS_UI_data *ui = (PS_UI_data *) data;
    PS_data *ps = ui->ps;
    
    set_wait_cursor();
    
    if (ui->frame == NULL) {
        char *title;
        Widget ps_setup_rc, fr, rc;
        OptionItem colorspace_ops[3] = {
            {PS_COLORSPACE_GRAYSCALE, "Grayscale"},
            {PS_COLORSPACE_RGB,       "RGB"      },
            {PS_COLORSPACE_CMYK,      "CMYK"     }
        };
        OptionItem docdata_ops[3] = {
            {PS_DOCDATA_7BIT,   "7bit"  },
            {PS_DOCDATA_8BIT,   "8bit"  },
            {PS_DOCDATA_BINARY, "Binary"}
        };
        OptionItem ops[3] = {
            {PS_MEDIA_FEED_AUTO,   "Automatic" },
            {PS_MEDIA_FEED_MATCH,  "Match size"},
            {PS_MEDIA_FEED_MANUAL, "Manual"    }
        };
        OptionItem font_ops[4] = {
            {PS_FONT_EMBED_NONE,  "None"               },
            {PS_FONT_EMBED_BUT13, "All but 13 standard"},
            {PS_FONT_EMBED_BUT35, "All but 35 standard"},
            {PS_FONT_EMBED_ALL,   "All"                }
        };
        
        if (ps->format == PS_FORMAT) {
            title = "PS options";
        } else {
            title = "EPS options";
        }
	ui->frame = CreateDialogForm(app_shell, title);

        ps_setup_rc = CreateVContainer(ui->frame);

	fr = CreateFrame(ps_setup_rc, "PS options");
        rc = CreateVContainer(fr);
	ui->level2 = CreateToggleButton(rc, "PS Level 2");
        ui->colorspace =
            CreateOptionChoice(rc, "Colorspace:", 1, 3, colorspace_ops);
	AddToggleButtonCB(ui->level2,
            colorspace_cb, ui->colorspace);
	ui->docdata =
            CreateOptionChoice(rc, "Document data:", 1, 3, docdata_ops);
	ui->fonts =
            CreateOptionChoice(rc, "Embed fonts:", 1, 4, font_ops);

        if (ps->format == EPS_FORMAT) {
	    ui->printable = CreateToggleButton(rc,
                "Printable as standalone");
        }
        
        if (ps->format == PS_FORMAT) {
	    fr = CreateFrame(ps_setup_rc, "Page offsets (pt)");
            rc = CreateHContainer(fr);
	    ui->offset_x = CreateSpinChoice(rc,
                "X: ", 4, SPIN_TYPE_INT, -999.0, 999.0, 10.0);
	    ui->offset_y = CreateSpinChoice(rc,
                "Y: ", 4, SPIN_TYPE_INT, -999.0, 999.0, 10.0);

	    fr = CreateFrame(ps_setup_rc, "Hardware");
            rc = CreateVContainer(fr);
	    ui->feed = CreateOptionChoice(rc, "Media feed:", 1, 3, ops);
	    ui->hwres = CreateToggleButton(rc, "Set hardware resolution");
        }

	CreateAACDialog(ui->frame, ps_setup_rc, set_ps_setup_proc, ui);
    }
    update_ps_setup_frame(ui);
    
    RaiseWindow(GetParent(ui->frame));
    unset_wait_cursor();
}

static void update_ps_setup_frame(PS_UI_data *ui)
{
    if (ui->frame) {
        PS_data *ps = ui->ps;
        
        SetToggleButtonState(ui->level2, ps->level2);
        SetOptionChoice(ui->colorspace, ps->colorspace);
        colorspace_cb(NULL, ps->level2, ui->colorspace);
        SetOptionChoice(ui->fonts, ps->fonts);
        SetOptionChoice(ui->docdata, ps->docdata);
        if (ps->format == EPS_FORMAT) {
            SetToggleButtonState(ui->printable, ps->printable);
        }
        if (ps->format == PS_FORMAT) {
            SetSpinChoice(ui->offset_x, (double) ps->offset_x);
            SetSpinChoice(ui->offset_y, (double) ps->offset_y);
            SetOptionChoice(ui->feed, ps->feed);
            SetToggleButtonState(ui->hwres, ps->hwres);
        }
    }
}

static int set_ps_setup_proc(void *data)
{
    PS_UI_data *ui = (PS_UI_data *) data;
    PS_data *ps = ui->ps;

    ps->level2     = GetToggleButtonState(ui->level2);
    ps->docdata    = GetOptionChoice(ui->docdata);
    ps->colorspace = GetOptionChoice(ui->colorspace);
    ps->fonts      = GetOptionChoice(ui->fonts);
    if (ps->format == EPS_FORMAT) {
        ps->printable  = GetToggleButtonState(ui->printable);
    }
    if (ps->format == PS_FORMAT) {
        ps->offset_x   = (int) GetSpinChoice(ui->offset_x);
        ps->offset_y   = (int) GetSpinChoice(ui->offset_y);
        ps->feed       = GetOptionChoice(ui->feed);
        ps->hwres      = GetToggleButtonState(ui->hwres);
    }
    
    return RETURN_SUCCESS;
}

int attach_ps_drv_setup(Canvas *canvas, int device_id)
{
    dev_gui_setup *setup_data;
    PS_UI_data *ui_data;
    
    ui_data = xmalloc(sizeof(PS_UI_data));
    memset(ui_data, 0, sizeof(PS_UI_data));
    ui_data->ps = device_get_devdata(canvas, device_id);
    
    setup_data = xmalloc(sizeof(dev_gui_setup));
    setup_data->setup = ps_gui_setup;
    setup_data->ui = ui_data;
    
    device_set_udata(canvas, device_id, setup_data);

    return RETURN_SUCCESS;
}

int attach_eps_drv_setup(Canvas *canvas, int device_id)
{
    dev_gui_setup *setup_data;
    PS_UI_data *ui_data;
    
    ui_data = xmalloc(sizeof(PS_UI_data));
    memset(ui_data, 0, sizeof(PS_UI_data));
    ui_data->ps = device_get_devdata(canvas, device_id);
    
    setup_data = xmalloc(sizeof(dev_gui_setup));
    setup_data->setup = ps_gui_setup;
    setup_data->ui = ui_data;
    
    device_set_udata(canvas, device_id, setup_data);

    return RETURN_SUCCESS;
}


#ifdef HAVE_LIBXMI

typedef struct {
    PNM_data *pnm;
    
    Widget frame;
    Widget rawbits;
    OptionStructure *format;
} PNM_UI_data;

static int set_pnm_setup_proc(void *data)
{
    PNM_UI_data *ui = (PNM_UI_data *) data;
    PNM_data *pnm = ui->pnm;
    
    pnm->format = GetOptionChoice(ui->format);
    pnm->rawbits = GetToggleButtonState(ui->rawbits);
    
    return RETURN_SUCCESS;
}

static void update_pnm_setup_frame(PNM_UI_data *ui)
{
    if (ui->frame) {
        PNM_data *pnm = ui->pnm;
        SetOptionChoice(ui->format, pnm->format);
        SetToggleButtonState(ui->rawbits, pnm->rawbits);
    }
}

void pnm_gui_setup(const Canvas *canvas, void *data)
{
    PNM_UI_data *ui = (PNM_UI_data *) data;

    set_wait_cursor();
    
    if (ui->frame == NULL) {
        Widget fr, rc;
        
	ui->frame = CreateDialogForm(app_shell, "PNM options");

	fr = CreateFrame(ui->frame, "PNM options");
        rc = CreateVContainer(fr);
	ui->format = CreateOptionChoiceVA(rc, "Format: ",
            "1-bit mono (PBM)",      PNM_FORMAT_PBM,
            "8-bit grayscale (PGM)", PNM_FORMAT_PGM,
            "8-bit color (PPM)",     PNM_FORMAT_PPM,
            NULL);
	ui->rawbits = CreateToggleButton(rc, "\"Rawbits\"");

	CreateAACDialog(ui->frame, fr, set_pnm_setup_proc, ui);
    }
    update_pnm_setup_frame(ui);

    RaiseWindow(GetParent(ui->frame));
    unset_wait_cursor();
}


int attach_pnm_drv_setup(Canvas *canvas, int device_id)
{
    dev_gui_setup *setup_data;
    PNM_UI_data *ui_data;
    
    ui_data = xmalloc(sizeof(PNM_UI_data));
    memset(ui_data, 0, sizeof(PNM_UI_data));
    ui_data->pnm = device_get_devdata(canvas, device_id);
    
    setup_data = xmalloc(sizeof(dev_gui_setup));
    setup_data->setup = pnm_gui_setup;
    setup_data->ui = ui_data;
    
    device_set_udata(canvas, device_id, setup_data);

    return RETURN_SUCCESS;
}


#endif /* HAVE_LIBXMI */
