/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
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

#ifndef __DRAW_H_
#define __DRAW_H_

#include <stdio.h>

#include <t1lib.h>

#include "defines.h"

/* bpp that Grace uses internally ( = 256 colors) */
#define GRACE_BPP	8
#define MAXCOLORS	(0x01 << GRACE_BPP)

#define MAXPATTERNS 32

#define MAXLINESTYLES 9

#define MAX_LINEWIDTH 10.0        /* max width of drawn lines */

#define MAGIC_LINEW_SCALE 0.0015

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

#define T1_DEFAULT_ENCODING_FILE  "Default.enc"
#define T1_FALLBACK_ENCODING_FILE "IsoLatin1.enc"

#define T1_AALEVELS 5


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

#define TEXT_ADVANCING_LR   0
#define TEXT_ADVANCING_RL   1

#define STRING_DIRECTION_LR 0
#define STRING_DIRECTION_RL 1

#define MARK_NONE   -1
#define MAX_MARKS   32
#define MARK_CR     MAX_MARKS

#define UNIT_TM {1.0, 0.0, 0.0, 1.0}


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

/* Types of axis scale mappings */
typedef enum {
    SCALE_NORMAL,
    SCALE_LOG,
    SCALE_REC,
    SCALE_LOGIT,
    SCALE_BAD
} ScaleType;

#define NUMBER_OF_SCALETYPES  SCALE_BAD

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
    VPoint start;
    VPoint stop;
    GLYPH *glyph;
} CompositeString;

typedef struct {
    int mapped_id;
    char alias[32];
    char fallback[32];
} FontDB;


typedef struct _Canvas Canvas;

/* device pixel routine */
typedef void (*DevDrawPixelProc)(const Canvas *canvas, const VPoint *vp);
/* device polyline routine */
typedef void (*DevDrawPolyLineProc)(const Canvas *canvas,
    const VPoint *vps, int n, int mode);
/* device polygon routine */
typedef void (*DevFillPolygonProc)(const Canvas *canvas,
    const VPoint *vps, int nc);
/* device arc routine */
typedef void (*DevDrawArcProc)(const Canvas *canvas,
    const VPoint *vp1, const VPoint *vp2, int a1, int a2);
/* device fill arc routine */
typedef void (*DevFillArcProc)(const Canvas *canvas,
    const VPoint *vp1, const VPoint *vp2, int a1, int a2, int mode);
/* device pixmap drawing */
typedef void (*DevPutPixmapProc)(const Canvas *canvas,
    const VPoint *vp, int width, int height, char *databits,
    int pixmap_bpp, int bitmap_pad, int pixmap_type);
/* device text typesetting */
typedef void (*DevPutTextProc)(const Canvas *canvas,
    const VPoint *vp, const char *s, int len, int font, const TextMatrix *tm,
    int underline, int overline, int kerning);

/* update color map */
typedef void (*DevUpdateCmapProc)(const Canvas *canvas);

/* device exit */
typedef void (*DevLeaveGraphicsProc)(const Canvas *canvas);

typedef struct {
    RGB rgb;
    char *cname;
    int ctype;
    int tstamp;
} CMap_entry;

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

typedef struct {
    unsigned long width;
    unsigned long height;
    float dpi;
} Page_geometry;

typedef struct {
    int type;
    char *name;		                   /* name of device */
    int (*init)(Canvas *);	           /* function to initialize device */
    int (*parser)(Canvas *, const char *); /* function to parse device-specific commands */
    void (*setup)(Canvas *);               /* function (GUI interface) to setup device */
    char *fext;		                   /* filename extension */
    int devfonts;                          /* device has its own fonts */
    int fontaa;                            /* font antialiasing */
    Page_geometry pg;                      /* device defaults */
    void *data;                            /* device private data */
} Device_entry;


/* Canvas */
struct _Canvas {
    DrawProps draw_props;
    
    /* colors */
    int maxcolors;
    CMap_entry *cmap_table;
    int revflag;

    /* fonts */
    int nfonts;
    FontDB *FontDBtable;
    char **DefEncoding;

    /* clipping */
    int clipflag;
    view clipview;
    
    int draw_mode;

    BBox_type bboxes[2];

    int max_path_length;

    /* devices */
    unsigned int ndevices;
    int curdevice;
    Device_entry **device_table;
    
    /* low-level device routines */
    DevDrawPixelProc     devdrawpixel;
    DevDrawPolyLineProc  devdrawpolyline;
    DevFillPolygonProc   devfillpolygon;
    DevDrawArcProc       devdrawarc;
    DevFillArcProc       devfillarc;
    DevPutPixmapProc     devputpixmap;
    DevPutTextProc       devputtext;
    DevUpdateCmapProc    devupdatecmap;
    DevLeaveGraphicsProc devleavegraphics;
    
