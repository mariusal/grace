/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1996-2001 Grace Development Team
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
 *  GRACE PostScript driver
 */

#include <config.h>
#include <cmath.h>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "defines.h"
#include "utils.h"
#include "draw.h"
#include "device.h"
#include "devlist.h"
#include "patterns.h"
#include "psdrv.h"
#include "protos.h"

#ifndef NONE_GUI
#  include "motifinc.h"
#endif

static void put_string(FILE *fp, const char *s, int len);

static int curformat = DEFAULT_PS_FORMAT;

static unsigned long page_scale;
static double pixel_size;
static float page_scalef;
static int page_orientation;

static int *psfont_status = NULL;

static int ps_color;
static int ps_pattern;
static double ps_linew;
static int ps_lines;
static int ps_linecap;
static int ps_linejoin;

static int ps_level2 = TRUE;
static PSColorSpace ps_colorspace = DEFAULT_COLORSPACE;
static int docdata = DOCDATA_8BIT;

static int ps_setup_offset_x = 0;
static int ps_setup_offset_y = 0;

static int ps_setup_level2 = TRUE;
static int ps_setup_colorspace = DEFAULT_COLORSPACE;
static int ps_setup_docdata = DOCDATA_8BIT;

static int ps_setup_feed = MEDIA_FEED_AUTO;
static int ps_setup_hwres = FALSE;

static int eps_setup_level2 = TRUE;
static int eps_setup_colorspace = DEFAULT_COLORSPACE;
static int eps_setup_tight_bb = TRUE;
static int eps_setup_docdata = DOCDATA_8BIT;

static int tight_bb;

static Device_entry dev_ps = {DEVICE_PRINT,
          "PostScript",
          "ps",
          TRUE,
          FALSE,
          {3300, 2550, 300.0},

          psprintinitgraphics,
          ps_op_parser,
          ps_gui_setup,
          NULL,
          ps_leavegraphics,
          ps_drawpixel,
          ps_drawpolyline,
          ps_fillpolygon,
          ps_drawarc,
          ps_fillarc,
          ps_putpixmap,
          ps_puttext,

          NULL
         };

static Device_entry dev_eps = {DEVICE_FILE,
          "EPS",
          "eps",
          TRUE,
          FALSE,
          {2500, 2500, 300.0},

          epsinitgraphics,
          eps_op_parser,
          eps_gui_setup,
          NULL,
          ps_leavegraphics,
          ps_drawpixel,
          ps_drawpolyline,
          ps_fillpolygon,
          ps_drawarc,
          ps_fillarc,
          ps_putpixmap,
          ps_puttext,

          NULL
         };

int register_ps_drv(Canvas *canvas)
{
    return register_device(canvas, &dev_ps);
}

int register_eps_drv(Canvas *canvas)
{
    return register_device(canvas, &dev_eps);
}

