%{
/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-95 Paul J Turner, Portland, OR
 * Copyright (c) 1996-99 Grace Development Team
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
#include <cmath.h>

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
#include "cephes/cephes.h"
#include "device.h"
#include "utils.h"
#include "files.h"
#include "graphs.h"
#include "graphutils.h"
#include "plotone.h"
#include "dlmodule.h"
#include "t1fonts.h"
#include "protos.h"

extern graph *g;

double result;		/* return value if expression */

static int interr;

static double *freelist[100]; 	/* temporary vectors */
static int fcnt;		/* number allocated */

int naxis = 0;	/* current axis */
static int curline, curbox, curellipse, curstring;
/* these guys attempt to avoid reentrancy problems */
static int gotbatch = 0, gotparams = 0, gotread = 0, gotnlfit = 0; 
int readtype, readsrc, readxformat;
static int nlfit_gno, nlfit_setno, nlfit_nsteps;

char batchfile[GR_MAXPATHLEN] = "",
     paramfile[GR_MAXPATHLEN] = "",
     readfile[GR_MAXPATHLEN] = "";

static char f_string[MAX_STRING_LENGTH]; /* buffer for string to parse */
static int pos = 0;

/* scratch arrays used in scanner */
static int maxarr = 0;
static double *ax = NULL, *bx = NULL, *cx = NULL, *dx = NULL;

static int lxy;
static int whichgraph;
static int whichset;

static int alias_force = FALSE; /* controls whether aliases can override
                                                       existing keywords */

extern char print_file[];
extern int change_type;
extern char *close_input;

static int check_err;

static int filltype_obs;

static int index_shift = 0;     /* 0 for C, 1 for F77 index notation */

double rnorm(double mean, double sdev);
double fx(double x);
double vmin(double *x, int n);
double vmax(double *x, int n);
void set_prop(int gno,...);
int getcharstr(void);
void ungetchstr(void);
int follow(int expect, int ifyes, int ifno);

static double ai_wrap(double x);
static double bi_wrap(double x);
static double ci_wrap(double x);
static double si_wrap(double x);
static double chi_wrap(double x);
static double shi_wrap(double x);
static double fresnlc_wrap(double x);
static double fresnls_wrap(double x);
static double iv_wrap(double v, double x);
static double jv_wrap(double v, double x);
static double kn_wrap(int n, double x);
static double yv_wrap(double v, double x);
static double sqr_wrap(double x);
static double max_wrap(double x, double y);
static double min_wrap(double x, double y);
static double irand_wrap(int x);

/* constants */
static double pi_const(void);
static double deg_uconst(void);
static double rad_uconst(void);


int yylex(void);
int yyparse(void);
void yyerror(char *s);

int findf(symtab_entry *keytable, char *s);

/* Total (intrinsic + user-defined) list of functions and keywords */
symtab_entry *key;

%}

%union {
    double val;
    long ival;
    double *ptr;
    long func;
    long pset;
    char *str;
}

%token <func> INDEX
%token <func> JDAY
%token <func> JDAY0

%token <func> CONSTANT	 /* a (double) constant                                     */
%token <func> UCONSTANT	 /* a (double) unit constant                                */
%token <func> FUNC_I	 /* a function of 1 int variable                            */
%token <func> FUNC_D	 /* a function of 1 double variable                         */
%token <func> FUNC_NN    /* a function of 2 int parameters                          */
%token <func> FUNC_ND    /* a function of 1 int parameter and 1 double variable     */
%token <func> FUNC_DD    /* a function of 2 double variables                        */
%token <func> FUNC_NND   /* a function of 2 int parameters and 1 double variable    */
%token <func> FUNC_PPD   /* a function of 2 double parameters and 1 double variable */
%token <func> FUNC_PPPD  /* a function of 3 double parameters and 1 double variable */
%token <pset> PROC_CONST
%token <pset> PROC_UNIT
%token <pset> PROC_FUNC_I
%token <pset> PROC_FUNC_D
%token <pset> PROC_FUNC_NN
%token <pset> PROC_FUNC_ND
%token <pset> PROC_FUNC_DD
%token <pset> PROC_FUNC_NND
%token <pset> PROC_FUNC_PPD
%token <pset> PROC_FUNC_PPPD

%token <pset> ABOVE
%token <pset> ABSOLUTE
%token <pset> ALIAS
%token <pset> ALT
%token <pset> ALTXAXIS
%token <pset> ALTYAXIS
%token <pset> ANGLE
%token <pset> ANTIALIASING
%token <pset> APPEND
%token <pset> ARRANGE
%token <pset> ARROW
%token <pset> ASCENDING
%token <pset> ASPLINE
%token <pset> AUTO
%token <pset> AUTOSCALE
%token <pset> AUTOTICKS
%token <pset> AVALUE
%token <pset> AVG
%token <pset> AXES
%token <pset> BACKGROUND
%token <pset> BAR
%token <pset> BARDY
%token <pset> BARDYDY
%token <pset> BASELINE
%token <pset> BATCH
%token <pset> BEGIN
%token <pset> BELOW
%token <pset> BETWEEN
%token <pset> BLACKMAN
%token <pset> BLOCK
%token <pset> BOTH
%token <pset> BOTTOM
%token <pset> BOX
%token <pset> CD
%token <pset> CENTER
%token <pset> CHAR
%token <pset> CHART
%token <pset> CHRSTR
%token <pset> CLEAR
%token <pset> CLICK
%token <pset> CLOSE
%token <pset> COEFFICIENTS
%token <pset> COLOR
%token <pset> COMMENT
%token <pset> COMPLEX
%token <pset> CONSTRAINTS
%token <pset> COPY
%token <pset> CYCLE
%token <pset> DAYMONTH
%token <pset> DAYOFWEEKL
%token <pset> DAYOFWEEKS
%token <pset> DAYOFYEAR
%token <pset> DDMMYY
%token <pset> DECIMAL
%token <pset> DEF
%token <pset> DEFAULT
%token <pset> DEFINE
%token <pset> DEGREESLAT
%token <pset> DEGREESLON
%token <pset> DEGREESMMLAT
%token <pset> DEGREESMMLON
%token <pset> DEGREESMMSSLAT
%token <pset> DEGREESMMSSLON
%token <pset> DESCENDING
%token <pset> DESCRIPTION
%token <pset> DEVICE
%token <pset> DFT
%token <pset> DIFFERENCE
%token <pset> DISK
%token <pset> DOWN
%token <pset> DPI
%token <pset> DROP
%token <pset> DROPLINE
%token <pset> ECHO
%token <pset> ELLIPSE
%token <pset> ENGINEERING
%token <pset> ERRORBAR
%token <pset> EXIT
%token <pset> EXPONENTIAL
%token <pset> FFT
%token <pset> FILEP
%token <pset> FILL
%token <pset> FIT
%token <pset> FIXED
%token <pset> FIXEDPOINT
%token <pset> FLUSH
%token <pset> FOCUS
%token <pset> FOLLOWS
%token <pset> FONTP
%token <pset> FORCE
%token <pset> FORMAT
%token <pset> FORMULA
%token <pset> FRAMEP
%token <pset> FREE
%token <pset> FREQUENCY
%token <pset> FROM
%token <pset> GENERAL
%token <pset> GETP
%token <pset> GRAPHNO
%token <pset> GRAPHS
%token <pset> GRID
%token <pset> HAMMING
%token <pset> HANNING
%token <pset> HARDCOPY
%token <pset> HBAR
%token <pset> HGAP
%token <pset> HIDDEN
%token <pset> HISTO
%token <pset> HMS
%token <pset> HORIZI
%token <pset> HORIZONTAL
%token <pset> HORIZO
%token <pset> ID
%token <pset> IFILTER
%token <pset> IN
%token <pset> INCREMENT
%token <pset> INOUT
%token <pset> INTEGRATE
%token <pset> INTERP
%token <pset> INVDFT
%token <pset> INVERT
%token <pset> INVFFT
%token <pset> JUST
%token <pset> KILL
%token <pset> LABEL
%token <pset> LANDSCAPE
%token <pset> LAYOUT
%token <pset> LEFT
%token <pset> LEGEND
%token <pset> LENGTH
%token <pset> LINE
%token <pset> LINESTYLE
%token <pset> LINEWIDTH
%token <pset> LINK
%token <pset> LOAD
%token <pset> LOCTYPE
%token <pset> LOG
%token <pset> LOGARITHMIC
%token <pset> LOGX
%token <pset> LOGXY
%token <pset> LOGY
%token <pset> MAGIC
%token <pset> MAGNITUDE
%token <pset> MAJOR
%token <pset> MAP
%token <pset> MAXP
%token <pset> MINP
%token <pset> MINOR
%token <pset> MMDD
%token <pset> MMDDHMS
%token <pset> MMDDYY
%token <pset> MMDDYYHMS
%token <pset> MMSSLAT
%token <pset> MMSSLON
%token <pset> MMYY
%token <pset> MONTHDAY
%token <pset> MONTHL
%token <pset> MONTHS
%token <pset> MONTHSY
%token <pset> MOVE
%token <pset> NEGATE
%token <pset> NEW
%token <pset> NONE
%token <pset> NONLFIT
%token <pset> NORMAL
%token <pset> NXY
%token <pset> OFF
%token <pset> OFFSET
%token <pset> OFFSETX
%token <pset> OFFSETY
%token <pset> OFILTER
%token <pset> ON
%token <pset> OP
%token <pset> OUT
%token <pset> PAGE
%token <pset> PARA
%token <pset> PARAMETERS
%token <pset> PARZEN
%token <pset> PATTERN
%token <pset> PERIOD
%token <pset> PERP
%token <pset> PHASE
%token <pset> PIPE
%token <pset> PLACE
%token <pset> POINT
%token <pset> POLAR
%token <pset> POLYI
%token <pset> POLYO
%token <pset> POP
%token <pset> PORTRAIT
%token <pset> POWER
%token <pset> PREC
%token <pset> PREPEND
%token <pset> PRINT
%token <pset> PS
%token <pset> PUSH
%token <pset> PUTP
%token <pset> READ
%token <pset> REAL
%token <pset> RECIPROCAL
%token <pset> REDRAW
%token <pset> REGNUM
%token <pset> REGRESS
%token <pset> RIGHT
%token <pset> RISER
%token <pset> ROT
%token <pset> ROUNDED
%token <pset> RULE
%token <pset> RUNAVG
%token <pset> RUNMAX
%token <pset> RUNMED
%token <pset> RUNMIN
%token <pset> RUNSTD
%token <pset> SAVEALL
%token <pset> SCALE
%token <pset> SCIENTIFIC
%token <pset> SCROLL
%token <pset> SD
%token <pset> SET
%token <pset> SETNUM
%token <pset> SETS
%token <pset> SFORMAT
%token <pset> SIGN
%token <pset> SIZE
%token <pset> SKIP
%token <pset> SLEEP
%token <pset> SMITH 
%token <pset> SORT
%token <pset> SOURCE
%token <pset> SPEC
%token <pset> SPLINE
%token <pset> STACK
%token <pset> STACKED
%token <pset> STACKEDBAR
%token <pset> STACKEDHBAR
%token <pset> STAGGER
%token <pset> START
%token <pset> STOP
%token <pset> STRING
%token <pset> SUBTITLE
%token <pset> SYMBOL
%token <pset> TARGET
%token <pset> TICKLABEL
%token <pset> TICKP
%token <pset> TICKSP
%token <pset> TIMER
%token <pset> TIMESTAMP
%token <pset> TITLE
%token <pset> TO
%token <pset> TOP
%token <pset> TRIANGULAR
%token <pset> TYPE
%token <pset> UP
%token <pset> USE
%token <pset> UNLINK
%token <pset> VERSION
%token <pset> VERTI
%token <pset> VERTICAL
%token <pset> VERTO
%token <pset> VGAP
%token <pset> VIEW
%token <pset> VX1
%token <pset> VX2
%token <pset> VXMAX
%token <pset> VY1
%token <pset> VY2
%token <pset> VYMAX
%token <pset> WELCH
%token <pset> WITH
%token <pset> WORLD
%token <pset> WRITE
%token <pset> WX1
%token <pset> WX2
%token <pset> WY1
%token <pset> WY2
%token <pset> X_TOK
%token <pset> X0
%token <pset> X1
%token <pset> XAXES
%token <pset> XAXIS
%token <pset> XCOR
%token <pset> XMAX
%token <pset> XMIN
%token <pset> XY
%token <pset> XYDX
%token <pset> XYDXDX
%token <pset> XYDXDY
%token <pset> XYDY
%token <pset> XYDYDY
%token <pset> XYHILO
%token <pset> XYR
%token <pset> XYSTRING
%token <pset> XYZ
%token <pset> Y_TOK
%token <pset> Y0
%token <pset> Y1
%token <pset> Y2
%token <pset> Y3
%token <pset> Y4
%token <pset> YAXES
%token <pset> YAXIS
%token <pset> YMAX
%token <pset> YMIN
%token <pset> YYMMDD
%token <pset> YYMMDDHMS
%token <pset> ZERO

%token <ival> SCRARRAY

%token <val> FITPARM
%token <val> FITPMAX
%token <val> FITPMIN
%token <val> NUMBER

%type <pset> colpat_obs
%type <pset> direction

%type <pset> extremetype

%type <pset> filtermethod
%type <pset> filtertype

%type <pset> formatchoice
%type <pset> graphtype
%type <pset> inoutchoice
%type <pset> justchoice
%type <pset> color_select
%type <pset> pattern_select
%type <pset> font_select
%type <pset> lines_select
%type <pset> onoff
%type <pset> opchoice
%type <pset> pagelayout
%type <pset> pageorient
%type <pset> regiontype
%type <pset> runtype
%type <pset> scaletype
%type <pset> selectsets
%type <pset> signchoice
%type <pset> sourcetype
%type <pset> vector
%type <pset> worldview
%type <pset> xytype

%type <pset> proctype

%type <pset> ffttype
%type <pset> fourierdata
%type <pset> fourierloadx
%type <pset> fourierloady
%type <pset> nonlfitopts
%type <pset> sortdir
%type <pset> sorton
%type <pset> windowtype

%type <val> expr
%type <val> linew_select

%type <ptr> vexpr
%type <ptr> asgn
%type <ptr> vasgn

/* Precedence */
%right '='
%nonassoc '?' ':'
%right UCONSTANT
%left OR
%left AND
%nonassoc GT LT LE GE EQ NE
%left '+' '-'
%left '*' '/' '%'
%left UMINUS NOT	/* negation--unary minus */
%right '^'		/* exponentiation        */


%%

list:
	parmset {}
	| parmset_obs {}
	| regionset {}
	| setaxis {}
	| set_setprop {}
	| actions {}
	| options {}
        | expr {
            result = $1;
        }
        | vexpr {
            result = *$1;
        }
	| asgn {}
	| vasgn {}
	| error {
	    return 1;
	}
	;



