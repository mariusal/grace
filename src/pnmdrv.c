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
 * Grace PNM driver (based upon the generic xrst driver)
 */

#include <config.h>

#ifdef HAVE_LIBXMI

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defines.h"
#include "utils.h"
#include "devlist.h"
#include "draw.h"

#ifndef NONE_GUI
#  include "motifinc.h"
#endif

#include "protos.h"

/* PNM sub-formats */
#define PNM_FORMAT_PBM  0
#define PNM_FORMAT_PGM  1
#define PNM_FORMAT_PPM  2

#define DEFAULT_PNM_FORMAT PNM_FORMAT_PPM

typedef struct {
    int format;
    int rawbits;
#ifndef NONE_GUI
    Widget frame;
    Widget rawbits_item;
    OptionStructure *format_item;
#endif
} Pnm_data;

#ifndef NONE_GUI
static void pnm_gui_setup(const Canvas *canvas, void *data);
static void update_pnm_setup_frame(Pnm_data *pnmdata);
static int set_pnm_setup_proc(void *data);
#else
#  define pnm_gui_setup NULL
#endif

static int pnm_op_parser(const Canvas *canvas, void *data, const char *opstring)
{
    Pnm_data *pnmdata = (Pnm_data *) data;
    
    if (!strcmp(opstring, "rawbits:on")) {
        pnmdata->rawbits = TRUE;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "rawbits:off")) {
        pnmdata->rawbits = FALSE;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "format:pbm")) {
        pnmdata->format = PNM_FORMAT_PBM;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "format:pgm")) {
        pnmdata->format = PNM_FORMAT_PGM;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "format:ppm")) {
        pnmdata->format = PNM_FORMAT_PPM;
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

static int pnm_output(const Canvas *canvas, void *data,
    unsigned int ncolors, unsigned int *colors, Xrst_pixmap *pm)
{
    Pnm_data *pnmdata = (Pnm_data *) data;
    FILE *fp;
    int w, h;
    int i, j, k;
    int c;
    unsigned char r, g, b;
    unsigned char y, pbm_buf;
    
    fp = canvas->prstream;
    w = pm->width;
    h = pm->height;
    
    if (pnmdata->rawbits == TRUE) {
        switch (pnmdata->format) {
        case PNM_FORMAT_PBM:
            fprintf(fp, "P4\n");
            break;
        case PNM_FORMAT_PGM:
            fprintf(fp, "P5\n");
            break;
        case PNM_FORMAT_PPM:
            fprintf(fp, "P6\n");
            break;
        }
    } else {
        switch (pnmdata->format) {
        case PNM_FORMAT_PBM:
            fprintf(fp, "P1\n");
            break;
        case PNM_FORMAT_PGM:
            fprintf(fp, "P2\n");
            break;
        case PNM_FORMAT_PPM:
            fprintf(fp, "P3\n");
            break;
        }
    }
    
    fprintf(fp, "#Creator: %s\n", bi_version_string());
    
    fprintf(fp, "%d %d\n", w, h);
    
    if (pnmdata->format != PNM_FORMAT_PBM) {
        fprintf(fp, "255\n");
    }
    
    k = 0;
    pbm_buf = 0;
    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            RGB rgb;
            c = pm->matrix[i][j];
            if (get_rgb(canvas, c, &rgb) == RETURN_SUCCESS) {
                r = rgb.red   >> (GRACE_BPP - 8);
                g = rgb.green >> (GRACE_BPP - 8);
                b = rgb.blue  >> (GRACE_BPP - 8);
            } else {
                r = 0;
                g = 0;
                b = 0;
            }
            if (pnmdata->rawbits == TRUE) {
                switch (pnmdata->format) {
                case PNM_FORMAT_PBM:
                    y = (r == 255 &&  g == 255 && b == 255 ? 0x00:0x01);
                    pbm_buf |= (y << (7 - k));
                    k++;
                    /* completed byte or padding line */
                    if (k == 8 || j == w - 1) {
                        fwrite(&pbm_buf, 1, 1, fp);
                        k = 0;
                        pbm_buf = 0;
                    }
                    break;
                case PNM_FORMAT_PGM:
                    y = INTENSITY(r, g, b);
                    fwrite(&y, 1, 1, fp);
                    break;
                case PNM_FORMAT_PPM:
                    fwrite(&r, 1, 1, fp);
                    fwrite(&g, 1, 1, fp);
                    fwrite(&b, 1, 1, fp);
                    break;
                }
            } else {
                switch (pnmdata->format) {
                case PNM_FORMAT_PBM:
                    y = (r == 255 &&  g == 255 && b == 255 ? 0:1);
                    fprintf(fp, "%1d\n", y);
                    break;
                case PNM_FORMAT_PGM:
                    y = INTENSITY(r, g, b);
                    fprintf(fp, "%3d\n", y);
                    break;
                case PNM_FORMAT_PPM:
                    fprintf(fp, "%3d %3d %3d\n", r, g, b);
                    break;
                }
            }
        }
    }
    
    return RETURN_SUCCESS;
}

#ifndef NONE_GUI

void pnm_gui_setup(const Canvas *canvas, void *data)
{
    Pnm_data *pnmdata = (Pnm_data *) data;

    set_wait_cursor();
    
    if (pnmdata->frame == NULL) {
        Widget fr, rc;
        
	pnmdata->frame = CreateDialogForm(app_shell, "PNM options");

	fr = CreateFrame(pnmdata->frame, "PNM options");
        rc = CreateVContainer(fr);
	pnmdata->format_item = CreatePanelChoice(rc, "Format: ",
					 4,
					 "1-bit mono (PBM)",
					 "8-bit grayscale (PGM)",
					 "8-bit color (PPM)",
                                         0, 0);
	pnmdata->rawbits_item = CreateToggleButton(rc, "\"Rawbits\"");

	CreateAACDialog(pnmdata->frame, fr, set_pnm_setup_proc, pnmdata);
    }
    update_pnm_setup_frame(pnmdata);

    RaiseWindow(GetParent(pnmdata->frame));
    unset_wait_cursor();
}

static void update_pnm_setup_frame(Pnm_data *pnmdata)
{
    if (pnmdata->frame) {
        SetOptionChoice(pnmdata->format_item, pnmdata->format);
        SetToggleButtonState(pnmdata->rawbits_item, pnmdata->rawbits);
    }
}

static int set_pnm_setup_proc(void *data)
{
    Pnm_data *pnmdata = (Pnm_data *) data;
    
    pnmdata->format = GetOptionChoice(pnmdata->format_item);
    pnmdata->rawbits = GetToggleButtonState(pnmdata->rawbits_item);
    
    return RETURN_SUCCESS;
}

#endif

int register_pnm_drv(Canvas *canvas)
{
    XrstDevice_entry xdev;
    Pnm_data *pnmdata;
    
    pnmdata = xmalloc(sizeof(Pnm_data));
    if (pnmdata) {
        memset(pnmdata, 0, sizeof(Pnm_data));
        pnmdata->format  = DEFAULT_PNM_FORMAT;
        pnmdata->rawbits = TRUE;
        
        xdev.type   = DEVICE_FILE;
        xdev.name   = "PNM";
        xdev.fext   = "pnm";
        xdev.fontaa = TRUE;
        xdev.parser = pnm_op_parser;
        xdev.setup  = pnm_gui_setup;
        xdev.dump   = pnm_output;
        xdev.data   = pnmdata;

        return register_xrst_device(canvas, &xdev);
    } else {
        return RETURN_FAILURE;
    }
}

#else
void _pnmdrv_c_dummy_func(void) {}
#endif