static int ps_initgraphics(const Canvas *canvas, int format)
{
    int i, j;
    Page_geometry *pg;
    int width_pp, height_pp, page_offset_x, page_offset_y;
    char **enc;
    
    time_t time_value;
    
    curformat = format;
    
    pg = get_page_geometry(canvas);
    
    page_scale = MIN2(pg->height, pg->width);
    pixel_size = 1.0/page_scale;
    page_scalef = (float) page_scale*72.0/pg->dpi;

    if (curformat == PS_FORMAT && pg->height < pg->width) {
        page_orientation = PAGE_ORIENT_LANDSCAPE;
    } else {
        page_orientation = PAGE_ORIENT_PORTRAIT;
    }
    
    /* undefine all graphics state parameters */
    ps_color = -1;
    ps_pattern = -1;
    ps_linew = -1.0;
    ps_lines = -1;
    ps_linecap = -1;
    ps_linejoin = -1;
    
    /* CMYK is a PS2 feature */
    if (ps_level2 == FALSE && ps_colorspace == COLORSPACE_CMYK) {
        ps_colorspace = COLORSPACE_RGB;
    }

    /* Font status table */
    if (psfont_status != NULL) {
        xfree(psfont_status);
    }
    psfont_status = xmalloc(number_of_fonts(canvas)*SIZEOF_INT);
    for (i = 0; i < number_of_fonts(canvas); i++) {
        psfont_status[i] = FALSE;
    }
    
    switch (curformat) {
    case PS_FORMAT:
        fprintf(canvas->prstream, "%%!PS-Adobe-3.0\n");
        tight_bb = FALSE;
        page_offset_x = ps_setup_offset_x;
        page_offset_y = ps_setup_offset_y;
        break;
    case EPS_FORMAT:
        fprintf(canvas->prstream, "%%!PS-Adobe-3.0 EPSF-3.0\n");
        tight_bb = eps_setup_tight_bb;
        page_offset_x = 0;
        page_offset_y = 0;
        break;
    default:
        errmsg("Invalid PS format");
        return RETURN_FAILURE;
    }
    
    if (page_orientation == PAGE_ORIENT_LANDSCAPE) {
        width_pp  = (int) rint(72.0*pg->height/pg->dpi);
        height_pp = (int) rint(72.0*pg->width/pg->dpi);
    } else {
        width_pp  = (int) rint(72.0*pg->width/pg->dpi);
        height_pp = (int) rint(72.0*pg->height/pg->dpi);
    }
    
    if (tight_bb == TRUE) {
        fprintf(canvas->prstream, "%%%%BoundingBox: (atend)\n");
    } else {
        fprintf(canvas->prstream, "%%%%BoundingBox: %d %d %d %d\n", 
            page_offset_x, page_offset_y,
            width_pp + page_offset_x, height_pp + page_offset_y);
    }
    
    if (ps_level2 == TRUE) {
        fprintf(canvas->prstream, "%%%%LanguageLevel: 2\n");
    } else {
        fprintf(canvas->prstream, "%%%%LanguageLevel: 1\n");
    }
    
    fprintf(canvas->prstream, "%%%%Creator: %s\n", bi_version_string());

    time(&time_value);
    fprintf(canvas->prstream, "%%%%CreationDate: %s", ctime(&time_value));
    switch (docdata) {
    case DOCDATA_7BIT:
        fprintf(canvas->prstream, "%%%%DocumentData: Clean7Bit\n");
        break;
    case DOCDATA_8BIT:
        fprintf(canvas->prstream, "%%%%DocumentData: Clean8Bit\n");
        break;
    default:
        fprintf(canvas->prstream, "%%%%DocumentData: Binary\n");
        break;
    }
    if (page_orientation == PAGE_ORIENT_LANDSCAPE) {
        fprintf(canvas->prstream, "%%%%Orientation: Landscape\n");
    } else {
        fprintf(canvas->prstream, "%%%%Orientation: Portrait\n");
    }
    
    if (curformat == PS_FORMAT) {
        fprintf(canvas->prstream, "%%%%Pages: 1\n");
        fprintf(canvas->prstream, "%%%%PageOrder: Ascend\n");
    }
    fprintf(canvas->prstream, "%%%%Title: %s\n", canvas_get_docname(canvas));
    fprintf(canvas->prstream, "%%%%For: %s\n", canvas_get_username(canvas));
    fprintf(canvas->prstream, "%%%%DocumentNeededResources: (atend)\n");
    fprintf(canvas->prstream, "%%%%EndComments\n");

    /* Definitions */
    fprintf(canvas->prstream, "%%%%BeginProlog\n");
    if (curformat == PS_FORMAT) {
        fprintf(canvas->prstream, "/PAGE_OFFSET_X %d def\n", page_offset_x);
        fprintf(canvas->prstream, "/PAGE_OFFSET_Y %d def\n", page_offset_y);
    }
    fprintf(canvas->prstream, "/m {moveto} def\n");
    fprintf(canvas->prstream, "/l {lineto} def\n");
    fprintf(canvas->prstream, "/s {stroke} def\n");
    fprintf(canvas->prstream, "/n {newpath} def\n");
    fprintf(canvas->prstream, "/c {closepath} def\n");
    fprintf(canvas->prstream, "/RL {rlineto} def\n");
    fprintf(canvas->prstream, "/SLW {setlinewidth} def\n");
    fprintf(canvas->prstream, "/GS {gsave} def\n");
    fprintf(canvas->prstream, "/GR {grestore} def\n");
    fprintf(canvas->prstream, "/SC {setcolor} def\n");
    fprintf(canvas->prstream, "/SGRY {setgray} def\n");
    fprintf(canvas->prstream, "/SRGB {setrgbcolor} def\n");
    if (ps_colorspace == COLORSPACE_CMYK) {
        fprintf(canvas->prstream, "/SCMYK {setcmykcolor} def\n");
    }
    fprintf(canvas->prstream, "/SD {setdash} def\n");
    fprintf(canvas->prstream, "/SLC {setlinecap} def\n");
    fprintf(canvas->prstream, "/SLJ {setlinejoin} def\n");
    fprintf(canvas->prstream, "/SCS {setcolorspace} def\n");
    fprintf(canvas->prstream, "/FFSF {findfont setfont} def\n");
    fprintf(canvas->prstream, "/CC {concat} def\n");
    fprintf(canvas->prstream, "/PXL {n m 0 0 RL s} def\n");
    
    for (i = 0; i < number_of_colors(canvas); i++) {
        fprintf(canvas->prstream,"/Color%d {", i);
        switch (ps_colorspace) {
        case COLORSPACE_GRAYSCALE:
            fprintf(canvas->prstream,"%.4f", get_colorintensity(canvas, i));
            break;
        case COLORSPACE_RGB:
            {
                fRGB frgb;
                if (get_frgb(canvas, i, &frgb) == RETURN_SUCCESS) {
                    fprintf(canvas->prstream, "%.4f %.4f %.4f",
                                      frgb.red, frgb.green, frgb.blue);
                }
            }
            break;
        case COLORSPACE_CMYK:
            {
                fCMYK fcmyk;
                if (get_fcmyk(canvas, i, &fcmyk) == RETURN_SUCCESS) {
                    fprintf(canvas->prstream, "%.4f %.4f %.4f %.4f",
                                      fcmyk.cyan, fcmyk.magenta,
                                      fcmyk.yellow, fcmyk.black);
                }
            }
            break;
        }
        fprintf(canvas->prstream,"} def\n");
    }
       
    if (ps_level2 == TRUE) {
        fprintf(canvas->prstream, "/PTRN {\n");
        fprintf(canvas->prstream, " /pat_bits exch def \n");
        fprintf(canvas->prstream, " <<\n");
        fprintf(canvas->prstream, "  /PaintType 2\n");
        fprintf(canvas->prstream, "  /PatternType 1 /TilingType 1\n");
        fprintf(canvas->prstream, "  /BBox[0 0 16 16]\n");
        fprintf(canvas->prstream, "  /XStep 16 /YStep 16\n");
        fprintf(canvas->prstream, "  /PaintProc {\n");
        fprintf(canvas->prstream, "   pop\n");
        fprintf(canvas->prstream, "   16 16 true [-1 0 0 -1 16 16] pat_bits imagemask\n");
        fprintf(canvas->prstream, "  }\n");
        fprintf(canvas->prstream, " >>\n");
        fprintf(canvas->prstream, " [%.4f 0 0 %.4f 0 0]\n", 1.0/page_scalef, 1.0/page_scalef);
        fprintf(canvas->prstream, " makepattern\n");
        fprintf(canvas->prstream, "} def\n");
        for (i = 0; i < number_of_patterns(canvas); i++) {
            fprintf(canvas->prstream, "/Pattern%d {<", i);
            for (j = 0; j < 32; j++) {
                fprintf(canvas->prstream, "%02x", pat_bits[i][j]);
            }
            fprintf(canvas->prstream, "> PTRN} bind def\n");
        }
    }
    
    /* Elliptic arc */
    fprintf(canvas->prstream, "/ellipsedict 8 dict def\n");
    fprintf(canvas->prstream, "ellipsedict /mtrx matrix put\n");
    fprintf(canvas->prstream, "/EARC {\n");
    fprintf(canvas->prstream, " ellipsedict begin\n");
    fprintf(canvas->prstream, "  /endangle exch def\n");
    fprintf(canvas->prstream, "  /startangle exch def\n");
    fprintf(canvas->prstream, "  /yrad exch def\n");
    fprintf(canvas->prstream, "  /xrad exch def\n");
    fprintf(canvas->prstream, "  /y exch def\n");
    fprintf(canvas->prstream, "  /x exch def\n");
    fprintf(canvas->prstream, "  /savematrix mtrx currentmatrix def\n");
    fprintf(canvas->prstream, "  x y translate\n");
    fprintf(canvas->prstream, "  xrad yrad scale\n");
    fprintf(canvas->prstream, "  0 0 1 startangle endangle arc\n");
    fprintf(canvas->prstream, "  savematrix setmatrix\n");
    fprintf(canvas->prstream, " end\n");
    fprintf(canvas->prstream, "} def\n");

    /* Text under/overlining etc */
    fprintf(canvas->prstream, "/TL {\n");
    fprintf(canvas->prstream, "  /kcomp exch def\n");
    fprintf(canvas->prstream, "  /linewidth exch def\n");
    fprintf(canvas->prstream, "  /offset exch def\n");
    fprintf(canvas->prstream, "  GS\n");
    fprintf(canvas->prstream, "  0 offset rmoveto\n");
    fprintf(canvas->prstream, "  linewidth SLW\n");
    fprintf(canvas->prstream, "  dup stringwidth exch kcomp add exch RL s\n");
    fprintf(canvas->prstream, "  GR\n");
    fprintf(canvas->prstream, "} def\n");

    /* Kerning stuff */
    fprintf(canvas->prstream, "/KINIT\n");
    fprintf(canvas->prstream, "{\n");
    fprintf(canvas->prstream, " /kvector exch def\n");
    fprintf(canvas->prstream, " /kid 0 def\n");
    fprintf(canvas->prstream, "} def\n");
    fprintf(canvas->prstream, "/KPROC\n");
    fprintf(canvas->prstream, "{\n");
    fprintf(canvas->prstream, " pop pop\n");
    fprintf(canvas->prstream, " kvector kid get\n");
    fprintf(canvas->prstream, " 0 rmoveto\n");
    fprintf(canvas->prstream, " /kid 1 kid add def\n");
    fprintf(canvas->prstream, "} def\n");

    /* Default encoding */
    enc = get_default_encoding(canvas);
    fprintf(canvas->prstream, "/DefEncoding [\n");
    for (i = 0; i < 256; i++) {
        fprintf(canvas->prstream, " /%s\n", enc[i]);
    }
    fprintf(canvas->prstream, "] def\n");

    fprintf(canvas->prstream, "%%%%EndProlog\n");

    fprintf(canvas->prstream, "%%%%BeginSetup\n");
    if (ps_level2 == TRUE && curformat == PS_FORMAT) {
        /* page size feed */
        switch (ps_setup_feed) {
        case MEDIA_FEED_AUTO:
            break;
        case MEDIA_FEED_MATCH:
            fprintf(canvas->prstream, "%%%%BeginFeature: *PageSize\n");
            fprintf(canvas->prstream,
                "<</PageSize [%d %d] /ImagingBBox null>> setpagedevice\n",
                width_pp, height_pp);
            fprintf(canvas->prstream, "%%%%EndFeature\n");
            break;
        case MEDIA_FEED_MANUAL:
            fprintf(canvas->prstream, "%%%%BeginFeature: *ManualFeed\n");
            fprintf(canvas->prstream, "<</ManualFeed true>> setpagedevice\n");
            fprintf(canvas->prstream, "%%%%EndFeature\n");
            break;
        }
        
        /* force HW resolution */
        if (ps_setup_hwres == TRUE) {
            fprintf(canvas->prstream, "%%%%BeginFeature: *HWResolution\n");
            fprintf(canvas->prstream, "<</HWResolution [%d %d]>> setpagedevice\n",
                (int) pg->dpi, (int) pg->dpi);
            fprintf(canvas->prstream, "%%%%EndFeature\n");
        }
    }
    
    /* compensate for printer page offsets */
    if (curformat == PS_FORMAT) {
        fprintf(canvas->prstream, "PAGE_OFFSET_X PAGE_OFFSET_Y translate\n");
    }
    fprintf(canvas->prstream, "%.2f %.2f scale\n", page_scalef, page_scalef);
    /* rotate to get landscape on hardcopy */
    if (page_orientation == PAGE_ORIENT_LANDSCAPE) {
        fprintf(canvas->prstream, "90 rotate\n");
        fprintf(canvas->prstream, "0.0 -1.0 translate\n");
    }
    fprintf(canvas->prstream, "%%%%EndSetup\n");

    if (curformat == PS_FORMAT) {
        fprintf(canvas->prstream, "%%%%Page: 1 1\n");
    }

    return RETURN_SUCCESS;
}

