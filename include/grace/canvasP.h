/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1996-2006 Grace Development Team
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

#ifndef __CANVASP_H_
#define __CANVASP_H_

#include <stdio.h>

#include <t1lib.h>

#include <grace/canvas.h>

/* Max value of color channel (255) */
#define MAX_CC_VAL      ((0x01 << CANVAS_BPCC) - 1)

#define COLOR_NONE      0
#define COLOR_AUX       1
#define COLOR_MAIN      2

#if defined(DEBUG_T1LIB)
#  define T1LOGFILE LOGFILE
#else
#  define T1LOGFILE NO_LOGFILE
#endif

#define T1_DEFAULT_BITMAP_PAD  8

#define T1_AALEVELS_LOW   5
#define T1_AALEVELS_HIGH 17

#define fRGB2fSRGB(c) (c <= 0.0031308 ? 12.92*c:1.055*pow(c, 1.0/2.4) - 0.055)

/* Drawing properties */
typedef struct {
    Pen pen;
    int bgcolor;
    int lines;
    double linew;
    int linecap;
    int linejoin;
    double charsize;
    int font;
    int fillrule;
} DrawProps;

typedef struct {
    char *alias;
    char used;
    char chars_used[256];
} FontDB;

typedef struct {
    int active;
    view v;
    view fv;
} BBox_type;

typedef struct _CSGlyphCache {
    VPoint start;
    VPoint stop;
    GLYPH *glyph;
} CSGlyphCache;

/* Canvas */
struct _Canvas {
    /* drawing properties */
    DrawProps draw_props;
    
    /* page background fill */
    int pagefill;
    
    /* colors */
    unsigned int ncolors;
    CMap_entry *cmap;

    /* patterns */
    unsigned int npatterns;
    PMap_entry *pmap;
    
    /* linestyles */
    unsigned int nlinestyles;
    LMap_entry *lmap;
    
    /* fonts */
    unsigned int nfonts;
    FontDB *FontDBtable;
    char **DefEncoding;

    /* clipping */
    int clipflag;
    view clipview;
    
    int draw_mode;
    int drypass;

    BBox_type bboxes[2];

    int max_path_length;

    /* device array */
    unsigned int ndevices;
    Device_entry **device_table;
    
    /* current device */
    Device_entry *curdevice;
    
    /* "device ready" flag */
    int device_ready;
    
    /* output stream */
    void *prstream;
    
    /* info */
    char *username;
    char *docname;
    char *description;
    
    /* user-supplied procedure for parsing composite strings */
    CanvasCSParseProc csparse_proc;
    /* user-supplied procedure for mapping font ids */
    CanvasFMapProc fmap_proc;
    
    /* font size scale */
    double fscale;
    /* line width scale */
    double lscale;
    
    /* user data */
    void *udata;

    /* cached values of the grayscale AA levels and validity flags */
    int aacolors_low_ok;
    unsigned long aacolors_low[T1_AALEVELS_LOW];
    int aacolors_high_ok;
    unsigned long aacolors_high[T1_AALEVELS_HIGH];
};

int clip_line(const Canvas *canvas,
    const VPoint *vp1, const VPoint *vp2, VPoint *vp1c, VPoint *vp2c);
int points_overlap(const Canvas *canvas, const VPoint *vp1, const VPoint *vp2);

void canvas_dev_drawpixel(Canvas *canvas, const VPoint *vp);
void canvas_dev_drawpolyline(Canvas *canvas,
    const VPoint *vps, int n, int mode);
void canvas_dev_fillpolygon(Canvas *canvas, const VPoint *vps, int nc);
void canvas_dev_drawarc(Canvas *canvas,
    const VPoint *vp1, const VPoint *vp2, double a1, double a2);
void canvas_dev_fillarc(Canvas *canvas,
    const VPoint *vp1, const VPoint *vp2, double a1, double a2, int mode);
void canvas_dev_putpixmap(Canvas *canvas, const VPoint *vp, const CPixmap *pm);
void canvas_dev_puttext(Canvas *canvas,
    const VPoint *vp, const char *s, int len, int font, const TextMatrix *tm,
    int underline, int overline, int kerning);

void canvas_stats_reset(Canvas *canvas);

int initgraphics(Canvas *canvas, const CanvasStats *cstats);
void leavegraphics(Canvas *canvas, const CanvasStats *cstats);

int is_valid_bbox(const view *v);

void reset_bboxes(Canvas *canvas);
void update_bboxes(Canvas *canvas, const VPoint *vp);
int update_bboxes_with_view(Canvas *canvas, const view *v);
int update_bboxes_with_vpoints(Canvas *canvas,
    const VPoint *vps, int n, double lw);

int is_valid_color(const RGB *rgb);
int find_color(const Canvas *canvas, const RGB *rgb);
int realloc_color(Canvas *canvas, int n);
void canvas_color_trans(Canvas *canvas, CMap_entry *cmap);
int make_color_scale(Canvas *canvas,
    unsigned int fg, unsigned int bg,
    unsigned int ncolors, unsigned long *colors);
int get_colortype(const Canvas *canvas, unsigned int cindex);
int add_color(Canvas *canvas, const RGB *rgb, int ctype);

int canvas_set_pattern(Canvas *canvas, unsigned int n, const Pattern *pat);
int canvas_set_linestyle(Canvas *canvas, unsigned int n, const LineStyle *ls);

int init_t1(void);
void initialize_patterns(Canvas *canvas);
void initialize_linestyles(Canvas *canvas);

void get_page_viewport(const Canvas *canvas, double *vx, double *vy);

Device_entry *get_curdevice_props(const Canvas *canvas);

#endif /* __CANVASP_H_ */
