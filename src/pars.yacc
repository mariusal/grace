%{
/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
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
#if defined(HAVE_SYS_PARAM_H)
#  include <sys/param.h>
#endif

/* bison not always handles it well itself */
#if defined(HAVE_ALLOCA_H)
#  include <alloca.h>
#endif

#include "defines.h"
#include "globals.h"
#include "grace.h"
#include "cephes.h"
#include "utils.h"
#include "files.h"
#include "core_utils.h"
#include "plotone.h"
#include "dlmodule.h"
#include "ssdata.h"
#include "protos.h"
#include "parser.h"
#include "mathstuff.h"


#define MAX_PARS_STRING_LENGTH  4096

#define canvas grace->rt->canvas

/* Tick sign type (obsolete) */
#define SIGN_NORMAL     0
#define SIGN_ABSOLUTE   1
#define SIGN_NEGATE     2

#define CAST_DBL_TO_BOOL(x) (fabs(x) < 0.5 ? 0:1)

typedef double (*ParserFnc)();

/* the graph, set, axis, and object of the parser's current state */
static Quark *whichframe;
static Quark *whichgraph;
static Quark *whichset;
static tickmarks *curtm;
static DObject *curobject;
static Quark *curatext;
static Quark *objgno;
static int curobject_loctype = COORD_VIEW;
static int dobject_id = 0;

static double  s_result;    /* return value if a scalar expression is scanned*/
static grarr *v_result;    /* return value if a vector expression is scanned*/

static int expr_parsed, vexpr_parsed;

static int interr;

static grarr freelist[100]; 	/* temporary vectors */
static int fcnt = 0;		/* number of the temporary vectors allocated */

/* this one attempts to avoid reentrancy problems */
static int gotparams = FALSE; 
static char paramfile[GR_MAXPATHLEN] = "";

static char f_string[MAX_PARS_STRING_LENGTH]; /* buffer for string to parse */
static unsigned int pos;


/* the graph and set of the left part of a vector assignment */
static Quark *vasgn_pset;

static int alias_force = FALSE; /* controls whether aliases can override
                                                       existing keywords */

extern char *close_input;

static int filltype_obs;

static int leg_loctype_obs;
static double leg_x1_obs;

static int index_shift = 0;     /* 0 for C, 1 for F77 index notation */

static void free_tmpvrbl(grarr *vrbl);
static void copy_vrbl(grarr *dest, grarr *src);

static int getcharstr(void);
static void ungetchstr(void);
static int follow(int expect, int ifyes, int ifno);

static int findf(symtab_entry *keytable, char *s);

static void add_xmgr_fonts(Quark *project);

static Quark *allocate_graph(Quark *project, int gno);
static Quark *allocate_set(Quark *gr, int setno);
static Quark *allocate_region(Quark *gr, int rn);

/* Total (intrinsic + user-defined) list of functions and keywords */
symtab_entry *key;

static int yylex(void);
static int yyparse(void);
static void yyerror(char *s);

%}

%union {
    int     ival;
    double  dval;
    char   *sval;
    double *dptr;
    Quark  *quark;
    grarr  *vrbl;
}

%token KEY_VAR       
%token KEY_VEC       

%token KEY_CONST     
%token KEY_UNIT      
%token KEY_FUNC_I    
%token KEY_FUNC_D    
%token KEY_FUNC_NN   
%token KEY_FUNC_ND   
%token KEY_FUNC_DD   
%token KEY_FUNC_NND  
%token KEY_FUNC_PPD  
%token KEY_FUNC_PPPD 

%token <ival> DATE

%token <dptr> VAR_D	 /* a (pointer to) double variable                                     */
%token <vrbl> VEC_D	 /* a (pointer to) double array variable                                     */

%token <ival> CONSTANT	 /* a (double) constant                                     */
%token <ival> UCONSTANT	 /* a (double) unit constant                                */
%token <ival> FUNC_I	 /* a function of 1 int variable                            */
%token <ival> FUNC_D	 /* a function of 1 double variable                         */
%token <ival> FUNC_NN    /* a function of 2 int parameters                          */
%token <ival> FUNC_ND    /* a function of 1 int parameter and 1 double variable     */
%token <ival> FUNC_DD    /* a function of 2 double variables                        */
%token <ival> FUNC_NND   /* a function of 2 int parameters and 1 double variable    */
%token <ival> FUNC_PPD   /* a function of 2 double parameters and 1 double variable */
%token <ival> FUNC_PPPD  /* a function of 3 double parameters and 1 double variable */

%token <ival> ABOVE
%token <ival> ABSOLUTE
%token <ival> ALIAS
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
%token <ival> CLEAR
%token <ival> CLICK
%token <ival> CLIP
%token <ival> COLOR
%token <ival> COMMENT
%token <ival> DAYMONTH
%token <ival> DAYOFWEEKL
%token <ival> DAYOFWEEKS
%token <ival> DAYOFYEAR
%token <ival> DDMMYY
%token <ival> DECIMAL
%token <ival> DEF
%token <ival> DEFAULT
%token <ival> DEFINE
%token <ival> DEGREESLAT
%token <ival> DEGREESLON
%token <ival> DEGREESMMLAT
%token <ival> DEGREESMMLON
%token <ival> DEGREESMMSSLAT
%token <ival> DEGREESMMSSLON
%token <ival> DESCENDING
%token <ival> DESCRIPTION
%token <ival> DEVICE
%token <ival> DISK
%token <ival> DROPLINE
%token <ival> ECHO
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
%token <ival> FROM
%token <ival> GENERAL
%token <ival> GETP
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
%token <ival> IFILTER
%token <ival> IN
%token <ival> INCREMENT
%token <ival> INOUT
%token <ival> INVERT
%token <ival> JUST
%token <ival> KILL
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
%token <ival> MAGIC
%token <ival> MAJOR
%token <ival> MAP
%token <ival> MAXP
%token <ival> MESH
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
%token <ival> OFILTER
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
%token <ival> RAND
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
%token <ival> USE
%token <ival> VERSION
%token <ival> VERTI
%token <ival> VERTICAL
%token <ival> VERTO
%token <ival> VGAP
%token <ival> VIEW
%token <ival> VX1
%token <ival> VX2
%token <ival> VXMAX
%token <ival> VY1
%token <ival> VY2
%token <ival> VYMAX
%token <ival> WITH
%token <ival> WORLD
%token <ival> WRAP
%token <ival> WX1
%token <ival> WX2
%token <ival> WY1
%token <ival> WY2
%token <ival> X_TOK
%token <ival> X0
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
%token <ival> Y_TOK
%token <ival> Y0
%token <ival> Y1
%token <ival> Y2
%token <ival> Y3
%token <ival> Y4
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

%token <sval> NEW_TOKEN

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

%type <ival> formatchoice
%type <ival> inoutchoice
%type <ival> justchoice

%type <ival> opchoice
%type <ival> opchoice_sel
%type <ival> opchoice_obs
%type <ival> opchoice_sel_obs

%type <ival> worldview

%type <ival> filtermethod
%type <ival> filtertype

%type <ival> tickspectype

%type <ival> sourcetype

%type <ival> objecttype

%type <ival> stattype

%type <ival> datacolumn

%type <ival> proctype

%type <ival> indx
%type <ival> iexpr
%type <ival> nexpr
%type <dval> jdate
%type <dval> jrawdate
%type <dval> expr

%type <vrbl> array
%type <vrbl> lside_array

%type <vrbl> vexpr

/* Precedence */
%nonassoc '?' ':'
%left OR
%left AND
%nonassoc GT LT LE GE EQ NE
%right UCONSTANT
%left '+' '-'
%left '*' '/' '%'
%nonassoc UMINUS NOT	/* negation--unary minus */
%right '^'		/* exponentiation        */


%%

full_list:
        multi_list
        | expr {
            expr_parsed = TRUE;
            s_result = $1;
        }
        | vexpr {
            vexpr_parsed = TRUE;
            v_result = $1;
        }
        ;

multi_list:
        list
        | multi_list ';' list
        ;

list:
	parmset {}
	| parmset_obs {}
	| regionset {}
	| setaxis {}
	| set_setprop {}
	| actions {}
	| options {}
	| asgn {}
	| vasgn {}
	| defines {}
	| error {
	    return 1;
	}
	;



expr:	NUMBER {
	    $$ = $1;
	}
	|  VAR_D {
	    $$ = *($1);
	}
	|  array indx {
            if ($2 >= $1->length) {
                errmsg("Access beyond array bounds");
                return 1;
            }
            $$ = $1->data[$2];
	}
	| stattype '(' vexpr ')' {
	    double dummy;
            int length = $3->length;
	    if ($3->data == NULL) {
		yyerror("NULL variable, check set type");
		return 1;
	    }
	    switch ($1) {
	    case MINP:
		$$ = vmin($3->data, length);
		break;
	    case MAXP:
		$$ = vmax($3->data, length);
		break;
            case AVG:
		stasum($3->data, length, &$$, &dummy);
                break;
            case SD:
		stasum($3->data, length, &dummy, &$$);
                break;
	    }
	}
	| VEC_D '.' LENGTH {
	    $$ = $1->length;
	}
	| selectset '.' LENGTH {
	    $$ = set_get_length($1);
	}
	| CONSTANT
	{
            $$ = ((ParserFnc) (key[$1].data)) ();
	}
	| expr UCONSTANT
	{
	    $$ = $1 * ((ParserFnc) (key[$2].data)) ();
	}
	| RAND
	{
	    $$ = drand48();
	}
	| FUNC_I '(' iexpr ')'
	{
	    $$ = ((ParserFnc) (key[$1].data)) ($3);
	}
	| FUNC_D '(' expr ')'
	{
	    $$ = ((ParserFnc) (key[$1].data)) ($3);
	}
	| FUNC_ND '(' iexpr ',' expr ')'
	{
	    $$ = ((ParserFnc) (key[$1].data)) ($3, $5);
	}
	| FUNC_NN '(' iexpr ',' iexpr ')'
	{
	    $$ = ((ParserFnc) (key[$1].data)) ($3, $5);
	}
	| FUNC_DD '(' expr ',' expr ')'
	{
	    $$ = ((ParserFnc) (key[$1].data)) ($3, $5);
	}
	| FUNC_NND '(' iexpr ',' iexpr ',' expr ')'
	{
	    $$ = ((ParserFnc) (key[$1].data)) ($3, $5, $7);
	}
	| FUNC_PPD '(' expr ',' expr ',' expr ')'
	{
	    $$ = ((ParserFnc) (key[$1].data)) ($3, $5, $7);
	}
	| FUNC_PPPD '(' expr ',' expr ',' expr ',' expr ')'
	{
	    $$ = ((ParserFnc) (key[$1].data)) ($3, $5, $7, $9);
	}
	| DATE '(' jdate ')' {
            $$ = $3;
	}
	| DATE '(' iexpr ',' nexpr ',' nexpr ')' { /* yr, mo, day */
	    $$ = cal_and_time_to_jul($3, $5, $7, 12, 0, 0.0);
	}
	| DATE '(' iexpr ',' nexpr ',' nexpr ',' nexpr ',' nexpr ',' expr ')' 
	{ /* yr, mo, day, hr, min, sec */
	    $$ = cal_and_time_to_jul($3, $5, $7, $9, $11, $13);
	}
	| VXMAX {
	    double vx, vy;
            project_get_viewport(grace->project, &vx, &vy);
            $$ = vx;
	}
	| VYMAX {
	    double vx, vy;
            project_get_viewport(grace->project, &vx, &vy);
            $$ = vy;
	}
	| '(' expr ')' {
	    $$ = $2;
	}
	| expr '+' expr {
	    $$ = $1 + $3;
	}
	| expr '-' expr {
	    $$ = $1 - $3;
	}
	| '-' expr %prec UMINUS {
	    $$ = -$2;
	}
	| '+' expr %prec UMINUS {
	    $$ = $2;
	}
	| expr '*' expr {
	    $$ = $1 * $3;
	}
	| expr '/' expr
	{
	    if ($3 != 0.0) {
		$$ = $1 / $3;
	    } else {
		yyerror("Divide by zero");
		return 1;
	    }
	}
	| expr '%' expr {
	    if ($3 != 0.0) {
		$$ = fmod($1, $3);
	    } else {
		yyerror("Divide by zero");
		return 1;
	    }
	}
	| expr '^' expr {
	    if ($1 < 0 && rint($3) != $3) {
		yyerror("Negative value raised to non-integer power");
		return 1;
            } else if ($1 == 0.0 && $3 <= 0.0) {
		yyerror("Zero raised to non-positive power");
		return 1;
            } else {
                $$ = pow($1, $3);
            }
	}
	| expr '?' expr ':' expr {
	    $$ = $1 ? $3 : $5;
	}
	| expr GT expr {
	   $$ = ($1 > $3);
	}
	| expr LT expr  {
	   $$ = ($1 < $3);
	}
	| expr LE expr {
	   $$ = ($1 <= $3);
	}
	| expr GE expr {
	   $$ = ($1 >= $3);
	}
	| expr EQ expr {
	   $$ = ($1 == $3);
	}
	| expr NE expr {
	    $$ = ($1 != $3);
	}
	| expr AND expr {
	    $$ = $1 && $3;
	}
	| expr OR expr {
	    $$ = $1 || $3;
	}
	| NOT expr {
	    $$ = !($2);
	}
	;