void ps_setpen(const Canvas *canvas, const Pen *pen)
{
    if (pen->color != ps_color || pen->pattern != ps_pattern) {
        if (ps_level2 == TRUE) {
            if (pen->pattern == 1) {
                switch (ps_colorspace) {
                case COLORSPACE_GRAYSCALE:
                    fprintf(canvas->prstream, "[/DeviceGray] SCS\n");
                    break;
                case COLORSPACE_RGB:
                    fprintf(canvas->prstream, "[/DeviceRGB] SCS\n");
                    break;
                case COLORSPACE_CMYK:
                    fprintf(canvas->prstream, "[/DeviceCMYK] SCS\n");
                    break;
                }
                fprintf(canvas->prstream, "Color%d SC\n", pen->color);
            } else {
                switch (ps_colorspace) {
                case COLORSPACE_GRAYSCALE:
                    fprintf(canvas->prstream, "[/Pattern /DeviceGray] SCS\n");
                    break;
                case COLORSPACE_RGB:
                    fprintf(canvas->prstream, "[/Pattern /DeviceRGB] SCS\n");
                    break;
                case COLORSPACE_CMYK:
                    fprintf(canvas->prstream, "[/Pattern /DeviceCMYK] SCS\n");
                    break;
                }
                fprintf(canvas->prstream,
                    "Color%d Pattern%d SC\n", pen->color, pen->pattern);
            }
        } else {
            if (ps_colorspace == COLORSPACE_GRAYSCALE) {
                fprintf(canvas->prstream, "Color%d SGRY\n", pen->color);
            } else {
                fprintf(canvas->prstream, "Color%d SRGB\n", pen->color);
            }
        }
        ps_color = pen->color;
        ps_pattern = pen->pattern;
    }
}

