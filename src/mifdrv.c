/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
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
 * Driver for the Maker Interchange Format
 */

#include <config.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "defines.h"
#include "utils.h"
#include "cmath.h"
#include "draw.h"
#include "device.h"
#include "devlist.h"
#include "patterns.h"
#include "mifdrv.h"

extern FILE *prstream;

static Device_entry dev_mif = {DEVICE_FILE,
                               "MIF",
                               mifinitgraphics,
                               NULL,
                               NULL,
                               "mif",
                               FONTSRC_DEVICE,
                               FALSE,
                               {DEFAULT_PAGE_WIDTH, DEFAULT_PAGE_HEIGHT, 72.0},
                               NULL
                              };

/* mapping between Grace and MIF fill patterns. This is really ugly but
 * MIF does not allow to define document patterns */
static int mif_fillpattern(int fillpattern)
{
  switch (fillpattern) {
  case 0 :
    return 15;
  case 1 :
    return 0;
  case 2 :
    return 1;
  case 3 :
    return 2;
  case 4 :
    return 3;
  case 5 :
    return 4;
  case 6 :
    return 5;
  case 7 :
    return 6;
  case 8 :
    return 7;
  case 9 :
    return 8;
  case 10 :
    return 9;
  case 11 :
    return 10;
  case 12 :
    return 11;
  case 13 :
    return 12;
  case 14 :
    return 13;
  case 15 :
    return 14;
  case 16 :
    return 10;
  case 17 :
    return 11;
  case 18 :
    return 12;
  case 19 :
    return 2;
  case 20 :
    return 3;
  case 21 :
    return 4;
  case 22 :
    return 5;
  case 23 :
    return 6;
  case 24 :
    return 7;
  case 25 :
    return 8;
  case 26 :
    return 9;
  case 27 :
    return 10;
  case 28 :
    return 11;
  case 29 :
    return 12;
  case 30 :
    return 13;
  case 31 :
    return 14;
  default :
    return 0;
  }
}

/*
 * special characters processing
 */
static char hex_char(int i)
{
    return (i < 10) ? (i + '0') : (i - 10 + 'a');
}

/*
 * escape special characters
 */
static char *escape_specials(unsigned char *s)
{
    static char *es = NULL;
    int i, elen = 0;
    
    elen = 0;
    for (i = 0; i < strlen((char *) s); i++) {
        if (s[i] == '\t' || s[i] == '>' || s[i] == '`'
            || s[i] == '\'' || s[i] == '\\') {
            elen++;
        } else if (s[i] > 0x7f) {
            elen += 4;
        }
        elen++;
    }
    
    es = xrealloc(es, (elen + 1)*SIZEOF_CHAR);
    
    elen = 0;
    for (i = 0; i < strlen((char *) s); i++) {
        if (s[i] == '\t') {
            es[elen++] = '\\';
            es[elen++] = 't';
        } else if (s[i] == '>') {
            es[elen++] = '\\';
            es[elen++] = '>';
        } else if (s[i] == '`') {
            es[elen++] = '\\';
            es[elen++] = 'Q';
        } else if (s[i] == '\'') {
            es[elen++] = '\\';
            es[elen++] = 'q';
        } else if (s[i] == '\\') {
            es[elen++] = '\\';
            es[elen++] = '\\';
        } else if (s[i] > 0x7f) {
            es[elen++] = '\\';
            es[elen++] = 'x';
            es[elen++] = hex_char(s[i] >> 4);
            es[elen++] = hex_char(s[i] & 0xf);
            es[elen++] = ' ';
        } else {
            es[elen++] = (char) s[i];
        }
    }
    es[elen] = '\0';
    
    return (es);
}

int register_mif_drv(void)
{
    return register_device(dev_mif);
}

