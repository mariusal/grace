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

#ifndef __CANVAS_H_
#define __CANVAS_H_

#include <stdio.h>

#include <t1lib.h>

#include "grace/base.h"

/* bpp that Grace uses internally ( = 256 colors) */
#define GRACE_BPP	8
#define MAXCOLORS	(0x01 << GRACE_BPP)

#define MAX_LINEWIDTH 20.0        /* max width of drawn lines */

/* polyline drawing modes */
#define POLYLINE_OPEN	    0
#define POLYLINE_CLOSED	    1

/* polygon fill type */
#define FILLRULE_WINDING    0
#define FILLRULE_EVENODD    1

/* arc fill modes */
#define ARCFILL_CHORD       0
#define ARCFILL_PIESLICE    1

/* pixmap transparency types */
#define PIXMAP_TRANSPARENT  0
#define PIXMAP_OPAQUE	    1

/* line cap parameter */
#define LINECAP_BUTT        0
#define LINECAP_ROUND       1
#define LINECAP_PROJ        2

/* line join type */
#define LINEJOIN_MITER      0
#define LINEJOIN_ROUND      1
#define LINEJOIN_BEVEL      2

/* Text string justifications */
/* Horizontal */
#define JUST_LEFT       0
#define JUST_RIGHT      1
#define JUST_CENTER     2

/* Vertical */
#define JUST_BLINE      0
#define JUST_BOTTOM     4
#define JUST_TOP        8
#define JUST_MIDDLE    12

#if defined(DEBUG_T1LIB)
#  define T1LOGFILE LOGFILE
#else
#  define T1LOGFILE NO_LOGFILE
#endif

#define T1_DEFAULT_BITMAP_PAD  8

#define T1_AALEVELS_LOW   5
#define T1_AALEVELS_HIGH 17


#define BAD_FONT_ID     -1

/* TODO */
#define MAGIC_FONT_SCALE    0.028
#define MAGIC_LINEW_SCALE   0.0015


#define TEXT_ADVANCING_LR   0
#define TEXT_ADVANCING_RL   1

#define STRING_DIRECTION_LR 0
#define STRING_DIRECTION_RL 1

#define MARK_NONE   -1
#define MAX_MARKS   32
#define MARK_CR     MAX_MARKS

#define UNIT_TM {1.0, 0.0, 0.0, 1.0}

#define MM_PER_INCH	25.4
#define CM_PER_INCH	(MM_PER_INCH/10)

/* hardcopy or terminal device */
/* device output can be redirected to file/printer(both) */
#define DEVICE_TERM	0
#define DEVICE_FILE	1
#define DEVICE_PRINT	2

/* Page orientation */
#define PAGE_ORIENT_LANDSCAPE  0
#define PAGE_ORIENT_PORTRAIT   1

#define INTENSITY(r, g, b) ((299*r + 587*g + 114*b)/1000)

#define fRGB2fSRGB(c) (c <= 0.0031308 ? 12.92*c:1.055*pow(c, 1.0/2.4) - 0.055)

#define COLOR_TRANS_NONE        0
#define COLOR_TRANS_GREYSCALE   1
#define COLOR_TRANS_BW          2
#define COLOR_TRANS_NEGATIVE    3
#define COLOR_TRANS_REVERSE     4
#define COLOR_TRANS_SRGB        5

/* A point in viewport coordinates */
typedef struct {
    double x;
    double y;
} VPoint;

typedef struct {
    double x;
    double y;
} VVector;

/* Viewport */
typedef struct {
    double xv1, xv2, yv1, yv2;
} view;

typedef struct {
    int color;
    int pattern;
    /* int transparency; */
} Pen;

typedef struct {
    Pen pen;
    int style;
    double width;
} Line;

typedef struct {
    int red;
    int green;
    int blue;
} RGB;

typedef struct {
    double red;
    double green;
    double blue;
} fRGB;

typedef struct {
    double y;
    double i;
    double q;
} YIQ;

typedef struct {
    int cyan;
    int magenta;
    int yellow;
} CMY;