void ps_setdrawbrush(const Canvas *canvas)
{
    int i;
    int ls;
    double lw;
    Pen pen;
    
    getpen(canvas, &pen);
    ps_setpen(canvas, &pen);

    ls = getlinestyle(canvas);
    lw = MAX2(getlinewidth(canvas), pixel_size);
    
    if (ls != ps_lines || lw != ps_linew) {    
        fprintf(canvas->prstream, "[");
        if (ls > 1) {
            for (i = 0; i < dash_array_length[ls]; i++) {
                fprintf(canvas->prstream, "%.4f ", lw*dash_array[ls][i]);
            }
        }
        fprintf(canvas->prstream, "] 0 SD\n");
        fprintf(canvas->prstream, "%.4f SLW\n", lw);
        ps_linew = lw;
        ps_lines = ls;
    }
}

void ps_setlineprops(const Canvas *canvas)
{
    int lc, lj;
    
    lc = getlinecap(canvas);
    lj = getlinejoin(canvas);
    
    if (lc != ps_linecap) {
        switch (lc) {
        case LINECAP_BUTT:
            fprintf(canvas->prstream, "0 SLC\n");
            break;
        case LINECAP_ROUND:
            fprintf(canvas->prstream, "1 SLC\n");
            break;
        case LINECAP_PROJ:
            fprintf(canvas->prstream, "2 SLC\n");
            break;
        }
        ps_linecap = lc;
    }

    if (lj != ps_linejoin) {
        switch (lj) {
        case LINEJOIN_MITER:
            fprintf(canvas->prstream, "0 SLJ\n");
            break;
        case LINEJOIN_ROUND:
            fprintf(canvas->prstream, "1 SLJ\n");
            break;
        case LINEJOIN_BEVEL:
            fprintf(canvas->prstream, "2 SLJ\n");
            break;
        }
        ps_linejoin = lj;
    }
}

void ps_drawpixel(const Canvas *canvas, const VPoint *vp)
{
    Pen pen;
    getpen(canvas, &pen);
    ps_setpen(canvas, &pen);
    
    if (ps_linew != pixel_size) {
        fprintf(canvas->prstream, "%.4f SLW\n", pixel_size);
        ps_linew = pixel_size;
    }
    if (ps_linecap != LINECAP_ROUND) {
        fprintf(canvas->prstream, "1 SLC\n");
        ps_linecap = LINECAP_ROUND;
    }
    if (ps_lines != 1) {
        fprintf(canvas->prstream, "[] 0 SD\n");
        ps_lines = 1;
    }
    
    fprintf(canvas->prstream, "%.4f %.4f PXL\n", vp->x, vp->y);
}

void ps_drawpolyline(const Canvas *canvas, const VPoint *vps, int n, int mode)
{
    int i;
    
    ps_setdrawbrush(canvas);
    
    ps_setlineprops(canvas);
    
    fprintf(canvas->prstream, "n\n");
    fprintf(canvas->prstream, "%.4f %.4f m\n", vps[0].x, vps[0].y);
    for (i = 1; i < n; i++) {
        fprintf(canvas->prstream, "%.4f %.4f l\n", vps[i].x, vps[i].y);
    }
    if (mode == POLYLINE_CLOSED) {
        fprintf(canvas->prstream, "%.4f %.4f l\n", vps[0].x, vps[0].y);
        fprintf(canvas->prstream, "c\n");
    }
    fprintf(canvas->prstream, "s\n");
}

void ps_fillpolygon(const Canvas *canvas, const VPoint *vps, int nc)
{
    int i;
    Pen pen;
    getpen(canvas, &pen);
    
    if (pen.pattern == 0 || nc < 3) {
        return;
    }
    
    fprintf(canvas->prstream, "n\n");
    fprintf(canvas->prstream, "%.4f %.4f m\n", vps[0].x, vps[0].y);
    for (i = 1; i < nc; i++) {
        fprintf(canvas->prstream, "%.4f %.4f l\n", vps[i].x, vps[i].y);
    }
    fprintf(canvas->prstream, "c\n");

    /* fill bg first if the pattern != solid */
    if (pen.pattern != 1 && ps_level2 == TRUE) {
        Pen bgpen;
        bgpen.color   = getbgcolor(canvas);
        bgpen.pattern = 1;
        fprintf(canvas->prstream, "GS\n");
        ps_setpen(canvas, &bgpen);
        fprintf(canvas->prstream, "fill\n");
        fprintf(canvas->prstream, "GR\n");
    }
    
    getpen(canvas, &pen);
    ps_setpen(canvas, &pen);
    if (getfillrule(canvas) == FILLRULE_WINDING) {
        fprintf(canvas->prstream, "fill\n");
    } else {
        fprintf(canvas->prstream, "eofill\n");
    }
}

void ps_drawarc(const Canvas *canvas,
    const VPoint *vp1, const VPoint *vp2, int a1, int a2)
{
    VPoint vpc;
    double rx, ry;
    
    ps_setdrawbrush(canvas);

    vpc.x = (vp1->x + vp2->x)/2;
    vpc.y = (vp1->y + vp2->y)/2;
    rx = fabs(vp2->x - vp1->x)/2;
    ry = fabs(vp2->y - vp1->y)/2;
    
    fprintf(canvas->prstream, "n %.4f %.4f %.4f %.4f %d %d EARC s\n",
                       vpc.x, vpc.y, rx, ry, a1, a2);
}