int mifinitgraphics(void)
{
    int i;
    double *data;
    double c, m, y, k;
    fRGB *frgb;

    /* device-dependent routines */
    devupdatecmap   = NULL;
    
    devdrawpixel    = mif_drawpixel;
    devdrawpolyline = mif_drawpolyline;
    devfillpolygon  = mif_fillpolygon;
    devdrawarc      = mif_drawarc;
    devfillarc      = mif_fillarc;
    devputpixmap    = mif_putpixmap;
    devputtext      = mif_puttext;

    devleavegraphics = mif_leavegraphics;

    data = (double *) xrealloc(NULL, SIZEOF_DOUBLE);
    if (data == NULL) {
        errmsg("Not enough memory for mif driver");
        return GRACE_EXIT_FAILURE;
    }
    *data = MIN2(page_width_pp, page_height_pp);
    set_curdevice_data((void *) data);

    fprintf(prstream, "<MIFFile 5.50> # Generated by %s\n",
            bi_version_string());
    fprintf(prstream, "<Units Upt>\n");

    fprintf(prstream, "<ColorCatalog\n");
    for (i = 0; i < number_of_colors(); i++) {
        frgb = get_frgb(i);
        if (frgb != NULL) {

            /* convert RGB to CMYK */
            if (frgb->red > 1e-3 || frgb->green > 1e-3 || frgb->blue > 1e-3) {
                c = 100.0 - 100.0*frgb->red;
                m = 100.0 - 100.0*frgb->green;
                y = 100.0 - 100.0*frgb->blue;
                k = 0.0;
            } else {
                c = 0.0;
                m = 0.0;
                y = 0.0;
                k = 100.0;
            }

            fprintf(prstream, " <Color\n");
            fprintf(prstream, "  <ColorTag `%s'>\n", get_colorname(i));
            fprintf(prstream, "  <ColorCyan %10.6f>\n", c);
            fprintf(prstream, "  <ColorMagenta %10.6f>\n", m);
            fprintf(prstream, "  <ColorYellow %10.6f>\n", y);
            fprintf(prstream, "  <ColorBlack %10.6f>\n", k);
            if (c < 0.1 && m < 0.1 && y < 0.1 && k > 99.9) {
                fprintf(prstream, "  <ColorAttribute ColorIsBlack>\n");
            } else if (c < 0.1 && m < 0.1 && y < 0.1 && k < 0.1) {
                fprintf(prstream, "  <ColorAttribute ColorIsWhite>\n");
            } else if (c < 0.1 && m > 99.9 && y > 99.9 && k < 0.1) {
                fprintf(prstream, "  <ColorAttribute ColorIsRed>\n");
            } else if (c > 99.9 && m < 0.1 && y > 99.9 && k < 0.1) {
                fprintf(prstream, "  <ColorAttribute ColorIsGreen>\n");
            } else if (c > 99.9 && m > 99.9 && y < 0.1 && k < 0.1) {
                fprintf(prstream, "  <ColorAttribute ColorIsBlue>\n");
            } else if (c > 99.9 && m < 0.1 && y < 0.1 && k < 0.1) {
                fprintf(prstream, "  <ColorAttribute ColorIsCyan>\n");
            } else if (c < 0.1 && m > 99.9 && y < 0.1 && k < 0.1) {
                fprintf(prstream, "  <ColorAttribute ColorIsMagenta>\n");
            } else if (c < 0.1 && m < 0.1 && y > 99.9 && k < 0.1) {
                fprintf(prstream, "  <ColorAttribute ColorIsYellow>\n");
            }
            fprintf(prstream, " > # end of Color\n");
        }
    }
    fprintf(prstream, "> # end of ColorCatalog\n");

    fprintf(prstream, "<Document\n");
    fprintf(prstream, " <DPageSize %8.3f pt %8.3f pt>\n",
            page_width_pp, page_height_pp);
    fprintf(prstream, "> # end of Document\n");

    fprintf(prstream, "<Page\n");
    fprintf(prstream, " <PageType BodyPage>\n");
    fprintf(prstream, " <PageNum `1'>\n");
    fprintf(prstream, " <PageAngle 0>\n");
    fprintf(prstream, " <PageBackground `Default'>\n");
    fprintf(prstream, " <Frame\n");
    fprintf(prstream, "  <ShapeRect  0.0 pt 0.0 pt %8.3f pt %8.3f pt>\n",
            page_width_pp, page_height_pp);
    fprintf(prstream, "  <FrameType NotAnchored>\n");
    
    return GRACE_EXIT_SUCCESS;
}

