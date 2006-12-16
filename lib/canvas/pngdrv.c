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
 * Grace PNG driver (based upon the generic xrst driver)
 */
#include <config.h>

#if (defined(HAVE_LIBXMI) && defined(HAVE_LIBPNG))

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <png.h>

#include "grace/baseP.h"
#define CANVAS_BACKEND_API
#include "grace/canvas.h"

static int png_op_parser(const Canvas *canvas, void *data, const char *opstring)
{
    PNG_data *pngdata = (PNG_data *) data;
    
    char *bufp;
    
    if (!strcmp(opstring, "interlaced:on")) {
        pngdata->interlaced = TRUE;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "interlaced:off")) {
        pngdata->interlaced = FALSE;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "transparent:on")) {
        pngdata->transparent = TRUE;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "transparent:off")) {
        pngdata->transparent = FALSE;
        return RETURN_SUCCESS;
    } else if (!strncmp(opstring, "compression:", 12)) {
        bufp = strchr(opstring, ':');
        bufp++;
        if (!string_is_empty(bufp)) {
            pngdata->compression = atoi(bufp);
            return RETURN_SUCCESS;
        } else {
            return RETURN_FAILURE;
        }
    } else {
        return RETURN_FAILURE;
    }
}

static int png_output(const Canvas *canvas, void *data,
    unsigned int ncolors, unsigned int *colors, Xrst_pixmap *pm)
{
    PNG_data *pngdata = (PNG_data *) data;
    FILE *fp;
    png_structp png_ptr;
    png_infop info_ptr;
    unsigned int w, h;
    int interlace_type;
    unsigned int i, j;
    int bg;
    png_byte *trans;
    int num_text;
    png_text text_ptr[4];
    char *s;
    png_byte **image;
    png_uint_32 res_meter;
    
    fp = canvas_get_prstream(canvas);
    w = pm->width;
    h = pm->height;
    
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
        NULL, NULL, NULL);
    if (png_ptr == NULL) {
        return RETURN_FAILURE;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
        png_destroy_write_struct(&png_ptr, NULL);
        return RETURN_FAILURE;
    }

    if (setjmp(png_ptr->jmpbuf)) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return RETURN_FAILURE;
    }

    png_init_io(png_ptr, fp);

    /* set the zlib compression level */
    png_set_compression_level(png_ptr, pngdata->compression);

    if (pngdata->interlaced) {
        interlace_type = PNG_INTERLACE_ADAM7;
    } else {
        interlace_type = PNG_INTERLACE_NONE;
    }

    png_set_IHDR(png_ptr, info_ptr, w, h,
        8, PNG_COLOR_TYPE_RGB, interlace_type,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT);

    trans = xmalloc(ncolors*sizeof(png_byte));
    if (trans == NULL) {
        return RETURN_FAILURE;
    }
    
    bg = getbgcolor(canvas);
    
    res_meter = (png_uint_32) rint(page_dpi(canvas)/MM_PER_INCH*1000.0);
    png_set_pHYs(png_ptr, info_ptr, res_meter, res_meter, PNG_RESOLUTION_METER);

#ifdef PNG_WRITE_tRNS_SUPPORTED
    if (pngdata->transparent) {
        png_set_tRNS(png_ptr, info_ptr, trans, ncolors, NULL);
    }
#endif
    
#if (defined(PNG_WRITE_tEXt_SUPPORTED) || defined(PNG_WRITE_zTXt_SUPPORTED))
    text_ptr[0].key         = "Title";
    text_ptr[0].text        = canvas_get_docname(canvas);
    text_ptr[0].compression = PNG_TEXT_COMPRESSION_NONE;
    text_ptr[1].key         = "Author";
    text_ptr[1].text        = canvas_get_username(canvas);
    text_ptr[1].compression = PNG_TEXT_COMPRESSION_NONE;
    text_ptr[2].key         = "Software";
    text_ptr[2].text        = "Grace/libcanvas";
    text_ptr[2].compression = PNG_TEXT_COMPRESSION_NONE;
    num_text = 3;
    s = canvas_get_description(canvas);
    if (!string_is_empty(s)) {
        text_ptr[3].key         = "Description";
        text_ptr[3].text        = s;
        if (strlen(s) > 1024) {
            text_ptr[3].compression = PNG_TEXT_COMPRESSION_zTXt;
        } else {
            text_ptr[3].compression = PNG_TEXT_COMPRESSION_NONE;
        }
        num_text++;
    }
    png_set_text(png_ptr, info_ptr, text_ptr, num_text);
#endif
    
    png_write_info(png_ptr, info_ptr);
    
    /* allocate image of byte-sized pixels */
    image = xmalloc(h*SIZEOF_VOID_P);
    for (i = 0; i < h; i++) {
        image[i] = xmalloc(w*3*sizeof(png_byte));
        for (j = 0; j < w; j++) {
            RGB rgb;
            int r, g, b;
            if (get_rgb(canvas, pm->matrix[i][j], &rgb) == RETURN_SUCCESS) {
                r = rgb.red;
                g = rgb.green;
                b = rgb.blue;
            } else {
                r = 0;
                g = 0;
                b = 0;
            }
            image[i][3*j + 0] = r;
            image[i][3*j + 1] = g;
            image[i][3*j + 2] = b;
        }
    }
    
    png_write_image(png_ptr, image);
    
    png_write_end(png_ptr, info_ptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    xfree(trans);
    
    /* free the tmp image */
    for (i = 0; i < h; i++) {
        xfree(image[i]);
    }
    xfree(image);
    
    return RETURN_SUCCESS;
}

int register_png_drv(Canvas *canvas)
{
    XrstDevice_entry xdev;
    PNG_data *pngdata;
    
    pngdata = xmalloc(sizeof(PNG_data));
    if (pngdata) {
        memset(pngdata, 0, sizeof(PNG_data));
        pngdata->interlaced = FALSE;
        pngdata->transparent = FALSE;
        pngdata->compression = 4;
        
        xdev.type     = DEVICE_FILE;
        xdev.name     = "PNG";
        xdev.fext     = "png";
        xdev.fontaa   = TRUE;
        xdev.parser   = png_op_parser;
        xdev.dump     = png_output;
        xdev.devdata  = pngdata;
        xdev.freedata = xfree;

        return register_xrst_device(canvas, &xdev);
    } else {
        return -1;
    }
}

#else
void _pngdrv_c_dummy_func(void) {}
#endif