expr:	NUMBER {
	    $$ = $1;
	}
	|  FITPARM {
	    $$ = nonl_parms[(int) $1].value;
	}
	|  FITPMAX {
	    $$ = nonl_parms[(int) $1].max;
	}
	|  FITPMIN {
	    $$ = nonl_parms[(int) $1].min;
	}
	|  SCRARRAY '[' expr ']' {
	    int itmp = (int) $3 - index_shift;
	    if (itmp >= maxarr || itmp < 0) {
		yyerror("Access beyond array bounds");
		return 1;
	    } else {
	        double *ptr = get_scratch((int) $1);
                $$ = ptr[itmp];
            }
	}
	| vector '[' expr ']' {
	    double *ptr = getcol(get_cg(), curset, $1);
	    if (ptr != NULL) {
		$$ = ptr[(int) $3 - index_shift];
	    }
	    else {
		yyerror("NULL variable, check set type");
		return 1;
	    }
	}
	| SETNUM '.' vector '[' expr ']' {
	    double *ptr = getcol(get_cg(), $1, $3);
	    if (ptr != NULL) {
		$$ = ptr[(int) $5 - index_shift];
	    }
	    else {
		yyerror("NULL variable, check set type");
		return 1;
	    }
	}
	| GRAPHNO '.' SETNUM '.' vector '[' expr ']' {
	    double *ptr = getcol($1, $3, $5);
	    if (ptr != NULL) {
		$$ = ptr[(int) $7 - index_shift];
	    }
	    else {
		yyerror("NULL variable, check set type");
		return 1;
	    }
	}
	| SETNUM '.' vector '.' extremetype {
	    double bar, sd;
	    double *ptr = getcol(get_cg(), $1, $3);
	    if (ptr == NULL) {
		yyerror("NULL variable, check set type");
		return 1;
	    }
	    switch ((int) $5) {
	    case MINP:
		$$ = vmin(ptr, getsetlength(get_cg(), $1));
		break;
	    case MAXP:
		$$ = vmax(ptr, getsetlength(get_cg(), $1));
		break;
            case AVG:
	        stasum(ptr, getsetlength(get_cg(), $1), &bar, &sd);
	        $$ = bar;
                break;
            case SD:
	        stasum(ptr, getsetlength(get_cg(), $1), &bar, &sd);
                $$ = sd;
                break;
	    }
	}
	| GRAPHNO '.' SETNUM '.' vector '.' extremetype {
	    double bar, sd;
	    double *ptr = getcol($1, $3, $5);
	    if (ptr == NULL) {
		yyerror("NULL variable, check set type");
		return 1;
	    }
	    switch ((int) $7) {
	    case MINP:
		$$ = vmin(ptr, getsetlength($1, $3));
		break;
	    case MAXP:
		$$ = vmax(ptr, getsetlength($1, $3));
		break;
            case AVG:
		stasum(ptr, getsetlength($1, $3), &bar, &sd);
	        $$ = bar;
                break;
            case SD:
		stasum(ptr, getsetlength($1, $3), &bar, &sd);
                $$ = sd;
                break;
	    }
	}
	| vector '.' extremetype {
	    double bar, sd;
	    double *ptr = getcol(get_cg(), curset, $1);
	    if (ptr == NULL) {
		yyerror("NULL variable, check set type");
		return 1;
	    }
	    switch ((int) $3) {
	    case MINP:
		$$ = vmin(ptr, getsetlength(get_cg(), curset));
		break;
	    case MAXP:
		$$ = vmax(ptr, getsetlength(get_cg(), curset));
		break;
            case AVG:;
		stasum(ptr, getsetlength(get_cg(), curset), &bar, &sd);
	        $$ = bar;
                break;
            case SD:
		stasum(ptr, getsetlength(get_cg(), curset), &bar, &sd);
                $$ = sd;
                break;
	    }
	}
	| GRAPHNO '.' SETNUM '.' LENGTH {
	    $$ = getsetlength($1, $3);
	}
	| SETNUM '.' LENGTH {
	    $$ = getsetlength(get_cg(), $1);
	}
	| LENGTH {
	    $$ = getsetlength(get_cg(), curset);
	}
	| GRAPHNO '.' SETNUM '.' ID {
	    $$ = $3;
	}
	| SETNUM '.' ID {
	    $$ = $1;
	}
	| GRAPHNO '.' ID {
	    $$ = $1;
	}
	| CONSTANT
	{
	    $$ = key[$1].fnc();
	}
	| expr UCONSTANT
	{
	    $$ = $1 * key[$2].fnc();
	}
	| FUNC_I '(' expr ')'
	{
	    $$ = key[$1].fnc((int) $3);
	}
	| FUNC_D '(' expr ')'
	{
	    $$ = key[$1].fnc($3);
	}
	| FUNC_ND '(' expr ',' expr ')'
	{
	    $$ = key[$1].fnc((int) $3, $5);
	}
	| FUNC_NN '(' expr ',' expr ')'
	{
	    $$ = key[$1].fnc((int) $3, (int) $5);
	}
	| FUNC_DD '(' expr ',' expr ')'
	{
	    $$ = key[$1].fnc($3, $5);
	}
	| FUNC_NND '(' expr ',' expr ',' expr ')'
	{
	    $$ = key[$1].fnc((int) $3, (int) $5, $7);
	}
	| FUNC_PPD '(' expr ',' expr ',' expr ')'
	{
	    $$ = key[$1].fnc($3, $5, $7);
	}
	| FUNC_PPPD '(' expr ',' expr ',' expr ',' expr ')'
	{
	    $$ = key[$1].fnc($3, $5, $7, $9);
	}
	| GRAPHNO '.' VX1 {
	    $$ = g[$1].v.xv1;
	}
	| GRAPHNO '.' VX2 {
	    $$ = g[$1].v.xv2;
	}
	| GRAPHNO '.' VY1 {
	    $$ = g[$1].v.yv1;
	}
	| GRAPHNO '.' VY2 {
	    $$ = g[$1].v.yv2;
	}
	| GRAPHNO '.' WX1 {
	    $$ = g[$1].w.xg1;
	}
	| GRAPHNO '.' WX2 {
	    $$ = g[$1].w.xg2;
	}
	| GRAPHNO '.' WY1 {
	    $$ = g[$1].w.yg1;
	}
	| GRAPHNO '.' WY2 {
	    $$ = g[$1].w.yg2;
	}
	| JDAY '(' expr ',' expr ',' expr ')' { /* yr, mo, day */
	    $$ = julday((int) $5, (int) $7, (int) $3, 12, 0, 0.0);
	}
	| JDAY0 '(' expr ',' expr ',' expr ',' expr ',' expr ',' expr ')' 
	{ /* yr, mo, day, hr, min, sec */
	    $$ = julday((int) $5, (int) $7, (int) $3, (int) $9, (int) $11, (double) $13);
	}
	| VX1 {
	    $$ = g[get_cg()].v.xv1;
	}
	| VX2 {
	    $$ = g[get_cg()].v.xv2;
	}
	| VY1 {
	    $$ = g[get_cg()].v.yv1;
	}
	| VY2 {
	    $$ = g[get_cg()].v.yv2;
	}
	| WX1 {
	    $$ = g[get_cg()].w.xg1;
	}
	| WX2 {
	    $$ = g[get_cg()].w.xg2;
	}
	| WY1 {
	    $$ = g[get_cg()].w.yg1;
	}
	| WY2 {
	    $$ = g[get_cg()].w.yg2;
	}
	| VXMAX {
	    double vx, vy;
            get_page_viewport(&vx, &vy);
            $$ = vx;
	}
	| VYMAX {
	    double vx, vy;
            get_page_viewport(&vx, &vy);
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
	| expr '*' expr {
	    $$ = $1 * $3;
	}
	| expr '/' expr
	{
	    if ($3 != 0.0) {
		$$ = $1 / $3;
	    } else {
		yyerror("Divide by Zero");
		return 1;
	    }
	}
	| expr '%' expr {
	    $$ = fmod($1, $3);
	}
	| expr '^' expr {
	    $$ = pow($1, $3);
	}
	| expr '?' expr ':' expr {
	    if ((int) $1) {
		$$ = $3;
	    } else {
		$$ = $5;
	    }
	}
	| '(' expr GT expr ')' {
	    $$ = ($2 > $4);
	}
	| '(' expr LT expr ')' {
	    $$ = ($2 < $4);
	}
	| '(' expr LE expr ')' {
	    $$ = ($2 <= $4);
	}
	| '(' expr GE expr ')' {
	    $$ = ($2 >= $4);
	}
	| '(' expr EQ expr ')' {
	    $$ = ($2 == $4);
	}
	| '(' expr NE expr ')' {
	    $$ = ($2 != $4);
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

vexpr:
	SCRARRAY
	{
	    if (lxy > maxarr) {
		yyerror("Access beyond array bounds");
		return 1;
	    } else {
	        int i;
	        double *ptr = get_scratch((int) $1);
	        $$ = calloc(lxy, SIZEOF_DOUBLE);
	        freelist[fcnt++] = $$;
	        for (i = 0; i < lxy; i++) {
	            $$[i] = ptr[i];
	        }
            }
	}
	| vector
	{
	    int i;
	    double *ptr = getcol(get_cg(), curset, $1);
	    if (ptr == NULL) {
		yyerror("NULL variable, check set type");
		return 1;
	    }
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = ptr[i];
	    }
	}
	| SETNUM '.' vector
	{
	    int i;
	    double *ptr = getcol(get_cg(), $1, $3);
	    if (ptr == NULL) {
		yyerror("NULL variable, check set type");
		return 1;
	    }
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = ptr[i];
	    }
	}
	| GRAPHNO '.' SETNUM '.' vector
	{
	    int i;
	    double *ptr = getcol($1, $3, $5);
	    if (ptr == NULL) {
		yyerror("NULL variable, check set type");
		return 1;
	    }
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = ptr[i];
	    }
	}
	| FUNC_I '(' vexpr ')'
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = key[$1].fnc((int) $3[i]);
	    }
	}
	| FUNC_D '(' vexpr ')'
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = key[$1].fnc($3[i]);
	    }
	}
	| FUNC_DD '(' vexpr ',' vexpr ')'
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = key[$1].fnc($3[i], $5[i]);
	    }
	}
	| FUNC_DD '(' expr ',' vexpr ')'
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = key[$1].fnc($3, $5[i]);
	    }
	}
	| FUNC_DD '(' vexpr ',' expr ')'
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = key[$1].fnc($3[i], $5);
	    }
	}
	| FUNC_ND '(' expr ',' vexpr ')'
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = key[$1].fnc((int) $3, $5[i]);
	    }
	}
	| FUNC_NND '(' expr ',' expr ',' vexpr ')'
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = key[$1].fnc((int) $3, (int) $5, $7[i]);
	    }
	}
	| FUNC_PPD '(' expr ',' expr ',' vexpr ')'
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = key[$1].fnc($3, $5, $7[i]);
	    }
	}
	| FUNC_PPPD '(' expr ',' expr ',' expr ',' vexpr ')'
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = key[$1].fnc($3, $5, $7, $9[i]);
	    }
	}
	| INDEX
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = i + index_shift;
	    }
	}
	| vexpr '+' vexpr
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $1[i] + $3[i];
	    }
	}
	| vexpr '+' expr
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $1[i] + $3;
	    }
	}
	| expr '+' vexpr
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $1 + $3[i];
	    }
	}
	| vexpr '-' vexpr
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $1[i] - $3[i];
	    }
	}
	| vexpr '-' expr
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $1[i] - $3;
	    }
	}
	| expr '-' vexpr
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $1 - $3[i];
	    }
	}
	| vexpr '*' vexpr
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $1[i] * $3[i];
	    }
	}
	| vexpr '*' expr
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $1[i] * $3;
	    }
	}
	| expr '*' vexpr
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $1 * $3[i];
	    }
	}
	| vexpr '/' vexpr
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		if ($3[i] == 0.0) {
		    yyerror("Divide by Zero");
		    return 1;
		}
	    }
	    for (i = 0; i < lxy; i++) {
		$$[i] = $1[i] / $3[i];
	    }
	}
	| vexpr '/' expr
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		if ($3 == 0.0) {
		    yyerror("Divide by Zero");
		    return 1;
		}
	    }
	    for (i = 0; i < lxy; i++) {
		$$[i] = $1[i] / $3;
	    }
	}
	| expr '/' vexpr
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		if ($3[i] == 0.0) {
		    yyerror("Divide by Zero");
		    return 1;
		}
	    }
	    for (i = 0; i < lxy; i++) {
		$$[i] = $1 / $3[i];
	    }
	}
	| vexpr '^' vexpr
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = pow($1[i], $3[i]);
	    }
	}
	| vexpr '^' expr
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = pow($1[i], $3);
	    }
	}
	| expr '^' vexpr
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = pow($1, $3[i]);
	    }
	}
	| vexpr UCONSTANT
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $1[i] * key[$2].fnc();
	    }
	}
	| vexpr '?' expr ':' expr {
           int i;
           $$ = calloc(lxy, SIZEOF_DOUBLE); 
           freelist[fcnt++] = $$;
           for (i = 0; i < lxy; i++) { 
               if ((int) $1[i]) {
                   $$[i] = $3;
               } else {
                   $$[i] = $5;
               }
           }
	}
	| vexpr '?' expr ':' vexpr {
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
	        if ((int) $1[i]) {
		    $$[i] = $3;
	        } else {
		    $$[i] = $5[i];
	        }
	    }
	}
	| vexpr '?' vexpr ':' expr {
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
	        if ((int) $1[i]) {
		    $$[i] = $3[i];
	        } else {
		    $$[i] = $5;
	        }
	    }
	}
	| vexpr '?' vexpr ':' vexpr {
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
	        if ((int) $1[i]) {
		    $$[i] = $3[i];
	        } else {
		    $$[i] = $5[i];
	        }
	    }
	}
	| '(' vexpr GT vexpr ')'
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $2[i] > $4[i];
	    }
	}
	| '(' expr GT vexpr ')'
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $2 > $4[i];
	    }
	}
	| '(' vexpr GT expr ')'
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $2[i] > $4;
	    }
	}
	| '(' vexpr LT vexpr ')'
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $2[i] < $4[i];
	    }
	}
	| '(' expr LT vexpr ')'
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $2 < $4[i];
	    }
	}
	| '(' vexpr LT expr ')'
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $2[i] < $4;
	    }
	}
	| '(' vexpr LE vexpr ')'
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $2[i] <= $4[i];
	    }
	}
	| '(' expr LE vexpr ')'
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $2 <= $4[i];
	    }
	}
	| '(' vexpr LE expr ')'
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $2[i] <= $4;
	    }
	}
	| '(' vexpr GE vexpr ')'
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $2[i] >= $4[i];
	    }
	}
	| '(' expr GE vexpr ')'
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $2 >= $4[i];
	    }
	}
	| '(' vexpr GE expr ')'
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $2[i] >= $4;
	    }
	}
	| '(' vexpr EQ vexpr ')'
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $2[i] == $4[i];
	    }
	}
	| '(' expr EQ vexpr ')'
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $2 == $4[i];
	    }
	}
	| '(' vexpr EQ expr ')'
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $2[i] == $4;
	    }
	}
	| '(' vexpr NE vexpr ')'
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $2[i] != $4[i];
	    }
	}
	| '(' expr NE vexpr ')'
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $2 != $4[i];
	    }
	}
	| '(' vexpr NE expr ')'
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $2[i] != $4;
	    }
	}
	| vexpr AND vexpr
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $1[i] && $3[i];
	    }
	}
	| vexpr AND expr
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $1[i] && $3;
	    }
	}
	| expr AND vexpr
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $1 && $3[i];
	    }
	}
	| vexpr OR vexpr
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $1[i] || $3[i];
	    }
	}
	| vexpr OR expr
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $1[i] || $3;
	    }
	}
	| expr OR vexpr
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $1 || $3[i];
	    }
	}
	| NOT vexpr
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = !($2[i]);
	    }
	}
	| '(' vexpr ')'
	{
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $2[i];
	    }
	}
	| '-' vexpr %prec UMINUS {
	    int i;
	    $$ = calloc(lxy, SIZEOF_DOUBLE);
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = -$2[i];
	    }
	}
	;


asgn:
	FITPARM '=' expr
	{
	    nonl_parms[(int) $1].value = $3;
	}
	| FITPMAX '=' expr
	{
	    nonl_parms[(int) $1].max = $3;
	}
	| FITPMIN '=' expr
	{
	    nonl_parms[(int) $1].min = $3;
	}
	| SCRARRAY '[' expr ']' '=' expr
	{
	    double *ptr;
	    int itmp = (int) $3 - index_shift;
	    if (itmp < 0) {
		yyerror("Access beyond array bounds");
                return 1;
	    } else if (itmp >= maxarr) {
		init_scratch_arrays(itmp + 1);
	    }
	    ptr = get_scratch((int) $1);
	    ptr[itmp] = $6;
	    result = $6;
	}
	| vector '[' expr ']' '=' expr
	{
	    int itmp = (int) $3 - index_shift;
	    double *ptr = getcol(get_cg(), curset, $1);
	    if (ptr != NULL) {
	        ptr[itmp] = $6;
	    }
	    else {
		yyerror("NULL variable, check set type");
		return 1;
	    }
	    result = $6;
	}
	| SETNUM '.' vector '[' expr ']' '=' expr
	{
	    int itmp = (int) $5 - index_shift;
	    double *ptr = getcol(get_cg(), $1, $3);
	    if (ptr != NULL) {
	        ptr[itmp] = $8;
	    }
	    else {
		yyerror("NULL variable, check set type");
		return 1;
	    }
	    result = $8;
	}
	| GRAPHNO '.' SETNUM '.' vector '[' expr ']' '=' expr
	{
	    int itmp = (int) $7 - index_shift;
	    double *ptr = getcol($1, $3, $5);
	    if (ptr != NULL) {
	        ptr[itmp] = $10;
	    }
	    else {
		yyerror("NULL variable, check set type");
		return 1;
	    }
	    result = $10;
	}
	;


vasgn:
	SCRARRAY '=' vexpr
	{
	    int i;
	    double *ptr;
	    if (lxy > maxarr) {
		init_scratch_arrays(lxy);
	    }
	    ptr = get_scratch((int) $1);
            for (i = 0; i < lxy; i++) {
		ptr[i] = $3[i];
	    }
	    result = $3[0];
	}
	| SCRARRAY '=' expr
	{
	    int i;
	    double *ptr;
	    if (lxy > maxarr) {
		init_scratch_arrays(lxy);
	    }
	    ptr = get_scratch((int) $1);
	    for (i = 0; i < lxy; i++) {
		ptr[i] = $3;
	    }
	    result = $3;
	}
	| vector '=' vexpr
	{
	    int i;
	    double *ptr;
	    if (!is_set_active(get_cg(), curset)) {
		setlength(get_cg(), curset, lxy);
		setcomment(get_cg(), curset, "Created");
	    }
	    ptr = getcol(get_cg(), curset, $1);
	    if (ptr != NULL) {
	        for (i = 0; i < lxy; i++) {
		    ptr[i] = $3[i];
	        }
	        result = $3[0];
	    }
	    else {
		yyerror("NULL variable, check set type");
		return 1;
	    }
	}
	| vector '=' expr
	{
	    int i;
	    double *ptr;
	    if (!is_set_active(get_cg(), curset)) {
		setlength(get_cg(), curset, lxy);
		setcomment(get_cg(), curset, "Created");
	    }
	    ptr = getcol(get_cg(), curset, $1);
	    if (ptr != NULL) {
	        for (i = 0; i < lxy; i++) {
		    ptr[i] = $3;
	        }
	        result = $3;
	    }
	    else {
		yyerror("NULL variable, check set type");
		return 1;
	    }
	}
	| SETNUM '.' vector '=' vexpr {
	    int i;
	    double *ptr;
	    if (!is_set_active(get_cg(), $1)) {
		setlength(get_cg(), $1, lxy);
		setcomment(get_cg(), $1, "Created");
	    }
	    ptr = getcol(get_cg(), $1, $3);
	    if (ptr != NULL) {
	        for (i = 0; i < lxy; i++) {
		    ptr[i] = $5[i];
	        }
	        result = $5[0];

	    }
	    else {
		yyerror("NULL variable, check set type");
		return 1;
	    }
	}
	| SETNUM '.' vector '=' expr {
	    int i;
	    double *ptr;
	    if (!is_set_active(get_cg(), $1)) {
		setlength(get_cg(), $1, lxy);
		setcomment(get_cg(), $1, "Created");
	    }
	    ptr = getcol(get_cg(), $1, $3);
	    if (ptr != NULL) {
	        for (i = 0; i < lxy; i++) {
		    ptr[i] = $5;
	        }
	        result = $5;

	    }
	    else {
		yyerror("NULL variable, check set type");
		return 1;
	    }
	}
	| GRAPHNO '.' SETNUM '.' vector '=' vexpr
	{
	    int i;
	    double *ptr;
	    
	    if (!is_set_active($1, $3)) {
		setlength($1, $3, lxy);
		setcomment($1, $3, "Created");
	    }
	    ptr = getcol($1, $3, $5);
	    if (ptr != NULL) {
	        for (i = 0; i < lxy; i++) {
		    ptr[i] = $7[i];
	        }
	        result = $7[0];

	    } else {
		yyerror("NULL variable, check set type");
		return 1;
	    }
	}
 	| GRAPHNO '.' SETNUM '.' vector '=' expr
	{
	    int i;
	    double *ptr;
	    
	    if (!is_set_active($1, $3)) {
		setlength($1, $3, lxy);
		setcomment($1, $3, "Created");
	    }
	    ptr = getcol($1, $3, $5);
	    if (ptr != NULL) {
	        for (i = 0; i < lxy; i++) {
		    ptr[i] = $7;
	        }
	        result = $7;

	    } else {
		yyerror("NULL variable, check set type");
		return 1;
	    }
	}
        ;

regionset:
	REGNUM onoff {
	    rg[$1].active = $2;
	}
	| REGNUM TYPE regiontype {
	    rg[$1].type = $3;
	}
	| REGNUM color_select {
	    rg[$1].color = (int) $2;
	}
	| REGNUM lines_select {
	    rg[$1].lines = (int) $2;
	}
	| REGNUM linew_select {
	    rg[$1].linew = $2;
	}
	| REGNUM LINE expr ',' expr ',' expr ',' expr
	{
	    rg[$1].x1 = $3;
	    rg[$1].y1 = $5;
	    rg[$1].x2 = $7;
	    rg[$1].y2 = $9;
	}
	| REGNUM XY expr ',' expr
	{
	    if (rg[$1].x == NULL || rg[$1].n == 0) {
		rg[$1].n = 0;
		rg[$1].x = calloc(1, SIZEOF_DOUBLE);
		rg[$1].y = calloc(1, SIZEOF_DOUBLE);
	    } else {
		rg[$1].x = xrealloc(rg[$1].x, (rg[$1].n + 1) * SIZEOF_DOUBLE);
		rg[$1].y = xrealloc(rg[$1].y, (rg[$1].n + 1) * SIZEOF_DOUBLE);
	    }
	    rg[$1].x[rg[$1].n] = $3;
	    rg[$1].y[rg[$1].n] = $5;
	    rg[$1].n++;
	}
	| LINK REGNUM TO GRAPHNO {
	    rg[$2].linkto[$4] = TRUE;
	}
	| UNLINK REGNUM FROM GRAPHNO {
	    rg[$2].linkto[$4]=FALSE;
	}
	;


