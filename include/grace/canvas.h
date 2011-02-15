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

#ifndef __CANVAS_H_
#define __CANVAS_H_

#include "grace/base.h"

/* bits per color channel (i.e., 256^3 colors) */
#define CANVAS_BPCC     8

/* polyline drawing modes */
#define POLYLINE_OPEN       0
#define POLYLINE_CLOSED     1

/* polygon fill type */
#define FILLRULE_WINDING    0
#define FILLRULE_EVENODD    1

/* arc fill modes */
#define ARCCLOSURE_CHORD    0
#define ARCCLOSURE_PIESLICE 1

/* pixmap transparency types */
#define PIXMAP_TRANSPARENT  0
#define PIXMAP_OPAQUE       1

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

#define BAD_FONT_ID     -1


#define TEXT_ADVANCING_LR   0
#define TEXT_ADVANCING_RL   1

#define STRING_DIRECTION_LR 0
#define STRING_DIRECTION_RL 1

#define MARK_NONE   -1
#define MAX_MARKS   32
#define MARK_CR     MAX_MARKS

#define UNIT_TM {1.0, 0.0, 0.0, 1.0}

#define MM_PER_INCH     25.4
#define CM_PER_INCH     (MM_PER_INCH/10)

/* hardcopy or terminal device */
/* device output can be redirected to file/printer(both) */
#define DEVICE_TERM     0
#define DEVICE_FILE     1
#define DEVICE_PRINT    2
#define DEVICE_AUX      3

/* Page orientation */
#define PAGE_ORIENT_LANDSCAPE  0
#define PAGE_ORIENT_PORTRAIT   1

#define INTENSITY(r, g, b) ((299*r + 587*g + 114*b)/1000)

#define COLOR_TRANS_NONE        0
#define COLOR_TRANS_GREYSCALE   1
#define COLOR_TRANS_BW          2
#define COLOR_TRANS_NEGATIVE    3
#define COLOR_TRANS_REVERSE     4
#define COLOR_TRANS_SRGB        5

#define BAD_COLOR       -1

#define BBOX_TYPE_GLOB  0
#define BBOX_TYPE_TEMP  1

/* The default max drawing path limit */
#define MAX_DRAWING_PATH  20000


/* A point in viewport coordinates */
typedef struct {
    double x;
    double y;
} VPoint, VVector;

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
    unsigned int nsegs;
    CStringSegment *segs;
    struct _CSGlyphCache *cglyphs;
} CompositeString;

typedef struct {
    int font;
    char chars_used[256];
} FontStats;

typedef struct {
    unsigned int ncolors;
    unsigned int *colors;
    unsigned int npatterns;
    unsigned int *patterns;
    unsigned int nlinestyles;
    unsigned int *linestyles;
    unsigned int nfonts;
    FontStats *fonts;
    view bbox;
} CanvasStats;

typedef struct {
    int width;
    int height;
    char *bits;
    int bpp;
    int pad;
    int type;
} CPixmap;

typedef struct _Canvas Canvas;

typedef void (*DevFreeDataProc)(void *data);

/* function to initialize device */
typedef int (*DevInitProc)(const Canvas *canvas, void *data,
    const CanvasStats *cstats);
/* function to parse device-specific commands */
typedef int (*DevParserProc)(const Canvas *canvas, void *data, const char *s);
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
    const VPoint *vp, const CPixmap *pm);
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
    int ctype;
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
    char *name;                            /* name of device */
    char *fext;                            /* filename extension */
    FontRaster fontrast;                   /* font rasterizing */
    Page_geometry pg;                      /* device defaults */
    
    int twopass;                           /* two-pass mode */
    int autocrop;                          /* resize canvas to tight BBox */

    int color_trans;                       /* color transformation type */
    
    int is_xrst;                           /* the device is Xrst-based */

    /* low-level device routines */
    DevInitProc          initgraphics;
    DevLeaveGraphicsProc leavegraphics;
    DevParserProc        parser;
    DevUpdateCmapProc    updatecmap;
    DevDrawPixelProc     drawpixel;
    DevDrawPolyLineProc  drawpolyline;
    DevFillPolygonProc   fillpolygon;
    DevDrawArcProc       drawarc;
    DevFillArcProc       fillarc;
    DevPutPixmapProc     putpixmap;
    DevPutTextProc       puttext;
    
    void                 *devdata;         /* device private data */
    DevFreeDataProc      freedata;         /* freeing private data */
    
    void                 *udata;           /* user-supplied data */
} Device_entry;