void ps_fillarc(const Canvas *canvas,
    const VPoint *vp1, const VPoint *vp2, int a1, int a2, int mode)
{
    VPoint vpc;
    double rx, ry;
    Pen pen;
    getpen(canvas, &pen);
    
    if (pen.pattern == 0) {
        return;
    }

    vpc.x = (vp1->x + vp2->x)/2;
    vpc.y = (vp1->y + vp2->y)/2;
    rx = fabs(vp2->x - vp1->x)/2;
    ry = fabs(vp2->y - vp1->y)/2;
    
    fprintf(canvas->prstream, "n\n");
    
    if (mode == ARCFILL_PIESLICE) {
        fprintf(canvas->prstream, "%.4f %.4f m\n", vpc.x, vpc.y);
    }
    fprintf(canvas->prstream, "%.4f %.4f %.4f %.4f %d %d EARC c\n",
                       vpc.x, vpc.y, rx, ry, a1, a2);

    /* fill bg first if the pattern != solid */
    if (pen.pattern != 1 && ps_level2 == TRUE) {
        Pen bgpen;
        bgpen.color   = getbgcolor(canvas);
        bgpen.pattern = 1;
        fprintf(canvas->prstream, "GS\n");
        ps_setpen(canvas, &bgpen);
        fprintf(canvas->prstream, "fill\n");
        fprintf(canvas->prstream, "GR\n");
    }

    getpen(canvas, &pen);
    ps_setpen(canvas, &pen);
    fprintf(canvas->prstream, "fill\n");
}

void ps_putpixmap(const Canvas *canvas,
    const VPoint *vp, int width, int height, char *databits,
    int pixmap_bpp, int bitmap_pad, int pixmap_type)
{
    int j, k;
    int cindex;
    int paddedW;
    fRGB frgb;
    fCMYK fcmyk;
    unsigned char tmpbyte;
    Pen pen;

    getpen(canvas, &pen);
    ps_setpen(canvas, &pen);
    
    fprintf(canvas->prstream, "GS\n");
    fprintf(canvas->prstream, "%.4f %.4f translate\n", vp->x, vp->y);
    fprintf(canvas->prstream, "%.4f %.4f scale\n", (float) width/page_scale, 
                                           (float) height/page_scale);    
    if (pixmap_bpp != 1) {
        int layers = 1, bpp = 8;
        if (pixmap_type == PIXMAP_TRANSPARENT) {
            /* TODO: mask */
        }
        switch (ps_colorspace) {
        case COLORSPACE_GRAYSCALE:
            layers = 1;
            bpp = 8;
            break;
        case COLORSPACE_RGB:
            layers = 3;
            bpp = GRACE_BPP;
            break;
        case COLORSPACE_CMYK:
            layers = 4;
            bpp = GRACE_BPP;
            break;
        }
        fprintf(canvas->prstream, "/picstr %d string def\n", width*layers);
        fprintf(canvas->prstream, "%d %d %d\n", width, height, bpp);
        fprintf(canvas->prstream, "[%d 0 0 %d 0 0]\n", width, -height);
        fprintf(canvas->prstream, "{currentfile picstr readhexstring pop}\n");
        if (ps_colorspace == COLORSPACE_GRAYSCALE || ps_level2 == FALSE) {
            /* No color images in Level1 */
            fprintf(canvas->prstream, "image\n");
        } else {
            fprintf(canvas->prstream, "false %d\n", layers);
            fprintf(canvas->prstream, "colorimage\n");
        }
        for (k = 0; k < height; k++) {
            for (j = 0; j < width; j++) {
                cindex = (databits)[k*width+j];
                if (ps_colorspace == COLORSPACE_GRAYSCALE ||
                    ps_level2 == FALSE) {
                    fprintf(canvas->prstream,"%02x",
                        (int) (255*get_colorintensity(canvas, cindex)));
                } else {
                    if (ps_colorspace == COLORSPACE_CMYK) {
                        CMYK cmyk;
                        get_cmyk(canvas, cindex, &cmyk);
                        fprintf(canvas->prstream, "%02x%02x%02x%02x",
                                          cmyk.cyan, cmyk.magenta,
                                          cmyk.yellow, cmyk.black);
                    } else {
                        RGB rgb;
                        get_rgb(canvas, cindex, &rgb);
                        fprintf(canvas->prstream, "%02x%02x%02x",
                                           rgb.red, rgb.green, rgb.blue);
                    }
                }
            }
            fprintf(canvas->prstream, "\n");
        }
    } else { /* monocolor bitmap */
        paddedW = PAD(width, bitmap_pad);
        if (pixmap_type == PIXMAP_OPAQUE) {
            cindex = getbgcolor(canvas);
            switch (ps_colorspace) {
            case COLORSPACE_GRAYSCALE:
                fprintf(canvas->prstream,"%.4f SGRY\n",
                    get_colorintensity(canvas, cindex));
                break;
            case COLORSPACE_RGB:
                get_frgb(canvas, cindex, &frgb);
                fprintf(canvas->prstream,"%.4f %.4f %.4f SRGB\n",
                                  frgb.red, frgb.green, frgb.blue);
                break;
            case COLORSPACE_CMYK:
                get_fcmyk(canvas, cindex, &fcmyk);
                fprintf(canvas->prstream, "%.4f %.4f %.4f %.4f SCMYK\n",
                                  fcmyk.cyan, fcmyk.magenta,
                                  fcmyk.yellow, fcmyk.black);
                break;
            }
            fprintf(canvas->prstream, "0 0 1 -1 rectfill\n");
        }
        cindex = getcolor(canvas);
        switch (ps_colorspace) {
        case COLORSPACE_GRAYSCALE:
            fprintf(canvas->prstream,"%.4f SGRY\n",
                get_colorintensity(canvas, cindex));
            break;
        case COLORSPACE_RGB:
            get_frgb(canvas, cindex, &frgb);
            fprintf(canvas->prstream,"%.4f %.4f %.4f SRGB\n",
                              frgb.red, frgb.green, frgb.blue);
            break;
        case COLORSPACE_CMYK:
            get_fcmyk(canvas, cindex, &fcmyk);
            fprintf(canvas->prstream, "%.4f %.4f %.4f %.4f SCMYK\n",
                              fcmyk.cyan, fcmyk.magenta,
                              fcmyk.yellow, fcmyk.black);
            break;
        }
        fprintf(canvas->prstream, "/picstr %d string def\n", paddedW/8);
        fprintf(canvas->prstream, "%d %d true\n", paddedW, height);
        fprintf(canvas->prstream, "[%d 0 0 %d 0 0]\n", paddedW, -height);
        fprintf(canvas->prstream, "{currentfile picstr readhexstring pop}\n");
        fprintf(canvas->prstream, "imagemask\n");
        for (k = 0; k < height; k++) {
            for (j = 0; j < paddedW/bitmap_pad; j++) {
                tmpbyte = reversebits((unsigned char) (databits)[k*paddedW/bitmap_pad + j]);
                fprintf(canvas->prstream, "%02x", tmpbyte);
            }
            fprintf(canvas->prstream, "\n");
        }
    }
    fprintf(canvas->prstream, "GR\n");
}

