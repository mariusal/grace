/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1996-2003 Grace Development Team
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
#include "grace/canvas.h"
#include "devlist.h"
#include "mifdrv.h"

#define MIF_MARGIN 15.0

static double page_side = 0.0;

/* mapping between Grace and MIF fill patterns. This is really ugly but
 * MIF uses only 16 patterns which can only be customised on UNIX platforms
 * and there only for the whole FrameMaker-product and not for a single
 * document. */
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
 * escape special characters
 */
static char *escape_specials(unsigned char *s, int len)
{
    static char *es = NULL;
    int i, elen = 0;
    
    /* Define Array with all charactercodes from 128 to 255 for the
       conversion of the ISOLatin1 codes to FrameMaker codes.
       Characters, which are not part of the FrameMaker characterset
       are coded as \xc0 (exclamdown)
       The following conversions are defined
       onesuperior -> 1
       twosuperior -> 2
       threesuperior -> 3
       degree -> ring
       multiply -> x
       Yacute -> Y
       divide -> :
       yacute -> y
       Matthias Dillier, 10.1.2001 */
    static char *code[128] = {
        "80","81","82","83","84","85","86","87",
        "88","89","8a","8b","8c","8d","8e","8f",
        "f5","d4","d5","f6","f7","f8","f9","fa",
        "ac","99","fb","fc","9c","fd","fe","c0",
        "a0","c1","a2","a3","db","b4","c0","a4",
        "ac","a9","bb","c7","c2","2d","a8","f8",
        "fb","c0","32","33","ab","c0","a6","e1",
        "fc","31","bc","c8","c0","c0","c0","c0",
        "cb","e7","e5","cc","80","81","ae","82",
        "e9","83","e6","e8","ed","ea","eb","ec",
        "c0","84","f1","ee","ef","cd","85","78",
        "af","f4","f2","f3","86","59","c0","a7",
        "88","87","89","8b","8a","8c","be","8d",
        "8f","8e","90","91","93","92","94","95",
        "c0","96","98","97","99","9b","9a","3a",
        "bf","9d","9c","9e","9f","79","c0","d8"
    };
    
    elen = 0;
    for (i = 0; i < len; i++) {
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
    
    for (i = 0; i < len; i++) {
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

            /* Convert special characters to mif-charactercodes */
            es[elen++] = code[s[i] - 128][0];
            es[elen++] = code[s[i] - 128][1];
            es[elen++] = ' ';
        } else {
            es[elen++] = (char) s[i];
        }
    }
          
    es[elen] = '\0';
    
    return (es);
}

int register_mif_drv(Canvas *canvas)
{
    Device_entry *d;

    d = device_new("MIF", DEVICE_FILE, TRUE, (void *) &page_side);
    if (!d) {
        return -1;
    }
    
    device_set_fext(d, "mif");
    
    device_set_procs(d,
        mif_initgraphics,
        mif_leavegraphics,
        NULL,
        NULL,
        NULL,
        mif_drawpixel,
        mif_drawpolyline,
        mif_fillpolygon,
        mif_drawarc,
        mif_fillarc,
        mif_putpixmap,
        mif_puttext);
    
    return register_device(canvas, d);
}

