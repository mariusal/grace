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
 * Driver for the Scalable Vector Graphics Format from W3C
 */

#include <config.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "defines.h"
#include "utils.h"
#include "cmath.h"
#include "draw.h"
#include "graphs.h"
#include "devlist.h"
#include "svgdrv.h"

typedef struct {
    double side;
    int   *pattern_defined;
    int   *pattern_empty;
    int   *pattern_full;
    int    group_is_open;
    double line_width;
    Pen    pen;
    int    fillrule;
    int    linecap;
    int    linejoin;
    int    linestyle;
    int    draw;
    int    fill;
} Svg_data;

static Device_entry dev_svg = {
    DEVICE_FILE,
    "SVG",
    "svg",
    TRUE,
    FALSE,
    {DEFAULT_PAGE_WIDTH, DEFAULT_PAGE_HEIGHT, 72.0},
    
    FALSE,
    FALSE,

    svginitgraphics,
    NULL,
    NULL,
    NULL,
    svg_leavegraphics,
    svg_drawpixel,
    svg_drawpolyline,
    svg_fillpolygon,
    svg_drawarc,
    svg_fillarc,
    svg_putpixmap,
    svg_puttext,

    NULL
};

static Svg_data *init_svg_data(const Canvas *canvas)
{
    Svg_data *data;

    /* we need to perform the allocations */
    data = (Svg_data *) xmalloc(sizeof(Svg_data));
    if (data == NULL) {
        return NULL;
    }

    memset(data, 0, sizeof(Svg_data));
    
    return data;
}

/*
 * SVG conventions :
 *   Y coordinates increase downwards
 *   angles increase counterclockwise
 */

/*
 * convert coordinate system
 */
static double convertX (const Svg_data *data, double x)
{
    return x*data->side;
}

static double convertY (const Svg_data *data, double y)
{
    return (1.0 - y)*data->side;
}

static double convertW (const Svg_data *data, double width)
{
    return width*data->side;
}

static double convertH (const Svg_data *data, double height)
{
    return height*data->side;
}

int register_svg_drv(Canvas *canvas)
{
    Svg_data *data;
    
    data = init_svg_data(canvas);
    if (!data) {
        return RETURN_FAILURE;
    }
    dev_svg.data = data;

    return register_device(canvas, &dev_svg);
}

static void define_pattern(Svg_data *data, int i)
{
#ifndef EXPERIMENTAL_SVG_PATTERNS
    data->pattern_full[i]  = TRUE;
    data->pattern_defined[i] = TRUE;
    return;
#else
    int j, k, l;

    if (data->pattern_defined[i] == TRUE) {
        return;
    }

    /* testing if the pattern is either empty or full */
    data->pattern_empty[i] = TRUE;
    data->pattern_full[i]  = TRUE;
    for (j = 0; j < 32; j++) {
        if (pat_bits[i][j] != 0x00) {
            data->pattern_empty[i] = FALSE;
        }
        if (pat_bits[i][j] != 0xff) {
            data->pattern_full[i] = FALSE;
        }
    }

    if (data->pattern_empty[i] != TRUE && data->pattern_full[i] != TRUE) {
        /* this is an horrible hack ! */
        /* we define pixels as squares in vector graphics */
        fprintf(canvas->prstream,
                "  <defs>\n   <pattern id=\"pattern%d\""
                " width=\"%d\" height=\"%d\">\n",
                i, 16, 16);
        for (j = 0; j < 256; j++) {
            k = j/16;
            l = j%16;
            if ((pat_bits[i][j/8] >> (j%8)) & 0x01) {
                /* the bit is set */
                fprintf(canvas->prstream,
                        "     <rect x=\"%d\" y=\"%d\""
                        " width=\"1\" height=\"1\"/>\n",
                        l, 15 - k);
            }
        }
        fprintf(canvas->prstream, "   </pattern>\n  </defs>\n");
    }

    data->pattern_defined[i] = TRUE;
#endif
}

/*
 * escape special characters
 */
