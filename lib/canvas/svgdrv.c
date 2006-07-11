/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1996-2005 Grace Development Team
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
 * Driver for the Scalable Vector Graphics Format from W3C
 */

#include <config.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "grace/baseP.h"
#define CANVAS_BACKEND_API
#include "grace/canvas.h"

static char *svg_charnames[] =
    {
    /* ISO 8859-1 */
    "space", "exclam", "quotedbl", "numbersign", "dollar", "percent", "ampersand", "quoteright", "parenleft", "parenright", "asterisk",
    "plus", "comma", "hyphen", "period", "slash", "zero", "one", "two", "three", "four", "five",
    "six", "seven", "eight", "nine", "colon", "semicolon", "less", "equal", "greater", "question", "at",
    "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K",
    "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V",
    "W", "X", "Y", "Z", "bracketleft", "backslash", "bracketright", "asciicircum", "underscore", "grave", "a",
    "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l",
    "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w",
    "x", "y", "z", "braceleft", "bar", "braceright", "asciitilde", "space", "exclamdown", "cent", "sterling",
    "currency", "yen", "brokenbar", "section", "dieresis", "copyright", "ordfeminine", "guillemotleft", "logicalnot", "hyphen", "registered",
    "macron", "degree", "plusminus", "twosuperior", "threesuperior", "acute", "mu", "paragraph", "periodcentered", "cedilla", "onesuperior",
    "ordmasculine", "guillemotright", "onequarter", "onehalf", "threequarters", "questiondown", "Agrave", "Aacute", "Acircumflex", "Atilde", "Adieresis",
    "Aring", "AE", "Ccedilla", "Egrave", "Eacute", "Ecircumflex", "Edieresis", "Igrave", "Iacute", "Icircumflex", "Idieresis",
    "Eth", "Ntilde", "Ograve", "Oacute", "Ocircumflex", "Otilde", "Odieresis", "multiply", "Oslash", "Ugrave", "Uacute",
    "Ucircumflex", "Udieresis", "Yacute", "Thorn", "germandbls", "agrave", "aacute", "acircumflex", "atilde", "adieresis", "aring",
    "ae", "ccedilla", "egrave", "eacute", "ecircumflex", "edieresis", "igrave", "iacute", "icircumflex", "idieresis", "eth",
    "ntilde", "ograve", "oacute", "ocircumflex", "otilde", "odieresis", "divide", "oslash", "ugrave", "uacute", "ucircumflex",
    "udieresis", "yacute", "thorn", "ydieresis",
    /*additional in WinANSI*/
    "quotesingle", "Euro", "quotesinglbase", "florin", "quotedblbase", "ellipsis", "dagger", "daggerdbl", "circumflex", "perthousand", "Scaron",
    "guilsinglleft", "OE", "Zcaron", "quoteleft", "quoteright", "quotedblleft", "quotedblright", "bullet", "endash", "emdash", "tilde",
    "trademark", "scaron", "guilsinglright", "oe", "zcaron", "Ydieresis",
    /* additional in symbol font, as far as the Adobe SVG-Viewer understands the codes */
    "universal", "existential", "asteriskmath", "minus", "congruent", "Alpha", "Beta", "Chi", "Delta", "Epsilon", "Phi",
    "Gamma", "Eta", "Iota", "theta1", "Kappa", "Lambda", "Mu", "Nu", "Omicron", "Pi", "Theta",
    "Rho", "Sigma", "Tau", "Upsilon", "sigma1", "Omega", "Xi", "Psi", "Zeta", "therefore", "perpendicular",
    "alpha", "beta", "chi", "delta", "epsilon", "phi", "gamma", "eta", "iota", "phi1", "kappa",
    "lambda", "mu", "nu", "omicron", "pi", "theta", "rho", "sigma", "tau", "upsilon", "omega1",
    "omega", "xi", "psi", "zeta", "similar", "Upsilon1", "minute", "lessequal", "fraction", "infinity", "club",
    "diamond", "heart", "spade", "arrowboth", "arrowleft", "arrowup", "arrowright", "arrowdown", "degree", "plusminus", "second",
    "greaterequal", "multiply", "proportional", "partialdiff", "divide", "notequal", "equivalence", "approxequal", "carriagereturn", "aleph", "Ifraktur",
    "Rfraktur", "weierstrass", "circlemultiply", "circleplus", "emptyset", "intersection", "union", "propersuperset", "reflexsuperset", "notsubset", "propersubset",
    "reflexsubset", "element", "notelement", "angle", "gradient", "product", "radical", "dotmath", "logicalnot", "logicaland", "logicalor",
    "arrowdblboth", "arrowdblleft", "arrowdblup", "arrowdblright", "arrowdbldown", "lozenge", "angleleft", "summation", "angleright", "integral", "integraltp",
    "integralbt",
    /*last item in array*/
    "\0"
    };
