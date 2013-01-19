%{
/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
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

/*  
 * 
 * evaluate expressions, commands, parameter files
 * 
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#if defined(HAVE_SYS_PARAM_H)
#  include <sys/param.h>
#endif

/* bison not always handles it well itself */
#if defined(HAVE_ALLOCA_H)
#  include <alloca.h>
#endif

#include <grace/coreP.h> /* FIXME!!! */

#include "defines.h"
#include "globals.h"
#include "graceapp.h"
#include "utils.h"
#include "files.h"
#include "ssdata.h"
#include "core_utils.h"

/* symbol table entry type */
typedef struct {
    char *s;
    int type;
    void *data;
} symtab_entry;

/* array variable */
typedef struct _grarr {
    int length;
    double *data;
} grarr;



#define MAX_PARS_STRING_LENGTH  4096

#define canvas grace_get_canvas(gapp->grace)

/* Tick sign type (obsolete) */
#define SIGN_NORMAL     0
#define SIGN_ABSOLUTE   1
#define SIGN_NEGATE     2

#define CAST_DBL_TO_BOOL(x) (fabs(x) < 0.5 ? 0:1)

typedef double (*ParserFnc)();

/* the graph, set, axis, and object of the parser's current state */
static Quark *project;
static Quark *whichframe;
static Quark *whichgraph;
static Quark *whichset;

/* target set */
static Quark *target_set;


static Quark *whichaxisgrid;
static Quark *normaxis,  *oppaxis;
static Quark *normlabel, *opplabel;
static tickmarks *curtm;

static DObject *curobject;
static Quark *curatext;
static Quark *objgno;
static int curobject_loctype = COORD_VIEW;
static int dobject_id = 0;

static int expr_parsed, vexpr_parsed;

static int interr;

static char f_string[MAX_PARS_STRING_LENGTH]; /* buffer for string to parse */
static unsigned int pos;


static int filltype_obs;

static int leg_loctype_obs;
static double leg_x1_obs;

static int set_parser_gno(Quark *g);
static int set_parser_setno(Quark *pset);

static int getcharstr(void);
static void ungetchstr(void);

static int findf(symtab_entry *keytable, char *s);

static void add_xmgr_fonts(Quark *project);
static void add_xmgr_colors(Quark *project);

static Quark *allocate_graph(Quark *project, int gno);
static Quark *allocate_set(Quark *gr, int setno);
static Quark *allocate_region(Quark *gr, int rn);

static int yylex(void);
static int yyparse(void);
static void yyerror(char *s);

%}

%union {
    int     ival;
    double  dval;
    char   *sval;
    double *dptr;
    Format *fmt;
    Quark  *quark;
    grarr  *vrbl;
}

%token <ival> DATE

%token <ival> ABOVE
%token <ival> ABSOLUTE
%token <ival> ALT
%token <ival> ALTXAXIS
%token <ival> ALTYAXIS
%token <ival> ANGLE
%token <ival> APPEND
%token <ival> ARROW
%token <ival> AUTO
%token <ival> AUTOSCALE
%token <ival> AVALUE
%token <ival> AVG
%token <ival> BACKGROUND
%token <ival> BAR
%token <ival> BARDY
%token <ival> BARDYDY
%token <ival> BASELINE
%token <ival> BEGIN
%token <ival> BELOW
%token <ival> BETWEEN
%token <ival> BOTH
%token <ival> BOTTOM
%token <ival> BOX
%token <ival> CENTER
%token <ival> CHAR
%token <ival> CHART
%token <sval> CHRSTR
%token <ival> CLIP
%token <ival> COLOR
%token <ival> COMMENT
%token <ival> COMPUTING
%token <ival> DAYMONTH
%token <ival> DAYOFWEEKL
%token <ival> DAYOFWEEKS
%token <ival> DAYOFYEAR
%token <ival> DDMMYY
%token <ival> DECIMAL
%token <ival> DEF
%token <ival> DEFAULT
%token <ival> DEGREESLAT
%token <ival> DEGREESLON
%token <ival> DEGREESMMLAT
%token <ival> DEGREESMMLON
%token <ival> DEGREESMMSSLAT
%token <ival> DEGREESMMSSLON
%token <ival> DESCRIPTION
%token <ival> DEVICE
%token <ival> DISK
%token <ival> DROPLINE
%token <ival> ELLIPSE
%token <ival> ENGINEERING
%token <ival> ERRORBAR
%token <ival> EXPONENTIAL
%token <ival> FILL
%token <ival> FIXED
%token <ival> FIXEDPOINT
%token <ival> FONTP
%token <ival> FORMAT
%token <ival> FORMULA
%token <ival> FRAMEP
%token <ival> FREE
%token <ival> GENERAL
%token <ival> GRAPH
%token <ival> GRAPHNO
%token <ival> GRID
%token <ival> HARDCOPY
%token <ival> HBAR
%token <ival> HGAP
%token <ival> HIDDEN
%token <ival> HMS
%token <ival> HORIZI
%token <ival> HORIZONTAL
%token <ival> HORIZO
%token <ival> IN
%token <ival> INCREMENT
%token <ival> INOUT
%token <ival> INVERT
%token <ival> JUST
%token <ival> LABEL
%token <ival> LANDSCAPE
%token <ival> LAYOUT
%token <ival> LEFT
%token <ival> LEGEND
%token <ival> LENGTH
%token <ival> LINE
%token <ival> LINEAR
%token <ival> LINESTYLE
%token <ival> LINEWIDTH
%token <ival> LINK
%token <ival> LOCTYPE
%token <ival> LOG
%token <ival> LOGARITHMIC
%token <ival> LOGIT
%token <ival> LOGX
%token <ival> LOGXY
%token <ival> LOGY
%token <ival> MAJOR
%token <ival> MAP
%token <ival> MAXP
%token <ival> MINP
%token <ival> MINOR
%token <ival> MMDD
%token <ival> MMDDHMS
%token <ival> MMDDYY
%token <ival> MMDDYYHMS
%token <ival> MMSSLAT
%token <ival> MMSSLON
%token <ival> MMYY
%token <ival> MONTHDAY
%token <ival> MONTHL
%token <ival> MONTHS
%token <ival> MONTHSY
%token <ival> NEGATE
%token <ival> NONE
%token <ival> NORMAL
%token <ival> OFF
%token <ival> OFFSET
%token <ival> OFFSETX
%token <ival> OFFSETY
%token <ival> ON
%token <ival> OP
%token <ival> OPPOSITE
%token <ival> OUT
%token <ival> PAGE
%token <ival> PARA
%token <ival> PATTERN
%token <ival> PERP
%token <ival> PIE
%token <ival> PIPE
%token <ival> PLACE
%token <ival> POLAR
%token <ival> POLYI
%token <ival> POLYO
%token <ival> PORTRAIT
%token <ival> POWER
%token <ival> PREC
%token <ival> PREPEND
%token <ival> PS
%token <ival> RECIPROCAL
%token <ival> REFERENCE
%token <ival> REGNUM
%token <ival> RIGHT
%token <ival> RISER
%token <ival> ROT
%token <ival> ROUNDED
%token <ival> RULE
%token <ival> SCALE
%token <ival> SCIENTIFIC
%token <ival> SCROLL
%token <ival> SD
%token <ival> SET
%token <ival> SETNUM
%token <ival> SFORMAT
%token <ival> SIGN
%token <ival> SIZE
%token <ival> SKIP
%token <ival> SMITH 
%token <ival> SOURCE
%token <ival> SPEC
%token <ival> STACK
%token <ival> STACKED
%token <ival> STACKEDBAR
%token <ival> STACKEDHBAR
%token <ival> STAGGER
%token <ival> START
%token <ival> STOP
%token <ival> STRING
%token <ival> SUBTITLE
%token <ival> SYMBOL
%token <ival> TARGET
%token <ival> TICKLABEL
%token <ival> TICKP
%token <ival> TICKSP
%token <ival> TIMESTAMP
%token <ival> TITLE
%token <ival> TO
%token <ival> TOP
%token <ival> TYPE
%token <ival> UP
%token <ival> VERSION
%token <ival> VERTI
%token <ival> VERTICAL
%token <ival> VERTO
%token <ival> VGAP
%token <ival> VIEW
%token <ival> WITH
%token <ival> WORLD
%token <ival> WRAP
%token <ival> X1
%token <ival> XAXES
%token <ival> XAXIS
%token <ival> XMAX
%token <ival> XMIN
%token <ival> XY
%token <ival> XYBOXPLOT
%token <ival> XYCOLOR
%token <ival> XYCOLPAT
%token <ival> XYDX
%token <ival> XYDXDX
%token <ival> XYDXDXDYDY
%token <ival> XYDXDY
%token <ival> XYDY
%token <ival> XYDYDY
%token <ival> XYHILO
%token <ival> XYR
%token <ival> XYSIZE
%token <ival> XYSTRING
%token <ival> XYVMAP
%token <ival> XYZ
%token <ival> Y1
%token <ival> YAXES
%token <ival> YAXIS
%token <ival> YEAR
%token <ival> YMAX
%token <ival> YMIN
%token <ival> YYMMDD
%token <ival> YYMMDDHMS
%token <ival> ZERO
%token <ival> ZNORM

%token <dval> NUMBER

%type <ival> onoff

%type <quark> selectgraph
%type <quark> selectset
%type <quark> selectregion
%type <quark> title
%type <quark> atext

%type <ival> pagelayout
%type <ival> pageorient

%type <ival> regiontype

%type <ival> color_select
%type <ival> pattern_select
%type <ival> font_select

%type <ival> lines_select
%type <dval> linew_select

%type <ival> graphtype
%type <ival> xytype

%type <ival> scaletype
%type <ival> signchoice

%type <ival> colpat_obs

%type <fmt>  formatchoice
%type <ival> inoutchoice
%type <ival> justchoice

%type <ival> opchoice
%type <ival> opchoice_sel
%type <ival> opchoice_obs
%type <ival> opchoice_sel_obs

%type <ival> worldview

%type <ival> tickspectype

%type <ival> sourcetype

%type <ival> objecttype

%type <dval> expr
%type <ival> iexpr
%type <ival> nexpr

%%

list:
	parmset {}
	| parmset_obs {}
	| regionset {}
	| setaxis {}
	| set_setprop {}
	| actions {}
	| options {}
	| error {
	    return 1;
	}
	;


expr:	NUMBER {
            $$ = $1;
	}
        | '-' NUMBER {
            $$ = -$2;
	}
        ;

iexpr:  expr {
	    int itmp = rint($1);
            if (fabs(itmp - $1) > 1.e-6) {
		yyerror("Non-integer value supplied for integer");
		return 1;
            }
            $$ = itmp;
        }
        ;

nexpr:	iexpr {
            if ($1 < 0) {
		yyerror("Negative value supplied for non-negative");
		return 1;
            }
            $$ = $1;
	}
        ;

regionset:
	selectregion onoff {
            quark_set_active($1, $2);
	}
	| selectregion TYPE regiontype {
	    region_set_type($1, $3);
	}
	| selectregion color_select {
	    region_set_color($1, $2);
        }
	| selectregion lines_select {
	}
	| selectregion linew_select {
	}
	| selectregion LINE expr ',' expr ',' expr ',' expr
	{
	    WPoint wp;
            wp.x = $3;
            wp.y = $5;
            region_add_point($1, &wp);
            wp.x = $7;
            wp.y = $9;
	    region_add_point($1, &wp);
	}
	| selectregion XY expr ',' expr
	{
	    WPoint wp;
            wp.x = $3;
            wp.y = $5;
	    region_add_point($1, &wp);
	}
	| LINK selectregion TO selectgraph {
	    quark_reparent($2, $4);
	}
	;


parmset:
        VERSION nexpr {
            if (project_set_version_id(project, $2) != RETURN_SUCCESS) {
                errmsg("Project version is newer than software!");
            }
            if (project_get_version_id(project) < 50001) {
                add_xmgr_fonts(project);
            }
            add_xmgr_colors(project);
            dobject_id = 0;
        }
        | PAGE SIZE nexpr ',' nexpr {
            project_set_page_dimensions(project, $3, $5);
        }
        | REFERENCE DATE expr {
            project_set_ref_date(project, $3);
	}
        | DATE WRAP onoff {
            project_allow_two_digits_years(project, $3);
	}
        | DATE WRAP YEAR iexpr {
            project_set_wrap_year(project, $4);
	}
	| BACKGROUND color_select {
	    Project *pr = project_get_data(project);
            pr->bgcolor = $2;
	}
	| PAGE BACKGROUND FILL onoff {
	    Project *pr = project_get_data(project);
	    pr->bgfill = $4;
	}
	| PAGE SCROLL expr '%' {
	    gapp->rt->scrollper = $3 / 100.0;
	}
	| PAGE INOUT expr '%' {
	    gapp->rt->shexper = $3 / 100.0;
	}

	| LINK PAGE onoff {
	}

	| TARGET selectset {
	    target_set = $2;
	    set_parser_setno($2);
	}
	| WITH selectgraph {
	    set_parser_gno($2);
	}
	| WITH selectset {
	    set_parser_setno($2);
	}

/* Hot links */
	| selectset LINK sourcetype CHRSTR {
	    AMem *amem = quark_get_amem($1);
	    ss_hotlink *hotlink = ssd_get_hotlink(get_parent_ssd($1));
            hotlink->is_pipe = ($3 == PIPE);
            hotlink->src = amem_strdup(amem, $4);
	    xfree($4);
	}
	| selectset LINK onoff {
	    ss_hotlink *hotlink = ssd_get_hotlink(get_parent_ssd($1));
            hotlink->active = $3;
	}

