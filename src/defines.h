/*
 * Grace - Graphics for Exploratory Data Analysis
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-95 Paul J Turner, Portland, OR
 * Copyright (c) 1996-98 GRACE Development Team
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

/*
 * some constants
 *
 */

/* max path length */
#define GR_MAXPATHLEN 256

/* max length for strings */
#define MAX_STRING_LENGTH 512


#define MAXPLOT 30              /* max number of sets in a graph */
#define MAX_SET_COLS 6          /* max number of data columns for a set */
#define MAXREGION 5             /* max number of regions */
#define MAXAXES 4               /* max number of axes per graph */
#define MAX_TICKS 100           /* max number of ticks/labels per axis */

/* max number of different objects */
#define MAXLINES 50             /* max number of lines */
#define MAXBOXES 50             /* max number of boxes */
#define MAXELLIPSES 50          /* max number of ellipses */
#define MAXSTR 100              /* max number of strings */

#define MAX_LINEWIDTH 10        /* max width of drawn lines */

#define MAX_ZOOM_STACK 20       /* max stack depth for world stack */
#define MAXPARM 10              /* max number of parameters for non-lin fit */

#define MAXFIT 12               /* max degree of polynomial+1 that can be
                                 * fitted */


#define MAX_JUST 2
#define MAX_ARROW 3
#define MAX_PREC 10


#ifndef MAXARR
#  define MAXARR 20000          /* max elements in an array */
#endif

#ifndef MAXPICKDIST
#  define MAXPICKDIST 0.1       /* the maximum distance away from an object */
#endif                          /* you may be when picking it (in viewport  */
                                /* coordinates)                             */  

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

/*
 *  location types for objects
 */
#define LOCWORLD 0
#define LOCVIEW 1

/*
 * types of coordinate frames
 */
#define COORDINATES_XY      0       /* Cartesian coordinates */
#define COORDINATES_POLAR   1       /* Polar coordinates */
                                
/*
 * types of axis scale mappings
 */
#define SCALE_NORMAL    0       /* normal linear scale */
#define SCALE_LOG       1       /* logarithmic  scale */
#define SCALE_REC       2       /* reciprocal, reserved */

/*
 * coordinates
 */
#define AXIS_TYPE_ANY -1
#define AXIS_TYPE_X    0
#define AXIS_TYPE_Y    1
#define AXIS_TYPE_BAD  2

/*
 * types of axes
 */
#define ALL_AXES    -3
#define ALL_X_AXES  -2
#define ALL_Y_AXES  -1

#define X_AXIS  0
#define Y_AXIS  1
#define ZX_AXIS 2
#define ZY_AXIS 3

/*
 * setno == all sets selected
 */
#define ALL_SETS    -1

/*
 * gno == all graphs selected
 */
#define ALL_GRAPHS    -1

/* type of splines */
#define SPLINE_NONE     0
#define SPLINE_CUBIC    1
#define SPLINE_AKIMA    2

/* Canvas types */
#define PAGE_FREE       0
#define PAGE_FIXED      1

/* Strings and things */
#define OBJECT_LINE     0
#define OBJECT_BOX      1
#define OBJECT_ELLIPSE  2
#define OBJECT_STRING   3

/* Region definitions */
#define REGION_ABOVE    0
#define REGION_BELOW    1
#define REGION_TOLEFT   2
#define REGION_TORIGHT  3
#define REGION_POLYI    4
#define REGION_POLYO    5
#define REGION_HORIZI   6
#define REGION_VERTI    7
#define REGION_HORIZO   8
#define REGION_VERTO    9

/* Axis label layout */
#define LAYOUT_PARALLEL         0
#define LAYOUT_PERPENDICULAR    1

/* Axis & tick placement */
#define PLACE_LEFT      0
#define PLACE_RIGHT     1
#define PLACE_TOP       2
#define PLACE_BOTTOM    3
#define PLACE_BOTH      4

/* Tick label placement */
#define LABEL_ONTICK    0
#define LABEL_BETWEEN   1

/* Coordinates */
#define COORD_VIEW      0
#define COORD_WORLD     1