void mif_object_props (int draw, int fill)
{
    int i, ls;
    double lw;
    Pen pen;

    pen = getpen();
    if (draw) {
        fprintf(prstream, "   <Pen 0>\n");
        lw = getlinewidth();
        fprintf(prstream, "   <PenWidth %8.3f pt>\n", lw);

        fprintf(prstream, "   <DashedPattern\n");
        ls = getlinestyle();

        if (ls <= 1) {
        fprintf(prstream, "    <DashedStyle Solid>\n");
        } else {
          fprintf(prstream, "   <DashedStyle Dashed>\n");
          for (i = 0; i < dash_array_length[ls]; i++) {
            fprintf(prstream, "   <DashSegment %8.3f pt>\n",
                    lw*dash_array[ls][i]);
          }
        }
        fprintf(prstream, "   > # end of DashedPattern\n");
    } else {
        fprintf(prstream, "   <Pen 15>\n");
    }

    if (fill) {
        fprintf(prstream, "   <Fill %d>\n", mif_fillpattern(pen.pattern));
    } else {
        fprintf(prstream, "   <Fill 15>\n");
    }
    fprintf(prstream, "   <ObColor `%s'>\n", get_colorname(pen.color));

}

void mif_drawpixel(VPoint vp)
{
    Pen pen;
    double side;

    pen  = getpen();
    side = *((double *) get_curdevice_data());

    fprintf(prstream,
            "  <Rectangle\n");
    mif_object_props(FALSE, TRUE);
    fprintf(prstream,
            "   <ShapeRect %8.3f pt %8.3f pt %8.3f pt %8.3f pt>\n",
            vp.x*side, (1.0 - vp.y)*side, 72.0/page_dpi, 72.0/page_dpi);
    fprintf(prstream, "  > # end of Rectangle\n");

}

void mif_drawpolyline(VPoint *vps, int n, int mode)
{
    int i;
    double side;

    side = *((double *) get_curdevice_data());
    
    if (mode == POLYLINE_CLOSED) {
        fprintf(prstream, "  <Polygon\n");
    } else {
        fprintf(prstream, "  <PolyLine\n");
    }
    mif_object_props(TRUE, FALSE);
    for (i = 0; i < n; i++) {
        fprintf(prstream, "   <Point %8.3f pt %8.3f>\n",
                vps[i].x*side, (1.0 - vps[i].y)*side);
    }
    if (mode == POLYLINE_CLOSED) {
        fprintf(prstream, "  > # end of Polygon\n");
    } else {
        switch (getlinecap()) {
        case LINECAP_BUTT :
          fprintf(prstream, "   <HeadCap Butt>\n");
          fprintf(prstream, "   <TailCap Butt>\n");
          break;
        case LINECAP_ROUND :
          fprintf(prstream, "   <HeadCap Round>\n");
          fprintf(prstream, "   <TailCap Round>\n");
          break;
        case LINECAP_PROJ :
          fprintf(prstream, "   <HeadCap Square>\n");
          fprintf(prstream, "   <TailCap Square>\n");
          break;
        default :
          fprintf(prstream, "   <HeadCap Butt>\n");
          fprintf(prstream, "   <TailCap Butt>\n");
          break;
        }
        fprintf(prstream, "  > # end of PolyLine\n");
    }
}

void mif_fillpolygon(VPoint *vps, int nc)
{
    int i;
    double side;

    side = *((double *) get_curdevice_data());
    
    fprintf(prstream, "  <Polygon\n");
    mif_object_props(FALSE, TRUE);
    for (i = 0; i < nc; i++) {
        fprintf(prstream, "   <Point %8.3f pt %8.3f>\n",
                vps[i].x*side, (1.0 - vps[i].y)*side);
    }
    fprintf(prstream, "  > # end of Polygon\n");
}

static void mif_arc(int draw, int fill, VPoint vp1, VPoint vp2, int a1, int a2)
{

    double side;

    side = *((double *) get_curdevice_data());

    fprintf(prstream, "  <Arc\n");
    mif_object_props(draw, fill);
    fprintf(prstream, "   <ArcRect %8.3f pt %8.3f pt %8.3f pt %8.3f pt>\n",
            MIN2(vp1.x, vp2.x)*side, (1.0 - MAX2(vp1.y, vp2.y))*side,
            fabs(vp2.x - vp1.x)*side, fabs(vp2.y - vp1.y)*side);
    fprintf(prstream, "   <ArcTheta %d>\n",
            (a2 > 90) ? (450 - a2) : (90 - a2));
    fprintf(prstream, "   <ArcDTheta %d>\n", a2 - a1);
    switch (getlinecap()) {
    case LINECAP_BUTT :
        fprintf(prstream, "   <HeadCap Butt>\n");
        fprintf(prstream, "   <TailCap Butt>\n");
        break;
    case LINECAP_ROUND :
        fprintf(prstream, "   <HeadCap Round>\n");
        fprintf(prstream, "   <TailCap Round>\n");
        break;
    case LINECAP_PROJ :
        fprintf(prstream, "   <HeadCap Square>\n");
        fprintf(prstream, "   <TailCap Square>\n");
        break;
    default :
            fprintf(prstream, "   <HeadCap Butt>\n");
        fprintf(prstream, "   <TailCap Butt>\n");
        break;
    }
    fprintf(prstream, "  > # end of Arc\n");
}