/* Objects */
	| WITH objecttype {
            curobject = object_data_new_complete(quark_get_amem(project), $2);
	}
	| objecttype onoff {
	    /* was never off */
        }
	| LINE selectgraph {
	    objgno = $2;
	}
	| BOX selectgraph {
	    objgno = $2;
	}
	| ELLIPSE selectgraph {
	    objgno = $2;
	}
	| objecttype LOCTYPE worldview {
	    if (!curobject) {
                yyerror("No active object");
	    } else {
	        curobject_loctype = $3;
            }
	}
	| objecttype lines_select {
	    if (!curobject) {
                yyerror("No active object");
	    } else {
	        curobject->line.style = $2;
            }
	}
	| objecttype linew_select {
	    if (!curobject) {
                yyerror("No active object");
	    } else {
	        curobject->line.width = $2;
            }
	}
	| objecttype color_select {
	    if (!curobject) {
                yyerror("No active object");
	    } else {
	        curobject->line.pen.color = $2;
            }
	}
	| objecttype FILL color_select {
	    if (!curobject) {
                yyerror("No active object");
	    } else {
	        curobject->fillpen.color = $3;
            }
	}
	| objecttype FILL pattern_select {
	    if (!curobject) {
                yyerror("No active object");
	    } else {
	        curobject->fillpen.pattern = $3;
            }
	}
	| objecttype ROT expr {
	    if (!curobject) {
                yyerror("No active object");
	    } else {
                curobject->angle = $3;
            }
        }
	| LINE expr ',' expr ',' expr ',' expr {
	    if (!curobject) {
                yyerror("No active object");
	    } else if (curobject->type != DO_LINE) {
                yyerror("The object is not a line");
	    } else {
	        DOLineData *l = (DOLineData *) curobject->odata;
                curobject->ap.x = $2;
                curobject->ap.y = $4;
                l->vector.x = $6 - $2;
                l->vector.y = $8 - $4;
            }
	}
	| LINE ARROW nexpr {
	    if (!curobject) {
                yyerror("No active object");
	    } else if (curobject->type != DO_LINE) {
                yyerror("The object is not a line");
	    } else {
	        DOLineData *l = (DOLineData *) curobject->odata;
	        l->arrow_end = $3;
            }
	}
	| LINE ARROW LENGTH expr {
	    if (!curobject) {
                yyerror("No active object");
	    } else if (curobject->type != DO_LINE) {
                yyerror("The object is not a line");
	    } else {
	        DOLineData *l = (DOLineData *) curobject->odata;
	        l->arrow.length = $4;
            }
	}
	| LINE ARROW TYPE nexpr {
	    if (!curobject) {
                yyerror("No active object");
	    } else if (curobject->type != DO_LINE) {
                yyerror("The object is not a line");
	    } else {
	        DOLineData *l = (DOLineData *) curobject->odata;
	        switch ($4) {
                case 0:
                    l->arrow.type = ARROW_TYPE_LINE;
                    break;
                case 1:
                    l->arrow.type = ARROW_TYPE_FILLED;
                    curobject->fillpen.pattern = 1;
                    curobject->fillpen.color = curobject->line.pen.color;
                    break;
                case 2:
                    l->arrow.type = ARROW_TYPE_FILLED;
                    curobject->fillpen.pattern = 1;
                    curobject->fillpen.color = getbgcolor(canvas);
                    break;
                }
            }
	}
	| LINE ARROW LAYOUT expr ',' expr {
	    if (!curobject) {
                yyerror("No active object");
	    } else if (curobject->type != DO_LINE) {
                yyerror("The object is not a line");
	    } else {
	        DOLineData *l = (DOLineData *) curobject->odata;
	        l->arrow.dL_ff = $4;
	        l->arrow.lL_ff = $6;
            }
	}
	| BOX expr ',' expr ',' expr ',' expr {
	    if (!curobject) {
                yyerror("No active object");
	    } else if (curobject->type != DO_BOX) {
                yyerror("The object is not a box");
            } else {
	        DOBoxData *b = (DOBoxData *) curobject->odata;
                b->width  = fabs($6 - $2);
                b->height = fabs($8 - $4);
                curobject->ap.x = ($6 + $2)/2;
                curobject->ap.y = ($8 + $4)/2;
            }
	}
	| ELLIPSE expr ',' expr ',' expr ',' expr {
	    if (!curobject) {
                yyerror("No active object");
	    } else if (curobject->type != DO_ARC) {
                yyerror("The object is not an arc");
	    } else {
	        DOArcData *a = (DOArcData *) curobject->odata;
                a->width  = fabs($6 - $2);
                a->height = fabs($8 - $4);
                a->angle1 =   0.0;
                a->angle2 = 360.0;
                a->closure_type = ARCCLOSURE_CHORD;
                curobject->ap.x = ($6 + $2)/2;
                curobject->ap.y = ($8 + $4)/2;
            }
	}
	| objecttype DEF {
            if (!curobject) {
                yyerror("No active object");
	    } else {
                Quark *q = NULL;
                if (curobject_loctype == COORD_VIEW) {
                    q = object_new(project);
                } else {
                    Quark *gr;
                    gr = objgno;
                    if (!gr) {
                        gr = allocate_graph(project, 0);
                    }
                    if (gr) {
                        q = object_new(gr);
                    }
                }
                if (q) {
                    char buf[16];
                    object_data_free(quark_get_amem(q), object_get_data(q));
                    q->data = curobject;
	            sprintf(buf, "DO%02d", dobject_id);
                    quark_idstr_set(q, buf);
                    dobject_id++;
                }
            }
        }

/* timestamp and string */
	| WITH STRING {
            char buf[16];
            curatext = atext_new(project);
	    sprintf(buf, "DO%02d", dobject_id);
            quark_idstr_set(curatext, buf);
            dobject_id++;
	}
	| atext selectgraph {
	    quark_reparent($1, $2);
	}
	| atext LOCTYPE worldview {
	}
	| atext onoff {
            quark_set_active($1, $2);
        }
	| atext font_select {
            atext_set_font($1, $2);
        }
	| atext CHAR SIZE expr {
            atext_set_char_size($1, $4);
        }
	| atext ROT expr {
            atext_set_angle($1, $3);
        }
	| atext color_select {
            atext_set_color($1, $2);
        }
	| atext expr ',' expr {
            APoint ap;
            ap.x = $2; ap.y = $4;
            atext_set_ap($1, &ap);
	}
	| atext JUST nexpr {
            atext_set_just($1, $3);
        }
	| atext DEF CHRSTR {
	    if (!strings_are_equal("timestamp", quark_idstr_get($1))) {
                atext_set_string($1, $3);
            }
            xfree($3);
	}

/* defaults */
	| DEFAULT lines_select {
            Project *pr = project_get_data(project);
	    pr->grdefaults.line.style = $2;
	}
	| DEFAULT linew_select {
            Project *pr = project_get_data(project);
	    pr->grdefaults.line.width = $2;
	}
	| DEFAULT color_select {
            Project *pr = project_get_data(project);
	    pr->grdefaults.line.pen.color = $2;
	}
	| DEFAULT pattern_select {
            Project *pr = project_get_data(project);
	    pr->grdefaults.line.pen.pattern = $2;
	}
	| DEFAULT CHAR SIZE expr {
            Project *pr = project_get_data(project);
	    pr->grdefaults.charsize = $4;
	}
	| DEFAULT font_select {
            Project *pr = project_get_data(project);
	    pr->grdefaults.font = $2;
	}
	| DEFAULT SYMBOL SIZE expr {
	}
	| DEFAULT SFORMAT CHRSTR {
	    unsigned int prec;
            if (sscanf($3, "%u", &prec) == 1) {
                project_set_prec(project, prec);
            }
	    xfree($3);
	}
	| MAP FONTP nexpr TO CHRSTR ',' CHRSTR {
	    Fontdef f;
            f.id = $3;
            f.fontname = $5;
            f.fallback = $7;
            project_add_font(project, &f);
            xfree($5);
	    xfree($7);
	}
	| MAP COLOR nexpr TO '(' nexpr ',' nexpr ',' nexpr ')' ',' CHRSTR {
	    Colordef c;
            c.id        = $3;
            c.rgb.red   = $6;
            c.rgb.green = $8;
            c.rgb.blue  = $10;
            c.cname     = $13;
            if (project_add_color(project, &c) == RETURN_FAILURE) {
                errmsg("Failed mapping a color");
            }
	    xfree($13);
        }

	| WORLD expr ',' expr ',' expr ',' expr {
	    world w;
            w.xg1 = $2;
	    w.yg1 = $4;
	    w.xg2 = $6;
	    w.yg2 = $8;
            graph_set_world(whichgraph, &w);
	}
	| WORLD XMIN expr {
	    world w;
            graph_get_world(whichgraph, &w);
	    w.xg1 = $3;
            if (w.xg1 >= w.xg2) {
                w.xg2 = w.xg1 + 1.0;
            }
            graph_set_world(whichgraph, &w);
	}
	| WORLD XMAX expr {
	    world w;
            graph_get_world(whichgraph, &w);
	    w.xg2 = $3;
            graph_set_world(whichgraph, &w);
	}
	| WORLD YMIN expr {
	    world w;
            graph_get_world(whichgraph, &w);
	    w.yg1 = $3;
            if (w.yg1 >= w.yg2) {
                w.yg2 = w.yg1 + 1.0;
            }
            graph_set_world(whichgraph, &w);
	}
	| WORLD YMAX expr {
	    world w;
            graph_get_world(whichgraph, &w);
	    w.yg2 = $3;
            graph_set_world(whichgraph, &w);
	}
	| ZNORM expr {
	    graph_set_znorm(whichgraph, $2);
	}
	| VIEW expr ',' expr ',' expr ',' expr {
	    view v;
	    v.xv1 = $2;
	    v.yv1 = $4;
	    v.xv2 = $6;
	    v.yv2 = $8;
            frame_set_view(whichframe, &v);
	}
	| VIEW XMIN expr {
	    view v;
            graph_get_viewport(whichgraph, &v);
	    v.xv1 = $3;
            if (v.xv1 >= v.xv2) {
                v.xv2 = v.xv1 + 0.1;
            }
            frame_set_view(whichframe, &v);
	}
	| VIEW XMAX expr {
	    view v;
            graph_get_viewport(whichgraph, &v);
	    v.xv2 = $3;
            frame_set_view(whichframe, &v);
	}
	| VIEW YMIN expr {
	    view v;
            graph_get_viewport(whichgraph, &v);
	    v.yv1 = $3;
            if (v.yv1 >= v.yv2) {
                v.yv2 = v.yv1 + 0.1;
            }
            frame_set_view(whichframe, &v);
	}
	| VIEW YMAX expr {
	    view v;
            graph_get_viewport(whichgraph, &v);
	    v.yv2 = $3;
            frame_set_view(whichframe, &v);
	}
	
        | title CHRSTR {
            atext_set_string($1, $2);
	    xfree($2);
	}
	| title font_select {
            AText *at = atext_get_data($1);
            if (at) {
                at->text_props.font = $2;
            }
	}
	| title SIZE expr {
            AText *at = atext_get_data($1);
            if (at) {
                at->text_props.charsize = $3;
            }
	}
	| title color_select {
            AText *at = atext_get_data($1);
            if (at) {
                at->text_props.color = $2;
            }
	}

	| XAXES SCALE scaletype {
	    graph_set_xscale(whichgraph, $3);
	}
	| YAXES SCALE scaletype {
	    graph_set_yscale(whichgraph, $3);
	}
	| XAXES INVERT onoff {
	    graph_set_xinvert(whichgraph, $3);
	}
	| YAXES INVERT onoff {
	    graph_set_yinvert(whichgraph, $3);
	}

	| DESCRIPTION CHRSTR {
            char *s;
            s = copy_string(NULL, project_get_description(project));
            s = concat_strings(s, $2);
	    xfree($2);
            s = concat_strings(s, "\n");
            project_set_description(project, s);
            xfree(s);
	}

	| LEGEND onoff {
	    legend *l = frame_get_legend(whichframe);
            l->active = $2;
	}
	| LEGEND LOCTYPE worldview {
	    leg_loctype_obs = $3;
	}
	| LEGEND VGAP nexpr {
	    legend *l = frame_get_legend(whichframe);
            l->vgap = 0.01*$3;
	}
	| LEGEND HGAP nexpr {
	    legend *l = frame_get_legend(whichframe);
	    l->hgap = 0.01*$3;
	}
	| LEGEND LENGTH nexpr {
	    legend *l = frame_get_legend(whichframe);
	    l->len = 0.01*$3;
	}
	| LEGEND INVERT onoff {
	    legend *l = frame_get_legend(whichframe);
	    l->invert = $3;
        }
	| LEGEND BOX FILL color_select {
	    legend *l = frame_get_legend(whichframe);
	    l->boxfillpen.color = $4;
        }
	| LEGEND BOX FILL pattern_select {
	    legend *l = frame_get_legend(whichframe);
	    l->boxfillpen.pattern = $4;
        }
	| LEGEND BOX color_select {
	    legend *l = frame_get_legend(whichframe);
	    l->boxline.pen.color = $3;
	}
	| LEGEND BOX pattern_select {
	    legend *l = frame_get_legend(whichframe);
	    l->boxline.pen.pattern = $3;
	}
	| LEGEND BOX lines_select {
	    legend *l = frame_get_legend(whichframe);
	    l->boxline.style = $3;
	}
	| LEGEND BOX linew_select {
	    legend *l = frame_get_legend(whichframe);
	    l->boxline.width = $3;
	}
	| LEGEND expr ',' expr {
	    legend *l = frame_get_legend(whichframe);
	    VPoint vp;
            view gv;
            if (leg_loctype_obs == COORD_VIEW) {
                vp.x = $2;
                vp.y = $4;
            } else {
                WPoint wp;
                wp.x = $2;
                wp.y = $4; 
                Wpoint2Vpoint(whichgraph, &wp, &vp);
            }
            frame_get_view(whichframe, &gv);
            l->offset.x = vp.x - gv.xv1;
            l->offset.y = vp.y - gv.yv2;

            l->anchor.x = 0.0;
            l->anchor.y = 1.0;
            l->just     = JUST_LEFT | JUST_TOP;
	}
	| LEGEND CHAR SIZE expr {
	    legend *l = frame_get_legend(whichframe);
	    l->charsize = $4;
	}
	| LEGEND font_select {
	    legend *l = frame_get_legend(whichframe);
	    l->font = $2;
	}
	| LEGEND color_select {
	    legend *l = frame_get_legend(whichframe);
	    l->color = $2;
	}

	| FRAMEP onoff {
	    frame *f = frame_get_data(whichframe);
            f->outline.pen.pattern = $2;
	}
	| FRAMEP TYPE nexpr {
	    frame *f = frame_get_data(whichframe);
	    f->type = $3;
	}
	| FRAMEP lines_select {
	    frame *f = frame_get_data(whichframe);
	    f->outline.style = $2;
	}
	| FRAMEP linew_select {
	    frame *f = frame_get_data(whichframe);
	    f->outline.width = $2;
	}
	| FRAMEP color_select {
	    frame *f = frame_get_data(whichframe);
	    f->outline.pen.color = $2;
	}
	| FRAMEP pattern_select {
	    frame *f = frame_get_data(whichframe);
	    f->outline.pen.pattern = $2;
	}
	| FRAMEP BACKGROUND color_select
        { 
	    frame *f = frame_get_data(whichframe);
            f->fillpen.color = $3;
        }
	| FRAMEP BACKGROUND pattern_select
        {
	    frame *f = frame_get_data(whichframe);
            f->fillpen.pattern = $3;
        }

	| selectgraph onoff {
            quark_set_active($1, $2);
            quark_set_active(get_parent_frame($1), $2);
        }
	| selectgraph HIDDEN onoff {
            quark_set_active($1, !$3);
            quark_set_active(get_parent_frame($1), !$3);
        }
	| selectgraph TYPE graphtype {
            graph_set_type($1, $3);
        }
	| selectgraph STACKED onoff {
            graph_set_stacked($1, $3);
        }

	| selectgraph BAR HGAP expr {
	    graph_set_bargap($1, $4);
	}
        
	| selectgraph FIXEDPOINT onoff {
            GLocator *gloc = graph_get_locator($1);
            gloc->pointset = $3;
        }
	| selectgraph FIXEDPOINT FORMAT formatchoice formatchoice {
            GLocator *gloc = graph_get_locator($1);
	    AMem *amem = quark_get_amem($1);
	    gloc->fx.type = $4->type;
            gloc->fx.fstring = amem_strdup(amem, $4->fstring);
	    gloc->fy.type = $5->type;
            gloc->fy.fstring = amem_strdup(amem, $5->fstring);
            xfree($4);
            xfree($5);
	}
	| selectgraph FIXEDPOINT PREC expr ',' expr {
            GLocator *gloc = graph_get_locator($1);
	    gloc->fx.prec = $4;
	    gloc->fy.prec = $6;
	}
	| selectgraph FIXEDPOINT XY expr ',' expr {
            GLocator *gloc = graph_get_locator($1);
	    gloc->origin.x = $4;
	    gloc->origin.y = $6;
	}
	| selectgraph FIXEDPOINT TYPE nexpr {
            GLocator *gloc = graph_get_locator($1);
            switch ($4) {
            case 0:
            case 1:
                gloc->type = GLOCATOR_TYPE_XY;
                break;
            case 2:
            case 3:
                gloc->type = GLOCATOR_TYPE_POLAR;
                break;
            default:
                gloc->type = GLOCATOR_TYPE_NONE;
                break;
            }
        }
        
	| TYPE xytype {
	}
	;