parmset:
        VERSION NUMBER {
            if (set_project_version((int) $2) != GRACE_EXIT_SUCCESS) {
                errmsg("Project version is newer than software!");
            }
            if (get_project_version() < 50001) {
                map_fonts(FONT_MAP_ACEGR);
            } else {
                map_fonts(FONT_MAP_DEFAULT);
            }
        }
        | PAGE SIZE NUMBER NUMBER {
            Page_geometry pg;
            pg = get_page_geometry();
            pg.width =  (long) $3;
            pg.height = (long) $4;
            set_page_geometry(pg);
        }
        | DEVICE CHRSTR PAGE SIZE NUMBER NUMBER {
            int device_id;
            Device_entry dev;
            
            device_id = get_device_by_name((char *) $2);
            if (device_id < 0) {
                yyerror("Unknown device");
            } else {
                dev = get_device_props(device_id);
                dev.pg.width =  (long) $5;
                dev.pg.height = (long) $6;
                set_device_props(device_id, dev);
            }
            free((char *) $2);
        }
        | DEVICE CHRSTR DPI NUMBER NUMBER {
            int device_id;
            Device_entry dev;
            
            device_id = get_device_by_name((char *) $2);
            if (device_id < 0) {
                yyerror("Unknown device");
            } else {
                dev = get_device_props(device_id);
                dev.pg.dpi_x = (long) $4;
                dev.pg.dpi_y = (long) $5;
                set_device_props(device_id, dev);
            }
            free((char *) $2);
        }
        | DEVICE CHRSTR DPI NUMBER {
            int device_id;
            Device_entry dev;
            
            device_id = get_device_by_name((char *) $2);
            if (device_id < 0) {
                yyerror("Unknown device");
            } else {
                dev = get_device_props(device_id);
                dev.pg.dpi_x = dev.pg.dpi_y = (long) $4;
                set_device_props(device_id, dev);
            }
            free((char *) $2);
        }
        | DEVICE CHRSTR FONTP ANTIALIASING onoff {
            int device_id;
            Device_entry dev;
            
            device_id = get_device_by_name((char *) $2);
            if (device_id < 0) {
                yyerror("Unknown device");
            } else {
                dev = get_device_props(device_id);
                dev.fontaa = (int) $5;
                set_device_props(device_id, dev);
            }
            free((char *) $2);
        }
        | DEVICE CHRSTR FONTP onoff {
            int device_id;
            Device_entry dev;
            
            device_id = get_device_by_name((char *) $2);
            if (device_id < 0) {
                yyerror("Unknown device");
            } else {
                dev = get_device_props(device_id);
                dev.devfonts = (int) $4;
                set_device_props(device_id, dev);
            }
            free((char *) $2);
        }
        | DEVICE CHRSTR OP CHRSTR {
            int device_id;
            
            device_id = get_device_by_name((char *) $2);
            if (device_id < 0) {
                yyerror("Unknown device");
            } else {
                if (parse_device_options(device_id, (char *) $4) != 
                                                        GRACE_EXIT_SUCCESS) {
                    yyerror("Incorrect device option string");
                }
            }
            free((char *) $2);
            free((char *) $4);
        }
        | HARDCOPY DEVICE CHRSTR {
            set_printer_by_name((char *) $3);
            free((char *) $3);
        }
	| BACKGROUND color_select {
	    setbgcolor((int) $2);
	}
	| PAGE BACKGROUND FILL onoff {
	    setbgfill((int) $4);
	}
	| PAGE SCROLL NUMBER '%' {
	    scroll_proc((int) $3);
	}
	| PAGE INOUT NUMBER '%' {
	    scrollinout_proc((int) $3);
	}
	| LINK PAGE onoff {
	    scrolling_islinked = $3;
	}

	| STACK WORLD expr ',' expr ',' expr ',' expr
	{
	    add_world(get_cg(), $3, $5, $7, $9);
	}

	| TARGET SETNUM {
	    target_set.gno = get_cg();
	    target_set.setno = (int) $2;
	}
	| TARGET GRAPHNO '.' SETNUM {
            select_graph($2);
	    target_set.gno = $2;
	    target_set.setno = $4;
	}
	| TIMER NUMBER {
            timer_delay = (int) $2;
	}
	| WITH GRAPHNO {
            select_graph($2);
	}
	| WITH SETNUM {
	    curset = (int) $2;
	}

/* Hot links */
	| SETNUM LINK sourcetype CHRSTR {
	    set_hotlink(get_cg(), $1, 1, (char *) $4, $3);
	    free((char *) $4);
	}
	| GRAPHNO '.' SETNUM LINK sourcetype CHRSTR {
	    set_hotlink($1, $3, 1, (char *) $6, $5);
	    free((char *) $6);
	}
	| SETNUM LINK onoff {
	    set_hotlink(get_cg(), $1, $3, NULL, 0);
	}
	| GRAPHNO '.' SETNUM LINK onoff {
	    set_hotlink($1, $3, $5, NULL, 0);
	}

/* boxes */
	| WITH BOX {
	    curbox = next_box();
	}
	| WITH BOX NUMBER {
	    curbox = (int) $3;
	}
	| BOX onoff {
	    boxes[curbox].active = $2;
	}
	| BOX GRAPHNO {
	    boxes[curbox].gno = $2;
	}
	| BOX expr ',' expr ',' expr ',' expr {
	    if (curbox >= 0 && curbox < number_of_boxes()) {
		boxes[curbox].x1 = $2;
		boxes[curbox].y1 = $4;
		boxes[curbox].x2 = $6;
		boxes[curbox].y2 = $8;
	    }
	}
	| BOX LOCTYPE worldview {
	    box_loctype = $3;
	}
	| BOX lines_select {
	    box_lines = (int) $2;
	}
	| BOX linew_select {
	    box_linew = $2;
	}
	| BOX color_select {
	    box_color = (int) $2;
	}
	| BOX FILL color_select {
	    box_fillcolor = (int) $3;
	}
	| BOX FILL pattern_select {
	    box_fillpat = (int) $3;
	}
	| BOX DEF {
	    if (curbox >= 0 && curbox < number_of_boxes()) {
		boxes[curbox].lines = box_lines;
		boxes[curbox].linew = box_linew;
		boxes[curbox].color = box_color;
		if (get_project_version() <= 40102) {
                    switch (filltype_obs) {
                    case COLOR:
                        boxes[curbox].fillcolor = box_fillcolor;
		        boxes[curbox].fillpattern = 1;
                        break;
                    case PATTERN:
                        boxes[curbox].fillcolor = 1;
		        boxes[curbox].fillpattern = box_fillpat;
                        break;
                    default: /* NONE */
                        boxes[curbox].fillcolor = box_fillcolor;
		        boxes[curbox].fillpattern = 0;
                        break;
                    }
		} else {
                    boxes[curbox].fillcolor = box_fillcolor;
		    boxes[curbox].fillpattern = box_fillpat;
                }
                boxes[curbox].loctype = box_loctype;
	    }
	}

/* ellipses */
	| WITH ELLIPSE {
		curellipse = next_ellipse();
	}
	| WITH ELLIPSE NUMBER {
	    curellipse = (int) $3;
	}
	| ELLIPSE onoff {
	    ellip[curellipse].active = $2;
	}
	| ELLIPSE GRAPHNO {
	    ellip[curellipse].gno = $2;
	}
	| ELLIPSE expr ',' expr ',' expr ',' expr {
	    if (curellipse >= 0 && curellipse < number_of_ellipses()) {
		ellip[curellipse].x1 = $2;
		ellip[curellipse].y1 = $4;
		ellip[curellipse].x2 = $6;
		ellip[curellipse].y2 = $8;
	    }
	}
	| ELLIPSE LOCTYPE worldview {
	    ellipse_loctype = $3;
	}
	| ELLIPSE lines_select {
	    ellipse_lines = (int) $2;
	}
	| ELLIPSE linew_select {
	    ellipse_linew = $2;
	}
	| ELLIPSE color_select {
	    ellipse_color = (int) $2;
	}
	| ELLIPSE FILL color_select {
	    ellipse_fillcolor = (int) $3;
	}
	| ELLIPSE FILL pattern_select {
	    ellipse_fillpat = (int) $3;
	}
	| ELLIPSE DEF {
	    if (curellipse >= 0 && curellipse < number_of_ellipses()) {
		ellip[curellipse].lines = ellipse_lines;
		ellip[curellipse].linew = ellipse_linew;
		ellip[curellipse].color = ellipse_color;
		if (get_project_version() <= 40102) {
                    switch (filltype_obs) {
                    case COLOR:
                        ellip[curellipse].fillcolor = ellipse_fillcolor;
		        ellip[curellipse].fillpattern = 1;
                        break;
                    case PATTERN:
                        ellip[curellipse].fillcolor = 1;
		        ellip[curellipse].fillpattern = ellipse_fillpat;
                        break;
                    default: /* NONE */
                        ellip[curellipse].fillcolor = ellipse_fillcolor;
		        ellip[curellipse].fillpattern = 0;
                        break;
                    }
		} else {
                    ellip[curellipse].fillcolor = ellipse_fillcolor;
		    ellip[curellipse].fillpattern = ellipse_fillpat;
                }
		ellip[curellipse].loctype = ellipse_loctype;
	    }
	}

/* lines */
	| WITH LINE {
	    curline = next_line();
	}
	| WITH LINE NUMBER {
	    curline = (int) $3;
	}
	| LINE onoff {
	    lines[curline].active = $2;
	}
	| LINE GRAPHNO {
	    lines[curline].gno = $2;
	}
	| LINE expr ',' expr ',' expr ',' expr {
	    lines[curline].x1 = $2;
	    lines[curline].y1 = $4;
	    lines[curline].x2 = $6;
	    lines[curline].y2 = $8;
	}
	| LINE LOCTYPE worldview {
	    line_loctype = $3;
	}
	| LINE linew_select {
	    line_linew = $2;
	}
	| LINE lines_select {
	    line_lines = (int) $2;
	}
	| LINE color_select {
	    line_color = (int) $2;
	}
	| LINE ARROW NUMBER {
	    line_arrow_end = (int) $3;
	}
	| LINE ARROW LENGTH expr {
	    line_asize = $4;
	}
	| LINE ARROW TYPE NUMBER {
	    line_atype = (int) $4;
	}
	| LINE ARROW LAYOUT expr ',' expr {
	    line_a_dL_ff = $4;
	    line_a_lL_ff = $6;
	}
	| LINE DEF {
	    if (curline >= 0 && curline < number_of_lines()) {
		lines[curline].lines = line_lines;
		lines[curline].linew = line_linew;
		lines[curline].color = line_color;
		lines[curline].arrow_end = line_arrow_end;
		lines[curline].arrow.length = line_asize;
		lines[curline].arrow.type = line_atype;
		lines[curline].arrow.dL_ff = line_a_dL_ff;
		lines[curline].arrow.lL_ff = line_a_lL_ff;
		lines[curline].loctype = line_loctype;
	    }
	}

/* strings */
	| WITH STRING {
            curstring = next_string();
        }
	| WITH STRING NUMBER {
            curstring = (int) $3;
        }
	| STRING onoff {
            pstr[curstring].active = $2;
        }
	| STRING GRAPHNO {
            pstr[curstring].gno = $2;
        }
	| STRING expr ',' expr {
	    pstr[curstring].x = $2;
	    pstr[curstring].y = $4;
	}
	| STRING LOCTYPE worldview {
            string_loctype = $3;
        }
	| STRING color_select {
            string_color = (int) $2;
        }
	| STRING ROT NUMBER {
            string_rot = (int) $3;
        }
	| STRING font_select {
            string_font = (int) $2;
        }
	| STRING JUST NUMBER {
            string_just = (int) $3;
        }
	| STRING CHAR SIZE NUMBER {
            string_size = $4;
        }
	| STRING DEF CHRSTR {
	    set_plotstr_string(&pstr[curstring], (char *) $3);
	    pstr[curstring].color = string_color;
	    pstr[curstring].font = string_font;
	    pstr[curstring].just = string_just;
	    pstr[curstring].loctype = string_loctype;
	    pstr[curstring].rot = string_rot;
	    pstr[curstring].charsize = string_size;
	    free((char *) $3);
	}

/* timestamp */
	| TIMESTAMP onoff {
            timestamp.active = $2;
        }
	| TIMESTAMP font_select {
            timestamp.font = (int) $2;
        }
	| TIMESTAMP CHAR SIZE NUMBER {
            timestamp.charsize = $4;
        }
	| TIMESTAMP ROT NUMBER {
            timestamp.rot = (int) $3;
        }
	| TIMESTAMP color_select {
            timestamp.color = (int) $2;
        }
	| TIMESTAMP NUMBER ',' NUMBER {
	    timestamp.x = $2;
	    timestamp.y = $4;
	}
	| TIMESTAMP DEF CHRSTR {
	  set_plotstr_string(&timestamp, (char *) $3);
	  free((char *) $3);
	}

/* defaults */
	| DEFAULT lines_select {
	    grdefaults.lines = (int) $2;
	}
	| DEFAULT linew_select {
	    grdefaults.linew = $2;
	}
	| DEFAULT color_select {
	    grdefaults.color = (int) $2;
	}
	| DEFAULT pattern_select {
	    grdefaults.pattern = (int) $2;
	}
	| DEFAULT CHAR SIZE NUMBER {
	    grdefaults.charsize = $4;
	}
	| DEFAULT font_select {
	    grdefaults.font = (int) $2;
	}
	| DEFAULT SYMBOL SIZE NUMBER {
	    grdefaults.symsize = $4;
	}
	| DEFAULT SFORMAT CHRSTR {
	    strcpy(sformat, (char *) $3);
	    free((char *) $3);
	}
	| MAP FONTP NUMBER TO CHRSTR ',' CHRSTR {
	    if ((map_font_by_name((char *) $5, (int) $3) == GRACE_EXIT_SUCCESS) || 
                (map_font_by_name((char *) $7, (int) $3) == GRACE_EXIT_SUCCESS)) {
                ;
            } else {
                errmsg("Failed mapping a font");
                map_font(0, (int) $3);
            }
            free((char *) $5);
	    free((char *) $7);
	}
	| MAP COLOR NUMBER TO '(' NUMBER ',' NUMBER ',' NUMBER ')' ',' CHRSTR {
	    CMap_entry cmap;
            cmap.rgb.red   = $6;
            cmap.rgb.green = $8;
            cmap.rgb.blue  = $10;
            cmap.ctype = COLOR_MAIN;
            cmap.cname = (char *) $13;
            if (store_color((int) $3, cmap) == GRACE_EXIT_FAILURE) {
                errmsg("Failed mapping a color");
            }
	    free((char *) $13);
        }

	| WORLD expr ',' expr ',' expr ',' expr {
	    g[get_cg()].w.xg1 = $2;
	    g[get_cg()].w.yg1 = $4;
	    g[get_cg()].w.xg2 = $6;
	    g[get_cg()].w.yg2 = $8;
	}
	| WORLD XMIN expr {
	    g[get_cg()].w.xg1 = $3;
	}
	| WORLD XMAX expr {
	    g[get_cg()].w.xg2 = $3;
	}
	| WORLD YMIN expr {
	    g[get_cg()].w.yg1 = $3;
	}
	| WORLD YMAX expr {
	    g[get_cg()].w.yg2 = $3;
	}
	| VIEW expr ',' expr ',' expr ',' expr {
	    g[get_cg()].v.xv1 = $2;
	    g[get_cg()].v.yv1 = $4;
	    g[get_cg()].v.xv2 = $6;
	    g[get_cg()].v.yv2 = $8;
	}
	| VIEW XMIN expr {
	    g[get_cg()].v.xv1 = $3;
	}
	| VIEW XMAX expr {
	    g[get_cg()].v.xv2 = $3;
	}
	| VIEW YMIN expr {
	    g[get_cg()].v.yv1 = $3;
	}
	| VIEW YMAX expr {
	    g[get_cg()].v.yv2 = $3;
	}
	| TITLE CHRSTR {
	    set_plotstr_string(&g[get_cg()].labs.title, (char *) $2);
	    free((char *) $2);
	}
	| TITLE font_select {
	    g[get_cg()].labs.title.font = (int) $2;
	}
	| TITLE SIZE NUMBER {
	    g[get_cg()].labs.title.charsize = $3;
	}
	| TITLE color_select {
	    g[get_cg()].labs.title.color = (int) $2;
	}
	| SUBTITLE CHRSTR {
	    set_plotstr_string(&g[get_cg()].labs.stitle, (char *) $2);
	    free((char *) $2);
	}
	| SUBTITLE font_select {
	    g[get_cg()].labs.stitle.font = (int) $2;
	}
	| SUBTITLE SIZE NUMBER {
	    g[get_cg()].labs.stitle.charsize = $3;
	}
	| SUBTITLE color_select {
	    g[get_cg()].labs.stitle.color = (int) $2;
	}

	| DESCRIPTION CHRSTR {
            char *s;
            s = copy_string(NULL, get_project_description());
            s = concat_strings(s, (char *) $2);
	    free((char *) $2);
            s = concat_strings(s, "\n");
            set_project_description(s);
            free(s);
	}
        | CLEAR DESCRIPTION {
            set_project_description(NULL);
        }

	| LEGEND onoff {
	    g[get_cg()].l.active = $2;
	}
	| LEGEND LOCTYPE worldview {
	    g[get_cg()].l.loctype = $3;
	}
	| LEGEND VGAP NUMBER {
            g[get_cg()].l.vgap = (int) $3;
	}
	| LEGEND HGAP NUMBER {
	    g[get_cg()].l.hgap = (int) $3;
	}
	| LEGEND LENGTH NUMBER {
	    g[get_cg()].l.len = (int) $3;
	}
	| LEGEND INVERT onoff {
	    g[get_cg()].l.invert = (int) $3;
        }
	| LEGEND BOX FILL color_select {
	    g[get_cg()].l.boxfillpen.color = (int) $4;
        }
	| LEGEND BOX FILL pattern_select {
	    g[get_cg()].l.boxfillpen.pattern = (int) $4;
        }
	| LEGEND BOX color_select {
	    g[get_cg()].l.boxpen.color = (int) $3;
	}
	| LEGEND BOX pattern_select {
	    g[get_cg()].l.boxpen.pattern = (int) $3;
	}
	| LEGEND BOX lines_select {
	    g[get_cg()].l.boxlines = (int) $3;
	}
	| LEGEND BOX linew_select {
	    g[get_cg()].l.boxlinew = $3;
	}
	| LEGEND expr ',' expr {
	    g[get_cg()].l.legx = $2;
	    g[get_cg()].l.legy = $4;
	}
	| LEGEND X1 expr {
	    g[get_cg()].l.legx = $3;
	}
	| LEGEND Y1 expr {
	    g[get_cg()].l.legy = $3;
	}
	| LEGEND CHAR SIZE NUMBER {
	    g[get_cg()].l.charsize = $4;
	}
	| LEGEND font_select {
	    g[get_cg()].l.font = (int) $2;
	}
	| LEGEND color_select {
	    g[get_cg()].l.color = (int) $2;
	}
	| LEGEND STRING NUMBER CHRSTR {
	    strcpy(g[get_cg()].p[(int) $3].lstr, (char *) $4);
	    free((char *) $4);
	}

	| FRAMEP onoff {
            g[get_cg()].f.pen.pattern = $2;
	}
	| FRAMEP TYPE NUMBER {
	    g[get_cg()].f.type = (int) $3;
	}
	| FRAMEP lines_select {
	    g[get_cg()].f.lines = (int) $2;
	}
	| FRAMEP linew_select {
	    g[get_cg()].f.linew = $2;
	}
	| FRAMEP color_select {
	    g[get_cg()].f.pen.color = (int) $2;
	}
	| FRAMEP pattern_select {
	    g[get_cg()].f.pen.pattern = (int) $2;
	}
	| FRAMEP BACKGROUND color_select
        { 
            g[get_cg()].f.fillpen.color = (int) $3;
        }
	| FRAMEP BACKGROUND pattern_select
        {
            g[get_cg()].f.fillpen.pattern = (int) $3;
        }

	| GRAPHNO onoff {
            set_graph_active($1, $2);
        }
	| GRAPHNO HIDDEN onoff {
            set_graph_hidden($1, $3);
        }
	| GRAPHNO TYPE graphtype {
            set_graph_type($1, $3);
        }
	| GRAPHNO STACKED onoff {
            set_graph_stacked($1, $3);
        }

	| GRAPHNO BAR HGAP expr {
	    set_graph_bargap($1, $4);
	}
        
	| GRAPHNO FIXEDPOINT onoff {
            g[$1].locator.pointset = $3;
        }
	| GRAPHNO FIXEDPOINT FORMAT formatchoice formatchoice {
	    g[$1].locator.fx = $4;
	    g[$1].locator.fy = $5;
	}
	| GRAPHNO FIXEDPOINT PREC NUMBER ',' NUMBER {
	    g[$1].locator.px = $4;
	    g[$1].locator.py = $6;
	}
	| GRAPHNO FIXEDPOINT XY expr ',' expr {
	    g[$1].locator.dsx = $4;
	    g[$1].locator.dsy = $6;
	}
	| GRAPHNO FIXEDPOINT TYPE NUMBER {
            g[$1].locator.pt_type = (int) $4;
        }
        
	| TYPE xytype {
	    curtype = $2;
	    change_type = curtype;
	}


	| ALIAS CHRSTR CHRSTR {
	    int position;
	    lowtoupper((char *) $2);
	    lowtoupper((char *) $3);
	    if ((position = findf(key, (char *) $3)) >= 0) {
	        symtab_entry tmpkey;
		tmpkey.s = malloc(strlen((char *) $2) + 1);
		strcpy(tmpkey.s, (char *) $2);
		tmpkey.type = key[position].type;
		tmpkey.fnc = key[position].fnc;
		if (addto_symtab(tmpkey) != 0) {
		    yyerror("Keyword already exists");
		}
		free (tmpkey.s);
	    } else {
	        yyerror("Aliased keyword not found");
	    }
	    free((char *) $2);
	    free((char *) $3);
	}
	| ALIAS FORCE onoff {
	    alias_force = (int) $3;
	}
	| USE CHRSTR TYPE proctype FROM CHRSTR {
	    if (load_module((char *) $6, (char *) $2, (char *) $2, $4) != 0) {
	        yyerror("DL module load failed");
	    }
	    free((char *) $2);
	    free((char *) $6);
	}
	| USE CHRSTR TYPE proctype FROM CHRSTR ALIAS CHRSTR {
	    if (load_module((char *) $6, (char *) $2, (char *) $8, $4) != 0) {
	        yyerror("DL module load failed");
	    }
	    free((char *) $2);
	    free((char *) $6);
	    free((char *) $8);
	}

