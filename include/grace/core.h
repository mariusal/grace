/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1996-2004 Grace Development Team
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

#ifndef __CORE_H_
#define __CORE_H_

#include "grace/baseP.h"
#include "grace/canvas.h"

#define QUARK_ETYPE_MODIFY  1
#define QUARK_ETYPE_DELETE  2

#define CORNER_LL   0
#define CORNER_UL   1
#define CORNER_UR   2
#define CORNER_LR   3

/*
 * axis types
 */
#define AXIS_TYPE_X    0
#define AXIS_TYPE_Y    1

#define MAX_TICKS 256           /* max number of ticks/labels per axis */

/* Axis label layout */
#define LAYOUT_PARALLEL         0
#define LAYOUT_PERPENDICULAR    1

/* Placement of labels etc */
#define TYPE_AUTO       0
#define TYPE_SPEC       1

/* ticks */
#define TICK_TYPE_MAJOR     0
#define TICK_TYPE_MINOR     1

/* User-defined tickmarks/labels */
#define TICKS_SPEC_NONE     0
#define TICKS_SPEC_MARKS    1
#define TICKS_SPEC_BOTH     2

/* Tick direction */
#define TICKS_IN        0
#define TICKS_OUT       1
#define TICKS_BOTH      2

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

/* Arrow types */
#define ARROW_TYPE_LINE     0
#define ARROW_TYPE_FILLED   1
#define ARROW_TYPE_CIRCLE   2

#define FRAME_DECOR_NONE    0
#define FRAME_DECOR_LINE    1
#define FRAME_DECOR_RECT    2
#define FRAME_DECOR_OVAL    3

/* Data source type */
#define SOURCE_DISK     0
#define SOURCE_PIPE     1

/* Region types */
#define REGION_POLYGON  0
#define REGION_BAND     1
#define REGION_FORMULA  2

/* types of coordinate frames */
#define COORDINATES_XY      0       /* Cartesian coordinates */
#define COORDINATES_POLAR   1       /* Polar coordinates */
                                
/* Coordinates */
#define COORD_VIEW      0
#define COORD_FRAME     1
#define COORD_WORLD     2


typedef struct _Quark Quark;

typedef void  (*Quark_data_free)(void *data); 
typedef void *(*Quark_data_new)(void); 
typedef void *(*Quark_data_copy)(void *data); 

typedef int (*Quark_cb)(Quark *q, int etype, void *data); 

typedef int (*Quark_comp_proc)(const Quark *q1, const Quark *q2, void *udata); 

typedef struct _QuarkFlavor {
    unsigned int    fid;
    Quark_data_new  data_new;
    Quark_data_free data_free;
    Quark_data_copy data_copy;
} QuarkFlavor;

typedef struct _QuarkFactory {
    unsigned int refcount;
    
    QuarkFlavor *qflavours;
    unsigned int nflavours;
    
    void *udata;
} QuarkFactory;

struct _Quark {
    QuarkFactory *qfactory;
    
    void *udata;
    
    unsigned int fid;
    char *idstr;
    
    struct _Quark *parent;
    Storage *children;
    unsigned int dirtystate;
    unsigned int refcount;
    
    void *data;
    
    Quark_cb cb;
    void *cbdata;
};

typedef struct {
    unsigned int depth;
    unsigned int step;
    int post;
    int descend;
} QTraverseClosure;

typedef int (*Quark_traverse_hook)(Quark *q,
    void *udata, QTraverseClosure *closure); 


enum {
    QFlavorProject,
    QFlavorFrame,
    QFlavorGraph,
    QFlavorSet,
    QFlavorAxis,
    QFlavorRegion,
    QFlavorDObject,
    QFlavorAText,
    QFlavorContainer
};

#define QIDSTR(q) (q->idstr ? q->idstr:"unnamed")

/*
 * graphics defaults
 */
typedef struct {
    Line line;
    Pen fillpen;

    int font;
    double charsize;
} defaults;

typedef struct {
    int id;
    char *fontname;
    char *fallback;
} Fontdef;

