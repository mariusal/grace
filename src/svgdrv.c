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
 * Driver for the Scalable Vector Graphics Format from W3C
 */

#include <config.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "defines.h"
#include "utils.h"
#include "grace/canvas.h"
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
static double convertX (const Svg_data *svgdata, double x)
{
    return x*svgdata->side;
}

static double convertY (const Svg_data *svgdata, double y)
{
    return (1.0 - y)*svgdata->side;
}

static double convertW (const Svg_data *svgdata, double width)
{
    return width*svgdata->side;
}

static double convertH (const Svg_data *svgdata, double height)
{
    return height*svgdata->side;
}

int register_svg_drv(Canvas *canvas)
{
    Device_entry *d;
    Svg_data *data;
    
    data = init_svg_data(canvas);
    if (!data) {
        return -1;
    }
    
    d = device_new("SVG", DEVICE_FILE, TRUE, (void *) data);
    if (!d) {
        xfree(data);
        return -1;
    }
    
    device_set_fext(d, "svg");
    
    device_set_procs(d,
        svg_initgraphics,
        svg_leavegraphics,
        NULL,
        NULL,
        NULL,
        svg_drawpixel,
        svg_drawpolyline,
        svg_fillpolygon,
        svg_drawarc,
        svg_fillarc,
        svg_putpixmap,
        svg_puttext);
    
    return register_device(canvas, d);
}