/* I/O filters */
	| DEFINE filtertype CHRSTR filtermethod CHRSTR {
	    if (add_io_filter((int) $2, (int) $4, (char *) $5, (char *) $3) != 0) {
	        yyerror("Failed adding i/o filter");
	    }
	    free((char *) $3);
	    free((char *) $5);
	}
	| CLEAR filtertype {
	    clear_io_filters((int) $2);
	}

	| SOURCE sourcetype {
	    cursource = $2;
	}
	| FORMAT formatchoice {
	    readxformat = $2;
	}
        | FIT nonlfitopts { }
	| FITPARM CONSTRAINTS onoff {
	    nonl_parms[(int) $1].constr = $3;
	}
	;

actions:
	REDRAW {
	    drawgraph();
	}
	| CD CHRSTR {
	    set_workingdir((char *) $2);
	    free((char *) $2);
	}
	| ECHO CHRSTR {
	    echomsg((char *) $2);
	    free((char *) $2);
	}
	| ECHO expr {
	    char buf[MAX_STRING_LENGTH];
            sprintf(buf, "%g", (double) $2);
            echomsg(buf);
	}
	| CLOSE {
	    close_input = copy_string(close_input, "");
	}
	| CLOSE CHRSTR {
	    close_input = copy_string(close_input, (char *) $2);
	}
	| EXIT {
	    exit(0);
	}
	| PRINT {
	    do_hardcopy();
	}
	| PRINT TO DEVICE {
            set_ptofile(FALSE);
	}
	| PRINT TO CHRSTR {
            set_ptofile(TRUE);
	    strcpy(print_file, (char *) $3);
            free((char *) $3);
	}
	| PAGE direction {
	    switch ((int) $2) {
	    case UP:
		graph_scroll(GSCROLL_UP);
		break;
	    case DOWN:
		graph_scroll(GSCROLL_DOWN);
		break;
	    case RIGHT:
		graph_scroll(GSCROLL_RIGHT);
		break;
	    case LEFT:
		graph_scroll(GSCROLL_LEFT);
		break;
	    case IN:
		graph_zoom(GZOOM_SHRINK);
		break;
	    case OUT:
		graph_zoom(GZOOM_EXPAND);
		break;
	    }
	}
	| SLEEP NUMBER {
	    if ($2 > 0) {
	        msleep_wrap((unsigned int) (1000 * $2));
	    }
	}
	| GETP CHRSTR {
	    gotparams = TRUE;
	    strcpy(paramfile, (char *) $2);
	    free((char *) $2);
	}
	| PUTP CHRSTR {
	    FILE *pp = grace_openw((char *) $2);
	    if (pp != NULL) {
	        putparms(get_cg(), pp, 0);
	        grace_close(pp);
	    }
	    free((char *) $2);
	}
	| SETNUM HIDDEN onoff {
	    set_set_hidden(get_cg(), $1, (int) $3);
	}
	| GRAPHNO '.' SETNUM HIDDEN onoff {
	    set_set_hidden($1, $3, (int) $5);
	}
	| SETNUM LENGTH NUMBER {
	    setlength(get_cg(), $1, (int) $3);
	}
	| GRAPHNO '.' SETNUM LENGTH NUMBER {
	    setlength($1, $3, (int) $5);
	}
	| SETNUM POINT expr ',' expr {
	    add_point(get_cg(), $1, $3, $5);
	}
	| GRAPHNO '.' SETNUM POINT expr ',' expr {
	    add_point($1, $3, $5, $7);
	}

	| SETNUM DROP NUMBER ',' NUMBER {
	    int start = (int) $3 - 1;
	    int stop = (int) $5 - 1;
	    int dist = stop - start + 1;
	    if (dist > 0 && start >= 0) {
	        droppoints(get_cg(), $1, start, stop, dist);
	    }
	}
	| GRAPHNO '.' SETNUM DROP NUMBER ',' NUMBER {
	    int start = (int) $5 - 1;
	    int stop = (int) $7 - 1;
	    int dist = stop - start + 1;
	    if (dist > 0 && start >= 0) {
	        droppoints($1, $3, start, stop, dist);
	    }
	}
	| SORT SETNUM sorton sortdir {
	    if (is_set_active(get_cg(), $2)) {
	        sortset(get_cg(), $2, $3, $4 == ASCENDING ? 0 : 1);
	    } else {
		errmsg("Set not active!");
	    }
	}
	| COPY SETNUM TO SETNUM {
	    do_copyset(get_cg(), $2, get_cg(), $4);
	}
	| COPY GRAPHNO '.' SETNUM TO GRAPHNO '.' SETNUM {
	    do_copyset($2, $4, $6, $8);
	}
	| MOVE SETNUM TO SETNUM {
	    do_moveset(get_cg(), $2, get_cg(), $4);
	}
	| MOVE GRAPHNO '.' SETNUM TO GRAPHNO '.' SETNUM {
	    do_moveset($2, $4, $6, $8);
	}
	| KILL SETNUM {
	    killset(get_cg(), $2);
	}
	| KILL SETS {
	    int i;
	    for (i = 0; i < g[get_cg()].maxplot; i++) {
		killset(get_cg(), i);
	    }
	}
	| KILL SETNUM SAVEALL {
            killsetdata(get_cg(), $2);
        }
	| KILL SETS SAVEALL {
	    int i;
            int cg = get_cg();
	    for (i = 0; i < number_of_sets(cg); i++) {
		killsetdata(cg, i);
	    }
	}
	| KILL GRAPHNO {
            kill_graph($2);
        }
	| KILL GRAPHS {
            kill_all_graphs();
        }
	| FLUSH {
            wipeout();
        }
	| ARRANGE NUMBER ',' NUMBER {
            arrange_graphs((int) $2, (int) $4);
        }
	| LOAD SCRARRAY NUMBER ',' expr ',' expr {
	    int i, ilen = (int) $3;
            double *ptr;
	    if (ilen < 0) {
		yyerror("Length of array < 0");
		return 1;
	    } else if (ilen > maxarr) {
		init_scratch_arrays(ilen);
	    }
            ptr = get_scratch((int) $2);
	    for (i = 0; i < ilen; i++) {
		ptr[i] = $5 + $7 * i;
	    }
	}
	| NONLFIT '(' SETNUM ',' NUMBER ')' {
	    gotnlfit = TRUE;
	    nlfit_gno = get_cg();
	    nlfit_setno = $3;
	    nlfit_nsteps = (int) $5;
	}
	| NONLFIT '(' GRAPHNO '.' SETNUM ',' NUMBER ')' {
	    gotnlfit = TRUE;
	    nlfit_gno = $3;
	    nlfit_setno = $5;
	    nlfit_nsteps = (int) $7;
	}
	| REGRESS '(' SETNUM ',' NUMBER ')' {
	    int setno = $3, ideg = (int) $5;
	    do_regress(setno, ideg, 0, -1, 0, -1);
	}
	| runtype '(' SETNUM ',' NUMBER ')' {
	    do_running_command($1, $3, (int) $5);
	}
	| ffttype '(' SETNUM ',' NUMBER ')' {
	    do_fourier_command($1, $3, (int) $5);
	}
        | ffttype '(' SETNUM ',' fourierdata ',' windowtype ',' 
          fourierloadx ','  fourierloady ')' {
	    switch ((int) $1) {
	    case FFT_DFT:
                do_fourier(0, $3, $11, $9, 0, $5, $7);
	        break;
	    case FFT_INVDFT:
                do_fourier(0, $3, $11, $9, 1, $5, $7);
	        break;
	    case FFT_FFT:
                do_fourier(1, $3, $11, $9, 0, $5, $7);
	        break;
	    case FFT_INVFFT:
                do_fourier(1, $3, $11, $9, 1, $5, $7);
	        break;
	    default:
                errmsg("Internal error");
	        break;
	    }
        }
	| SPLINE '(' SETNUM ',' expr ',' expr ',' NUMBER ')' {
	    do_spline($3, $5, $7, (int) $9, SPLINE_CUBIC);
	}
	| ASPLINE '(' SETNUM ',' expr ',' expr ',' NUMBER ')' {
	    do_spline($3, $5, $7, (int) $9, SPLINE_AKIMA);
	}
	| INTERP '(' SETNUM ',' SETNUM ',' NUMBER ')' {
	    do_interp($3, $5, (int) $7);
	}
	| HISTO '(' SETNUM ',' expr ',' expr ',' NUMBER ')' {
            do_histo(get_cg(), $3, SET_SELECT_NEXT, -1, $7, $5, $5 + ((int) $9)*$7, 
                                        HISTOGRAM_TYPE_ORDINARY);
	}
	| DIFFERENCE '(' SETNUM ',' NUMBER ')' {
	    do_differ($3, (int) $5);
	}
	| INTEGRATE '(' SETNUM ')' {
	    do_int($3, 0);
	}
 	| XCOR '(' SETNUM ',' SETNUM ',' NUMBER ')' {
	    do_xcor($3, $5, (int) $7);
	}
	| AUTOSCALE {
	    if (activeset(get_cg())) {
		autoscale_graph(get_cg(), AUTOSCALE_XY);
	    } else {
		errmsg("No active sets!");
	    }
	}
	| AUTOSCALE XAXES {
	    if (activeset(get_cg())) {
		autoscale_graph(get_cg(), AUTOSCALE_X);
	    } else {
		errmsg("No active sets!");
	    }
	}
	| AUTOSCALE YAXES {
	    if (activeset(get_cg())) {
                autoscale_graph(get_cg(), AUTOSCALE_Y);
	    } else {
		errmsg("No active sets!");
	    }
	}
	| AUTOSCALE SETNUM {
	    if (is_set_active(get_cg(), $2)) {
		autoscale_byset(get_cg(), $2, AUTOSCALE_XY);
	    } else {
		errmsg("Set not active");
	    }
	}
        | AUTOTICKS {
            autotick_axis(get_cg(), ALL_AXES);
        }
	| FOCUS GRAPHNO {
	    int gno = (int) $2;
            if (is_graph_hidden(gno) == FALSE) {
                select_graph(gno);
            } else {
		errmsg("Graph is not active");
            }
	}
	| READ CHRSTR {
	    gotread = TRUE;
	    readtype = curtype;
	    readsrc = cursource;
	    strcpy(readfile, (char *) $2);
	    free((char *) $2);
	}
	| READ BATCH CHRSTR {
	    gotbatch = TRUE;
	    strcpy(batchfile, (char *) $3);
	    free((char *) $3);
	}
	| READ BLOCK CHRSTR {
	    getdata(get_cg(), (char *) $3, SOURCE_DISK, SET_BLOCK);
	    free((char *) $3);
	}
	| READ BLOCK sourcetype CHRSTR {
	    getdata(get_cg(), (char *) $4, $3, SET_BLOCK);
	    free((char *) $4);
	}
	| BLOCK xytype CHRSTR {
	    create_set_fromblock(get_cg(), $2, (char *) $3);
	    free((char *) $3);
	}
	| READ xytype CHRSTR {
	    gotread = TRUE;
	    readtype = $2;
	    readsrc = cursource;
	    strcpy(readfile, (char *) $3);
	    free((char *) $3);
	}
	| READ xytype sourcetype CHRSTR {
	    gotread = TRUE;
	    strcpy(readfile, (char *) $4);
	    readtype = $2;
	    readsrc = $3;
	    free((char *) $4);
	}
	| WRITE SETNUM {
	    outputset(get_cg(), $2, (char *) NULL, (char *) NULL);
	}
	| WRITE SETNUM FORMAT CHRSTR {
	    outputset(get_cg(), $2, (char *) NULL, (char *) $4);
	    free((char *) $4);
	}
	| WRITE SETNUM FILEP CHRSTR {
	    outputset(get_cg(), $2, (char *) $4, (char *) NULL);
	    free((char *) $4);
	}
	| WRITE SETNUM FILEP CHRSTR FORMAT CHRSTR {
	    outputset(get_cg(), $2, (char *) $4, (char *) $6);
	    free((char *) $4);
	    free((char *) $6);
	}
        | SAVEALL CHRSTR {
            save_project((char *) $2);
            free((char *) $2);
        }
        | LOAD CHRSTR {
            load_project((char *) $2);
            free((char *) $2);
        }
        | NEW {
            new_project(NULL);
        }
        | NEW FROM CHRSTR {
            new_project((char *) $3);
            free((char *) $3);
        }
	| PUSH {
	    push_world();
	}
	| POP {
	    pop_world();
	}
	| CYCLE {
	    cycle_world_stack();
	}
	| STACK NUMBER {
	    if ((int) $2 > 0)
		show_world_stack((int) $2 - 1);
	}
	| CLEAR STACK {
	    clear_world_stack();
	}
	| CLEAR BOX {
	    do_clear_boxes();
	}
	| CLEAR ELLIPSE {
	    do_clear_ellipses();
	}
	| CLEAR LINE {
	    do_clear_lines();
	}
	| CLEAR STRING {
	    do_clear_text();
	}
        ;


options:
        PAGE LAYOUT pagelayout {
#ifndef NONE_GUI
            set_pagelayout($3);
#endif
        }
	| AUTO REDRAW onoff {
	    auto_redraw = $3;
	}
	| FOCUS onoff {
	    draw_focus_flag = $2;
	}
	| FOCUS SET {
	    focus_policy = FOCUS_SET;
	}
	| FOCUS FOLLOWS {
	    focus_policy = FOCUS_FOLLOWS;
	}
	| FOCUS CLICK {
	    focus_policy = FOCUS_CLICK;
	}
        ;


set_setprop:
	selectsets setprop {}
	| selectsets setprop_obs {}
	;

