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
 * t1fonts.h
 * Type1 fonts for Grace
 */

#ifndef __T1_FONTS_H_
#define __T1_FONTS_H_

#include <config.h>
#include <cmath.h>

#include <t1lib.h>

#include "defines.h"

#define BAD_FONT_ID     -1

/* Font mappings */
#define FONT_MAP_DEFAULT    0
#define FONT_MAP_ACEGR      1

/* TODO */
#define MAGIC_FONT_SCALE	0.028

#define SSCRIPT_SCALE M_SQRT1_2
#define SUBSCRIPT_SHIFT 0.4
#define SUPSCRIPT_SHIFT 0.6
#define ENLARGE_SCALE sqrt(M_SQRT2)
#define OBLIQUE_FACTOR 0.25


#define T1_DEFAULT_ENCODING_FILE  "Default.enc"
#define T1_FALLBACK_ENCODING_FILE "IsoLatin1.enc"
#define T1_DEFAULT_SLANT 0.0
#define T1_AALEVELS 5

#define TEXT_ADVANCING_LR   0
#define TEXT_ADVANCING_RL   1

#define STRING_DIRECTION_LR 0
#define STRING_DIRECTION_RL 1

#define MARK_NONE   -1
#define MAX_MARKS   32

#define UNIT_TM {1.0, 0.0, 0.0, 1.0}

typedef struct {
    double cxx, cxy;
    double cyx, cyy;
} TextMatrix;

typedef struct {
    int direction;
    int advancing;
    int ligatures;
    int setmark;
    int gotomark;
} CSAux;

typedef struct {
    char *s;
    int len;
    int font;
    int color;
    TextMatrix tm;
    double hshift;
    double vshift;
    int underline;
    int overline;
    int kerning;
    CSAux aux;
    GLYPH *glyph;
    VPoint start;
    VPoint stop;
} CompositeString;

typedef struct {
    int mapped_id;
    char alias[32];
    char fallback[32];
} FontDB;

int init_t1(void);
void update_t1(void);

int number_of_fonts(void);
char *get_fontname(int font);
char *get_afmfilename(int font);
char *get_fontalias(int font);
char *get_fontfallback(int font);
char *get_fontfilename(int font);
char *get_encodingscheme(int font);

int get_font_by_name(char *fname);
int get_font_mapped_id(int font);
int get_mapped_font(int mapped_id);
int map_font(int font, int mapped_id);
int map_font_by_name(char *fname, int mapped_id);
void map_fonts(int map);

#endif /* __T1_FONTS_H_ */