actions:
	selectset HIDDEN onoff {
	    quark_set_active($1, !$3);
	}
        ;


options:
        PAGE LAYOUT pagelayout {
#ifndef NONE_GUI
            gui_set_page_free(gapp->gui, $3 == FREE);
#endif
        }
        ;


set_setprop:
	setprop {}
	| setprop_obs {}
	;

setprop:
	selectset onoff {
	    quark_set_active($1, $2);
	}
	| selectset TYPE xytype {
	    SetType stype;
            Dataset *dsp = set_get_dataset($1);
            dsp->cols[DATA_X] = 0;
            dsp->cols[DATA_Y] = 1;
            stype = SET_XY;
            switch ($3) {
	    case XY:
                stype = SET_XY;
                break;
	    case BAR:
                stype = SET_BAR;
                break;
	    case BARDY:
                stype = SET_BAR;
                dsp->cols[DATA_Y1] = 2;
                break;
	    case BARDYDY:
                stype = SET_BAR;
                dsp->cols[DATA_Y1] = 2;
                dsp->cols[DATA_Y2] = 3;
                break;
	    case XYZ:
                stype = SET_XY;
                dsp->acol = 2;
                dsp->cols[DATA_Y1] = COL_NONE;
                break;
	    case XYDX:
                stype = SET_XY;
                dsp->cols[DATA_Y1] = 2;
                break;
	    case XYDY:
                stype = SET_XY;
                dsp->cols[DATA_Y3] = 2;
                break;
	    case XYDXDX:
                stype = SET_XY;
                dsp->cols[DATA_Y1] = 2;
                dsp->cols[DATA_Y2] = 3;
                break;
	    case XYDYDY:
                stype = SET_XY;
                break;
	    case XYDXDY:
                stype = SET_XY;
                dsp->cols[DATA_Y1] = 2;
                dsp->cols[DATA_Y3] = 3;
                break;
	    case XYDXDXDYDY:
                stype = SET_XY;
                dsp->cols[DATA_Y1] = 2;
                dsp->cols[DATA_Y2] = 3;
                dsp->cols[DATA_Y3] = 4;
                dsp->cols[DATA_Y4] = 5;
                break;
	    case XYHILO:
                stype = SET_XYHILO;
                dsp->cols[DATA_Y1] = 2;
                dsp->cols[DATA_Y2] = 3;
                dsp->cols[DATA_Y3] = 4;
                break;
	    case XYR:
                stype = SET_XYR;
                dsp->cols[DATA_Y1] = 2;
                break;
	    case XYSIZE:
                stype = SET_XYSIZE;
                dsp->cols[DATA_Y1] = 2;
                break;
	    case XYCOLOR:
                stype = SET_XYCOLOR;
                dsp->cols[DATA_Y1] = 2;
                break;
	    case XYCOLPAT:
                stype = SET_XYCOLPAT;
                dsp->cols[DATA_Y1] = 2;
                dsp->cols[DATA_Y2] = 3;
                break;
	    case XYVMAP:
                stype = SET_XYVMAP;
                dsp->cols[DATA_Y1] = 2;
                dsp->cols[DATA_Y2] = 3;
                break;
	    case XYBOXPLOT:
                stype = SET_BOXPLOT;
                dsp->cols[DATA_Y1] = 2;
                dsp->cols[DATA_Y2] = 3;
                dsp->cols[DATA_Y3] = 4;
                dsp->cols[DATA_Y4] = 5;
                break;
	    case XYSTRING:
                stype = SET_XY;
                break;
            }

            set_set_type($1, stype);
	}

	| selectset SYMBOL nexpr {
	    set *p = set_get_data($1);
            p->sym.type = $3;
	}
	| selectset SYMBOL color_select {
	    set *p = set_get_data($1);
	    p->sym.line.pen.color = $3;
	}
	| selectset SYMBOL pattern_select {
	    set *p = set_get_data($1);
	    p->sym.line.pen.pattern = $3;
	}
	| selectset SYMBOL linew_select {
	    set *p = set_get_data($1);
	    p->sym.line.width = $3;
	}
	| selectset SYMBOL lines_select {
	    set *p = set_get_data($1);
	    p->sym.line.style = $3;
	}
	| selectset SYMBOL FILL color_select {
	    set *p = set_get_data($1);
	    p->sym.fillpen.color = $4;
	}
	| selectset SYMBOL FILL pattern_select {
	    set *p = set_get_data($1);
	    p->sym.fillpen.pattern = $4;
	}
	| selectset SYMBOL SIZE expr {
	    set *p = set_get_data($1);
	    p->sym.size = $4;
	}
	| selectset SYMBOL CHAR nexpr {
	    set *p = set_get_data($1);
	    p->sym.symchar = $4;
	}
	| selectset SYMBOL CHAR font_select {
	    set *p = set_get_data($1);
	    p->sym.charfont = $4;
	}
	| selectset SYMBOL SKIP nexpr {
	    set *p = set_get_data($1);
	    p->symskip = $4;
	}

	| selectset LINE TYPE nexpr
        {
	    set *p = set_get_data($1);
	    p->line.type = $4;
	}
	| selectset LINE lines_select
        {
	    set *p = set_get_data($1);
	    p->line.line.style = $3;
	}
	| selectset LINE linew_select
        {
	    set *p = set_get_data($1);
	    p->line.line.width = $3;
	}
	| selectset LINE color_select
        {
	    set *p = set_get_data($1);
	    p->line.line.pen.color = $3;
	}
	| selectset LINE pattern_select
        {
	    set *p = set_get_data($1);
	    p->line.line.pen.pattern = $3;
	}

	| selectset FILL TYPE nexpr
        {
	    set *p = set_get_data($1);
	    p->line.filltype = $4;
	}
	| selectset FILL RULE nexpr
        {
	    set *p = set_get_data($1);
	    p->line.fillrule = $4;
	}
	| selectset FILL color_select
        {
	    set *p = set_get_data($1);
	    int prop = $3;

	    if (project_get_version_id(project) <= 40102 && project_get_version_id(project) >= 30000) {
                switch (filltype_obs) {
                case COLOR:
                    break;
                case PATTERN:
                    prop = 1;
                    break;
                default: /* NONE */
	            prop = 0;
                    break;
                }
	    }
	    p->line.fillpen.color = prop;
	}
	| selectset FILL pattern_select
        {
	    set *p = set_get_data($1);
	    int prop = $3;

	    if (project_get_version_id(project) <= 40102) {
                switch (filltype_obs) {
                case COLOR:
                    prop = 1;
                    break;
                case PATTERN:
                    break;
                default: /* NONE */
	            prop = 0;
                    break;
                }
	    }
	    p->line.fillpen.pattern = prop;
	}

        
	| selectset BASELINE onoff
        {
	    set *p = set_get_data($1);
	    p->line.baseline = $3;
	}
	| selectset BASELINE TYPE nexpr
        {
	    set *p = set_get_data($1);
	    p->line.baseline_type = $4;
	}
        
	| selectset DROPLINE onoff
        {
	    set *p = set_get_data($1);
	    p->line.droplines = $3;
	}

	| selectset AVALUE onoff
        {
	    set *p = set_get_data($1);
	    p->avalue.active = $3;
	}
	| selectset AVALUE TYPE nexpr
        {
	    set *p = set_get_data($1);
	    switch ($4) {
            case 1:
                p->ds.acol = DATA_X;
                break;
            case 2:
                p->ds.acol = DATA_Y;
                break;
            case 4:
                p->ds.acol = set_get_ncols($1);
                break;
            case 5:
                p->ds.acol = DATA_Y1;
                break;
            default:
                p->ds.acol = COL_NONE;
                break;
            }
	}
	| selectset AVALUE CHAR SIZE expr
        {
	    set *p = set_get_data($1);
	    p->avalue.tprops.charsize = $5;
	}
	| selectset AVALUE font_select
        {
	    set *p = set_get_data($1);
	    p->avalue.tprops.font = $3;
	}
	| selectset AVALUE color_select
        {
	    set *p = set_get_data($1);
	    p->avalue.tprops.color = $3;
	}
	| selectset AVALUE ROT nexpr
        {
	    set *p = set_get_data($1);
	    p->avalue.tprops.angle = $4;
	}
	| selectset AVALUE FORMAT formatchoice
        {
	    set *p = set_get_data($1);
	    AMem *amem = quark_get_amem($1);
	    p->avalue.format.type = $4->type;
	    p->avalue.format.fstring = amem_strdup(amem, $4->fstring);
            xfree($4);
	}
	| selectset AVALUE PREC nexpr
        {
	    set *p = set_get_data($1);
	    p->avalue.format.prec = $4;
	}
	| selectset AVALUE OFFSET expr ',' expr {
	    set *p = set_get_data($1);
	    p->avalue.offset.x = $4;
	    p->avalue.offset.y = $6;
	}
	| selectset AVALUE PREPEND CHRSTR
        {
	    AMem *amem = quark_get_amem($1);
            set *p = set_get_data($1);
            p->avalue.prestr = amem_strcpy(amem, p->avalue.prestr, $4);
	    xfree($4);
	}
	| selectset AVALUE APPEND CHRSTR
        {
	    AMem *amem = quark_get_amem($1);
	    set *p = set_get_data($1);
            p->avalue.appstr = amem_strcpy(amem, p->avalue.appstr, $4);
	    xfree($4);
	}

	| selectset ERRORBAR onoff {
	    set *p = set_get_data($1);
	    p->errbar.active = $3;
	}
	| selectset ERRORBAR opchoice_sel {
            Dataset *dsp = set_get_dataset($1);
	    switch ($3) {
            case NORMAL:
                break;
            case OPPOSITE:
                if (set_get_type($1) == SET_XY || set_get_type($1) == SET_BAR) {
                    iswap(&dsp->cols[DATA_Y1], &dsp->cols[DATA_Y2]);
                    iswap(&dsp->cols[DATA_Y3], &dsp->cols[DATA_Y4]);
                }
                break;
            case BOTH:
                if (set_get_type($1) == SET_XY || set_get_type($1) == SET_BAR) {
                    dsp->cols[DATA_Y2] = dsp->cols[DATA_Y1];
                    dsp->cols[DATA_Y4] = dsp->cols[DATA_Y3];
                }
                break;
            }
	}
	| selectset ERRORBAR color_select {
	    set *p = set_get_data($1);
	    p->errbar.pen.color = $3;
	}
	| selectset ERRORBAR pattern_select {
	    set *p = set_get_data($1);
	    p->errbar.pen.pattern = $3;
	}
	| selectset ERRORBAR SIZE expr {
	    set *p = set_get_data($1);
            p->errbar.barsize = $4;
	}
	| selectset ERRORBAR linew_select {
	    set *p = set_get_data($1);
            p->errbar.linew = $3;
	}
	| selectset ERRORBAR lines_select {
	    set *p = set_get_data($1);
            p->errbar.lines = $3;
	}
	| selectset ERRORBAR RISER linew_select {
	    set *p = set_get_data($1);
            p->errbar.riser_linew = $4;
	}
	| selectset ERRORBAR RISER lines_select {
	    set *p = set_get_data($1);
            p->errbar.riser_lines = $4;
	}
	| selectset ERRORBAR RISER CLIP onoff {
	    set *p = set_get_data($1);
            p->errbar.arrow_clip = $5;
	}
	| selectset ERRORBAR RISER CLIP LENGTH expr {
	    set *p = set_get_data($1);
            p->errbar.cliplen = $6;
	}

	| selectset COMMENT CHRSTR {
	    xfree($3);
	}
        
	| selectset LEGEND CHRSTR {
	    set_set_legstr($1, $3);
	    xfree($3);
	}
	;