typedef struct {
    double cyan;
    double magenta;
    double yellow;
} fCMY;

typedef struct {
    int cyan;
    int magenta;
    int yellow;
    int black;
} CMYK;

typedef struct {
    double cyan;
    double magenta;
    double yellow;
    double black;
} fCMYK;

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
    double cxx, cxy;
    double cyx, cyy;
} TextMatrix;

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
    int setmark;
    int gotomark;
    int direction;
    int advancing;
    int ligatures;
    int kerning;
} CStringSegment;

typedef struct {
    VPoint start;
    VPoint stop;
    GLYPH *glyph;
} CSGlyphCache;

typedef struct {
    unsigned int nsegs;
    CStringSegment *segs;
    CSGlyphCache *cglyphs;
} CompositeString;

typedef struct {
    char *alias;
    char used;
    char chars_used[256];
} FontDB;

typedef struct {
    int font;
    char chars_used[256];
} FontStats;

typedef struct {
    unsigned int ncolors;
    unsigned int *colors;
    int npatterns;
    int *patterns;
    int nlinestyles;
    int *linestyles;
    int nfonts;
    FontStats *fonts;
    view bbox;
} CanvasStats;

typedef struct _Canvas Canvas;

/* function to initialize device */
typedef int (*DevInitProc)(const Canvas *canvas, void *data,
    const CanvasStats *cstats);
/* function to parse device-specific commands */
typedef int (*DevParserProc)(const Canvas *canvas, void *data, const char *s);
/* function (GUI interface) to setup device */
typedef void (*DevSetupProc)(const Canvas *canvas, void *data);
/* update color map */
typedef void (*DevUpdateCmapProc)(const Canvas *canvas, void *data);
/* device exit */
typedef void (*DevLeaveGraphicsProc)(const Canvas *canvas, void *data,
    const CanvasStats *cstats);


/* device pixel routine */
typedef void (*DevDrawPixelProc)(const Canvas *canvas, void *data,
    const VPoint *vp);
/* device polyline routine */
typedef void (*DevDrawPolyLineProc)(const Canvas *canvas, void *data,
    const VPoint *vps, int n, int mode);
/* device polygon routine */
typedef void (*DevFillPolygonProc)(const Canvas *canvas, void *data,
    const VPoint *vps, int nc);
/* device arc routine */
typedef void (*DevDrawArcProc)(const Canvas *canvas, void *data,
    const VPoint *vp1, const VPoint *vp2, double a1, double a2);
/* device fill arc routine */
typedef void (*DevFillArcProc)(const Canvas *canvas, void *data,
    const VPoint *vp1, const VPoint *vp2, double a1, double a2, int mode);
/* device pixmap drawing */
typedef void (*DevPutPixmapProc)(const Canvas *canvas, void *data,
    const VPoint *vp, int width, int height, char *databits,
    int pixmap_bpp, int bitmap_pad, int pixmap_type);
/* device text typesetting */
typedef void (*DevPutTextProc)(const Canvas *canvas, void *data,
    const VPoint *vp, const char *s, int len, int font, const TextMatrix *tm,
    int underline, int overline, int kerning);

/* drawing procedure */
typedef void (*CanvasDrawProc)(Canvas *canvas, void *data);

typedef int (*CanvasCSParseProc)(const Canvas *canvas,
    const char *s, CompositeString *cstring);

typedef int (*CanvasFMapProc)(const Canvas *canvas, int);

typedef struct {
    RGB rgb;
    char *cname;
    int ctype;
} Color;

typedef struct {
    Color color;
    char used;
    RGB devrgb;     /* Converted RGB - for the current device */
} CMap_entry;

typedef struct {
    unsigned int width;
    unsigned int height;
    unsigned char *bits;
} Pattern;

typedef struct {
    Pattern pattern;
    char used;
} PMap_entry;

typedef struct {
    unsigned int length;
    unsigned int *array;
} LineStyle;

typedef struct {
    LineStyle linestyle;
    char used;
} LMap_entry;

