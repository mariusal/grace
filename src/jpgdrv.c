/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
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
 * Grace JPEG driver (based upon the generic xrst driver)
 */
#include <config.h>

#if (defined(HAVE_LIBXMI) && defined(HAVE_LIBJPEG))

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define JPEG_INTERNAL_OPTIONS
#include <jpeglib.h>

#include "defines.h"
#include "utils.h"
#include "devlist.h"
#include "draw.h"

#ifndef NONE_GUI
#  include "motifinc.h"
#endif

#include "protos.h"

#define JPEG_DCT_IFAST  0
#define JPEG_DCT_ISLOW  1
#define JPEG_DCT_FLOAT  2

#define JPEG_DCT_DEFAULT    JPEG_DCT_ISLOW


typedef struct {
    int quality;
    int grayscale;
    int baseline;
    int progressive;
    int optimize;
    int smoothing;
    int dct;
#ifndef NONE_GUI
    Widget frame;
    Widget grayscale_item;
    Widget baseline_item;
    Widget optimize_item;
    Widget progressive_item;
    SpinStructure *quality_item;
    SpinStructure *smoothing_item;
    OptionStructure *dct_item;
#endif
} Jpg_data;

#ifndef NONE_GUI
static void jpg_gui_setup(const Canvas *canvas, void *data);
static void update_jpg_setup_frame(Jpg_data *jpgdata);
static int set_jpg_setup_proc(void *data);
#else
#  define jpg_gui_setup NULL
#endif

static int jpg_op_parser(const Canvas *canvas, void *data, const char *opstring)
{
    Jpg_data *jpgdata = (Jpg_data *) data;
    
    char *bufp;
    
    if (!strcmp(opstring, "grayscale")) {
        jpgdata->grayscale = TRUE;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "color")) {
        jpgdata->grayscale = FALSE;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "optimize:on")) {
        jpgdata->optimize = TRUE;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "optimize:off")) {
        jpgdata->optimize = FALSE;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "baseline:on")) {
        jpgdata->baseline = TRUE;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "baseline:off")) {
        jpgdata->baseline = FALSE;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "progressive:on")) {
        jpgdata->progressive = TRUE;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "progressive:off")) {
        jpgdata->progressive = FALSE;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "dct:ifast")) {
        jpgdata->dct = JPEG_DCT_IFAST;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "dct:islow")) {
        jpgdata->dct = JPEG_DCT_ISLOW;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "dct:float")) {
        jpgdata->dct = JPEG_DCT_FLOAT;
        return RETURN_SUCCESS;
    } else if (!strncmp(opstring, "quality:", 8)) {
        bufp = strchr(opstring, ':');
        bufp++;
        if (!is_empty_string(bufp)) {
            jpgdata->quality = atoi(bufp);
            return RETURN_SUCCESS;
        } else {
            return RETURN_FAILURE;
        }
    } else if (!strncmp(opstring, "smoothing:", 10)) {
        bufp = strchr(opstring, ':');
        bufp++;
        if (!is_empty_string(bufp)) {
            jpgdata->smoothing = atoi(bufp);
            return RETURN_SUCCESS;
        } else {
            return RETURN_FAILURE;
        }
    } else {
        return RETURN_FAILURE;
    }
}

static int jpg_output(const Canvas *canvas, void *data,
    unsigned int ncolors, unsigned int *colors, Xrst_pixmap *pm)
{
    Jpg_data *jpgdata = (Jpg_data *) data;
    FILE *fp;
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    J_DCT_METHOD dct_method;
    JSAMPROW row_pointer;        /* pointer to a single row */
    int w, h;
    int i, j, k;
    int c;
    int r, g, b;
    unsigned char y;
    
    fp = canvas->prstream;
    w = pm->width;
    h = pm->height;
    
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, canvas->prstream);
    
    cinfo.image_width  = w;
    cinfo.image_height = h;
    if (jpgdata->grayscale) {
        cinfo.input_components = 1;
        cinfo.in_color_space = JCS_GRAYSCALE;
    } else {
        cinfo.input_components = 3;
        cinfo.in_color_space = JCS_RGB;
    }

    jpeg_set_defaults(&cinfo);

    jpeg_set_quality(&cinfo, jpgdata->quality, jpgdata->baseline);

    cinfo.smoothing_factor = jpgdata->smoothing;

    switch (jpgdata->dct) {
    case JPEG_DCT_IFAST:
        dct_method = JDCT_IFAST;
        break;
    case JPEG_DCT_ISLOW:
        dct_method = JDCT_ISLOW;
        break;
    case JPEG_DCT_FLOAT:
        dct_method = JDCT_FLOAT;
        break;
    default:
        dct_method = JDCT_DEFAULT;
    }
    cinfo.dct_method = dct_method;

    if (jpgdata->progressive) {
#ifdef C_PROGRESSIVE_SUPPORTED
        jpeg_simple_progression(&cinfo);
#else
        errmsg("jpeglib: sorry, progressive output was not compiled");
#endif
    }

    if (jpgdata->optimize) {
#ifdef ENTROPY_OPT_SUPPORTED
        cinfo.optimize_coding = TRUE;
#else
        errmsg("jpeglib: sorry, entropy optimization was not compiled");
#endif
    }

    jpeg_start_compress(&cinfo, TRUE);
    
    if (jpgdata->grayscale) {
        row_pointer = xmalloc(w);
    } else {
        row_pointer = xmalloc(3*w);
    }
    
    while ((i = cinfo.next_scanline) < h) {
        k = 0;
        for (j = 0; j < w; j++) {
            RGB rgb;
            c = pm->matrix[i][j];
            if (get_rgb(canvas, c, &rgb) == RETURN_SUCCESS) {
                r = rgb.red;
                g = rgb.green;
                b = rgb.blue;
            } else {
                r = 0;
                g = 0;
                b = 0;
            }
            if (jpgdata->grayscale) {
                y = INTENSITY(r, g, b);
                row_pointer[k++] = y;
            } else {
                row_pointer[k++] = r;
                row_pointer[k++] = g;
                row_pointer[k++] = b;
            }
        }
        jpeg_write_scanlines(&cinfo, &row_pointer, 1);
    }
    xfree(row_pointer);
    
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    
    return RETURN_SUCCESS;
}