axisfeature:
	onoff {
	    quark_set_active(whichaxisgrid, $1);
	}
	| TYPE ZERO onoff {
	    if ($3) {
                axis_set_position(normaxis, AXIS_POS_ZERO);
	        axis_set_position(oppaxis,  AXIS_POS_ZERO);
            }
	}
	| TICKP tickattr {}
	| TICKP tickattr_obs {}
	| TICKLABEL ticklabelattr {}
	| TICKLABEL ticklabelattr_obs {}
	| LABEL axislabeldesc {}
	| LABEL axislabeldesc_obs {}
	| BAR axisbardesc {}
	| OFFSET expr ',' expr {
	    axis_set_offset(normaxis, $2);
	    axis_set_offset(oppaxis,  $4);
	}
	;

tickattr:
	onoff {
            axis_enable_ticks(normaxis, $1);
            axis_enable_ticks(oppaxis,  $1);
	}
	| MAJOR expr {
            curtm->tmajor = $2;
	}
	| MINOR TICKSP nexpr {
	    curtm->nminor = $3;
	}
	| PLACE ROUNDED onoff {
	    curtm->t_round = $3;
	}

	| OFFSETX expr {
            axis_set_offset(normaxis, $2);
	}
	| OFFSETY expr {
            axis_set_offset(oppaxis,  $2);
	}
	| DEFAULT nexpr {
	    curtm->t_autonum = $2;
	}
	| inoutchoice {
	    curtm->props.inout = $1;
	    curtm->mprops.inout = $1;
	}
	| MAJOR SIZE expr {
	    curtm->props.size = $3;
	}
	| MINOR SIZE expr {
	    curtm->mprops.size = $3;
	}
	| color_select {
	    curtm->props.line.pen.color = curtm->mprops.line.pen.color = $1;
	}
	| MAJOR color_select {
	    curtm->props.line.pen.color = $2;
	}
	| MINOR color_select {
	    curtm->mprops.line.pen.color = $2;
	}
	| linew_select {
	    curtm->props.line.width = curtm->mprops.line.width = $1;
	}
	| MAJOR linew_select {
	    curtm->props.line.width = $2;
	}
	| MINOR linew_select {
	    curtm->mprops.line.width = $2;
	}
	| MAJOR lines_select {
	    curtm->props.line.style = $2;
	}
	| MINOR lines_select {
	    curtm->mprops.line.style = $2;
	}
	| MAJOR GRID onoff {
	    curtm->gprops.onoff = $3;
	}
	| MINOR GRID onoff {
	    curtm->mgprops.onoff = $3;
	}
	| opchoice_sel {
	    if ($1 == OPPOSITE) {
                axis_enable_ticks(normaxis, FALSE);
                axis_enable_bar(normaxis, FALSE);
            }
	    if ($1 == NORMAL) {
                axis_enable_ticks(oppaxis, FALSE);
                axis_enable_bar(oppaxis, FALSE);
            }
	}
	| SPEC TYPE tickspectype {
	    curtm->t_spec = $3;
	}
	| SPEC nexpr {
	    curtm->nticks = $2;
	}
	| MAJOR nexpr ',' expr {
	    curtm->tloc[$2].wtpos = $4;
	    curtm->tloc[$2].type = TICK_TYPE_MAJOR;
	}
	| MINOR nexpr ',' expr {
	    curtm->tloc[$2].wtpos = $4;
	    curtm->tloc[$2].type = TICK_TYPE_MINOR;
	}
	;

ticklabelattr:
	onoff {
            axis_enable_labels(normaxis, $1);
            axis_enable_labels(oppaxis,  $1);
	}
	| FORMAT formatchoice {
	    AMem *amem = quark_get_amem(whichaxisgrid);
	    curtm->tl_format.type = $2->type;
	    curtm->tl_format.fstring = amem_strdup(amem, $2->fstring);
            xfree($2);
	}
	| PREC nexpr {
	    curtm->tl_format.prec = $2;
	}
	| APPEND CHRSTR {
	    AMem *amem = quark_get_amem(whichaxisgrid);
            curtm->tl_appstr = amem_strcpy(amem, curtm->tl_appstr, $2);
	    xfree($2);
	}
	| PREPEND CHRSTR {
	    AMem *amem = quark_get_amem(whichaxisgrid);
            curtm->tl_prestr = amem_strcpy(amem, curtm->tl_prestr, $2);
	    xfree($2);
	}
	| ANGLE nexpr {
	    curtm->tl_tprops.angle = $2;
	}
	| SKIP nexpr {
	    curtm->tl_skip = $2;
	}
	| STAGGER nexpr {
	    curtm->tl_staggered = $2;
	}
	| opchoice_sel {
	    if ($1 == OPPOSITE) {
                axis_enable_labels(normaxis, FALSE);
            }
	    if ($1 == NORMAL) {
                axis_enable_labels(oppaxis, FALSE);
            }
	}
	| FORMULA CHRSTR {
	    AMem *amem = quark_get_amem(whichaxisgrid);
            curtm->tl_formula =
                amem_strcpy(amem, curtm->tl_formula, $2);
            xfree($2);
	}
	| START expr {
	    curtm->tl_start = $2;
	}
	| STOP expr {
	    curtm->tl_stop = $2;
	}
	| START TYPE SPEC {
	    curtm->tl_starttype = TYPE_SPEC;
	}
	| START TYPE AUTO {
	    curtm->tl_starttype = TYPE_AUTO;
	}
	| STOP TYPE SPEC {
	    curtm->tl_stoptype = TYPE_SPEC;
	}
	| STOP TYPE AUTO {
	    curtm->tl_stoptype = TYPE_AUTO;
	}
	| CHAR SIZE expr {
	    curtm->tl_tprops.charsize = $3;
	}
	| font_select {
	    curtm->tl_tprops.font = $1;
	}
	| color_select {
	    curtm->tl_tprops.color = $1;
	}
	| nexpr ',' CHRSTR {
	    AMem *amem = quark_get_amem(whichaxisgrid);
	    curtm->tloc[$1].label = 
                amem_strcpy(amem, curtm->tloc[$1].label, $3);
	    xfree($3);
	}
	| OFFSET AUTO {
	    curtm->tl_gap.x = 0.0;
	    curtm->tl_gap.y = 0.01;
	}
	| OFFSET SPEC {
	}
	| OFFSET expr ',' expr {
	    curtm->tl_gap.x = $2;
	    curtm->tl_gap.y = $4;
	}
	;

axislabeldesc:
	CHRSTR {
	    atext_set_string(normlabel, $1);
	    atext_set_string(opplabel,  $1);
	    xfree($1);
	}
	| LAYOUT PERP {
	    if (axisgrid_is_x(whichaxisgrid)) {
                atext_set_angle(normlabel, 90);
	        atext_set_angle(opplabel,  90);
            } else {
                atext_set_angle(normlabel, 0);
	        atext_set_angle(opplabel,  0);
            }
	}
	| LAYOUT PARA {
	    if (axisgrid_is_x(whichaxisgrid)) {
	        atext_set_angle(normlabel, 0);
	        atext_set_angle(opplabel,  0);
            } else {
                atext_set_angle(normlabel, 90);
	        atext_set_angle(opplabel,  90);
            }
	}
	| PLACE AUTO {
	    /* This is now default */
	}
	| PLACE SPEC {
	    /* FIXME */
	}
	| PLACE expr ',' expr {
	    /* FIXME */
	}
	| JUST justchoice {
	}
	| CHAR SIZE expr {
	    atext_set_char_size(normlabel, $3);
	    atext_set_char_size(opplabel,  $3);
	}
	| font_select {
	    atext_set_font(normlabel, $1);
	    atext_set_font(opplabel,  $1);
	}
	| color_select {
	    atext_set_color(normlabel, $1);
	    atext_set_color(opplabel,  $1);
	}
	| opchoice_sel {
	    if ($1 == NORMAL || $1 == BOTH) {
                quark_set_active(normlabel, TRUE);
            } else {
                quark_set_active(normlabel, FALSE);
            }
	    if ($1 == OPPOSITE || $1 == BOTH) {
                quark_set_active(opplabel, TRUE);
            } else {
                quark_set_active(opplabel, FALSE);
            }
	}
	;

axisbardesc:
	onoff {
            axis_enable_bar(normaxis, $1);
            axis_enable_bar(oppaxis, $1);
	}
	| color_select {
	    curtm->bar.pen.color = $1;
	}
	| lines_select {
	    curtm->bar.style = $1;
	}
	| linew_select {
	    curtm->bar.width = $1;
	}
	;

selectgraph:
        GRAPHNO
        {
            $$ = allocate_graph(project, $1);
            whichgraph = $$;
        }
        ;

selectset:
	selectgraph '.' SETNUM
	{
            $$ = allocate_set($1, $3);
	}
	| SETNUM
	{
            $$ = allocate_set(whichgraph, $1);
	}
	;

setaxis:
	axis axisfeature {}
	| selectgraph axis axisfeature {}
	;

axis:
	XAXIS {
            Quark *q = quark_find_descendant_by_idstr(whichgraph, "x_axisgrid");
            if (!q) {
                Quark *a, *l;
                APoint ap;
                VPoint vp;
                
                q = axisgrid_new(whichgraph);
                quark_idstr_set(q, "x_axisgrid");
                axisgrid_set_type(q, AXIS_TYPE_X);
                
                a = axis_new(q);
                quark_idstr_set(a, "normal");
                axis_set_position(a, AXIS_POS_NORMAL);
                l = atext_new(a);
                quark_idstr_set(l, "label");
                ap.x = 0.5; ap.y = 0.0;
                atext_set_ap(l, &ap);
                atext_set_just(l, JUST_CENTER|JUST_TOP);
                vp.x = 0.0; vp.y = -0.01;
                atext_set_offset(l, &vp);
                
                a = axis_new(q);
                quark_idstr_set(a, "opposite");
                axis_set_position(a, AXIS_POS_OPPOSITE);
                l = atext_new(a);
                quark_idstr_set(l, "label");
                ap.x = 0.5; ap.y = 1.0;
                atext_set_ap(l, &ap);
                atext_set_just(l, JUST_CENTER|JUST_BOTTOM);
                vp.x = 0.0; vp.y = 0.01;
                atext_set_offset(l, &vp);
            }
            curtm = axisgrid_get_data(q);
            whichaxisgrid = q;
            normaxis  = quark_find_descendant_by_idstr(q, "normal");
            oppaxis   = quark_find_descendant_by_idstr(q, "opposite");
            normlabel = quark_find_descendant_by_idstr(normaxis, "label");
            opplabel  = quark_find_descendant_by_idstr(oppaxis,  "label");
            
        }
	| YAXIS {
            Quark *q = quark_find_descendant_by_idstr(whichgraph, "y_axisgrid");
            if (!q) {
                Quark *a, *l;
                APoint ap;
                VPoint vp;
                
                q = axisgrid_new(whichgraph);
                quark_idstr_set(q, "y_axisgrid");
                axisgrid_set_type(q, AXIS_TYPE_Y);
                
                a = axis_new(q);
                quark_idstr_set(a, "normal");
                axis_set_position(a, AXIS_POS_NORMAL);
                l = atext_new(a);
                quark_idstr_set(l, "label");
                ap.x = 0.0; ap.y = 0.5;
                atext_set_ap(l, &ap);
                atext_set_just(l, JUST_RIGHT|JUST_MIDDLE);
                vp.x = -0.01; vp.y = 0.0;
                atext_set_offset(l, &vp);
                
                a = axis_new(q);
                quark_idstr_set(a, "opposite");
                axis_set_position(a, AXIS_POS_OPPOSITE);
                l = atext_new(a);
                quark_idstr_set(l, "label");
                ap.x = 1.0; ap.y = 0.5;
                atext_set_ap(l, &ap);
                atext_set_just(l, JUST_LEFT|JUST_MIDDLE);
                vp.x = 0.01; vp.y = 0.0;
                atext_set_offset(l, &vp);
            }
            curtm = axisgrid_get_data(q);
            whichaxisgrid = q;
            normaxis  = quark_find_descendant_by_idstr(q, "normal");
            oppaxis   = quark_find_descendant_by_idstr(q, "opposite");
            normlabel = quark_find_descendant_by_idstr(normaxis, "label");
            opplabel  = quark_find_descendant_by_idstr(oppaxis,  "label");
        }
	| ALTXAXIS {
            Quark *q = quark_find_descendant_by_idstr(whichgraph, "altx_axisgrid");
            if (!q) {
                Quark *a, *l;
                APoint ap;
                VPoint vp;
                
                q = axisgrid_new(whichgraph);
                quark_idstr_set(q, "altx_axisgrid");
                axisgrid_set_type(q, AXIS_TYPE_X);
                
                a = axis_new(q);
                quark_idstr_set(a, "normal");
                axis_set_position(a, AXIS_POS_NORMAL);
                l = atext_new(a);
                quark_idstr_set(l, "label");
                ap.x = 0.5; ap.y = 0.0;
                atext_set_ap(l, &ap);
                vp.x = 0.0; vp.y = -0.01;
                atext_set_offset(l, &vp);
                
                a = axis_new(q);
                quark_idstr_set(a, "opposite");
                axis_set_position(a, AXIS_POS_OPPOSITE);
                l = atext_new(a);
                quark_idstr_set(l, "label");
                ap.x = 0.5; ap.y = 1.0;
                atext_set_ap(l, &ap);
                vp.x = 0.0; vp.y = 0.01;
                atext_set_offset(l, &vp);
            }
            curtm = axisgrid_get_data(q);
            whichaxisgrid = q;
            normaxis  = quark_find_descendant_by_idstr(q, "normal");
            oppaxis   = quark_find_descendant_by_idstr(q, "opposite");
            normlabel = quark_find_descendant_by_idstr(normaxis, "label");
            opplabel  = quark_find_descendant_by_idstr(oppaxis,  "label");
        }
	| ALTYAXIS {
            Quark *q = quark_find_descendant_by_idstr(whichgraph, "alty_axisgrid");
            if (!q) {
                Quark *a, *l;
                APoint ap;
                VPoint vp;
                
                q = axisgrid_new(whichgraph);
                quark_idstr_set(q, "alty_axisgrid");
                axisgrid_set_type(q, AXIS_TYPE_Y);
                
                a = axis_new(q);
                quark_idstr_set(a, "normal");
                axis_set_position(a, AXIS_POS_NORMAL);
                l = atext_new(a);
                quark_idstr_set(l, "label");
                ap.x = 0.0; ap.y = 0.5;
                atext_set_ap(l, &ap);
                atext_set_just(l, JUST_RIGHT|JUST_MIDDLE);
                vp.x = -0.01; vp.y = 0.0;
                atext_set_offset(l, &vp);
                
                a = axis_new(q);
                quark_idstr_set(a, "opposite");
                axis_set_position(a, AXIS_POS_OPPOSITE);
                l = atext_new(a);
                quark_idstr_set(l, "label");
                ap.x = 1.0; ap.y = 0.5;
                atext_set_ap(l, &ap);
                atext_set_just(l, JUST_LEFT|JUST_MIDDLE);
                vp.x = 0.01; vp.y = 0.0;
                atext_set_offset(l, &vp);
            }
            curtm = axisgrid_get_data(q);
            whichaxisgrid = q;
            normaxis  = quark_find_descendant_by_idstr(q, "normal");
            oppaxis   = quark_find_descendant_by_idstr(q, "opposite");
            normlabel = quark_find_descendant_by_idstr(normaxis, "label");
            opplabel  = quark_find_descendant_by_idstr(oppaxis,  "label");
        }
	;