void ps_puttext(const Canvas *canvas,
    const VPoint *vp, const char *s, int len, int font, const TextMatrix *tm,
    int underline, int overline, int kerning)
{
    char *fontname;
    char *encscheme;
    double *kvector;
    int i;
    Pen pen;
    
    if (psfont_status[font] == FALSE) {
        fontname = get_fontalias(canvas, font);
        encscheme = get_encodingscheme(canvas, font);
        fprintf(canvas->prstream, "/%s findfont\n", fontname);
        if (strcmp(encscheme, "FontSpecific") != 0) {
            fprintf(canvas->prstream, "dup length dict begin\n");
            fprintf(canvas->prstream, " {1 index /FID ne {def} {pop pop} ifelse} forall\n");
            fprintf(canvas->prstream, " /Encoding DefEncoding def\n");
            fprintf(canvas->prstream, " currentdict\n");
            fprintf(canvas->prstream, "end\n");
        }
        fprintf(canvas->prstream, "/Font%d exch definefont pop\n", font);
        psfont_status[font] = TRUE;
    }
    fprintf(canvas->prstream, "/Font%d FFSF\n", font);

    getpen(canvas, &pen);
    ps_setpen(canvas, &pen);
    
    fprintf(canvas->prstream, "%.4f %.4f m\n", vp->x, vp->y);
    fprintf(canvas->prstream, "GS\n");
    fprintf(canvas->prstream, "[%.4f %.4f %.4f %.4f 0 0] CC\n",
                        tm->cxx, tm->cyx, tm->cxy, tm->cyy);
    
    if (kerning) {
        kvector = get_kerning_vector(canvas, s, len, font);
    } else {
        kvector = NULL;
    }
    
    if (kvector) {
        fprintf(canvas->prstream, "[");
        for (i = 0; i < len - 1; i++) {
            fprintf(canvas->prstream, "%.4f ", kvector[i]);
        }
        fprintf(canvas->prstream, "] KINIT\n");
        fprintf(canvas->prstream, "{KPROC} ");
    }
    
    put_string(canvas->prstream, s, len);

    if (underline | overline) {
        double w, pos, kcomp;
        
        if (kvector) {
            kcomp = kvector[len - 1];
        } else {
            kcomp = 0.0;
        }
        w = get_textline_width(canvas, font);
        if (underline) {
            pos = get_underline_pos(canvas, font);
            fprintf(canvas->prstream, " %.4f %.4f %.4f TL", pos, w, kcomp);
        }
        if (overline) {
            pos = get_overline_pos(canvas, font);
            fprintf(canvas->prstream, " %.4f %.4f %.4f TL", pos, w, kcomp);
        }
    }
    
    if (kvector) {
        fprintf(canvas->prstream, " kshow\n");
        xfree(kvector);
    } else {
        fprintf(canvas->prstream, " show\n");
    }
    
    fprintf(canvas->prstream, "GR\n");
}


void ps_leavegraphics(const Canvas *canvas)
{
    view v;
    int i, first;
    
    if (curformat == PS_FORMAT) {
        fprintf(canvas->prstream, "showpage\n");
        fprintf(canvas->prstream, "%%%%PageTrailer\n");
    }
    fprintf(canvas->prstream, "%%%%Trailer\n");
    
    if (tight_bb == TRUE) {
        get_bbox(canvas, BBOX_TYPE_GLOB, &v);
        if (page_orientation == PAGE_ORIENT_LANDSCAPE) {
            fprintf(canvas->prstream, "%%%%BoundingBox: %d %d %d %d\n",
                                         (int) (page_scalef*(1.0 - v.yv2)) - 1,
                                         (int) (page_scalef*v.xv1) - 1,
                                         (int) (page_scalef*(1.0 - v.yv1)) + 2,
                                         (int) (page_scalef*v.xv2) + 2);
        } else {
            fprintf(canvas->prstream, "%%%%BoundingBox: %d %d %d %d\n",
                                         (int) (page_scalef*v.xv1) - 1,
                                         (int) (page_scalef*v.yv1) - 1,
                                         (int) (page_scalef*v.xv2) + 2,
                                         (int) (page_scalef*v.yv2) + 2);
        }
    }
    
    first = TRUE;
    for (i = 0; i < number_of_fonts(canvas); i++) {
        if (psfont_status[i] == TRUE) {
            if (first) {
                fprintf(canvas->prstream, "%%%%DocumentNeededResources: font %s\n",
                    get_fontalias(canvas, i));
                first = FALSE;
            } else {
                fprintf(canvas->prstream, "%%%%+ font %s\n",
                    get_fontalias(canvas, i));
            }
        }
    }

    fprintf(canvas->prstream, "%%%%EOF\n");
}

static int is7bit(unsigned char uc)
{
    if (uc >= 0x1b && uc <= 0x7e) {
        return TRUE;
    } else {
        return FALSE;
    }
}