typedef struct _Project {
    /* Version ID */
    int version_id;
    
    /* textual description */
    char *description;
    
#if 0
    Storage *blockdata;
#endif
    /* (pointer to) current graph */
    Quark *cg;
    
    /* timestamp */
    char *timestamp;
    
    /* page size */
    int page_wpp, page_hpp;
    
    /* font map */
    unsigned int nfonts;
    Fontdef *fontmap;
    
    /* page fill */
    int bgcolor;
    int bgfill;
    
    /* font size scale */
    double fscale;
    /* line width scale */
    double lscale;

    /* format for saving data sets */
    char *sformat;

    /* dates */
    double ref_date;
    int wrap_year;
    int two_digits_years;

    /* default graphics properties */
    defaults grdefaults;
    
    /* project file name */
    char *docname;	
} Project;

/* Locator props */
typedef struct {
    int pointset;               /* if (dsx, dsy) have been set */
    int pt_type;                /* type of locator display */
    double dsx, dsy;            /* locator fixed point */
    int fx, fy;                 /* locator format type */
    int px, py;                 /* locator precision */
} GLocator;

/* Legend box */
typedef struct {
    int active;                 /* legend on or off */

    int acorner;                /* anchor corner */
    VPoint offset;              /* location on graph */

    int singlesym;              /* draw single set symbol in the legend */
    
    double vgap;                /* vertical gap between entries */
    double hgap;                /* horizontal gap(s) between legend string
                                   elements */
    double len;                 /* length of line to draw */
    int invert;                 /* switch between ascending and descending
                                   order of set legends */
    int font;
    double charsize;
    int color;

    Line boxline;               /* legend frame line */
    Pen boxfillpen;             /* legend frame fill */

    view bb;
} legend;

typedef struct {
    int active;
    
    int type;                   /* frame type */
    
    Line outline;               /* outline    */
    Pen fillpen;                /* fill pen   */

    legend l;                   /* legends */
    view v;                     /* viewport */
} frame;

/* Graph type */
typedef enum {
    GRAPH_XY   ,
    GRAPH_CHART,
    GRAPH_POLAR,
    GRAPH_SMITH,
    GRAPH_FIXED,
    GRAPH_PIE,
    GRAPH_BAD
} GraphType;

#define NUMBER_OF_GRAPHTYPES  GRAPH_BAD

/* Types of axis scale mappings */
typedef enum {
    SCALE_NORMAL,
    SCALE_LOG,
    SCALE_REC,
    SCALE_LOGIT,
    SCALE_BAD
} ScaleType;

#define NUMBER_OF_SCALETYPES  SCALE_BAD

typedef struct {
    double xg1, xg2, yg1, yg2;  /* window into world coords */
} world;

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

typedef struct {
    double xv_med;
    double yv_med;
    double xv_rc;
    double yv_rc;
    double fxg_med;
    double fyg_med;
} ctrans_cache;

/*
 * a graph
 */
typedef struct {
    int active;                 /* display or not */

    int type;                   /* type of graph */
    int stacked;                /* TRUE if graph is stacked */

    world w;                    /* world bounds */

    int xscale;                 /* scale mapping of X axes*/
    int yscale;                 /* scale mapping of Y axes*/
    int xinvert;                /* X axes inverted, TRUE or FALSE */
    int yinvert;                /* Y axes inverted, TRUE or FALSE */
    int xyflip;                 /* whether x and y axes should be flipped */

    double znorm;               /* Normalization of pseudo-3D graphs */
    double bargap;              /* Distance between bars (in bar charts) */

    GLocator locator;           /* locator props */
    
    ctrans_cache ccache;        /* cached data for coordinate transforms */
} graph;


/* Placement (axis labels, ticks, error bars */
typedef enum {
    PLACEMENT_NORMAL,
    PLACEMENT_OPPOSITE,
    PLACEMENT_BOTH
} PlacementType;

typedef struct {
    int type;
    double wtpos;
    char *label;
} tickloc;

typedef struct {
    int inout;                /* inward, outward or both */
    double size;              /* length of tickmarks */
    int color;                /* color of tickmarks */
    double linew;             /* linewidth of tickmarks */
    int lines;                /* linestyle of tickmarks */
    int gridflag;             /* grid lines at tick marks */
} tickprops;

typedef struct {
    int color;
    double angle;
    int font;
    int just;
    double charsize;
} TextProps;

