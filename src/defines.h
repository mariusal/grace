/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2003 Grace Development Team
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
 *
 * constants and typedefs
 *
 */
#ifndef __DEFINES_H_
#define __DEFINES_H_

#include <config.h>

/*
 * some constants
 *
 */

/* max path length */
#define GR_MAXPATHLEN 256

#define MAXPARM 10              /* max number of parameters for non-lin fit */

#define MAX_PREC 10

/* dot (obsolete) */
#define SYM_DOT_OBS     1

/* max width of drawn lines */
#define MAX_LINEWIDTH 20.0

/* type of splines */
#define INTERP_LINEAR   0
#define INTERP_SPLINE   1
#define INTERP_ASPLINE  2

/* Focus policy */
#define FOCUS_CLICK     0
#define FOCUS_SET       1
#define FOCUS_FOLLOWS   2


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

/* Private colormap */
#define CMAP_INSTALL_NEVER      0
#define CMAP_INSTALL_ALWAYS     1
#define CMAP_INSTALL_AUTO       2

/* Zoom step */
#define ZOOM_STEP sqrt(M_SQRT2)


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
    int           delay;  /* real-time input delay */
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

/* tokens for the calendar dates parser */
typedef struct { int value;
                 int digits;
               } Int_token;

#endif /* __DEFINES_H_ */
