/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-95 Paul J Turner, Portland, OR
 * Copyright (c) 1996-99 Grace Development Team
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
#  include <Xm/Xm.h>
#  include <Xm/Form.h>
#  include <Xm/RowColumn.h>
#  include <Xm/DialogS.h>

#  include "motifinc.h"
#endif

extern FILE *prstream;

static char *escape_paren(char *s);

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

static int ps_grayscale = FALSE;
static int ps_level2 = TRUE;

static int ps_setup_offset_x = 0;
static int ps_setup_offset_y = 0;

static int ps_setup_grayscale = FALSE;
static int ps_setup_level2 = TRUE;

static int eps_setup_grayscale = FALSE;
static int eps_setup_tight_bb = TRUE;

static int tight_bb;

static Device_entry dev_ps = {DEVICE_PRINT,
          "PostScript",
          psprintinitgraphics,
          ps_op_parser,
          ps_gui_setup,
          "ps",
          TRUE,
          FALSE,
          {3300, 2550, 300.0}
         };

static Device_entry dev_eps = {DEVICE_FILE,
          "EPS",
          epsinitgraphics,
          eps_op_parser,
          eps_gui_setup,
          "eps",
          TRUE,
          FALSE,
          {2500, 2500, 300.0}
         };

int register_ps_drv(void)
{
    return register_device(dev_ps);
}

int register_eps_drv(void)
{
    return register_device(dev_eps);
}