int mif_initgraphics(const Canvas *canvas, void *data,
    const CanvasStats *cstats)
{
    int i;
    double c, m, y, k;
    double *side;
    fRGB frgb;
    FILE *prstream = canvas_get_prstream(canvas);

    side = (double *) data;
    *side = MIN2(page_width_pp(canvas), page_height_pp(canvas));

    fprintf(prstream, "<MIFFile 5.50> # Generated by %s\n",
            bi_version_string());
    fprintf(prstream, "<Units Upt>\n");

    fprintf(prstream, "<ColorCatalog\n");
    for (i = 0; i < number_of_colors(canvas); i++) {
        if (get_frgb(canvas, i, &frgb) == RETURN_SUCCESS) {

            /* convert RGB to CMYK */
            if (frgb.red > 1e-3 || frgb.green > 1e-3 || frgb.blue > 1e-3) {
                c = 100.0 - 100.0*frgb.red;
                m = 100.0 - 100.0*frgb.green;
                y = 100.0 - 100.0*frgb.blue;
                k = 0.0;
            } else {
                c = 0.0;
                m = 0.0;
                y = 0.0;
                k = 100.0;
            }

            fprintf(prstream, " <Color\n");
            fprintf(prstream, "  <ColorTag `%s'>\n",
                get_colorname(canvas, i));
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
            page_width_pp(canvas) + 2*MIF_MARGIN,
            page_height_pp(canvas) + 2*MIF_MARGIN);
    fprintf(prstream, " <DMargins 0 pt 0 pt 0 pt 0 pt>\n");
    fprintf(prstream, " <DColumns 1>\n");
    fprintf(prstream, "> # end of Document\n");

    fprintf(prstream, "<Page # Create a right master page.\n");
    fprintf(prstream, " <PageType RightMasterPage>\n");
    fprintf(prstream, " <PageTag `Right'>\n");
    fprintf(prstream, " <TextRect\n");
    fprintf(prstream, "   <ID 10>\n");
    fprintf(prstream, "   <Pen 15>\n");
    fprintf(prstream, "   <Fill 15>\n");
    fprintf(prstream, "   <ShapeRect 0 pt 0 pt %8.3f pt %8.3f pt>\n",
            page_width_pp(canvas) + 2*MIF_MARGIN,
            page_height_pp(canvas) + 2*MIF_MARGIN);
    fprintf(prstream, "   <TRNumColumns 1>\n");
    fprintf(prstream, "   <TRColumnGap 0.0 pt>\n");
    fprintf(prstream, " > # end of TextRect\n");
    fprintf(prstream, "> # end of Page\n");

    fprintf(prstream, "<Page # Create a body page.\n");
    fprintf(prstream, " <PageType BodyPage>\n");
    fprintf(prstream, " <PageNum `1'>\n");
    fprintf(prstream, " <PageAngle 0>\n");
    fprintf(prstream, " <PageBackground `Default'>\n");
    fprintf(prstream, " <TextRect\n");
    fprintf(prstream, "   <ID 20>\n");
    fprintf(prstream, "   <ShapeRect 0 pt 0 pt %8.3f pt %8.3f pt>\n",
            page_width_pp(canvas) + 2*MIF_MARGIN,
            page_height_pp(canvas) + 2*MIF_MARGIN);
    fprintf(prstream, "   <TRNumColumns 1> \n");
    fprintf(prstream, "   <TRColumnGap 0.0 pt>\n");
    fprintf(prstream, " > # end TextRect\n");
    fprintf(prstream, "> # end Page\n");

    fprintf(prstream, "<AFrames\n");
    fprintf(prstream, " <Frame\n");
    fprintf(prstream, "  <ID 30>\n");
    fprintf(prstream, "  <Pen 15>\n");
    fprintf(prstream, "  <Fill 15>\n");
    fprintf(prstream, "  <RunaroundGap  0 pt>\n");
    fprintf(prstream, "  <RunaroundType None>\n");
    fprintf(prstream, "   <ShapeRect 0 pt 0 pt %8.3f pt %8.3f pt>\n",
            page_width_pp(canvas) + 2*MIF_MARGIN,
            page_height_pp(canvas) + 2*MIF_MARGIN);
    fprintf(prstream, "  <FrameType RunIntoParagraph>\n");
    fprintf(prstream, "  <NSOffset  0.0 mm>\n");
    fprintf(prstream, "  <BLOffset  0.0 mm>\n");
    fprintf(prstream, "  <AnchorAlign Left>\n");

    return RETURN_SUCCESS;
}

void mif_object_props(const Canvas *canvas, double side, int draw, int fill)
{
    int i, ls;
    double lw;
    Pen pen;
    FILE *prstream = canvas_get_prstream(canvas);

    getpen(canvas, &pen);
    if (draw) {
        
        fprintf(prstream, "   <Pen 0>\n");
        lw = side*getlinewidth(canvas);
        fprintf(prstream, "   <PenWidth %8.3f pt>\n", lw);

        fprintf(prstream, "   <DashedPattern\n");
        ls = getlinestyle(canvas);

        if (ls <= 1) {
          fprintf(prstream, "    <DashedStyle Solid>\n");
        } else {
          LineStyle *linestyle = canvas_get_linestyle(canvas, ls);
          fprintf(prstream, "   <DashedStyle Dashed>\n");
          for (i = 0; i < linestyle->length; i++) {
            fprintf(prstream, "   <DashSegment %8.3f pt>\n",
                    lw*linestyle->array[i]);
          }
        }
        fprintf(prstream, "   > # end of DashedPattern\n");
    } else {
        fprintf(prstream, "   <Pen 15>\n");
    }

    if (fill) {
        fprintf(prstream, "   <Fill %d>\n",
            mif_fillpattern(pen.pattern));
    } else {
        fprintf(prstream, "   <Fill 15>\n");
    }
    fprintf(prstream, "   <ObColor `%s'>\n",
        get_colorname(canvas, pen.color));
    fprintf(prstream, "   <GroupID 1>\n");

}

void mif_drawpixel(const Canvas *canvas, void *data, const VPoint *vp)
{
    double side;
    FILE *prstream = canvas_get_prstream(canvas);

    side = *((double *) data);

    fprintf(prstream,
            "  <Rectangle\n");
    mif_object_props(canvas, side, FALSE, TRUE);
    fprintf(prstream,
            "   <ShapeRect %8.3f pt %8.3f pt %8.3f pt %8.3f pt>\n",
            vp->x*side + MIF_MARGIN, (1.0 - vp->y)*side + MIF_MARGIN,
            72.0/page_dpi(canvas), 72.0/page_dpi(canvas));
    fprintf(prstream, "  > # end of Rectangle\n");

}

void mif_drawpolyline(const Canvas *canvas, void *data,
    const VPoint *vps, int n, int mode)
{
    int i;
    double side;
    FILE *prstream = canvas_get_prstream(canvas);

    side = *((double *) data);
    
    if (mode == POLYLINE_CLOSED) {
        fprintf(prstream, "  <Polygon\n");
    } else {
        fprintf(prstream, "  <PolyLine\n");
    }
    mif_object_props(canvas, side, TRUE, FALSE);
    for (i = 0; i < n; i++) {
        fprintf(prstream, "   <Point %8.3f pt %8.3f>\n",
                vps[i].x*side + MIF_MARGIN, (1.0 - vps[i].y)*side + MIF_MARGIN);
    }
    if (mode == POLYLINE_CLOSED) {
        fprintf(prstream, "  > # end of Polygon\n");
    } else {
        switch (getlinecap(canvas)) {
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

void mif_fillpolygon(const Canvas *canvas, void *data,
    const VPoint *vps, int nc)
{
    int i;
    double side;
    FILE *prstream = canvas_get_prstream(canvas);

    side = *((double *) data);
    
    fprintf(prstream, "  <Polygon\n");
    mif_object_props(canvas, side, FALSE, TRUE);
    for (i = 0; i < nc; i++) {
        fprintf(prstream, "   <Point %8.3f pt %8.3f>\n",
                vps[i].x*side + MIF_MARGIN, (1.0 - vps[i].y)*side + MIF_MARGIN);
    }
    fprintf(prstream, "  > # end of Polygon\n");
}

static void mif_arc(const Canvas *canvas, double side,
    int draw, int fill, const VPoint *vp1, const VPoint *vp2, double a1, double a2)
{
    FILE *prstream = canvas_get_prstream(canvas);

    fprintf(prstream, "  <Arc\n");
    mif_object_props(canvas, side, draw, fill);
    fprintf(prstream, "   <ArcRect %8.3f pt %8.3f pt %8.3f pt %8.3f pt>\n",
            MIN2(vp1->x, vp2->x)*side + MIF_MARGIN,
            (1.0 - MAX2(vp1->y, vp2->y))*side + MIF_MARGIN,
            fabs(vp2->x - vp1->x)*side, fabs(vp2->y - vp1->y)*side);
    fprintf(prstream, "   <ArcTheta %8.3f>\n",
            (a2 > 90) ? (450 - a2) : (90 - a2));
    fprintf(prstream, "   <ArcDTheta %8.3f>\n", a2 - a1);
    switch (getlinecap(canvas)) {
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

void mif_drawarc(const Canvas *canvas, void *data,
    const VPoint *vp1, const VPoint *vp2, double a1, double a2)
{
    double side = *((double *) data);
    a2 += a1;
    mif_arc(canvas, side, TRUE, FALSE, vp1, vp2, a1, a2);
}

void mif_fillarc(const Canvas *canvas, void *data,
    const VPoint *vp1, const VPoint *vp2, double a1, double a2, int mode)
{
    double rx, ry;
    VPoint vp[3];
    double side = *((double *) data);

    a2 += a1;
    mif_arc(canvas, side, FALSE, TRUE, vp1, vp2, a1, a2);

    if (mode == ARCFILL_CHORD) {

        /* compute the associated triangle */
        rx      = fabs(vp2->x - vp1->x)/2;
        ry      = fabs(vp2->y - vp1->y)/2;
        vp[0].x = (vp1->x + vp2->x)/2;
        vp[0].y = (vp1->y + vp2->y)/2;
        vp[1].x = vp[0].x + rx * cos(a1*M_PI/180.0);
        vp[1].y = vp[0].y + ry * sin(a1*M_PI/180.0);
        vp[2].x = vp[0].x + rx * cos(a2*M_PI/180.0);
        vp[2].y = vp[0].y + ry * sin(a2*M_PI/180.0);

        if (a2 - a1 > 180) {
            /* the chord is larger than the default pieslice */

            if (a2 - a1 < 360) {
                /* the triangle is not degenerated, we need to fill it */
                mif_fillpolygon(canvas, data, vp, 3);
            }

        } else {
            /* the chord is smaller than the default pieslice */
#if 0 /* FIXME!!! */
            int old_color;
            /* this is a terrible hack ! MIF does not support filling only
               the chord of an arc so we overwrite with the background
               color, thus erasing underlying objects ... */
            old_color = getcolor(canvas);
            setcolor(canvas, getbgcolor(canvas));
            mif_fillpolygon(canvas, vp, 3);
            setcolor(canvas, old_color);
#endif
        }

    }
}

/*
 * the following function does not work yet :-(
 */
void mif_putpixmap(const Canvas *canvas, void *data,
    const VPoint *vp, int width, int height, char *databits,
    int pixmap_bpp, int bitmap_pad, int pixmap_type)
{
    int i, j, k, paddedW;
    double side;
    fRGB frgb;
    unsigned char tmpbyte;
    FILE *prstream = canvas_get_prstream(canvas);

    if (pixmap_bpp != 1 && pixmap_bpp != 8) {
        /* MIF supports only black and white or 256 colors images */
        return;
    }

    side = *((double *) data);

    fprintf(prstream,
            "  <ImportObject\n");
    mif_object_props(canvas, side, FALSE, FALSE);
    fprintf(prstream,
            "   <ShapeRect %8.3f pt %8.3f pt %8.3f pt %8.3f pt>\n",
            vp->x*side + MIF_MARGIN, (1.0 - vp->y)*side + MIF_MARGIN,
            72.0*width/page_dpi(canvas), 72.0*height/page_dpi(canvas));
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
                tmpbyte =reversebits((unsigned char) (databits)[k*paddedW/bitmap_pad + j]);
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
            get_frgb(canvas, i, &frgb);
            fprintf(prstream, "&%.2x\n", (unsigned int) frgb.red);
        }
        for (i = 0; i < 256; i++) {
            /* green intensities */
            get_frgb(canvas, i, &frgb);
            fprintf(prstream, "&%.2x\n", (unsigned int) frgb.green);
        }
        for (i = 0; i < 256; i++) {
            /* blue intensities */
            get_frgb(canvas, i, &frgb);
            fprintf(prstream, "&%.2x\n", (unsigned int) frgb.blue);
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
    fprintf(prstream, "   <GroupID 1>\n");
    fprintf(prstream, "  > # end of ImportObject\n");
}

void mif_puttext(const Canvas *canvas, void *data,
    const VPoint *vp, const char *s, int len, int font, const TextMatrix *tm,
    int underline, int overline, int kerning)
{
    char *fontalias, *fontfullname;
    double angle, side, size;
    Pen pen;
    FILE *prstream = canvas_get_prstream(canvas);

    side = *((double *) data);
    getpen(canvas, &pen);

    fprintf(prstream, "  <TextLine\n");
    mif_object_props(canvas, side, FALSE, FALSE);
    angle = atan2(tm->cyx, tm->cyy)*180.0/M_PI;
    if (angle < 0.0) {
        angle += 360.0;
    }
    fprintf(prstream, "   <Angle %f>\n", angle);
    fprintf(prstream, "   <TLOrigin %9.3f pt %9.3f pt>\n",
            side*vp->x + MIF_MARGIN, side*(1.0 - vp->y) + MIF_MARGIN);
    fprintf(prstream, "   <Font\n");
    fprintf(prstream, "    <FTag `'>\n");

    fontalias = get_fontalias(canvas, font);
    fontfullname = get_fontfullname(canvas, font);

    fprintf(prstream, "    <FFamily `%s'>\n", get_fontfamilyname(canvas, font));
    fprintf(prstream, "    <FWeight `%s'>\n", get_fontweight(canvas, font));

    if (strstr(fontfullname, "UltraCompressed") != NULL) {
        fprintf(prstream, "    <FVar `UltraCompressed'>\n");
    } else if (strstr(fontfullname, "ExtraCompressed") != NULL) {
        fprintf(prstream, "    <FVar `ExtraCompressed'>\n");
    } else if (strstr(fontfullname, "Compressed") != NULL) {
        fprintf(prstream, "    <FVar `Compressed'>\n");
    } else if (strstr(fontfullname, "UltraCondensed") != NULL) {
        fprintf(prstream, "    <FVar `UltraCondensed'>\n");
    } else if (strstr(fontfullname, "ExtraCondensed") != NULL) {
        fprintf(prstream, "    <FVar `ExtraCondensed'>\n");
    } else if (strstr(fontfullname, "Condensed") != NULL) {
        fprintf(prstream, "    <FVar `Condensed'>\n");
    } else if (strstr(fontfullname, "Narrow") != NULL) {
        fprintf(prstream, "    <FVar `Narrow'>\n");
    } else if (strstr(fontfullname, "Wide") != NULL) {
        fprintf(prstream, "    <FVar `Wide'>\n");
    } else if (strstr(fontfullname, "Poster") != NULL) {
        fprintf(prstream, "    <FVar `Poster'>\n");
    } else if (strstr(fontfullname, "Expanded") != NULL) {
        fprintf(prstream, "    <FVar `Expanded'>\n");
    } else if (strstr(fontfullname, "ExtraExtended") != NULL) {
        fprintf(prstream, "    <FVar `ExtraExtended'>\n");
    } else if (strstr(fontfullname, "Extended") != NULL) {
        fprintf(prstream, "    <FVar `Extended'>\n");
    }
    if (get_italic_angle(canvas, font) != 0) {
        if (strstr(fontfullname, "Italic") != NULL) {
            fprintf(prstream, "    <FAngle `Italic'>\n");
        } else if (strstr(fontfullname, "Obliqued") != NULL) {
            fprintf(prstream, "    <FAngle `Obliqued'>\n");
        } else if (strstr(fontfullname, "Oblique") != NULL) {
            fprintf(prstream, "    <FAngle `Oblique'>\n");
        } else if (strstr(fontfullname, "Upright") != NULL) {
            fprintf(prstream, "    <FAngle `Upright'>\n");
        } else if (strstr(fontfullname, "Kursiv") != NULL) {
            fprintf(prstream, "    <FAngle `Kursiv'>\n");
        } else if (strstr(fontfullname, "Cursive") != NULL) {
            fprintf(prstream, "    <FAngle `Cursive'>\n");
        } else if (strstr(fontfullname, "Slanted") != NULL) {
            fprintf(prstream, "    <FAngle `Slanted'>\n");
        } else if (strstr(fontfullname, "Inclined") != NULL) {
            fprintf(prstream, "    <FAngle `Inclined'>\n");
        } else {
            fprintf(prstream, "    <FAngle `Italic'>\n");
        }
    }

    size = fabs((tm->cxx*tm->cyy - tm->cxy*tm->cyx)
             / sqrt(tm->cxx*tm->cxx + tm->cyx*tm->cyx));
    fprintf(prstream, "    <FSize %9.3f pt>\n", side*size);

    fprintf(prstream, "    <FPostScriptName `%s'>\n", fontalias);

    fprintf(prstream, "    <FUnderlining %s>\n",
            (underline == TRUE) ? "FSingle" : "FNoUnderlining");
    fprintf(prstream, "    <FOverline %s>\n",
            (overline == TRUE) ? "Yes" : "No");
    fprintf(prstream, "    <FColor `%s'>\n",
        get_colorname(canvas, pen.color));
    fprintf(prstream, "   > # end of Font\n");
    fprintf(prstream, "   <String `%s'>\n",
            escape_specials((unsigned char *) s, len));
    fprintf(prstream, "  > # end of TextLine\n");
}

void mif_leavegraphics(const Canvas *canvas, void *data,
    const CanvasStats *cstats)
{
    FILE *prstream = canvas_get_prstream(canvas);
    fprintf(prstream, " <Group\n");
    fprintf(prstream, "  <ID 1>\n");
    fprintf(prstream, " > # end of Group\n");

    fprintf(prstream, " > # end of Frame\n");
    fprintf(prstream, "> # end of AFrames\n");
    fprintf(prstream, "<TextFlow\n");
    fprintf(prstream, " <TFTag `A'>\n");
    fprintf(prstream, " <TFAutoConnect Yes>\n");
    fprintf(prstream, " <Para\n");
    fprintf(prstream, "  <ParaLine\n");
    fprintf(prstream, "   <TextRectID 10>\n");
    fprintf(prstream, "  > # end of ParaLine\n");
    fprintf(prstream, " > # end of Para\n");
    fprintf(prstream, "> # end of TextFlow\n");
    fprintf(prstream, "<TextFlow\n");
    fprintf(prstream, " <TFTag `A'>\n");
    fprintf(prstream, " <TFAutoConnect Yes>\n");
    fprintf(prstream, " <Para\n");
    fprintf(prstream, " <TextRectID 20>\n");
    fprintf(prstream, "   <ParaLine\n");
    fprintf(prstream, "     <AFrame 30>\n");
    fprintf(prstream, "  > # end of ParaLine\n");
    fprintf(prstream, " > # end of Para\n");
    fprintf(prstream, "> # end of TextFlow\n");
          
    fprintf(prstream, "# End of MIFFile\n");
}