static char *escape_specials(unsigned char *s, int len)
{
    static char *es = NULL;
    int i, elen = 0;

    elen = 0;
    for (i = 0; i < len; i++) {
        if (s[i] == '&') {
            elen += 4;
        } else if (s[i] == '<' || s[i] == '>') {
            elen += 3;
        }
        elen++;
    }

    es = xrealloc(es, (elen + 1)*SIZEOF_CHAR);

    elen = 0;
    for (i = 0; i < len; i++) {
        if (s[i] == '&') {
            es[elen++] = '&';
            es[elen++] = 'a';
            es[elen++] = 'm';
            es[elen++] = 'p';
            es[elen++] = ';';
        } else if (s[i] == '<') {
            es[elen++] = '&';
            es[elen++] = 'l';
            es[elen++] = 't';
            es[elen++] = ';';
        } else if (s[i] == '>') {
            es[elen++] = '&';
            es[elen++] = 'g';
            es[elen++] = 't';
            es[elen++] = ';';
        } else {
            es[elen++] = (char) s[i];
        }
    }
    es[elen] = '\0';

    return (es);
}

int svginitgraphics(const Canvas *canvas, const CanvasStats *cstats)
{
    Svg_data *data;
    int i;

    data = get_curdevice_data(canvas);
    
    data->pattern_defined = NULL;
    data->pattern_empty = NULL;
    data->pattern_full = NULL;

    data->side = MIN2(page_width_pp(canvas), page_height_pp(canvas));

    data->pattern_defined = 
        xrealloc(data->pattern_defined, number_of_patterns(canvas)*SIZEOF_INT);
    data->pattern_empty   =
        xrealloc(data->pattern_empty,   number_of_patterns(canvas)*SIZEOF_INT);
    data->pattern_full    =
        xrealloc(data->pattern_full,    number_of_patterns(canvas)*SIZEOF_INT);
    for (i = 0; i < number_of_patterns(canvas); i++) {
        data->pattern_defined[i] = FALSE;
        data->pattern_empty[i]   = FALSE;
        data->pattern_full[i]    = FALSE;
    }

    data->group_is_open = FALSE;
    data->line_width    = 0.0;
    data->pen.color     = 0;
    data->pen.pattern   = 0;
    data->fillrule      = 0;
    data->linecap       = 0;
    data->linejoin      = 0;
    data->linestyle     = 0;
    data->draw          = FALSE;
    data->fill          = FALSE;

    fprintf(canvas->prstream, "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n");
    fprintf(canvas->prstream, "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 20000303 Stylable//EN\"");
    fprintf(canvas->prstream, " \"http://www.w3.org/TR/2000/03/WD-SVG-20000303/DTD/svg-20000303-stylable.dtd\">\n");
    fprintf(canvas->prstream,
        "<!-- generated by %s -->\n", bi_version_string());
    fprintf(canvas->prstream, "<svg xml:space=\"preserve\" ");
    fprintf(canvas->prstream,
        "width=\"%.4fin\" height=\"%.4fin\" viewBox=\"%.4f %.4f %.4f %.4f\">\n",
        page_width_in(canvas), page_height_in(canvas),
        0.0, 0.0, page_width_pp(canvas), page_height_pp(canvas));

    /* project description */
    if (get_project_description() != NULL) {
        fprintf(canvas->prstream,
            " <desc>%s</desc>\n", get_project_description());
    }
    
    return RETURN_SUCCESS;
}