typedef struct {
    int active;                 /* active or not */

    int type;                   /* X or Y */
    int zero;                   /* "zero" axis or plain */

    int label_active;
    char *label;
    VVector label_offset;       /* axis label offset */
    TextProps label_tprops;     /* axis label text properties */
    int label_layout;           /* axis label orientation (h or v) */
    int label_place;            /* axis label placement (specfied or auto) */
    PlacementType label_op;     /* axis labels on opposite side or both */

    int t_drawbar;              /* draw a bar connecting tick marks */
    int t_drawbarcolor;         /* color of bar */
    int t_drawbarlines;         /* linestyle of bar */
    double t_drawbarlinew;      /* line width of bar */

    double offsx, offsy;        /* offset of axes in viewport coords
                                   (attention: these
				   are not x and y coordinates but
				   perpendicular and parallel offsets */

    int t_flag;                 /* toggle tickmark display */
    int t_autonum;              /* approximate default number of major ticks */

    int t_spec;                 /* special (user-defined) tickmarks/ticklabels, */
                                /* can be none/marks/both marks and labels */

    int t_round;                /* place major ticks at rounded positions */

    double tmajor;              /* major tick divisions */
    int nminor;                 /* number of minor ticks per one major division */

    int nticks;                 /* total number of ticks */
    tickloc tloc[MAX_TICKS];    /* locations of ticks */

    PlacementType t_op;         /* ticks on opposite side */
    
    tickprops props;
    tickprops mprops;

    int tl_flag;                /* toggle ticmark labels on or off */
    VVector tl_gap;             /* tick label to tickmark distance
				   (parallel and perpendicular to axis) */
    TextProps tl_tprops;        /* tick label text properties */

    int tl_format;              /* tickmark label format */
    int tl_prec;                /* places to right of decimal point */

    char tl_appstr[64];         /* append string to tick label */
    char tl_prestr[64];         /* prepend string to tick label */

    char *tl_formula;           /* transformation formula */

    int tl_skip;                /* tick labels to skip */
    int tl_staggered;           /* tick labels staggered */
    int tl_starttype;           /* start at graphmin or use tl_start/stop */
    int tl_stoptype;            /* start at graphmax or use tl_start/stop */
    double tl_start;            /* value of x to begin tick labels and major ticks */
    double tl_stop;             /* value of x to end tick labels and major ticks */

    PlacementType tl_op;        /* tick labels on opposite side or both */

    int tl_gaptype;             /* tick label placement auto or specified */
} tickmarks;


/* Set types */
typedef enum {
    SET_XY        ,
    SET_XYDX      ,
    SET_XYDY      ,
    SET_XYDXDX    ,
    SET_XYDYDY    ,
    SET_XYDXDY    ,
    SET_XYDXDXDYDY,
    SET_BAR       ,
    SET_BARDY     ,
    SET_BARDYDY   ,
    SET_XYHILO    ,
    SET_XYZ       ,
    SET_XYR       ,
    SET_XYSIZE    ,
    SET_XYCOLOR   ,
    SET_XYCOLPAT  ,
    SET_XYVMAP    ,
    SET_BOXPLOT   ,
    SET_BAD
} SetType;
#define NUMBER_OF_SETTYPES  SET_BAD

/* Data column names; */
typedef enum {
    DATA_X ,
    DATA_Y ,
    DATA_Y1,
    DATA_Y2,
    DATA_Y3,
    DATA_Y4,
    DATA_BAD
} DataColumn;
#define MAX_SET_COLS    DATA_BAD


typedef struct {
    int len;                    /* dataset length */
    int ncols;                  /* number of data columns */
    double *ex[MAX_SET_COLS];   /* arrays of x, y, z, ... depending on type */
    char **s;                   /* pointer to strings */

    char *comment;              /* how this dataset originated & alike */

    int hotlink;                /* hot linked set */
    int hotsrc;                 /* source for hot linked file (DISK|PIPE) */
    char *hotfile;              /* hot linked filename */
} Dataset;

typedef struct {
    double ex[MAX_SET_COLS];   /* x, y, dx, z, ... depending on dataset type */
    char *s;                   /* string */
} Datapoint;

typedef struct {
    int type;
    double size;
    Line line;
    Pen fillpen;
    char symchar;
    int charfont;
} Symbol;

typedef struct {
    int type;
    int filltype;
    int fillrule;
    int baseline;
    int baseline_type;
    int droplines;
    Line line;
    Pen fillpen;
} SetLine;

typedef struct {
    double size;
    Line line;
} BarLine;

typedef struct {
    int arrow_clip;
    double clip_length;
    Line line;
} RiserLine;

