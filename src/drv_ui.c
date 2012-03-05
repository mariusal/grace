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

#include <string.h>

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
	ui->frame = CreateDialog(app_shell, title);

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
    
    DialogRaise(ui->frame);
    unset_wait_cursor();
}

static void update_ps_setup_frame(PS_UI_data *ui)
{
    if (ui->frame) {
        PS_data *ps = ui->ps;
        
        ToggleButtonSetState(ui->level2, ps->level2);
        SetOptionChoice(ui->colorspace, ps->colorspace);
        colorspace_cb(NULL, ps->level2, ui->colorspace);
        SetOptionChoice(ui->fonts, ps->fonts);
        SetOptionChoice(ui->docdata, ps->docdata);
        if (ps->format == EPS_FORMAT) {
            ToggleButtonSetState(ui->printable, ps->printable);
        }
        if (ps->format == PS_FORMAT) {
            SetSpinChoice(ui->offset_x, (double) ps->offset_x);
            SetSpinChoice(ui->offset_y, (double) ps->offset_y);
            SetOptionChoice(ui->feed, ps->feed);
            ToggleButtonSetState(ui->hwres, ps->hwres);
        }
    }
}

static int set_ps_setup_proc(void *data)
{
    PS_UI_data *ui = (PS_UI_data *) data;
    PS_data *ps = ui->ps;

    ps->level2     = ToggleButtonGetState(ui->level2);
    ps->docdata    = GetOptionChoice(ui->docdata);
    ps->colorspace = GetOptionChoice(ui->colorspace);
    ps->fonts      = GetOptionChoice(ui->fonts);
    if (ps->format == EPS_FORMAT) {
        ps->printable  = ToggleButtonGetState(ui->printable);
    }
    if (ps->format == PS_FORMAT) {
        ps->offset_x   = (int) GetSpinChoice(ui->offset_x);
        ps->offset_y   = (int) GetSpinChoice(ui->offset_y);
        ps->feed       = GetOptionChoice(ui->feed);
        ps->hwres      = ToggleButtonGetState(ui->hwres);
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

#ifdef HAVE_LIBPDF

typedef struct {
    PDF_data        *pdf;
    
    Widget           frame;
    OptionStructure *compat;
    SpinStructure   *compression;
    SpinStructure   *fpprec;
    OptionStructure *colorspace;
} PDF_UI_data;

static void update_pdf_setup_frame(PDF_UI_data *ui)
{
    if (ui->frame) {
        PDF_data *pdf = ui->pdf;
        
        SetOptionChoice(ui->compat,     pdf->compat);
        SetOptionChoice(ui->colorspace, pdf->colorspace);
        SetSpinChoice(ui->compression, (double) pdf->compression);
        SetSpinChoice(ui->fpprec,      (double) pdf->fpprec);
    }
}

static int set_pdf_setup_proc(void *data)
{
    PDF_UI_data *ui = (PDF_UI_data *) data;
    PDF_data *pdf = ui->pdf;

    pdf->compat      = GetOptionChoice(ui->compat);
    pdf->colorspace  = GetOptionChoice(ui->colorspace);
    pdf->compression = (int) GetSpinChoice(ui->compression);
    pdf->fpprec      = (int) GetSpinChoice(ui->fpprec);
    
    return RETURN_SUCCESS;
}

void pdf_gui_setup(const Canvas *canvas, void *data)
{
    PDF_UI_data *ui = (PDF_UI_data *) data;

    set_wait_cursor();
    
    if (ui->frame == NULL) {
        Widget fr, rc;
        OptionItem compat_ops[3] = {
            {PDF_1_3, "PDF-1.3"},
            {PDF_1_4, "PDF-1.4"},
            {PDF_1_5, "PDF-1.5"}
        };
        OptionItem colorspace_ops[3] = {
            {PDF_COLORSPACE_GRAYSCALE, "Grayscale"},
            {PDF_COLORSPACE_RGB,       "RGB"      },
            {PDF_COLORSPACE_CMYK,      "CMYK"     }
        };
    
	ui->frame = CreateDialog(app_shell, "PDF options");

	fr = CreateFrame(ui->frame, "PDF options");
        rc = CreateVContainer(fr);
	ui->compat =
            CreateOptionChoice(rc, "Compatibility:", 1, 3, compat_ops);
        ui->colorspace =
            CreateOptionChoice(rc, "Colorspace:", 1, 3, colorspace_ops);
	ui->compression = CreateSpinChoice(rc,
            "Compression:", 1, SPIN_TYPE_INT, 0.0, 9.0, 1.0);
	ui->fpprec = CreateSpinChoice(rc,
            "FP precision:", 1, SPIN_TYPE_INT, 4.0, 6.0, 1.0);

	CreateAACDialog(ui->frame, fr, set_pdf_setup_proc, ui);
    }
    update_pdf_setup_frame(ui);
    DialogRaise(ui->frame);
    unset_wait_cursor();
}

int attach_pdf_drv_setup(Canvas *canvas, int device_id)
{
    dev_gui_setup *setup_data;
    PDF_UI_data *ui_data;
    
    ui_data = xmalloc(sizeof(PDF_UI_data));
    memset(ui_data, 0, sizeof(PDF_UI_data));
    ui_data->pdf = device_get_devdata(canvas, device_id);
    
    setup_data = xmalloc(sizeof(dev_gui_setup));
    setup_data->setup = pdf_gui_setup;
    setup_data->ui = ui_data;
    
    device_set_udata(canvas, device_id, setup_data);

    return RETURN_SUCCESS;
}

#endif /* HAVE_LIBPDF */

#ifdef HAVE_HARU

typedef struct {
    HPDF_data       *hpdf;
    
    Widget           frame;
    Widget           compression;
    OptionStructure *colorspace;
} HPDF_UI_data;

static void update_hpdf_setup_frame(HPDF_UI_data *ui)
{
    if (ui->frame) {
        HPDF_data *hpdf = ui->hpdf;
        
        SetOptionChoice(ui->colorspace, hpdf->colorspace);
        ToggleButtonSetState(ui->compression, hpdf->compression);
    }
}

static int set_hpdf_setup_proc(void *data)
{
    HPDF_UI_data *ui = (HPDF_UI_data *) data;
    HPDF_data *hpdf = ui->hpdf;

    hpdf->colorspace  = GetOptionChoice(ui->colorspace);
    hpdf->compression = ToggleButtonGetState(ui->compression);
    
    return RETURN_SUCCESS;
}

void hpdf_gui_setup(const Canvas *canvas, void *data)
{
    HPDF_UI_data *ui = (HPDF_UI_data *) data;

    set_wait_cursor();
    
    if (ui->frame == NULL) {
        Widget fr, rc;
        OptionItem colorspace_ops[3] = {
            {HPDF_COLORSPACE_GRAYSCALE, "Grayscale"},
            {HPDF_COLORSPACE_RGB,       "RGB"      },
            {HPDF_COLORSPACE_CMYK,      "CMYK"     }
        };
    
	ui->frame = CreateDialog(app_shell, "hPDF options");

	fr = CreateFrame(ui->frame, "hPDF options");
        rc = CreateVContainer(fr);
        ui->colorspace =
            CreateOptionChoice(rc, "Colorspace:", 1, 3, colorspace_ops);
	ui->compression = CreateToggleButton(rc, "Compression");

	CreateAACDialog(ui->frame, fr, set_hpdf_setup_proc, ui);
    }
    update_hpdf_setup_frame(ui);
    DialogRaise(ui->frame);
    unset_wait_cursor();
}

int attach_hpdf_drv_setup(Canvas *canvas, int device_id)
{
    dev_gui_setup *setup_data;
    HPDF_UI_data *ui_data;
    
    ui_data = xmalloc(sizeof(HPDF_UI_data));
    memset(ui_data, 0, sizeof(HPDF_UI_data));
    ui_data->hpdf = device_get_devdata(canvas, device_id);
    
    setup_data = xmalloc(sizeof(dev_gui_setup));
    setup_data->setup = hpdf_gui_setup;
    setup_data->ui = ui_data;
    
    device_set_udata(canvas, device_id, setup_data);

    return RETURN_SUCCESS;
}

#endif /* HAVE_HARU */

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
    pnm->rawbits = ToggleButtonGetState(ui->rawbits);
    
    return RETURN_SUCCESS;
}

static void update_pnm_setup_frame(PNM_UI_data *ui)
{
    if (ui->frame) {
        PNM_data *pnm = ui->pnm;
        SetOptionChoice(ui->format, pnm->format);
        ToggleButtonSetState(ui->rawbits, pnm->rawbits);
    }
}

static void pnm_gui_setup(const Canvas *canvas, void *data)
{
    PNM_UI_data *ui = (PNM_UI_data *) data;

    set_wait_cursor();
    
    if (ui->frame == NULL) {
        Widget fr, rc;
        
	ui->frame = CreateDialog(app_shell, "PNM options");

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

    DialogRaise(ui->frame);
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


#ifdef HAVE_LIBPNG
#include <zlib.h>

typedef struct {
    PNG_data *png;
    
    Widget frame;
    Widget interlaced;
    Widget transparent;
    SpinStructure *compression;
} PNG_UI_data;

static void update_png_setup_frame(PNG_UI_data *ui)
{
    if (ui->frame) {
        PNG_data *png = ui->png;
        
        ToggleButtonSetState(ui->interlaced,  png->interlaced);
        ToggleButtonSetState(ui->transparent, png->transparent);
        SetSpinChoice(ui->compression,        png->compression);
    }
}

static int set_png_setup_proc(void *data)
{
    PNG_UI_data *ui = (PNG_UI_data *) data;
    PNG_data *png = ui->png;
    
    png->interlaced  = ToggleButtonGetState(ui->interlaced);
    png->transparent = ToggleButtonGetState(ui->transparent);
    png->compression = GetSpinChoice(ui->compression);
    
    return RETURN_SUCCESS;
}

static void png_gui_setup(const Canvas *canvas, void *data)
{
    PNG_UI_data *ui = (PNG_UI_data *) data;

    set_wait_cursor();
    
    if (ui->frame == NULL) {
        Widget fr, rc;
        
	ui->frame = CreateDialog(app_shell, "PNG options");

	fr = CreateFrame(ui->frame, "PNG options");
        rc = CreateVContainer(fr);
	ui->interlaced = CreateToggleButton(rc, "Interlaced");
	ui->transparent = CreateToggleButton(rc, "Transparent");
	ui->compression = CreateSpinChoice(rc,
            "Compression:", 1, SPIN_TYPE_INT,
            (double) Z_NO_COMPRESSION, (double) Z_BEST_COMPRESSION, 1.0);

	CreateAACDialog(ui->frame, fr, set_png_setup_proc, ui);
    }
    update_png_setup_frame(ui);
    
    DialogRaise(ui->frame);
    unset_wait_cursor();
}

int attach_png_drv_setup(Canvas *canvas, int device_id)
{
    dev_gui_setup *setup_data;
    PNG_UI_data *ui_data;
    
    ui_data = xmalloc(sizeof(PNG_UI_data));
    memset(ui_data, 0, sizeof(PNG_UI_data));
    ui_data->png = device_get_devdata(canvas, device_id);
    
    setup_data = xmalloc(sizeof(dev_gui_setup));
    setup_data->setup = png_gui_setup;
    setup_data->ui = ui_data;
    
    device_set_udata(canvas, device_id, setup_data);

    return RETURN_SUCCESS;
}

#endif /* HAVE_LIBPNG */

#ifdef HAVE_LIBJPEG

typedef struct {
    JPG_data *jpg;
    
    Widget frame;
    Widget grayscale;
    Widget baseline;
    Widget optimize;
    Widget progressive;
    SpinStructure *quality;
    SpinStructure *smoothing;
    OptionStructure *dct;
} JPG_UI_data;


static void update_jpg_setup_frame(JPG_UI_data *ui)
{
    if (ui->frame) {
        JPG_data *jpg = ui->jpg;
        
        ToggleButtonSetState(ui->grayscale,   jpg->grayscale);
        ToggleButtonSetState(ui->baseline,    jpg->baseline);
        ToggleButtonSetState(ui->optimize,    jpg->optimize);
        ToggleButtonSetState(ui->progressive, jpg->progressive);
        SetSpinChoice       (ui->quality,     jpg->quality);
        SetSpinChoice       (ui->smoothing,   jpg->smoothing);
        SetOptionChoice     (ui->dct,         jpg->dct);
    }
}

static int set_jpg_setup_proc(void *data)
{
    JPG_UI_data *ui = (JPG_UI_data *) data;
    JPG_data *jpg = ui->jpg;
    
    jpg->grayscale   = ToggleButtonGetState(ui->grayscale);
    jpg->baseline    = ToggleButtonGetState(ui->baseline);
    jpg->optimize    = ToggleButtonGetState(ui->optimize);
    jpg->progressive = ToggleButtonGetState(ui->progressive);
    jpg->quality     = (int) GetSpinChoice (ui->quality);
    jpg->smoothing   = (int) GetSpinChoice (ui->smoothing);
    jpg->dct         = GetOptionChoice     (ui->dct);
    
    return RETURN_SUCCESS;
}

static void jpg_gui_setup(const Canvas *canvas, void *data)
{
    JPG_UI_data *ui = (JPG_UI_data *) data;

    set_wait_cursor();
    
    if (ui->frame == NULL) {
        Widget rc, fr, rc1;
        
	ui->frame = CreateDialog(app_shell, "JPEG options");

        rc = CreateVContainer(ui->frame);

	fr = CreateFrame(rc, "JPEG options");
        rc1 = CreateVContainer(fr);
	ui->quality = CreateSpinChoice(rc1,
            "Quality:", 3, SPIN_TYPE_INT, 0.0, 100.0, 5.0);
	ui->optimize = CreateToggleButton(rc1, "Optimize");
	ui->progressive = CreateToggleButton(rc1, "Progressive");
	ui->grayscale = CreateToggleButton(rc1, "Grayscale");

	fr = CreateFrame(rc, "JPEG advanced options");
        rc1 = CreateVContainer(fr);
	ui->smoothing = CreateSpinChoice(rc1,
            "Smoothing:", 3, SPIN_TYPE_INT, 0.0, 100.0, 10.0);
	ui->baseline = CreateToggleButton(rc1, "Force baseline");
	ui->dct = CreateOptionChoiceVA(rc, "DCT: ",
            "Fast integer", JPEG_DCT_IFAST,
            "Slow integer", JPEG_DCT_ISLOW,
            "Float",        JPEG_DCT_FLOAT,
            NULL);

	CreateAACDialog(ui->frame, rc, set_jpg_setup_proc, ui);
    }
    update_jpg_setup_frame(ui);

    DialogRaise(ui->frame);
    unset_wait_cursor();
}

int attach_jpg_drv_setup(Canvas *canvas, int device_id)
{
    dev_gui_setup *setup_data;
    JPG_UI_data *ui_data;
    
    ui_data = xmalloc(sizeof(JPG_UI_data));
    memset(ui_data, 0, sizeof(JPG_UI_data));
    ui_data->jpg = device_get_devdata(canvas, device_id);
    
    setup_data = xmalloc(sizeof(dev_gui_setup));
    setup_data->setup = jpg_gui_setup;
    setup_data->ui = ui_data;
    
    device_set_udata(canvas, device_id, setup_data);

    return RETURN_SUCCESS;
}

#endif /* HAVE_LIBJPEG */

#endif /* HAVE_LIBXMI */