void mif_drawarc(VPoint vp1, VPoint vp2, int a1, int a2)
{
    mif_arc(TRUE, FALSE, vp1, vp2, a1, a2);
}

void mif_fillarc(VPoint vp1, VPoint vp2, int a1, int a2, int mode)
{
    /* FIXME - mode */
    mif_arc(FALSE, TRUE, vp1, vp2, a1, a2);
}

/*
 * the following function does not work yet :-(
 */
void mif_putpixmap(VPoint vp, int width, int height, char *databits, 
                   int pixmap_bpp, int bitmap_pad, int pixmap_type)
{
    int i, j, k, paddedW;
    double side;
    fRGB *frgb;
    unsigned char tmpbyte;

    if (pixmap_bpp != 1 && pixmap_bpp != 8) {
        /* MIF supports only black and white or 256 colors images */
        return;
    }

    side = *((double *) get_curdevice_data());

    fprintf(prstream,
            "  <ImportObject\n");
    mif_object_props(FALSE, FALSE);
    fprintf(prstream,
            "   <ShapeRect %8.3f pt %8.3f pt %8.3f pt %8.3f pt>\n",
            vp.x*side, (1.0 - vp.y)*side,
            72.0*width/page_dpi, 72.0*height/page_dpi);
    fprintf(prstream, "   <ImportObFixedSize Yes>\n");
    fprintf(prstream, "=FrameImage\n");
    fprintf(prstream, "&%%v\n");
    fprintf(prstream, "&\\x\n");

    /* image header */
    fprintf(prstream, "&59a66a95\n");
    fprintf(prstream, "&%.8x\n", (unsigned int) width);
    fprintf(prstream, "&%.8x\n", (unsigned int) height);
    fprintf(prstream, "&%.8x\n", (unsigned int) pixmap_bpp);
    fprintf(prstream, "&00000000\n");
    fprintf(prstream, "&00000001\n");
    if (pixmap_bpp == 1) {
        fprintf(prstream, "&00000000\n");
        fprintf(prstream, "&00000000\n");

        /* image data */
        paddedW = PAD(width, bitmap_pad);

        for (k = 0; k < height; k++) {
            fprintf(prstream, "&");
            for (j = 0; j < paddedW/bitmap_pad; j++) {
                tmpbyte = reversebits((unsigned char) (databits)[k*paddedW/bitmap_pad + j]);
                fprintf(prstream, "%.2x", tmpbyte);
            }
            fprintf(prstream, "\n");
        }
    } else {
        fprintf(prstream, "&00000001\n");
        fprintf(prstream, "&00000300\n");

        /* colormap */
        for (i = 0; i < 256; i++) {
            /* red intensities */
            frgb = get_frgb(i);
            fprintf(prstream, "&%.2x\n",
                    (frgb != NULL) ? ((unsigned int) frgb->red) : 0);
        }
        for (i = 0; i < 256; i++) {
            /* green intensities */
            frgb = get_frgb(i);
            fprintf(prstream, "&%.2x\n",
                    (frgb != NULL) ? ((unsigned int) frgb->green) : 0);
        }
        for (i = 0; i < 256; i++) {
            /* blue intensities */
            frgb = get_frgb(i);
            fprintf(prstream, "&%.2x\n",
                    (frgb != NULL) ? ((unsigned int) frgb->blue) : 0);
        }

        /* image data */
        for (k = 0; k < height; k++) {
            fprintf(prstream, "&");
            for (j = 0; j < width; j++) {
                fprintf(prstream, "%.2x",
                        (unsigned int) (databits)[k*width+j]);
            }
            fprintf(prstream, "\n");
        }

    }

    fprintf(prstream, "&\\x\n");
    fprintf(prstream, "=EndInset\n");
    fprintf(prstream, "  > # end of ImportObject\n");

}