typedef struct {
    int active;          /* on/off */
    PlacementType ptype; /* placement type */
    Pen pen;             /* pen */
    double linew;        /* error bar line width */
    int lines;           /* error bar line style */
    double riser_linew;  /* connecting line between error limits line width */
    int riser_lines;     /* connecting line between error limits line style */
    double barsize;      /* size of error bar */
    int arrow_clip;      /* draw arrows if clipped */
    double cliplen;      /* riser clipped length (v.p.) */
} Errbar;

/* Annotative strings for data values */
typedef struct {
    int active;                 /* active or not */

    TextProps tprops;
    VVector   offset;           /* offset */
    
    int type;                   /* type */
    int format;                 /* format */
    int prec;                   /* precision */

    char prestr[64];            /* prepend string */
    char appstr[64];            /* append string */
} AValue;

typedef struct {
    int active;                 /* on/off */

    int type;                   /* set type */

    Symbol sym;                 /* set plot symbol props */

    int symskip;                /* number of symbols to skip */

    SetLine line;               /* set line type */

    AValue avalue;              /* Parameters for annotative string */
    Errbar errbar;              /* error bar properties */

    char *legstr;               /* legend for this set */
    
    Dataset *data;              /* dataset */
} set;


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

/* A 2D point */
typedef struct {
    double x;
    double y;
} APoint;

typedef struct {
    int active;                 /* region on or off */
    
    int type;                   /* region type */
    
    int n;                      /* number of points */
    WPoint *wps;                /* coordinates of points */

    int color;
} region;

/* Object types */
typedef enum {
    DO_NONE,
    DO_LINE,
    DO_BOX,
    DO_ARC
} OType;

typedef struct _DObject {
    int active;

    APoint ap;
    
    VPoint offset;
    double angle;
    
    Line line;
    Pen fillpen;

    OType type;     /* object type */
    void *odata;
    
    view bb;        /* BBox (calculated at run-time) */
} DObject;

typedef struct {
    int type;
    double length;  /* head length (L) */
    double dL_ff;   /* d/L form factor */
    double lL_ff;   /* l/L form factor */
} Arrow;

typedef struct _DOLineData {
    VVector vector;
    int arrow_end;
    Arrow arrow;
} DOLineData;

typedef struct _DOBoxData {
    double width;
    double height;
} DOBoxData;

typedef struct _DOArcData {
    double width;
    double height;
    
    double angle1;
    double angle2;
    
    int fillmode;
} DOArcData;

typedef struct _AText {
    int active;

    APoint ap;
    VVector offset;
    
    char *s;
    TextProps text_props;

    int frame_decor;
    double frame_offset;
    Line line;
    Pen fillpen;
    int arrow_flag;
    Arrow arrow;
    
    view bb;
} AText;


QuarkFactory *qfactory_new(void);
void qfactory_free(QuarkFactory *qfactory);
int quark_flavor_add(QuarkFactory *qfactory, const QuarkFlavor *qf);
QuarkFlavor *quark_flavor_get(const QuarkFactory *qfactory, unsigned int fid);
int quark_factory_set_udata(QuarkFactory *qfactory, void *udata);
void *quark_factory_get_udata(const QuarkFactory *qfactory);

Quark *quark_root(QuarkFactory *qfactory, unsigned int fid);
Quark *quark_new(Quark *parent, unsigned int fid);
void quark_free(Quark *q);
Quark *quark_copy(const Quark *q);
Quark *quark_copy2(Quark *newparent, const Quark *q);

Quark *quark_parent_get(const Quark *q);

void quark_dirtystate_set(Quark *q, int flag);
int quark_dirtystate_get(const Quark *q);

void quark_idstr_set(Quark *q, const char *s);
char *quark_idstr_get(const Quark *q);
Quark *quark_find_child_by_idstr(Quark *q, const char *s);
Quark *quark_find_descendant_by_idstr(Quark *q, const char *s);

int quark_cb_set(Quark *q, Quark_cb cb, void *cbdata);
void quark_traverse(Quark *q, Quark_traverse_hook hook, void *udata);

int quark_count_children(const Quark *q);
int quark_child_exist(const Quark *parent, const Quark *child);
int quark_reparent(Quark *q, Quark *newparent);
int quark_reparent_children(Quark *parent, Quark *newparent);

int quark_move(const Quark *q, int forward);
int quark_push(const Quark *q, int forward);

int quark_sort_children(Quark *q, Quark_comp_proc fcomp, void *udata);