title:
	TITLE {
            Quark *q = quark_find_descendant_by_idstr(whichframe, "title");
            if (!q) {
                AText *at;
                q = atext_new(whichframe);
                quark_set_active(q, TRUE);
                at = atext_get_data(q);
                if (at) {
                    APoint ap = {0.5, 1.0};
                    VPoint offset = {0.0, 0.06};
                    quark_idstr_set(q, "title");
                    at->ap = ap;
                    at->offset = offset;
                    at->text_props.just = JUST_CENTER|JUST_BOTTOM;
                }
            }
            
            $$ = q;
        }
	| SUBTITLE {
            Quark *q = quark_find_descendant_by_idstr(whichframe, "subtitle");
            if (!q) {
                AText *at;
                q = atext_new(whichframe);
                quark_set_active(q, TRUE);
                at = atext_get_data(q);
                if (at) {
                    APoint ap = {0.5, 1.0};
                    VPoint offset = {0.0, 0.02};
                    quark_idstr_set(q, "subtitle");
                    at->ap = ap;
                    at->offset = offset;
                    at->text_props.just = JUST_CENTER|JUST_BOTTOM;
                }
            }
            
            $$ = q;
        }
	;

atext:
	STRING {
            $$ = curatext;
        }
	| TIMESTAMP {
            Quark *q = quark_find_descendant_by_idstr(project,
                "timestamp");
            if (!q) {
                q = atext_new(project);
                quark_idstr_set(q, "timestamp");
                atext_set_string(q, "\\${timestamp}");
            }
            
            $$ = q;
        }
	;

selectregion:
        REGNUM
        {
            if (!whichgraph) {
                whichgraph = allocate_graph(project, 0);
            }
            $$ = allocate_region(whichgraph, $1);
        }
        ;

tickspectype:
	NONE { $$ =  TICKS_SPEC_NONE; }
	| TICKSP { $$ = TICKS_SPEC_MARKS; }
	| BOTH { $$ = TICKS_SPEC_BOTH; }
	;

xytype:
	XY
	| BAR
	| BARDY
	| BARDYDY
	| XYZ
	| XYDX
	| XYDY
	| XYDXDX
	| XYDYDY
	| XYDXDY
	| XYDXDXDYDY
	| XYHILO
	| XYR
	| XYSIZE
	| XYCOLOR
	| XYCOLPAT
	| XYVMAP
	| XYBOXPLOT
	| XYSTRING
	;

graphtype:
	XY { $$ = GRAPH_XY; }
	| CHART { $$ = GRAPH_CHART; }
	| POLAR { $$ = GRAPH_POLAR; }
	| SMITH { $$ = GRAPH_SMITH; }
	| FIXED { $$ = GRAPH_FIXED; }
	| PIE   { $$ = GRAPH_PIE;   }
	;

objecttype:
	BOX       { $$ = DO_BOX;    }
	| ELLIPSE { $$ = DO_ARC;    }
	| LINE    { $$ = DO_LINE;   }
	;

pagelayout:
        FREE
        | FIXED
        ;

pageorient:
        LANDSCAPE  { $$ = PAGE_ORIENT_LANDSCAPE; }
        | PORTRAIT { $$ = PAGE_ORIENT_PORTRAIT;  }
        ;

regiontype:
	ABOVE     { $$ = REGION_POLYGON; }
	|  BELOW  { $$ = REGION_POLYGON; }
	|  LEFT   { $$ = REGION_POLYGON; }
	|  RIGHT  { $$ = REGION_POLYGON; }
	|  POLYI  { $$ = REGION_POLYGON; }
	|  POLYO  { $$ = REGION_POLYGON; }
	|  HORIZI { $$ = REGION_BAND;    }
	|  VERTI  { $$ = REGION_BAND;    }
	|  HORIZO { $$ = REGION_BAND;    }
	|  VERTO  { $$ = REGION_BAND;    }
	;

scaletype: NORMAL { $$ = SCALE_NORMAL; }
	| LOGARITHMIC { $$ = SCALE_LOG; }
	| RECIPROCAL { $$ = SCALE_REC; }
	| LOGIT { $$ = SCALE_LOGIT; }
	;

onoff: ON { $$ = TRUE; }
	| OFF { $$ = FALSE; }
	;

sourcetype: 
        DISK { $$ = SOURCE_DISK; }
	| PIPE {
            if (!gapp->rt->safe_mode) {
                $$ = SOURCE_PIPE;
            } else {
                yyerror("Pipe inputs are disabled in safe mode");
                $$ = SOURCE_DISK;
            }
        }
	;

justchoice: RIGHT { $$ = JUST_RIGHT; }
	| LEFT { $$ = JUST_LEFT; }
	| CENTER { $$ = JUST_CENTER; }
	;

inoutchoice: IN { $$ = TICKS_IN; }
	| OUT { $$ = TICKS_OUT; }
	| BOTH { $$ = TICKS_BOTH; }
	;

formatchoice: DECIMAL { $$ = format_new(); $$->type = FORMAT_DECIMAL; }
	| EXPONENTIAL { $$ = format_new(); $$->type = FORMAT_EXPONENTIAL; }
	| GENERAL { $$ = format_new(); $$->type = FORMAT_GENERAL; }
	| SCIENTIFIC { $$ = format_new(); $$->type = FORMAT_SCIENTIFIC; }
	| ENGINEERING { $$ = format_new(); $$->type = FORMAT_ENGINEERING; }
	| COMPUTING { $$ = format_new(); $$->type = FORMAT_COMPUTING; }
	| POWER { $$ = format_new(); $$->type = FORMAT_POWER; }
	| DDMMYY { $$ = format_new(); $$->type = FORMAT_DATETIME; $$->fstring = "%d-%m-%Y"; }
	| MMDDYY { $$ = format_new(); $$->type = FORMAT_DATETIME; $$->fstring = "%m-%d-%Y"; }
	| YYMMDD { $$ = format_new(); $$->type = FORMAT_DATETIME; $$->fstring = "%Y-%m-%d"; }
	| MMYY { $$ = format_new(); $$->type = FORMAT_DATETIME; $$->fstring = "%m-%Y"; }
	| MMDD { $$ = format_new(); $$->type = FORMAT_DATETIME; $$->fstring = "%m-%d"; }
	| MONTHDAY { $$ = format_new(); $$->type = FORMAT_DATETIME; $$->fstring = "%b-%d"; }
	| DAYMONTH { $$ = format_new(); $$->type = FORMAT_DATETIME; $$->fstring = "%d-%b"; }
	| MONTHS { $$ = format_new(); $$->type = FORMAT_DATETIME; $$->fstring = "%b"; }
	| MONTHSY { $$ = format_new(); $$->type = FORMAT_DATETIME; $$->fstring = "%b-%Y"; }
	| MONTHL { $$ = format_new(); $$->type = FORMAT_DATETIME; $$->fstring = "%B"; }
	| DAYOFWEEKS { $$ = format_new(); $$->type = FORMAT_DATETIME; $$->fstring = "%a"; }
	| DAYOFWEEKL { $$ = format_new(); $$->type = FORMAT_DATETIME; $$->fstring = "%A"; }
	| DAYOFYEAR { $$ = format_new(); $$->type = FORMAT_DATETIME; $$->fstring = "%j"; }
	| HMS { $$ = format_new(); $$->type = FORMAT_DATETIME; $$->fstring = "%H:%M:%S"; }
	| MMDDHMS { $$ = format_new(); $$->type = FORMAT_DATETIME; $$->fstring = "%m-%d %H:%M:%S"; }
	| MMDDYYHMS { $$ = format_new(); $$->type = FORMAT_DATETIME; $$->fstring = "%m-%d-%Y %H:%M:%S"; }
	| YYMMDDHMS { $$ = format_new(); $$->type = FORMAT_DATETIME; $$->fstring = "%Y-%m-%d %H:%M:%S"; }
	| DEGREESLON { $$ = format_new(); $$->type = FORMAT_GEOGRAPHIC; $$->fstring = "%D%X"; }
	| DEGREESMMLON { $$ = format_new(); $$->type = FORMAT_GEOGRAPHIC; $$->fstring = "%D %M' %X"; }
	| DEGREESMMSSLON { $$ = format_new(); $$->type = FORMAT_GEOGRAPHIC; $$->fstring = "%D %M' %S\" %X"; }
	| MMSSLON { $$ = format_new(); $$->type = FORMAT_GEOGRAPHIC; $$->fstring = "%M' %S\" %X"; }
	| DEGREESLAT { $$ = format_new(); $$->type = FORMAT_GEOGRAPHIC; $$->fstring = "%D%Y"; }
	| DEGREESMMLAT { $$ = format_new(); $$->type = FORMAT_GEOGRAPHIC; $$->fstring = "%D %M' %Y"; }
	| DEGREESMMSSLAT { $$ = format_new(); $$->type = FORMAT_GEOGRAPHIC; $$->fstring = "%D %M' %S\" %Y"; }
	| MMSSLAT { $$ = format_new(); $$->type = FORMAT_GEOGRAPHIC; $$->fstring = "%M' %S\" %Y"; }
	;

signchoice: NORMAL { $$ = SIGN_NORMAL; }
	| ABSOLUTE { $$ = SIGN_ABSOLUTE; }
	| NEGATE { $$ = SIGN_NEGATE; }
	;

worldview: WORLD { $$ = COORD_WORLD; }
	| VIEW { $$ = COORD_VIEW; }
	;

font_select:
        FONTP nexpr
        {
            $$ = $2;
        }
        | FONTP CHRSTR
        {
            $$ = project_get_font_by_name(project, $2);
            xfree($2);
        }
        ;

lines_select:
        LINESTYLE nexpr
        {
	    unsigned int lines = $2;
            if (lines >= 0 && lines < number_of_linestyles(canvas)) {
	        $$ = lines;
	    } else {
	        errmsg("invalid linestyle");
	        $$ = 1;
	    }
        }
        ;

pattern_select:
        PATTERN nexpr
        {
	    unsigned int patno = $2;
            if (patno >= 0 && patno < number_of_patterns(canvas)) {
	        $$ = patno;
	    } else {
	        errmsg("invalid pattern number");
	        $$ = 1;
	    }
        }
        ;

color_select:
        COLOR nexpr
        {
            unsigned int c = $2;
	    Project *pr = project_get_data(project);
            if (c >= 0 && c < pr->ncolors) {
                $$ = c;
            } else {
                errmsg("Invalid color ID");
                $$ = 1;
            }
        }
        ;

linew_select:
        LINEWIDTH expr
        {
            double linew;
            linew = $2;
            if (linew < 0.0) {
                yyerror("Negative linewidth");
                linew = 0.0;
            } else if (linew > MAX_LINEWIDTH) {
                yyerror("Linewidth too large");
                linew = MAX_LINEWIDTH;
            }
            $$ = linew;
        }
        ;

opchoice_sel: PLACE opchoice
        {
            $$ = $2;
        }
        ;

opchoice: NORMAL
	| OPPOSITE
	| BOTH
	;