static char *svg_charcodes[] =
    {
    /* ISO 8859-1 */
    "\x20", "\x21", "&quot;", "\x23", "\x24", "\x25", "&amp;", "&apos;", "\x28", "\x29", "\x2A",
    "\x2B", "\x2C", "\x2D", "\x2E", "\x2F", "\x30", "\x31", "\x32", "\x33", "\x34", "\x35",
    "\x36", "\x37", "\x38", "\x39", "\x3A", "\x3B", "&lt;", "\x3D", "&gt;", "\x3F", "\x40",
    "\x41", "\x42", "\x43", "\x44", "\x45", "\x46", "\x47", "\x48", "\x49", "\x4A", "\x4B",
    "\x4C", "\x4D", "\x4E", "\x4F", "\x50", "\x51", "\x52", "\x53", "\x54", "\x55", "\x56",
    "\x57", "\x58", "\x59", "\x5A", "\x5B", "\x5C", "\x5D", "\x5E", "\x5F", "\x60", "\x61",
    "\x62", "\x63", "\x64", "\x65", "\x66", "\x67", "\x68", "\x69", "\x6A", "\x6B", "\x6C",
    "\x6D", "\x6E", "\x6F", "\x70", "\x71", "\x72", "\x73", "\x74", "\x75", "\x76", "\x77",
    "\x78", "\x79", "\x7A", "\x7B", "\x7C", "\x7D", "\x7E", "\xA0", "\xA1", "\xA2", "\xA3",
    "\xA4", "\xA5", "\xA6", "\xA7", "\xA8", "\xA9", "\xAA", "\xAB", "\xAC", "\xAD", "\xAE",
    "\xAF", "\xB0", "\xB1", "\xB2", "\xB3", "\xB4", "\xB5", "\xB6", "\xB7", "\xB8", "\xB9",
    "\xBA", "\xBB", "\xBC", "\xBD", "\xBE", "\xBF", "\xC0", "\xC1", "\xC2", "\xC3", "\xC4",
    "\xC5", "\xC6", "\xC7", "\xC8", "\xC9", "\xCA", "\xCB", "\xCC", "\xCD", "\xCE", "\xCF",
    "\xD0", "\xD1", "\xD2", "\xD3", "\xD4", "\xD5", "\xD6", "\xD7", "\xD8", "\xD9", "\xDA",
    "\xDB", "\xDC", "\xDD", "\xDE", "\xDF", "\xE0", "\xE1", "\xE2", "\xE3", "\xE4", "\xE5",
    "\xE6", "\xE7", "\xE8", "\xE9", "\xEA", "\xEB", "\xEC", "\xED", "\xEE", "\xEF", "\xF0",
    "\xF1", "\xF2", "\xF3", "\xF4", "\xF5", "\xF6", "\xF7", "\xF8", "\xF9", "\xFA", "\xFB",
    "\xFC", "\xFD", "\xFE", "\xFF",
    /*additional in WinANSI*/
    "&apos;", "&#x20AC;", "&#x201A;", "&#x0192;", "&#x201E;", "&#x2026;", "&#x2020;", "&#x2021;", "&#x02C6;", "&#x2030;", "&#x0160;",
    "&#x2039;", "&#x0152;", "&#x017D;", "&#x2018;", "&#x2019;", "&#x201C;", "&#x201D;", "&#x2022;", "&#x2013;", "&#x2014;", "&#x02DC;",
    "&#x2122;", "&#x0161;", "&#x203A;", "&#x0153;", "&#x017E;", "&#x0178;",
    /* additional in symbol font, as far as the Adobe SVG-Viewer understands the codes */
    "&#8704;", "&#8707;", "&#8727;", "&#8722;", "&#8773;", "&#913;", "&#914;", "&#935;", "&#916;", "&#917;", "&#928;",
    "&#915;", "&#919;", "&#921;", "&#977;", "&#922;", "&#923;", "&#924;", "&#925;", "&#927;", "&#928;", "&#920;",
    "&#929;", "&#931;", "&#932;", "&#933;", "&#963;", "&#937;", "&#926;", "&#936;", "&#918;", "&#8756;", "&#8869;",
    "&#945;", "&#946;", "&#967;", "&#948;", "&#949;", "&#966;", "&#947;", "&#951;", "&#953;", "&#981;", "&#954;",
    "&#955;", "&#956;", "&#957;", "&#959;", "&#960;", "&#952;", "&#961;", "&#963;", "&#964;", "&#965;", "&#982;",
    "&#969;", "&#958;", "&#968;", "&#950;", "&#8764;", "&#978;", "&#8242;", "&#8804;", "&#8260;", "&#8734;", "&#9827;",
    "&#9830;", "&#9829;", "&#9824;", "&#8596;", "&#8592;", "&#8593;", "&#8594;", "&#8595;", "&#176;", "&#177;", "&#8243;",
    "&#8805;", "&#215;", "&#8733;", "&#8706;", "&#247;", "&#8800;", "&#8801;", "&#8776;", "&#8629;", "&#8501;", "&#8465;",
    "&#8476;", "&#8472;", "&#8855;", "&#8853;", "&#8709;", "&#8745;", "&#8746;", "&#8835;", "&#8839;", "&#8836;", "&#8834;",
    "&#8838;", "&#8712;", "&#8713;", "&#8736;", "&#8711;", "&#8719;", "&#8730;", "&#8901;", "&#172;", "&#8743;", "&#8744;",
    "&#8660;", "&#8656;", "&#8657;", "&#8658;", "&#8659;", "&#9674;", "&#9001;", "&#8721;", "&#9002;", "&#8747;", "&#8992;",
    "&#8993;",
    /*last item in array*/
    "\0"
    };
    