int quark_set_udata(Quark *q, void *udata);
void *quark_get_udata(const Quark *q);

/* Project */
Project *project_data_new(void);
void project_data_free(Project *pr);
void project_free(Quark *q);

Project *project_get_data(const Quark *q);

int project_get_version_id(const Quark *q);
int project_set_version_id(Quark *q, int version_id);
char *project_get_description(const Quark *q);
int project_set_description(Quark *q, char *descr);
char *project_get_sformat(const Quark *q);
int project_set_sformat(Quark *q, const char *s);
char *project_get_docname(const Quark *q);
int project_set_docname(Quark *q, char *s);

int project_set_fontsize_scale(Quark *q, double fscale);
int project_set_linewidth_scale(Quark *q, double lscale);

int project_set_ref_date(Quark *q, double ref);
int project_allow_two_digits_years(Quark *q, int flag);

char *project_get_timestamp(Quark *q);
int project_update_timestamp(Quark *q, time_t *t);

int project_add_font(Quark *project, const Fontdef *f);

Quark *get_parent_project(const Quark *q);

/* Frame */
frame *frame_data_new(void);
void frame_data_free(frame *f);
frame *frame_data_copy(frame *f);
frame *frame_get_data(const Quark *q);

Quark *frame_new(Quark *project);

int frame_is_active(const Quark *q);
int frame_set_active(Quark *q, int flag);

int frame_get_view(const Quark *q, view *v);
int frame_set_view(Quark *q, const view *v);

int frame_set_type(Quark *q, int type);
legend *frame_get_legend(const Quark *gr);
int frame_set_legend(Quark *q, const legend *l);
int frame_set_outline(Quark *q, const Line *line);
int frame_set_fillpen(Quark *q, const Pen *pen);

Quark *get_parent_frame(const Quark *q);

int frame_shift(Quark *q, const VVector *vshift);
int frame_legend_shift(Quark *q, const VVector *vshift);

/* Graph */
graph *graph_data_new(void);
void graph_data_free(graph *g);
graph *graph_data_copy(graph *g);

graph *graph_get_data(const Quark *q);

Quark *graph_new(Quark *q);

int graph_get_type(const Quark *gr);
int graph_set_type(Quark *gr, int gtype);
GLocator *graph_get_locator(const Quark *gr);
int graph_set_locator(Quark *gr, const GLocator *locator);
int graph_get_world(const Quark *gr, world *w);
int graph_set_world(Quark *gr, const world *w);
int graph_get_xyflip(const Quark *gr);
int graph_set_xyflip(Quark *gr, int xyflip);
int graph_is_active(const Quark *gr);
int graph_set_active(Quark *gr, int flag);
int graph_is_stacked(const Quark *gr);
int graph_set_stacked(Quark *gr, int flag);
double graph_get_bargap(const Quark *gr);
int graph_set_bargap(Quark *gr, double bargap);
int graph_get_xscale(const Quark *gr);
int graph_set_xscale(Quark *gr, int scale);
int graph_get_yscale(const Quark *gr);
int graph_set_yscale(Quark *gr, int scale);
double graph_get_znorm(const Quark *gr);
int graph_set_znorm(Quark *gr, double norm);
int graph_is_xinvert(const Quark *gr);
int graph_set_xinvert(Quark *gr, int flag);
int graph_is_yinvert(const Quark *gr);
int graph_set_yinvert(Quark *gr, int flag);

Quark *get_parent_graph(const Quark *child);

/* Axis */
tickmarks *axis_data_new(void);
tickmarks *axis_data_copy(tickmarks *t);
void axis_data_free(tickmarks *t);

Quark *axis_new(Quark *q);

tickmarks *axis_get_data(const Quark *q);

int axis_set_type(Quark *q, int type);
int axis_set_active(Quark *q, int flag);
int axis_is_x(const Quark *q);
int axis_is_y(const Quark *q);
void axis_autotick(Quark *q);

/* Set */
double *copy_data_column(double *src, int nrows);
char **copy_string_column(char **src, int nrows);

int settype_cols(int type);

Dataset *dataset_new(void);
int dataset_empty(Dataset *dsp);
void dataset_free(Dataset *dsp);
Dataset *dataset_copy(Dataset *data);

int dataset_set_nrows(Dataset *data, int len);
int dataset_set_ncols(Dataset *data, int ncols);
int dataset_enable_scol(Dataset *data, int yesno);