/* Tick sign type */
#define SIGN_NORMAL     0
#define SIGN_ABSOLUTE   1
#define SIGN_NEGATE     2


/* Tick label/display formats */
#define FORMAT_INVALID         -1
#define FORMAT_DECIMAL          0
#define FORMAT_EXPONENTIAL      1
#define FORMAT_GENERAL          2
#define FORMAT_POWER            3
#define FORMAT_SCIENTIFIC       4
#define FORMAT_ENGINEERING      5
#define FORMAT_DDMMYY           6
#define FORMAT_MMDDYY           7
#define FORMAT_YYMMDD           8
#define FORMAT_MMYY             9
#define FORMAT_MMDD            10
#define FORMAT_MONTHDAY        11
#define FORMAT_DAYMONTH        12
#define FORMAT_MONTHS          13
#define FORMAT_MONTHSY         14
#define FORMAT_MONTHL          15
#define FORMAT_DAYOFWEEKS      16
#define FORMAT_DAYOFWEEKL      17
#define FORMAT_DAYOFYEAR       18
#define FORMAT_HMS             19
#define FORMAT_MMDDHMS         20
#define FORMAT_MMDDYYHMS       21
#define FORMAT_YYMMDDHMS       22
#define FORMAT_DEGREESLON      23
#define FORMAT_DEGREESMMLON    24
#define FORMAT_DEGREESMMSSLON  25
#define FORMAT_MMSSLON         26
#define FORMAT_DEGREESLAT      27
#define FORMAT_DEGREESMMLAT    28
#define FORMAT_DEGREESMMSSLAT  29
#define FORMAT_MMSSLAT         30

/* Focus policy */
#define FOCUS_CLICK     0
#define FOCUS_SET       1
#define FOCUS_FOLLOWS   2

/* Autoscale, tick mark etc. type */
#define TYPE_AUTO       0
#define TYPE_SPEC       1

/* Tick direction */
#define TICKS_IN        0
#define TICKS_OUT       1
#define TICKS_BOTH      2

/* Data source type */
#define SOURCE_DISK     0
#define SOURCE_PIPE     1

/* Justifications */
#define JUST_LEFT       0
#define JUST_RIGHT      1
#define JUST_CENTER     2
#define JUST_BOTTOM     0
#define JUST_TOP        4
#define JUST_MIDDLE     8
#define JUST_BBOX       0
#define JUST_OBJECT     16


/* Types of running command */
#define RUN_AVG         0
#define RUN_MED         1
#define RUN_MIN         2
#define RUN_MAX         3
#define RUN_STD         4

/* Types of Fourier transforms */
#define FFT_FFT         0
#define FFT_INVFFT      1
#define FFT_DFT         2
#define FFT_INVDFT      3

/* return codes */
#define GRACE_EXIT_SUCCESS (0)
#define GRACE_EXIT_FAILURE (1)

#ifndef TRUE
#  define TRUE 1
#endif
#ifndef FALSE
#  define FALSE 0
#endif

/* types of autscales */
#define AUTOSCALE_NONE    0
#define AUTOSCALE_X       1
#define AUTOSCALE_Y       2
#define AUTOSCALE_XY      3

/*
 * for set selector gadgets
 */
#define SET_SELECT_ERROR -99
#define SET_SELECT_ACTIVE 0
#define SET_SELECT_ALL -1
#define SET_SELECT_NEXT -2
#define SET_SELECT_NEAREST -3
#define GRAPH_SELECT_CURRENT -1
#define GRAPH_SELECT_ALL -2
#define FILTER_SELECT_NONE 0
#define FILTER_SELECT_ACTIVE 1
#define FILTER_SELECT_ALL 2
#define FILTER_SELECT_INACT 3
#define FILTER_SELECT_DEACT 4
#define FILTER_SELECT_SORT 5
#define SELECTION_TYPE_SINGLE 0
#define SELECTION_TYPE_MULTIPLE 1