/*
 * the mif_puttext does not handle correctly composite strings
 */
void mif_puttext (VPoint start, VPoint end, double size, 
                  CompositeString *cstring)
{
    int iglyph;
    double side, dx, dy, angle, co, si;
    double scaled_size;
    char *fontalias, *dash, *family;
    VPoint current;
    GLYPH *glyph;

    side  = *((double *) get_curdevice_data());
    dx    = end.x - start.x;
    dy    = end.y - start.y;
    angle = atan2(dy, dx);
    if (angle < 0.0) {
        angle += M_PI + M_PI;
    }
    co      = cos(angle);
    si      = sin(angle);
    angle  *= 180.0/M_PI;
    iglyph  = 0;
    family  = NULL;
    current = start;
    while (cstring[iglyph].s != NULL) {
        scaled_size = size*cstring[iglyph].scale;
        fprintf(prstream, "  <TextLine\n");
        mif_object_props(FALSE, FALSE);
        fprintf(prstream, "   <TLOrigin %9.3f pt %9.3f pt>\n",
                current.x*side, (1.0 - current.y)*side);
        fprintf(prstream, "   <TLAlignment %s>\n",
                (cstring[iglyph].advancing == TEXT_ADVANCING_LR) ?
                "Left" : "Right");
        fprintf(prstream, "   <Angle %5.2f>\n", angle);
        fprintf(prstream, "   <Font\n");
        fprintf(prstream, "    <FTag `'>\n");

        fontalias = get_fontalias(cstring[iglyph].font);
        if ((dash = strchr(fontalias, '-')) == NULL) {
            family = copy_string(family, fontalias);
        } else {
            family    = xrealloc(family, dash - fontalias + 1);
            strncpy(family, fontalias, dash - fontalias);
            family[dash - fontalias] = '\0';
        }
        fprintf(prstream, "    <FFamily `%s'>\n", family);

        if (strstr(fontalias, "Bold") != NULL) {
            fprintf(prstream, "    <FWeight `Bold'>\n");
        } else {
            fprintf(prstream, "    <FWeight `Regular'>\n");
        }

        if (strstr(fontalias, "Italic") != NULL) {
            fprintf(prstream, "    <FAngle `Italic'>\n");
        } else if (strstr(fontalias, "Oblique") != NULL) {
            fprintf(prstream, "    <FAngle `Oblique'>\n");
        } else {
            fprintf(prstream, "    <FAngle `Regular'>\n");
        }

        fprintf(prstream, "    <FPostScriptName `%s'>\n", fontalias);

        fprintf(prstream, "    <FSize %8.3f pt>\n",
                size*cstring[iglyph].scale);
        fprintf(prstream, "    <FUnderlining %s>\n",
                (cstring[iglyph].underline == TRUE) ? "FSingle" : "FNoUnderlining");
        fprintf(prstream, "    <FOverline %s>\n",
                (cstring[iglyph].overline == TRUE) ? "Yes" : "No");
        fprintf(prstream, "   > # end of Font\n");
        fprintf(prstream, "   <String `%s'>\n",
                escape_specials((unsigned char *) cstring[iglyph].s));
        fprintf(prstream, "  > # end of TextLine\n");

        /* try to find the next origin (this probably don't work when
           subscripts, superscripts, or things like that occur, a
           better way would be to have some information from the
           calling WriteString function) */
	glyph = GetGlyphString(cstring[iglyph].font, scaled_size, angle,
                               T1_UNDERLINE * cstring[iglyph].underline |
                               T1_OVERLINE  * cstring[iglyph].overline,
                               cstring[iglyph].s);
        current.x += (co*glyph->metrics.advanceX
            - si*glyph->metrics.advanceY)/side;
        current.y += (si*glyph->metrics.advanceX
            + co*glyph->metrics.advanceY)/side;

        iglyph++;

    }

    copy_string(family, NULL);

}

void mif_leavegraphics(void)
{
    fprintf(prstream, " > # end of Frame\n");
    fprintf(prstream, "> # end of Page\n");
    fprintf(prstream, "# End of MIFFile\n");
}