jdate:  expr {
            $$ = $1;
        }
        | CHRSTR {
            double jul;
            Dates_format dummy;
            if (parse_date(grace->project, $1, get_date_hint(), FALSE, &jul, &dummy)
                == RETURN_SUCCESS) {
                xfree($1);
                $$ = jul;
            } else {
                xfree($1);
		yyerror("Invalid date");
		return 1;
            }
        }
        ;

jrawdate:  expr {
            $$ = $1;
        }
        | CHRSTR {
            double jul;
            Dates_format dummy;
            if (parse_date(grace->project, $1, get_date_hint(), TRUE, &jul, &dummy)
                == RETURN_SUCCESS) {
                xfree($1);
                $$ = jul;
            } else {
                xfree($1);
		yyerror("Invalid date");
		return 1;
            }
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

indx:	'[' iexpr ']' {
	    int itmp = $2 - index_shift;
            if (itmp < 0) {
		yyerror("Negative index");
		return 1;
            }
            $$ = itmp;
	}
        ;

array:
	VEC_D
	{
            $$ = $1;
	}
        | datacolumn
	{
	    double *ptr = set_get_col(vasgn_pset, $1);
            $$ = &freelist[fcnt++];
            $$->type = GRARR_SET;
            $$->data = ptr;
            if (ptr == NULL) {
                errmsg("NULL variable - check set type");
                return 1;
            } else {
                $$->length = set_get_length(vasgn_pset);
            }
	}
	| selectset '.' datacolumn
	{
	    double *ptr = set_get_col($1, $3);
            $$ = &freelist[fcnt++];
            $$->type = GRARR_SET;
            $$->data = ptr;
            if (ptr == NULL) {
                errmsg("NULL variable - check set type");
                return 1;
            } else {
                $$->length = set_get_length($1);
            }
	}
        ;
        
vexpr:
	array
	{
            $$ = $1;
	}
	| MESH '(' nexpr ')'
	{
            int len = $3;
            if (len < 1) {
                yyerror("npoints must be > 0");
            } else {
                double *ptr = allocate_index_data(len);
                if (ptr == NULL) {
                    errmsg("Malloc failed");
                    return 1;
                } else {
                    $$ = &freelist[fcnt++];
                    $$->type = GRARR_TMP;
                    $$->data = ptr;
                    $$->length = len;
                }
            }
	}
	| MESH '(' expr ',' expr ',' nexpr ')'
	{
            int len = $7;
            if (len < 2) {
                yyerror("npoints must be > 1");
            } else {
                double *ptr = allocate_mesh($3, $5, len);
                if (ptr == NULL) {
                    errmsg("Malloc failed");
                    return 1;
                } else {
                    $$ = &freelist[fcnt++];
                    $$->type = GRARR_TMP;
                    $$->data = ptr;
                    $$->length = len;
                }
            }
	}
	| RAND '(' nexpr ')'
	{
	    int i;
            $$ = &freelist[fcnt++];
	    $$->data = xmalloc($3*SIZEOF_DOUBLE);
            if ($$->data == NULL) {
                errmsg("Not enough memory");
                return 1;
            } else {
                $$->length = $3;
                $$->type = GRARR_TMP;
            }
            for (i = 0; i < $$->length; i++) {
		$$->data[i] = drand48();
	    }
	}
	| FUNC_I '(' vexpr ')'
	{
	    int i;
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $3);
            $$->type = GRARR_TMP;
	    for (i = 0; i < $$->length; i++) {
		$$->data[i] = ((ParserFnc) (key[$1].data)) ((int) ($3->data[i]));
	    }
	}
	| FUNC_D '(' vexpr ')'
	{
	    int i;
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $3);
            $$->type = GRARR_TMP;
	    for (i = 0; i < $$->length; i++) {
		$$->data[i] = ((ParserFnc) (key[$1].data)) (($3->data[i]));
	    }
	}
	| FUNC_DD '(' vexpr ',' vexpr ')'
	{
	    int i;
	    if ($3->length != $5->length) {
                errmsg("Can't operate on vectors of different lengths");
                return 1;
            }
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $3);
            $$->type = GRARR_TMP;
            
	    for (i = 0; i < $$->length; i++) {
		$$->data[i] = ((ParserFnc) (key[$1].data)) ($3->data[i], $5->data[i]);
	    }
	}
	| FUNC_DD '(' expr ',' vexpr ')'
	{
	    int i;
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $5);
            $$->type = GRARR_TMP;
            
	    for (i = 0; i < $$->length; i++) {
		$$->data[i] = ((ParserFnc) (key[$1].data)) ($3, $5->data[i]);
	    }
	}
	| FUNC_DD '(' vexpr ',' expr ')'
	{
	    int i;
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $3);
            $$->type = GRARR_TMP;
            
	    for (i = 0; i < $$->length; i++) {
		$$->data[i] = ((ParserFnc) (key[$1].data)) ($3->data[i], $5);
	    }
	}
	| FUNC_ND '(' iexpr ',' vexpr ')'
	{
	    int i;
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $5);
            $$->type = GRARR_TMP;

	    for (i = 0; i < $$->length; i++) {
		$$->data[i] = ((ParserFnc) (key[$1].data)) ($3, $5->data[i]);
	    }
	}
	| FUNC_NND '(' iexpr ',' iexpr ',' vexpr ')'
	{
	    int i;
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $7);
            $$->type = GRARR_TMP;

	    for (i = 0; i < $$->length; i++) {
		$$->data[i] = ((ParserFnc) (key[$1].data)) ($3, $5, $7->data[i]);
	    }
	}
	| FUNC_PPD '(' expr ',' expr ',' vexpr ')'
	{
	    int i;
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $7);
            $$->type = GRARR_TMP;

	    for (i = 0; i < $$->length; i++) {
		$$->data[i] = ((ParserFnc) (key[$1].data)) ($3, $5, $7->data[i]);
	    }
	}
	| FUNC_PPPD '(' expr ',' expr ',' expr ',' vexpr ')'
	{
	    int i;
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $9);
            $$->type = GRARR_TMP;

	    for (i = 0; i < $$->length; i++) {
		$$->data[i] = ((ParserFnc) (key[$1].data)) ($3, $5, $7, $9->data[i]);
	    }
	}
	| vexpr '+' vexpr
	{
	    int i;
	    if ($1->length != $3->length) {
                errmsg("Can't operate on vectors of different lengths");
                return 1;
            }
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $1);
            $$->type = GRARR_TMP;

	    for (i = 0; i < $$->length; i++) {
		$$->data[i] = $1->data[i] + $3->data[i];
	    }
	}
	| vexpr '+' expr
	{
	    int i;
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $1);
            $$->type = GRARR_TMP;

	    for (i = 0; i < $$->length; i++) {
		$$->data[i] = $1->data[i] + $3;
	    }
	}
	| expr '+' vexpr
	{
	    int i;
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $3);
            $$->type = GRARR_TMP;

	    for (i = 0; i < $$->length; i++) {
		$$->data[i] = $1 + $3->data[i];
	    }
	}
	| vexpr '-' vexpr
	{
	    int i;
	    if ($1->length != $3->length) {
                errmsg("Can't operate on vectors of different lengths");
                return 1;
            }
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $1);
            $$->type = GRARR_TMP;

	    for (i = 0; i < $$->length; i++) {
		$$->data[i] = $1->data[i] - $3->data[i];
	    }
	}
	| vexpr '-' expr
	{
	    int i;
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $1);
            $$->type = GRARR_TMP;

	    for (i = 0; i < $$->length; i++) {
		$$->data[i] = $1->data[i] - $3;
	    }
	}
	| expr '-' vexpr
	{
	    int i;
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $3);
            $$->type = GRARR_TMP;

	    for (i = 0; i < $$->length; i++) {
		$$->data[i] = $1 - $3->data[i];
	    }
	}
	| vexpr '*' vexpr
	{
	    int i;
	    if ($1->length != $3->length) {
                errmsg("Can't operate on vectors of different lengths");
                return 1;
            }
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $1);
            $$->type = GRARR_TMP;

	    for (i = 0; i < $$->length; i++) {
		$$->data[i] = $1->data[i] * $3->data[i];
	    }
	}
	| vexpr '*' expr
	{
	    int i;
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $1);
            $$->type = GRARR_TMP;

	    for (i = 0; i < $$->length; i++) {
		$$->data[i] = $1->data[i] * $3;
	    }
	}
	| expr '*' vexpr
	{
	    int i;
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $3);
            $$->type = GRARR_TMP;

	    for (i = 0; i < $$->length; i++) {
		$$->data[i] = $1 * $3->data[i];
	    }
	}
	| vexpr '/' vexpr
	{
	    int i;
	    if ($1->length != $3->length) {
                errmsg("Can't operate on vectors of different lengths");
                return 1;
            }
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $1);
            $$->type = GRARR_TMP;

	    for (i = 0; i < $$->length; i++) {
		if ($3->data[i] == 0.0) {
                    errmsg("Divide by zero");
                    return 1;
                }
                $$->data[i] = $1->data[i] / $3->data[i];
	    }
	}
	| vexpr '/' expr
	{
	    int i;
	    if ($3 == 0.0) {
                errmsg("Divide by zero");
                return 1;
            }
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $1);
            $$->type = GRARR_TMP;

	    for (i = 0; i < $$->length; i++) {
		$$->data[i] = $1->data[i] / $3;
	    }
	}
	| expr '/' vexpr
	{
	    int i;
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $3);
            $$->type = GRARR_TMP;

	    for (i = 0; i < $$->length; i++) {
		if ($3->data[i] == 0.0) {
                    errmsg("Divide by zero");
                    return 1;
                }
		$$->data[i] = $1 / $3->data[i];
	    }
	}
	| vexpr '%' vexpr
	{
	    int i;
	    if ($1->length != $3->length) {
                errmsg("Can't operate on vectors of different lengths");
                return 1;
            }
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $1);
            $$->type = GRARR_TMP;

	    for (i = 0; i < $$->length; i++) {
		if ($3->data[i] == 0.0) {
                    errmsg("Divide by zero");
                    return 1;
                } else {
                    $$->data[i] = fmod($1->data[i], $3->data[i]);
                }
	    }
	}
	| vexpr '%' expr
	{
	    int i;
	    if ($3 == 0.0) {
                errmsg("Divide by zero");
                return 1;
            }
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $1);
            $$->type = GRARR_TMP;

	    for (i = 0; i < $$->length; i++) {
		$$->data[i] = fmod($1->data[i], $3);
	    }
	}
	| expr '%' vexpr
	{
	    int i;
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $3);
            $$->type = GRARR_TMP;

	    for (i = 0; i < $$->length; i++) {
		if ($3->data[i] == 0.0) {
                    errmsg("Divide by zero");
                    return 1;
                } else {
		    $$->data[i] = fmod($1, $3->data[i]);
                }
	    }
	}
	| vexpr '^' vexpr
	{
	    int i;
	    if ($1->length != $3->length) {
                errmsg("Can't operate on vectors of different lengths");
                return 1;
            }
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $1);
            $$->type = GRARR_TMP;

	    for (i = 0; i < $$->length; i++) {
	        if ($1->data[i] < 0 && rint($3->data[i]) != $3->data[i]) {
	            yyerror("Negative value raised to non-integer power");
	            return 1;
                } else if ($1->data[i] == 0.0 && $3->data[i] <= 0.0) {
	            yyerror("Zero raised to non-positive power");
	            return 1;
                } else {
                    $$->data[i] = pow($1->data[i], $3->data[i]);
                }
	    }
	}
	| vexpr '^' expr
	{
	    int i;
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $1);
            $$->type = GRARR_TMP;

	    for (i = 0; i < $$->length; i++) {
	        if ($1->data[i] < 0 && rint($3) != $3) {
	            yyerror("Negative value raised to non-integer power");
	            return 1;
                } else if ($1->data[i] == 0.0 && $3 <= 0.0) {
	            yyerror("Zero raised to non-positive power");
	            return 1;
                } else {
                    $$->data[i] = pow($1->data[i], $3);
                }
	    }
	}
	| expr '^' vexpr
	{
	    int i;
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $3);
            $$->type = GRARR_TMP;

	    for (i = 0; i < $$->length; i++) {
	        if ($1 < 0 && rint($3->data[i]) != $3->data[i]) {
	            yyerror("Negative value raised to non-integer power");
	            return 1;
                } else if ($1 == 0.0 && $3->data[i] <= 0.0) {
	            yyerror("Zero raised to non-positive power");
	            return 1;
                } else {
                    $$->data[i] = pow($1, $3->data[i]);
                }
	    }
	}
	| vexpr UCONSTANT
	{
	    int i;
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $1);
            $$->type = GRARR_TMP;
	    for (i = 0; i < $$->length; i++) {
		$$->data[i] = $1->data[i] * ((ParserFnc) (key[$2].data)) ();
	    }
	}
	| vexpr '?' expr ':' expr {
            int i;
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $1);
            $$->type = GRARR_TMP;
            for (i = 0; i < $$->length; i++) { 
                $$->data[i] = CAST_DBL_TO_BOOL($1->data[i]) ? $3 : $5;
            }
	}
	| vexpr '?' expr ':' vexpr {
            int i;
	    if ($1->length != $5->length) {
                errmsg("Can't operate on vectors of different lengths");
                return 1;
            }
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $1);
            $$->type = GRARR_TMP;
            for (i = 0; i < $$->length; i++) { 
                $$->data[i] = CAST_DBL_TO_BOOL($1->data[i]) ? $3 : $5->data[i];
            }
	}
	| vexpr '?' vexpr ':' expr {
            int i;
	    if ($1->length != $3->length) {
                errmsg("Can't operate on vectors of different lengths");
                return 1;
            }
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $1);
            $$->type = GRARR_TMP;
            for (i = 0; i < $$->length; i++) { 
                $$->data[i] = CAST_DBL_TO_BOOL($1->data[i]) ? $3->data[i] : $5;
            }
	}
	| vexpr '?' vexpr ':' vexpr {
            int i;
	    if ($1->length != $5->length || $1->length != $3->length) {
                errmsg("Can't operate on vectors of different lengths");
                return 1;
            }
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $1);
            $$->type = GRARR_TMP;
            for (i = 0; i < $$->length; i++) { 
                $$->data[i] = CAST_DBL_TO_BOOL($1->data[i]) ? $3->data[i] : $5->data[i];
            }
	}
	| vexpr OR vexpr
	{
	    int i;
	    if ($1->length != $3->length) {
                errmsg("Can't operate on vectors of different lengths");
                return 1;
            }
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $1);
            $$->type = GRARR_TMP;

	    for (i = 0; i < $$->length; i++) {
		$$->data[i] = $1->data[i] || $3->data[i];
	    }
	}
	| vexpr OR expr
	{
	    int i;
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $1);
            $$->type = GRARR_TMP;

	    for (i = 0; i < $$->length; i++) {
		$$->data[i] = $1->data[i] || $3;
	    }
	}
	| expr OR vexpr
	{
	    int i;
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $3);
            $$->type = GRARR_TMP;

	    for (i = 0; i < $$->length; i++) {
		$$->data[i] = $1 || $3->data[i];
	    }
	}
	| vexpr AND vexpr
	{
	    int i;
	    if ($1->length != $3->length) {
                errmsg("Can't operate on vectors of different lengths");
                return 1;
            }
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $1);
            $$->type = GRARR_TMP;

	    for (i = 0; i < $$->length; i++) {
		$$->data[i] = $1->data[i] && $3->data[i];
	    }
	}
	| vexpr AND expr
	{
	    int i;
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $1);
            $$->type = GRARR_TMP;

	    for (i = 0; i < $$->length; i++) {
		$$->data[i] = $1->data[i] && $3;
	    }
	}
	| expr AND vexpr
	{
	    int i;
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $3);
            $$->type = GRARR_TMP;

	    for (i = 0; i < $$->length; i++) {
		$$->data[i] = $1 && $3->data[i];
	    }
	}
	| vexpr GT vexpr
	{
	    int i;
	    if ($1->length != $3->length) {
                errmsg("Can't operate on vectors of different lengths");
                return 1;
            }
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $1);
            $$->type = GRARR_TMP;

	    for (i = 0; i < $$->length; i++) {
		$$->data[i] = ($1->data[i] > $3->data[i]);
	    }
	}
	| vexpr GT expr
	{
	    int i;
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $1);
            $$->type = GRARR_TMP;

	    for (i = 0; i < $$->length; i++) {
		$$->data[i] = ($1->data[i] > $3);
	    }
	}
	| expr GT vexpr
	{
	    int i;
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $3);
            $$->type = GRARR_TMP;

	    for (i = 0; i < $$->length; i++) {
		$$->data[i] = ($1 > $3->data[i]);
	    }
	}
	| vexpr LT vexpr
	{
	    int i;
	    if ($1->length != $3->length) {
                errmsg("Can't operate on vectors of different lengths");
                return 1;
            }
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $1);
            $$->type = GRARR_TMP;

	    for (i = 0; i < $$->length; i++) {
		$$->data[i] = ($1->data[i] < $3->data[i]);
	    }
	}
	| vexpr LT expr
	{
	    int i;
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $1);
            $$->type = GRARR_TMP;

	    for (i = 0; i < $$->length; i++) {
		$$->data[i] = ($1->data[i] < $3);
	    }
	}
	| expr LT vexpr
	{
	    int i;
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $3);
            $$->type = GRARR_TMP;

	    for (i = 0; i < $$->length; i++) {
		$$->data[i] = ($1 < $3->data[i]);
	    }
	}
	| vexpr GE vexpr
	{
	    int i;
	    if ($1->length != $3->length) {
                errmsg("Can't operate on vectors of different lengths");
                return 1;
            }
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $1);
            $$->type = GRARR_TMP;

	    for (i = 0; i < $$->length; i++) {
		$$->data[i] = ($1->data[i] >= $3->data[i]);
	    }
	}
	| vexpr GE expr
	{
	    int i;
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $1);
            $$->type = GRARR_TMP;

	    for (i = 0; i < $$->length; i++) {
		$$->data[i] = ($1->data[i] >= $3);
	    }
	}
	| expr GE vexpr
	{
	    int i;
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $3);
            $$->type = GRARR_TMP;

	    for (i = 0; i < $$->length; i++) {
		$$->data[i] = ($1 >= $3->data[i]);
	    }
	}
	| vexpr LE vexpr
	{
	    int i;
	    if ($1->length != $3->length) {
                errmsg("Can't operate on vectors of different lengths");
                return 1;
            }
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $1);
            $$->type = GRARR_TMP;

	    for (i = 0; i < $$->length; i++) {
		$$->data[i] = ($1->data[i] <= $3->data[i]);
	    }
	}
	| vexpr LE expr
	{
	    int i;
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $1);
            $$->type = GRARR_TMP;

	    for (i = 0; i < $$->length; i++) {
		$$->data[i] = ($1->data[i] <= $3);
	    }
	}
	| expr LE vexpr
	{
	    int i;
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $3);
            $$->type = GRARR_TMP;

	    for (i = 0; i < $$->length; i++) {
		$$->data[i] = ($1 <= $3->data[i]);
	    }
	}
	| vexpr EQ vexpr
	{
	    int i;
	    if ($1->length != $3->length) {
                errmsg("Can't operate on vectors of different lengths");
                return 1;
            }
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $1);
            $$->type = GRARR_TMP;

	    for (i = 0; i < $$->length; i++) {
		$$->data[i] = ($1->data[i] == $3->data[i]);
	    }
	}
	| vexpr EQ expr
	{
	    int i;
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $1);
            $$->type = GRARR_TMP;

	    for (i = 0; i < $$->length; i++) {
		$$->data[i] = ($1->data[i] == $3);
	    }
	}
	| expr EQ vexpr
	{
	    int i;
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $3);
            $$->type = GRARR_TMP;

	    for (i = 0; i < $$->length; i++) {
		$$->data[i] = ($1 == $3->data[i]);
	    }
	}
	| vexpr NE vexpr
	{
	    int i;
	    if ($1->length != $3->length) {
                errmsg("Can't operate on vectors of different lengths");
                return 1;
            }
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $1);
            $$->type = GRARR_TMP;

	    for (i = 0; i < $$->length; i++) {
		$$->data[i] = ($1->data[i] != $3->data[i]);
	    }
	}
	| vexpr NE expr
	{
	    int i;
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $1);
            $$->type = GRARR_TMP;

	    for (i = 0; i < $$->length; i++) {
		$$->data[i] = ($1->data[i] != $3);
	    }
	}
	| expr NE vexpr
	{
	    int i;
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $3);
            $$->type = GRARR_TMP;

	    for (i = 0; i < $$->length; i++) {
		$$->data[i] = ($1 != $3->data[i]);
	    }
	}
	| NOT vexpr
	{
	    int i;
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $2);
            $$->type = GRARR_TMP;
            for (i = 0; i < $$->length; i++) { 
                $$->data[i] = !$2->data[i];
            }
	}
	| '(' vexpr ')'
	{
	    int i;
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $2);
            $$->type = GRARR_TMP;
            for (i = 0; i < $$->length; i++) { 
                $$->data[i] = $2->data[i];
            }
	}
	| '-' vexpr %prec UMINUS {
	    int i;
            $$ = &freelist[fcnt++];
	    copy_vrbl($$, $2);
            $$->type = GRARR_TMP;
            for (i = 0; i < $$->length; i++) { 
                $$->data[i] = - $2->data[i];
            }
	}
	;