char *canvas_get_username(const Canvas *canvas);
char *canvas_get_docname(const Canvas *canvas);
char *canvas_get_description(const Canvas *canvas);

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


void canvas_set_prstream(Canvas *canvas, void *prstream);
void *canvas_get_prstream(const Canvas *canvas);

void set_max_path_limit(Canvas *canvas, int limit);
int get_max_path_limit(const Canvas *canvas);

unsigned int number_of_colors(const Canvas *canvas);
double get_rgb_intensity(const RGB *rgb);
int compare_rgb(const RGB *rgb1, const RGB *rgb2);

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
char *get_charname(const Canvas *canvas, int font, char c);

unsigned int number_of_patterns(const Canvas *canvas);
Pattern *canvas_get_pattern(const Canvas *canvas, unsigned int n);

unsigned int number_of_linestyles(const Canvas *canvas);
LineStyle *canvas_get_linestyle(const Canvas *canvas, unsigned int n);

int device_set_udata(Canvas *canvas, unsigned int dindex, void *udata);
void *device_get_udata(const Canvas *canvas, unsigned int dindex);

#if !defined(CANVAS_BACKEND_API) || defined(__CANVASP_H_)

int canvas_init(void);
Canvas *canvas_new(void);
void canvas_free(Canvas *canvas);

void canvas_set_udata(Canvas *canvas, void *data);
void *canvas_get_udata(const Canvas *canvas);

void canvas_set_username(Canvas *canvas, const char *s);
void canvas_set_docname(Canvas *canvas, const char *s);
void canvas_set_description(Canvas *canvas, const char *s);

void canvas_set_fmap_proc(Canvas *canvas, CanvasFMapProc fmap_proc);
void canvas_set_csparse_proc(Canvas *canvas, CanvasCSParseProc csparse_proc);
void canvas_set_fontsize_scale(Canvas *canvas, double fscale);
void canvas_set_linewidth_scale(Canvas *canvas, double lscale);

void canvas_set_pagefill(Canvas *canvas, int flag);

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

void DrawPixel(Canvas *canvas, const VPoint *vp);
void DrawPolyline(Canvas *canvas, const VPoint *vps, int n, int mode);
void DrawPolygon(Canvas *canvas, const VPoint *vps, int n);
void DrawArc(Canvas *canvas, const VPoint *vp1, const VPoint *vp2,
    double angle1, double angle2, int closure_type, int draw_closure);
void DrawFilledArc(Canvas *canvas, const VPoint *vp1, const VPoint *vp2,
    double angle1, double angle2, int mode);
void WriteString(Canvas *canvas,
    const VPoint *vp, double angle, int just, const char *s);

void DrawRect(Canvas *canvas, const VPoint *vp1, const VPoint *vp2);
void DrawFilledRect(Canvas *canvas, const VPoint *vp1, const VPoint *vp2);
void DrawLine(Canvas *canvas, const VPoint *vp1, const VPoint *vp2);
void DrawEllipse(Canvas *canvas, const VPoint *vp1, const VPoint *vp2);
void DrawFilledEllipse(Canvas *canvas, const VPoint *vp1, const VPoint *vp2);
void DrawCircle(Canvas *canvas, const VPoint *vp, double radius);
void DrawFilledCircle(Canvas *canvas, const VPoint *vp, double radius);

CStringSegment *cstring_seg_new(CompositeString *cstring);
double tm_size(const TextMatrix *tm);
int tm_scale(TextMatrix *tm, double s);
int tm_rotate(TextMatrix *tm, double angle);
int tm_slant(TextMatrix *tm, double slant);
int tm_product(TextMatrix *tm, const TextMatrix *p);

void set_draw_mode(Canvas *canvas, int mode);
int get_draw_mode(const Canvas *canvas);

int is_validVPoint(const Canvas *canvas, const VPoint *vp);

void reset_bbox(Canvas *canvas, int type);
void freeze_bbox(Canvas *canvas, int type);
int get_bbox(const Canvas *canvas, int type, view *v);
void update_bbox(Canvas *canvas, int type, const VPoint *vp);
int melt_bbox(Canvas *canvas, int type);
void activate_bbox(Canvas *canvas, int type, int status);