/* for canvas event proc */
#define ZOOM_1ST 1
#define ZOOM_2ND 2
#define VIEW_1ST 3
#define VIEW_2ND 4
#define STR_LOC 5
#define LEG_LOC 6
#define FIND_POINT 7
#define DEL_POINT 8
#define MOVE_POINT1ST 9
#define MOVE_POINT2ND 10
#define ADD_POINT 11
#define DEL_OBJECT 12
#define MOVE_OBJECT_1ST 13
#define MOVE_OBJECT_2ND 14
#define MAKE_BOX_1ST 15
#define MAKE_BOX_2ND 16
#define MAKE_LINE_1ST 17
#define MAKE_LINE_2ND 18
#define MAKE_CIRC_1ST 19
#define MAKE_CIRC_2ND 20
#define MAKE_ARC_1ST 21
#define MAKE_ARC_2ND 22
#define MAKE_ELLIP_1ST 23
#define MAKE_ELLIP_2ND 24
#define SEL_POINT 25
#define STR_EDIT 26
#define COMP_AREA 27
#define COMP_PERIMETER 28
#define STR_LOC1ST 29
#define STR_LOC2ND 30
#define GRAPH_FOCUS 31
#define TRACKER 32
#define DEF_REGION 43
#define DEF_REGION1ST 44
#define DEF_REGION2ND 45
#define PAINT_POINTS 46
#define KILL_NEAREST 47
#define COPY_NEAREST1ST 48
#define COPY_NEAREST2ND 49
#define MOVE_NEAREST1ST 50
#define MOVE_NEAREST2ND 51
#define REVERSE_NEAREST 52
#define JOIN_NEAREST1ST 53
#define JOIN_NEAREST2ND 54
#define DEACTIVATE_NEAREST 55
#define EXTRACT_NEAREST1ST 56
#define EXTRACT_NEAREST2ND 57
#define DELETE_NEAREST1ST 58
#define DELETE_NEAREST2ND 59
#define INSERT_POINTS 60
#define INSERT_SET 61
#define EDIT_OBJECT 62
#define PLACE_TIMESTAMP 63
#define COPY_OBJECT1ST 64
#define COPY_OBJECT2ND 65
#define CUT_OBJECT 66
#define PASTE_OBJECT 67
#define AUTO_NEAREST 68
#define ZOOMX_1ST 69
#define ZOOMX_2ND 70
#define ZOOMY_1ST 71
#define ZOOMY_2ND 72
#define PICK_SET 73
#define PICK_SET1 74
#define PICK_SET2 75
#define PICK_EXPR 76
#define PICK_HISTO 77
#define PICK_FOURIER 78
#define PICK_RUNAVG 79
#define PICK_RUNSTD 80
#define PICK_RUNMIN 81
#define PICK_RUNMAX 82
#define PICK_DIFF 83
#define PICK_INT 84
#define PICK_REG 85
#define PICK_XCOR 86
#define PICK_SAMP 87
#define PICK_PRUNE 88
#define PICK_FILTER 89
#define PICK_EXPR2 90
#define PICK_SPLINE 91
#define PICK_INTERP 92
#define PICK_SAMPLE 93
#define PICK_SEASONAL 94
#define PICK_BREAK 95
#define ADD_POINT1ST 96
#define ADD_POINT2ND 97
#define ADD_POINT3RD 98
#define ADD_POINT_INTERIOR 99
#define DISLINE1ST 100
#define DISLINE2ND 101

/* for stufftext() in monwin.c used here and there */
#define STUFF_TEXT  0
#define STUFF_START 1
#define STUFF_STOP  2

#define NONAME "Untitled"

/* for data pruning */
#define PRUNE_INTERPOLATION     0
#define PRUNE_CIRCLE            1
#define PRUNE_ELLIPSE           2
#define PRUNE_RECTANGLE         3

#define PRUNE_LIN               0
#define PRUNE_LOG               1

#define PRUNE_VIEWPORT          0
#define PRUNE_WORLD             1


/* for io filters */
#define FILTER_INPUT    0
#define FILTER_OUTPUT   1

#define FILTER_MAGIC    0
#define FILTER_PATTERN  1

/* histogram types */
#define HISTOGRAM_TYPE_ORDINARY     0
#define HISTOGRAM_TYPE_CUMULATIVE   1

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

/* nonlprefs.load possible values */
#define LOAD_VALUES         0
#define LOAD_RESIDUALS      1
#define LOAD_FUNCTION       2