asgn:
	VAR_D '=' expr
	{
	    *($1) = $3;
	}
	| array indx '=' expr
	{
	    if ($2 >= $1->length) {
		yyerror("Access beyond array bounds");
		return 1;
            }
            $1->data[$2] = $4;
	}
	;

lside_array:
        array
        {
            switch ($1->type) {
            case GRARR_SET:
#if 0
                if (find_set_bydata($1->data, &vasgn_pset) != RETURN_SUCCESS) {
                    errmsg("Internal error");
		    return 1;
                }
#endif
                break;
            case GRARR_VEC:
                vasgn_pset = NULL;
                break;
            default:
                /* It can NOT be a tmp array on the left side! */
                errmsg("Internal error");
	        return 1;
            }
            $$ = $1;
        }
        ;

vasgn:
	lside_array '=' vexpr
	{
	    int i;
	    if ($1->length != $3->length) {
                errmsg("Left and right vectors are of different lengths");
                return 1;
            }
	    for (i = 0; i < $1->length; i++) {
	        $1->data[i] = $3->data[i];
	    }
	}
	| lside_array '=' expr
	{
	    int i;
	    for (i = 0; i < $1->length; i++) {
	        $1->data[i] = $3;
	    }
	}
        ;

defines:
	USE CHRSTR TYPE proctype FROM CHRSTR {
	    if (load_module($6, $2, $2, $4) != 0) {
	        yyerror("DL module load failed");
	    }
	    xfree($2);
	    xfree($6);
	}
	| USE CHRSTR TYPE proctype FROM CHRSTR ALIAS CHRSTR {
	    if (load_module($6, $2, $8, $4) != 0) {
	        yyerror("DL module load failed");
	    }
	    xfree($2);
	    xfree($6);
	    xfree($8);
	}
        ;