int view_extend(view *v, double w);
int merge_bboxes(const view *v1, const view *v2, view *v);
void vpswap(VPoint *vp1, VPoint *vp2);
int VPoints2bbox(const VPoint *vp1, const VPoint *vp2, view *bb);

int is_vpoint_inside(const view *v, const VPoint *vp, double epsilon);

int canvas_cmap_reset(Canvas *canvas);
int canvas_store_color(Canvas *canvas, unsigned int n, const RGB *rgb);

int canvas_set_encoding(Canvas *canvas, char *encfile);
int canvas_add_font(Canvas *canvas, char *ffile, const char *alias);

int canvas_get_font_by_name(const Canvas *canvas, const char *fname);

int select_device(Canvas *canvas, unsigned int dindex);

Device_entry *get_device_props(const Canvas *canvas, int device);

char *get_device_name(const Canvas *canvas, int device);

int is_valid_page_geometry(const Page_geometry *pg);
int set_page_geometry(Canvas *canvas, const Page_geometry *pg);

int get_device_page_dimensions(const Canvas *canvas,
    unsigned int dindex, int *wpp, int *hpp);

int get_device_by_name(const Canvas *canvas, const char *dname);

int parse_device_options(Canvas *canvas, unsigned int dindex, char *options);

int number_of_devices(const Canvas *canvas);

int terminal_device(const Canvas *canvas);
int device_is_aux(const Canvas *canvas, unsigned int dindex);
int device_set_aux(const Canvas *canvas, unsigned int dindex);

PageFormat get_page_format(const Canvas *canvas, int device);

int canvas_draw(Canvas *canvas, CanvasDrawProc dproc, void *data);

int get_string_bbox(Canvas *canvas,
    const VPoint *vp, double angle, int just, const char *s, view *bbox);

CPixmap *canvas_raster_char(Canvas *canvas,
    int font, char c, float size, int *vshift, int *hshift);
void canvas_cpixmap_free(CPixmap *pm);

int isvalid_viewport(const view *v);

#endif

#if defined(CANVAS_BACKEND_API) || defined(__CANVASP_H_)

/* Some useful macros */
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

typedef struct {
    unsigned int width;
    unsigned int height;
    unsigned int **matrix;
} Xrst_pixmap;

typedef int (*XrstDumpProc)(const Canvas *canvas, void *data,
    unsigned int ncolors, unsigned int *colors, Xrst_pixmap *pm);

typedef struct _XrstDevice_entry {
    int             type;
    char            *name;
    char            *fext;
    int             fontaa;
    DevParserProc   parser;
    
    XrstDumpProc    dump;
    
    void            *devdata;   /* device private data */
    DevFreeDataProc freedata;   /* freeing private data */
} XrstDevice_entry;


Device_entry *device_new(const char *name, int type, int twopass,
    void *devdata, DevFreeDataProc freedata);
void device_free(Device_entry *d);
int device_set_procs(Device_entry *d,
    DevInitProc          initgraphics,
    DevLeaveGraphicsProc leavegraphics,
    DevParserProc        parser,
    DevUpdateCmapProc    updatecmap,
    DevDrawPixelProc     drawpixel,
    DevDrawPolyLineProc  drawpolyline,
    DevFillPolygonProc   fillpolygon,
    DevDrawArcProc       drawarc,
    DevFillArcProc       fillarc,
    DevPutPixmapProc     putpixmap,
    DevPutTextProc       puttext);
int device_set_fext(Device_entry *d, const char *fext);
int device_set_autocrop(Device_entry *d, int autocrop);
int device_set_fontrast(Device_entry *d, FontRaster fontrast);
int device_set_dpi(Device_entry *d, float dpi);

Page_geometry *get_page_geometry(const Canvas *canvas);

int register_device(Canvas *canvas, Device_entry *d);

#ifdef HAVE_LIBXMI
int register_xrst_device(Canvas *canvas, const XrstDevice_entry *xdev);
#endif

int get_rgb(const Canvas *canvas, unsigned int cindex, RGB *rgb);
int  get_frgb(const Canvas *canvas, unsigned int cindex, fRGB *frgb);

double get_colorintensity(const Canvas *canvas, int cindex);

int get_cmy(const Canvas *canvas, unsigned int cindex, CMY *cmy);
int get_cmyk(const Canvas *canvas, unsigned int cindex, CMYK *cmyk);
int get_fcmyk(const Canvas *canvas, unsigned int cindex, fCMYK *fcmyk);