parmset_obs:
        PAGE LAYOUT pageorient
        {
            int wpp, hpp;
            if ($3 == PAGE_ORIENT_LANDSCAPE) {
                wpp = 792;
                hpp = 612;
            } else {
                wpp = 612;
                hpp = 792;
            }
            project_set_page_dimensions(project, wpp, hpp);
        }
        | PAGE SIZE NUMBER NUMBER {
            project_set_page_dimensions(project, (int) $3, (int) $4);
        }
	| PAGE nexpr {
	    gapp->rt->scrollper = $2 / 100.0;
	}
	| PAGE INOUT nexpr {
	    gapp->rt->shexper = $3 / 100.0;
	}

	| DEFAULT FONTP SOURCE expr {
	}

	| STACK WORLD expr ',' expr ',' expr ',' expr
	{
	}
	| STACK WORLD expr ',' expr ',' expr ',' expr TICKP expr ',' expr ',' expr ',' expr
	{
	}

	| objecttype FILL colpat_obs {filltype_obs = $3;}

	| atext linew_select { }

	| title linew_select { }

	| LEGEND BOX onoff {
	    if ($3 == FALSE && project_get_version_id(project) <= 40102) {
                legend *l = frame_get_legend(whichframe);
                l->boxline.pen.pattern = 0;
            }
	}
	| LEGEND X1 expr {
            leg_x1_obs = $3;
	}
	| LEGEND Y1 expr {
	    VPoint vp;
            view gv;
            legend *l = frame_get_legend(whichframe);
            if (leg_loctype_obs == COORD_VIEW) {
                vp.x = leg_x1_obs;
                vp.y = $3;
            } else {
                WPoint wp;
                wp.x = leg_x1_obs;
                wp.y = $3;
                Wpoint2Vpoint(whichgraph, &wp, &vp);
            }
            frame_get_view(whichframe, &gv);
            l->offset.x = vp.x - gv.xv1;
            l->offset.y = vp.y - gv.yv2;

            l->anchor.x = 0.0;
            l->anchor.y = 1.0;
            l->just     = JUST_LEFT | JUST_TOP;
	}
	| LEGEND STRING nexpr CHRSTR {
            int nsets;
            Quark *pset, **psets;
            nsets = quark_get_descendant_sets(whichgraph, &psets);
            if ($3 >= 0 && $3 < nsets) {
                pset = psets[$3];
            } else {
                pset = NULL;
            }
            if (set_set_legstr(pset, $4) != RETURN_SUCCESS) {
                yyerror("Unallocated set");
            }
            xfree(psets);
            xfree($4);
	}
	| LEGEND BOX FILL onoff { }
	| LEGEND BOX FILL WITH colpat_obs {filltype_obs = $5;}
	| LEGEND lines_select { }
	| LEGEND linew_select { }

	| selectgraph LABEL onoff { }

	| selectgraph TYPE LOGX { 
	    graph_set_type($1, GRAPH_XY);
            graph_set_xscale($1, SCALE_LOG);
	}
	| selectgraph TYPE LOGY { 
	    graph_set_type($1, GRAPH_XY);
            graph_set_yscale($1, SCALE_LOG);
	}
	| selectgraph TYPE LOGXY
	{ 
	    graph_set_type($1, GRAPH_XY);
            graph_set_xscale($1, SCALE_LOG);
            graph_set_yscale($1, SCALE_LOG);
	}
	| selectgraph TYPE BAR
	{ 
	    graph_set_type($1, GRAPH_CHART);
	    graph_set_stacked($1, FALSE);
	    graph_set_xyflip($1, FALSE);
	}
	| selectgraph TYPE HBAR
	{ 
	    graph_set_type($1, GRAPH_CHART);
	    graph_set_stacked($1, FALSE);
	    graph_set_xyflip($1, TRUE);
	}
	| selectgraph TYPE STACKEDBAR
	{ 
	    graph_set_type($1, GRAPH_CHART);
	    graph_set_stacked($1, TRUE);
	    graph_set_xyflip($1, FALSE);
	}
	| selectgraph TYPE STACKEDHBAR
	{ 
	    graph_set_type($1, GRAPH_CHART);
	    graph_set_stacked($1, TRUE);
	    graph_set_xyflip($1, TRUE);
	}

	| LEGEND LAYOUT expr {
	}

	| FRAMEP FILL onoff { 
	    frame *f = frame_get_data(whichframe);
            f->fillpen.pattern = $3;
        }

	| selectgraph AUTOSCALE TYPE AUTO {
        }
	| selectgraph AUTOSCALE TYPE SPEC {
        }

	| LINE ARROW SIZE expr {
	    if (!curobject) {
                yyerror("No active object");
	    } else if (curobject->type != DO_LINE) {
                yyerror("The object is not a line");
	    } else {
	        DOLineData *l = (DOLineData *) curobject->odata;
	        l->arrow.length = 2.0*$4;
            }
	}

        | HARDCOPY DEVICE iexpr { }
        | PS LINEWIDTH BEGIN expr { }
        | PS LINEWIDTH INCREMENT expr { }
        | PS linew_select { }
        ;


axislabeldesc_obs:
	linew_select { }
	| opchoice_sel_obs {
	    if ($1 == NORMAL || $1 == BOTH) {
                quark_set_active(normlabel, TRUE);
            } else {
                quark_set_active(normlabel, FALSE);
            }
	    if ($1 == OPPOSITE || $1 == BOTH) {
                quark_set_active(opplabel, TRUE);
            } else {
                quark_set_active(opplabel, FALSE);
            }
	}
        ;

setprop_obs:
	selectset SYMBOL FILL nexpr {
	    set *p = set_get_data($1);
            switch ($4){
	    case 0:
	        p->sym.fillpen.pattern = 0;
	        break;
	    case 1:
	        p->sym.fillpen.pattern = 1;
	        break;
	    case 2:
	        p->sym.fillpen.pattern = 1;
	        p->sym.fillpen.color = getbgcolor(canvas);
	        break;
	    }
	}
	| selectset SKIP nexpr
        {
	    set *p = set_get_data($1);
	    p->symskip = $3;
	}
	| selectset FILL nexpr
        {
	    set *p = set_get_data($1);
	    switch ($3) {
            case 0:
                p->line.filltype = SETFILL_NONE;
                break;
            case 1:
                p->line.filltype = SETFILL_POLYGON;
                break;
            case 2:
                p->line.filltype = SETFILL_BASELINE;
                p->line.baseline_type = BASELINE_TYPE_0;
                break;
            case 6:
                p->line.filltype = SETFILL_BASELINE;
                p->line.baseline_type = BASELINE_TYPE_GMIN;
                break;
            case 7:
                p->line.filltype = SETFILL_BASELINE;
                p->line.baseline_type = BASELINE_TYPE_GMAX;
                break;
            }
	}
	| selectset ERRORBAR TYPE opchoice_obs {
            Dataset *dsp = set_get_dataset($1);
	    switch ($4) {
            case NORMAL:
                break;
            case OPPOSITE:
                if (set_get_type($1) == SET_XY || set_get_type($1) == SET_BAR) {
                    iswap(&dsp->cols[DATA_Y1], &dsp->cols[DATA_Y2]);
                    iswap(&dsp->cols[DATA_Y3], &dsp->cols[DATA_Y4]);
                }
                break;
            case BOTH:
                break;
            }
	}
/*
 * 	| selectset SYMBOL COLOR '-' N_NUMBER {
 * 	    p->sympen.color = -1;
 * 	}
 */
	| selectset SYMBOL CENTER onoff { }
	| selectset lines_select {
	    set *p = set_get_data($1);
	    p->line.line.style = $2;
	}
	| selectset linew_select {
	    set *p = set_get_data($1);
	    p->line.line.width = $2;
	}
	| selectset color_select {
	    set *p = set_get_data($1);
	    p->line.line.pen.color = $2;
	}
	| selectset FILL WITH colpat_obs {filltype_obs = $4;}
	| selectset XYZ expr ',' expr { }
	| selectset ERRORBAR LENGTH expr {
	    set *p = set_get_data($1);
            p->errbar.barsize = $4;
	}
	| selectset ERRORBAR RISER onoff { }
        ;
        

tickattr_obs:
	MAJOR onoff {
	    /* <= xmgr-4.1 */
	    quark_set_active(whichaxisgrid, $2);
	}
	| MINOR onoff { }
	| ALT onoff   { }
	| MINP NUMBER   { }
	| MAXP NUMBER   { }
	| LOG onoff   { }
	| TYPE AUTO {
	    curtm->t_spec = TICKS_SPEC_NONE;
	}
	| TYPE SPEC {
	    if (curtm->t_spec != TICKS_SPEC_BOTH) {
                curtm->t_spec = TICKS_SPEC_MARKS;
            }
	}
	| MINOR expr {
	    if ($2 != 0.0) {
                curtm->nminor = 
                            (int) rint(curtm->tmajor / $2 - 1);
            } else {
                curtm->nminor = 0;
            }
	}
	| SIZE expr {
	    curtm->props.size = $2;
	}
	| nexpr ',' expr {
	    curtm->tloc[$1].wtpos = $3;
	    curtm->tloc[$1].type = TICK_TYPE_MAJOR;
	}
	| opchoice_sel_obs {
	    if ($1 == OPPOSITE) {
                axis_enable_ticks(normaxis, FALSE);
                axis_enable_bar(normaxis, FALSE);
            }
	    if ($1 == NORMAL) {
                axis_enable_ticks(oppaxis, FALSE);
                axis_enable_bar(oppaxis, FALSE);
            }
	}
        ;

ticklabelattr_obs:
	linew_select { }
	| TYPE AUTO {
	    if (curtm->t_spec == TICKS_SPEC_BOTH) {
                curtm->t_spec = TICKS_SPEC_MARKS;
            }
	}
	| FORMAT expr {
	    curtm->tl_format.type = $2;
	}
	| TYPE SPEC {
	    curtm->t_spec = TICKS_SPEC_BOTH;
	}
	| LAYOUT SPEC { }

	| LAYOUT HORIZONTAL {
	    curtm->tl_tprops.angle = 0;
	}
	| LAYOUT VERTICAL {
	    curtm->tl_tprops.angle = 90;
	}
	| PLACE ON TICKSP { }
	| PLACE BETWEEN TICKSP { }
	| opchoice_sel_obs {
	    if ($1 == OPPOSITE) {
                axis_enable_labels(normaxis, FALSE);
            }
	    if ($1 == NORMAL) {
                axis_enable_labels(oppaxis, FALSE);
            }
	}
	| SIGN signchoice {
	    AMem *amem = quark_get_amem(whichaxisgrid);
	    switch($2) {
            case SIGN_NEGATE:
                curtm->tl_formula =
                    amem_strcpy(amem, curtm->tl_formula, "-$t");
                break;
            case SIGN_ABSOLUTE:
                curtm->tl_formula =
                    amem_strcpy(amem, curtm->tl_formula, "abs($t)");
                break;
            default:
                curtm->tl_formula =
                    amem_strcpy(amem, curtm->tl_formula, NULL);
                break;
            }
	}
        ;

colpat_obs: NONE
	| COLOR
	| PATTERN
	;

opchoice_sel_obs: OP opchoice_obs
        {
            $$ = $2;
        }
        ;

opchoice_obs: TOP { $$ = OPPOSITE; }
	| BOTTOM  { $$ = NORMAL; }
	| LEFT    { $$ = NORMAL; }
	| RIGHT   { $$ = OPPOSITE; }
	| BOTH    { $$ = BOTH; }
	;

%%