typedef struct {
    double side;
    int   *pattern_defined;
    int   *pattern_empty;
    int   *pattern_full;
    int   *colorfilter_defined;
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

static void svg_data_free(void *data)
{   
    Svg_data *svgdata = (Svg_data *) data;
    if (svgdata) {
        xfree(svgdata->pattern_defined);
        xfree(svgdata->pattern_empty);
        xfree(svgdata->pattern_full);
        xfree(svgdata->colorfilter_defined);
        
        xfree(svgdata);
    }
}

/*
 * scale coordinates, using a SVG-viewer to do this gives rounding-problems
 */
static double scaleval (const Svg_data *svgdata, double val)
{
    return val*svgdata->side;
}

static void define_pattern(const Canvas *canvas, Svg_data *svgdata, unsigned int i, unsigned int c)
{
    int j, k, l;
    Pattern *pat = canvas_get_pattern(canvas, i);
    fRGB frgb;
    double bg_red, bg_green, bg_blue;
    FILE *prstream = canvas_get_prstream(canvas);

    if (svgdata->pattern_defined[i] == TRUE && c < number_of_colors(canvas) && svgdata->colorfilter_defined[c] == TRUE) {
        return;
    }

    if (svgdata->pattern_defined[i] != TRUE) {
        /* testing if the pattern is either empty or full */
        svgdata->pattern_empty[i] = TRUE;
        svgdata->pattern_full[i]  = TRUE;
        for (j = 0; j < 32; j++) {
            if (pat->bits[j] != 0x00) {
                svgdata->pattern_empty[i] = FALSE;
            }
            if (pat->bits[j] != 0xff) {
                svgdata->pattern_full[i] = FALSE;
            }
        }
    }

    if (svgdata->pattern_empty[i] != TRUE && svgdata->pattern_full[i] != TRUE) {
        fprintf(prstream, "  <defs>\n");
        /* test if the pattern is already defined. */
        if (svgdata->pattern_defined[i] != TRUE) {
            /* this is an horrible hack ! */
            /* we define pixels as squares in vector graphics */
            /* first fill the whole pattern */
            fprintf(prstream,
                    "   <pattern id=\"pattern%d\" viewBox=\"0 0 16 16\""
                    " width=\"%d\" height=\"%d\" patternUnits=\"userSpaceOnUse\">\n",
                    i, 16, 16);
            fprintf(prstream,"     <rect fill=\"#FFFFFF\" x=\"0\" y=\"0\""
                            " width=\"16\" height=\"16\"/>\n");
            for (j = 0; j < 256; j++) {
                k = j/16;
                l = j%16;
                if ((pat->bits[j/8] >> (j%8)) & 0x01) {
                    /* the bit is set */
                    fprintf(prstream,
                            "     <rect x=\"%d\" y=\"%d\""
                            " width=\"1\" height=\"1\"/>\n",
                            l, 15-k);
                }
            }
            fprintf(prstream, "   </pattern>\n");
            svgdata->pattern_defined[i] = TRUE;
        }
        /* test if the needed colorfilter is already defined. */
        /* color-patterns can be drawn with black patterns and then
           applying a colorfilter to change white to the background-color
           and black to the patterncolor. */
        if (c < number_of_colors(canvas) && svgdata->colorfilter_defined[c] != TRUE) {
            get_frgb(canvas, getbgcolor(canvas), &frgb);
            bg_red=frgb.red;
            bg_green=frgb.green;
            bg_blue=frgb.blue;
            get_frgb(canvas, c, &frgb);
            fprintf(prstream, "   <filter id=\"tocolor%d\" filterUnits=\"objectBoundingBox\"\n", c);
            fprintf(prstream, "    color-interpolation-filters=\"sRGB\" x=\"0%%\" y=\"0%%\" width=\"100%%\" height=\"100%%\">\n");
            fprintf(prstream, "    <feComponentTransfer>\n");
            fprintf(prstream, "      <feFuncR type=\"discrete\" tableValues=\"%.6f %.6f\"/>\n",
                    frgb.red, bg_red);
            fprintf(prstream, "      <feFuncG type=\"discrete\" tableValues=\"%.6f %.6f\"/>\n",
                    frgb.green, bg_green);
            fprintf(prstream, "      <feFuncB type=\"discrete\" tableValues=\"%.6f %.6f\"/>\n",
                    frgb.blue, bg_blue);
            fprintf(prstream, "    </feComponentTransfer>\n");
            fprintf(prstream, "   </filter>\n");
            svgdata->colorfilter_defined[c] = TRUE;
        }
        fprintf(prstream, "  </defs>\n");
    }
}

/*
 * escape special characters and use Unicode for non ISO-8859-1 characters
 */
static char *escape_specials(const Canvas *canvas,
    unsigned char *s, int len, int font)
{
    static char *es = NULL;
    int i, j;

    es = xrealloc(es, (len * 8 + 1)*SIZEOF_CHAR);
    es[0] = '\0';

    for (i = 0; i < len; i++) {
        int found = FALSE;
        char *cname = get_charname(canvas, font, s[i]);
        for (j = 0; svg_charnames[j] != '\0'; j++) {
            if (strings_are_equal(cname, svg_charnames[j])) {
                es = strcat(es, svg_charcodes[j]);
                found = TRUE;
                break;
            }
        }
        if (!found) {
            /* Use private area of Unicode for characters not found in list. */
            char privcode[9];
            sprintf(privcode, "&#%d;", 57344 + (int) s[i]);
            es = strcat(es, privcode); 
        }
    }
   
    return (es);
}

static int svg_initgraphics(const Canvas *canvas, void *data,
    const CanvasStats *cstats)
{
    Svg_data *svgdata = (Svg_data *) data;
    unsigned int i;
    char *s;
    FILE *prstream = canvas_get_prstream(canvas);

    svgdata->pattern_defined = NULL;
    svgdata->pattern_empty = NULL;
    svgdata->pattern_full = NULL;
    svgdata->colorfilter_defined = NULL;

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
    svgdata->colorfilter_defined    =
        xrealloc(svgdata->colorfilter_defined,number_of_colors(canvas)*SIZEOF_INT);
    for (i = 0; i < number_of_colors(canvas); i++) {
        svgdata->colorfilter_defined[i] = FALSE;
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

    fprintf(prstream, "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n");
    fprintf(prstream, "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.0//EN\"");
    fprintf(prstream, " \"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd\">\n");
    fprintf(prstream,
        "<!-- generated by %s -->\n", "Grace/libcanvas");
    fprintf(prstream, "<svg xml:space=\"preserve\" ");
    fprintf(prstream,
        "width=\"%.4fin\" height=\"%.4fin\" viewBox=\"%.4f %.4f %.4f %.4f\">\n",
        page_width_in(canvas), page_height_in(canvas),
        0.0, 0.0, page_width_pp(canvas), page_height_pp(canvas));
    fprintf(prstream, " <g transform=\"translate(0,%.4f) scale(1,-1)\">\n",
            page_height_pp(canvas));

    /* project description */
    s = canvas_get_description(canvas);
    if (!string_is_empty(s)) {
        fprintf(prstream,
            " <desc>%s</desc>\n", s);
    }
    
    return RETURN_SUCCESS;
}

static void svg_group_props(const Canvas *canvas, Svg_data *svgdata,
    int draw, int fill)
{
    unsigned int i, needs_group;
    double lw;
    Pen pen;
    int fillrule, linecap, linejoin, linestyle;
    RGB rgb;
    int red, green, blue;
    FILE *prstream = canvas_get_prstream(canvas);

    /* do we need to redefine a group with new properties ? */
    needs_group = (svgdata->group_is_open == TRUE) ? FALSE : TRUE;
    lw        = MAX2(scaleval(svgdata, getlinewidth(canvas)),
                     1.0/page_dpi(canvas));
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
            fprintf(prstream, "  </g>\n");
            svgdata->group_is_open = FALSE;
        }

        define_pattern(canvas, svgdata, pen.pattern, pen.color);
        if (get_rgb(canvas, pen.color, &rgb) == RETURN_SUCCESS) {
            red   = rgb.red   >> (CANVAS_BPCC - 8);
            green = rgb.green >> (CANVAS_BPCC - 8);
            blue  = rgb.blue  >> (CANVAS_BPCC - 8);
        } else {
            red   = 0;
            green = 0;
            blue  = 0;
        }

        if (fill && svgdata->pattern_empty[pen.pattern] != TRUE) {
            if (svgdata->pattern_full[pen.pattern] == TRUE) {
                fprintf(prstream,
                    "  <g style=\"fill:#%2.2X%2.2X%2.2X", red, green, blue);
            } else {
                fprintf(prstream,
                    "  <g style=\"filter:url(#tocolor%d);", pen.color);
                fprintf(prstream,
                    " fill:url(#pattern%d)", pen.pattern);
            }
            if (getfillrule(canvas) == FILLRULE_WINDING) {
                fprintf(prstream, "; fill-rule:nonzero");
            } else {
                fprintf(prstream, "; fill-rule:evenodd");
            }
        } else {
            fprintf(prstream, "  <g style=\"fill:none");
        }

        if (draw) {

            fprintf(prstream,
                "; stroke:#%2.2X%2.2X%2.2X", red, green, blue);

            fprintf(prstream, "; stroke-width:%8.4f", lw);

            switch (linecap) {
            case LINECAP_BUTT :
                fprintf(prstream, "; stroke-linecap:butt");
                break;
            case LINECAP_ROUND :
                fprintf(prstream, "; stroke-linecap:round");
                break;
            case LINECAP_PROJ :
                fprintf(prstream, "; stroke-linecap:square");
                break;
            default :
                fprintf(prstream, "; stroke-linecap:inherit");
                break;
            }

            switch (linejoin) {
            case LINEJOIN_MITER :
                fprintf(prstream, "; stroke-linejoin:miter");
                break;
            case LINEJOIN_ROUND :
                fprintf(prstream, "; stroke-linejoin:round");
                break;
            case LINEJOIN_BEVEL :
                fprintf(prstream, "; stroke-linejoin:bevel");
                break;
            default :
                fprintf(prstream, "; stroke-linejoin:inherit");
                break;
            }

            if (linestyle <= 1) {
                fprintf(prstream, "; stroke-dasharray:none");
            } else {
                LineStyle *ls = canvas_get_linestyle(canvas, linestyle);
                fprintf(prstream, "; stroke-dasharray:");
                for (i = 0; i < ls->length; i++) {
                    fprintf(prstream,
                        " %d", (int) rint(lw*ls->array[i]));
                }
            }
        }

        fprintf(prstream, "\">\n");


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

static void svg_drawpixel(const Canvas *canvas, void *data, const VPoint *vp)
{
    Svg_data *svgdata = (Svg_data *) data;
    FILE *prstream = canvas_get_prstream(canvas);

    svg_group_props(canvas, svgdata, FALSE, TRUE);
    fprintf(prstream,
            "   <rect x=\"%.4f\" y=\"%.4f\" width=\"%.4f\" height=\"%.4f\"/>\n",
            scaleval(data, vp->x), scaleval(data, vp->y),
            scaleval(data, 1.0), scaleval(data, 1.0));
}

static void svg_drawpolyline(const Canvas *canvas, void *data,
    const VPoint *vps, int n, int mode)
{
    Svg_data *svgdata = (Svg_data *) data;
    int i;
    FILE *prstream = canvas_get_prstream(canvas);

    if (n <= 0) {
        return;
    }

    svg_group_props(canvas, svgdata, TRUE, FALSE);
    fprintf(prstream, "   <path d=\"M%.4f,%.4f",
            scaleval(data, vps[0].x), scaleval(data, vps[0].y));
    for (i = 1; i < n; i++) {
        if (i%10 == 0) {
            fprintf(prstream, "\n            ");
        }
        fprintf(prstream,
            "L%.4f,%.4f", scaleval(data, vps[i].x), scaleval(data, vps[i].y));
    }
    if (mode == POLYLINE_CLOSED) {
        fprintf(prstream, "z\"/>\n");
    } else {
        fprintf(prstream, "\"/>\n");
    }
}

static void svg_fillpolygon(const Canvas *canvas, void *data,
    const VPoint *vps, int nc)
{
    Svg_data *svgdata = (Svg_data *) data;
    int i;
    FILE *prstream = canvas_get_prstream(canvas);

    if (nc <= 0) {
        return;
    }

    svg_group_props(canvas, svgdata, FALSE, TRUE);
    fprintf(prstream, "   <path  d=\"M%.4f,%.4f",
            scaleval(data, vps[0].x), scaleval(data, vps[0].y));
    for (i = 1; i < nc; i++) {
        if (i%10 == 0) {
            fprintf(prstream, "\n             ");
        }
        fprintf(prstream,
            "L%.4f,%.4f", scaleval(data, vps[i].x), scaleval(data, vps[i].y));
    }
    fprintf(prstream, "z\"/>\n");
}

static void svg_drawarc(const Canvas *canvas, void *data,
    const VPoint *vp1, const VPoint *vp2, double a1, double a2)
{
    Svg_data *svgdata = (Svg_data *) data;
    VPoint center;
    double rx, ry;
    FILE *prstream = canvas_get_prstream(canvas);

    if (a2 == 0.0) {
        return;
    }
    
    center.x = 0.5*(vp1->x + vp2->x);
    center.y = 0.5*(vp1->y + vp2->y);
    rx       = 0.5*fabs(vp2->x - vp1->x);
    ry       = 0.5*fabs(vp2->y - vp1->y);

    svg_group_props(canvas, svgdata, TRUE, FALSE);

    if (a2 == 360.0) {
        fprintf(prstream,
            "   <ellipse rx=\"%.4f\" ry=\"%.4f\" cx=\"%.4f\" cy=\"%.4f\"/>\n",
            scaleval(data, rx), scaleval(data, ry),
            scaleval(data, center.x), scaleval(data, center.y));
    } else {
        VPoint start, end;
        
        a2 += a1;

        start.x = center.x + rx*cos((M_PI/180.0)*a1);
        start.y = center.y + ry*sin((M_PI/180.0)*a1);
        end.x   = center.x + rx*cos((M_PI/180.0)*a2);
        end.y   = center.y + ry*sin((M_PI/180.0)*a2);

        fprintf(prstream,
            "   <path d=\"M%.4f, %.4fA%.4f, %.4f %d %d %d %.4f, %.4f\"/>\n",
            scaleval(data, start.x), scaleval(data, start.y),
            scaleval(data, rx), scaleval(data, ry),
            0,
            (fabs(a2 - a1) > 180) ? 1 : 0,
            (a2 > a1) ? 1 : 0,
            scaleval(data, end.x), scaleval(data, end.y));
    }
}

static void svg_fillarc(const Canvas *canvas, void *data,
    const VPoint *vp1, const VPoint *vp2, double a1, double a2, int mode)
{
    Svg_data *svgdata = (Svg_data *) data;
    VPoint center;
    double rx, ry;
    FILE *prstream = canvas_get_prstream(canvas);

    if (a2 == 0.0) {
        return;
    }

    center.x = 0.5*(vp1->x + vp2->x);
    center.y = 0.5*(vp1->y + vp2->y);
    rx       = 0.5*fabs(vp2->x - vp1->x);
    ry       = 0.5*fabs(vp2->y - vp1->y);

    svg_group_props(canvas, svgdata, FALSE, TRUE);

    if (a2 == 360.0) {
        fprintf(prstream,
            "   <ellipse rx=\"%.4f\" ry=\"%.4f\" cx=\"%.4f\" cy=\"%.4f\"/>\n",
            scaleval(data, rx), scaleval(data, ry),
            scaleval(data, center.x), scaleval(data, center.y));
    } else {
        VPoint start, end;
        
        a2 += a1;

        start.x = center.x + rx*cos((M_PI/180.0)*a1);
        start.y = center.y + ry*sin((M_PI/180.0)*a1);
        end.x   = center.x + rx*cos((M_PI/180.0)*a2);
        end.y   = center.y + ry*sin((M_PI/180.0)*a2);

        if (mode == ARCCLOSURE_CHORD) {
            fprintf(prstream,
                "   <path d=\"M%.4f, %.4fA%.4f, %.4f %d %d %d %.4f, %.4fz\"/>\n",
                scaleval(data, start.x), scaleval(data, start.y),
                scaleval(data, rx), scaleval(data, ry),
                0,
                (fabs(a2 - a1) > 180) ? 1 : 0,
                (a2 > a1) ? 1 : 0,
                scaleval(data, end.x), scaleval(data, end.y));
        } else {
            fprintf(prstream,
                "   <path d=\"M%.4f,%.4fL%.4f,%.4fA%.4f,%.4f %d %d %d %.4f,%.4fz\"/>\n",
                scaleval(data, center.x), scaleval(data, center.y),
                scaleval(data, start.x), scaleval(data, start.y),
                scaleval(data, rx), scaleval(data, ry),
                0,
                (fabs(a2 - a1) > 180) ? 1 : 0,
                (a2 > a1) ? 1 : 0,
                scaleval(data, end.x), scaleval(data, end.y));
        }
    }
}

static void svg_putpixmap(const Canvas *canvas, void *data,
    const VPoint *vp, const CPixmap *pm)
{
    /* not implemented yet */
}

static void svg_puttext(const Canvas *canvas, void *data,
    const VPoint *vp, const char *s, int len, int font, const TextMatrix *tm,
    int underline, int overline, int kerning)
{
    char *fontalias, *fontfullname, *fontweight;
    char *dash, *family, *familyff;
    Svg_data *svgdata = (Svg_data *) data;
    double fsize = svgdata->side;
    FILE *prstream = canvas_get_prstream(canvas);

    svg_group_props(canvas, svgdata, FALSE, TRUE);
    
    fprintf(prstream, "   <text  ");

    family = NULL;
    fontalias = get_fontalias(canvas, font);
    fontfullname = get_fontfullname(canvas, font);
    if ((dash = strchr(fontalias, '-')) == NULL) {
        family = copy_string(family, fontalias);
    } else {
        family = xrealloc(family, dash - fontalias + 1);
        strncpy(family, fontalias, dash - fontalias);
        family[dash - fontalias] = '\0';
    }
    fprintf(prstream, " style=\"font-family:%s", family);
    
    familyff=get_fontfamilyname(canvas, font);
    if (strcmp(family,familyff) != 0){
        fprintf(prstream, ",'%s'",familyff);
    }
    
    copy_string(family, NULL);

    if (get_italic_angle(canvas, font) != 0) {
        if ((strstr(fontfullname, "Obliqued") != NULL) ||
            (strstr(fontfullname, "Oblique") != NULL) ||
            (strstr(fontfullname, "Upright") != NULL) ||
            (strstr(fontfullname, "Kursiv") != NULL) ||
            (strstr(fontfullname, "Cursive") != NULL) ||
            (strstr(fontfullname, "Slanted") != NULL) ||
            (strstr(fontfullname, "Inclined") != NULL)) {
            fprintf(prstream, "; font-style:oblique");
        } else {
            fprintf(prstream, "; font-style:italic");
        }
    } else {
        fprintf(prstream, "; font-style:normal");
    }

    fontweight=get_fontweight(canvas, font);
    if ((strstr(fontweight, "UltraLight") != NULL) ||
        (strstr(fontweight, "ExtraLight") != NULL)) {
        fprintf(prstream, "; font-weight:100");
    } else if ((strstr(fontweight, "SemiLight") != NULL) ||
               (strstr(fontweight, "Thin") != NULL)) {
        fprintf(prstream, "; font-weight:200");
    } else if (strstr(fontweight, "Light") != NULL) {
        fprintf(prstream, "; font-weight:300");
    } else if (strstr(fontweight, "Book") != NULL) {
        fprintf(prstream, "; font-weight:500");
    } else if (strstr(fontweight, "miBold") != NULL) {
        fprintf(prstream, "; font-weight:600");
    } else if ((strstr(fontweight, "ExtraBold") != NULL) ||
               (strstr(fontweight, "Heavy") != NULL) ||
               (strstr(fontweight, "UltraBold") != NULL)) {
        fprintf(prstream, "; font-weight:800");
    } else if (strstr(fontweight, "Bold") != NULL) {
        fprintf(prstream, "; font-weight:bold");
    } else if ((strstr(fontweight, "ExtraBlack") != NULL) ||
               (strstr(fontweight, "Ultra") != NULL)) {
        fprintf(prstream, "; font-weight:900");
    } else if (strstr(fontweight, "Black") != NULL) {
        fprintf(prstream, "; font-weight:800");
    } else {
        fprintf(prstream, "; font-weight:normal");
    }

    if ((strstr(fontfullname, "UltraCompressed") != NULL) ||
        (strstr(fontfullname, "UltraCondensed") != NULL)) {
        fprintf(prstream, "; font-stretch:ultra-condensed");
    } else if ((strstr(fontfullname, "ExtraCompressed") != NULL) ||
               (strstr(fontfullname, "ExtraCondensed") != NULL)) {
        fprintf(prstream, "; font-stretch:extra-condensed");
    } else if ((strstr(fontfullname, "SemiCondensed") != NULL) ||
               (strstr(fontfullname, "Narrow") != NULL)) {
        fprintf(prstream, "; font-stretch:semi-condensed");
    } else if (strstr(fontfullname, "Condensed") != NULL) {
        fprintf(prstream, "; font-stretch:condensed");
    } else if ((strstr(fontfullname, "Wide") != NULL) ||
               (strstr(fontfullname, "Poster") != NULL) ||
               (strstr(fontfullname, "SemiExpanded") != NULL)) {
        fprintf(prstream, "; font-stretch:semi-expanded");
    } else if ((strstr(fontfullname, "ExtraExpanded") != NULL) ||
               (strstr(fontfullname, "ExtraExtended") != NULL)) {
        fprintf(prstream, "; font-stretch:extra-expanded");
    } else if ((strstr(fontfullname, "UltraExpanded") != NULL) ||
               (strstr(fontfullname, "UltraExtended") != NULL)) {
        fprintf(prstream, "; font-stretch:ultra-expanded");
    } else if ((strstr(fontfullname, "Expanded") != NULL) ||
               (strstr(fontfullname, "Extended") != NULL)) {
        fprintf(prstream, "; font-stretch:expanded");
    }

    fprintf(prstream, "; font-size:%.4f", fsize);

    if (underline == TRUE) {
        if (overline == TRUE) {
            fprintf(prstream, "; text-decoration:underline|overline");
        } else {
            fprintf(prstream, "; text-decoration:underline");
        }
    } else {
        if (overline == TRUE) {
            fprintf(prstream, "; text-decoration:overline");
        }
    }

    fprintf(prstream, "\" transform=\"matrix(%.4f,%.4f,%.4f,%.4f,%.4f,%.4f)\">",
            tm->cxx, tm->cyx,
            -tm->cxy,-tm->cyy,
            scaleval(data, vp->x), scaleval(data, vp->y));

    fprintf(prstream, "%s", escape_specials(canvas,
        (unsigned char *) s, len, font));

    fprintf(prstream, "</text>\n");
}

static void svg_leavegraphics(const Canvas *canvas, void *data,
    const CanvasStats *cstats)
{
    Svg_data *svgdata = (Svg_data *) data;
    FILE *prstream = canvas_get_prstream(canvas);

    if (svgdata->group_is_open == TRUE) {
        fprintf(prstream, "  </g>\n");
        svgdata->group_is_open = FALSE;
    }
    fprintf(prstream, " </g>\n");
    fprintf(prstream, "</svg>\n");
}

int register_svg_drv(Canvas *canvas)
{
    Device_entry *d;
    Svg_data *data;
    
    data = init_svg_data(canvas);
    if (!data) {
        return -1;
    }
    
    d = device_new("SVG", DEVICE_FILE, TRUE, (void *) data, svg_data_free);
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
        svg_drawpixel,
        svg_drawpolyline,
        svg_fillpolygon,
        svg_drawarc,
        svg_fillarc,
        svg_putpixmap,
        svg_puttext);
    
    return register_device(canvas, d);
}
