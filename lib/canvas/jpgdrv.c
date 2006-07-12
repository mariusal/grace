/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
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
 * Grace JPEG driver (based upon the generic xrst driver)
 */
#include <config.h>

#if (defined(HAVE_LIBXMI) && defined(HAVE_LIBJPEG))

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define JPEG_INTERNAL_OPTIONS
#include <jpeglib.h>

#include "grace/baseP.h"
#define CANVAS_BACKEND_API
#include "grace/canvas.h"

#define JPEG_DCT_DEFAULT    JPEG_DCT_ISLOW

static int jpg_op_parser(const Canvas *canvas, void *data, const char *opstring)
{
    JPG_data *jpgdata = (JPG_data *) data;
    
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
        if (!string_is_empty(bufp)) {
            jpgdata->quality = atoi(bufp);
            return RETURN_SUCCESS;
        } else {
            return RETURN_FAILURE;
        }
    } else if (!strncmp(opstring, "smoothing:", 10)) {
        bufp = strchr(opstring, ':');
        bufp++;
        if (!string_is_empty(bufp)) {
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
    JPG_data *jpgdata = (JPG_data *) data;
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
    
    fp = canvas_get_prstream(canvas);
    w = pm->width;
    h = pm->height;
    
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, fp);
    
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

int register_jpg_drv(Canvas *canvas)
{
    XrstDevice_entry xdev;
    JPG_data *jpgdata;
    
    jpgdata = xmalloc(sizeof(JPG_data));
    if (jpgdata) {
        memset(jpgdata, 0, sizeof(JPG_data));
        jpgdata->quality     = 75;                 
        jpgdata->grayscale   = FALSE;              
        jpgdata->baseline    = FALSE;              
        jpgdata->progressive = FALSE;              
        jpgdata->optimize    = FALSE;              
        jpgdata->smoothing   = 0;                  
        jpgdata->dct         = JPEG_DCT_DEFAULT;   
        
        xdev.type     = DEVICE_FILE;
        xdev.name     = "JPEG";
        xdev.fext     = "jpg";
        xdev.fontaa   = TRUE;
        xdev.parser   = jpg_op_parser;
        xdev.dump     = jpg_output;
        xdev.devdata  = jpgdata;
        xdev.freedata = xfree;

        return register_xrst_device(canvas, &xdev);
    } else {
        return -1;
    }
}

#else
void _jpgdrv_c_dummy_func(void) {}
#endif
