/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
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
 *
 * constants and typedefs
 *
 */
#ifndef __DEFINES_H_
#define __DEFINES_H_

#include <config.h>

#include "grace/canvas.h"

/*
 * some constants
 *
 */

/* max path length */
#define GR_MAXPATHLEN 256

/* max length for strings */
#define MAX_STRING_LENGTH 512

#define MAXPARM 10              /* max number of parameters for non-lin fit */

#define MAX_ARROW 3
#define MAX_PREC 10

/* symbol types */

#define SYM_NONE    0
#define SYM_CIRCLE  1
#define SYM_SQUARE  2
#define SYM_DIAMOND 3
#define SYM_TRIANG1 4
#define SYM_TRIANG2 5
#define SYM_TRIANG3 6
#define SYM_TRIANG4 7
#define SYM_PLUS    8
#define SYM_X       9
#define SYM_SPLAT  10
#define SYM_CHAR   11

/* max number of symbols defined */
#define MAXSYM  12

/* dot (obsolete) */
#define SYM_DOT_OBS     1

/* max width of drawn lines */
#define MAX_LINEWIDTH 20.0

/*
 * axis types
 */
#define AXIS_TYPE_X    0
#define AXIS_TYPE_Y    1

/*
 * axis type masks
 */
#define AXIS_MASK_X  1
#define AXIS_MASK_Y  2
#define AXIS_MASK_XY 3

/* type of splines */
#define INTERP_LINEAR   0
#define INTERP_SPLINE   1
#define INTERP_ASPLINE  2

/* Canvas types */
#define PAGE_FREE       0
#define PAGE_FIXED      1

/* Axis label layout */
#define LAYOUT_PARALLEL         0
#define LAYOUT_PERPENDICULAR    1

/* Placement (axis labels, ticks, error bars */
typedef enum {
    PLACEMENT_NORMAL,
    PLACEMENT_OPPOSITE,
    PLACEMENT_BOTH
} PlacementType;

/* Tick label placement */
#define LABEL_ONTICK    0
#define LABEL_BETWEEN   1

/* Coordinates */
#define COORD_VIEW      0
#define COORD_FRAME     1
#define COORD_WORLD     2


/* Tick label/display formats */
typedef enum {
    FORMAT_DECIMAL,
    FORMAT_EXPONENTIAL,
    FORMAT_GENERAL,
    FORMAT_POWER,
    FORMAT_SCIENTIFIC,
    FORMAT_ENGINEERING,
    FORMAT_DDMMYY,
    FORMAT_MMDDYY,
    FORMAT_YYMMDD,
    FORMAT_MMYY,
    FORMAT_MMDD,
    FORMAT_MONTHDAY,
    FORMAT_DAYMONTH,
    FORMAT_MONTHS,
    FORMAT_MONTHSY,
    FORMAT_MONTHL,
    FORMAT_DAYOFWEEKS,
    FORMAT_DAYOFWEEKL,
    FORMAT_DAYOFYEAR,
    FORMAT_HMS,
    FORMAT_MMDDHMS,
    FORMAT_MMDDYYHMS,
    FORMAT_YYMMDDHMS,
    FORMAT_DEGREESLON,
    FORMAT_DEGREESMMLON,
    FORMAT_DEGREESMMSSLON,
    FORMAT_MMSSLON,
    FORMAT_DEGREESLAT,
    FORMAT_DEGREESMMLAT,
    FORMAT_DEGREESMMSSLAT,
    FORMAT_MMSSLAT,
    FORMAT_BAD
} FormatType;

#define NUMBER_OF_FORMATTYPES   FORMAT_BAD

/* Focus policy */
#define FOCUS_CLICK     0
#define FOCUS_SET       1
#define FOCUS_FOLLOWS   2

/* Placement of labels etc */
#define TYPE_AUTO       0
#define TYPE_SPEC       1

/* User-defined tickmarks/labels */
#define TICKS_SPEC_NONE     0
#define TICKS_SPEC_MARKS    1
#define TICKS_SPEC_BOTH     2

/* Tick direction */
#define TICKS_IN        0
#define TICKS_OUT       1
#define TICKS_BOTH      2

/* Data source type */
#define SOURCE_DISK     0
#define SOURCE_PIPE     1


/* Types of running command */
#define RUN_AVG         0
#define RUN_MED         1
#define RUN_MIN         2
#define RUN_MAX         3
#define RUN_STD         4

/* types of autscales */
#define AUTOSCALE_NONE    0
#define AUTOSCALE_X       AXIS_MASK_X
#define AUTOSCALE_Y       AXIS_MASK_Y
#define AUTOSCALE_XY      AXIS_MASK_XY

/* Default document name */
#define NONAME "Untitled"

/* for io filters */
#define FILTER_INPUT    0
#define FILTER_OUTPUT   1

#define FILTER_MAGIC    0
#define FILTER_PATTERN  1

/* set line types */
#define LINE_TYPE_NONE          0
#define LINE_TYPE_STRAIGHT      1
#define LINE_TYPE_LEFTSTAIR     2
#define LINE_TYPE_RIGHTSTAIR    3
#define LINE_TYPE_SEGMENT2      4
#define LINE_TYPE_SEGMENT3      5

/* baseline types */
#define BASELINE_TYPE_0         0
#define BASELINE_TYPE_SMIN      1
#define BASELINE_TYPE_SMAX      2
#define BASELINE_TYPE_GMIN      3
#define BASELINE_TYPE_GMAX      4

/* set fill types */
#define SETFILL_NONE            0
#define SETFILL_POLYGON         1
#define SETFILL_BASELINE        2