setprop:
	onoff {
	    set_set_hidden(whichgraph, whichset, !$1);
	}
	| TYPE xytype {
	    set_prop(whichgraph, SET, SETNUM, whichset, TYPE, $2, 0);
	}
	| PREC NUMBER {
	    set_prop(whichgraph, SET, SETNUM, whichset, PREC, (int) $2, 0);
	}
	| FORMAT formatchoice {
	    set_prop(whichgraph, SET, SETNUM, whichset, FORMAT, $2, 0);
	}

	| SYMBOL NUMBER {
            set_prop(whichgraph, SET, SETNUM, whichset, SYMBOL, TYPE, (int) $2, 0);
	}
	| SYMBOL color_select {
	    g[whichgraph].p[whichset].sympen.color = (int) $2;
	}
	| SYMBOL pattern_select {
	    g[whichgraph].p[whichset].sympen.pattern = (int) $2;
	}
	| SYMBOL linew_select {
	    set_prop(whichgraph, SET, SETNUM, whichset, SYMBOL, LINEWIDTH, $2, 0);
	}
	| SYMBOL lines_select {
	    set_prop(whichgraph, SET, SETNUM, whichset, SYMBOL, LINESTYLE, (int) $2, 0);
	}
	| SYMBOL FILL color_select {
	    g[whichgraph].p[whichset].symfillpen.color = (int) $3;
	}
	| SYMBOL FILL pattern_select {
	    g[whichgraph].p[whichset].symfillpen.pattern = (int) $3;
	}
	| SYMBOL SIZE expr {
	    set_prop(whichgraph, SET, SETNUM, whichset, SYMBOL, SIZE, $3, 0);
	}
	| SYMBOL CHAR NUMBER {
	    set_prop(whichgraph, SET, SETNUM, whichset, SYMBOL, CHAR, (int) $3, 0);
	}
	| SYMBOL CHAR font_select {
	    g[whichgraph].p[whichset].charfont = (int) $3;
	}
	| SYMBOL SKIP NUMBER {
	    set_prop(whichgraph, SET, SETNUM, whichset, SYMBOL, SKIP, (int) $3, 0);
	}

	| LINE TYPE NUMBER
        {
	    g[whichgraph].p[whichset].linet = (int) $3;
	}
	| LINE lines_select
        {
	    g[whichgraph].p[whichset].lines = (int) $2;
	}
	| LINE linew_select
        {
	    g[whichgraph].p[whichset].linew = $2;
	}
	| LINE color_select
        {
	    g[whichgraph].p[whichset].linepen.color = (int) $2;
	}
	| LINE pattern_select
        {
	    g[whichgraph].p[whichset].linepen.pattern = (int) $2;
	}

	| FILL NUMBER
        {
	    set_prop(whichgraph, SET, SETNUM, whichset, FILL, TYPE, (int) $2, 0);
	}
	| FILL TYPE NUMBER
        {
	    g[whichgraph].p[whichset].filltype = (int) $3;
	}
	| FILL RULE NUMBER
        {
	    g[whichgraph].p[whichset].fillrule = (int) $3;
	}
	| FILL color_select
        {
	    int prop = (int) $2;

	    if (get_project_version() <= 40102 && get_project_version() >= 30000) {
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
	    g[whichgraph].p[whichset].setfillpen.color = prop;
	}
	| FILL pattern_select
        {
	    int prop = (int) $2;

	    if (get_project_version() <= 40102) {
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
	    g[whichgraph].p[whichset].setfillpen.pattern = prop;
	}

	| SKIP NUMBER
        {
	    set_prop(whichgraph, SET, SETNUM, whichset, SKIP, (int) $2, 0);
	}
        
	| BASELINE onoff
        {
	    g[whichgraph].p[whichset].baseline = (int) $2;
	}
	| BASELINE TYPE NUMBER
        {
	    g[whichgraph].p[whichset].baseline_type = (int) $3;
	}
        
	| DROPLINE onoff
        {
	    g[whichgraph].p[whichset].dropline = (int) $2;
	}

	| AVALUE onoff
        {
	    g[whichgraph].p[whichset].avalue.active = (int) $2;
	}
	| AVALUE TYPE NUMBER
        {
	    g[whichgraph].p[whichset].avalue.type = (int) $3;
	}
	| AVALUE CHAR SIZE NUMBER
        {
	    g[whichgraph].p[whichset].avalue.size = $4;
	}
	| AVALUE font_select
        {
	    g[whichgraph].p[whichset].avalue.font = (int) $2;
	}
	| AVALUE color_select
        {
	    g[whichgraph].p[whichset].avalue.color = (int) $2;
	}
	| AVALUE ROT NUMBER
        {
	    g[whichgraph].p[whichset].avalue.angle = (int) $3;
	}
	| AVALUE FORMAT formatchoice
        {
	    g[whichgraph].p[whichset].avalue.format = (int) $3;
	}
	| AVALUE PREC NUMBER
        {
	    g[whichgraph].p[whichset].avalue.prec = (int) $3;
	}
	| AVALUE OFFSET expr ',' expr {
	    g[whichgraph].p[whichset].avalue.offset.x = $3;
	    g[whichgraph].p[whichset].avalue.offset.y = $5;
	}
	| AVALUE PREPEND CHRSTR
        {
	    strcpy(g[whichgraph].p[whichset].avalue.prestr, (char *) $3);
	    free((char *) $3);
	}
	| AVALUE APPEND CHRSTR
        {
	    strcpy(g[whichgraph].p[whichset].avalue.appstr, (char *) $3);
	    free((char *) $3);
	}

	| ERRORBAR onoff {
	    g[whichgraph].p[whichset].errbar.active = (int) $2;
	}
	| ERRORBAR LENGTH NUMBER {
            g[whichgraph].p[whichset].errbar.length = $3;
	}
	| ERRORBAR TYPE opchoice {
	    set_prop(whichgraph, SET, SETNUM, whichset, ERRORBAR, TYPE, $3, 0);
	}
	| ERRORBAR linew_select {
	    set_prop(whichgraph, SET, SETNUM, whichset, ERRORBAR, LINEWIDTH, $2, 0);
	}
	| ERRORBAR lines_select {
	    set_prop(whichgraph, SET, SETNUM, whichset, ERRORBAR, LINESTYLE, (int) $2, 0);
	}
	| ERRORBAR RISER onoff {
	    set_prop(whichgraph, SET, SETNUM, whichset, ERRORBAR, RISER, ON, $3, 0);
	}
	| ERRORBAR RISER linew_select {
	    set_prop(whichgraph, SET, SETNUM, whichset, ERRORBAR, RISER, LINEWIDTH, $3, 0);
	}
	| ERRORBAR RISER lines_select {
	    set_prop(whichgraph, SET, SETNUM, whichset, ERRORBAR, RISER, LINESTYLE, (int) $3, 0);
	}

	| COMMENT CHRSTR {
	    strncpy(g[whichgraph].p[whichset].comments, (char *) $2, MAX_STRING_LENGTH - 1);
	    free((char *) $2);
	}
        
	| LEGEND CHRSTR {
	    strncpy(g[whichgraph].p[whichset].lstr, (char *) $2, MAX_STRING_LENGTH - 1);
	    free((char *) $2);
	}
	;

axesprops:
	SCALE scaletype {
	    switch (naxis) {
	    case ALL_AXES:
		g[whichgraph].xscale = (int) $2;
		g[whichgraph].yscale = (int) $2;
		break;
	    case ALL_X_AXES:
		g[whichgraph].xscale = (int) $2;
		break;
	    case ALL_Y_AXES:
		g[whichgraph].yscale = (int) $2;
		break;
	    }
	}
	| INVERT onoff {
	    switch (naxis) {
	    case ALL_AXES:
		g[whichgraph].xinvert = (int) $2;
		g[whichgraph].yinvert = (int) $2;
		break;
	    case ALL_X_AXES:
		g[whichgraph].xinvert = (int) $2;
		break;
	    case ALL_Y_AXES:
		g[whichgraph].yinvert = (int) $2;
		break;
	    }
	}
	;

axisfeature:
	onoff {
	    g[get_cg()].t[naxis].active = (int) $1;
	}
	| TYPE ZERO onoff {
	    g[get_cg()].t[naxis].zero = (int) $3;
	}
	| TICKP tickattr {}
	| TICKP tickattr_obs {}
	| TICKLABEL ticklabelattr {}
	| TICKLABEL ticklabelattr_obs {}
	| LABEL axislabeldesc {}
	| LABEL axislabeldesc_obs {}
	| BAR axisbardesc {}
	| OFFSET expr ',' expr {
            g[get_cg()].t[naxis].offsx = $2;
	    g[get_cg()].t[naxis].offsy = $4;
	}
	;

tickattr:
	onoff {
	    g[get_cg()].t[naxis].t_flag = (int) $1;
	}
	| MAJOR expr {
            g[get_cg()].t[naxis].tmajor = $2;
	}
	| MINOR TICKSP NUMBER {
	    g[get_cg()].t[naxis].nminor = (int) $3;
	}
	| PLACE ROUNDED onoff {
	    g[get_cg()].t[naxis].t_round = (int) $3;
	}

	| OFFSETX expr {
            g[get_cg()].t[naxis].offsx = $2;
	}
	| OFFSETY expr {
            g[get_cg()].t[naxis].offsy = $2;
	}
	| DEFAULT NUMBER {
	    g[get_cg()].t[naxis].t_autonum = (int) $2;
	}
	| inoutchoice {
	    g[get_cg()].t[naxis].t_inout = $1;
	}
	| MAJOR SIZE NUMBER {
	    g[get_cg()].t[naxis].props.size = $3;
	}
	| MINOR SIZE NUMBER {
	    g[get_cg()].t[naxis].mprops.size = $3;
	}
	| color_select {
	    g[get_cg()].t[naxis].props.color = g[get_cg()].t[naxis].mprops.color = (int) $1;
	}
	| MAJOR color_select {
	    g[get_cg()].t[naxis].props.color = (int) $2;
	}
	| MINOR color_select {
	    g[get_cg()].t[naxis].mprops.color = (int) $2;
	}
	| linew_select {
	    g[get_cg()].t[naxis].props.linew = g[get_cg()].t[naxis].mprops.linew = $1;
	}
	| MAJOR linew_select {
	    g[get_cg()].t[naxis].props.linew = $2;
	}
	| MINOR linew_select {
	    g[get_cg()].t[naxis].mprops.linew = $2;
	}
	| MAJOR lines_select {
	    g[get_cg()].t[naxis].props.lines = (int) $2;
	}
	| MINOR lines_select {
	    g[get_cg()].t[naxis].mprops.lines = (int) $2;
	}
	| MAJOR GRID onoff {
	    g[get_cg()].t[naxis].props.gridflag = $3;
	}
	| MINOR GRID onoff {
	    g[get_cg()].t[naxis].mprops.gridflag = $3;
	}
	| OP opchoice {
	    g[get_cg()].t[naxis].t_op = $2;
	}
	| TYPE AUTO {
	    g[get_cg()].t[naxis].t_type = TYPE_AUTO;
	}
	| TYPE SPEC {
	    g[get_cg()].t[naxis].t_type = TYPE_SPEC;
	}
	| SPEC NUMBER {
	    g[get_cg()].t[naxis].nticks = (int) $2;
	}
	| MAJOR NUMBER ',' expr {
	    g[get_cg()].t[naxis].tloc[(int) $2].wtpos = $4;
	    g[get_cg()].t[naxis].tloc[(int) $2].type = TICK_TYPE_MAJOR;
	}
	| MINOR NUMBER ',' expr {
	    g[get_cg()].t[naxis].tloc[(int) $2].wtpos = $4;
	    g[get_cg()].t[naxis].tloc[(int) $2].type = TICK_TYPE_MINOR;
	}
	;

ticklabelattr:
	onoff {
	    g[get_cg()].t[naxis].tl_flag = $1;
	}
	| TYPE AUTO {
	    g[get_cg()].t[naxis].tl_type = TYPE_AUTO;
	}
	| TYPE SPEC {
	    g[get_cg()].t[naxis].tl_type = TYPE_SPEC;
	}
	| PREC NUMBER {
	    g[get_cg()].t[naxis].tl_prec = (int) $2;
	}
	| FORMAT formatchoice {
	    g[get_cg()].t[naxis].tl_format = $2;
	}
	| FORMAT NUMBER {
	    g[get_cg()].t[naxis].tl_format = $2;
	}
	| APPEND CHRSTR {
	    strcpy(g[get_cg()].t[naxis].tl_appstr, (char *) $2);
	    free((char *) $2);
	}
	| PREPEND CHRSTR {
	    strcpy(g[get_cg()].t[naxis].tl_prestr, (char *) $2);
	    free((char *) $2);
	}
	| ANGLE NUMBER {
	    g[get_cg()].t[naxis].tl_angle = (int) $2;
	}
	| SKIP NUMBER {
	    g[get_cg()].t[naxis].tl_skip = (int) $2;
	}
	| STAGGER NUMBER {
	    g[get_cg()].t[naxis].tl_staggered = (int) $2;
	}
	| OP opchoice {
	    g[get_cg()].t[naxis].tl_op = $2;
	}
	| SIGN signchoice {
	    g[get_cg()].t[naxis].tl_sign = $2;
	}
	| START expr {
	    g[get_cg()].t[naxis].tl_start = $2;
	}
	| STOP expr {
	    g[get_cg()].t[naxis].tl_stop = $2;
	}
	| START TYPE SPEC {
	    g[get_cg()].t[naxis].tl_starttype = TYPE_SPEC;
	}
	| START TYPE AUTO {
	    g[get_cg()].t[naxis].tl_starttype = TYPE_AUTO;
	}
	| STOP TYPE SPEC {
	    g[get_cg()].t[naxis].tl_stoptype = TYPE_SPEC;
	}
	| STOP TYPE AUTO {
	    g[get_cg()].t[naxis].tl_stoptype = TYPE_AUTO;
	}
	| CHAR SIZE NUMBER {
	    g[get_cg()].t[naxis].tl_charsize = $3;
	}
	| font_select {
	    g[get_cg()].t[naxis].tl_font = (int) $1;
	}
	| color_select {
	    g[get_cg()].t[naxis].tl_color = (int) $1;
	}
	| NUMBER ',' CHRSTR {
	    g[get_cg()].t[naxis].tloc[(int) $1].label = 
                copy_string(g[get_cg()].t[naxis].tloc[(int) $1].label, (char *) $3);
	    free((char *) $3);
	}
	| OFFSET AUTO {
	    g[get_cg()].t[naxis].tl_gaptype = TYPE_AUTO;
	}
	| OFFSET SPEC {
	    g[get_cg()].t[naxis].tl_gaptype = TYPE_SPEC;
	}
	| OFFSET expr ',' expr {
	    g[get_cg()].t[naxis].tl_gap.x = $2;
	    g[get_cg()].t[naxis].tl_gap.y = $4;
	}
	;

axislabeldesc:
	CHRSTR {
	    set_plotstr_string(&g[get_cg()].t[naxis].label, (char *) $1);
	    free((char *) $1);
	}
	| LAYOUT PERP {
	    g[get_cg()].t[naxis].label_layout = LAYOUT_PERPENDICULAR;
	}
	| LAYOUT PARA {
	    g[get_cg()].t[naxis].label_layout = LAYOUT_PARALLEL;
	}
	| PLACE AUTO {
	    g[get_cg()].t[naxis].label_place = TYPE_AUTO;
	}
	| PLACE SPEC {
	    g[get_cg()].t[naxis].label_place = TYPE_SPEC;
	}
	| PLACE expr ',' expr {
	    g[get_cg()].t[naxis].label.x = $2;
	    g[get_cg()].t[naxis].label.y = $4;
	}
	| JUST justchoice {
	    g[get_cg()].t[naxis].label.just = (int) $2;
	}
	| CHAR SIZE NUMBER {
	    g[get_cg()].t[naxis].label.charsize = $3;
	}
	| font_select {
	    g[get_cg()].t[naxis].label.font = (int) $1;
	}
	| color_select {
	    g[get_cg()].t[naxis].label.color = (int) $1;
	}
	| OP opchoice {
	    g[get_cg()].t[naxis].label_op = $2;
	}
	;

axisbardesc:
	onoff {
	    g[get_cg()].t[naxis].t_drawbar = $1;
	}
	| color_select {
	    g[get_cg()].t[naxis].t_drawbarcolor = (int) $1;
	}
	| lines_select {
	    g[get_cg()].t[naxis].t_drawbarlines = (int) $1;
	}
	| linew_select {
	    g[get_cg()].t[naxis].t_drawbarlinew = $1;
	}
	;

nonlfitopts:
        TITLE CHRSTR { 
          strcpy(nonl_opts.title, (char *) $2);
	  free((char *) $2);
        }
        | FORMULA CHRSTR { 
          strcpy(nonl_opts.formula, (char *) $2);
	  free((char *) $2);
        }
        | WITH NUMBER PARAMETERS { 
            nonl_opts.parnum = (int) $2; 
        }
        | PREC NUMBER { 
            nonl_opts.tolerance = $2; 
        }
        ;

selectsets:
	GRAPHNO '.' SETNUM
	{
	    whichgraph = $1;
	    whichset = $3;
	}
	| SETNUM
	{
	    whichgraph = get_cg();
	    whichset = $1;
	}
	|  SETS
	{
	    whichgraph = get_cg();
	    whichset = $1;
	}
	| GRAPHNO SETS
	{
	    whichgraph = $1;
	    whichset = $2;
	}
	|  GRAPHS SETS
	{
	    whichgraph = $1;
	    whichset = $2;
	}
	|  GRAPHS SETNUM
	{
	    whichgraph = $1;
	    whichset = $2;
	}
	;

setaxis:
	axis axisfeature {}
	| GRAPHNO axis axisfeature {}
	| allaxes axesprops {}
	| GRAPHNO allaxes axesprops {}
	;

axis:
	XAXIS {}
	| YAXIS {}
	| ALTXAXIS {}
	| ALTYAXIS {}
	;

allaxes:
        AXES {}
        | XAXES {}
	| YAXES {}
	;

proctype:
        PROC_CONST        { $$ = CONSTANT; }
        | PROC_UNIT      { $$ = UCONSTANT; }
        | PROC_FUNC_I       { $$ = FUNC_I; }
	| PROC_FUNC_D       { $$ = FUNC_D; }
	| PROC_FUNC_ND     { $$ = FUNC_ND; }
	| PROC_FUNC_NN     { $$ = FUNC_NN; }
	| PROC_FUNC_DD     { $$ = FUNC_DD; }
	| PROC_FUNC_NND   { $$ = FUNC_NND; }
	| PROC_FUNC_PPD   { $$ = FUNC_PPD; }
	| PROC_FUNC_PPPD { $$ = FUNC_PPPD; }
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
	| XYHILO { $$ = SET_XYHILO; }
	| XYR { $$ = SET_XYR; }
	| XYSTRING { $$ = SET_XYSTRING; }
	| NXY { $$ = SET_NXY; }
	;

graphtype:
	XY { $$ = GRAPH_XY; }
	| CHART { $$ = GRAPH_CHART; }
	| POLAR { $$ = GRAPH_POLAR; }
	| SMITH { $$ = GRAPH_SMITH; }
	| FIXED { $$ = GRAPH_FIXED; }
	;
        
pagelayout:
        FREE { $$ = PAGE_FREE; }
        | FIXED { $$ = PAGE_FIXED; }
        ;

pageorient:
        LANDSCAPE  { $$ = PAGE_ORIENT_LANDSCAPE; }
        | PORTRAIT { $$ = PAGE_ORIENT_PORTRAIT;  }
        ;

regiontype:
	ABOVE { $$ = REGION_ABOVE; }
	|  BELOW { $$ = REGION_BELOW; }
	|  LEFT { $$ = REGION_TOLEFT; }
	|  RIGHT { $$ = REGION_TORIGHT; }
	|  POLYI { $$ = REGION_POLYI; }
	|  POLYO { $$ = REGION_POLYO; }
	|  HORIZI { $$ = REGION_HORIZI; }
	|  VERTI { $$ = REGION_VERTI; }
	|  HORIZO { $$ = REGION_HORIZO; }
	|  VERTO { $$ = REGION_VERTO; }
	;

scaletype: NORMAL { $$ = SCALE_NORMAL; }
	| LOGARITHMIC { $$ = SCALE_LOG; }
	| RECIPROCAL { $$ = SCALE_REC; }
	;

onoff: ON { $$ = TRUE; }
	| OFF { $$ = FALSE; }
	;

colpat_obs: NONE
	| COLOR
	| PATTERN
	;

runtype: RUNAVG { $$ = RUN_AVG; }
	| RUNSTD { $$ = RUN_STD; }
	| RUNMED { $$ = RUN_MED; }
	| RUNMAX { $$ = RUN_MAX; }
	| RUNMIN { $$ = RUN_MIN; }
	;

sourcetype: 
        DISK 
        {
            $$ = SOURCE_DISK;
        }
	| PIPE
        {
            $$ = SOURCE_PIPE;
        }
	;

opchoice: TOP { $$ = PLACE_TOP; }
	| BOTTOM { $$ = PLACE_BOTTOM; }
	| LEFT { $$ = PLACE_LEFT; }
	| RIGHT { $$ = PLACE_RIGHT; }
	| BOTH { $$ = PLACE_BOTH; }
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

direction: UP { $$ = UP; }
	| DOWN { $$ = DOWN; }
	| RIGHT { $$ = RIGHT; }
	| LEFT { $$ = LEFT; }
	| IN { $$ = IN; }
	| OUT { $$ = OUT; }
	;

worldview: WORLD { $$ = COORD_WORLD; }
	| VIEW { $$ = COORD_VIEW; }
	;

vector: X_TOK { $$ = DATA_X; }
	| Y_TOK { $$ = DATA_Y; }
	| X0 { $$ = DATA_X; }
	| Y0 { $$ = DATA_Y; }
	| Y1 { $$ = DATA_Y1; }
	| Y2 { $$ = DATA_Y2; }
	| Y3 { $$ = DATA_Y3; }
	| Y4 { $$ = DATA_Y4; }
	;

sortdir: ASCENDING { $$ = ASCENDING; }
	| DESCENDING { $$ = DESCENDING; }
	;

sorton: X_TOK { $$ = DATA_X; }
	| Y_TOK { $$ = DATA_Y; }
	;

ffttype: DFT { $$ = FFT_DFT; }
	| FFT { $$ = FFT_FFT; }
	| INVDFT { $$ = FFT_INVDFT; }
	| INVFFT { $$ = FFT_INVFFT; }
	;

fourierdata:
	REAL {$$=0;}
	| COMPLEX {$$=1;}
	;

fourierloadx:
	INDEX {$$=0;}
	| FREQUENCY {$$=1;}
	| PERIOD {$$=2;}
	;

fourierloady:
	MAGNITUDE {$$=0;}
	| PHASE {$$=1;}
	| COEFFICIENTS {$$=2;}
	;

windowtype:
	NONE {$$=0;}
	| TRIANGULAR {$$=1;}
	| HANNING {$$=2;}
	| WELCH {$$=3;}
	| HAMMING {$$=4;}
	| BLACKMAN {$$=5;}
	| PARZEN {$$=6;}
	;
	
extremetype: MINP { $$ = MINP; }
	| MAXP { $$ = MAXP; }
        | AVG { $$ = AVG; }
	| SD { $$ = SD; }
	;

font_select:
        FONTP NUMBER
        {
            $$ = get_mapped_font((int) $2);
        }
        | FONTP CHRSTR
        {
            $$ = get_font_by_name((char *) $2);
            free((char *) $2);
        }
        ;

lines_select:
        LINESTYLE NUMBER
        {
	    int lines = (int) $2;
            if (lines >= 0 && lines < number_of_linestyles()) {
	        $$ = lines;
	    } else {
	        errmsg("invalid linestyle");
	        $$ = 1;
	    }
        }
        ;

pattern_select:
        PATTERN NUMBER
        {
	    int patno = (int) $2;
            if (patno >= 0 && patno < number_of_patterns()) {
	        $$ = patno;
	    } else {
	        errmsg("invalid pattern number");
	        $$ = 1;
	    }
        }
        ;

color_select:
        COLOR NUMBER
        {
            int c = (int) $2;
            if (c >= 0 && c < number_of_colors()) {
                $$ = c;
            } else {
                errmsg("Invalid color ID");
                $$ = 1;
            }
        }
        | COLOR CHRSTR
        {
            int c = get_color_by_name((char *) $2);
            if (c == BAD_COLOR) {
                errmsg("Invalid color name");
                c = 1;
            }
            free((char *) $2);
            $$ = c;
        }
        | COLOR '(' NUMBER ',' NUMBER ',' NUMBER ')'
        {
            int c;
            CMap_entry cmap;
            cmap.rgb.red = (int) $3;
            cmap.rgb.green = (int) $5;
            cmap.rgb.blue = (int) $7;
            cmap.ctype = COLOR_MAIN;
            cmap.cname = NULL;
            c = add_color(cmap);
            if (c == BAD_COLOR) {
                errmsg("Can't allocate requested color");
                c = 1;
            }
            $$ = c;
        }
        ;

linew_select:
        LINEWIDTH NUMBER
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

parmset_obs:
        PAGE LAYOUT pageorient
        {
            Page_geometry pg;
            if ($3 == PAGE_ORIENT_LANDSCAPE) {
                pg.width =  792;
                pg.height = 612;
            } else {
                pg.width =  612;
                pg.height = 792;
            }
            pg.dpi_x = 72.0;
            pg.dpi_y = 72.0;
            set_page_geometry(pg);
        }
	| PAGE NUMBER {
	    scroll_proc((int) $2);
	}
	| PAGE INOUT NUMBER {
	    scrollinout_proc((int) $3);
	}

	| DEFAULT FONTP SOURCE NUMBER {
	}

	| STACK WORLD expr ',' expr ',' expr ',' expr TICKP expr ',' expr ',' expr ',' expr
	{
	    add_world(get_cg(), $3, $5, $7, $9);
	}

	| BOX FILL colpat_obs {filltype_obs = (int) $3;}

	| ELLIPSE FILL colpat_obs {filltype_obs = (int) $3;}

	| STRING linew_select { }

	| TIMESTAMP linew_select { }

	| TITLE linew_select { }
	| SUBTITLE linew_select { }

	| LEGEND BOX onoff {
	    if ($3 == FALSE && get_project_version() <= 40102) {
                g[get_cg()].l.boxpen.pattern = 0;
            }
	}
	| LEGEND BOX FILL onoff { }
	| LEGEND BOX FILL WITH colpat_obs {filltype_obs = (int) $5;}
	| LEGEND lines_select { }
	| LEGEND linew_select { }

	| GRAPHNO LABEL onoff { }

	| GRAPHNO TYPE LOGX { 
	    g[$1].type = GRAPH_XY;
	    g[$1].xscale = SCALE_LOG;
	}
	| GRAPHNO TYPE LOGY { 
	    g[$1].type = GRAPH_XY;
	    g[$1].yscale = SCALE_LOG;
	}
	| GRAPHNO TYPE LOGXY
	{ 
	    g[$1].type = GRAPH_XY;
	    g[$1].xscale = SCALE_LOG;
	    g[$1].yscale = SCALE_LOG;
	}
	| GRAPHNO TYPE BAR
	{ 
	    g[$1].type = GRAPH_CHART;
	    g[$1].xyflip = FALSE;
	    g[$1].stacked = FALSE;
	}
	| GRAPHNO TYPE HBAR
	{ 
	    g[$1].type = GRAPH_CHART;
	    g[$1].xyflip = TRUE;
	}
	| GRAPHNO TYPE STACKEDBAR
	{ 
	    g[$1].type = GRAPH_CHART;
	    g[$1].stacked = TRUE;
	}
	| GRAPHNO TYPE STACKEDHBAR
	{ 
	    g[$1].type = GRAPH_CHART;
	    g[$1].stacked = TRUE;
	    g[$1].xyflip = TRUE;
	}

	| LEGEND LAYOUT NUMBER {
	}

	| FRAMEP FILL onoff { 
            g[get_cg()].f.fillpen.pattern = $3;
        }

	| GRAPHNO AUTOSCALE TYPE AUTO {
        }
	| GRAPHNO AUTOSCALE TYPE SPEC {
        }

	| LINE ARROW SIZE NUMBER {
	    line_asize = 2.0*$4;
	}

        | HARDCOPY DEVICE NUMBER { }
        | PS LINEWIDTH BEGIN NUMBER { }
        | PS LINEWIDTH INCREMENT NUMBER { }
        | PS linew_select { }
        ;


axislabeldesc_obs:
	linew_select { }
        ;

setprop_obs:
	SYMBOL FILL NUMBER {
	    set_prop(whichgraph, SET, SETNUM, whichset, SYMBOL, FILL, (int) $3, 0);
	}
	| SYMBOL COLOR '-' NUMBER {
	    g[whichgraph].p[whichset].sympen.color = -1;
	}
	| SYMBOL CENTER onoff {
	}
	| lines_select {
	    g[whichgraph].p[whichset].lines = (int) $1;
	}
	| linew_select {
	    g[whichgraph].p[whichset].linew = $1;
	}
	| color_select {
	    g[whichgraph].p[whichset].linepen.color = (int) $1;
	}
	| FILL WITH colpat_obs {filltype_obs = (int) $3;}
	|  XYZ expr ',' expr { }
        ;
        

tickattr_obs:
	MAJOR onoff {
	    /* <= xmgr-4.1 */
	    g[get_cg()].t[naxis].active = (int) $2;
	}
	| MINOR onoff { }
	| ALT onoff   { }
	| MINP expr   { }
	| MAXP expr   { }
	| LOG onoff   { }
	| MINOR expr {
	    if ($2 != 0.0) {
                g[get_cg()].t[naxis].nminor = 
                            (int) rint(g[get_cg()].t[naxis].tmajor / $2 - 1);
            } else {
                g[get_cg()].t[naxis].nminor = 0;
            }
	}
	| SIZE NUMBER {
	    g[get_cg()].t[naxis].props.size = $2;
	}
	| NUMBER ',' expr {
	    g[get_cg()].t[naxis].tloc[(int) $1].wtpos = $3;
	    g[get_cg()].t[naxis].tloc[(int) $1].type = TICK_TYPE_MAJOR;
	}
        ;

ticklabelattr_obs:
	linew_select { }
	| LAYOUT SPEC { }

	| LAYOUT HORIZONTAL {
	    g[get_cg()].t[naxis].tl_angle = 0;
	}
	| LAYOUT VERTICAL {
	    g[get_cg()].t[naxis].tl_angle = 90;
	}
	| PLACE ON TICKSP {
	}
	| PLACE BETWEEN TICKSP {
	}
        ;

%%

/* list of intrinsic functions and keywords */
symtab_entry ikey[] = {
	{"A", SCRARRAY, NULL},
	{"A0", FITPARM, NULL},
	{"A0MAX", FITPMAX, NULL},
	{"A0MIN", FITPMIN, NULL},
	{"A1", FITPARM, NULL},
	{"A1MAX", FITPMAX, NULL},
	{"A1MIN", FITPMIN, NULL},
	{"A2", FITPARM, NULL},
	{"A2MAX", FITPMAX, NULL},
	{"A2MIN", FITPMIN, NULL},
	{"A3", FITPARM, NULL},
	{"A3MAX", FITPMAX, NULL},
	{"A3MIN", FITPMIN, NULL},
	{"A4", FITPARM, NULL},
	{"A4MAX", FITPMAX, NULL},
	{"A4MIN", FITPMIN, NULL},
	{"A5", FITPARM, NULL},
	{"A5MAX", FITPMAX, NULL},
	{"A5MIN", FITPMIN, NULL},
	{"A6", FITPARM, NULL},
	{"A6MAX", FITPMAX, NULL},
	{"A6MIN", FITPMIN, NULL},
	{"A7", FITPARM, NULL},
	{"A7MAX", FITPMAX, NULL},
	{"A7MIN", FITPMIN, NULL},
	{"A8", FITPARM, NULL},
	{"A8MAX", FITPMAX, NULL},
	{"A8MIN", FITPMIN, NULL},
	{"A9", FITPARM, NULL},
	{"A9MAX", FITPMAX, NULL},
	{"A9MIN", FITPMIN, NULL},
	{"ABOVE", ABOVE, NULL},
	{"ABS", FUNC_D, fabs},
	{"ABSOLUTE", ABSOLUTE, NULL},
	{"ACOS", FUNC_D, acos},
	{"ACOSH", FUNC_D, acosh},
	{"AI", FUNC_D, ai_wrap},
	{"ALIAS", ALIAS, NULL},
	{"ALT", ALT, NULL},
	{"ALTXAXIS", ALTXAXIS, NULL},
	{"ALTYAXIS", ALTYAXIS, NULL},
	{"AND", AND, NULL},
	{"ANGLE", ANGLE, NULL},
	{"ANTIALIASING", ANTIALIASING, NULL},
	{"APPEND", APPEND, NULL},
	{"ARRANGE", ARRANGE, NULL},
	{"ARROW", ARROW, NULL},
	{"ASCENDING", ASCENDING, NULL},
	{"ASIN", FUNC_D, asin},
	{"ASINH", FUNC_D, asinh},
	{"ASPLINE", ASPLINE, NULL},
	{"ATAN", FUNC_D, atan},
	{"ATAN2", FUNC_DD, atan2},
	{"ATANH", FUNC_D, atanh},
	{"AUTO", AUTO, NULL},
	{"AUTOSCALE", AUTOSCALE, NULL},
	{"AUTOTICKS", AUTOTICKS, NULL},
	{"AVALUE", AVALUE, NULL},
	{"AVG", AVG, NULL},
	{"AXES", AXES, NULL},
	{"B", SCRARRAY, NULL},
	{"BACKGROUND", BACKGROUND, NULL},
	{"BAR", BAR, NULL},
	{"BARDY", BARDY, NULL},
	{"BARDYDY", BARDYDY, NULL},
	{"BASELINE", BASELINE, NULL},
	{"BATCH", BATCH, NULL},
        {"BEGIN", BEGIN, NULL},
	{"BELOW", BELOW, NULL},
	{"BETA", FUNC_DD, beta},
	{"BETWEEN", BETWEEN, NULL},
	{"BI", FUNC_D, bi_wrap},
	{"BLACKMAN", BLACKMAN, NULL},
	{"BLOCK", BLOCK, NULL},
	{"BOTH", BOTH, NULL},
	{"BOTTOM", BOTTOM, NULL},
	{"BOX", BOX, NULL},
	{"C", SCRARRAY, NULL},
	{"CD", CD, NULL},
	{"CEIL", FUNC_D, ceil},
	{"CENTER", CENTER, NULL},
	{"CHAR", CHAR, NULL},
	{"CHART", CHART, NULL},
	{"CHDTR", FUNC_DD, chdtr},
	{"CHDTRC", FUNC_DD, chdtrc},
	{"CHDTRI", FUNC_DD, chdtri},
	{"CHI", FUNC_D, chi_wrap},
	{"CHRSTR", CHRSTR, NULL},
	{"CI", FUNC_D, ci_wrap},
	{"CLEAR", CLEAR, NULL},
	{"CLICK", CLICK, NULL},
	{"CLOSE", CLOSE, NULL},
	{"COEFFICIENTS", COEFFICIENTS, NULL},
	{"COLOR", COLOR, NULL},
	{"COMMENT", COMMENT, NULL},
	{"COMPLEX", COMPLEX, NULL},
	{"CONST", PROC_CONST, NULL},
	{"CONSTRAINTS", CONSTRAINTS, NULL},
	{"COPY", COPY, NULL},
	{"COS", FUNC_D, cos},
	{"COSH", FUNC_D, cosh},
	{"CYCLE", CYCLE, NULL},
	{"D", SCRARRAY, NULL},
	{"DAWSN", FUNC_D, dawsn},
	{"DAYMONTH", DAYMONTH, NULL},
	{"DAYOFWEEKL", DAYOFWEEKL, NULL},
	{"DAYOFWEEKS", DAYOFWEEKS, NULL},
	{"DAYOFYEAR", DAYOFYEAR, NULL},
	{"DDMMYY", DDMMYY, NULL},
	{"DECIMAL", DECIMAL, NULL},
	{"DEF", DEF, NULL},
	{"DEFAULT", DEFAULT, NULL},
	{"DEFINE", DEFINE, NULL},
	{"DEG", UCONSTANT, deg_uconst},
	{"DEGREESLAT", DEGREESLAT, NULL},
	{"DEGREESLON", DEGREESLON, NULL},
	{"DEGREESMMLAT", DEGREESMMLAT, NULL},
	{"DEGREESMMLON", DEGREESMMLON, NULL},
	{"DEGREESMMSSLAT", DEGREESMMSSLAT, NULL},
	{"DEGREESMMSSLON", DEGREESMMSSLON, NULL},
	{"DESCENDING", DESCENDING, NULL},
	{"DESCRIPTION", DESCRIPTION, NULL},
	{"DEVICE", DEVICE, NULL},
	{"DFT", DFT, NULL},
	{"DIFF", DIFFERENCE, NULL},
	{"DIFFERENCE", DIFFERENCE, NULL},
	{"DISK", DISK, NULL},
	{"DOWN", DOWN, NULL},
	{"DPI", DPI, NULL},
	{"DROP", DROP, NULL},
	{"DROPLINE", DROPLINE, NULL},
	{"ECHO", ECHO, NULL},
	{"ELLIE", FUNC_DD, ellie},
	{"ELLIK", FUNC_DD, ellik},
	{"ELLIPSE", ELLIPSE, NULL},
	{"ELLPE", FUNC_D, ellpe},
	{"ELLPK", FUNC_D, ellpk},
	{"ENGINEERING", ENGINEERING, NULL},
	{"EQ", EQ, NULL},
	{"ER", ERRORBAR, NULL},
	{"ERF", FUNC_D, erf},
	{"ERFC", FUNC_D, erfc},
	{"ERRORBAR", ERRORBAR, NULL},
	{"EXIT", EXIT, NULL},
	{"EXP", FUNC_D, exp},
	{"EXPN", FUNC_ND, expn},
	{"EXPONENTIAL", EXPONENTIAL, NULL},
	{"FAC", FUNC_I, fac},
	{"FALSE", OFF, NULL},
	{"FDTR", FUNC_NND, fdtr},
	{"FDTRC", FUNC_NND, fdtrc},
	{"FDTRI", FUNC_NND, fdtri},
	{"FFT", FFT, NULL},
	{"FILE", FILEP, NULL},
	{"FILL", FILL, NULL},
	{"FIT", FIT, NULL},
	{"FIXED", FIXED, NULL},
	{"FIXEDPOINT", FIXEDPOINT, NULL},
	{"FLOOR", FUNC_D, floor},
	{"FLUSH", FLUSH, NULL},
	{"FOCUS", FOCUS, NULL},
	{"FOLLOWS", FOLLOWS, NULL},
	{"FONT", FONTP, NULL},
	{"FORCE", FORCE, NULL},
	{"FORMAT", FORMAT, NULL},
	{"FORMULA", FORMULA, NULL},
	{"FRAME", FRAMEP, NULL},
	{"FREE", FREE, NULL},
	{"FREQUENCY", FREQUENCY, NULL},
	{"FRESNLC", FUNC_D, fresnlc_wrap},
	{"FRESNLS", FUNC_D, fresnls_wrap},
	{"FROM", FROM, NULL},
	{"F_OF_D", PROC_FUNC_D, NULL},
	{"F_OF_DD", PROC_FUNC_DD, NULL},
        {"F_OF_I", PROC_FUNC_I, NULL},
	{"F_OF_ND", PROC_FUNC_ND, NULL},
	{"F_OF_NN", PROC_FUNC_NN, NULL},
	{"F_OF_NND", PROC_FUNC_NND, NULL},
	{"F_OF_PPD", PROC_FUNC_PPD, NULL},
	{"F_OF_PPPD", PROC_FUNC_PPPD, NULL},
	{"GAMMA", FUNC_D, true_gamma},
	{"GDTR", FUNC_PPD, gdtr},
	{"GDTRC", FUNC_PPD, gdtrc},
	{"GE", GE, NULL},
	{"GENERAL", GENERAL, NULL},
	{"GETP", GETP, NULL},
	{"GRAPHS", GRAPHS, NULL},
	{"GRID", GRID, NULL},
	{"GT", GT, NULL},
	{"HAMMING", HAMMING, NULL},
	{"HANNING", HANNING, NULL},
	{"HARDCOPY", HARDCOPY, NULL},
	{"HBAR", HBAR, NULL},
	{"HGAP", HGAP, NULL},
	{"HIDDEN", HIDDEN, NULL},
	{"HISTO", HISTO, NULL},
	{"HMS", HMS, NULL},
	{"HORIZI", HORIZI, NULL},
	{"HORIZO", HORIZO, NULL},
	{"HORIZONTAL", HORIZONTAL, NULL},
	{"HYP2F1", FUNC_PPPD, hyp2f1},
	{"HYPERG", FUNC_PPD, hyperg},
	{"HYPOT", FUNC_DD, hypot},
	{"I0E", FUNC_D, i0e},
	{"I1E", FUNC_D, i1e},
	{"ID", ID, NULL},
	{"IFILTER", IFILTER, NULL},
	{"IGAM", FUNC_DD, igam},
	{"IGAMC", FUNC_DD, igamc},
	{"IGAMI", FUNC_DD, igami},
	{"IN", IN, NULL},
	{"INCBET", FUNC_PPD, incbet},
	{"INCBI", FUNC_PPD, incbi},
	{"INCREMENT", INCREMENT, NULL},
	{"INDEX", INDEX, NULL},
	{"INOUT", INOUT, NULL},
	{"INTEGRATE", INTEGRATE, NULL},
	{"INTERP", INTERP, NULL},
	{"INVDFT", INVDFT, NULL},
	{"INVERT", INVERT, NULL},
	{"INVFFT", INVFFT, NULL},
	{"IRAND", FUNC_I, irand_wrap},
	{"IV", FUNC_DD, iv_wrap},
	{"JDAY", JDAY, NULL},
	{"JDAY0", JDAY0, NULL},
	{"JUST", JUST, NULL},
	{"JV", FUNC_DD, jv_wrap},
	{"K0E", FUNC_D, k0e},
	{"K1E", FUNC_D, k1e},
	{"KILL", KILL, NULL},
	{"KN", FUNC_ND, kn_wrap},
	{"LABEL", LABEL, NULL},
	{"LANDSCAPE", LANDSCAPE, NULL},
	{"LAYOUT", LAYOUT, NULL},
	{"LBETA", FUNC_DD, lbeta},
	{"LE", LE, NULL},
	{"LEFT", LEFT, NULL},
	{"LEGEND", LEGEND, NULL},
	{"LENGTH", LENGTH, NULL},
	{"LGAMMA", FUNC_D, lgamma},
	{"LINE", LINE, NULL},
	{"LINESTYLE", LINESTYLE, NULL},
	{"LINEWIDTH", LINEWIDTH, NULL},
	{"LINK", LINK, NULL},
	{"LN", FUNC_D, log},
	{"LOAD", LOAD, NULL},
	{"LOCTYPE", LOCTYPE, NULL},
	{"LOG", LOG, NULL},
	{"LOG10", FUNC_D, log10},
	{"LOG2", FUNC_D, log2},
	{"LOGARITHMIC", LOGARITHMIC, NULL},
	{"LOGX", LOGX, NULL},
	{"LOGXY", LOGXY, NULL},
	{"LOGY", LOGY, NULL},
	{"LT", LT, NULL},
	{"MAGIC", MAGIC, NULL},
	{"MAGNITUDE", MAGNITUDE, NULL},
	{"MAJOR", MAJOR, NULL},
	{"MAP", MAP, NULL},
	{"MAX", MAXP, NULL},
	{"MAXOF", FUNC_DD, max_wrap},
	{"MIN", MINP, NULL},
	{"MINOF", FUNC_DD, min_wrap},
	{"MINOR", MINOR, NULL},
	{"MMDD", MMDD, NULL},
	{"MMDDHMS", MMDDHMS, NULL},
	{"MMDDYY", MMDDYY, NULL},
	{"MMDDYYHMS", MMDDYYHMS, NULL},
	{"MMSSLAT", MMSSLAT, NULL},
	{"MMSSLON", MMSSLON, NULL},
	{"MMYY", MMYY, NULL},
	{"MOD", FUNC_DD, fmod},
	{"MONTHDAY", MONTHDAY, NULL},
	{"MONTHL", MONTHL, NULL},
	{"MONTHS", MONTHS, NULL},
	{"MONTHSY", MONTHSY, NULL},
	{"MOVE", MOVE, NULL},
	{"NDTR", FUNC_D, ndtr},
	{"NDTRI", FUNC_D, ndtri},
	{"NE", NE, NULL},
	{"NEGATE", NEGATE, NULL},
	{"NEW", NEW, NULL},
	{"NONE", NONE, NULL},
	{"NONLFIT", NONLFIT, NULL},
	{"NORM", FUNC_D, fx},
	{"NORMAL", NORMAL, NULL},
	{"NOT", NOT, NULL},
	{"NUMBER", NUMBER, NULL},
	{"NXY", NXY, NULL},
	{"OFF", OFF, NULL},
	{"OFFSET", OFFSET, NULL},
	{"OFFSETX", OFFSETX, NULL},
	{"OFFSETY", OFFSETY, NULL},
	{"OFILTER", OFILTER, NULL},
	{"ON", ON, NULL},
	{"OP", OP, NULL},
	{"OR", OR, NULL},
	{"OUT", OUT, NULL},
	{"PAGE", PAGE, NULL},
	{"PARA", PARA, NULL},
	{"PARAMETERS", PARAMETERS, NULL},
	{"PARZEN", PARZEN, NULL},
	{"PATTERN", PATTERN, NULL},
	{"PDTR", FUNC_ND, pdtr},
	{"PDTRC", FUNC_ND, pdtrc},
	{"PDTRI", FUNC_ND, pdtri},
	{"PERIOD", PERIOD, NULL},
	{"PERP", PERP, NULL},
	{"PHASE", PHASE, NULL},
	{"PI", CONSTANT, pi_const},
	{"PIPE", PIPE, NULL},
	{"PLACE", PLACE, NULL},
	{"POINT", POINT, NULL},
	{"POLAR", POLAR, NULL},
	{"POLYI", POLYI, NULL},
	{"POLYO", POLYO, NULL},
	{"POP", POP, NULL},
	{"PORTRAIT", PORTRAIT, NULL},
	{"POWER", POWER, NULL},
	{"PREC", PREC, NULL},
	{"PREPEND", PREPEND, NULL},
	{"PRINT", PRINT, NULL},
	{"PS", PS, NULL},
	{"PSI", FUNC_D, psi},
	{"PUSH", PUSH, NULL},
	{"PUTP", PUTP, NULL},
	{"RAD", UCONSTANT, rad_uconst},
	{"RAND", CONSTANT, drand48},
	{"READ", READ, NULL},
	{"REAL", REAL, NULL},
	{"RECIPROCAL", RECIPROCAL, NULL},
	{"REDRAW", REDRAW, NULL},
	{"REGRESS", REGRESS, NULL},
	{"RGAMMA", FUNC_D, rgamma},
	{"RIGHT", RIGHT, NULL},
	{"RINT", FUNC_D, rint},
	{"RISER", RISER, NULL},
	{"RNORM", FUNC_DD, rnorm},
	{"ROT", ROT, NULL},
	{"ROUNDED", ROUNDED, NULL},
	{"RULE", RULE, NULL},
	{"RUNAVG", RUNAVG, NULL},
	{"RUNMAX", RUNMAX, NULL},
	{"RUNMED", RUNMED, NULL},
	{"RUNMIN", RUNMIN, NULL},
	{"RUNSTD", RUNSTD, NULL},
	{"SAVEALL", SAVEALL, NULL},
	{"SCALE", SCALE, NULL},
	{"SCIENTIFIC", SCIENTIFIC, NULL},
	{"SCROLL", SCROLL, NULL},
	{"SD", SD, NULL},
	{"SET", SET, NULL},
	{"SETS", SETS, NULL},
	{"SFORMAT", SFORMAT, NULL},
	{"SHI", FUNC_D, shi_wrap},
	{"SI", FUNC_D, si_wrap},
	{"SIGN", SIGN, NULL},
	{"SIN", FUNC_D, sin},
	{"SINH", FUNC_D, sinh},
	{"SIZE", SIZE, NULL},
	{"SKIP", SKIP, NULL},
	{"SLEEP", SLEEP, NULL},
	{"SMITH", SMITH, NULL},
	{"SORT", SORT, NULL},
	{"SOURCE", SOURCE, NULL},
	{"SPEC", SPEC, NULL},
	{"SPENCE", FUNC_D, spence},
	{"SPLINE", SPLINE, NULL},
	{"SQR", FUNC_D, sqr_wrap},
	{"SQRT", FUNC_D, sqrt},
	{"STACK", STACK, NULL},
	{"STACKED", STACKED, NULL},
	{"STACKEDBAR", STACKEDBAR, NULL},
	{"STACKEDHBAR", STACKEDHBAR, NULL},
	{"STAGGER", STAGGER, NULL},
	{"START", START, NULL},
	{"STDTR", FUNC_ND, stdtr},
	{"STDTRI", FUNC_ND, stdtri},
	{"STOP", STOP, NULL},
	{"STRING", STRING, NULL},
	{"STRUVE", FUNC_DD, struve},
	{"SUBTITLE", SUBTITLE, NULL},
	{"SYMBOL", SYMBOL, NULL},
	{"TAN", FUNC_D, tan},
	{"TANH", FUNC_D, tanh},
	{"TARGET", TARGET, NULL},
	{"TICK", TICKP, NULL},
	{"TICKLABEL", TICKLABEL, NULL},
	{"TICKS", TICKSP, NULL},
	{"TIMER", TIMER, NULL},
	{"TIMESTAMP", TIMESTAMP, NULL},
	{"TITLE", TITLE, NULL},
	{"TO", TO, NULL},
	{"TOP", TOP, NULL},
	{"TRIANGULAR", TRIANGULAR, NULL},
	{"TRUE", ON, NULL},
	{"TYPE", TYPE, NULL},
	{"UNLINK", UNLINK, NULL},
	{"UNIT", PROC_UNIT, NULL},
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
	{"WELCH", WELCH, NULL},
	{"WITH", WITH, NULL},
	{"WORLD", WORLD, NULL},
	{"WRITE", WRITE, NULL},
	{"WX1", WX1, NULL},
	{"WX2", WX2, NULL},
	{"WY1", WY1, NULL},
	{"WY2", WY2, NULL},
	{"X", X_TOK, NULL},
	{"X0", X0, NULL},
	{"X1", X1, NULL},
	{"XAXES", XAXES, NULL},
	{"XAXIS", XAXIS, NULL},
	{"XCOR", XCOR, NULL},
	{"XMAX", XMAX, NULL},
	{"XMIN", XMIN, NULL},
	{"XY", XY, NULL},
	{"XYDX", XYDX, NULL},
	{"XYDXDX", XYDXDX, NULL},
	{"XYDXDY", XYDXDY, NULL},
	{"XYDY", XYDY, NULL},
	{"XYDYDY", XYDYDY, NULL},
	{"XYHILO", XYHILO, NULL},
	{"XYR", XYR, NULL},
	{"XYSTRING", XYSTRING, NULL},
	{"XYZ", XYZ, NULL},
	{"Y", Y_TOK, NULL},
	{"Y0", Y0, NULL},
	{"Y1", Y1, NULL},
	{"Y2", Y2, NULL},
	{"Y3", Y3, NULL},
	{"Y4", Y4, NULL},
	{"YAXES", YAXES, NULL},
	{"YAXIS", YAXIS, NULL},
	{"YMAX", YMAX, NULL},
	{"YMIN", YMIN, NULL},
	{"YV", FUNC_DD, yv_wrap},
	{"YYMMDD", YYMMDD, NULL},
	{"YYMMDDHMS", YYMMDDHMS, NULL},
	{"ZERO", ZERO, NULL},
	{"ZEROXAXIS", ALTXAXIS, NULL},
	{"ZEROYAXIS", ALTYAXIS, NULL},
	{"ZETA", FUNC_DD, zeta},
	{"ZETAC", FUNC_D, zetac}
};

static int maxfunc = sizeof(ikey) / sizeof(symtab_entry);

int get_parser_gno(void)
{
    return whichgraph;
}

void set_parser_gno(int gno)
{
    if (is_valid_gno(gno) == TRUE) {
        whichgraph = gno;
    }
}

int get_parser_setno(void)
{
    return whichgraph;
}

void set_parser_setno(int setno)
{
    if (is_valid_setno(whichgraph, setno) == TRUE) {
        whichset = setno;
    }
}

int init_array(double **a, int n)
{
    *a = xrealloc(*a, n * SIZEOF_DOUBLE);
    
    return *a == NULL ? 1 : 0;
}

int init_scratch_arrays(int n)
{
    if (!init_array(&ax, n)) {
	if (!init_array(&bx, n)) {
	    if (!init_array(&cx, n)) {
		if (!init_array(&dx, n)) {
		    maxarr = n;
		    return 0;
		}
		free(cx);
	    }
	    free(bx);
	}
	free(ax);
    }
    return 1;
}

double *get_scratch(int ind)
{
    switch (ind) {
    case 0:
        return ax;
        break;
    case 1:
        return bx;
        break;
    case 2:
        return cx;
        break;
    case 3:
        return dx;
        break;
    default:
        return NULL;
        break;
    }
}

void scanner(char *s, int len, int setno, int *errpos)
{
    char *seekpos;
    int i;
    
    if (s == NULL || s[0] == '\0') {
        return;
    }
    
    strncpy(f_string, s, MAX_STRING_LENGTH - 2);
    f_string[MAX_STRING_LENGTH - 2] = '\0';
    strcat(f_string, " ");
    
    seekpos = f_string;

    while ((seekpos - f_string < MAX_STRING_LENGTH - 1) && (*seekpos == ' ' || *seekpos == '\t')) {
        seekpos++;
    }
    if (*seekpos == '\n' || *seekpos == '#') {
        return;
    }
    
    lowtoupper(f_string);
        
    interr = 0;
    whichgraph = get_cg();
    whichset = setno;
    curset = setno;
    pos = 0;
    lxy = len;

    fcnt = 0;
    log_results(s);
    yyparse();
    *errpos = interr;
    for (i = 0; i < fcnt; i++) {
	free(freelist[i]);
	freelist[i] = NULL;
    }
    
    if (gotparams) {
	gotparams = FALSE;
        getparms(paramfile);
    }
    
    if (gotread) {
	gotread = FALSE;
        getdata(get_cg(), readfile, readsrc, readtype);
    }
    
    if (gotnlfit) {
	gotnlfit = FALSE;
        do_nonlfit(nlfit_gno, nlfit_setno, nlfit_nsteps);
    }
}


int findf(symtab_entry *keytable, char *s)
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

int compare_keys (const void *a, const void *b)
{
  return (int) strcmp (((const symtab_entry*)a)->s, ((const symtab_entry*)b)->s);
}

/* add new entry to the symbol table */
int addto_symtab(symtab_entry newkey)
{
    int position;
    if ((position = findf(key, newkey.s)) < 0) {
        if ((key = (symtab_entry *) realloc(key, (maxfunc + 1)*sizeof(symtab_entry))) != NULL) {
	    key[maxfunc].type = newkey.type;
	    key[maxfunc].fnc = newkey.fnc;
	    key[maxfunc].s = malloc(strlen(newkey.s) + 1);
	    strcpy(key[maxfunc].s, newkey.s);
	    maxfunc++;
	    qsort(key, maxfunc, sizeof(symtab_entry), compare_keys);
	    return 0;
	} else {
	    errmsg ("Memory allocation failed in addto_symtab()!");
	    return -2;
	}
    } else if (alias_force == TRUE) { /* already exists but alias_force enabled */
        key[position].type = newkey.type;
	key[position].fnc = newkey.fnc;
	return 0;
    } else {
        return -1;
    }
}

/* initialize symbol table */
void init_symtab(void)
{
    int i;
    
    if ((key = (symtab_entry *) malloc(maxfunc*sizeof(symtab_entry))) != NULL) {
    	memcpy (key, ikey, maxfunc*sizeof(symtab_entry));
	for (i = 0; i < maxfunc; i++) {
	    key[i].s = malloc(strlen(ikey[i].s) + 1);
	    strcpy(key[i].s, ikey[i].s);
	}
	qsort(key, maxfunc, sizeof(symtab_entry), compare_keys);
	return;
    } else {
        errmsg ("Memory allocation failed in init_symtab()!");
	key = ikey;
	return;
    }
}

int getcharstr(void)
{
    if (pos >= strlen(f_string))
	 return EOF;
    return (f_string[pos++]);
}

void ungetchstr(void)
{
    if (pos > 0)
	pos--;
}

int yylex(void)
{
    int c, i;
    int found;
    static char s[MAX_STRING_LENGTH];
    char sbuf[MAX_STRING_LENGTH + 40];
    char *str;

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
	    s[i] = c;
	    i++;
	}
	if (c == EOF) {
	    yyerror("Nonterminating string");
	    return 0;
	}
	s[i] = '\0';
	str = malloc(strlen(s) + 1);
	strcpy(str, s);
	yylval.str = str;
	return CHRSTR;
    }
    if (c == '.' || isdigit(c)) {
	char stmp[80];
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
	    stmp[i++] = c;
	    c = getcharstr();
	}
	if (c == 'E' || c == 'e') {
	    stmp[i++] = c;
	    c = getcharstr();
	    if (c == '+' || c == '-') {
		stmp[i++] = c;
		c = getcharstr();
	    }
	    while (isdigit(c)) {
		stmp[i++] = c;
		c = getcharstr();
	    }
	}
	if (gotdot && i == 1) {
	    ungetchstr();
	    return '.';
	}
	stmp[i] = '\0';
	ungetchstr();
	sscanf(stmp, "%lf", &d);
	yylval.val = d;
	return NUMBER;
    }