char *font_subset(const Canvas *canvas,
    int font, char *mask, unsigned long *datalen);

double *get_kerning_vector(const Canvas *canvas,
    const char *str, int len, int font);

void *device_get_devdata(const Canvas *canvas, unsigned int dindex);


/* PostScript/EPS driver */
#define PS_FORMAT   0
#define EPS_FORMAT  1

typedef enum {
    PS_COLORSPACE_GRAYSCALE,
    PS_COLORSPACE_RGB,
    PS_COLORSPACE_CMYK
} PSColorSpace;

#define PS_MEDIA_FEED_AUTO    0  
#define PS_MEDIA_FEED_MATCH   1
#define PS_MEDIA_FEED_MANUAL  2

#define PS_DOCDATA_7BIT       0
#define PS_DOCDATA_8BIT       1
#define PS_DOCDATA_BINARY     2

#define PS_FONT_EMBED_NONE    0
#define PS_FONT_EMBED_BUT13   1
#define PS_FONT_EMBED_BUT35   2
#define PS_FONT_EMBED_ALL     3

typedef struct {
    int format;

    unsigned long page_scale;
    double pixel_size;
    float page_scalef;
    int page_orientation;

    int color;
    int pattern;
    double linew;
    int lines;
    int linecap;
    int linejoin;

    int level2;
    PSColorSpace colorspace;
    int docdata;
    int fonts;
    int printable;

    int offset_x;
    int offset_y;
    int feed;
    int hwres;
} PS_data;


#ifdef HAVE_LIBPDF
#include <pdflib.h>

typedef enum {
    PDF_1_3,
    PDF_1_4,
    PDF_1_5
} PDFCompatibility;

typedef enum {
    PDF_COLORSPACE_GRAYSCALE,
    PDF_COLORSPACE_RGB,
    PDF_COLORSPACE_CMYK
} PDFColorSpace;

typedef struct {
    PDF             *phandle;

    unsigned long    page_scale;
    float            pixel_size;
    float            page_scalef;

    int             *font_ids;
    int             *pattern_ids;

    int              color;
    int              pattern;
    double           linew;
    int              lines;
    int              linecap;
    int              linejoin;

    PDFCompatibility compat;
    PDFColorSpace    colorspace;
    int              compression;
    int              fpprec;
    
    int              kerning_supported;
} PDF_data;

#endif /* HAVE_LIBPDF */

#ifdef HAVE_LIBXMI

/* PNM sub-formats */
#define PNM_FORMAT_PBM  0
#define PNM_FORMAT_PGM  1
#define PNM_FORMAT_PPM  2

typedef struct {
    int format;
    int rawbits;
} PNM_data;

#ifdef HAVE_LIBPNG

typedef struct {
    int interlaced;
    int transparent;
    int compression;
} PNG_data;

#endif /* HAVE_LIBPNG */

#ifdef HAVE_LIBJPEG

#define JPEG_DCT_IFAST  0
#define JPEG_DCT_ISLOW  1
#define JPEG_DCT_FLOAT  2

typedef struct {
    int quality;
    int grayscale;
    int baseline;
    int progressive;
    int optimize;
    int smoothing;
    int dct;
} JPG_data;

#endif /* HAVE_LIBJPEG */

#endif /* HAVE_LIBXMI */

#endif

/* Dummy/NULL driver */
int register_dummy_drv(Canvas *canvas);

/* Qt driver */
int register_qt_drv(Canvas *canvas);

/* EMF driver */
int register_emf_drv(Canvas *canvas);

/* Grace Metafile driver */
int register_mf_drv(Canvas *canvas);

/* PostScript driver */
int register_ps_drv(Canvas *canvas);
/* EPS driver */
int register_eps_drv(Canvas *canvas);

/* MIF driver */
int register_mif_drv(Canvas *canvas);

/* SVG driver */
int register_svg_drv(Canvas *canvas);

#ifdef HAVE_LIBPDF
int register_pdf_drv(Canvas *canvas);
#endif

#ifdef HAVE_LIBXMI
int register_pnm_drv(Canvas *canvas);

#ifdef HAVE_LIBJPEG
int register_jpg_drv(Canvas *canvas);
#endif

#ifdef HAVE_LIBPNG
int register_png_drv(Canvas *canvas);
#endif
#endif /* HAVE_LIBXMI */

#endif /* __CANVAS_H_ */