static void svg_group_props(const Canvas *canvas, int draw, int fill)
{
    int i, needs_group;
    double lw;
    Pen pen;
    int fillrule, linecap, linejoin, linestyle;
    RGB rgb;
    int red, green, blue;
    Svg_data *data;

    data = (Svg_data *) get_curdevice_data(canvas);

    /* do we need to redefine a group with new properties ? */
    needs_group = (data->group_is_open == TRUE) ? FALSE : TRUE;
    lw        = data->side*getlinewidth(canvas);
    fillrule  = getfillrule(canvas);
    linecap   = getlinecap(canvas);
    linejoin  = getlinejoin(canvas);
    linestyle = getlinestyle(canvas);
    if (fabs(lw - data->line_width) >= 1.0e-6*(1.0 + fabs(data->line_width))) {
        needs_group = TRUE;
    }
    getpen(canvas, &pen);
    if ((pen.color != data->pen.color) || (pen.pattern != data->pen.pattern)) {
        needs_group = TRUE;
    }
    if (fillrule != data->fillrule) {
        needs_group = TRUE;
    }
    if (linecap != data->linecap) {
        needs_group = TRUE;
    }
    if (linejoin != data->linejoin) {
        needs_group = TRUE;
    }
    if (linestyle != data->linestyle) {
        needs_group = TRUE;
    }
    if ((draw != data->draw) || (fill != data->fill)) {
        needs_group = TRUE;
    }

    if (needs_group == TRUE) {
        /* we need to write the characteristics of the group */

        if (data->group_is_open == TRUE) {
            /* first, we should close the preceding group */
            fprintf(canvas->prstream, "  </g>\n");
            data->group_is_open = FALSE;
        }

        define_pattern(data, pen.pattern);
        if (get_rgb(canvas, pen.color, &rgb) == RETURN_SUCCESS) {
            red   = rgb.red   >> (GRACE_BPP - 8);
            green = rgb.green >> (GRACE_BPP - 8);
            blue  = rgb.blue  >> (GRACE_BPP - 8);
        } else {
            red   = 0;
            green = 0;
            blue  = 0;
        }

        if (fill && data->pattern_empty[pen.pattern] != TRUE) {
            if (data->pattern_full[pen.pattern] == TRUE) {
                fprintf(canvas->prstream,
                    "  <g style=\"fill:#%2.2X%2.2X%2.2X", red, green, blue);
            } else {
                fprintf(canvas->prstream,
                    "  <g style=\"color:#%2.2X%2.2X%2.2X",
                        red, green, blue);
                fprintf(canvas->prstream,
                    "; fill:url(#pattern%d)", pen.pattern);
            }
            if (getfillrule(canvas) == FILLRULE_WINDING) {
                fprintf(canvas->prstream, "; fill-rule:nonzero");
            } else {
                fprintf(canvas->prstream, "; fill-rule:evenodd");
            }
        } else {
            fprintf(canvas->prstream, "  <g style=\"fill:none");
        }

        if (draw) {

            fprintf(canvas->prstream,
                "; stroke:#%2.2X%2.2X%2.2X", red, green, blue);

            fprintf(canvas->prstream, "; stroke-width:%8.4f", lw);

            switch (linecap) {
            case LINECAP_BUTT :
                fprintf(canvas->prstream, "; stroke-linecap:butt");
                break;
            case LINECAP_ROUND :
                fprintf(canvas->prstream, "; stroke-linecap:round");
                break;
            case LINECAP_PROJ :
                fprintf(canvas->prstream, "; stroke-linecap:square");
                break;
            default :
                fprintf(canvas->prstream, "; stroke-linecap:inherit");
                break;
            }

            switch (linejoin) {
            case LINEJOIN_MITER :
                fprintf(canvas->prstream, "; stroke-linejoin:miter");
                break;
            case LINEJOIN_ROUND :
                fprintf(canvas->prstream, "; stroke-linejoin:round");
                break;
            case LINEJOIN_BEVEL :
                fprintf(canvas->prstream, "; stroke-linejoin:bevel");
                break;
            default :
                fprintf(canvas->prstream, "; stroke-linejoin:inherit");
                break;
            }

            if (linestyle <= 1) {
                fprintf(canvas->prstream, "; stroke-dasharray:none");
            } else {
                LineStyle *ls = canvas_get_linestyle(canvas, linestyle);
                fprintf(canvas->prstream, "; stroke-dasharray:");
                for (i = 0; i < ls->length; i++) {
                    fprintf(canvas->prstream,
                        " %d", (int) rint(lw*ls->array[i]));
                }
            }
        }

        fprintf(canvas->prstream, "\">\n");


        data->group_is_open = TRUE;
        data->line_width    = lw;
        data->pen           = pen;
        data->fillrule      = fillrule;
        data->linecap       = linecap;
        data->linejoin      = linejoin;
        data->linestyle     = linestyle;
        data->draw          = draw;
        data->fill          = fill;
    }
}

void svg_drawpixel(const Canvas *canvas, const VPoint *vp)
{
    Svg_data *data = (Svg_data *) get_curdevice_data(canvas);
    svg_group_props(canvas, FALSE, TRUE);
    fprintf(canvas->prstream,
            "   <rect x=\"%.4f\" y=\"%.4f\" width=\"%.4f\" height=\"%.4f\"/>\n",
            convertX(data, vp->x), convertY(data, vp->y),
            convertW(data, 1.0), convertH(data, 1.0));
}

