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
 * Driver for the Grace Metafile format
 */

#include <config.h>

#include <stdio.h>
#include <string.h>

#define CANVAS_BACKEND_API
#include "grace/canvas.h"

#include "devlist.h"
#include "mfdrv.h"

int register_mf_drv(Canvas *canvas)
{
    Device_entry *d;

    d = device_new("Metafile", DEVICE_FILE, TRUE, NULL, NULL);
    if (!d) {
        return -1;
    }
    
    device_set_fext(d, "gmf");
    
    device_set_procs(d,
        mf_initgraphics,
        mf_leavegraphics,
        NULL,
        NULL,
        NULL,
        mf_drawpixel,
        mf_drawpolyline,
        mf_fillpolygon,
        mf_drawarc,
        mf_fillarc,
        mf_putpixmap,
        mf_puttext);
    
    return register_device(canvas, d);
}

int mf_initgraphics(const Canvas *canvas, void *data, const CanvasStats *cstats)
{
    unsigned int i, j;
    Page_geometry *pg;
    FILE *prstream = canvas_get_prstream(canvas);

    fprintf(prstream, "#GMF-%s\n", GMF_VERSION);

    fprintf(prstream, "FontResources {\n");
    for (i = 0; i < cstats->nfonts; i++) {
        int font = cstats->fonts[i].font;
        fprintf(prstream, "\t( %d , \"%s\" , \"%s\" )\n", 
            font, get_fontalias(canvas, font), get_fontname(canvas, font));
    }
    fprintf(prstream, "}\n");

    fprintf(prstream, "ColorResources {\n");
    for (i = 0; i < cstats->ncolors; i++) {
        int cindex = cstats->colors[i];
        RGB rgb;
        get_rgb(canvas, cindex, &rgb);
        fprintf(prstream, "\t( %d , %d , %d , %d )\n", 
            cindex, rgb.red, rgb.green, rgb.blue);
    }
    fprintf(prstream, "}\n");

    fprintf(prstream, "PatternResources {\n");
    for (i = 0; i < cstats->npatterns; i++) {
        int patno = cstats->patterns[i];
        Pattern *pat = canvas_get_pattern(canvas, patno);
        fprintf(prstream, "\t( %d , ", patno);
        for (j = 0; j < pat->width*pat->height/8; j++) {
            fprintf(prstream, "%02x", pat->bits[j]);
        }
        fprintf(prstream, " )\n");
    }
    fprintf(prstream, "}\n");

    fprintf(prstream, "DashResources {\n");
    for (i = 0; i < cstats->nlinestyles; i++) {
        int lines = cstats->linestyles[i];
        LineStyle *ls = canvas_get_linestyle(canvas, lines);
        fprintf(prstream, "\t( %d , [ ", lines);
        for (j = 0; j < ls->length; j++) {
            fprintf(prstream, "%d ", ls->array[j]);
        }
        fprintf(prstream, "] )\n");
    }
    fprintf(prstream, "}\n");
    
    pg = get_page_geometry(canvas);
    fprintf(prstream, "InitGraphics { %.4f %ld %ld }\n",
        pg->dpi, pg->width, pg->height);
    
    return RETURN_SUCCESS;
}

void mf_setpen(const Canvas *canvas)
{
    Pen pen;
    FILE *prstream = canvas_get_prstream(canvas);
    
    getpen(canvas, &pen);
    fprintf(prstream, "SetPen { %d %d }\n", pen.color, pen.pattern);
}

void mf_setdrawbrush(const Canvas *canvas)
{
    FILE *prstream = canvas_get_prstream(canvas);
    fprintf(prstream, "SetLineWidth { %.4f }\n", getlinewidth(canvas));
    fprintf(prstream, "SetLineStyle { %d }\n", getlinestyle(canvas));
}

void mf_drawpixel(const Canvas *canvas, void *data, const VPoint *vp)
{
    FILE *prstream = canvas_get_prstream(canvas);
    mf_setpen(canvas);

    fprintf(prstream, "DrawPixel { ( %.4f , %.4f ) }\n", vp->x, vp->y);
}