    /* output stream */
    FILE *prstream;
    
    /* info */
    char *username;
    char *docname;
};


/* The default max drawing path limit */
#define MAX_DRAWING_PATH  20000

Canvas *canvas_new(void);
void canvas_free(Canvas *canvas);

void canvas_set_username(Canvas *canvas, const char *s);
void canvas_set_docname(Canvas *canvas, const char *s);
char *canvas_get_username(const Canvas *canvas);
char *canvas_get_docname(const Canvas *canvas);

void setclipping(Canvas *canvas, int flag);

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

void leavegraphics(Canvas *canvas);

void DrawPixel(Canvas *canvas, const VPoint *vp);
void DrawPolyline(Canvas *canvas, const VPoint *vps, int n, int mode);
void DrawPolygon(Canvas *canvas, const VPoint *vps, int n);
void DrawArc(Canvas *canvas, const VPoint *vp1, const VPoint *vp2,
    int angle1, int angle2);
void DrawFilledArc(Canvas *canvas, const VPoint *vp1, const VPoint *vp2,
    int angle1, int angle2, int mode);
void WriteString(Canvas *canvas,
    const VPoint *vp, double angle, int just, const char *s);

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

int is_valid_color(const RGB *rgb);
int find_color(const Canvas *canvas, const RGB *rgb);
int get_color_by_name(const Canvas *canvas, const char *cname);
int realloc_color(Canvas *canvas, int n);
int store_color(Canvas *canvas, int n, const CMap_entry *cmap);
int add_color(Canvas *canvas, const CMap_entry *cmap);
int get_rgb(const Canvas *canvas, unsigned int cindex, RGB *rgb);
int  get_frgb(const Canvas *canvas, unsigned int cindex, fRGB *frgb);
CMap_entry *get_cmap_entry(const Canvas *canvas, unsigned int cindex);
char *get_colorname(const Canvas *canvas, unsigned int cindex);
int get_colortype(const Canvas *canvas, unsigned int cindex);

double get_colorintensity(const Canvas *canvas, int cindex);

int get_cmy(const Canvas *canvas, unsigned int cindex, CMY *cmy);
int get_cmyk(const Canvas *canvas, unsigned int cindex, CMYK *cmyk);
int get_fcmyk(const Canvas *canvas, unsigned int cindex, fCMYK *fcmyk);

void initialize_cmap(Canvas *canvas);
void reverse_video(Canvas *canvas);
int is_video_reversed(const Canvas *canvas);

int init_t1(Canvas *canvas);

int map_font(Canvas *canvas, int font, int mapped_id);
int map_font_by_name(Canvas *canvas, const char *fname, int mapped_id);
void map_fonts(Canvas *canvas, int map);

int number_of_fonts(const Canvas *canvas);
char *get_fontname(const Canvas *canvas, int font);
char *get_fontfilename(const Canvas *canvas, int font, int abspath);
char *get_afmfilename(const Canvas *canvas, int font, int abspath);
char *get_fontalias(const Canvas *canvas, int font);
char *get_fontfallback(const Canvas *canvas, int font);
char *get_encodingscheme(const Canvas *canvas, int font);
char **get_default_encoding(const Canvas *canvas);
double get_textline_width(const Canvas *canvas, int font);
double get_underline_pos(const Canvas *canvas, int font);
double get_overline_pos(const Canvas *canvas, int font);
double *get_kerning_vector(const Canvas *canvas,
    const char *str, int len, int font);

int get_font_by_name(const Canvas *canvas, const char *fname);
int get_font_mapped_id(const Canvas *canvas, int font);
int get_mapped_font(const Canvas *canvas, int mapped_id);

int number_of_colors(const Canvas *canvas);
int number_of_patterns(const Canvas *canvas);
int number_of_linestyles(const Canvas *canvas);


char *scale_types(ScaleType it);
ScaleType get_scale_type_by_name(const char *name);

int isvalid_viewport(const view *v);

int is_wpoint_inside(WPoint *wp, world *w);
int is_validWPoint(WPoint wp);

double fscale(double wc, int scale);
double ifscale(double vc, int scale);

int polar2xy(double phi, double rho, double *x, double *y);
void xy2polar(double x, double y, double *phi, double *rho);

double xy_xconv(double wx);
double xy_yconv(double wy);
VPoint Wpoint2Vpoint(WPoint wp);
int world2view(double x, double y, double *xv, double *yv);
void view2world(double xv, double yv, double *xw, double *yw);

int definewindow(Canvas *canvas,
    const world *w, const view *v, int gtype, 
    int xscale, int yscale,
    int invx, int invy);

#endif /* __DRAW_H_ */