void svg_drawpolyline(const Canvas *canvas, const VPoint *vps, int n, int mode)
{
    Svg_data *data = (Svg_data *) get_curdevice_data(canvas);
    int i;

    if (n <= 0) {
        return;
    }

    svg_group_props(canvas, TRUE, FALSE);
    fprintf(canvas->prstream, "   <path d=\"M%.4f,%.4f",
            convertX(data, vps[0].x), convertY(data, vps[0].y));
    for (i = 1; i < n; i++) {
        if (i%10 == 0) {
            fprintf(canvas->prstream, "\n            ");
        }
        fprintf(canvas->prstream,
            "L%.4f,%.4f", convertX(data, vps[i].x), convertY(data, vps[i].y));
    }
    if (mode == POLYLINE_CLOSED) {
        fprintf(canvas->prstream, "z\"/>\n");
    } else {
        fprintf(canvas->prstream, "\"/>\n");
    }

}

void svg_fillpolygon(const Canvas *canvas, const VPoint *vps, int nc)
{
    Svg_data *data = (Svg_data *) get_curdevice_data(canvas);
    int i;

    if (nc <= 0) {
        return;
    }

    svg_group_props(canvas, FALSE, TRUE);
    fprintf(canvas->prstream, "   <path  d=\"M%.4f,%.4f",
            convertX(data, vps[0].x), convertY(data, vps[0].y));
    for (i = 1; i < nc; i++) {
        if (i%10 == 0) {
            fprintf(canvas->prstream, "\n             ");
        }
        fprintf(canvas->prstream,
            "L%.4f,%.4f", convertX(data, vps[i].x), convertY(data, vps[i].y));
    }
    fprintf(canvas->prstream, "z\"/>\n");
}

void svg_drawarc(const Canvas *canvas,
    const VPoint *vp1, const VPoint *vp2, int a1, int a2)
{
    Svg_data *data = (Svg_data *) get_curdevice_data(canvas);
    VPoint center;
    double rx, ry;

    if (a1 == a2) {
        return;
    }
    
    center.x = 0.5*(vp1->x + vp2->x);
    center.y = 0.5*(vp1->y + vp2->y);
    rx       = 0.5*fabs(vp2->x - vp1->x);
    ry       = 0.5*fabs(vp2->y - vp1->y);

    svg_group_props(canvas, TRUE, FALSE);

    if ((a1 - a2)%360 == 0) {
        fprintf(canvas->prstream,
            "   <ellipse  rx=\"%.4f\" ry=\"%.4f\" cx=\"%.4f\" cy=\"%.4f\"/>\n",
            convertW(data, rx), convertH(data, ry),
            convertX(data, center.x), convertY(data, center.y));
    } else {
        VPoint start, end;
        
        start.x = center.x + rx*cos((M_PI/180.0)*a1);
        start.y = center.y + ry*sin((M_PI/180.0)*a1);
        end.x   = center.x + rx*cos((M_PI/180.0)*a2);
        end.y   = center.y + ry*sin((M_PI/180.0)*a2);

        fprintf(canvas->prstream,
            "   <path  d=\"M%.4f, %.4fA%.4f, %.4f %d %d %d %.4f, %.4f\"/>\n",
            convertX(data, start.x), convertY(data, start.y),
            convertW(data, rx), convertH(data, ry),
            0,
            (abs(a2 - a1) > 180) ? 1 : 0,
            (a2 > a1) ? 0 : 1,
            convertX(data, end.x), convertY(data, end.y));
    }
}