#define BAD_COLOR	-1

#define COLOR_NONE      0
#define COLOR_AUX       1
#define COLOR_MAIN      2

#define BBOX_TYPE_GLOB	0
#define BBOX_TYPE_TEMP	1

typedef struct {
    int active;
    view v;
    view fv;
} BBox_type;

/* Standard formats */
typedef enum {
    PAGE_FORMAT_CUSTOM,
    PAGE_FORMAT_USLETTER,
    PAGE_FORMAT_A4
} PageFormat;

/* Font rasterizing types */
typedef enum {
    FONT_RASTER_DEVICE,
    FONT_RASTER_MONO,
    FONT_RASTER_AA_LOW,
    FONT_RASTER_AA_HIGH,
    FONT_RASTER_AA_SMART
} FontRaster;

typedef struct {
    unsigned long width;
    unsigned long height;
    float dpi;
} Page_geometry;

typedef struct {
    int type;
    char *name;		                   /* name of device */
    char *fext;		                   /* filename extension */
    FontRaster fontrast;                   /* font rasterizing */
    Page_geometry pg;                      /* device defaults */
    
    int twopass;                           /* two-pass mode */
    int autocrop;                          /* resize canvas to tight BBox */

    int color_trans;                       /* color transformation type */
    
    /* low-level device routines */
    DevInitProc          initgraphics;
    DevLeaveGraphicsProc leavegraphics;
    DevParserProc        parser;
    DevSetupProc         setup;
    DevUpdateCmapProc    updatecmap;
    DevDrawPixelProc     drawpixel;
    DevDrawPolyLineProc  drawpolyline;
    DevFillPolygonProc   fillpolygon;
    DevDrawArcProc       drawarc;
    DevFillArcProc       fillarc;
    DevPutPixmapProc     putpixmap;
    DevPutTextProc       puttext;
    
    void *data;                            /* device private data */
} Device_entry;

typedef struct {
    unsigned int width;
    unsigned int height;
    unsigned int **matrix;
} Xrst_pixmap;

typedef int (*XrstDumpProc)(const Canvas *canvas, void *data,
    unsigned int ncolors, unsigned int *colors, Xrst_pixmap *pm);

typedef struct _XrstDevice_entry {
    int           type;
    char          *name;
    char          *fext;
    int           fontaa;
    DevParserProc parser;
    DevSetupProc  setup;
    
    XrstDumpProc  dump;
    
    void          *data; /* device private data */
} XrstDevice_entry;


/* Canvas */
struct _Canvas {
    /* drawing properties */
    DrawProps draw_props;
    
    /* page background fill */
    Pen pagepen;
    
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
    FILE *prstream;
    
    /* info */
    char *username;
    char *docname;
    
    /* user-supplied procsedure for parsing composite strings */
    CanvasCSParseProc csparse_proc;
    /* user-supplied procsedure for mapping font ids */
    CanvasFMapProc fmap_proc;
    
    /* user data */
    void *udata;

    /* cached values of the grayscale AA levels and validity flags */
    int aacolors_low_ok;
    unsigned long aacolors_low[T1_AALEVELS_LOW];
    int aacolors_high_ok;
    unsigned long aacolors_high[T1_AALEVELS_HIGH];
};

/* The default max drawing path limit */
#define MAX_DRAWING_PATH  20000

Canvas *canvas_new(void);
void canvas_free(Canvas *canvas);

void canvas_set_udata(Canvas *canvas, void *data);
void *canvas_get_udata(const Canvas *canvas);

void canvas_set_username(Canvas *canvas, const char *s);
void canvas_set_docname(Canvas *canvas, const char *s);
char *canvas_get_username(const Canvas *canvas);
char *canvas_get_docname(const Canvas *canvas);

void canvas_set_fmap_proc(Canvas *canvas, CanvasFMapProc fmap_proc);
void canvas_set_csparse_proc(Canvas *canvas, CanvasCSParseProc csparse_proc);

void canvas_set_pagepen(Canvas *canvas, const Pen *pen);