static int ps_initgraphics(int format)
{
    int i, j;
    Page_geometry pg;
    fRGB *frgb;
    int width_pp, height_pp, page_offset_x, page_offset_y;
    
    time_t time_value;
    
    curformat = format;
    
    /* device-dependent routines */
    devupdatecmap = NULL;
    
    devdrawpixel = ps_drawpixel;
    devdrawpolyline = ps_drawpolyline;
    devfillpolygon = ps_fillpolygon;
    devdrawarc = ps_drawarc;
    devfillarc = ps_fillarc;
    devputpixmap = ps_putpixmap;
    devputtext = ps_puttext;
    
    devleavegraphics = ps_leavegraphics;

    pg = get_page_geometry();
    
    page_scale = MIN2(pg.height, pg.width);
    pixel_size = 1.0/page_scale;
    page_scalef = (float) page_scale*72.0/pg.dpi;

    if (pg.height < pg.width) {
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

    /* Font status table */
    if (psfont_status != NULL) {
        free(psfont_status);
    }
    psfont_status = malloc(number_of_fonts()*SIZEOF_INT);
    for (i = 0; i < number_of_fonts(); i++) {
        psfont_status[i] = FALSE;
    }
    
    switch (curformat) {
    case PS_FORMAT:
        fprintf(prstream, "%%!PS-Adobe-3.0\n");
        tight_bb = FALSE;
        page_offset_x = ps_setup_offset_x;
        page_offset_y = ps_setup_offset_y;
        break;
    case EPS_FORMAT:
        fprintf(prstream, "%%!PS-Adobe-3.0 EPSF-3.0\n");
        tight_bb = eps_setup_tight_bb;
        page_offset_x = 0;
        page_offset_y = 0;
        break;
    default:
        errmsg("Invalid PS format");
        return GRACE_EXIT_FAILURE;
    }
    
    if (page_orientation == PAGE_ORIENT_PORTRAIT) {
        width_pp  = (int) rint(72.0*pg.width/pg.dpi);
        height_pp = (int) rint(72.0*pg.height/pg.dpi);
    } else {
        width_pp  = (int) rint(72.0*pg.height/pg.dpi);
        height_pp = (int) rint(72.0*pg.width/pg.dpi);
    }
    
    if (tight_bb == TRUE) {
        fprintf(prstream, "%%%%BoundingBox: (atend)\n");
    } else {
        fprintf(prstream, "%%%%BoundingBox: %d %d %d %d\n", 
            page_offset_x, page_offset_y,
            width_pp + page_offset_x, height_pp + page_offset_y);
    }
    
    if (ps_level2 == TRUE) {
        fprintf(prstream, "%%%%LanguageLevel: 2\n");
    } else {
        fprintf(prstream, "%%%%LanguageLevel: 1\n");
    }
    
    fprintf(prstream, "%%%%Creator: %s\n", bi_version_string());

    time(&time_value);
    fprintf(prstream, "%%%%CreationDate: %s", ctime(&time_value));
    fprintf(prstream, "%%%%DocumentData: Clean7Bit\n");

    if (page_orientation == PAGE_ORIENT_LANDSCAPE) {
        fprintf(prstream, "%%%%Orientation: Landscape\n");
    } else {
        fprintf(prstream, "%%%%Orientation: Portrait\n");
    }
    
    if (curformat == PS_FORMAT) {
        fprintf(prstream, "%%%%Pages: 1\n");
        fprintf(prstream, "%%%%PageOrder: Ascend\n");
    }
    fprintf(prstream, "%%%%Title: %s\n", get_docname());
    fprintf(prstream, "%%%%For: %s\n", get_username());
    fprintf(prstream, "%%%%EndComments\n");

    /* Definitions */
    fprintf(prstream, "%%%%BeginProlog\n");
    if (curformat == PS_FORMAT) {
        fprintf(prstream, "/PAGE_OFFSET_X %d def\n", page_offset_x);
        fprintf(prstream, "/PAGE_OFFSET_Y %d def\n", page_offset_y);
    }
    fprintf(prstream, "/m {moveto} def\n");
    fprintf(prstream, "/l {lineto} def\n");
    fprintf(prstream, "/s {stroke} def\n");
    fprintf(prstream, "/n {newpath} def\n");
    fprintf(prstream, "/c {closepath} def\n");
    fprintf(prstream, "/pix {n m 0 0 rlineto s} def\n");
    
    for (i = 0; i < number_of_colors(); i++) {
        fprintf(prstream,"/Color%d {\n", i);
        if (ps_grayscale == TRUE) {
            fprintf(prstream," %.4f\n", get_colorintensity(i));
        } else {
            frgb = get_frgb(i);
            if (frgb != NULL) {
                fprintf(prstream, " %.4f %.4f %.4f\n",
                                    frgb->red,frgb->green, frgb->blue);
            }
        }
        fprintf(prstream,"} def\n");
    }
       
    if (ps_level2 == TRUE) {
        fprintf(prstream, "/pattern {\n");
        fprintf(prstream, " /pat_bits exch def \n");
        fprintf(prstream, " <<\n");
        fprintf(prstream, "  /PaintType 2\n");
        fprintf(prstream, "  /PatternType 1 /TilingType 1\n");
        fprintf(prstream, "  /BBox[0 0 16 16]\n");
        fprintf(prstream, "  /XStep 16 /YStep 16\n");
        fprintf(prstream, "  /PaintProc {\n");
        fprintf(prstream, "   pop\n");
        fprintf(prstream, "   16 16 true [-1 0 0 -1 16 16] pat_bits imagemask\n");
        fprintf(prstream, "  }\n");
        fprintf(prstream, " >>\n");
        fprintf(prstream, " [%.4f 0 0 %.4f 0 0]\n", 1.0/page_scalef, 1.0/page_scalef);
        fprintf(prstream, " makepattern\n");
        fprintf(prstream, "} def\n");
        for (i = 0; i < number_of_patterns(); i++) {
            fprintf(prstream, "/Pattern%d {<", i);
            for (j = 0; j < 32; j++) {
                fprintf(prstream, "%02x", pat_bits[i][j]);
            }
            fprintf(prstream, "> pattern} bind def\n");
        }
    }
    fprintf(prstream, "/ellipsedict 8 dict def\n");
    fprintf(prstream, "ellipsedict /mtrx matrix put\n");
    fprintf(prstream, "/ellipse {\n");
    fprintf(prstream, " ellipsedict begin\n");
    fprintf(prstream, "  /endangle exch def\n");
    fprintf(prstream, "  /startangle exch def\n");
    fprintf(prstream, "  /yrad exch def\n");
    fprintf(prstream, "  /xrad exch def\n");
    fprintf(prstream, "  /y exch def\n");
    fprintf(prstream, "  /x exch def\n");
    fprintf(prstream, "  /savematrix mtrx currentmatrix def\n");
    fprintf(prstream, "  x y translate\n");
    fprintf(prstream, "  xrad yrad scale\n");
    fprintf(prstream, "  0 0 1 startangle endangle arc\n");
    fprintf(prstream, "  savematrix setmatrix\n");
    fprintf(prstream, " end\n");
    fprintf(prstream, "} def\n");

    fprintf(prstream, "%%%%EndProlog\n");

    fprintf(prstream, "%%%%BeginSetup\n");
    /* page size */
    if (ps_level2 == TRUE) {
        fprintf(prstream, "%%%%BeginFeature: *PageSize\n");
        fprintf(prstream, "<</PageSize [%d %d] /ImagingBBox null>> setpagedevice\n",
            width_pp, height_pp);
        fprintf(prstream, "%%%%EndFeature\n");
    }
    
    /* compensate for printer page offsets */
    if (curformat == PS_FORMAT) {
        fprintf(prstream, "PAGE_OFFSET_X PAGE_OFFSET_Y translate\n");
    }
    fprintf(prstream, "%.2f %.2f scale\n", page_scalef, page_scalef);
    if (page_orientation == PAGE_ORIENT_LANDSCAPE) {
        fprintf(prstream, "90 rotate\n");
        fprintf(prstream, "0.0 -1.0 translate\n");
    }
    fprintf(prstream, "%%%%EndSetup\n");

    if (curformat == PS_FORMAT) {
        fprintf(prstream, "%%%%Page: 1 1\n");
    }

    return GRACE_EXIT_SUCCESS;
}

void ps_setpen(void)
{
    Pen pen;
    
    pen = getpen();
    
    if (pen.color != ps_color || pen.pattern != ps_pattern) {
        if (ps_level2 == TRUE) {
            if (pen.pattern == 1) {
                if (ps_grayscale == TRUE) {
                    fprintf(prstream, "[/DeviceGray] setcolorspace\n");
                } else {
                    fprintf(prstream, "[/DeviceRGB] setcolorspace\n");
                }
                fprintf(prstream, "Color%d setcolor\n", pen.color);
            } else {
                if (ps_grayscale == TRUE) {
                    fprintf(prstream, "[/Pattern /DeviceGray] setcolorspace\n");
                } else {
                    fprintf(prstream, "[/Pattern /DeviceRGB] setcolorspace\n");
                }
                fprintf(prstream,
                    "Color%d Pattern%d setcolor\n", pen.color, pen.pattern);
            }
        } else {
            if (ps_grayscale == TRUE) {
                fprintf(prstream, "Color%d setgray\n", pen.color);
            } else {
                fprintf(prstream, "Color%d setrgbcolor\n", pen.color);
            }
        }
        ps_color = pen.color;
        ps_pattern = pen.pattern;
    }
}

void ps_setdrawbrush(void)
{
    int i;
    int ls;
    double lw;
    
    ps_setpen();

    ls = getlinestyle();
    lw = MAX2(getlinewidth(), pixel_size);
    
    if (ls != ps_lines || lw != ps_linew) {    
        fprintf(prstream, "[");
        if (ls > 1) {
            for (i = 0; i < dash_array_length[ls]; i++) {
                fprintf(prstream, "%.4f ", lw*dash_array[ls][i]);
            }
        }
        fprintf(prstream, "] 0 setdash\n");
        fprintf(prstream, "%.4f setlinewidth\n", lw);
        ps_linew = lw;
        ps_lines = ls;
    }
}

void ps_setlineprops(void)
{
    int lc, lj;
    
    lc = getlinecap();
    lj = getlinejoin();
    
    if (lc != ps_linecap) {
        switch (lc) {
        case LINECAP_BUTT:
            fprintf(prstream, "0 setlinecap\n");
            break;
        case LINECAP_ROUND:
            fprintf(prstream, "1 setlinecap\n");
            break;
        case LINECAP_PROJ:
            fprintf(prstream, "2 setlinecap\n");
            break;
        }
        ps_linecap = lc;
    }

    if (lj != ps_linejoin) {
        switch (lj) {
        case LINEJOIN_MITER:
            fprintf(prstream, "0 setlinejoin\n");
            break;
        case LINEJOIN_ROUND:
            fprintf(prstream, "1 setlinejoin\n");
            break;
        case LINEJOIN_BEVEL:
            fprintf(prstream, "2 setlinejoin\n");
            break;
        }
        ps_linejoin = lj;
    }
}

void ps_drawpixel(VPoint vp)
{
    ps_setpen();
    
    if (ps_linew != pixel_size) {
        fprintf(prstream, "%.4f setlinewidth\n", pixel_size);
        ps_linew = pixel_size;
    }
    if (ps_linecap != LINECAP_ROUND) {
        fprintf(prstream, "1 setlinecap\n");
        ps_linecap = LINECAP_ROUND;
    }
    if (ps_lines != 1) {
        fprintf(prstream, "[] 0 setdash\n");
        ps_lines = 1;
    }
    
    fprintf(prstream, "%.4f %.4f pix\n", vp.x, vp.y);
}

void ps_drawpolyline(VPoint *vps, int n, int mode)
{
    int i;
    
    ps_setdrawbrush();
    
    ps_setlineprops();
    
    fprintf(prstream, "n\n");
    fprintf(prstream, "%.4f %.4f m\n", vps[0].x, vps[0].y);
    for (i = 1; i < n; i++) {
        fprintf(prstream, "%.4f %.4f l\n", vps[i].x, vps[i].y);
    }
    if (mode == POLYLINE_CLOSED) {
        fprintf(prstream, "%.4f %.4f l\n", vps[0].x, vps[0].y);
        fprintf(prstream, "c\n");
    }
    fprintf(prstream, "s\n");
}

void ps_fillpolygon(VPoint *vps, int nc)
{
    int i;
    Pen pen = getpen();
    
    if (pen.pattern == 0 || nc < 3) {
        return;
    }
    
    fprintf(prstream, "n\n");
    fprintf(prstream, "%.4f %.4f m\n", vps[0].x, vps[0].y);
    for (i = 1; i < nc; i++) {
        fprintf(prstream, "%.4f %.4f l\n", vps[i].x, vps[i].y);
    }
    fprintf(prstream, "c\n");

    /* fill bg first if the pattern != solid */
    if (pen.pattern != 1 && ps_level2 == TRUE) {
        fprintf(prstream, "gsave\n");
        if (ps_grayscale == TRUE) {
            if (ps_pattern != 1) {
                fprintf(prstream, "[/DeviceGray] setcolorspace\n");
            }
            fprintf(prstream, "Color%d setgray\n", getbgcolor());
        } else {
            if (ps_pattern != 1) {
                fprintf(prstream, "[/DeviceRGB] setcolorspace\n");
            }
            fprintf(prstream, "Color%d setrgbcolor\n", getbgcolor());
        }
        if (getfillrule() == FILLRULE_WINDING) {
            fprintf(prstream, "fill\n");
        } else {
            fprintf(prstream, "eofill\n");
        }
        fprintf(prstream, "grestore\n");
    }
    
    ps_setpen();
    if (getfillrule() == FILLRULE_WINDING) {
        fprintf(prstream, "fill\n");
    } else {
        fprintf(prstream, "eofill\n");
    }
}

void ps_drawarc(VPoint vp1, VPoint vp2, int a1, int a2)
{
    VPoint vpc;
    double rx, ry;
    
    ps_setdrawbrush();

    vpc.x = (vp1.x + vp2.x)/2;
    vpc.y = (vp1.y + vp2.y)/2;
    rx = fabs(vp2.x - vp1.x)/2;
    ry = fabs(vp2.y - vp1.y)/2;
    
    fprintf(prstream, "n\n");
    fprintf(prstream, "%.4f %.4f %.4f %.4f %d %d ellipse s\n",
                       vpc.x, vpc.y, rx, ry, a1, a2);
}

void ps_fillarc(VPoint vp1, VPoint vp2, int a1, int a2)
{
    VPoint vpc;
    double rx, ry;
    Pen pen = getpen();
    
    if (pen.pattern == 0) {
        return;
    }

    vpc.x = (vp1.x + vp2.x)/2;
    vpc.y = (vp1.y + vp2.y)/2;
    rx = fabs(vp2.x - vp1.x)/2;
    ry = fabs(vp2.y - vp1.y)/2;
    
    fprintf(prstream, "n\n");
    fprintf(prstream, "%.4f %.4f %.4f %.4f %d %d ellipse c\n",
                       vpc.x, vpc.y, rx, ry, a1, a2);

    /* fill bg first if the pattern != solid */
    if (pen.pattern != 1 && ps_level2 == TRUE) {
        fprintf(prstream, "gsave\n");
        if (ps_grayscale == TRUE) {
            if (ps_pattern != 1) {
                fprintf(prstream, "[/DeviceGray] setcolorspace\n");
            }
            fprintf(prstream, "Color%d setgray\n", getbgcolor());
        } else {
            if (ps_pattern != 1) {
                fprintf(prstream, "[/DeviceRGB] setcolorspace\n");
            }
            fprintf(prstream, "Color%d setrgbcolor\n", getbgcolor());
        }
        fprintf(prstream, "fill\n");
        fprintf(prstream, "grestore\n");
    }

    ps_setpen();
    fprintf(prstream, "fill\n");
}

void ps_putpixmap(VPoint vp, int width, int height, 
     char *databits, int pixmap_bpp, int bitmap_pad, int pixmap_type)
{
    int j, k;
    int cindex;
    int paddedW;
    RGB *rgb;
    fRGB *frgb;
    unsigned char tmpbyte;

    ps_setpen();
    
    fprintf(prstream, "gsave\n");
    fprintf(prstream, "%.4f %.4f translate\n", vp.x, vp.y);
    fprintf(prstream, "%.4f %.4f scale\n", (float) width/page_scale, 
                                           (float) height/page_scale);    
    if (pixmap_bpp != 1) {
        if (pixmap_type == PIXMAP_TRANSPARENT) {
            /* TODO: mask */
        }
        if (ps_grayscale == TRUE) {
            fprintf(prstream, "/picstr %d string def\n", width);
            fprintf(prstream, "%d %d %d\n", width, height, 8);
        } else {
            fprintf(prstream, "/picstr %d string def\n", 3*width);
            fprintf(prstream, "%d %d %d\n", width, height, GRACE_BPP);
        }
        fprintf(prstream, "[%d 0 0 %d 0 0]\n", width, -height);
        fprintf(prstream, "{currentfile picstr readhexstring pop}\n");
        if (ps_grayscale == TRUE || ps_level2 == FALSE) {
            /* No color images in Level1 */
            fprintf(prstream, "image\n");
        } else {
            fprintf(prstream, "false 3\n");
            fprintf(prstream, "colorimage\n");
        }
        for (k = 0; k < height; k++) {
            for (j = 0; j < width; j++) {
                cindex = (databits)[k*width+j];
                if (ps_grayscale == TRUE || ps_level2 == FALSE) {
                    fprintf(prstream,"%02x",
                                      (int) (255*get_colorintensity(cindex)));
                } else {
                    rgb = get_rgb(cindex);
                    fprintf(prstream, "%02x%02x%02x",
                                       rgb->red, rgb->green, rgb->blue);
                }
            }
            fprintf(prstream, "\n");
        }
    } else { /* monocolor bitmap */
        paddedW = PAD(width, bitmap_pad);
        if (pixmap_type == PIXMAP_OPAQUE) {
            if (ps_grayscale == TRUE) {
                fprintf(prstream,"%.4f setgray\n",
                                  get_colorintensity(getbgcolor()));
            } else {
                frgb = get_frgb(getbgcolor());
                fprintf(prstream,"%.4f %.4f %.4f setrgbcolor\n",
                                  frgb->red, frgb->green, frgb->blue);
            }
            fprintf(prstream, "0 0 1 -1 rectfill\n");
        }
        if (ps_grayscale == TRUE) {
            fprintf(prstream,"%.4f setgray\n", get_colorintensity(getcolor()));
        } else {
            frgb = get_frgb(getcolor());
            fprintf(prstream,"%.4f %.4f %.4f setrgbcolor\n",
                              frgb->red, frgb->green, frgb->blue);
        }
        fprintf(prstream, "/picstr %d string def\n", paddedW/8);
        fprintf(prstream, "%d %d true\n", paddedW, height);
        fprintf(prstream, "[%d 0 0 %d 0 0]\n", paddedW, -height);
        fprintf(prstream, "{currentfile picstr readhexstring pop}\n");
        fprintf(prstream, "imagemask\n");
        for (k = 0; k < height; k++) {
            for (j = 0; j < paddedW/bitmap_pad; j++) {
                tmpbyte = reversebits((unsigned char) (databits)[k*paddedW/bitmap_pad + j]);
                fprintf(prstream, "%02x", tmpbyte);
            }
            fprintf(prstream, "\n");
        }
    }
    fprintf(prstream, "grestore\n");
}

void ps_puttext(VPoint start, VPoint end, double size, CompositeString *cstring)
{
    int iglyph;
    int font;
    double angle;
    double length;
    char *fontname;
    char *encscheme;
    
    size /= page_scalef;
    
    angle = (180.0/M_PI) * atan2(end.y - start.y, end.x - start.x);
    length = hypot (end.x - start.x, end.y - start.y);
        
    /* initialize pslength variable */
    fprintf(prstream, "/pslength 0 def\n");
    iglyph = 0;
    while (cstring[iglyph].s != NULL) {
        font = cstring[iglyph].font;
        if (psfont_status[font] == FALSE) {
            fontname = get_fontalias(font);
            encscheme = get_encodingscheme(font);
            fprintf(prstream, "/%s findfont\n", fontname);
            if (strcmp(encscheme, "ISOLatin1Encoding") == 0) {
                fprintf(prstream, "dup length dict begin\n");
                fprintf(prstream, " {1 index /FID ne {def} {pop pop} ifelse} forall\n");
                fprintf(prstream, " /Encoding ISOLatin1Encoding def\n");
                fprintf(prstream, " currentdict\n");
                fprintf(prstream, "end\n");
            }
            fprintf(prstream, "/Font%d exch definefont pop\n", font);
            psfont_status[font] = TRUE;
        }
        fprintf(prstream, "/Font%d findfont\n", font);
        fprintf(prstream, "%.4f scalefont\n", size*cstring[iglyph].scale);
        fprintf(prstream, "setfont\n");
        /* pop removes unneeded Y coordinate from the stack */
        fprintf(prstream, "(%s) stringwidth pop\n", escape_paren(cstring[iglyph].s));
        fprintf(prstream, "pslength add\n");
        fprintf(prstream, "%.4f add\n", size*cstring[iglyph].hshift);
        fprintf(prstream, "/pslength exch def\n");
        iglyph++;
    }

    ps_setpen();
    
    fprintf(prstream, "gsave\n");
    fprintf(prstream, "%.4f %.4f translate\n", start.x, start.y);
    fprintf(prstream, "%.4f rotate\n", angle);
    /*
     * Compensate for diffs between PS & T1lib 
     * (should Y be scaled the same??)
     * I use higher (.6) precision here since rounding errors may lead to
     * incorrect BB calculations
     */
    fprintf(prstream, "%.6f pslength div %.6f scale\n", 
                                    length*size, size*72.0/page_dpi);

    fprintf(prstream, "0.0 0.0 moveto\n");
    
    iglyph = 0;
    while (cstring[iglyph].s != NULL) {
        fprintf(prstream, "/Font%d findfont\n", cstring[iglyph].font);
        fprintf(prstream, "%.4f scalefont\n", cstring[iglyph].scale);
        fprintf(prstream, "setfont\n");
        if (cstring[iglyph].vshift != 0.0 || cstring[iglyph].hshift != 0.0) {
            fprintf(prstream, "%.4f %.4f rmoveto\n",
                               cstring[iglyph].hshift, cstring[iglyph].vshift);
            fprintf(prstream, "(%s) show\n", escape_paren(cstring[iglyph].s));
            fprintf(prstream, "0.0 %.4f rmoveto\n", -cstring[iglyph].vshift);
        } else {
            fprintf(prstream, "(%s) show\n", escape_paren(cstring[iglyph].s));
        }
        
        if (cstring[iglyph].underline == TRUE) {
            /* TODO */
        }
        
        if (cstring[iglyph].overline == TRUE) {
            /* TODO */
        }
        
        iglyph++;
    }

    fprintf(prstream, "grestore\n");
}


void ps_leavegraphics(void)
{
    view v;
    
    if (curformat == PS_FORMAT) {
        fprintf(prstream, "showpage\n");
        fprintf(prstream, "%%%%PageTrailer\n");
    }
    fprintf(prstream, "%%%%Trailer\n");
    
    if (tight_bb == TRUE) {
        v = get_bbox(BBOX_TYPE_GLOB);
        if (page_orientation == PAGE_ORIENT_LANDSCAPE) {
            fprintf(prstream, "%%%%BoundingBox: %d %d %d %d\n",
                                         (int) (page_scalef*(1.0 - v.yv2)) - 1,
                                         (int) (page_scalef*v.xv1) - 1,
                                         (int) (page_scalef*(1.0 - v.yv1)) + 2,
                                         (int) (page_scalef*v.xv2) + 2);
        } else {
            fprintf(prstream, "%%%%BoundingBox: %d %d %d %d\n",
                                         (int) (page_scalef*v.xv1) - 1,
                                         (int) (page_scalef*v.yv1) - 1,
                                         (int) (page_scalef*v.xv2) + 2,
                                         (int) (page_scalef*v.yv2) + 2);
        }
    }
    
    fprintf(prstream, "%%%%EOF\n");
}

/*
 * escape parentheses
 */
static char *escape_paren(char *s)
{
    static char *es = NULL;
    int i, elen = 0;
    
    elen = 0;
    for (i = 0; i < strlen(s); i++) {
        if (s[i] == '(' || s[i] == ')') {
            elen++;
        }
        elen++;
    }
    
    es = xrealloc(es, (elen + 1)*SIZEOF_CHAR);
    
    elen = 0;
    for (i = 0; i < strlen(s); i++) {
        if (s[i] == '(' || s[i] == ')') {
            es[elen++] = '\\';
        }
        es[elen++] = s[i];
    }
    es[elen] = '\0';
    
    return (es);
}

int psprintinitgraphics(void)
{
    int result;
    
    ps_grayscale = ps_setup_grayscale;
    ps_level2 = ps_setup_level2;
    result = ps_initgraphics(PS_FORMAT);
    
    if (result == GRACE_EXIT_SUCCESS) {
        curformat = PS_FORMAT;
    }
    
    return (result);
}

int epsinitgraphics(void)
{
    int result;
    
    ps_grayscale = eps_setup_grayscale;
    ps_level2 = TRUE;
    result = ps_initgraphics(EPS_FORMAT);
    
    if (result == GRACE_EXIT_SUCCESS) {
        curformat = EPS_FORMAT;
    }
    
    return (result);
}

int ps_op_parser(char *opstring)
{
    if (!strcmp(opstring, "grayscale")) {
        ps_setup_grayscale = TRUE;
        return GRACE_EXIT_SUCCESS;
    } else if (!strcmp(opstring, "color")) {
        ps_setup_grayscale = FALSE;
        return GRACE_EXIT_SUCCESS;
    } else if (!strcmp(opstring, "level2")) {
        ps_setup_level2 = TRUE;
        return GRACE_EXIT_SUCCESS;
    } else if (!strcmp(opstring, "level1")) {
        ps_setup_level2 = FALSE;
        return GRACE_EXIT_SUCCESS;
    } else if (!strncmp(opstring, "xoffset:", 8)) {
        ps_setup_offset_x = atoi(opstring + 8);
        return GRACE_EXIT_SUCCESS;
    } else if (!strncmp(opstring, "yoffset:", 8)) {
        ps_setup_offset_y = atoi(opstring + 8);
        return GRACE_EXIT_SUCCESS;
    } else {
        return GRACE_EXIT_FAILURE;
    }
}

int eps_op_parser(char *opstring)
{
    if (!strcmp(opstring, "grayscale")) {
        ps_setup_grayscale = TRUE;
        return GRACE_EXIT_SUCCESS;
    } else if (!strcmp(opstring, "color")) {
        ps_setup_grayscale = FALSE;
        return GRACE_EXIT_SUCCESS;
    } else if (!strcmp(opstring, "bbox:tight")) {
        eps_setup_tight_bb = TRUE;
        return GRACE_EXIT_SUCCESS;
    } else if (!strcmp(opstring, "bbox:page")) {
        eps_setup_tight_bb = FALSE;
        return GRACE_EXIT_SUCCESS;
    } else {
        return GRACE_EXIT_FAILURE;
    }
}

#ifndef NONE_GUI

static void update_ps_setup_frame(void);
static void set_ps_setup_proc(void *data);

static Widget ps_setup_frame;
static Widget ps_setup_grayscale_item;
static Widget ps_setup_level2_item;
static SpinStructure *ps_setup_offset_x_item;
static SpinStructure *ps_setup_offset_y_item;

void ps_gui_setup(void)
{
    Widget ps_setup_panel, ps_setup_rc, fr, rc;
    
    set_wait_cursor();
    if (ps_setup_frame == NULL) {
	ps_setup_frame = XmCreateDialogShell(app_shell, "PS options", NULL, 0);
	handle_close(ps_setup_frame);
        ps_setup_panel = XtVaCreateWidget("device_panel", xmFormWidgetClass, 
                                        ps_setup_frame, NULL, 0);
        ps_setup_rc = XmCreateRowColumn(ps_setup_panel, "psetup_rc", NULL, 0);

	fr = CreateFrame(ps_setup_rc, "PS options");
        rc = XmCreateRowColumn(fr, "rc", NULL, 0);
	ps_setup_grayscale_item = CreateToggleButton(rc, "Grayscale output");
	ps_setup_level2_item = CreateToggleButton(rc, "PS Level 2");
	XtManageChild(rc);

	fr = CreateFrame(ps_setup_rc, "Page offsets (pt)");
        rc = XmCreateRowColumn(fr, "rc", NULL, 0);
        XtVaSetValues(rc, XmNorientation, XmHORIZONTAL, NULL);
	ps_setup_offset_x_item = CreateSpinChoice(rc,
            "X: ", 4, SPIN_TYPE_INT, -999.0, 999.0, 10.0);
	ps_setup_offset_y_item = CreateSpinChoice(rc,
            "Y: ", 4, SPIN_TYPE_INT, -999.0, 999.0, 10.0);
	XtManageChild(rc);

	CreateSeparator(ps_setup_rc);

	CreateAACButtons(ps_setup_rc, ps_setup_panel, set_ps_setup_proc);
        
	XtManageChild(ps_setup_rc);
	XtManageChild(ps_setup_panel);
    }
    XtRaise(ps_setup_frame);
    update_ps_setup_frame();
    unset_wait_cursor();
}

static void update_ps_setup_frame(void)
{
    if (ps_setup_frame) {
        SetToggleButtonState(ps_setup_grayscale_item, ps_setup_grayscale);
        SetToggleButtonState(ps_setup_level2_item, ps_setup_level2);
        SetSpinChoice(ps_setup_offset_x_item, (double) ps_setup_offset_x);
        SetSpinChoice(ps_setup_offset_y_item, (double) ps_setup_offset_y);
    }
}

static void set_ps_setup_proc(void *data)
{
    int aac_mode;
    aac_mode = (int) data;
    
    if (aac_mode == AAC_CLOSE) {
        XtUnmanageChild(ps_setup_frame);
        return;
    }
    
    ps_setup_grayscale = GetToggleButtonState(ps_setup_grayscale_item);
    ps_setup_level2 = GetToggleButtonState(ps_setup_level2_item);
    ps_setup_offset_x = (int) GetSpinChoice(ps_setup_offset_x_item);
    ps_setup_offset_y = (int) GetSpinChoice(ps_setup_offset_y_item);
    
    if (aac_mode == AAC_ACCEPT) {
        XtUnmanageChild(ps_setup_frame);
    }
}

static void update_eps_setup_frame(void);
static void set_eps_setup_proc(void *data);
static Widget eps_setup_frame;
static Widget eps_setup_grayscale_item;
static Widget eps_setup_tight_bb_item;

void eps_gui_setup(void)
{
    Widget eps_setup_panel, eps_setup_rc, fr, rc;
    
    set_wait_cursor();
    if (eps_setup_frame == NULL) {
	eps_setup_frame = XmCreateDialogShell(app_shell, "EPS options", NULL, 0);
	handle_close(eps_setup_frame);
        eps_setup_panel = XtVaCreateWidget("device_panel", xmFormWidgetClass, 
                                        eps_setup_frame, NULL, 0);
        eps_setup_rc = XmCreateRowColumn(eps_setup_panel, "psetup_rc", NULL, 0);

	fr = CreateFrame(eps_setup_rc, "EPS options");
        rc = XmCreateRowColumn(fr, "rc", NULL, 0);
	eps_setup_grayscale_item = CreateToggleButton(rc, "Grayscale output");
	eps_setup_tight_bb_item = CreateToggleButton(rc, "Tight BBox");
	XtManageChild(rc);

	CreateSeparator(eps_setup_rc);

	CreateAACButtons(eps_setup_rc, eps_setup_panel, set_eps_setup_proc);
        
	XtManageChild(eps_setup_rc);
	XtManageChild(eps_setup_panel);
    }
    XtRaise(eps_setup_frame);
    update_eps_setup_frame();
    unset_wait_cursor();
}

static void update_eps_setup_frame(void)
{
    if (eps_setup_frame) {
        SetToggleButtonState(eps_setup_grayscale_item, eps_setup_grayscale);
        SetToggleButtonState(eps_setup_tight_bb_item, eps_setup_tight_bb);
    }
}

static void set_eps_setup_proc(void *data)
{
    int aac_mode;
    aac_mode = (int) data;
    
    if (aac_mode == AAC_CLOSE) {
        XtUnmanageChild(eps_setup_frame);
        return;
    }
    
    eps_setup_grayscale = GetToggleButtonState(eps_setup_grayscale_item);
    eps_setup_tight_bb = GetToggleButtonState(eps_setup_tight_bb_item);
    
    if (aac_mode == AAC_ACCEPT) {
        XtUnmanageChild(eps_setup_frame);
    }
}

#endif