void svg_fillarc(const Canvas *canvas,
    const VPoint *vp1, const VPoint *vp2, int a1, int a2, int mode)
{
    Svg_data *data = (Svg_data *) get_curdevice_data(canvas);
    VPoint center;
    double rx, ry;

    if (a1 == a2) {
        return;
    }

    center.x = 0.5*(vp1->x + vp2->x);
    center.y = 0.5*(vp1->y + vp2->y);
    rx       = 0.5*fabs(vp2->x - vp1->x);
    ry       = 0.5*fabs(vp2->y - vp1->y);

    svg_group_props(canvas, FALSE, TRUE);

    if ((a1 - a2)%360 == 0) {
        fprintf(canvas->prstream,
            "   <ellipse  rx=\"%.4f\" ry=\"%.4f\" cx=\"%.4f\" cy=\"%.4f\"/>\n",
            convertW(data, rx), convertH(data, ry),
            convertX(data, center.x), convertY(data, center.y));
    } else {
        VPoint start, end;
        
        start.x = center.x + rx*cos((M_PI/180.0)*a1);
        start.y = center.y + ry*sin((M_PI/180.0)*a1);
        end.x   = center.x + rx*cos((M_PI/180.0)*a2);
        end.y   = center.y + ry*sin((M_PI/180.0)*a2);

        if (mode == ARCFILL_CHORD) {
            fprintf(canvas->prstream,
                "   <path  d=\"M%.4f, %.4fA%.4f, %.4f %d %d %d %.4f, %.4fz\"/>\n",
                convertX(data, start.x), convertY(data, start.y),
                convertW(data, rx), convertH(data, ry),
                0,
                (abs(a2 - a1) > 180) ? 1 : 0,
                (a2 > a1) ? 0 : 1,
                convertX(data, end.x), convertY(data, end.y));
        } else {
            fprintf(canvas->prstream,
                "   <path  d=\"M%.4f,%.4fL%.4f,%.4fA%.4f,%.4f %d %d %d %.4f,%.4fz\"/>\n",
                convertX(data, center.x), convertY(data, center.y),
                convertX(data, start.x), convertY(data, start.y),
                convertW(data, rx), convertH(data, ry),
                0,
                (abs(a2 - a1) > 180) ? 1 : 0,
                (a2 > a1) ? 0 : 1,
                convertX(data, end.x), convertY(data, end.y));
        }
    }
}

void svg_putpixmap(const Canvas *canvas,
    const VPoint *vp, int width, int height, char *databits,
    int pixmap_bpp, int bitmap_pad, int pixmap_type)
{
    /* not implemented yet */
}

void svg_puttext(const Canvas *canvas,
    const VPoint *vp, const char *s, int len, int font, const TextMatrix *tm,
    int underline, int overline, int kerning)
{
    char *fontalias, *dash, *family;
    Svg_data *data = get_curdevice_data(canvas);
    double fsize = data->side;

    svg_group_props(canvas, FALSE, TRUE);
    
    fprintf(canvas->prstream, "   <text  ");

    family  = NULL;
    fontalias = get_fontalias(canvas, font);
    if ((dash = strchr(fontalias, '-')) == NULL) {
        family = copy_string(family, fontalias);
    } else {
        family    = xrealloc(family, dash - fontalias + 1);
        strncpy(family, fontalias, dash - fontalias);
        family[dash - fontalias] = '\0';
    }
    fprintf(canvas->prstream, " style=\"font-family:%s", family);
    copy_string(family, NULL);

    if (strstr(fontalias, "Italic") != NULL) {
        fprintf(canvas->prstream, "; font-style:italic");
    } else {
        if (strstr(fontalias, "Oblique") != NULL) {
            fprintf(canvas->prstream, "; font-style:oblique");
        } else {
            fprintf(canvas->prstream, "; font-style:normal");
        }
    }

    if (strstr(fontalias, "Bold") != NULL) {
        fprintf(canvas->prstream, "; font-weight:bold");
    } else {
        fprintf(canvas->prstream, "; font-weight:normal");
    }

    fprintf(canvas->prstream, "; font-size:%.4f", fsize);

    if (underline == TRUE) {
        if (overline == TRUE) {
            fprintf(canvas->prstream, "; text-decoration:underline|overline");
        } else {
            fprintf(canvas->prstream, "; text-decoration:underline");
        }
    } else {
        if (overline == TRUE) {
            fprintf(canvas->prstream, "; text-decoration:overline");
        }
    }

    fprintf(canvas->prstream, "\" transform=\"matrix(%.4f,%.4f,%.4f,%.4f,%.4f,%.4f)\">",
            tm->cxx, -tm->cyx,
            -tm->cxy, tm->cyy,
            convertX(data, vp->x), convertY(data, vp->y));

    fprintf(canvas->prstream, escape_specials((unsigned char *) s, len));

    fprintf(canvas->prstream, "</text>\n");
}

void svg_leavegraphics(const Canvas *canvas, const CanvasStats *cstats)
{
    Svg_data *data = (Svg_data *) get_curdevice_data(canvas);
    if (data->group_is_open == TRUE) {
        fprintf(canvas->prstream, "  </g>\n");
        data->group_is_open = FALSE;
    }
    fprintf(canvas->prstream, "</svg>\n");
}