void setclipping(Canvas *canvas, int flag);
int canvas_set_clipview(Canvas *canvas, const view *v);

void setbgcolor(Canvas *canvas, int bgcolor);
void setpen(Canvas *canvas, const Pen *pen);
void setline(Canvas *canvas, const Line *line);
void setcolor(Canvas *canvas, int color);
void setlinestyle(Canvas *canvas, int lines);
void setlinewidth(Canvas *canvas, double linew);
void setpattern(Canvas *canvas, int pattern);
void setcharsize(Canvas *canvas, double charsize);
void setfont(Canvas *canvas, int font);
void setfillrule(Canvas *canvas, int rule);
void setlinecap(Canvas *canvas, int type);
void setlinejoin(Canvas *canvas, int type);

void getpen(const Canvas *canvas, Pen *pen);
int getcolor(const Canvas *canvas);
int getbgcolor(const Canvas *canvas);
int getpattern(const Canvas *canvas);
int getlinestyle(const Canvas *canvas);
int getlinecap(const Canvas *canvas);
int getlinejoin(const Canvas *canvas);
int getfillrule(const Canvas *canvas);
double getlinewidth(const Canvas *canvas);
double getcharsize(const Canvas *canvas);
int getfont(const Canvas *canvas);

int initgraphics(Canvas *canvas, const CanvasStats *cstats);
void leavegraphics(Canvas *canvas, const CanvasStats *cstats);

void DrawPixel(Canvas *canvas, const VPoint *vp);
void DrawPolyline(Canvas *canvas, const VPoint *vps, int n, int mode);
void DrawPolygon(Canvas *canvas, const VPoint *vps, int n);
void DrawArc(Canvas *canvas, const VPoint *vp1, const VPoint *vp2,
    double angle1, double angle2);
void DrawFilledArc(Canvas *canvas, const VPoint *vp1, const VPoint *vp2,
    double angle1, double angle2, int mode);
void WriteString(Canvas *canvas,
    const VPoint *vp, double angle, int just, const char *s);

CStringSegment *cstring_seg_new(CompositeString *cstring);
double tm_size(const TextMatrix *tm);
int tm_scale(TextMatrix *tm, double s);
int tm_rotate(TextMatrix *tm, double angle);
int tm_slant(TextMatrix *tm, double slant);
int tm_product(TextMatrix *tm, const TextMatrix *p);

void DrawRect(Canvas *canvas, const VPoint *vp1, const VPoint *vp2);
void FillRect(Canvas *canvas, const VPoint *vp1, const VPoint *vp2);
void DrawLine(Canvas *canvas, const VPoint *vp1, const VPoint *vp2);
void DrawEllipse(Canvas *canvas, const VPoint *vp1, const VPoint *vp2);
void DrawFilledEllipse(Canvas *canvas, const VPoint *vp1, const VPoint *vp2);
void DrawCircle(Canvas *canvas, const VPoint *vp, double radius);
void DrawFilledCircle(Canvas *canvas, const VPoint *vp, double radius);


int is_validVPoint(const Canvas *canvas, const VPoint *vp);

void reset_bbox(Canvas *canvas, int type);
void reset_bboxes(Canvas *canvas);
void freeze_bbox(Canvas *canvas, int type);
int get_bbox(const Canvas *canvas, int type, view *v);
void update_bbox(Canvas *canvas, int type, const VPoint *vp);
void update_bboxes(Canvas *canvas, const VPoint *vp);
int melt_bbox(Canvas *canvas, int type);
void activate_bbox(Canvas *canvas, int type, int status);
int update_bboxes_with_view(Canvas *canvas, const view *v);
int update_bboxes_with_vpoints(Canvas *canvas,
    const VPoint *vps, int n, double lw);

void set_draw_mode(Canvas *canvas, int mode);
int get_draw_mode(const Canvas *canvas);

void set_max_path_limit(Canvas *canvas, int limit);
int get_max_path_limit(const Canvas *canvas);