void mf_drawpolyline(const Canvas *canvas, void *data,
    const VPoint *vps, int n, int mode)
{
    int i;
    FILE *prstream = canvas_get_prstream(canvas);
    
    mf_setpen(canvas);
    mf_setdrawbrush(canvas);
    
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

void mf_fillpolygon(const Canvas *canvas, void *data,
    const VPoint *vps, int nc)
{
    int i;
    FILE *prstream = canvas_get_prstream(canvas);
    
    mf_setpen(canvas);
    
    fprintf(prstream, "FillPolygon {\n");
    for (i = 0; i < nc; i++) {
        fprintf(prstream, "\t( %.4f , %.4f )\n", vps[i].x, vps[i].y);
    }
    fprintf(prstream, "}\n"); 
}

void mf_drawarc(const Canvas *canvas, void *data,
    const VPoint *vp1, const VPoint *vp2, double a1, double a2)
{
    FILE *prstream = canvas_get_prstream(canvas);
    mf_setpen(canvas);
    mf_setdrawbrush(canvas);
    
    fprintf(prstream,
        "DrawArc { ( %.4f , %.4f ) ( %.4f , %.4f ) %.4f %.4f }\n", 
        vp1->x, vp1->y, vp2->x, vp2->y, a1, a2);
}

void mf_fillarc(const Canvas *canvas, void *data,
    const VPoint *vp1, const VPoint *vp2, double a1, double a2, int mode)
{
    char *name;
    FILE *prstream = canvas_get_prstream(canvas);
    
    mf_setpen(canvas);
    
    /* FIXME - mode */
    if (mode == ARCFILL_CHORD) {
        name = "FillChord";
    } else {
        name = "FillPieSlice";
    }
    fprintf(prstream, "%s { ( %.4f , %.4f ) ( %.4f , %.4f ) %.4f %.4f }\n", 
        name, vp1->x, vp1->y, vp2->x, vp2->y, a1, a2);
}

void mf_putpixmap(const Canvas *canvas, void *data,
    const VPoint *vp, const CPixmap *pm)
{
    int i, j, k;
    long paddedW;
    int bit;
    char buf[16];
    FILE *prstream = canvas_get_prstream(canvas);
    
    if (pm->bpp == 1) {
        strcpy(buf, "Bitmap");
    } else {
        strcpy(buf, "Pixmap");
    }
    fprintf(prstream, "Put%s {\n", buf);
   
    if (pm->type == PIXMAP_TRANSPARENT) {
        strcpy(buf, "Transparent");
    } else {
        strcpy(buf, "Opaque");
    }
    
    fprintf(prstream, "\t( %.4f , %.4f ) %dx%d %s\n", 
                           vp->x, vp->y, pm->width, pm->height, buf);
    if (pm->bpp != 1) {
        for (k = 0; k < pm->height; k++) {
            fprintf(prstream, "\t");
            for (j = 0; j < pm->width; j++) {
                fprintf(prstream, "%02x", (pm->bits)[k*pm->width+j]);
            }
            fprintf(prstream, "\n");
        }
    } else {
        paddedW = PADBITS(pm->width, pm->pad);
        for (k = 0; k < pm->height; k++) {
            fprintf(prstream, "\t");
            for (j = 0; j < paddedW/pm->pad; j++) {
                for (i = 0; i < pm->pad; i++) {
                    bit = bin_dump(&pm->bits[k*paddedW/pm->pad + j], i, pm->pad);
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

void mf_puttext(const Canvas *canvas, void *data,
    const VPoint *vp, const char *s, int len, int font, const TextMatrix *tm,
    int underline, int overline, int kerning)
{
    int i;
    FILE *prstream = canvas_get_prstream(canvas);
    
    mf_setpen(canvas);
    
    fprintf(prstream, "PutText {\n");
    fprintf(prstream, "\t( %.4f , %.4f )\n", vp->x, vp->y); 

    fprintf(prstream, "\t %d %.4f %.4f %.4f %.4f %d %d %d %d \"", 
                        font,
                        tm->cxx, tm->cxy, tm->cyx, tm->cyy, 
                        underline, overline, kerning, len);
    for (i = 0; i < len; i++) {
        fputc(s[i], prstream);
    }
    fprintf(prstream, "\"\n");

    fprintf(prstream, "}\n"); 
}

void mf_leavegraphics(const Canvas *canvas, void *data,
    const CanvasStats *cstats)
{
    FILE *prstream = canvas_get_prstream(canvas);
    view v = cstats->bbox;
    
    fprintf(prstream, "LeaveGraphics { %.4f %.4f %.4f %.4f }\n",
        v.xv1, v.yv1, v.xv2, v.yv2);
}