/* list of intrinsic functions and keywords */
symtab_entry key[] = {
	{"ABOVE", ABOVE, NULL},
	{"ABSOLUTE", ABSOLUTE, NULL},
	{"ALT", ALT, NULL},
	{"ALTXAXIS", ALTXAXIS, NULL},
	{"ALTYAXIS", ALTYAXIS, NULL},
	{"ANGLE", ANGLE, NULL},
	{"APPEND", APPEND, NULL},
	{"ARROW", ARROW, NULL},
	{"AUTO", AUTO, NULL},
	{"AUTOSCALE", AUTOSCALE, NULL},
	{"AVALUE", AVALUE, NULL},
	{"AVG", AVG, NULL},
	{"BACKGROUND", BACKGROUND, NULL},
	{"BAR", BAR, NULL},
	{"BARDY", BARDY, NULL},
	{"BARDYDY", BARDYDY, NULL},
	{"BASELINE", BASELINE, NULL},
        {"BEGIN", BEGIN, NULL},
	{"BELOW", BELOW, NULL},
	{"BETWEEN", BETWEEN, NULL},
	{"BOTH", BOTH, NULL},
	{"BOTTOM", BOTTOM, NULL},
	{"BOX", BOX, NULL},
	{"CENTER", CENTER, NULL},
	{"CHAR", CHAR, NULL},
	{"CHART", CHART, NULL},
	{"CHRSTR", CHRSTR, NULL},
	{"CLIP", CLIP, NULL},
	{"COLOR", COLOR, NULL},
	{"COMMENT", COMMENT, NULL},
	{"COMPUTING", COMPUTING, NULL},
	{"DATE", DATE, NULL},
	{"DAYMONTH", DAYMONTH, NULL},
	{"DAYOFWEEKL", DAYOFWEEKL, NULL},
	{"DAYOFWEEKS", DAYOFWEEKS, NULL},
	{"DAYOFYEAR", DAYOFYEAR, NULL},
	{"DDMMYY", DDMMYY, NULL},
	{"DECIMAL", DECIMAL, NULL},
	{"DEF", DEF, NULL},
	{"DEFAULT", DEFAULT, NULL},
	{"DEGREESLAT", DEGREESLAT, NULL},
	{"DEGREESLON", DEGREESLON, NULL},
	{"DEGREESMMLAT", DEGREESMMLAT, NULL},
	{"DEGREESMMLON", DEGREESMMLON, NULL},
	{"DEGREESMMSSLAT", DEGREESMMSSLAT, NULL},
	{"DEGREESMMSSLON", DEGREESMMSSLON, NULL},
	{"DESCRIPTION", DESCRIPTION, NULL},
	{"DEVICE", DEVICE, NULL},
	{"DISK", DISK, NULL},
	{"DROPLINE", DROPLINE, NULL},
	{"ELLIPSE", ELLIPSE, NULL},
	{"ENGINEERING", ENGINEERING, NULL},
	{"ER", ERRORBAR, NULL},
	{"ERRORBAR", ERRORBAR, NULL},
	{"EXPONENTIAL", EXPONENTIAL, NULL},
	{"FALSE", OFF, NULL},
	{"FILL", FILL, NULL},
	{"FIXED", FIXED, NULL},
	{"FIXEDPOINT", FIXEDPOINT, NULL},
	{"FONT", FONTP, NULL},
	{"FORMAT", FORMAT, NULL},
	{"FORMULA", FORMULA, NULL},
	{"FRAME", FRAMEP, NULL},
	{"FREE", FREE, NULL},
	{"GENERAL", GENERAL, NULL},
	{"GRAPH", GRAPH, NULL},
	{"GRID", GRID, NULL},
	{"HARDCOPY", HARDCOPY, NULL},
	{"HBAR", HBAR, NULL},
	{"HGAP", HGAP, NULL},
	{"HIDDEN", HIDDEN, NULL},
	{"HMS", HMS, NULL},
	{"HORIZI", HORIZI, NULL},
	{"HORIZO", HORIZO, NULL},
	{"HORIZONTAL", HORIZONTAL, NULL},
	{"IN", IN, NULL},
	{"INCREMENT", INCREMENT, NULL},
	{"INOUT", INOUT, NULL},
	{"INVERT", INVERT, NULL},
	{"JUST", JUST, NULL},
	{"LABEL", LABEL, NULL},
	{"LANDSCAPE", LANDSCAPE, NULL},
	{"LAYOUT", LAYOUT, NULL},
	{"LEFT", LEFT, NULL},
	{"LEGEND", LEGEND, NULL},
	{"LENGTH", LENGTH, NULL},
	{"LINE", LINE, NULL},
	{"LINEAR", LINEAR, NULL},
	{"LINESTYLE", LINESTYLE, NULL},
	{"LINEWIDTH", LINEWIDTH, NULL},
	{"LINK", LINK, NULL},
	{"LOCTYPE", LOCTYPE, NULL},
	{"LOG", LOG, NULL},
	{"LOGARITHMIC", LOGARITHMIC, NULL},
	{"LOGX", LOGX, NULL},
	{"LOGXY", LOGXY, NULL},
	{"LOGY", LOGY, NULL},
	{"LOGIT", LOGIT, NULL},
	{"MAJOR", MAJOR, NULL},
	{"MAP", MAP, NULL},
	{"MAX", MAXP, NULL},
	{"MIN", MINP, NULL},
	{"MINOR", MINOR, NULL},
	{"MMDD", MMDD, NULL},
	{"MMDDHMS", MMDDHMS, NULL},
	{"MMDDYY", MMDDYY, NULL},
	{"MMDDYYHMS", MMDDYYHMS, NULL},
	{"MMSSLAT", MMSSLAT, NULL},
	{"MMSSLON", MMSSLON, NULL},
	{"MMYY", MMYY, NULL},
	{"MONTHDAY", MONTHDAY, NULL},
	{"MONTHL", MONTHL, NULL},
	{"MONTHS", MONTHS, NULL},
	{"MONTHSY", MONTHSY, NULL},
	{"NEGATE", NEGATE, NULL},
	{"NONE", NONE, NULL},
	{"NORMAL", NORMAL, NULL},
	{"OFF", OFF, NULL},
	{"OFFSET", OFFSET, NULL},
	{"OFFSETX", OFFSETX, NULL},
	{"OFFSETY", OFFSETY, NULL},
	{"ON", ON, NULL},
	{"OP", OP, NULL},
	{"OPPOSITE", OPPOSITE, NULL},
	{"OUT", OUT, NULL},
	{"PAGE", PAGE, NULL},
	{"PARA", PARA, NULL},
	{"PATTERN", PATTERN, NULL},
	{"PERP", PERP, NULL},
	{"PIE", PIE, NULL},
	{"PIPE", PIPE, NULL},
	{"PLACE", PLACE, NULL},
	{"POLAR", POLAR, NULL},
	{"POLYI", POLYI, NULL},
	{"POLYO", POLYO, NULL},
	{"PORTRAIT", PORTRAIT, NULL},
	{"POWER", POWER, NULL},
	{"PREC", PREC, NULL},
	{"PREPEND", PREPEND, NULL},
	{"PS", PS, NULL},
	{"RECIPROCAL", RECIPROCAL, NULL},
	{"REFERENCE", REFERENCE, NULL},
	{"RIGHT", RIGHT, NULL},
	{"RISER", RISER, NULL},
	{"ROT", ROT, NULL},
	{"ROUNDED", ROUNDED, NULL},
	{"RULE", RULE, NULL},
	{"SCALE", SCALE, NULL},
	{"SCIENTIFIC", SCIENTIFIC, NULL},
	{"SCROLL", SCROLL, NULL},
	{"SD", SD, NULL},
	{"SET", SET, NULL},
	{"SFORMAT", SFORMAT, NULL},
	{"SIGN", SIGN, NULL},
	{"SIZE", SIZE, NULL},
	{"SKIP", SKIP, NULL},
	{"SMITH", SMITH, NULL},
	{"SOURCE", SOURCE, NULL},
	{"SPEC", SPEC, NULL},
	{"STACK", STACK, NULL},
	{"STACKED", STACKED, NULL},
	{"STACKEDBAR", STACKEDBAR, NULL},
	{"STACKEDHBAR", STACKEDHBAR, NULL},
	{"STAGGER", STAGGER, NULL},
	{"START", START, NULL},
	{"STOP", STOP, NULL},
	{"STRING", STRING, NULL},
	{"SUBTITLE", SUBTITLE, NULL},
	{"SYMBOL", SYMBOL, NULL},
	{"TARGET", TARGET, NULL},
	{"TICK", TICKP, NULL},
	{"TICKLABEL", TICKLABEL, NULL},
	{"TICKS", TICKSP, NULL},
	{"TIMESTAMP", TIMESTAMP, NULL},
	{"TITLE", TITLE, NULL},
	{"TO", TO, NULL},
	{"TOP", TOP, NULL},
	{"TRUE", ON, NULL},
	{"TYPE", TYPE, NULL},
	{"UP", UP, NULL},
	{"VERSION", VERSION, NULL},
	{"VERTI", VERTI, NULL},
	{"VERTICAL", VERTICAL, NULL},
	{"VERTO", VERTO, NULL},
	{"VGAP", VGAP, NULL},
	{"VIEW", VIEW, NULL},
	{"WITH", WITH, NULL},
	{"WORLD", WORLD, NULL},
	{"WRAP", WRAP, NULL},
	{"X1", X1, NULL},
	{"XAXES", XAXES, NULL},
	{"XAXIS", XAXIS, NULL},
	{"XMAX", XMAX, NULL},
	{"XMIN", XMIN, NULL},
	{"XY", XY, NULL},
	{"XYBOXPLOT", XYBOXPLOT, NULL},
	{"XYCOLOR", XYCOLOR, NULL},
	{"XYCOLPAT", XYCOLPAT, NULL},
	{"XYDX", XYDX, NULL},
	{"XYDXDX", XYDXDX, NULL},
	{"XYDXDXDYDY", XYDXDXDYDY, NULL},
	{"XYDXDY", XYDXDY, NULL},
	{"XYDY", XYDY, NULL},
	{"XYDYDY", XYDYDY, NULL},
	{"XYHILO", XYHILO, NULL},
	{"XYR", XYR, NULL},
	{"XYSIZE", XYSIZE, NULL},
	{"XYSTRING", XYSTRING, NULL},
	{"XYVMAP", XYVMAP, NULL},
	{"XYZ", XYZ, NULL},
	{"Y1", Y1, NULL},
	{"YAXES", YAXES, NULL},
	{"YAXIS", YAXIS, NULL},
	{"YEAR", YEAR, NULL},
	{"YMAX", YMAX, NULL},
	{"YMIN", YMIN, NULL},
	{"YYMMDD", YYMMDD, NULL},
	{"YYMMDDHMS", YYMMDDHMS, NULL},
	{"ZERO", ZERO, NULL},
	{"ZEROXAXIS", ALTXAXIS, NULL},
	{"ZEROYAXIS", ALTYAXIS, NULL},
	{"ZNORM", ZNORM, NULL}
};

static int maxfunc = sizeof(key) / sizeof(symtab_entry);

static Quark *get_parser_gno(void)
{
    return whichgraph;
}

static int set_parser_gno(Quark *gr)
{
    whichgraph = gr;
    if (gr) {
        whichframe = get_parent_frame(gr);
        return RETURN_SUCCESS;
    } else {
        whichframe = NULL;
        return RETURN_FAILURE;
    }
}