int clip_line(const Canvas *canvas,
    const VPoint *vp1, const VPoint *vp2, VPoint *vp1c, VPoint *vp2c);

int points_overlap(const Canvas *canvas, const VPoint *vp1, const VPoint *vp2);

int view_extend(view *v, double w);
int is_valid_bbox(const view *v);
int merge_bboxes(const view *v1, const view *v2, view *v);
void vpswap(VPoint *vp1, VPoint *vp2);
int VPoints2bbox(const VPoint *vp1, const VPoint *vp2, view *bb);

int is_vpoint_inside(const view *v, const VPoint *vp, double epsilon);

unsigned int number_of_colors(const Canvas *canvas);
int is_valid_color(const RGB *rgb);
int find_color(const Canvas *canvas, const RGB *rgb);
int get_color_by_name(const Canvas *canvas, const char *cname);
int realloc_color(Canvas *canvas, int n);
int store_color(Canvas *canvas, int n, const Color *color);
int add_color(Canvas *canvas, const Color *color);
int get_rgb(const Canvas *canvas, unsigned int cindex, RGB *rgb);
int  get_frgb(const Canvas *canvas, unsigned int cindex, fRGB *frgb);
Color *get_color_def(const Canvas *canvas, unsigned int cindex);
char *get_colorname(const Canvas *canvas, unsigned int cindex);
int get_colortype(const Canvas *canvas, unsigned int cindex);
void canvas_color_trans(Canvas *canvas, CMap_entry *cmap);

double get_colorintensity(const Canvas *canvas, int cindex);

int get_cmy(const Canvas *canvas, unsigned int cindex, CMY *cmy);
int get_cmyk(const Canvas *canvas, unsigned int cindex, CMYK *cmyk);
int get_fcmyk(const Canvas *canvas, unsigned int cindex, fCMYK *fcmyk);

void initialize_cmap(Canvas *canvas);

int make_color_scale(Canvas *canvas,
    unsigned int fg, unsigned int bg,
    unsigned int ncolors, unsigned long *colors);

int init_t1(Canvas *canvas);

int canvas_set_encoding(Canvas *canvas, char *encfile);
int canvas_add_font(Canvas *canvas, char *ffile, const char *alias);

unsigned int number_of_fonts(const Canvas *canvas);
char *get_fontname(const Canvas *canvas, int font);
char *get_fontfullname(const Canvas *canvas, int font);
char *get_fontfamilyname(const Canvas *canvas, int font);
char *get_fontweight(const Canvas *canvas, int font);
char *get_fontfilename(const Canvas *canvas, int font, int abspath);
char *get_afmfilename(const Canvas *canvas, int font, int abspath);
char *get_fontalias(const Canvas *canvas, int font);
char *get_encodingscheme(const Canvas *canvas, int font);
char **get_default_encoding(const Canvas *canvas);
double get_textline_width(const Canvas *canvas, int font);
double get_underline_pos(const Canvas *canvas, int font);
double get_overline_pos(const Canvas *canvas, int font);
double get_italic_angle(const Canvas *canvas, int font);
double *get_kerning_vector(const Canvas *canvas,
    const char *str, int len, int font);

int canvas_get_font_by_name(const Canvas *canvas, const char *fname);

char *font_subset(const Canvas *canvas,
    int font, char *mask, unsigned long *datalen);

unsigned int number_of_patterns(const Canvas *canvas);
int canvas_set_pattern(Canvas *canvas, unsigned int n, const Pattern *pat);
Pattern *canvas_get_pattern(const Canvas *canvas, unsigned int n);
void initialize_patterns(Canvas *canvas);

unsigned int number_of_linestyles(const Canvas *canvas);
int canvas_set_linestyle(Canvas *canvas, unsigned int n, const LineStyle *ls);
LineStyle *canvas_get_linestyle(const Canvas *canvas, unsigned int n);
void initialize_linestyles(Canvas *canvas);