static int is8bit(unsigned char uc)
{
    if (is7bit(uc) || uc >= 0x80) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/*
 * Put a NOT NULL-terminated string escaping parentheses and backslashes
 */
static void put_string(FILE *fp, const char *s, int len)
{
    int i;
    
    fputc('(', fp);
    for (i = 0; i < len; i++) {
        char c = s[i];
        unsigned char uc = (unsigned char) c;
        if (c == '(' || c == ')' || c == '\\') {
            fputc('\\', fp);
        }
        if ((docdata == DOCDATA_7BIT && !is7bit(uc)) ||
            (docdata == DOCDATA_8BIT && !is8bit(uc))) {
            fprintf(fp, "\\%03o", uc);
        } else {
            fputc(c, fp);
        }
    }
    fputc(')', fp);
}

int psprintinitgraphics(const Canvas *canvas)
{
    int result;
    
    ps_level2     = ps_setup_level2;
    ps_colorspace = ps_setup_colorspace;
    docdata       = ps_setup_docdata;
    result = ps_initgraphics(canvas, PS_FORMAT);
    
    if (result == RETURN_SUCCESS) {
        curformat = PS_FORMAT;
    }
    
    return (result);
}

int epsinitgraphics(const Canvas *canvas)
{
    int result;
    
    ps_level2     = eps_setup_level2;
    ps_colorspace = eps_setup_colorspace;
    docdata       = eps_setup_docdata;
    result = ps_initgraphics(canvas, EPS_FORMAT);
    
    if (result == RETURN_SUCCESS) {
        curformat = EPS_FORMAT;
    }
    
    return (result);
}

int ps_op_parser(const Canvas *canvas, const char *opstring)
{
    if (!strcmp(opstring, "level2")) {
        ps_setup_level2 = TRUE;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "level1")) {
        ps_setup_level2 = FALSE;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "colorspace:grayscale")) {
        ps_setup_colorspace = COLORSPACE_GRAYSCALE;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "colorspace:rgb")) {
        ps_setup_colorspace = COLORSPACE_RGB;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "colorspace:cmyk")) {
        ps_setup_colorspace = COLORSPACE_CMYK;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "docdata:7bit")) {
        ps_setup_docdata = DOCDATA_7BIT;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "docdata:8bit")) {
        ps_setup_docdata = DOCDATA_8BIT;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "docdata:binary")) {
        ps_setup_docdata = DOCDATA_BINARY;
        return RETURN_SUCCESS;
    } else if (!strncmp(opstring, "xoffset:", 8)) {
        ps_setup_offset_x = atoi(opstring + 8);
        return RETURN_SUCCESS;
    } else if (!strncmp(opstring, "yoffset:", 8)) {
        ps_setup_offset_y = atoi(opstring + 8);
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "hwresolution:on")) {
        ps_setup_hwres = TRUE;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "hwresolution:off")) {
        ps_setup_hwres = FALSE;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "mediafeed:auto")) {
        ps_setup_feed = MEDIA_FEED_AUTO;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "mediafeed:match")) {
        ps_setup_feed = MEDIA_FEED_MATCH;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "mediafeed:manual")) {
        ps_setup_feed = MEDIA_FEED_MANUAL;
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int eps_op_parser(const Canvas *canvas, const char *opstring)
{
    if (!strcmp(opstring, "level2")) {
        eps_setup_level2 = TRUE;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "level1")) {
        eps_setup_level2 = FALSE;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "colorspace:grayscale")) {
        eps_setup_colorspace = COLORSPACE_GRAYSCALE;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "colorspace:rgb")) {
        eps_setup_colorspace = COLORSPACE_RGB;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "colorspace:cmyk")) {
        eps_setup_colorspace = COLORSPACE_CMYK;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "docdata:7bit")) {
        eps_setup_docdata = DOCDATA_7BIT;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "docdata:8bit")) {
        eps_setup_docdata = DOCDATA_8BIT;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "docdata:binary")) {
        eps_setup_docdata = DOCDATA_BINARY;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "bbox:tight")) {
        eps_setup_tight_bb = TRUE;
        return RETURN_SUCCESS;
    } else if (!strcmp(opstring, "bbox:page")) {
        eps_setup_tight_bb = FALSE;
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

#ifndef NONE_GUI

static void update_ps_setup_frame(void);
static int set_ps_setup_proc(void *data);

static Widget ps_setup_frame;
static Widget ps_setup_level2_item;
static OptionStructure *ps_setup_colorspace_item;
static SpinStructure *ps_setup_offset_x_item;
static SpinStructure *ps_setup_offset_y_item;
static OptionStructure *ps_setup_feed_item;
static Widget ps_setup_hwres_item;
static OptionStructure *ps_setup_docdata_item;

static void colorspace_cb(int onoff, void *data)
{
    OptionStructure *opt = (OptionStructure *) data;
    
    OptionItem colorspace_op_items[3] = {
        {COLORSPACE_GRAYSCALE, "Grayscale"},
        {COLORSPACE_RGB,       "RGB"      },
        {COLORSPACE_CMYK,      "CMYK"     }
    };
    
    if (onoff) {
        UpdateOptionChoice(opt, 3, colorspace_op_items);
    } else {
        UpdateOptionChoice(opt, 2, colorspace_op_items);
    }
}

void ps_gui_setup(const Canvas *canvas)
{
    set_wait_cursor();
    
    if (ps_setup_frame == NULL) {
        Widget ps_setup_rc, fr, rc;
        OptionItem colorspace_op_items[3] = {
            {COLORSPACE_GRAYSCALE, "Grayscale"},
            {COLORSPACE_RGB,       "RGB"      },
            {COLORSPACE_CMYK,      "CMYK"     }
        };
        OptionItem docdata_op_items[3] = {
            {DOCDATA_7BIT,   "7bit"  },
            {DOCDATA_8BIT,   "8bit"  },
            {DOCDATA_BINARY, "Binary"}
        };
        OptionItem op_items[3] = {
            {MEDIA_FEED_AUTO,   "Automatic" },
            {MEDIA_FEED_MATCH,  "Match size"},
            {MEDIA_FEED_MANUAL, "Manual"    }
        };
        
	ps_setup_frame = CreateDialogForm(app_shell, "PS options");

        ps_setup_rc = CreateVContainer(ps_setup_frame);

	fr = CreateFrame(ps_setup_rc, "PS options");
        rc = CreateVContainer(fr);
	ps_setup_level2_item = CreateToggleButton(rc, "PS Level 2");
        ps_setup_colorspace_item =
            CreateOptionChoice(rc, "Colorspace:", 1, 3, colorspace_op_items);
	AddToggleButtonCB(ps_setup_level2_item,
            colorspace_cb, ps_setup_colorspace_item);
	ps_setup_docdata_item =
            CreateOptionChoice(rc, "Document data:", 1, 3, docdata_op_items);

	fr = CreateFrame(ps_setup_rc, "Page offsets (pt)");
        rc = CreateHContainer(fr);
	ps_setup_offset_x_item = CreateSpinChoice(rc,
            "X: ", 4, SPIN_TYPE_INT, -999.0, 999.0, 10.0);
	ps_setup_offset_y_item = CreateSpinChoice(rc,
            "Y: ", 4, SPIN_TYPE_INT, -999.0, 999.0, 10.0);

	fr = CreateFrame(ps_setup_rc, "Hardware");
        rc = CreateVContainer(fr);
	ps_setup_feed_item = CreateOptionChoice(rc, "Media feed:", 1, 3, op_items);
	ps_setup_hwres_item = CreateToggleButton(rc, "Set hardware resolution");

	CreateAACDialog(ps_setup_frame, ps_setup_rc, set_ps_setup_proc, NULL);
    }
    update_ps_setup_frame();
    
    RaiseWindow(GetParent(ps_setup_frame));
    unset_wait_cursor();
}

static void update_ps_setup_frame(void)
{
    if (ps_setup_frame) {
        SetToggleButtonState(ps_setup_level2_item, ps_setup_level2);
        SetOptionChoice(ps_setup_colorspace_item, ps_setup_colorspace);
        colorspace_cb(ps_setup_level2, ps_setup_colorspace_item);
        SetSpinChoice(ps_setup_offset_x_item, (double) ps_setup_offset_x);
        SetSpinChoice(ps_setup_offset_y_item, (double) ps_setup_offset_y);
        SetOptionChoice(ps_setup_feed_item, ps_setup_feed);
        SetToggleButtonState(ps_setup_hwres_item, ps_setup_hwres);
        SetOptionChoice(ps_setup_docdata_item, ps_setup_docdata);
    }
}

static int set_ps_setup_proc(void *data)
{
    ps_setup_level2     = GetToggleButtonState(ps_setup_level2_item);
    ps_setup_colorspace = GetOptionChoice(ps_setup_colorspace_item);
    ps_setup_offset_x   = (int) GetSpinChoice(ps_setup_offset_x_item);
    ps_setup_offset_y   = (int) GetSpinChoice(ps_setup_offset_y_item);
    ps_setup_feed       = GetOptionChoice(ps_setup_feed_item);
    ps_setup_hwres      = GetToggleButtonState(ps_setup_hwres_item);
    ps_setup_docdata    = GetOptionChoice(ps_setup_docdata_item);
    
    return RETURN_SUCCESS;
}

static void update_eps_setup_frame(void);
static int set_eps_setup_proc(void *data);
static Widget eps_setup_frame;
static Widget eps_setup_level2_item;
static OptionStructure *eps_setup_colorspace_item;
static Widget eps_setup_tight_bb_item;
static OptionStructure *eps_setup_docdata_item;

void eps_gui_setup(const Canvas *canvas)
{
    set_wait_cursor();
    
    if (eps_setup_frame == NULL) {
        Widget fr, rc;
        OptionItem colorspace_op_items[3] = {
            {COLORSPACE_GRAYSCALE, "Grayscale"},
            {COLORSPACE_RGB,       "RGB"      },
            {COLORSPACE_CMYK,      "CMYK"     }
        };
        OptionItem docdata_op_items[3] = {
            {DOCDATA_7BIT,   "7bit"  },
            {DOCDATA_8BIT,   "8bit"  },
            {DOCDATA_BINARY, "Binary"}
        };
	
        eps_setup_frame = CreateDialogForm(app_shell, "EPS options");

        fr = CreateFrame(eps_setup_frame, "EPS options");
        rc = CreateVContainer(fr);
	eps_setup_level2_item = CreateToggleButton(rc, "PS Level 2");
	eps_setup_colorspace_item =
            CreateOptionChoice(rc, "Colorspace:", 1, 3, colorspace_op_items);
	AddToggleButtonCB(eps_setup_level2_item,
            colorspace_cb, eps_setup_colorspace_item);
	eps_setup_docdata_item =
            CreateOptionChoice(rc, "Document data:", 1, 3, docdata_op_items);
	eps_setup_tight_bb_item = CreateToggleButton(rc, "Tight BBox");
	CreateAACDialog(eps_setup_frame, fr, set_eps_setup_proc, NULL);
    }
    update_eps_setup_frame();
    RaiseWindow(GetParent(eps_setup_frame));
    
    unset_wait_cursor();
}

static void update_eps_setup_frame(void)
{
    if (eps_setup_frame) {
        SetToggleButtonState(eps_setup_level2_item, eps_setup_level2);
        SetOptionChoice(eps_setup_colorspace_item, eps_setup_colorspace);
        colorspace_cb(eps_setup_level2, eps_setup_colorspace_item);
        SetToggleButtonState(eps_setup_tight_bb_item, eps_setup_tight_bb);
        SetOptionChoice(eps_setup_docdata_item, eps_setup_docdata);
    }
}

static int set_eps_setup_proc(void *data)
{
    eps_setup_level2     = GetToggleButtonState(eps_setup_level2_item);
    eps_setup_colorspace = GetOptionChoice(eps_setup_colorspace_item);
    eps_setup_tight_bb   = GetToggleButtonState(eps_setup_tight_bb_item);
    eps_setup_docdata    = GetOptionChoice(eps_setup_docdata_item);
    
    return RETURN_SUCCESS;
}

#endif