/* graphs, sets, regions resp. */
    if (c == 'G' || c == 'S' || c == 'R') {
	char stmp[80];
	int i = 0, ctmp = c, gn, sn, rn;
	c = getcharstr();
	while (isdigit(c) || c == '$' || c == '_') {
	    stmp[i++] = c;
	    c = getcharstr();
	}
	if (i == 0) {
	    c = ctmp;
	    ungetchstr();
	} else {
	    ungetchstr();
	    if (ctmp == 'G') {
	        stmp[i] = '\0';
		if (i == 1 && stmp[0] == '_') {
                    gn = get_recent_gno();
                } else if (i == 1 && stmp[0] == '$') {
                    gn = whichgraph;
                } else {
                    gn = atoi(stmp);
                }
		if (set_graph_active(gn, TRUE) == GRACE_EXIT_SUCCESS) {
		    yylval.ival = gn;
		    whichgraph = gn;
		    return GRAPHNO;
		}
	    } else if (ctmp == 'S') {
	        stmp[i] = '\0';
		if (i == 1 && stmp[0] == '_') {
                    sn = get_recent_setno();
                } else if (i == 1 && stmp[0] == '$') {
                    sn = whichset;
                } else {
		    sn = atoi(stmp);
                }
		if (allocate_set(whichgraph, sn) == GRACE_EXIT_SUCCESS) {
		    yylval.ival = sn;
		    whichset = sn;
		    lxy = getsetlength(whichgraph, sn);
		    return SETNUM;
		}
	    } else if (ctmp == 'R') {
	        stmp[i] = '\0';
		rn = atoi(stmp);
		if (rn >= 0 && rn < MAXREGION) {
		    yylval.ival = rn;
		    return REGNUM;
		}
	    }
	}
    }
    if (isalpha(c)) {
	char *p = sbuf;

	do {
	    *p++ = c;
	} while ((c = getcharstr()) != EOF && (isalpha(c) || isdigit(c) ||
                  c == '_' || c == '$'));
	ungetchstr();
	*p = '\0';
#ifdef DEBUG
        if (debuglevel == 2) {
	    printf("->%s<-\n", sbuf);
	}
#endif
	found = -1;
	if ((found = findf(key, sbuf)) >= 0) {
	    if (key[found].type == SCRARRAY) {
		switch (sbuf[0]) {
		case 'A':
		    yylval.ival = 0;
		    return SCRARRAY;
		case 'B':
		    yylval.ival = 1;
		    return SCRARRAY;
		case 'C':
		    yylval.ival = 2;
		    return SCRARRAY;
		case 'D':
		    yylval.ival = 3;
		    return SCRARRAY;
		}
	    }
	    else if (key[found].type == FITPARM) {
		int index = sbuf[1] - '0';
		yylval.val = index;
		return FITPARM;
	    }
	    else if (key[found].type == FITPMAX) {
		int index = sbuf[1] - '0';
		yylval.val = index;
		return FITPMAX;
	    }
	    else if (key[found].type == FITPMIN) {
		int index = sbuf[1] - '0';
		yylval.val = index;
		return FITPMIN;
	    }
	    else if (key[found].type == FUNC_I) {
		yylval.func = found;
		return FUNC_I;
	    }
	    else if (key[found].type == CONSTANT) {
		yylval.func = found;
		return CONSTANT;
	    }
	    else if (key[found].type == UCONSTANT) {
		yylval.func = found;
		return UCONSTANT;
	    }
	    else if (key[found].type == FUNC_D) {
		yylval.func = found;
		return FUNC_D;
	    }
	    else if (key[found].type == FUNC_ND) {
		yylval.func = found;
		return FUNC_ND;
	    }
	    else if (key[found].type == FUNC_DD) {
		yylval.func = found;
		return FUNC_DD;
	    }
	    else if (key[found].type == FUNC_NND) {
		yylval.func = found;
		return FUNC_NND;
	    }
	    else if (key[found].type == FUNC_PPD) {
		yylval.func = found;
		return FUNC_PPD;
	    }
	    else if (key[found].type == FUNC_PPPD) {
		yylval.func = found;
		return FUNC_PPPD;
	    }
	    else { /* set up special cases */
		switch (key[found].type) {
		case XAXIS:
		    naxis = X_AXIS;
		    break;
		case YAXIS:
		    naxis = Y_AXIS;
		    break;
		case ALTXAXIS:
		    naxis = ZX_AXIS;
		    break;
		case ALTYAXIS:
		    naxis = ZY_AXIS;
		    break;
		case AXES:
		    naxis = ALL_AXES;
		    break;
		case XAXES:
		    naxis = ALL_X_AXES;
		    break;
		case YAXES:
		    naxis = ALL_Y_AXES;
		    break;
		case GRAPHS:
		    yylval.ival = ALL_GRAPHS;
		    whichgraph = ALL_GRAPHS;
		    return GRAPHS;
		    break;
		case SETS:
		    yylval.ival = ALL_SETS;
		    whichset = ALL_SETS;
		    return SETS;
		    break;
		default:
		    break;
		}
	    }
	    yylval.func = key[found].type;
	    return key[found].type;
	} else {
	    strcat(sbuf, ": No such function or variable");
	    yyerror(sbuf);
	    return 0;
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

int follow(int expect, int ifyes, int ifno)
{
    int c = getcharstr();

    if (c == expect) {
	return ifyes;
    }
    ungetchstr();
    return ifno;
}

void yyerror(char *s)
{
    int i;
    char buf[2*MAX_STRING_LENGTH + 40];
    sprintf(buf, "%s: %s", s, f_string);
    i = strlen(buf);
    buf[i - 1] = 0;
    errmsg(buf);
    interr = 1;
}



/* TODO: the whole set_prop stuff to be removed! */
#include <stdarg.h>

void set_prop(int gno,...)
{
    va_list var;
    int prop, allsets = 0;
    int i, j, startg, endg, starts = 0, ends = 0;
    double dprop;
    char *cprop;
    char buf[256];

    if (gno == -1) {
	startg = 0;
	endg = number_of_graphs()  - 1;
    } else {
	startg = endg = gno;
    }

    va_start(var, gno);
    while ((prop = va_arg(var, int)) != 0) {
	switch (prop) {
	case SETS:
	    allsets = 1;
	    starts = 0;
	    ends = number_of_sets(gno) - 1;
	    break;
	case SET:
	    switch (prop = va_arg(var, int)) {
	    case SETNUM:
		prop = va_arg(var, int);
		if (prop == -1) {
		    allsets = 1;
		    starts = 0;
		    ends = number_of_sets(gno) - 1;
		} else {
		    allsets = 0;
		    starts = ends = prop;
		}
		break;
	    }
	    break;
	case TYPE:
	    prop = va_arg(var, int);
	    for (i = startg; i <= endg; i++) {
		if (allsets) {
		    ends = g[i].maxplot - 1;
		}
		for (j = starts; j <= ends; j++) {
		    g[i].p[j].type = prop;
		}
	    }
	    break;
	case PREC:
	    prop = va_arg(var, int);
	    for (i = startg; i <= endg; i++) {
		if (allsets) {
		    ends = g[i].maxplot - 1;
		}
		for (j = starts; j <= ends; j++) {
		    g[i].p[j].avalue.prec = prop;
		}
	    }
	    break;
	case FORMAT:
	    prop = va_arg(var, int);
	    for (i = startg; i <= endg; i++) {
		if (allsets) {
		    ends = g[i].maxplot - 1;
		}
		for (j = starts; j <= ends; j++) {
		    g[i].p[j].avalue.format = prop;
		}
	    }
	    break;
	case LINEWIDTH:
	    prop = va_arg(var, double);
	    for (i = startg; i <= endg; i++) {
		if (allsets) {
		    ends = g[i].maxplot - 1;
		}
		for (j = starts; j <= ends; j++) {
		    g[i].p[j].linew = prop;
		}
	    }
	    break;
	case LINESTYLE:
	    prop = va_arg(var, int);
	    for (i = startg; i <= endg; i++) {
		if (allsets) {
		    ends = g[i].maxplot - 1;
		}
		for (j = starts; j <= ends; j++) {
		    g[i].p[j].lines = prop;
		    if (check_err) {
			return;
		    }
		}
	    }
	    break;
	case COMMENT:
	    cprop = va_arg(var, char *);
	    for (i = startg; i <= endg; i++) {
		if (allsets) {
		    ends = g[i].maxplot - 1;
		}
		for (j = starts; j <= ends; j++) {
		    strcpy(g[i].p[j].comments, cprop);
		}
	    }
	    break;
	case FILL:
	    switch (prop = va_arg(var, int)) {
	    case TYPE:
		prop = va_arg(var, int);
		for (i = startg; i <= endg; i++) {
		    if (allsets) {
			ends = g[i].maxplot - 1;
		    }
		    for (j = starts; j <= ends; j++) {
			switch (prop) {
                        case 0:
                            g[i].p[j].filltype = SETFILL_NONE;
                            break;
                        case 1:
                            g[i].p[j].filltype = SETFILL_POLYGON;
                            break;
                        case 2:
                            g[i].p[j].filltype = SETFILL_BASELINE;
                            g[i].p[j].baseline_type = BASELINE_TYPE_0;
                            break;
                        case 6:
                            g[i].p[j].filltype = SETFILL_BASELINE;
                            g[i].p[j].baseline_type = BASELINE_TYPE_GMIN;
                            break;
                        case 7:
                            g[i].p[j].filltype = SETFILL_BASELINE;
                            g[i].p[j].baseline_type = BASELINE_TYPE_GMAX;
                            break;
                        }
		    }
		}
		break;
	    case WITH:
		prop = va_arg(var, int);
                break;
	    case PATTERN:
		prop = va_arg(var, int);
		for (i = startg; i <= endg; i++) {
		    if (allsets) {
			ends = g[i].maxplot - 1;
		    }
		    for (j = starts; j <= ends; j++) {
			g[i].p[j].setfillpen.pattern = prop;
		    }
		}
		break;
	    default:
		sprintf(buf, "Attribute not found in setprops()-FILL, # = %d", prop);
		errmsg(buf);
		break;
	    }
	    break;
	case SKIP:
	    prop = va_arg(var, int);
	    for (i = startg; i <= endg; i++) {
		if (allsets) {
		    ends = g[i].maxplot - 1;
		}
		for (j = starts; j <= ends; j++) {
		    g[i].p[j].symskip = prop;
		}
	    }
	    break;
	case SYMBOL:
	    switch (prop = va_arg(var, int)) {
	    case TYPE:
		prop = va_arg(var, int);
		for (i = startg; i <= endg; i++) {
		    if (allsets) {
			ends = g[i].maxplot - 1;
		    }
		    for (j = starts; j <= ends; j++) {
			g[i].p[j].sym = prop;
		    }
		}
		break;
	    case FILL:
		prop = va_arg(var, int);
		for (i = startg; i <= endg; i++) {
		    if (allsets) {
			ends = g[i].maxplot - 1;
		    }
		    for (j = starts; j <= ends; j++) {
	    	    	switch (prop){
	    	    	case 0:
	    	    	    g[i].p[j].symfillpen.pattern = 0;
	    	    	    break;
	    	    	case 1:
	    	    	    g[i].p[j].symfillpen.pattern = 1;
	    	    	    break;
	    	    	case 2:
	    	    	    g[i].p[j].symfillpen.pattern = 1;
			    g[i].p[j].symfillpen.color = getbgcolor();
	    	    	    break;
	    	    	}
		    }
		}
		break;
	    case SIZE:
		dprop = va_arg(var, double);
		for (i = startg; i <= endg; i++) {
		    if (allsets) {
			ends = g[i].maxplot - 1;
		    }
		    for (j = starts; j <= ends; j++) {
                        g[i].p[j].symsize = dprop;
		    }
		}
		break;
	    case SKIP:
		prop = va_arg(var, int);
		for (i = startg; i <= endg; i++) {
		    if (allsets) {
			ends = g[i].maxplot - 1;
		    }
		    for (j = starts; j <= ends; j++) {
			g[i].p[j].symskip = prop;
		    }
		}
		break;
	    case CHAR:
		prop = va_arg(var, int);
		for (i = startg; i <= endg; i++) {
		    if (allsets) {
			ends = g[i].maxplot - 1;
		    }
		    for (j = starts; j <= ends; j++) {
			g[i].p[j].symchar = prop;
		    }
		}
		break;
	    case PATTERN:
		prop = va_arg(var, int);
		for (i = startg; i <= endg; i++) {
		    if (allsets) {
			ends = g[i].maxplot - 1;
		    }
		    for (j = starts; j <= ends; j++) {
			g[i].p[j].sympen.pattern = prop;
		    }
		}
		break;
	    case LINEWIDTH:
		prop = va_arg(var, double);
		for (i = startg; i <= endg; i++) {
		    if (allsets) {
			ends = g[i].maxplot - 1;
		    }
		    for (j = starts; j <= ends; j++) {
			g[i].p[j].symlinew = prop;
		    }
		}
		break;
	    case LINESTYLE:
		prop = va_arg(var, int);
		for (i = startg; i <= endg; i++) {
		    if (allsets) {
			ends = g[i].maxplot - 1;
		    }
		    for (j = starts; j <= ends; j++) {
			g[i].p[j].symlines = prop;
		    }
		}
		break;
	    default:
		sprintf(buf, "Attribute not found in setprops()-SYMBOL, # = %d", prop);
		errmsg(buf);
		break;
	    }
	    break;
	case ERRORBAR:
	    switch (prop = va_arg(var, int)) {
	    case LENGTH:
		dprop = va_arg(var, double);
		for (i = startg; i <= endg; i++) {
		    if (allsets) {
			ends = g[i].maxplot - 1;
		    }
		    for (j = starts; j <= ends; j++) {
			g[i].p[j].errbar.length = dprop;
		    }
		}
		break;
	    case TYPE:
		prop = va_arg(var, int);
		for (i = startg; i <= endg; i++) {
		    if (allsets) {
			ends = g[i].maxplot - 1;
		    }
		    for (j = starts; j <= ends; j++) {
			g[i].p[j].errbar.type = prop;
		    }
		}
		break;
	    case LINEWIDTH:
		prop = va_arg(var, double);
		for (i = startg; i <= endg; i++) {
		    if (allsets) {
			ends = g[i].maxplot - 1;
		    }
		    for (j = starts; j <= ends; j++) {
			g[i].p[j].errbar.linew = prop;
		    }
		}
		break;
	    case LINESTYLE:
		prop = va_arg(var, int);
		for (i = startg; i <= endg; i++) {
		    if (allsets) {
			ends = g[i].maxplot - 1;
		    }
		    for (j = starts; j <= ends; j++) {
			g[i].p[j].errbar.lines = prop;
		    }
		}
		break;
	    case RISER:
		prop = va_arg(var, int);
		switch (prop) {
		case ON:
		    prop = va_arg(var, int);
                    break;
		case LINEWIDTH:
		    prop = va_arg(var, double);
		    for (i = startg; i <= endg; i++) {
			if (allsets) {
			    ends = g[i].maxplot - 1;
			}
			for (j = starts; j <= ends; j++) {
			    g[i].p[j].errbar.riser_linew = prop;
			}
		    }
		    break;
		case LINESTYLE:
		    prop = va_arg(var, int);
		    for (i = startg; i <= endg; i++) {
			if (allsets) {
			    ends = g[i].maxplot - 1;
			}
			for (j = starts; j <= ends; j++) {
			    g[i].p[j].errbar.riser_lines = prop;
			}
		    }
		    break;
		default:
		    sprintf(buf, "Attribute not found in setprops()-RISER, # = %d", prop);
		    errmsg(buf);
		    break;
		}
		break;
	    default:
		sprintf(buf, "Attribute not found in setprops()-ERRORBAR, # = %d", prop);
		errmsg(buf);
		break;
	    }
	    break;
	default:
	    sprintf(buf, "Attribute not found in setprops()-top, # = %d", prop);
	    errmsg(buf);
	    break;
	}
    }
    va_end(var);
    set_dirtystate();
}


/* Wrappers for some functions*/

static double ai_wrap(double x)
{
    double retval, dummy1, dummy2, dummy3;
    (void) airy(x, &retval, &dummy1, &dummy2, &dummy3);
    return retval;
}

static double bi_wrap(double x)
{
    double retval, dummy1, dummy2, dummy3;
    (void) airy(x, &dummy1, &dummy2, &retval, &dummy3);
    return retval;
}

static double ci_wrap(double x)
{
    double retval, dummy1;
    (void) sici(x, &dummy1, &retval);
    return retval;
}

static double si_wrap(double x)
{
    double retval, dummy1;
    (void) sici(x, &retval, &dummy1);
    return retval;
}

static double chi_wrap(double x)
{
    double retval, dummy1;
    (void) shichi(x, &dummy1, &retval);
    return retval;
}

static double shi_wrap(double x)
{
    double retval, dummy1;
    (void) shichi(x, &retval, &dummy1);
    return retval;
}

static double fresnlc_wrap(double x)
{
    double retval, dummy1;
    (void) fresnl(x, &dummy1, &retval);
    return retval;
}

static double fresnls_wrap(double x)
{
    double retval, dummy1;
    (void) fresnl(x, &retval, &dummy1);
    return retval;
}

static double iv_wrap(double v, double x)
{
    double retval;
    if (v == 0) {
	retval = i0(x);
    } else if (v == 1) {
	retval = i1(x);
    } else {
	retval = iv(v, x);
    }
    return retval;
}

static double jv_wrap(double v, double x)
{
    double retval;
    if (v == rint(v)) {
	retval = jn((int) v, x);
    } else {
	retval = jv(v, x);
    }
    return retval;
}

static double kn_wrap(int n, double x)
{
    double retval;
    if (n == 0) {
	retval = k0(x);
    } else if (n == 1) {
	retval = k1(x);
    } else {
	retval = kn(n, x);
    }
    return retval;
}

static double yv_wrap(double v, double x)
{
    double retval;
    if (v == rint(v)) {
	retval = yn((int) v, x);
    } else {
	retval = yv(v, x);
    }
    return retval;
}

static double sqr_wrap(double x)
{
    return x*x;
}

static double max_wrap(double x, double y)
{
	    return (x >= y ? x : y);
}

static double min_wrap(double x, double y)
{
	    return (x <= y ? x : y);
}

static double irand_wrap(int x)
{
    return (double) (lrand48() % x);
}

static double pi_const(void)
{
    return M_PI;
}

static double deg_uconst(void)
{
    return M_PI / 180.0;
}

static double rad_uconst(void)
{
    return 1.0;
}

#define C1 0.1978977093962766
#define C2 0.1352915131768107

double rnorm(double mean, double sdev)
{
    double u = drand48();

    return mean + sdev * (pow(u, C2) - pow(1.0 - u, C2)) / C1;
}

double fx(double x)
{
    return 1.0 / sqrt(2.0 * M_PI) * exp(-x * x * 0.5);
}