/*
 * symbol table entry type
 */
typedef struct {
    char *s;
    int type;
    double (*fnc)();
} symtab_entry;

/*
 * defaults
 */
typedef struct {
    int color;
    int bgcolor;
    int pattern;
    int lines;
    int linew;
    double charsize;
    int font;
    double symsize;
} defaults;

typedef struct {
    int color;
    int pattern;
/*
 *     int transparency;
 */
} Pen;

/* A point in world coordinates */
typedef struct {
    double x;
    double y;
} WPoint;


/* A point in viewport coordinates */
typedef struct {
    double x;
    double y;
} VPoint;

typedef struct {
    double x;
    double y;
} VVector;

/*
 * typedefs for objects
 */
typedef struct {
    int active;
    int loctype;
    int gno;
    double x1;
    double y1;
    double x2;
    double y2;
    int lines;
    int linew;
    int color;
    int fillcolor;
    int fillpattern;
} boxtype;

typedef struct {
    int active;
    int loctype;
    int gno;
    double x1;
    double y1;
    double x2;
    double y2;
    int lines;
    int linew;
    int color;
    int arrow;
    int atype;
    double asize;
} linetype;

typedef struct {
    int active;
    int loctype;
    int gno;
    double x1;
    double y1;
    double x2;
    double y2;
    int lines;
    int linew;
    int color;
    int fillcolor;
    int fillpattern;
} ellipsetype;

typedef struct {
    int active;
    int loctype;
    int gno;
    double x;
    double y;
    int color;
    int rot;
    int font;
    int just;
    double charsize;
    char *s;
} plotstr;


typedef struct {
    double xg1, xg2, yg1, yg2;  /* window into world coords */
} world;

typedef struct {
    double xv1, xv2, yv1, yv2;  /* device viewport */
} view;

/*
 * world stack
 */
typedef struct {
    world w;                    /* current world */
} world_stack;

typedef struct {
    plotstr title;              /* graph title */
    plotstr stitle;             /* graph subtitle */
} labels;

typedef struct {
    int active;                 /* active flag */
    int type;                   /* regression type */
    double xmin;
    double xmax;
    double coef[15];
} Regression;

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

typedef struct {
    int active;          /* on/off */
    int type;            /* type of error bar */
    int linew;           /* error bar line width */
    int lines;           /* error bar line style */
    int riser_linew;     /* connecting line between error limits line width */
    int riser_lines;     /* connecting line between error limits line style */
    double length;       /* length of error bar */
} Errbar;

/* Annotative strings for data values */
typedef struct {
    int active;                 /* active or not */
    int type;                   /* type */
    double size;                /* char size */
    int font;                   /* font */
    int color;                  /* color */
    int angle;                  /* angle */
    int format;                 /* format */
    int prec;                   /* precision */
    char prestr[64];            /* prepend string */
    char appstr[64];            /* append string */
    VPoint offset;              /* offset related to symbol position */
} AValue;



typedef struct {
    int type;
    double wtpos;
    char *label;
} tickloc;

typedef struct {
    double size;              /* length of tickmarks */
    int color;                /* color of tickmarks */
    int linew;                /* linewidth of tickmarks */
    int lines;                /* linestyle of tickmarks */
    int gridflag;             /* grid lines at tick marks */
} tickprops;

