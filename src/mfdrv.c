/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1996-99 Grace Development Team
 * Copyright (c) 1991-95 Paul J Turner, Portland, OR
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
 * Driver for the GRACE Metafile format
 */

#include <config.h>

#include <stdio.h>
#include <string.h>

#include "defines.h"
#include "utils.h"
#include "draw.h"
#include "device.h"
#include "devlist.h"
#include "mfdrv.h"

extern FILE *prstream;

static Device_entry dev_mf = {DEVICE_FILE,
          "Metafile",
          mfinitgraphics,
          NULL,
          NULL,
          "gmf",
          FALSE,
          TRUE,
          {DEFAULT_PAGE_WIDTH, DEFAULT_PAGE_HEIGHT, 72.0},
          NULL
         };

int register_mf_drv(void)
{
    return register_device(dev_mf);
}

int mfinitgraphics(void)
{
    /* device-dependent routines */
    devupdatecmap   = NULL;
    
    devdrawpixel    = mf_drawpixel;
    devdrawpolyline = mf_drawpolyline;
    devfillpolygon  = mf_fillpolygon;
    devdrawarc      = mf_drawarc;
    devfillarc      = mf_fillarc;
    devputpixmap    = mf_putpixmap;
    devputtext      = mf_puttext;
    
    devleavegraphics = mf_leavegraphics;

    fprintf(prstream, "InitGraphics { }\n");
    
    return RETURN_SUCCESS;
}

void mf_setpen(void)
{
    Pen pen;
    
    pen = getpen();
    fprintf(prstream, "SetPen { %3d %3d }\n", pen.color, pen.pattern);
}

void mf_setdrawbrush(void)
{
    fprintf(prstream, "SetLineWidth { %.4f }\n", getlinewidth());
    fprintf(prstream, "SetLineStyle { %3d }\n", getlinestyle());
}

void mf_drawpixel(VPoint vp)
{
    mf_setpen();

    fprintf(prstream, "DrawPixel { ( %.4f , %.4f ) }\n", vp.x, vp.y);
}

void mf_drawpolyline(VPoint *vps, int n, int mode)
{
    int i;
    
    mf_setpen();
    mf_setdrawbrush();
    
    fprintf(prstream, "DrawPolyline {\n");
    if (mode == POLYLINE_CLOSED) {
        fprintf(prstream, "\tClosed\n");
    } else {
        fprintf(prstream, "\tOpen\n");
    }
    for (i = 0; i < n; i++) {
        fprintf(prstream, "\t( %.4f , %.4f )\n", vps[i].x, vps[i].y);
    }
    fprintf(prstream, "}\n");
}

void mf_fillpolygon(VPoint *vps, int nc)
{
    int i;
    
    mf_setpen();
    
    fprintf(prstream, "FillPolygon {\n");
    for (i = 0; i < nc; i++) {
        fprintf(prstream, "\t( %.4f , %.4f )\n", vps[i].x, vps[i].y);
    }
    fprintf(prstream, "}\n"); 
}

void mf_drawarc(VPoint vp1, VPoint vp2, int a1, int a2)
{
    mf_setpen();
    mf_setdrawbrush();
    
    fprintf(prstream, "DrawArc { ( %.4f , %.4f ) ( %.4f , %.4f ) %3d %3d }\n", 
                                   vp1.x, vp1.y,   vp2.x, vp2.y, a1, a2);
}

void mf_fillarc(VPoint vp1, VPoint vp2, int a1, int a2, int mode)
{
    char *name;
    
    mf_setpen();
    
    /* FIXME - mode */
    if (mode == ARCFILL_CHORD) {
        name = "FillChord";
    } else {
        name = "FillPieSlice";
    }
    fprintf(prstream, "%s { ( %.4f , %.4f ) ( %.4f , %.4f ) %3d %3d }\n", 
        name, vp1.x, vp1.y,   vp2.x, vp2.y, a1, a2);
}

void mf_putpixmap(VPoint vp, int width, int height, char *databits, 
                             int pixmap_bpp, int bitmap_pad, int pixmap_type)
{
    int i, j, k;
    long paddedW;
    int bit;
    char buf[16];
    
    if (pixmap_bpp == 1) {
        strcpy(buf, "Bitmap");
    } else {
        strcpy(buf, "Pixmap");
    }
    fprintf(prstream, "Put%s {\n", buf);
   
    if (pixmap_type == PIXMAP_TRANSPARENT) {
        strcpy(buf, "Transparent");
    } else {
        strcpy(buf, "Opaque");
    }
    
    fprintf(prstream, "\t( %.4f , %.4f ) %dx%d %s\n", 
                           vp.x, vp.y, width, height, buf);
    if (pixmap_bpp != 1) {
        for (k = 0; k < height; k++) {
            fprintf(prstream, "\t");
            for (j = 0; j < width; j++) {
                fprintf(prstream, "%02x", (databits)[k*width+j]);
            }
            fprintf(prstream, "\n");
        }
    } else {
        paddedW = PAD(width, bitmap_pad);
        for (k = 0; k < height; k++) {
            fprintf(prstream, "\t");
            for (j = 0; j < paddedW/bitmap_pad; j++) {
                for (i = 0; i < bitmap_pad; i++) {
                    bit = bin_dump(&databits[k*paddedW/bitmap_pad + j], i, bitmap_pad);
                    if (bit) {
                        fprintf(prstream, "X");
                    } else {
                        fprintf(prstream, ".");
                    }
                }
            } 
            fprintf(prstream, "\n");
        }
    }

    fprintf(prstream, "}\n"); 
}

void mf_puttext (VPoint start, VPoint end, double size, 
                                            CompositeString *cstring)
{
    int iglyph;
    
    mf_setpen();
    
    fprintf(prstream, "PutText {\n");
    fprintf(prstream, "\t( %.4f , %.4f ) ( %.4f , %.4f )\n", 
                                start.x, start.y, end.x, end.y); 

    iglyph = 0;
    while (cstring[iglyph].s != NULL) {
        fprintf(prstream, "\t %d %.4f %.4f %.4f %d %d \"%s\"\n", 
                            cstring[iglyph].font,
                            size * cstring[iglyph].scale,
                            size * cstring[iglyph].hshift,
                            size * cstring[iglyph].vshift,
                            cstring[iglyph].underline,
                            cstring[iglyph].overline,
                            cstring[iglyph].s);
        iglyph++;
    }

    fprintf(prstream, "}\n"); 
}

void mf_leavegraphics(void)
{
    fprintf(prstream, "LeaveGraphics { }\n");
}