regionset:
	selectregion onoff {
	    region *r = region_get_data($1);
            if (r) {
                r->active = $2;
                XCFREE(r->wps);
                r->n = 0;
            }
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
            if (project_set_version_id(grace->project, $2) != RETURN_SUCCESS) {
                errmsg("Project version is newer than software!");
            }
            if (project_get_version_id(grace->project) < 50001) {
                add_xmgr_fonts(grace->project);
            }
            dobject_id = 0;
        }
        | PAGE SIZE nexpr ',' nexpr {
            set_page_dimensions(grace, $3, $5, FALSE);
        }
        | REFERENCE DATE jrawdate {
            project_set_ref_date(grace->project, $3);
	}
        | DATE WRAP onoff {
            project_allow_two_digits_years(grace->project, $3);
	}
        | DATE WRAP YEAR iexpr {
            project_set_wrap_year(grace->project, $4);
	}
	| BACKGROUND color_select {
	    Project *pr = project_get_data(grace->project);
            pr->bgcolor = $2;
	}
	| PAGE BACKGROUND FILL onoff {
	    Project *pr = project_get_data(grace->project);
	    pr->bgfill = $4;
	}
	| PAGE SCROLL expr '%' {
	    grace->rt->scrollper = $3 / 100.0;
	}
	| PAGE INOUT expr '%' {
	    grace->rt->shexper = $3 / 100.0;
	}

	| LINK PAGE onoff {
	}

	| TARGET selectset {
	    grace->rt->target_set = $2;
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
	    set_set_hotlink($1, 1, $4, $3);
	    xfree($4);
	}
	| selectset LINK onoff {
	    set_set_hotlink($1, $3, NULL, 0);
	}

/* Objects */
	| WITH objecttype {
            curobject = object_data_new_complete($2);
	}
	| objecttype onoff {
	    if (!curobject) {
                yyerror("No active object");
	    } else {
	        curobject->active = $2;
            }
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
                a->fillmode = ARCFILL_CHORD;
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
                    q = object_new(grace->project);
                } else {
                    Quark *gr;
                    gr = objgno;
                    if (!gr) {
                        gr = allocate_graph(grace->project, 0);
                    }
                    if (gr) {
                        q = object_new(gr);
                    }
                }
                if (q) {
                    char buf[16];
                    object_data_free(object_get_data(q));
                    q->data = curobject;
	            sprintf(buf, "DO%02d", dobject_id);
                    quark_idstr_set(q, buf);
                    dobject_id++;
                }
            }
        }

/* timestamp and string */
	| WITH STRING {
            curatext = atext_new(grace->project);
	}
	| atext selectgraph {
	    quark_reparent($1, $2);
	}
	| atext LOCTYPE worldview {
	}
	| atext onoff {
            atext_set_active($1, $2);
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
	    if (!compare_strings("timestamp", quark_idstr_get($1))) {
                atext_set_string($1, $3);
            }
            xfree($3);
	}