Device_entry *device_new(const char *name, int type, int twopass, void *data);
int device_set_procs(Device_entry *d,
    DevInitProc          initgraphics,
    DevLeaveGraphicsProc leavegraphics,
    DevParserProc        parser,
    DevSetupProc         setup,
    DevUpdateCmapProc    updatecmap,
    DevDrawPixelProc     drawpixel,
    DevDrawPolyLineProc  drawpolyline,
    DevFillPolygonProc   fillpolygon,
    DevDrawArcProc       drawarc,
    DevFillArcProc       fillarc,
    DevPutPixmapProc     putpixmap,
    DevPutTextProc       puttext);
int device_set_dpi(Device_entry *d, float dpi, int resize);
int device_set_fext(Device_entry *d, const char *fext);
int device_set_autocrop(Device_entry *d, int autocrop);
int device_set_fontrast(Device_entry *d, FontRaster fontrast);

int register_device(Canvas *canvas, Device_entry *d);

int register_xrst_device(Canvas *canvas, const XrstDevice_entry *xdev);

int select_device(Canvas *canvas, int dindex);

Device_entry *get_device_props(const Canvas *canvas, int device);
Device_entry *get_curdevice_props(const Canvas *canvas);

char *get_device_name(const Canvas *canvas, int device);

int is_valid_page_geometry(const Page_geometry *pg);
int set_page_geometry(Canvas *canvas, const Page_geometry *pg);
Page_geometry *get_page_geometry(const Canvas *canvas);

int get_device_page_dimensions(const Canvas *canvas,
    int dindex, int *wpp, int *hpp);

int get_device_by_name(const Canvas *canvas, const char *dname);

int parse_device_options(Canvas *canvas, int dindex, char *options);

int number_of_devices(const Canvas *canvas);

void get_page_viewport(const Canvas *canvas, double *vx, double *vy);

int terminal_device(const Canvas *canvas);

PageFormat get_page_format(const Canvas *canvas, int device);

/* some useful macros */
#define page_dpi(canvas)       ((get_page_geometry(canvas))->dpi)

#define page_width(canvas)     ((get_page_geometry(canvas))->width)
#define page_height(canvas)    ((get_page_geometry(canvas))->height)

#define page_width_in(canvas)  ((double) page_width(canvas)/page_dpi(canvas))
#define page_height_in(canvas) ((double) page_height(canvas)/page_dpi(canvas))

#define page_width_mm(canvas)  (MM_PER_INCH*page_width_in(canvas))
#define page_height_mm(canvas) (MM_PER_INCH*page_height_in(canvas))

#define page_width_cm(canvas)  (CM_PER_INCH*page_width_in(canvas))
#define page_height_cm(canvas) (CM_PER_INCH*page_height_in(canvas))

#define page_width_pp(canvas)  (72*page_width_in(canvas))
#define page_height_pp(canvas) (72*page_height_in(canvas))

void canvas_dev_drawpixel(Canvas *canvas, const VPoint *vp);
void canvas_dev_drawpolyline(Canvas *canvas,
    const VPoint *vps, int n, int mode);
void canvas_dev_fillpolygon(Canvas *canvas, const VPoint *vps, int nc);
void canvas_dev_drawarc(Canvas *canvas,
    const VPoint *vp1, const VPoint *vp2, double a1, double a2);
void canvas_dev_fillarc(Canvas *canvas,
    const VPoint *vp1, const VPoint *vp2, double a1, double a2, int mode);
void canvas_dev_putpixmap(Canvas *canvas,
    const VPoint *vp, int width, int height, char *databits,
    int pixmap_bpp, int bitmap_pad, int pixmap_type);
void canvas_dev_puttext(Canvas *canvas,
    const VPoint *vp, const char *s, int len, int font, const TextMatrix *tm,
    int underline, int overline, int kerning);

void canvas_stats_reset(Canvas *canvas);

int canvas_draw(Canvas *canvas, CanvasDrawProc dproc, void *data);

int get_string_bbox(Canvas *canvas,
    const VPoint *vp, double angle, int just, const char *s, view *bbox);

int isvalid_viewport(const view *v);

#endif /* __CANVAS_H_ */
