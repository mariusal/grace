/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
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

#ifndef __GRAPHS_H_
#define __GRAPHS_H_

#include "grace.h"
#include "ctrans.h"

#define CORNER_LL   0
#define CORNER_UL   1
#define CORNER_UR   2
#define CORNER_LR   3

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
    Dataset *data;              /* dataset */
    
    int hidden;                 /* hidden set */

    int type;                   /* set type */

    Symbol sym;                 /* set plot symbol props */

    int symskip;                /* number of symbols to skip */

    SetLine line;               /* set line type */

    AValue avalue;              /* Parameters for annotative string */
    Errbar errbar;              /* error bar properties */

    char *legstr;               /* legend for this set */
} set;

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
    int type;                   /* frame type */
    Pen pen;                    /* frame pen */
    int lines;                  /* frame linestyle */
    double linew;                  /* frame line width */
    Pen fillpen;                /* fill pen */
} framep;

typedef struct {
    int type;
    double wtpos;
    char *label;
} tickloc;

typedef struct {
    double size;              /* length of tickmarks */
    int color;                /* color of tickmarks */
    double linew;             /* linewidth of tickmarks */
    int lines;                /* linestyle of tickmarks */
    int gridflag;             /* grid lines at tick marks */
} tickprops;

typedef struct {
    int active;                 /* active or not */

    int zero;                   /* "zero" axis or plain */

    plotstr label;              /* graph axis label */
    int label_layout;           /* axis label orientation (h or v) */
    int label_place;            /* axis label placement (specfied or auto) */
    PlacementType label_op;     /* tick labels on opposite side or both */

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

    int t_inout;                /* ticks inward, outward or both */
    PlacementType t_op;         /* ticks on opposite side */
    
    tickprops props;
    tickprops mprops;

    int tl_flag;                /* toggle ticmark labels on or off */
    int tl_angle;               /* angle to draw labels */

    int tl_format;              /* tickmark label format */
    int tl_prec;                /* places to right of decimal point */

    char *tl_formula;           /* transformation formula */

    int tl_skip;                /* tick labels to skip */
    int tl_staggered;           /* tick labels staggered */
    int tl_starttype;           /* start at graphmin or use tl_start/stop */
    int tl_stoptype;            /* start at graphmax or use tl_start/stop */
    double tl_start;            /* value of x to begin tick labels and major ticks */
    double tl_stop;             /* value of x to end tick labels and major ticks */

    PlacementType tl_op;        /* tick labels on opposite side or both */

    int tl_gaptype;             /* tick label placement auto or specified */
    VVector tl_gap;             /* tick label to tickmark distance
				   (parallel and perpendicular to axis) */

    int tl_font;                /* font to use for tick labels */
    double tl_charsize;         /* character size for tick labels */
    int tl_color;               /* color of tick labels */

    char tl_appstr[64];         /* append string to tick label */
    char tl_prestr[64];         /* prepend string to tick label */

} tickmarks;

/*
 * a graph
 */
typedef struct {
    int hidden;                 /* display or not */

    int type;                   /* type of graph */

    int xscale;                 /* scale mapping of X axes*/
    int yscale;                 /* scale mapping of Y axes*/
    int xinvert;                /* X axes inverted, TRUE or FALSE */
    int yinvert;                /* Y axes inverted, TRUE or FALSE */
    int xyflip;                 /* whether x and y axes should be flipped */

    int stacked;                /* TRUE if graph is stacked */
    double bargap;              /* Distance between bars (in bar charts) */
    double znorm;               /* Normalization of pseudo-3D graphs */

    legend l;                   /* legends */

    world w;                    /* world */
    view v;                     /* viewport */

    labels labs;                /* title and subtitle */

    tickmarks *t[MAXAXES];      /* flags etc. for tickmarks for all axes */

    framep f;                   /* type of box around plot */

    GLocator locator;           /* locator props */

    world_stack ws[MAX_ZOOM_STACK]; /* zoom stack */
    int ws_top;                 /* stack pointer */
    int curw;                   /* for cycling through the stack */
} graph;

Symbol *symbol_new();
void symbol_free(Symbol *sym);
SetLine *setline_new();
void setline_free(SetLine *sl);
BarLine *barline_new(void);
RiserLine *riserline_new(void);

Quark *graph_new(Quark *project);
Quark *graph_next(Quark *project);

Quark *graph_get_current(const Quark *project);

graph *graph_get_data(Quark *gr);

graph *graph_data_new(void);
void graph_data_free(graph *g);
graph *graph_data_copy(graph *g);

void kill_all_graphs(Quark *project);
Quark *duplicate_graph(Quark *gr);

tickmarks *new_graph_tickmarks(void);
tickmarks *copy_graph_tickmarks(tickmarks *);
tickmarks *get_graph_tickmarks(const Quark *gr, int axis);
void free_graph_tickmarks(tickmarks *t);
int set_graph_tickmarks(Quark *gr, int a, tickmarks *t);
int is_axis_active(tickmarks *t);
int is_zero_axis(tickmarks *t);
int activate_tick_labels(tickmarks *t, int flag);