#ifndef NONE_GUI

void jpg_gui_setup(const Canvas *canvas, void *data)
{
    Jpg_data *jpgdata = (Jpg_data *) data;

    set_wait_cursor();
    
    if (jpgdata->frame == NULL) {
        Widget rc, fr, rc1;
        
	jpgdata->frame = CreateDialogForm(app_shell, "JPEG options");

        rc = CreateVContainer(jpgdata->frame);

	fr = CreateFrame(rc, "JPEG options");
        rc1 = CreateVContainer(fr);
	jpgdata->quality_item = CreateSpinChoice(rc1,
            "Quality:", 3, SPIN_TYPE_INT, 0.0, 100.0, 5.0);
	jpgdata->optimize_item = CreateToggleButton(rc1, "Optimize");
	jpgdata->progressive_item = CreateToggleButton(rc1, "Progressive");
	jpgdata->grayscale_item = CreateToggleButton(rc1, "Grayscale");

	fr = CreateFrame(rc, "JPEG advanced options");
        rc1 = CreateVContainer(fr);
	jpgdata->smoothing_item = CreateSpinChoice(rc1,
            "Smoothing:", 3, SPIN_TYPE_INT, 0.0, 100.0, 10.0);
	jpgdata->baseline_item = CreateToggleButton(rc1, "Force baseline");
	jpgdata->dct_item = CreatePanelChoice(rc, "DCT: ",
					 4,
					 "Fast integer",
					 "Slow integer",
					 "Float",
                                         0, 0);

	CreateAACDialog(jpgdata->frame, rc, set_jpg_setup_proc, jpgdata);
    }
    update_jpg_setup_frame(jpgdata);

    RaiseWindow(GetParent(jpgdata->frame));
    unset_wait_cursor();
}

static void update_jpg_setup_frame(Jpg_data *jpgdata)
{
    if (jpgdata->frame) {
        SetToggleButtonState(jpgdata->grayscale_item, jpgdata->grayscale);
        SetToggleButtonState(jpgdata->baseline_item, jpgdata->baseline);
        SetToggleButtonState(jpgdata->optimize_item, jpgdata->optimize);
        SetToggleButtonState(jpgdata->progressive_item, jpgdata->progressive);
        SetSpinChoice(jpgdata->quality_item, jpgdata->quality);
        SetSpinChoice(jpgdata->smoothing_item, jpgdata->smoothing);
        SetOptionChoice(jpgdata->dct_item, jpgdata->dct);
    }
}

static int set_jpg_setup_proc(void *data)
{
    Jpg_data *jpgdata = (Jpg_data *) data;
    
    jpgdata->grayscale = GetToggleButtonState(jpgdata->grayscale_item);
    jpgdata->baseline = GetToggleButtonState(jpgdata->baseline_item);
    jpgdata->optimize = GetToggleButtonState(jpgdata->optimize_item);
    jpgdata->progressive = GetToggleButtonState(jpgdata->progressive_item);
    jpgdata->quality = (int) GetSpinChoice(jpgdata->quality_item);
    jpgdata->smoothing = (int) GetSpinChoice(jpgdata->smoothing_item);
    jpgdata->dct = GetOptionChoice(jpgdata->dct_item);
    
    return RETURN_SUCCESS;
}

#endif

int register_jpg_drv(Canvas *canvas)
{
    XrstDevice_entry xdev;
    Jpg_data *jpgdata;
    
    jpgdata = xmalloc(sizeof(Jpg_data));
    if (jpgdata) {
        memset(jpgdata, 0, sizeof(Jpg_data));
        jpgdata->quality     = 75;                 
        jpgdata->grayscale   = FALSE;              
        jpgdata->baseline    = FALSE;              
        jpgdata->progressive = FALSE;              
        jpgdata->optimize    = FALSE;              
        jpgdata->smoothing   = 0;                  
        jpgdata->dct         = JPEG_DCT_DEFAULT;   
        
        xdev.type   = DEVICE_FILE;
        xdev.name   = "JPEG";
        xdev.fext   = "jpg";
        xdev.fontaa = TRUE;
        xdev.parser = jpg_op_parser;
        xdev.setup  = jpg_gui_setup;
        xdev.dump   = jpg_output;
        xdev.data   = jpgdata;

        return register_xrst_device(canvas, &xdev);
    } else {
        return RETURN_FAILURE;
    }
}

#else
void _jpgdrv_c_dummy_func(void) {}
#endif