set *set_data_new(void);
void set_data_free(set *p);
set *set_data_copy(set *p);
Quark *set_new(Quark *gr);

set *set_get_data(const Quark *q);

int set_is_active(Quark *pset);
int set_set_active(Quark *pset, int flag);

Dataset *set_get_dataset(Quark *pset);
int set_set_dataset(Quark *q, Dataset *dsp);

char *set_get_legstr(Quark *pset);
int set_set_legstr(Quark *pset, const char *s);

int set_get_ncols(Quark *pset);

int set_set_symskip(Quark *pset, int symskip);
int set_set_symbol(Quark *pset, const Symbol *sym);
int set_set_line(Quark *pset, const SetLine *sl);
int set_set_avalue(Quark *pset, const AValue *av);
int set_set_errbar(Quark *pset, const Errbar *ebar);

void set_set_hotlink(Quark *pset, int onoroff, char *fname, int src);
int set_is_hotlinked(Quark *pset);
char *set_get_hotlink_file(Quark *pset);
int set_get_hotlink_src(Quark *pset);

int set_set_strings(Quark *p, int len, char **s);
char **set_get_strings(Quark *p);

int set_set_length(Quark *p, unsigned int length);
int set_get_length(Quark *p);

double *set_get_col(Quark *p, unsigned int col);
void set_set_col(Quark *pset, unsigned int col, double *x, unsigned int len);

int set_get_type(Quark *p);
int set_set_type(Quark *p, int stype);

char *set_get_comment(Quark *p);
int set_set_comment(Quark *p, char *s);


/* Region */
region *region_data_new(void);
void region_data_free(region *r);
region *region_data_copy(region *r);

region *region_get_data(const Quark *q);

Quark *region_new(Quark *gr);

int region_set_active(Quark *q, int flag);
int region_set_type(Quark *q, int type);
int region_set_color(Quark *q, int color);

int region_add_point(Quark *q, const WPoint *wp);


/* DObject */
void *object_odata_new(OType type);

DObject *object_data_new(void);
void object_data_free(DObject *o);
DObject *object_data_copy(DObject *o);

DObject *object_data_new_complete(OType type);

Quark *object_new(Quark *gr);
Quark *object_new_complete(Quark *gr, OType type);

DObject *object_get_data(const Quark *q);

int object_set_active(Quark *q, int flag);
int object_set_angle(Quark *q, double angle);
int object_set_offset(Quark *q, const VPoint *offset);
int object_set_line(Quark *q, const Line *line);
int object_set_fillpen(Quark *q, const Pen *pen);
int object_set_location(Quark *q, const APoint *ap);

int object_get_bb(DObject *o, view *bb);

int object_shift(Quark *q, const VVector *vshift);

/* AText */
TextProps *textprops_new(void);
void set_default_textprops(TextProps *pstr, const defaults *grdefs);

AText *atext_data_new(void);
void atext_data_free(AText *at);
AText *atext_data_copy(AText *at);
Quark *atext_new(Quark *q);
AText *atext_get_data(const Quark *q);
int atext_set_active(Quark *q, int flag);
int atext_set_string(Quark *q, const char *s);
int atext_set_ap(Quark *q, const APoint *ap);
int atext_set_offset(Quark *q, const VPoint *offset);
int atext_set_tprops(Quark *q, const TextProps *tprops);
int atext_set_font(Quark *q, int font);
int atext_set_char_size(Quark *q, double size);
int atext_set_color(Quark *q, int color);
int atext_set_just(Quark *q, int just);
int atext_set_angle(Quark *q, double angle);

int atext_shift(Quark *q, const VVector *vshift);

/* co-ordinate transformation stuff */
int polar2xy(double phi, double rho, double *x, double *y);
void xy2polar(double x, double y, double *phi, double *rho);

double fscale(double wc, int scale);
double ifscale(double vc, int scale);

int is_validWPoint(const Quark *q, const WPoint *wp);

double xy_xconv(const Quark *q, double wx);
double xy_yconv(const Quark *q, double wy);

int Wpoint2Vpoint(const Quark *q, const WPoint *wp, VPoint *vp);
int Vpoint2Wpoint(const Quark *q, const VPoint *vp, WPoint *wp);

int Fpoint2Vpoint(const Quark *f, const FPoint *fp, VPoint *vp);

int update_graph_ccache(Quark *gr);

#endif /* __CORE_H_ */