static void define_pattern(const Canvas *canvas, Svg_data *svgdata, int i)
{
#ifndef EXPERIMENTAL_SVG_PATTERNS
    svgdata->pattern_full[i]  = TRUE;
    svgdata->pattern_defined[i] = TRUE;
    return;
#else
    int j, k, l;
    Pattern *pat;

    if (svgdata->pattern_defined[i] == TRUE) {
        return;
    }

    /* testing if the pattern is either empty or full */
    svgdata->pattern_empty[i] = TRUE;
    svgdata->pattern_full[i]  = TRUE;
    pat = canvas_get_pattern(canvas, i);
    for (j = 0; j < 32; j++) {
        if (pat->bits[j] != 0x00) {
            svgdata->pattern_empty[i] = FALSE;
        }
        if (pat->bits[j] != 0xff) {
            svgdata->pattern_full[i] = FALSE;
        }
    }

    if (svgdata->pattern_empty[i] != TRUE && svgdata->pattern_full[i] != TRUE) {
        /* this is an horrible hack ! */
        /* we define pixels as squares in vector graphics */
        fprintf(canvas->prstream,
                "  <defs>\n   <pattern id=\"pattern%d\""
                " width=\"%d\" height=\"%d\">\n",
                i, 16, 16);
        for (j = 0; j < 256; j++) {
            k = j/16;
            l = j%16;
            if ((pat->bits[j/8] >> (j%8)) & 0x01) {
                /* the bit is set */
                fprintf(canvas->prstream,
                        "     <rect x=\"%d\" y=\"%d\""
                        " width=\"1\" height=\"1\"/>\n",
                        l, 15 - k);
            }
        }
        fprintf(canvas->prstream, "   </pattern>\n  </defs>\n");
    }

    svgdata->pattern_defined[i] = TRUE;
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

int svg_initgraphics(const Canvas *canvas, void *data,
    const CanvasStats *cstats)
{
    Svg_data *svgdata = (Svg_data *) data;
    int i;

    svgdata->pattern_defined = NULL;
    svgdata->pattern_empty = NULL;
    svgdata->pattern_full = NULL;

    svgdata->side = MIN2(page_width_pp(canvas), page_height_pp(canvas));

    svgdata->pattern_defined = 
        xrealloc(svgdata->pattern_defined, number_of_patterns(canvas)*SIZEOF_INT);
    svgdata->pattern_empty   =
        xrealloc(svgdata->pattern_empty,   number_of_patterns(canvas)*SIZEOF_INT);
    svgdata->pattern_full    =
        xrealloc(svgdata->pattern_full,    number_of_patterns(canvas)*SIZEOF_INT);
    for (i = 0; i < number_of_patterns(canvas); i++) {
        svgdata->pattern_defined[i] = FALSE;
        svgdata->pattern_empty[i]   = FALSE;
        svgdata->pattern_full[i]    = FALSE;
    }

    svgdata->group_is_open = FALSE;
    svgdata->line_width    = 0.0;
    svgdata->pen.color     = 0;
    svgdata->pen.pattern   = 0;
    svgdata->fillrule      = 0;
    svgdata->linecap       = 0;
    svgdata->linejoin      = 0;
    svgdata->linestyle     = 0;
    svgdata->draw          = FALSE;
    svgdata->fill          = FALSE;

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

static void svg_group_props(const Canvas *canvas, Svg_data *svgdata,
    int draw, int fill)
{
    int i, needs_group;
    double lw;
    Pen pen;
    int fillrule, linecap, linejoin, linestyle;
    RGB rgb;
    int red, green, blue;

    /* do we need to redefine a group with new properties ? */
    needs_group = (svgdata->group_is_open == TRUE) ? FALSE : TRUE;
    lw        = svgdata->side*getlinewidth(canvas);
    fillrule  = getfillrule(canvas);
    linecap   = getlinecap(canvas);
    linejoin  = getlinejoin(canvas);
    linestyle = getlinestyle(canvas);
    if (fabs(lw - svgdata->line_width) >= 1.0e-6*(1.0 + fabs(svgdata->line_width))) {
        needs_group = TRUE;
    }
    getpen(canvas, &pen);
    if ((pen.color != svgdata->pen.color) || (pen.pattern != svgdata->pen.pattern)) {
        needs_group = TRUE;
    }
    if (fillrule != svgdata->fillrule) {
        needs_group = TRUE;
    }
    if (linecap != svgdata->linecap) {
        needs_group = TRUE;
    }
    if (linejoin != svgdata->linejoin) {
        needs_group = TRUE;
    }
    if (linestyle != svgdata->linestyle) {
        needs_group = TRUE;
    }
    if ((draw != svgdata->draw) || (fill != svgdata->fill)) {
        needs_group = TRUE;
    }

    if (needs_group == TRUE) {
        /* we need to write the characteristics of the group */

        if (svgdata->group_is_open == TRUE) {
            /* first, we should close the preceding group */
            fprintf(canvas->prstream, "  </g>\n");
            svgdata->group_is_open = FALSE;
        }

        define_pattern(canvas, svgdata, pen.pattern);
        if (get_rgb(canvas, pen.color, &rgb) == RETURN_SUCCESS) {
            red   = rgb.red   >> (GRACE_BPP - 8);
            green = rgb.green >> (GRACE_BPP - 8);
            blue  = rgb.blue  >> (GRACE_BPP - 8);
        } else {
            red   = 0;
            green = 0;
            blue  = 0;
        }

        if (fill && svgdata->pattern_empty[pen.pattern] != TRUE) {
            if (svgdata->pattern_full[pen.pattern] == TRUE) {
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


        svgdata->group_is_open = TRUE;
        svgdata->line_width    = lw;
        svgdata->pen           = pen;
        svgdata->fillrule      = fillrule;
        svgdata->linecap       = linecap;
        svgdata->linejoin      = linejoin;
        svgdata->linestyle     = linestyle;
        svgdata->draw          = draw;
        svgdata->fill          = fill;
    }
}

void svg_drawpixel(const Canvas *canvas, void *data, const VPoint *vp)
{
    Svg_data *svgdata = (Svg_data *) data;
    svg_group_props(canvas, svgdata, FALSE, TRUE);
    fprintf(canvas->prstream,
            "   <rect x=\"%.4f\" y=\"%.4f\" width=\"%.4f\" height=\"%.4f\"/>\n",
            convertX(data, vp->x), convertY(data, vp->y),
            convertW(data, 1.0), convertH(data, 1.0));
}

void svg_drawpolyline(const Canvas *canvas, void *data,
    const VPoint *vps, int n, int mode)
{
    Svg_data *svgdata = (Svg_data *) data;
    int i;

    if (n <= 0) {
        return;
    }

    svg_group_props(canvas, svgdata, TRUE, FALSE);
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

void svg_fillpolygon(const Canvas *canvas, void *data,
    const VPoint *vps, int nc)
{
    Svg_data *svgdata = (Svg_data *) data;
    int i;

    if (nc <= 0) {
        return;
    }

    svg_group_props(canvas, svgdata, FALSE, TRUE);
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

void svg_drawarc(const Canvas *canvas, void *data,
    const VPoint *vp1, const VPoint *vp2, double a1, double a2)
{
    Svg_data *svgdata = (Svg_data *) data;
    VPoint center;
    double rx, ry;

    if (a2 == 0.0) {
        return;
    }
    
    center.x = 0.5*(vp1->x + vp2->x);
    center.y = 0.5*(vp1->y + vp2->y);
    rx       = 0.5*fabs(vp2->x - vp1->x);
    ry       = 0.5*fabs(vp2->y - vp1->y);

    svg_group_props(canvas, svgdata, TRUE, FALSE);

    if (a2 == 360.0) {
        fprintf(canvas->prstream,
            "   <ellipse  rx=\"%.4f\" ry=\"%.4f\" cx=\"%.4f\" cy=\"%.4f\"/>\n",
            convertW(data, rx), convertH(data, ry),
            convertX(data, center.x), convertY(data, center.y));
    } else {
        VPoint start, end;
        
        a2 += a1;

        start.x = center.x + rx*cos((M_PI/180.0)*a1);
        start.y = center.y + ry*sin((M_PI/180.0)*a1);
        end.x   = center.x + rx*cos((M_PI/180.0)*a2);
        end.y   = center.y + ry*sin((M_PI/180.0)*a2);

        fprintf(canvas->prstream,
            "   <path  d=\"M%.4f, %.4fA%.4f, %.4f %d %d %d %.4f, %.4f\"/>\n",
            convertX(data, start.x), convertY(data, start.y),
            convertW(data, rx), convertH(data, ry),
            0,
            (fabs(a2 - a1) > 180) ? 1 : 0,
            (a2 > a1) ? 0 : 1,
            convertX(data, end.x), convertY(data, end.y));
    }
}

void svg_fillarc(const Canvas *canvas, void *data,
    const VPoint *vp1, const VPoint *vp2, double a1, double a2, int mode)
{
    Svg_data *svgdata = (Svg_data *) data;
    VPoint center;
    double rx, ry;

    if (a2 == 0.0) {
        return;
    }

    center.x = 0.5*(vp1->x + vp2->x);
    center.y = 0.5*(vp1->y + vp2->y);
    rx       = 0.5*fabs(vp2->x - vp1->x);
    ry       = 0.5*fabs(vp2->y - vp1->y);

    svg_group_props(canvas, svgdata, FALSE, TRUE);

    if (a2 == 360.0) {
        fprintf(canvas->prstream,
            "   <ellipse  rx=\"%.4f\" ry=\"%.4f\" cx=\"%.4f\" cy=\"%.4f\"/>\n",
            convertW(data, rx), convertH(data, ry),
            convertX(data, center.x), convertY(data, center.y));
    } else {
        VPoint start, end;
        
        a2 += a1;

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
                (fabs(a2 - a1) > 180) ? 1 : 0,
                (a2 > a1) ? 0 : 1,
                convertX(data, end.x), convertY(data, end.y));
        } else {
            fprintf(canvas->prstream,
                "   <path  d=\"M%.4f,%.4fL%.4f,%.4fA%.4f,%.4f %d %d %d %.4f,%.4fz\"/>\n",
                convertX(data, center.x), convertY(data, center.y),
                convertX(data, start.x), convertY(data, start.y),
                convertW(data, rx), convertH(data, ry),
                0,
                (fabs(a2 - a1) > 180) ? 1 : 0,
                (a2 > a1) ? 0 : 1,
                convertX(data, end.x), convertY(data, end.y));
        }
    }
}

void svg_putpixmap(const Canvas *canvas, void *data,
    const VPoint *vp, int width, int height, char *databits,
    int pixmap_bpp, int bitmap_pad, int pixmap_type)
{
    /* not implemented yet */
}

void svg_puttext(const Canvas *canvas, void *data,
    const VPoint *vp, const char *s, int len, int font, const TextMatrix *tm,
    int underline, int overline, int kerning)
{
    char *fontalias, *dash, *family;
    Svg_data *svgdata = (Svg_data *) data;
    double fsize = svgdata->side;

    svg_group_props(canvas, svgdata, FALSE, TRUE);
    
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

void svg_leavegraphics(const Canvas *canvas, void *data,
    const CanvasStats *cstats)
{
    Svg_data *svgdata = (Svg_data *) data;
    if (svgdata->group_is_open == TRUE) {
        fprintf(canvas->prstream, "  </g>\n");
        svgdata->group_is_open = FALSE;
    }
    fprintf(canvas->prstream, "</svg>\n");
}