static int set_parser_setno(Quark *pset)
{
    if (pset) {
        whichgraph = get_parent_graph(pset);
        whichframe = get_parent_frame(whichgraph);
        whichset = pset;
        /* those will usually be overridden except when evaluating
           a _standalone_ vexpr */
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

static int parser(const char *s)
{
    char *seekpos;
    
    if (string_is_empty(s)) {
        return RETURN_SUCCESS;
    }
    
    strncpy(f_string, s, MAX_PARS_STRING_LENGTH - 2);
    f_string[MAX_PARS_STRING_LENGTH - 2] = '\0';
    strcat(f_string, " ");
    
    seekpos = f_string;

    while ((seekpos - f_string < MAX_PARS_STRING_LENGTH - 1) && (*seekpos == ' ' || *seekpos == '\t')) {
        seekpos++;
    }
    if (*seekpos == '\n' || *seekpos == '#') {
        /* don't consider an empty string as error for generic parser */
        return RETURN_SUCCESS;
    }
    
    lowtoupper(f_string);
        
    pos = 0;
    interr = 0;
    expr_parsed  = FALSE;
    vexpr_parsed = FALSE;
    
    yyparse();

    return (interr ? RETURN_FAILURE:RETURN_SUCCESS);
}

static int findf(symtab_entry *keytable, char *s)
{

    int low, high, mid;

    low = 0;
    high = maxfunc - 1;
    while (low <= high) {
	mid = (low + high) / 2;
	if (strcmp(s, keytable[mid].s) < 0) {
	    high = mid - 1;
	} else {
	    if (strcmp(s, keytable[mid].s) > 0) {
		low = mid + 1;
	    } else {
		return (mid);
	    }
	}
    }
    return (-1);
}

static int getcharstr(void)
{
    if (pos >= strlen(f_string))
	 return EOF;
    return (f_string[pos++]);
}

static void ungetchstr(void)
{
    if (pos > 0)
	pos--;
}

static int yylex(void)
{
    int c, i;
    int found;
    char sbuf[MAX_PARS_STRING_LENGTH + 40];

    while ((c = getcharstr()) == ' ' || c == '\t');
    if (c == EOF) {
	return (0);
    }
    if (c == '"') {
	i = 0;
	while ((c = getcharstr()) != '"' && c != EOF) {
	    if (c == '\\') {
		int ctmp;
		ctmp = getcharstr();
		if (ctmp != '"') {
		    ungetchstr();
		}
		else {
		    c = ctmp;
		}
	    }
	    sbuf[i] = c;
	    i++;
	}
	if (c == EOF) {
	    yyerror("Nonterminating string");
	    return 0;
	}
	sbuf[i] = '\0';
	yylval.sval = copy_string(NULL, sbuf);
	return CHRSTR;
    }
    if (c == '.' || isdigit(c)) {
	double d;
	int i, gotdot = 0;

	i = 0;
	while (c == '.' || isdigit(c)) {
	    if (c == '.') {
		if (gotdot) {
		    yyerror("Reading number, too many dots");
	    	    return 0;
		} else {
		    gotdot = 1;
		}
	    }
	    sbuf[i++] = c;
	    c = getcharstr();
	}
	if (c == 'E' || c == 'e') {
	    sbuf[i++] = c;
	    c = getcharstr();
	    if (c == '+' || c == '-') {
		sbuf[i++] = c;
		c = getcharstr();
	    }
	    while (isdigit(c)) {
		sbuf[i++] = c;
		c = getcharstr();
	    }
	}
	if (gotdot && i == 1) {
	    ungetchstr();
	    return '.';
	}
	sbuf[i] = '\0';
	ungetchstr();
	sscanf(sbuf, "%lf", &d);
	yylval.dval = d;
	return NUMBER;
    }
/* graphs, sets, regions resp. */
    if (c == 'G' || c == 'S' || c == 'R') {
	int i = 0, ctmp = c, gn, sn, rn;
	c = getcharstr();
	while (isdigit(c)) {
	    sbuf[i++] = c;
	    c = getcharstr();
	}
	if (i == 0) {
	    c = ctmp;
	    ungetchstr();
	} else {
	    ungetchstr();
	    if (ctmp == 'G') {
	        sbuf[i] = '\0';
                gn = atoi(sbuf);
		yylval.ival = gn;
		return GRAPHNO;
	    } else if (ctmp == 'S') {
	        sbuf[i] = '\0';
		sn = atoi(sbuf);
		yylval.ival = sn;
		return SETNUM;
	    } else if (ctmp == 'R') {
	        sbuf[i] = '\0';
		rn = atoi(sbuf);
		if (rn >= 0) {
                    yylval.ival = rn;
		    return REGNUM;
		} else {
                    errmsg("Invalid region number");
                }
	    }
	}
    }
    if (isalpha(c)) {
	char *p = sbuf;

	do {
	    *p++ = c;
	} while ((c = getcharstr()) != EOF && (isalpha(c) || isdigit(c)));
	ungetchstr();
	*p = '\0';
	found = -1;
	if ((found = findf(key, sbuf)) >= 0) {
	    yylval.ival = key[found].type;
	    return key[found].type;
	}
    }
    return c;
}

static void yyerror(char *s)
{
    char *buf;
    
    buf = copy_string(NULL, s);
    buf = concat_strings(buf, ": ");
    buf = concat_strings(buf, f_string);
    errmsg(buf);
    xfree(buf);
    interr = 1;
}

static void add_xmgr_font(Quark *project, char *fname, int mapped_id)
{
    Fontdef f;
    f.id = mapped_id;
    f.fontname = fname;
    f.fallback = fname;
    project_add_font(project, &f);
}

static void add_xmgr_fonts(Quark *project)
{    
    add_xmgr_font(project, "Times-Roman", 0);
    add_xmgr_font(project, "Times-Bold", 1);
    add_xmgr_font(project, "Times-Italic", 2);
    add_xmgr_font(project, "Times-BoldItalic", 3);
    add_xmgr_font(project, "Helvetica", 4);
    add_xmgr_font(project, "Helvetica-Bold", 5);
    add_xmgr_font(project, "Helvetica-Oblique", 6);
    add_xmgr_font(project, "Helvetica-BoldOblique", 7);
    add_xmgr_font(project, "Symbol", 8);
    add_xmgr_font(project, "ZapfDingbats", 9);
}

static Colordef cmap_init[] = {
    /* white  */
    { 0, {255, 255, 255}, "white"},
    /* black  */
    { 1, {0, 0, 0}, "black"},
    /* red    */
    { 2, {255, 0, 0}, "red"},
    /* green  */
    { 3, {0, 255, 0}, "green"},
    /* blue   */
    { 4, {0, 0, 255}, "blue"},
    /* yellow */
    { 5, {255, 255, 0}, "yellow"},
    /* brown  */
    { 6, {188, 143, 143}, "brown"},
    /* grey   */
    { 7, {220, 220, 220}, "grey"},
    /* violet */
    { 8, {148, 0, 211}, "violet"},
    /* cyan   */
    { 9, {0, 255, 255}, "cyan"},
    /* magenta*/
    {10, {255, 0, 255}, "magenta"},
    /* orange */
    {11, {255, 165, 0}, "orange"},
    /* indigo */
    {12, {114, 33, 188}, "indigo"},
    /* maroon */
    {13, {103, 7, 72}, "maroon"},
    /* turquoise */
    {14, {64, 224, 208}, "turquoise"},
    /* forest green */
    {15, {0, 139, 0}, "green4"}
};

static void add_xmgr_colors(Quark *project)
{    
    int i, n;
    
    n = sizeof(cmap_init)/sizeof(Colordef);
    for (i = 0; i < n; i++) {
        project_add_color(project, &cmap_init[i]);
    }
}

static Quark *allocate_graph(Quark *project, int gno)
{
    Quark *gr = NULL;
    char buf[32];
    
    if (gno >= 0) {
        sprintf(buf, "G%02d", gno);
        gr = quark_find_descendant_by_idstr(project, buf);
        if (!gr) {
            gr = graph_next(project);
            quark_idstr_set(gr, buf);
            
            /* assign an idstr to the frame, too */
            sprintf(buf, "F%02d", gno);
            quark_idstr_set(get_parent_frame(gr), buf);
        }
    }
    
    return gr;
}

static Quark *allocate_region(Quark *gr, int rn)
{
    Quark *r = NULL;
    char buf[32];
    
    if (rn >= 0) {
        sprintf(buf, "R%d", rn);
        r = quark_find_descendant_by_idstr(gr, buf);
        if (!r) {
            r = region_new(gr);
            quark_idstr_set(r, buf);
            quark_set_active(r, FALSE);
        }
    }
    
    return r;
}

static Quark *allocate_set(Quark *gr, int setno)
{
    Quark *pset = NULL;
    char buf[32];
    
    if (setno >= 0) {
        sprintf(buf, "S%02d", setno);
        pset = quark_find_descendant_by_idstr(gr, buf);
        if (!pset) {
            pset = set_new(gr);
            quark_idstr_set(pset, buf);
        }
    }
    
    return pset;
}

static Quark *get_target_set(void)
{
    return target_set;
}

static void set_target_set(Quark *pset)
{
    target_set = pset;
}

static void parser_state_reset(Quark *pr)
{
    project = pr;
    if (project) {
        set_parser_gno(graph_get_current(project));
    } else {
        whichframe = NULL;
        whichgraph = NULL;
    }
    whichset      = NULL;
    target_set    = NULL;
    whichaxisgrid = NULL;
    normaxis      = NULL;
    oppaxis       = NULL;
    curtm         = NULL;
    curobject     = NULL;
    objgno        = NULL;
}


static Quark *nextset(Quark *ss)
{
    Quark *pset, *gr, **sets, *ssd;
    int nsets = 0;
    
    pset = get_target_set();
    
    if (pset) {
        set_target_set(NULL);
    } else {
        gr = get_parent_graph(ss);
        nsets = quark_get_descendant_sets(gr, &sets);
        if (nsets) {
            pset = sets[0];
        } else {
            ssd = get_parent_ssd(ss);
            if (!ssd) {
                ssd = ssd_new(ss);
            }
            pset = set_new(ssd);
        }
    }
    
    if (nsets) {
        xfree(sets);
    }
    
    return pset;
}

static int agr_parse_cb(const char *s, void *udata)
{
    if (*s == '@') {
        return parser(s + 1);
    } else
    if (*s == '&') {
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

static int agr_store_cb(Quark *q, void *udata)
{
    Quark *gr, *pset;

    gr = get_parser_gno();
    if (!gr) {
        return RETURN_FAILURE;
    }
    
    if (quark_reparent(q, gr) != RETURN_SUCCESS) {
        return RETURN_FAILURE;
    }
    
    pset = nextset(q);
    
    return quark_reparent(pset, q);
}

static int fcomp(const Quark *q1, const Quark *q2, void *udata)
{
    if (quark_fid_get(q1) == QFlavorAText && quark_fid_get(q2) == QFlavorAText) {
        return strcmp(QIDSTR(q1), QIDSTR(q2));
    } else
    if (quark_fid_get(q1) == QFlavorAText) {
        return 1;
    } else
    if (quark_fid_get(q2) == QFlavorAText) {
        return -1;
    } else
    if (quark_fid_get(q1) == QFlavorDObject && quark_fid_get(q2) == QFlavorDObject) {
        DObject *o1 = object_get_data(q1), *o2 = object_get_data(q2);
        if (o1->type != o2->type) {
            return (o1->type - o2->type);
        } else {
            return strcmp(QIDSTR(q1), QIDSTR(q2));
        }
    } else
    if (quark_fid_get(q1) == QFlavorDObject) {
        return 1;
    } else
    if (quark_fid_get(q2) == QFlavorDObject) {
        return -1;
    } else
    if (quark_fid_get(q1) == QFlavorAGrid) {
        tickmarks *t = axisgrid_get_data(q1);
        if (t->gprops.onoff || t->mgprops.onoff) {
            return -1;
        } else {
            return 1;
        }
    } else
    if (quark_fid_get(q2) == QFlavorAGrid) {
        tickmarks *t = axisgrid_get_data(q2);
        if (t->gprops.onoff || t->mgprops.onoff) {
            return 1;
        } else {
            return -1;
        }
    } else
    if (quark_fid_get(q1) == quark_fid_get(q2)) {
        return strcmp(QIDSTR(q1), QIDSTR(q2));
    } else {
        return 0;
    }
}

/* TODO */
#define MAGIC_FONT_SCALE    0.028
#define MAGIC_LINEW_SCALE   0.0015

static int project_postprocess_hook(Quark *q,
    void *udata, QTraverseClosure *closure)
{
    int version_id = *((int *) udata);
    Project *pr;
    frame *f;
    tickmarks *t;
    AText *at;
    DObject *o;
    Quark *gr;
    set *s;
    int gtype;
    
    switch (quark_fid_get(q)) {
    case QFlavorProject:
        pr = project_get_data(q);
        
        project_set_fontsize_scale(q, MAGIC_FONT_SCALE);
        project_set_linewidth_scale(q, MAGIC_LINEW_SCALE);

        if (version_id < 40005) {
            project_set_page_dimensions(q, 792, 612);
        }

        if (version_id < 50002) {
            pr->bgfill = TRUE;
        }

        if (version_id < 50003) {
            pr->two_digits_years = TRUE;
            pr->wrap_year = 1900;
        }

        if (version_id <= 40102) {
            double ext_x, ext_y;
#ifndef NONE_GUI
            GUI *gui = gui_from_quark(q);
            gui_set_page_free(gui, FALSE);
#endif
            if (project_get_viewport(q, &ext_x, &ext_y) == RETURN_SUCCESS) {
                rescale_viewport(q, ext_x, ext_y);
            }
        }
        break;
    case QFlavorFrame:
        f = frame_get_data(q);
        
	if (version_id <= 40102) {
            f->l.vgap -= 0.01;
        }

        break;
    case QFlavorGraph:

        break;
    case QFlavorAGrid:
        /* kill inactive axes in old projects */
        if (!quark_is_active(q)) {
            quark_free(q);
            closure->descend = FALSE;
            break;
        }

	t = axisgrid_get_data(q);

        if (version_id <= 40102) {
            if ((axisgrid_is_x(q) && islogx(q)) ||
                (axisgrid_is_y(q) && islogy(q))) {
                t->tmajor = pow(10.0, t->tmajor);
            }
        }
        if (version_id < 50105) {
            /* Starting with 5.1.5, X axis min & inverting is honored
               in pie charts */
            if (graph_get_type(q) == GRAPH_PIE) {
                world w;
                graph_get_world(q, &w);
                w.xg1 = 0.0;
                w.xg2 = 2*M_PI;
                graph_set_world(q, &w);
                graph_set_xinvert(q, FALSE);
            }
        }
        if (version_id < 50991) {
            /* Separate drawing props for grid lines introduced in 5.99.1 */
            t->gprops.line  = t->props.line;
            t->mgprops.line = t->mprops.line;
        }

        break;
    case QFlavorAxis:
        if (version_id <= 40102) {
            /* TODO : world/view translation */
        }
        break;
    case QFlavorSet:
        s = set_get_data(q);
        gtype = graph_get_type(get_parent_graph(q));

        if (version_id < 50000) {
            switch (s->sym.type) {
            case SYM_NONE:
                break;
            case SYM_DOT_OBS:
                s->sym.type = SYM_CIRCLE;
                s->sym.size = 0.0;
                s->sym.line.style = 0;
                s->sym.fillpen.pattern = 1;
                break;
            default:
                s->sym.type--;
                break;
            }
        }
        if ((version_id < 40004 && gtype != GRAPH_CHART) ||
            s->sym.line.pen.color == -1) {
            s->sym.line.pen.color = s->line.line.pen.color;
        }
        if (version_id < 40200 || s->sym.fillpen.color == -1) {
            s->sym.fillpen.color = s->sym.line.pen.color;
        }

	if (version_id < 30000) {
            s->line.fillpen.pattern = 1;
            s->sym.fillpen.pattern  = 1;
        }
	if (version_id <= 40102 && gtype == GRAPH_CHART) {
            s->type       = SET_BAR;
            s->sym.line    = s->line.line;
            s->line.line.style = 0;

            s->sym.fillpen = s->line.fillpen;
            s->line.fillpen.pattern = 0;
        }
	if (version_id <= 40102 && s->type == SET_XYHILO) {
            s->sym.line.width = s->line.line.width;
        }
	if (version_id <= 50112 && s->type == SET_XYHILO) {
            s->avalue.active = FALSE;
        }
	if (version_id < 50100 && s->type == SET_BOXPLOT) {
            s->sym.line.width = s->line.line.width;
            s->sym.line.style = s->line.line.style;
            s->sym.size = 2.0;
            s->errbar.riser_linew = s->line.line.width;
            s->errbar.riser_lines = s->line.line.style;
            s->line.line.style = 0;
            s->errbar.barsize = 0.0;
        }
        if (version_id < 50003) {
            s->errbar.active = TRUE;
            s->errbar.pen.color = s->sym.line.pen.color;
            s->errbar.pen.pattern = 1;
            
            if (s->type == SET_XY || s->type == SET_BAR) {
                iswap(&s->ds.cols[DATA_Y1], &s->ds.cols[DATA_Y2]);
                iswap(&s->ds.cols[DATA_Y3], &s->ds.cols[DATA_Y4]);
            }
        }
        if (version_id < 50002) {
            s->errbar.barsize *= 2;
        }

        if (version_id < 50107) {
            /* Starting with 5.1.7, symskip is honored for all set types */
            switch (s->type) {
            case SET_BAR:
            case SET_XYHILO:
            case SET_XYR:
            case SET_XYVMAP:
            case SET_BOXPLOT:
                s->symskip = 0;
                break;
            }
        }
        
        break;
    case QFlavorDObject:
        o = object_get_data(q);
        gr = get_parent_graph(q);
        if (object_get_loctype(q) == COORD_WORLD) {
            WPoint wp;
            VPoint vp1, vp2;

            switch (o->type) {
            case DO_BOX:
                {
                    DOBoxData *b = (DOBoxData *) o->odata;
                    wp.x = o->ap.x - b->width/2;
                    wp.y = o->ap.y - b->height/2;
                    Wpoint2Vpoint(gr, &wp, &vp1);
                    wp.x = o->ap.x + b->width/2;
                    wp.y = o->ap.y + b->height/2;
                    Wpoint2Vpoint(gr, &wp, &vp2);

                    b->width  = fabs(vp2.x - vp1.x);
                    b->height = fabs(vp2.y - vp1.y);
                }
                break;
            case DO_ARC:
                {
                    DOArcData *a = (DOArcData *) o->odata;
                    wp.x = o->ap.x - a->width/2;
                    wp.y = o->ap.y - a->height/2;
                    Wpoint2Vpoint(gr, &wp, &vp1);
                    wp.x = o->ap.x + a->width/2;
                    wp.y = o->ap.y + a->height/2;
                    Wpoint2Vpoint(gr, &wp, &vp2);

                    a->width  = fabs(vp2.x - vp1.x);
                    a->height = fabs(vp2.y - vp1.y);
                }
                break;
            case DO_LINE:
                {
                    DOLineData *l = (DOLineData *) o->odata;
                    wp.x = o->ap.x;
                    wp.y = o->ap.y;
                    Wpoint2Vpoint(gr, &wp, &vp1);
                    wp.x = o->ap.x + l->vector.x;
                    wp.y = o->ap.y + l->vector.y;
                    Wpoint2Vpoint(gr, &wp, &vp2);

                    l->vector.x = vp2.x - vp1.x;
                    l->vector.y = vp2.y - vp1.y;
                }
                break;
            case DO_NONE:
                break;
            }
        }

        break;
    case QFlavorAText:
        at = atext_get_data(q);
        if (version_id >= 40200 && version_id <= 50005        &&
            !strings_are_equal(quark_idstr_get(q), "timestamp") &&
            !strings_are_equal(quark_idstr_get(q), "title")     &&
            !strings_are_equal(quark_idstr_get(q), "subtitle")  &&
            !strings_are_equal(quark_idstr_get(q), "label")) {
            /* BBox type justification was erroneously set */
            if (at) {
                at->text_props.just |= JUST_MIDDLE;
            }
        }
        /* kill inactive labels in old projects */
        if (at && string_is_empty(at->s)) {
            quark_free(q);
            closure->descend = FALSE;
        }
        break;
    case QFlavorRegion:
        /* kill inactive regions in old projects */
        if (!quark_is_active(q)) {
            quark_free(q);
            closure->descend = FALSE;
        }
        break;
    }
    
    return TRUE;
}

static void project_postprocess(Quark *project)
{
    int version_id = project_get_version_id(project);
    
    quark_sort_children(project, fcomp, NULL);
    
    quark_traverse(project, project_postprocess_hook, &version_id);
}

GProject *load_agr_project(GraceApp *gapp, const char *fn)
{
    GProject *gp;
    Quark *project;
    FILE *fp;
    int retval;

    fp = gapp_openr(gapp, fn, SOURCE_DISK);
    if (fp == NULL) {
	return NULL;
    }
    
    gp = gproject_new(gapp->grace, AMEM_MODEL_LIBUNDO);
    project = gproject_get_top(gp);

    parser_state_reset(project);
    
    retval = uniread(project, fp, agr_parse_cb, agr_store_cb, NULL);

    gapp_close(fp);

    if (retval == RETURN_SUCCESS) {
        struct stat statb;
        time_t mtime;

        if (fn && !stat(fn, &statb)) {
            mtime = statb.st_mtime;
        } else {
            mtime = 0;
        }

        project_postprocess(project);

        project_update_timestamp(project, mtime);

        /* Clear dirtystate */
        quark_dirtystate_set(project, FALSE);

        gp->grf = grfile_new(fn);

        return gp;
    } else {
        gproject_free(gp);
        
        return NULL;
    }
}