int get_graph_world(Quark *gr, world *w);
int get_graph_viewport(Quark *gr, view *v);

framep *get_graph_frame(Quark *gr);
labels *get_graph_labels(Quark *gr);
legend *get_graph_legend(Quark *gr);
GLocator *get_graph_locator(Quark *gr);

void set_graph_frame(Quark *gr, const framep *f);
void set_graph_world(Quark *gr, const world *w);
void set_graph_viewport(Quark *gr, const view *v);
void set_graph_title(Quark *gr, const plotstr *s);
void set_graph_stitle(Quark *gr, const plotstr *s);
void set_graph_labels(Quark *gr, const labels *labs);
void set_graph_legend(Quark *gr, const legend *leg);
void set_graph_locator(Quark *gr, const GLocator *locator);

int get_graph_xyflip(Quark *gr);
void set_graph_xyflip(Quark *gr, int xyflip);

void set_graph_legend_active(Quark *gr, int flag);

#define is_graph_active(gr) is_valid_gno(gr)

int is_graph_hidden(Quark *gr);
int set_graph_hidden(Quark *gr, int flag);

int get_graph_type(Quark *gr);

int is_graph_stacked(Quark *gr);
int set_graph_stacked(Quark *gr, int flag);

double get_graph_bargap(Quark *gr);
int set_graph_bargap(Quark *gr, double bargap);

int islogx(Quark *gr);
int islogy(Quark *gr);

int islogitx(Quark *gr);
int islogity(Quark *gr);

int is_log_axis(Quark *gr, int axis);
int is_logit_axis(Quark *gr, int axis);

int number_of_graphs(const Quark *project);
int select_graph(Quark *g);

int realloc_graphs(Quark *project, int n);
int realloc_graph_plots(Quark *gr, int n);

int set_graph_xscale(Quark *gr, int scale);
int set_graph_yscale(Quark *gr, int scale);

int get_graph_xscale(Quark *gr);
int get_graph_yscale(Quark *gr);

int set_graph_znorm(Quark *gr, double norm);
double get_graph_znorm(Quark *gr);

int is_valid_gno(Quark *gr);

int set_graph_type(Quark *gr, int gtype);

int is_set_active(Quark *pset);
int is_set_hidden(Quark *pset);
int set_set_hidden(Quark *pset, int flag);

int set_set_symskip(Quark *pset, int flag);
int set_set_symbol(Quark *pset, const Symbol *sym);
int set_set_line(Quark *pset, const SetLine *sl);
int set_set_avalue(Quark *pset, const AValue *av);
int set_set_errbar(Quark *pset, const Errbar *ebar);
int set_set_legstr(Quark *pset, const char *s);

#define is_set_drawable(p) (is_set_active(p) && !is_set_hidden(p))

int number_of_sets(Quark *gr);

set *set_get_data(Quark *p);
Dataset *dataset_get(Quark *p);

int load_comments_to_legend(Quark *p);

int settype_cols(int type);
int dataset_type(Quark *p);
int dataset_cols(Quark *p);
char *dataset_colname(int col);

int is_refpoint_active(Quark *gr);

int set_refpoint(Quark *gr, const WPoint *wp);

WPoint get_refpoint(Quark *gr);

double *getcol(Quark *p, int col);
#define getx(p) getcol(p, 0)
#define gety(p) getcol(p, 1)

char *get_legend_string(Quark *p);
int set_legend_string(Quark *p, char *s);

int set_dataset_type(Quark *p, int stype);

char *getcomment(Quark *p);
int setcomment(Quark *p, char *s);

int set_set_strings(Quark *p, int len, char **s);
char **get_set_strings(Quark *p);

int setlength(Quark *p, int length);
int getsetlength(Quark *p);

double setybase(Quark *p);

int is_graph_xinvert(Quark *gr);
int is_graph_yinvert(Quark *gr);

int set_graph_xinvert(Quark *gr, int flag);
int set_graph_yinvert(Quark *gr, int flag);

void cycle_world_stack(Quark *pr);
void clear_world_stack(Quark *pr);
void show_world_stack(Quark *pr, int n);
void add_world(Quark *gr, double x1, double x2, double y1, double y2);
void push_world(Quark *pr);
void pop_world(Quark *pr);
int graph_world_stack_size(Quark *gr);
int get_world_stack_current(Quark *gr);
int get_world_stack_entry(Quark *gr, int n, world_stack *ws);

int get_graph_sets(Quark *gr, Quark ***sets);

int set_set_colors(Quark *p, int color);

int copysetdata(Quark *psrc, Quark *pdest);

void set_data_free(set *s);
set *set_data_copy(set *s);

void project_postprocess(Quark *pr);

#endif /* __GRAPHS_H_ */