typedef struct {
    int active;                 /* active or not */

    int zero;                   /* "zero" axis or plain */

    plotstr label;              /* graph axis label */
    int label_layout;           /* axis label orientation (h or v) */
    int label_place;            /* axis label placement (specfied or auto) */
    int label_op;               /* tick labels on opposite side or both */

    int t_drawbar;              /* draw a bar connecting tick marks */
    int t_drawbarcolor;         /* color of bar */
    int t_drawbarlines;         /* linestyle of bar */
    int t_drawbarlinew;         /* line width of bar */

    double offsx, offsy;        /* offset of axes in viewport coords
                                   (attention: these
				   are not x and y coordinates but
				   perpendicular and parallel offsets */

    int t_flag;                 /* toggle tickmark display */
    int t_type;                 /* type of tickmarks, auto or specified */
    int t_autonum;              /* approximate default number of major ticks */

    int t_round;                /* place major ticks at rounded positions */

    double tmajor;              /* major tick divisions */
    int nminor;                 /* number of minor ticks per one major division */

    int nticks;                 /* total number of ticks */
    tickloc tloc[MAX_TICKS];    /* locations of ticks */

    int t_inout;                /* ticks inward, outward or both */
    int t_op;                   /* ticks on opposite side */
    
    tickprops props;
    tickprops mprops;

    int tl_flag;                /* toggle ticmark labels on or off */
    int tl_type;                /* either auto or specified (below) */
    int tl_angle;               /* angle to draw labels */

    int tl_sign;                /* tick labels normal, absolute value, or negate */
    int tl_prec;                /* places to right of decimal point */
    int tl_format;              /* decimal or exp. or ... ticmark labels */

    int tl_skip;                /* tick labels to skip */
    int tl_staggered;           /* tick labels staggered */
    int tl_starttype;           /* start at graphmin or use tl_start/stop */
    int tl_stoptype;            /* start at graphmax or use tl_start/stop */
    double tl_start;            /* value of x to begin tick labels and major ticks */
    double tl_stop;             /* value of x to end tick labels and major ticks */

    int tl_op;                  /* tick labels on opposite side or both */

    int tl_gaptype;             /* tick label placement auto or specified */
    VVector tl_gap;             /* tick label to tickmark distance
				   (parallel and perpendicular to axis) */

    int tl_font;                /* font to use for tick labels */
    double tl_charsize;         /* character size for tick labels */
    int tl_color;               /* color of tick labels */

    char tl_appstr[64];         /* append string to tick label */
    char tl_prestr[64];         /* prepend string to tick label */

} tickmarks;

typedef struct {
    int active;                 /* legend on or off */
    int loctype;                /* locate in world or viewport coords */
    int vgap;                   /* verticle gap between entries */
    int hgap;                   /* horizontal gap(s) between legend string
                                                                  elements */
    int len;                    /* length of line to draw */
    int invert;                 /* switch between ascending and descending
                                   order of set legends */
    double legx;                /* location on graph */
    double legy;
    int font;
    double charsize;
    int color;
    Pen boxpen;
    Pen boxfillpen;
    int boxlinew;               /* legend frame line width */
    int boxlines;               /* legend frame line style */
} legend;

typedef struct {
    int active;                 /* region on or off */
    int type;                   /* region type */
    int color;                  /* region color */
    int lines;                  /* region linestyle */
    int linew;                  /* region line width */
    int *linkto;                /* associated with graphs in linkto */
    int n;                      /* number of points if type is POLY */
    double *x, *y;              /* coordinates if type is POLY */
    double x1, y1, x2, y2;      /* starting and ending points if type is not POLY */
} region;

typedef struct {
    int type;                   /* frame type */
    Pen pen;                    /* frame pen */
    int lines;                  /* frame linestyle */
    int linew;                  /* frame line width */
    Pen fillpen;                /* fill pen */
} framep;


/* parameters for non-linear fit */
typedef struct {
    double value;       /* parameter itself */
    int constr;         /* whether or not to use constraints */
    double min;         /* low bound constraint */
    double max;         /* upper bound constraint */
} nonlparms;

/* options for non-linear fit */
typedef struct {
    char title[256];    /* fit title */
    char formula[256];  /* fit function */
    int parnum;         /* # of fit parameters */
    double tolerance;   /* tolerance */
} nonlopts;

/* prefs for non-linear fit */
typedef struct {
    int autoload;       /* do autoload */
    int load;           /* load to... */
    int npoints;        /* # of points to evaluate function at */
    double start;       /* start... */
    double stop;        /* stop ... */
} nonlprefs;


/* target graph & set*/
typedef struct {
    int gno;    /* graph # */
    int setno;  /* set # */
} target;

#define copyx(gno, setfrom, setto)      copycol2(gno, setfrom, gno, setto, 0)
#define copyy(gno, setfrom, setto)      copycol2(gno, setfrom, gno, setto, 1)

#endif /* __DEFINES_H_ */