/* types of ann. values */
#define AVALUE_TYPE_NONE        0
#define AVALUE_TYPE_X           1
#define AVALUE_TYPE_Y           2
#define AVALUE_TYPE_XY          3
#define AVALUE_TYPE_STRING      4
#define AVALUE_TYPE_Z           5

/* ticks */
#define TICK_TYPE_MAJOR     0
#define TICK_TYPE_MINOR     1

/* arrow types */
#define ARROW_TYPE_LINE     0
#define ARROW_TYPE_FILLED   1

/* push set direction */
#define PUSH_SET_TOFRONT    0
#define PUSH_SET_TOBACK     1

/* restriction types */
#define RESTRICT_NONE  -1
#define RESTRICT_WORLD -2
#define RESTRICT_REG0   0
#define RESTRICT_REG1   1
#define RESTRICT_REG2   2
#define RESTRICT_REG3   3
#define RESTRICT_REG4   4

/* FFT stuff */
#define FFT_XSCALE_INDEX       0
#define FFT_XSCALE_NU          1
#define FFT_XSCALE_OMEGA       2

#define FFT_NORM_NONE          0
#define FFT_NORM_FORWARD       1
#define FFT_NORM_BACKWARD      2
#define FFT_NORM_SYMMETRIC     3

#define FFT_WINDOW_NONE        0
#define FFT_WINDOW_TRIANGULAR  1
#define FFT_WINDOW_PARZEN      2
#define FFT_WINDOW_WELCH       3
#define FFT_WINDOW_HANNING     4
#define FFT_WINDOW_HAMMING     5
#define FFT_WINDOW_FLATTOP     6
#define FFT_WINDOW_BLACKMAN    7
#define FFT_WINDOW_KAISER      8

#define FFT_OUTPUT_MAGNITUDE   0
#define FFT_OUTPUT_PHASE       1
#define FFT_OUTPUT_RE          2
#define FFT_OUTPUT_IM          3
#define FFT_OUTPUT_REIM        4
#define FFT_OUTPUT_APHI        5

/* Differentiation */
#define DIFF_XPLACE_LEFT    0
#define DIFF_XPLACE_CENTER  1
#define DIFF_XPLACE_RIGHT   2

/* Running properties */
#define RUN_XPLACE_LEFT    0
#define RUN_XPLACE_AVERAGE 1
#define RUN_XPLACE_RIGHT   2

/* Typesetting defines */
#define SSCRIPT_SCALE M_SQRT1_2
#define SUBSCRIPT_SHIFT 0.4
#define SUPSCRIPT_SHIFT 0.6
#define ENLARGE_SCALE sqrt(M_SQRT2)
#define OBLIQUE_FACTOR 0.25

#define T1_DEFAULT_ENCODING_FILE  "Default.enc"
#define T1_FALLBACK_ENCODING_FILE "IsoLatin1.enc"

/*
 * defaults
 */
typedef struct {
    int color;
    int bgcolor;
    int pattern;
    int lines;
    double linew;
    double charsize;
    int font;
    double symsize;
} defaults;

typedef struct {
    int id;
    char *fontname;
    char *fallback;
} Fontdef;

/* A point in world coordinates */
typedef struct {
    double x;
    double y;
} WPoint;

/* A point in frame coordinates */
typedef struct {
    double x;
    double y;
} FPoint;


typedef struct {
    double xg1, xg2, yg1, yg2;  /* window into world coords */
} world;

/*
 * typedefs for objects
 */
typedef struct {
    int type;
    double length;  /* head length (L) */
    double dL_ff;   /* d/L form factor */
    double lL_ff;   /* l/L form factor */
} Arrow;

typedef struct {
    int active;
    VPoint offset;
    int color;
    double angle;
    int font;
    int just;
    double charsize;
    char *s;
    view bb;
} plotstr;


typedef struct {
    plotstr title;              /* graph title */
    plotstr stitle;             /* graph subtitle */
} labels;

typedef struct {
    int active;                 /* active flag */
    int type;                   /* regression type */
    int npts;                   /* number of points */
    double xmin;
    double xmax;
    double *a;
    double *b;
    double *c;
    double *d;
} Spline;


/* parameters for non-linear fit */
typedef struct {
    char *name;         /* symbolic name */
    double value;       /* parameter itself */
    int constr;         /* whether or not to use constraints */
    double min;         /* low bound constraint */
    double max;         /* upper bound constraint */
} nonlparm;

/* options for non-linear fit */
typedef struct {
    char *title;             /* fit title */
    char *formula;           /* fit function */
    int parnum;              /* # of fit parameters */
    double tolerance;        /* tolerance */
    nonlparm parms[MAXPARM]; /* fit parameters */
} NLFit;

/* real time inputs */
typedef struct _Input_buffer {
    int           fd;     /* file descriptor */
    int           errors; /* number of successive parse errors */
    int           lineno; /* line number */
    int           zeros;  /* number of successive reads of zero byte */
    int           reopen; /* non-zero if we should close and reopen */
                          /* when other side is closed (mainly for fifos) */
    char         *name;   /* name of the input (filename or symbolic name) */
    int           size;   /* size of the buffer for already read lines */
    int           used;   /* number of bytes used in the buffer */
    char         *buf;    /* buffer for already read lines */
    unsigned long id;     /* id for X library */
} Input_buffer;

/* dates formats */
typedef enum   { FMT_iso,
                 FMT_european,
                 FMT_us,
                 FMT_nohint
               } Dates_format;

/* rounding types for dates */
#define ROUND_SECOND 1
#define ROUND_MINUTE 2
#define ROUND_HOUR   3
#define ROUND_DAY    4
#define ROUND_MONTH  5

/* tokens for the calendar dates parser */
typedef struct { int value;
                 int digits;
               } Int_token;

#endif /* __DEFINES_H_ */