/* defaults */
	| DEFAULT lines_select {
            Project *pr = project_get_data(grace->project);
	    pr->grdefaults.line.style = $2;
	}
	| DEFAULT linew_select {
            Project *pr = project_get_data(grace->project);
	    pr->grdefaults.line.width = $2;
	}
	| DEFAULT color_select {
            Project *pr = project_get_data(grace->project);
	    pr->grdefaults.line.pen.color = $2;
	}
	| DEFAULT pattern_select {
            Project *pr = project_get_data(grace->project);
	    pr->grdefaults.line.pen.pattern = $2;
	}
	| DEFAULT CHAR SIZE expr {
            Project *pr = project_get_data(grace->project);
	    pr->grdefaults.charsize = $4;
	}
	| DEFAULT font_select {
            Project *pr = project_get_data(grace->project);
	    pr->grdefaults.font = $2;
	}
	| DEFAULT SYMBOL SIZE expr {
	}
	| DEFAULT SFORMAT CHRSTR {
	    project_set_sformat(grace->project, $3);
	    xfree($3);
	}
	| MAP FONTP nexpr TO CHRSTR ',' CHRSTR {
	    Fontdef f;
            f.id = $3;
            f.fontname = $5;
            f.fallback = $7;
            project_add_font(grace->project, &f);
            xfree($5);
	    xfree($7);
	}
	| MAP COLOR nexpr TO '(' nexpr ',' nexpr ',' nexpr ')' ',' CHRSTR {
	    Color color;
            color.rgb.red   = $6;
            color.rgb.green = $8;
            color.rgb.blue  = $10;
            color.ctype = COLOR_MAIN;
            color.cname = $13;
            if (store_color(canvas, $3, &color) == RETURN_FAILURE) {
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
            s = copy_string(NULL, project_get_description(grace->project));
            s = concat_strings(s, $2);
	    xfree($2);
            s = concat_strings(s, "\n");
            project_set_description(grace->project, s);
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
            graph_get_viewport(whichgraph, &gv);
            l->offset.x = vp.x - gv.xv1;
            l->offset.y = gv.yv2 - vp.y;
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
            graph_set_active($1, $2);
            frame_set_active(get_parent_frame($1), $2);
        }
	| selectgraph HIDDEN onoff {
            graph_set_active($1, !$3);
            frame_set_active(get_parent_frame($1), !$3);
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
	    gloc->fx = $4;
	    gloc->fy = $5;
	}
	| selectgraph FIXEDPOINT PREC expr ',' expr {
            GLocator *gloc = graph_get_locator($1);
	    gloc->px = $4;
	    gloc->py = $6;
	}
	| selectgraph FIXEDPOINT XY expr ',' expr {
            GLocator *gloc = graph_get_locator($1);
	    gloc->dsx = $4;
	    gloc->dsy = $6;
	}
	| selectgraph FIXEDPOINT TYPE nexpr {
            GLocator *gloc = graph_get_locator($1);
            gloc->pt_type = $4;
        }
        
	| TYPE xytype {
	    grace->rt->curtype = $2;
	}

/* I/O filters */
	| DEFINE filtertype CHRSTR filtermethod CHRSTR {
	    if (add_io_filter($2, $4, $5, $3) != 0) {
	        yyerror("Failed adding i/o filter");
	    }
	    xfree($3);
	    xfree($5);
	}
	;

actions:
	ECHO CHRSTR {
	    echomsg($2);
	    xfree($2);
	}
	| ECHO expr {
	    char buf[32];
            sprintf(buf, "%g", $2);
            echomsg(buf);
	}
	| GETP CHRSTR {
	    gotparams = TRUE;
	    strcpy(paramfile, $2);
	    xfree($2);
	}
	| selectset HIDDEN onoff {
	    set_set_active($1, !$3);
	}
        ;


options:
        PAGE LAYOUT pagelayout {
#ifndef NONE_GUI
            gui_set_page_free(grace->gui, $3 == FREE);
#endif
        }
        ;


set_setprop:
	setprop {}
	| setprop_obs {}
	;

setprop:
	selectset onoff {
	    set_set_active($1, $2);
	}
	| selectset TYPE xytype {
	    set_set_type($1, $3);
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

	    if (project_get_version_id(grace->project) <= 40102 && project_get_version_id(grace->project) >= 30000) {
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

	    if (project_get_version_id(grace->project) <= 40102) {
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
	    p->avalue.type = $4;
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
	    p->avalue.format = $4;
	}
	| selectset AVALUE PREC nexpr
        {
	    set *p = set_get_data($1);
	    p->avalue.prec = $4;
	}
	| selectset AVALUE OFFSET expr ',' expr {
	    set *p = set_get_data($1);
	    p->avalue.offset.x = $4;
	    p->avalue.offset.y = $6;
	}
	| selectset AVALUE PREPEND CHRSTR
        {
	    set *p = set_get_data($1);
	    strcpy(p->avalue.prestr, $4);
	    xfree($4);
	}
	| selectset AVALUE APPEND CHRSTR
        {
	    set *p = set_get_data($1);
	    strcpy(p->avalue.appstr, $4);
	    xfree($4);
	}

	| selectset ERRORBAR onoff {
	    set *p = set_get_data($1);
	    p->errbar.active = $3;
	}
	| selectset ERRORBAR opchoice_sel {
	    set *p = set_get_data($1);
	    p->errbar.ptype = $3;
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
	    set_set_comment($1, $3);
	    xfree($3);
	}
        
	| selectset LEGEND CHRSTR {
	    set_set_legstr($1, $3);
	    xfree($3);
	}
	;


axisfeature:
	onoff {
	    curtm->active = $1;
	}
	| TYPE ZERO onoff {
	    curtm->zero = $3;
	}
	| TICKP tickattr {}
	| TICKP tickattr_obs {}
	| TICKLABEL ticklabelattr {}
	| TICKLABEL ticklabelattr_obs {}
	| LABEL axislabeldesc {}
	| LABEL axislabeldesc_obs {}
	| BAR axisbardesc {}
	| OFFSET expr ',' expr {
            curtm->offsx = $2;
	    curtm->offsy = $4;
	}
	;

tickattr:
	onoff {
	    curtm->t_flag = $1;
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
            curtm->offsx = $2;
	}
	| OFFSETY expr {
            curtm->offsy = $2;
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
	    curtm->props.color = curtm->mprops.color = $1;
	}
	| MAJOR color_select {
	    curtm->props.color = $2;
	}
	| MINOR color_select {
	    curtm->mprops.color = $2;
	}
	| linew_select {
	    curtm->props.linew = curtm->mprops.linew = $1;
	}
	| MAJOR linew_select {
	    curtm->props.linew = $2;
	}
	| MINOR linew_select {
	    curtm->mprops.linew = $2;
	}
	| MAJOR lines_select {
	    curtm->props.lines = $2;
	}
	| MINOR lines_select {
	    curtm->mprops.lines = $2;
	}
	| MAJOR GRID onoff {
	    curtm->props.gridflag = $3;
	}
	| MINOR GRID onoff {
	    curtm->mprops.gridflag = $3;
	}
	| opchoice_sel {
	    curtm->t_op = $1;
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
	    curtm->tl_flag = $1;
	}
	| PREC nexpr {
	    curtm->tl_prec = $2;
	}
	| FORMAT formatchoice {
	    curtm->tl_format = $2;
	}
	| FORMAT expr {
	    curtm->tl_format = $2;
	}
	| APPEND CHRSTR {
	    strcpy(curtm->tl_appstr, $2);
	    xfree($2);
	}
	| PREPEND CHRSTR {
	    strcpy(curtm->tl_prestr, $2);
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
	    curtm->tl_op = $1;
	}
	| FORMULA CHRSTR {
            curtm->tl_formula =
                copy_string(curtm->tl_formula, $2);
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
	    curtm->tloc[$1].label = 
                copy_string(curtm->tloc[$1].label, $3);
	    xfree($3);
	}
	| OFFSET AUTO {
	    curtm->tl_gaptype = TYPE_AUTO;
	}
	| OFFSET SPEC {
	    curtm->tl_gaptype = TYPE_SPEC;
	}
	| OFFSET expr ',' expr {
	    curtm->tl_gap.x = $2;
	    curtm->tl_gap.y = $4;
	}
	;

axislabeldesc:
	CHRSTR {
	    curtm->label = copy_string(curtm->label, $1);
	    xfree($1);
	}
	| LAYOUT PERP {
	    curtm->label_layout = LAYOUT_PERPENDICULAR;
	}
	| LAYOUT PARA {
	    curtm->label_layout = LAYOUT_PARALLEL;
	}
	| PLACE AUTO {
	    curtm->label_place = TYPE_AUTO;
	}
	| PLACE SPEC {
	    curtm->label_place = TYPE_SPEC;
	}
	| PLACE expr ',' expr {
	    curtm->label_offset.x = $2;
	    curtm->label_offset.y = $4;
	}
	| JUST justchoice {
	    curtm->label_tprops.just = $2;
	}
	| CHAR SIZE expr {
	    curtm->label_tprops.charsize = $3;
	}
	| font_select {
	    curtm->label_tprops.font = $1;
	}
	| color_select {
	    curtm->label_tprops.color = $1;
	}
	| opchoice_sel {
	    curtm->label_op = $1;
	}
	;

axisbardesc:
	onoff {
	    curtm->t_drawbar = $1;
	}
	| color_select {
	    curtm->t_drawbarcolor = $1;
	}
	| lines_select {
	    curtm->t_drawbarlines = $1;
	}
	| linew_select {
	    curtm->t_drawbarlinew = $1;
	}
	;

selectgraph:
        GRAPHNO
        {
            $$ = allocate_graph(grace->project, $1);
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
            Quark *q = quark_find_descendant_by_idstr(whichgraph, "x_axis");
            if (!q) {
                q = axis_new(whichgraph);
                quark_idstr_set(q, "x_axis");
                axis_set_type(q, AXIS_TYPE_X);
            }
            curtm = axis_get_data(q);
        }
	| YAXIS {
            Quark *q = quark_find_descendant_by_idstr(whichgraph, "y_axis");
            if (!q) {
                q = axis_new(whichgraph);
                quark_idstr_set(q, "y_axis");
                axis_set_type(q, AXIS_TYPE_Y);
            }
            curtm = axis_get_data(q);
        }
	| ALTXAXIS {
            Quark *q = quark_find_descendant_by_idstr(whichgraph, "altx_axis");
            if (!q) {
                q = axis_new(whichgraph);
                quark_idstr_set(q, "altx_axis");
                axis_set_type(q, AXIS_TYPE_X);
            }
            curtm = axis_get_data(q);
        }
	| ALTYAXIS {
            Quark *q = quark_find_descendant_by_idstr(whichgraph, "alty_axis");
            if (!q) {
                q = axis_new(whichgraph);
                quark_idstr_set(q, "alty_axis");
                axis_set_type(q, AXIS_TYPE_Y);
            }
            curtm = axis_get_data(q);
        }
	;

title:
	TITLE {
            Quark *q = quark_find_descendant_by_idstr(whichframe, "title");
            if (!q) {
                AText *at;
                q = atext_new(whichframe);
                atext_set_active(q, TRUE);
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
                atext_set_active(q, TRUE);
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
            Quark *q = quark_find_descendant_by_idstr(grace->project,
                "timestamp");
            if (!q) {
                q = atext_new(grace->project);
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
                whichgraph = allocate_graph(grace->project, 0);
            }
            $$ = allocate_region(whichgraph, $1);
        }
        ;

proctype:
        KEY_CONST        { $$ = CONSTANT; }
        | KEY_UNIT      { $$ = UCONSTANT; }
        | KEY_FUNC_I       { $$ = FUNC_I; }
	| KEY_FUNC_D       { $$ = FUNC_D; }
	| KEY_FUNC_ND     { $$ = FUNC_ND; }
	| KEY_FUNC_NN     { $$ = FUNC_NN; }
	| KEY_FUNC_DD     { $$ = FUNC_DD; }
	| KEY_FUNC_NND   { $$ = FUNC_NND; }
	| KEY_FUNC_PPD   { $$ = FUNC_PPD; }
	| KEY_FUNC_PPPD { $$ = FUNC_PPPD; }
	;

tickspectype:
	NONE { $$ =  TICKS_SPEC_NONE; }
	| TICKSP { $$ = TICKS_SPEC_MARKS; }
	| BOTH { $$ = TICKS_SPEC_BOTH; }
	;

filtertype:
        IFILTER       { $$ = FILTER_INPUT; }
	| OFILTER    { $$ = FILTER_OUTPUT; }
	;
	
filtermethod:
        MAGIC         { $$ = FILTER_MAGIC; }
	| PATTERN   { $$ = FILTER_PATTERN; }
	;
	
xytype:
	XY { $$ = SET_XY; }
	| BAR { $$ = SET_BAR; }
	| BARDY { $$ = SET_BARDY; }
	| BARDYDY { $$ = SET_BARDYDY; }
	| XYZ { $$ = SET_XYZ; }
	| XYDX { $$ = SET_XYDX; }
	| XYDY { $$ = SET_XYDY; }
	| XYDXDX { $$ = SET_XYDXDX; }
	| XYDYDY { $$ = SET_XYDYDY; }
	| XYDXDY { $$ = SET_XYDXDY; }
	| XYDXDXDYDY { $$ = SET_XYDXDXDYDY; }
	| XYHILO { $$ = SET_XYHILO; }
	| XYR { $$ = SET_XYR; }
	| XYSIZE { $$ = SET_XYSIZE; }
	| XYCOLOR { $$ = SET_XYCOLOR; }
	| XYCOLPAT { $$ = SET_XYCOLPAT; }
	| XYVMAP { $$ = SET_XYVMAP; }
	| XYBOXPLOT { $$ = SET_BOXPLOT; }
	| XYSTRING { $$ = SET_XY; }
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
	ABOVE { $$ = REGION_POLYGON; }
	|  BELOW { $$ = REGION_POLYGON; }
	|  LEFT { $$ = REGION_POLYGON; }
	|  RIGHT { $$ = REGION_POLYGON; }
	|  POLYI { $$ = REGION_POLYGON; }
	|  POLYO { $$ = REGION_POLYGON; }
	|  HORIZI { $$ = REGION_BAND; }
	|  VERTI { $$ = REGION_BAND; }
	|  HORIZO { $$ = REGION_BAND; }
	|  VERTO { $$ = REGION_BAND; }
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
            if (!grace->rt->safe_mode) {
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

formatchoice: DECIMAL { $$ = FORMAT_DECIMAL; }
	| EXPONENTIAL { $$ = FORMAT_EXPONENTIAL; }
	| GENERAL { $$ = FORMAT_GENERAL; }
	| SCIENTIFIC { $$ = FORMAT_SCIENTIFIC; }
	| ENGINEERING { $$ = FORMAT_ENGINEERING; }
	| POWER { $$ = FORMAT_POWER; }
	| DDMMYY { $$ = FORMAT_DDMMYY; }
	| MMDDYY { $$ = FORMAT_MMDDYY; }
	| YYMMDD { $$ = FORMAT_YYMMDD; }
	| MMYY { $$ = FORMAT_MMYY; }
	| MMDD { $$ = FORMAT_MMDD; }
	| MONTHDAY { $$ = FORMAT_MONTHDAY; }
	| DAYMONTH { $$ = FORMAT_DAYMONTH; }
	| MONTHS { $$ = FORMAT_MONTHS; }
	| MONTHSY { $$ = FORMAT_MONTHSY; }
	| MONTHL { $$ = FORMAT_MONTHL; }
	| DAYOFWEEKS { $$ = FORMAT_DAYOFWEEKS; }
	| DAYOFWEEKL { $$ = FORMAT_DAYOFWEEKL; }
	| DAYOFYEAR { $$ = FORMAT_DAYOFYEAR; }
	| HMS { $$ = FORMAT_HMS; }
	| MMDDHMS { $$ = FORMAT_MMDDHMS; }
	| MMDDYYHMS { $$ = FORMAT_MMDDYYHMS; }
	| YYMMDDHMS { $$ = FORMAT_YYMMDDHMS; }
	| DEGREESLON { $$ = FORMAT_DEGREESLON; }
	| DEGREESMMLON { $$ = FORMAT_DEGREESMMLON; }
	| DEGREESMMSSLON { $$ = FORMAT_DEGREESMMSSLON; }
	| MMSSLON { $$ = FORMAT_MMSSLON; }
	| DEGREESLAT { $$ = FORMAT_DEGREESLAT; }
	| DEGREESMMLAT { $$ = FORMAT_DEGREESMMLAT; }
	| DEGREESMMSSLAT { $$ = FORMAT_DEGREESMMSSLAT; }
	| MMSSLAT { $$ = FORMAT_MMSSLAT; }
	;

signchoice: NORMAL { $$ = SIGN_NORMAL; }
	| ABSOLUTE { $$ = SIGN_ABSOLUTE; }
	| NEGATE { $$ = SIGN_NEGATE; }
	;

worldview: WORLD { $$ = COORD_WORLD; }
	| VIEW { $$ = COORD_VIEW; }
	;

datacolumn: X_TOK { $$ = DATA_X; }
	| Y_TOK { $$ = DATA_Y; }
	| X0 { $$ = DATA_X; }
	| Y0 { $$ = DATA_Y; }
	| Y1 { $$ = DATA_Y1; }
	| Y2 { $$ = DATA_Y2; }
	| Y3 { $$ = DATA_Y3; }
	| Y4 { $$ = DATA_Y4; }
	;

stattype: MINP { $$ = MINP; }
	| MAXP { $$ = MAXP; }
        | AVG { $$ = AVG; }
	| SD { $$ = SD; }
	;

font_select:
        FONTP nexpr
        {
            $$ = $2;
        }
        | FONTP CHRSTR
        {
            $$ = get_font_by_name(grace->project, $2);
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
            if (c >= 0 && c < number_of_colors(canvas)) {
                $$ = c;
            } else {
                errmsg("Invalid color ID");
                $$ = 1;
            }
        }
        | COLOR CHRSTR
        {
            int c = get_color_by_name(canvas, $2);
            if (c == BAD_COLOR) {
                errmsg("Invalid color name");
                c = 1;
            }
            xfree($2);
            $$ = c;
        }
        | COLOR '(' nexpr ',' nexpr ',' nexpr ')'
        {
            int c;
            Color color;
            color.rgb.red = $3;
            color.rgb.green = $5;
            color.rgb.blue = $7;
            color.ctype = COLOR_MAIN;
            color.cname = NULL;
            c = add_color(canvas, &color);
            if (c == BAD_COLOR) {
                errmsg("Can't allocate requested color");
                c = 1;
            }
            $$ = c;
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

opchoice: NORMAL { $$ = PLACEMENT_NORMAL; }
	| OPPOSITE { $$ = PLACEMENT_OPPOSITE; }
	| BOTH { $$ = PLACEMENT_BOTH; }
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
            set_page_dimensions(grace, wpp, hpp, FALSE);
        }
        | PAGE SIZE NUMBER NUMBER {
            set_page_dimensions(grace, (int) $3, (int) $4, FALSE);
        }
	| PAGE nexpr {
	    grace->rt->scrollper = $2 / 100.0;
	}
	| PAGE INOUT nexpr {
	    grace->rt->shexper = $3 / 100.0;
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
	    if ($3 == FALSE && project_get_version_id(grace->project) <= 40102) {
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
            graph_get_viewport(whichgraph, &gv);
            l->offset.x = vp.x - gv.xv1;
            l->offset.y = gv.yv2 - vp.y;
	}
	| LEGEND STRING nexpr CHRSTR {
            int nsets;
            Quark *pset, **psets;
            nsets = get_descendant_sets(whichgraph, &psets);
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
	    curtm->label_op = $1;
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
	    set *p = set_get_data($1);
	    p->errbar.ptype = $4;
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
	    curtm->active = $2;
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
	    curtm->t_op = $1;
	}
        ;

ticklabelattr_obs:
	linew_select { }
	| TYPE AUTO {
	    if (curtm->t_spec == TICKS_SPEC_BOTH) {
                curtm->t_spec = TICKS_SPEC_MARKS;
            }
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
	    curtm->tl_op = $1;
	}
	| SIGN signchoice {
	    switch($2) {
            case SIGN_NEGATE:
                curtm->tl_formula =
                    copy_string(curtm->tl_formula, "-$t");
                break;
            case SIGN_ABSOLUTE:
                curtm->tl_formula =
                    copy_string(curtm->tl_formula, "abs($t)");
                break;
            default:
                curtm->tl_formula =
                    copy_string(curtm->tl_formula, NULL);
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

opchoice_obs: TOP { $$ = PLACEMENT_OPPOSITE; }
	| BOTTOM { $$ = PLACEMENT_NORMAL; }
	| LEFT { $$ = PLACEMENT_NORMAL; }
	| RIGHT { $$ = PLACEMENT_OPPOSITE; }
	| BOTH { $$ = PLACEMENT_BOTH; }
	;

%%

/* list of intrinsic functions and keywords */
symtab_entry ikey[] = {
	{"ABOVE", ABOVE, NULL},
	{"ABS", FUNC_D, (void *) fabs},
	{"ABSOLUTE", ABSOLUTE, NULL},
	{"ACOS", FUNC_D, (void *) acos},
	{"ACOSH", FUNC_D, (void *) acosh},
	{"AI", FUNC_D, (void *) ai_wrap},
	{"ALIAS", ALIAS, NULL},
	{"ALT", ALT, NULL},
	{"ALTXAXIS", ALTXAXIS, NULL},
	{"ALTYAXIS", ALTYAXIS, NULL},
	{"AND", AND, NULL},
	{"ANGLE", ANGLE, NULL},
	{"APPEND", APPEND, NULL},
	{"ARROW", ARROW, NULL},
	{"ASIN", FUNC_D, (void *) asin},
	{"ASINH", FUNC_D, (void *) asinh},
	{"ATAN", FUNC_D, (void *) atan},
	{"ATAN2", FUNC_DD, (void *) atan2},
	{"ATANH", FUNC_D, (void *) atanh},
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
	{"BETA", FUNC_DD, (void *) beta},
	{"BETWEEN", BETWEEN, NULL},
	{"BI", FUNC_D, (void *) bi_wrap},
	{"BOTH", BOTH, NULL},
	{"BOTTOM", BOTTOM, NULL},
	{"BOX", BOX, NULL},
	{"CEIL", FUNC_D, (void *) ceil},
	{"CENTER", CENTER, NULL},
	{"CHAR", CHAR, NULL},
	{"CHART", CHART, NULL},
	{"CHDTR", FUNC_DD, (void *) chdtr},
	{"CHDTRC", FUNC_DD, (void *) chdtrc},
	{"CHDTRI", FUNC_DD, (void *) chdtri},
	{"CHI", FUNC_D, (void *) chi_wrap},
	{"CHRSTR", CHRSTR, NULL},
	{"CI", FUNC_D, (void *) ci_wrap},
	{"CLEAR", CLEAR, NULL},
	{"CLICK", CLICK, NULL},
	{"CLIP", CLIP, NULL},
	{"COLOR", COLOR, NULL},
	{"COMMENT", COMMENT, NULL},
	{"CONST", KEY_CONST, NULL},
	{"COS", FUNC_D, (void *) cos},
	{"COSH", FUNC_D, (void *) cosh},
	{"DATE", DATE, NULL},
	{"DAWSN", FUNC_D, (void *) dawsn},
	{"DAYMONTH", DAYMONTH, NULL},
	{"DAYOFWEEKL", DAYOFWEEKL, NULL},
	{"DAYOFWEEKS", DAYOFWEEKS, NULL},
	{"DAYOFYEAR", DAYOFYEAR, NULL},
	{"DDMMYY", DDMMYY, NULL},
	{"DECIMAL", DECIMAL, NULL},
	{"DEF", DEF, NULL},
	{"DEFAULT", DEFAULT, NULL},
	{"DEFINE", DEFINE, NULL},
	{"DEG", UCONSTANT, (void *) deg_uconst},
	{"DEGREESLAT", DEGREESLAT, NULL},
	{"DEGREESLON", DEGREESLON, NULL},
	{"DEGREESMMLAT", DEGREESMMLAT, NULL},
	{"DEGREESMMLON", DEGREESMMLON, NULL},
	{"DEGREESMMSSLAT", DEGREESMMSSLAT, NULL},
	{"DEGREESMMSSLON", DEGREESMMSSLON, NULL},
	{"DESCENDING", DESCENDING, NULL},
	{"DESCRIPTION", DESCRIPTION, NULL},
	{"DEVICE", DEVICE, NULL},
	{"DISK", DISK, NULL},
	{"DROPLINE", DROPLINE, NULL},
	{"ECHO", ECHO, NULL},
	{"ELLIE", FUNC_DD, (void *) ellie},
	{"ELLIK", FUNC_DD, (void *) ellik},
	{"ELLIPSE", ELLIPSE, NULL},
	{"ELLPE", FUNC_D, (void *) ellpe},
	{"ELLPK", FUNC_D, (void *) ellpk},
	{"ENGINEERING", ENGINEERING, NULL},
	{"EQ", EQ, NULL},
	{"ER", ERRORBAR, NULL},
	{"ERF", FUNC_D, (void *) erf},
	{"ERFC", FUNC_D, (void *) erfc},
	{"ERRORBAR", ERRORBAR, NULL},
	{"EXP", FUNC_D, (void *) exp},
	{"EXPN", FUNC_ND, (void *) expn},
	{"EXPONENTIAL", EXPONENTIAL, NULL},
	{"FAC", FUNC_I, (void *) fac},
	{"FALSE", OFF, NULL},
	{"FDTR", FUNC_NND, (void *) fdtr},
	{"FDTRC", FUNC_NND, (void *) fdtrc},
	{"FDTRI", FUNC_NND, (void *) fdtri},
	{"FILL", FILL, NULL},
	{"FIXED", FIXED, NULL},
	{"FIXEDPOINT", FIXEDPOINT, NULL},
	{"FLOOR", FUNC_D, (void *) floor},
	{"FONT", FONTP, NULL},
	{"FORMAT", FORMAT, NULL},
	{"FORMULA", FORMULA, NULL},
	{"FRAME", FRAMEP, NULL},
	{"FREE", FREE, NULL},
	{"FRESNLC", FUNC_D, (void *) fresnlc_wrap},
	{"FRESNLS", FUNC_D, (void *) fresnls_wrap},
	{"FROM", FROM, NULL},
	{"F_OF_D", KEY_FUNC_D, NULL},
	{"F_OF_DD", KEY_FUNC_DD, NULL},
        {"F_OF_I", KEY_FUNC_I, NULL},
	{"F_OF_ND", KEY_FUNC_ND, NULL},
	{"F_OF_NN", KEY_FUNC_NN, NULL},
	{"F_OF_NND", KEY_FUNC_NND, NULL},
	{"F_OF_PPD", KEY_FUNC_PPD, NULL},
	{"F_OF_PPPD", KEY_FUNC_PPPD, NULL},
	{"GAMMA", FUNC_D, (void *) true_gamma},
	{"GDTR", FUNC_PPD, (void *) gdtr},
	{"GDTRC", FUNC_PPD, (void *) gdtrc},
	{"GE", GE, NULL},
	{"GENERAL", GENERAL, NULL},
	{"GETP", GETP, NULL},
	{"GRAPH", GRAPH, NULL},
	{"GRID", GRID, NULL},
	{"GT", GT, NULL},
	{"HARDCOPY", HARDCOPY, NULL},
	{"HBAR", HBAR, NULL},
	{"HGAP", HGAP, NULL},
	{"HIDDEN", HIDDEN, NULL},
	{"HMS", HMS, NULL},
	{"HORIZI", HORIZI, NULL},
	{"HORIZO", HORIZO, NULL},
	{"HORIZONTAL", HORIZONTAL, NULL},
	{"HYP2F1", FUNC_PPPD, (void *) hyp2f1},
	{"HYPERG", FUNC_PPD, (void *) hyperg},
	{"HYPOT", FUNC_DD, (void *) hypot},
	{"I0E", FUNC_D, (void *) i0e},
	{"I1E", FUNC_D, (void *) i1e},
	{"IFILTER", IFILTER, NULL},
	{"IGAM", FUNC_DD, (void *) igam},
	{"IGAMC", FUNC_DD, (void *) igamc},
	{"IGAMI", FUNC_DD, (void *) igami},
	{"IN", IN, NULL},
	{"INCBET", FUNC_PPD, (void *) incbet},
	{"INCBI", FUNC_PPD, (void *) incbi},
	{"INCREMENT", INCREMENT, NULL},
	{"INOUT", INOUT, NULL},
	{"INVERT", INVERT, NULL},
	{"IRAND", FUNC_I, (void *) irand_wrap},
	{"IV", FUNC_DD, (void *) iv_wrap},
	{"JUST", JUST, NULL},
	{"JV", FUNC_DD, (void *) jv_wrap},
	{"K0E", FUNC_D, (void *) k0e},
	{"K1E", FUNC_D, (void *) k1e},
	{"KILL", KILL, NULL},
	{"KN", FUNC_ND, (void *) kn_wrap},
	{"LABEL", LABEL, NULL},
	{"LANDSCAPE", LANDSCAPE, NULL},
	{"LAYOUT", LAYOUT, NULL},
	{"LBETA", FUNC_DD, (void *) lbeta},
	{"LE", LE, NULL},
	{"LEFT", LEFT, NULL},
	{"LEGEND", LEGEND, NULL},
	{"LENGTH", LENGTH, NULL},
	{"LGAMMA", FUNC_D, (void *) lgamma},
	{"LINE", LINE, NULL},
	{"LINEAR", LINEAR, NULL},
	{"LINESTYLE", LINESTYLE, NULL},
	{"LINEWIDTH", LINEWIDTH, NULL},
	{"LINK", LINK, NULL},
	{"LN", FUNC_D, (void *) log},
	{"LOCTYPE", LOCTYPE, NULL},
	{"LOG", LOG, NULL},
	{"LOG10", FUNC_D, (void *) log10},
	{"LOG2", FUNC_D, (void *) log2},
	{"LOGARITHMIC", LOGARITHMIC, NULL},
	{"LOGX", LOGX, NULL},
	{"LOGXY", LOGXY, NULL},
	{"LOGY", LOGY, NULL},
	{"LOGIT", LOGIT, NULL},
	{"LT", LT, NULL},
	{"MAGIC", MAGIC, NULL},
	{"MAJOR", MAJOR, NULL},
	{"MAP", MAP, NULL},
	{"MAX", MAXP, NULL},
	{"MAXOF", FUNC_DD, (void *) max_wrap},
	{"MESH", MESH, NULL},
	{"MIN", MINP, NULL},
	{"MINOF", FUNC_DD, (void *) min_wrap},
	{"MINOR", MINOR, NULL},
	{"MMDD", MMDD, NULL},
	{"MMDDHMS", MMDDHMS, NULL},
	{"MMDDYY", MMDDYY, NULL},
	{"MMDDYYHMS", MMDDYYHMS, NULL},
	{"MMSSLAT", MMSSLAT, NULL},
	{"MMSSLON", MMSSLON, NULL},
	{"MMYY", MMYY, NULL},
	{"MOD", FUNC_DD, (void *) fmod},
	{"MONTHDAY", MONTHDAY, NULL},
	{"MONTHL", MONTHL, NULL},
	{"MONTHS", MONTHS, NULL},
	{"MONTHSY", MONTHSY, NULL},
	{"NDTR", FUNC_D, (void *) ndtr},
	{"NDTRI", FUNC_D, (void *) ndtri},
	{"NE", NE, NULL},
	{"NEGATE", NEGATE, NULL},
	{"NONE", NONE, NULL},
	{"NORM", FUNC_D, (void *) fx},
	{"NORMAL", NORMAL, NULL},
	{"NOT", NOT, NULL},
	{"OFF", OFF, NULL},
	{"OFFSET", OFFSET, NULL},
	{"OFFSETX", OFFSETX, NULL},
	{"OFFSETY", OFFSETY, NULL},
	{"OFILTER", OFILTER, NULL},
	{"ON", ON, NULL},
	{"OP", OP, NULL},
	{"OPPOSITE", OPPOSITE, NULL},
	{"OR", OR, NULL},
	{"OUT", OUT, NULL},
	{"PAGE", PAGE, NULL},
	{"PARA", PARA, NULL},
	{"PATTERN", PATTERN, NULL},
	{"PDTR", FUNC_ND, (void *) pdtr},
	{"PDTRC", FUNC_ND, (void *) pdtrc},
	{"PDTRI", FUNC_ND, (void *) pdtri},
	{"PERP", PERP, NULL},
	{"PI", CONSTANT, (void *) pi_const},
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
	{"PSI", FUNC_D, (void *) psi},
	{"RAD", UCONSTANT, (void *) rad_uconst},
	{"RAND", RAND, NULL},
	{"RECIPROCAL", RECIPROCAL, NULL},
	{"REFERENCE", REFERENCE, NULL},
	{"RGAMMA", FUNC_D, (void *) rgamma},
	{"RIGHT", RIGHT, NULL},
	{"RINT", FUNC_D, (void *) rint},
	{"RISER", RISER, NULL},
	{"RNORM", FUNC_DD, (void *) rnorm},
	{"ROT", ROT, NULL},
	{"ROUNDED", ROUNDED, NULL},
	{"RULE", RULE, NULL},
	{"SCALE", SCALE, NULL},
	{"SCIENTIFIC", SCIENTIFIC, NULL},
	{"SCROLL", SCROLL, NULL},
	{"SD", SD, NULL},
	{"SET", SET, NULL},
	{"SFORMAT", SFORMAT, NULL},
	{"SHI", FUNC_D, (void *) shi_wrap},
	{"SI", FUNC_D, (void *) si_wrap},
	{"SIGN", SIGN, NULL},
	{"SIN", FUNC_D, (void *) sin},
	{"SINH", FUNC_D, (void *) sinh},
	{"SIZE", SIZE, NULL},
	{"SKIP", SKIP, NULL},
	{"SMITH", SMITH, NULL},
	{"SOURCE", SOURCE, NULL},
	{"SPEC", SPEC, NULL},
	{"SPENCE", FUNC_D, (void *) spence},
	{"SQR", FUNC_D, (void *) sqr_wrap},
	{"SQRT", FUNC_D, (void *) sqrt},
	{"STACK", STACK, NULL},
	{"STACKED", STACKED, NULL},
	{"STACKEDBAR", STACKEDBAR, NULL},
	{"STACKEDHBAR", STACKEDHBAR, NULL},
	{"STAGGER", STAGGER, NULL},
	{"START", START, NULL},
	{"STDTR", FUNC_ND, (void *) stdtr},
	{"STDTRI", FUNC_ND, (void *) stdtri},
	{"STOP", STOP, NULL},
	{"STRING", STRING, NULL},
	{"STRUVE", FUNC_DD, (void *) struve},
	{"SUBTITLE", SUBTITLE, NULL},
	{"SYMBOL", SYMBOL, NULL},
	{"TAN", FUNC_D, (void *) tan},
	{"TANH", FUNC_D, (void *) tanh},
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
	{"UNIT", KEY_UNIT, NULL},
	{"UP", UP, NULL},
	{"USE", USE, NULL},
	{"VERSION", VERSION, NULL},
	{"VERTI", VERTI, NULL},
	{"VERTICAL", VERTICAL, NULL},
	{"VERTO", VERTO, NULL},
	{"VGAP", VGAP, NULL},
	{"VIEW", VIEW, NULL},
	{"VX1", VX1, NULL},
	{"VX2", VX2, NULL},
	{"VXMAX", VXMAX, NULL},
	{"VY1", VY1, NULL},
	{"VY2", VY2, NULL},
	{"VYMAX", VYMAX, NULL},
	{"WITH", WITH, NULL},
	{"WORLD", WORLD, NULL},
	{"WRAP", WRAP, NULL},
	{"WX1", WX1, NULL},
	{"WX2", WX2, NULL},
	{"WY1", WY1, NULL},
	{"WY2", WY2, NULL},
	{"X", X_TOK, NULL},
	{"X0", X0, NULL},
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
	{"Y", Y_TOK, NULL},
	{"Y0", Y0, NULL},
	{"Y1", Y1, NULL},
	{"Y2", Y2, NULL},
	{"Y3", Y3, NULL},
	{"Y4", Y4, NULL},
	{"YAXES", YAXES, NULL},
	{"YAXIS", YAXIS, NULL},
	{"YEAR", YEAR, NULL},
	{"YMAX", YMAX, NULL},
	{"YMIN", YMIN, NULL},
	{"YV", FUNC_DD, (void *) yv_wrap},
	{"YYMMDD", YYMMDD, NULL},
	{"YYMMDDHMS", YYMMDDHMS, NULL},
	{"ZERO", ZERO, NULL},
	{"ZEROXAXIS", ALTXAXIS, NULL},
	{"ZEROYAXIS", ALTYAXIS, NULL},
	{"ZETA", FUNC_DD, (void *) zeta},
	{"ZETAC", FUNC_D, (void *) zetac},
	{"ZNORM", ZNORM, NULL}
};

static int maxfunc = sizeof(ikey) / sizeof(symtab_entry);

Quark *get_parser_gno(void)
{
    return whichgraph;
}

int set_parser_gno(Quark *gr)
{
    if (gr) {
        whichgraph = gr;
        whichframe = get_parent_frame(gr);
        return RETURN_SUCCESS;
    } else {
        whichframe = NULL;
        return RETURN_FAILURE;
    }
}

Quark *get_parser_setno(void)
{
    return whichset;
}

int set_parser_setno(Quark *pset)
{
    if (pset) {
        whichgraph = get_parent_graph(pset);
        whichframe = get_parent_frame(whichgraph);
        whichset = pset;
        /* those will usually be overridden except when evaluating
           a _standalone_ vexpr */
        vasgn_pset = pset;
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

void realloc_vrbl(grarr *vrbl, int len)
{
    double *a;
    int i, oldlen;
    
    if (vrbl->type != GRARR_VEC) {
        errmsg("Internal error");
        return;
    }
    oldlen = vrbl->length;
    if (oldlen == len) {
        return;
    } else {
        a = xrealloc(vrbl->data, len*SIZEOF_DOUBLE);
        if (a != NULL || len == 0) {
            vrbl->data = a;
            vrbl->length = len;
            for (i = oldlen; i < len; i++) {
                vrbl->data[i] = 0.0;
            }
        } else {
            errmsg("Malloc failed in realloc_vrbl()");
        }
    }
}


#define PARSER_TYPE_VOID    0
#define PARSER_TYPE_EXPR    1
#define PARSER_TYPE_VEXPR   2

static int parser(char *s, int type)
{
    char *seekpos;
    int i;
    
    if (is_empty_string(s)) {
        if (type == PARSER_TYPE_VOID) {
            /* don't consider an empty string as error for generic parser */
            return RETURN_SUCCESS;
        } else {
            return RETURN_FAILURE;
        }
    }
    
    strncpy(f_string, s, MAX_PARS_STRING_LENGTH - 2);
    f_string[MAX_PARS_STRING_LENGTH - 2] = '\0';
    strcat(f_string, " ");
    
    seekpos = f_string;

    while ((seekpos - f_string < MAX_PARS_STRING_LENGTH - 1) && (*seekpos == ' ' || *seekpos == '\t')) {
        seekpos++;
    }
    if (*seekpos == '\n' || *seekpos == '#') {
        if (type == PARSER_TYPE_VOID) {
            /* don't consider an empty string as error for generic parser */
            return RETURN_SUCCESS;
        } else {
            return RETURN_FAILURE;
        }
    }
    
    lowtoupper(f_string);
        
    pos = 0;
    interr = 0;
    expr_parsed  = FALSE;
    vexpr_parsed = FALSE;
    
    yyparse();

    /* free temp. arrays; for a vector expression keep the last one
     * (which is none but v_result), given there have been no errors
     * and it's what we've been asked for
     */
    if (vexpr_parsed && !interr && type == PARSER_TYPE_VEXPR) {
        for (i = 0; i < fcnt - 1; i++) {
            free_tmpvrbl(&(freelist[i]));
        }
    } else {
        for (i = 0; i < fcnt; i++) {
            free_tmpvrbl(&(freelist[i]));
        }
    }
    fcnt = 0;
    
    if ((type == PARSER_TYPE_VEXPR && !vexpr_parsed) ||
        (type == PARSER_TYPE_EXPR  && !expr_parsed)) {
        return RETURN_FAILURE;
    } else {
        return (interr ? RETURN_FAILURE:RETURN_SUCCESS);
    }
}

int s_scanner(char *s, double *res)
{
    int retval = parser(s, PARSER_TYPE_EXPR);
    *res = s_result;
    return retval;
}

int v_scanner(char *s, int *reslen, double **vres)
{
    int retval = parser(s, PARSER_TYPE_VEXPR);
    if (retval != RETURN_SUCCESS) {
        return RETURN_FAILURE;
    } else {
        *reslen = v_result->length;
        if (v_result->type == GRARR_TMP) {
            *vres = v_result->data;
            v_result->length = 0;
            v_result->data = NULL;
        } else {
            *vres = copy_data_column(v_result->data, v_result->length);
        }
        return RETURN_SUCCESS;
    }
}

int scanner(char *s)
{
    int retval = parser(s, PARSER_TYPE_VOID);
    if (retval != RETURN_SUCCESS) {
        return RETURN_FAILURE;
    }
    
    if (gotparams) {
	gotparams = FALSE;
        getparms(grace, paramfile);
    }
    
    return retval;
}

static void free_tmpvrbl(grarr *vrbl)
{
    if (vrbl->type == GRARR_TMP) {
        vrbl->length = 0;
        XCFREE(vrbl->data);
    }
}

static void copy_vrbl(grarr *dest, grarr *src)
{
    dest->type = src->type;
    dest->data = xmalloc(src->length*SIZEOF_DOUBLE);
    if (dest->data == NULL) {
        errmsg("Malloc failed in copy_vrbl()");
    } else {
        memcpy(dest->data, src->data, src->length*SIZEOF_DOUBLE);
        dest->length = src->length;
    }
}

void *get_parser_var_by_name(char * const name, int type)
{
    int position;
    char *s;

    s = copy_string(NULL, name);
    lowtoupper(s);

    position = findf(key, s);
    xfree(s);

    if (position >= 0 && key[position].type == type) {
       return key[position].data;
    }

    return NULL;
}

grarr *get_parser_arr_by_name(char * const name)
{
    return (grarr *) get_parser_var_by_name(name, KEY_VEC);
}

double *get_parser_scalar_by_name(char * const name)
{
    return (double *) get_parser_var_by_name(name, KEY_VAR);
}

double *define_parser_scalar(char * const name)
{
    if (get_parser_var_by_name(name, KEY_VAR) == NULL) {
        symtab_entry tmpkey;
        double *var;

        var = xmalloc(SIZEOF_DOUBLE);
        *var = 0.0;

        tmpkey.s = name;
        tmpkey.type = KEY_VAR;
        tmpkey.data = (void *) var;

        if (addto_symtab(tmpkey) == RETURN_SUCCESS) {
            return var;
        } else {
            return NULL;
        }
    } else {
        return NULL;
    }
}

grarr *define_parser_arr(char * const name)
{
     if (get_parser_var_by_name(name, KEY_VEC) == NULL) {
	symtab_entry tmpkey;
        grarr *var;
        
        var = xmalloc(sizeof(grarr));
        var->type = GRARR_VEC;
        var->length = 0;
        var->data = NULL;
        
	tmpkey.s = name;
	tmpkey.type = KEY_VEC;
	tmpkey.data = (void *) var;
	if (addto_symtab(tmpkey) == RETURN_SUCCESS) {
	    return var;
	} else {
            return NULL;
        }
     } else {
        return NULL;
     }
}

int undefine_parser_var(void *ptr)
{
    int i;
    
    for (i = 0; i < maxfunc; i++) {
	if (key[i].data == ptr) {
            xfree(key[i].s);
            maxfunc--;
            if (i != maxfunc) {
                memmove(&(key[i]), &(key[i + 1]), (maxfunc - i)*sizeof(symtab_entry));
            }
            key = xrealloc(key, maxfunc*sizeof(symtab_entry));
            return RETURN_SUCCESS;
        }
    }
    return RETURN_FAILURE;
}

#if 0
static int find_set_bydata(double *data, Quark **pset)
{
    if (data == NULL) {
        return RETURN_FAILURE;
    } else {
	int gno;
        int ngraphs = number_of_graphs(grace->project);
        for (gno = 0; gno < ngraphs; gno++) {
	    int setno;
            int nsets = number_of_sets(gno);
	    for (setno = 0; setno < nsets; setno++) {
                int ncol;
                for (ncol = 0; ncol < MAX_SET_COLS; ncol++) {
                    if (set_get_col(gno, setno, ncol) == data) {
                        tgt->gno   = gno;
                        tgt->setno = setno;
                        return RETURN_SUCCESS;
                    }
                }
	    }
	}
    }
    return RETURN_FAILURE;
}
#endif

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

static int compare_keys (const void *a, const void *b)
{
    return (int) strcmp (((const symtab_entry*)a)->s,
                         ((const symtab_entry*)b)->s);
}

/* add new entry to the symbol table */
int addto_symtab(symtab_entry newkey)
{
    int position;
    char *s;
    
    s = copy_string(NULL, newkey.s);
    lowtoupper(s);
    if ((position = findf(key, s)) < 0) {
        if ((key = (symtab_entry *) xrealloc(key, (maxfunc + 1)*sizeof(symtab_entry))) != NULL) {
	    key[maxfunc].type = newkey.type;
	    key[maxfunc].data = newkey.data;
	    key[maxfunc].s = s;
	    maxfunc++;
	    qsort(key, maxfunc, sizeof(symtab_entry), compare_keys);
	    return RETURN_SUCCESS;
	} else {
	    xfree(s);
	    return RETURN_FAILURE;
	}
    } else if (alias_force == TRUE) { /* already exists but alias_force enabled */
        key[position].type = newkey.type;
	key[position].data = newkey.data;
	return RETURN_SUCCESS;
    } else {
	xfree(s);
        return RETURN_FAILURE;
    }
}

/* initialize symbol table */
void init_symtab(void)
{
    int i;
    
    if ((key = (symtab_entry *) xmalloc(maxfunc*sizeof(symtab_entry))) != NULL) {
    	memcpy (key, ikey, maxfunc*sizeof(symtab_entry));
	for (i = 0; i < maxfunc; i++) {
	    key[i].s = xmalloc(strlen(ikey[i].s) + 1);
	    strcpy(key[i].s, ikey[i].s);
	}
	qsort(key, maxfunc, sizeof(symtab_entry), compare_keys);
	return;
    } else {
	key = ikey;
	return;
    }
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
	while (isdigit(c) || c == '$' || c == '_') {
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
    if (isalpha(c) || c == '$') {
	char *p = sbuf;

	do {
	    *p++ = c;
	} while ((c = getcharstr()) != EOF && (isalpha(c) || isdigit(c) ||
                  c == '_' || c == '$'));
	ungetchstr();
	*p = '\0';
#ifdef DEBUG
        if (get_debuglevel(grace) == 2) {
	    printf("->%s<-\n", sbuf);
	}
#endif
	found = -1;
	if ((found = findf(key, sbuf)) >= 0) {
	    if (key[found].type == KEY_VAR) {
		yylval.dptr = (double *) key[found].data;
		return VAR_D;
	    }
	    else if (key[found].type == KEY_VEC) {
		yylval.vrbl = (grarr *) key[found].data;
		return VEC_D;
	    }

	    else if (key[found].type == FUNC_I) {
		yylval.ival = found;
		return FUNC_I;
	    }
	    else if (key[found].type == CONSTANT) {
		yylval.ival = found;
		return CONSTANT;
	    }
	    else if (key[found].type == UCONSTANT) {
		yylval.ival = found;
		return UCONSTANT;
	    }
	    else if (key[found].type == FUNC_D) {
		yylval.ival = found;
		return FUNC_D;
	    }
	    else if (key[found].type == FUNC_ND) {
		yylval.ival = found;
		return FUNC_ND;
	    }
	    else if (key[found].type == FUNC_DD) {
		yylval.ival = found;
		return FUNC_DD;
	    }
	    else if (key[found].type == FUNC_NND) {
		yylval.ival = found;
		return FUNC_NND;
	    }
	    else if (key[found].type == FUNC_PPD) {
		yylval.ival = found;
		return FUNC_PPD;
	    }
	    else if (key[found].type == FUNC_PPPD) {
		yylval.ival = found;
		return FUNC_PPPD;
	    }
	    else {
	        yylval.ival = key[found].type;
	        return key[found].type;
	    }
	} else {
	    yylval.sval = copy_string(NULL, sbuf);
	    return NEW_TOKEN;
	}
    }
    switch (c) {
    case '>':
	return follow('=', GE, GT);
    case '<':
	return follow('=', LE, LT);
    case '=':
	return follow('=', EQ, '=');
    case '!':
	return follow('=', NE, NOT);
    case '|':
	return follow('|', OR, '|');
    case '&':
	return follow('&', AND, '&');
    case '\n':
	return '\n';
    default:
	return c;
    }
}

static int follow(int expect, int ifyes, int ifno)
{
    int c = getcharstr();

    if (c == expect) {
	return ifyes;
    }
    ungetchstr();
    return ifno;
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

void parser_state_reset(void)
{
    whichframe = NULL;
    whichgraph = NULL;
    whichset   = NULL;
    curtm      = NULL;
    curobject  = NULL;
    objgno     = NULL;
}
