%{
/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2006 Grace Development Team
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

#include "grace/baseP.h"
#include "grace/graalP.h"

#include "parser.h"
#include "scanner.h"

#define YYPARSE_PARAM scanner
#define YYLEX_PARAM   scanner

#define REGISTER_DARR(da)   graal_register_darr(yyget_extra(scanner), da)
#define SET_DOTCONTEXT(ctx) graal_set_dotcontext(yyget_extra(scanner), ctx)

void yyerror(char *s)
{
    errmsg(s);
}

%}

%locations
%pure_parser
         
%union {
    int    ival;
    double dval;
    DArray *darr;
    char   *sval;
    void   *gobj;
    GVar   *gvar;
}

%token <sval> TOK_NAME

%token <dval> TOK_NUMBER
%token <sval> TOK_STRING
%token <gobj> TOK_OBJECT

%token <sval> TOK_BLOCK

%token        TOK_PROP_NIL
%token <ival> TOK_PROP_BOOL
%token <dval> TOK_PROP_NUM
%token <sval> TOK_PROP_STR
%token <darr> TOK_PROP_ARR

%token <gvar> TOK_VAR_NIL
%token <gvar> TOK_VAR_BOOL
%token <gvar> TOK_VAR_NUM
%token <gvar> TOK_VAR_STR
%token <gvar> TOK_VAR_ARR

%token TOK_RANGE

%token TOK_EQ
%token TOK_NE
%token TOK_GE
%token TOK_LE

%token TOK_OR
%token TOK_AND

%token TOK_TRUE
%token TOK_FALSE

%token TOK_LENGTH

%token TOK_IF
%token TOK_ELSE

%type <gvar> avar

%type <dval> expr
%type <ival> iexpr
%type <ival> nexpr
%type <ival> bexpr
%type <darr> list
%type <darr> array
%type <darr> vexpr
%type <gobj> object
%type <sval> sarray
%type <sval> sexpr

/* Precedence */
%nonassoc '?' ':'
%left TOK_OR
%left TOK_AND
%nonassoc '>' '<' TOK_GE TOK_LE TOK_EQ TOK_NE
%left '-' '+' '@'
%left '*' '/' '%'
%nonassoc UMINUS
%right '^'

%%
input:	/* empty */
	|   input line ';' { graal_free_darrs(yyget_extra(scanner)); }
	;

line:	/* empty */
	|   TOK_NAME '=' expr {
                GVar *var = graal_get_var(yyget_extra(scanner), $1, TRUE);
                xfree($1);
                if (gvar_set_num(var, $3) != RETURN_SUCCESS) {
	            yyerror("assignment failed");
                    YYABORT;
                }
            }
	|   TOK_NAME '=' bexpr {
                GVar *var = graal_get_var(yyget_extra(scanner), $1, TRUE);
                xfree($1);
                if (gvar_set_bool(var, $3) != RETURN_SUCCESS) {
	            yyerror("assignment failed");
                    YYABORT;
                }
            }
	|   TOK_NAME '=' vexpr {
                GVar *var = graal_get_var(yyget_extra(scanner), $1, TRUE);
                xfree($1);
                if (gvar_set_arr(var, $3) != RETURN_SUCCESS) {
	            yyerror("assignment failed");
                    YYABORT;
                }
            }
	|   TOK_NAME '=' sexpr {
                GVar *var = graal_get_var(yyget_extra(scanner), $1, TRUE);
                xfree($1);
                if (gvar_set_str(var, $3) != RETURN_SUCCESS) {
                    xfree($3);
	            yyerror("assignment failed");
                    YYABORT;
                } else {
                    xfree($3);
                }
            }
	|   avar '=' expr {
                if (gvar_set_num($1, $3) != RETURN_SUCCESS) {
	            yyerror("assignment failed - check types");
                    YYABORT;
                }
            }
	|   avar '=' bexpr {
                if (gvar_set_bool($1, $3) != RETURN_SUCCESS) {
	            yyerror("assignment failed - check types");
                    YYABORT;
                }
            }
	|   avar '=' vexpr {
                if (gvar_set_arr($1, $3) != RETURN_SUCCESS) {
	            yyerror("assignment failed - check types");
                    YYABORT;
                }
            }
	|   avar '=' sexpr {
                if (gvar_set_str($1, $3) != RETURN_SUCCESS) {
                    xfree($3);
	            yyerror("assignment failed - check types");
                    YYABORT;
                } else {
                    xfree($3);
                }
            }
	|   TOK_VAR_NUM '+' '=' expr {
                double val;
                gvar_get_num($1, &val);
                gvar_set_num($1, val + $4);
            }
	|   TOK_VAR_NUM '-' '=' expr {
                double val;
                gvar_get_num($1, &val);
                gvar_set_num($1, val - $4);
            }
	|   TOK_VAR_NUM '*' '=' expr {
                double val;
                gvar_get_num($1, &val);
                gvar_set_num($1, val*$4);
            }
	|   TOK_VAR_NUM '/' '=' expr {
                if ($4 == 0) {
	            yyerror("divide by zero");
                    YYABORT;
                } else {
                    double val;
                    gvar_get_num($1, &val);
                    gvar_set_num($1, val/$4);
                }
            }
	|   TOK_VAR_ARR '[' nexpr ']' '=' expr {
                DArray *da;
                gvar_get_arr($1, &da);
                if ($3 < da->size) {
                    da->x[$3] = $6;
                } else {
	            yyerror("index beyond array bounds");
                    YYABORT;
                }
            }
	|   TOK_VAR_ARR '+' '=' expr {
                DArray *da;
                gvar_get_arr($1, &da);
                darray_add_val(da, $4);
            }
	|   TOK_VAR_ARR '-' '=' expr {
                DArray *da;
                gvar_get_arr($1, &da);
                darray_add_val(da, -$4);
            }
	|   TOK_VAR_ARR '*' '=' expr {
                DArray *da;
                gvar_get_arr($1, &da);
                darray_mul_val(da, $4);
            }
	|   TOK_VAR_ARR '/' '=' expr {
                if ($4 == 0) {
	            yyerror("divide by zero");
                    YYABORT;
                } else {
                    DArray *da;
                    gvar_get_arr($1, &da);
                    darray_mul_val(da, 1.0/$4);
                }
            }
	|   TOK_VAR_STR '@' '=' sexpr {
                char *val, *buf;
                gvar_get_str($1, &val);
                buf = copy_string(NULL, val);
                buf = concat_strings(buf, $4);
                xfree($4);
                gvar_set_str($1, buf);
                xfree(buf);
            }
	|   object '.' TOK_NAME '=' expr {
                Graal *g = yyget_extra(scanner);
                GVarData prop;
                prop.num = $5;
                if (graal_set_user_obj_prop(g, $1, $3, GVarNum, prop) !=
                    RETURN_SUCCESS) {
	            yyerror("assignment failed");
                }
                xfree($3);
            }
	|   object '.' TOK_NAME '=' bexpr {
                Graal *g = yyget_extra(scanner);
                GVarData prop;
                prop.bool = $5;
                if (graal_set_user_obj_prop(g, $1, $3, GVarBool, prop) !=
                    RETURN_SUCCESS) {
	            yyerror("assignment failed");
                }
                xfree($3);
            }
	|   object '.' TOK_NAME '=' sexpr {
                Graal *g = yyget_extra(scanner);
                GVarData prop;
                prop.str = $5;
                if (graal_set_user_obj_prop(g, $1, $3, GVarStr, prop) !=
                    RETURN_SUCCESS) {
	            yyerror("assignment failed");
                }
                xfree($3);
                xfree($5);
            }
	|   object '.' TOK_NAME '=' vexpr {
                Graal *g = yyget_extra(scanner);
                GVarData prop;
                prop.arr = $5;
                if (graal_set_user_obj_prop(g, $1, $3, GVarArr, prop) !=
                    RETURN_SUCCESS) {
	            yyerror("assignment failed");
                }
                xfree($3);
            }
	|   eval_anon expr {
                GVarData vardata;
                vardata.num = $2;
                graal_call_eval_proc(yyget_extra(scanner), GVarNum, vardata);
            }
	|   eval_anon bexpr {
                GVarData vardata;
                vardata.bool = $2;
                graal_call_eval_proc(yyget_extra(scanner), GVarBool, vardata);
            }
	|   eval_anon vexpr {
                GVarData vardata;
                vardata.arr = $2;
                graal_call_eval_proc(yyget_extra(scanner), GVarArr, vardata);
            }
	|   eval_anon sexpr {
                GVarData vardata;
                vardata.str = $2;
                graal_call_eval_proc(yyget_extra(scanner), GVarStr, vardata);
                xfree($2);
            }
	|   eval_anon TOK_VAR_NIL {
                GVarData vardata;
                vardata.num = 0;
                graal_call_eval_proc(yyget_extra(scanner), GVarNil, vardata);
            }
	|   '~' avar { gvar_clear($2); }
        ;

eval_anon:  '?' { graal_set_RHS(yyget_extra(scanner), TRUE); }
	;

avar:	    TOK_VAR_NIL  { $$ = $1; }
	|   TOK_VAR_NUM  { $$ = $1; }
	|   TOK_VAR_BOOL { $$ = $1; }
	|   TOK_VAR_ARR  { $$ = $1; }
	|   TOK_VAR_STR  { $$ = $1; }
        ;

expr:	    TOK_NUMBER { $$ = $1; }
	|   TOK_VAR_NUM {
                gvar_get_num($1, &$$);
            }
	|   object '.' TOK_PROP_NUM {
                $$ = $3;
            }
        |   expr '+' expr { $$ = $1 + $3; }
	|   expr '-' expr { $$ = $1 - $3; }
	|   expr '*' expr { $$ = $1 * $3; }
	|   expr '/' expr {
                if ($3 == 0.0) {
	            yyerror("divide by zero");
                    YYABORT;
                } else {
	            $$ = $1 / $3;
                }
	    }
        |   expr '^' expr {
                if ($1 < 0 && rint($3) != $3) {
                    yyerror("negative value raised to non-integer power");
                    YYABORT;
                } else if ($1 == 0.0 && $3 <= 0.0) {
                    yyerror("zero raised to non-positive power");
                    YYABORT;
                } else {
                    $$ = pow($1, $3);
                }
            }
	|   '-' expr %prec UMINUS { $$ = -$2; }
	|   '+' expr %prec UMINUS { $$ = $2; }
	|   '(' expr ')' { $$ = $2; }
	|   array '[' nexpr ']' {
                if ($3 < $1->size) {
                    $$ = $1->x[$3];
                } else {
	            yyerror("index beyond array bounds");
                    YYABORT;
                }
            }
	|   bexpr '?' expr ':' expr { $$ = $1 ? $3:$5; }
        |   TOK_LENGTH '(' vexpr ')' { $$ = $3 ? $3->size:0; }
        |   TOK_LENGTH '(' sexpr ')' {
                $$ = strlen($3);
                xfree($3);
            }
        ;

iexpr:	    expr {
                int itmp = rint($1);
                if (fabs(itmp - $1) > 1.e-6) {
                    yyerror("non-integer value supplied for integer");
                    YYABORT;
                } else {
                    $$ = itmp;
                }
	    }
	|   iexpr '%' iexpr {
                if ($3 == 0) {
	            yyerror("divide by zero");
                    YYABORT;
                } else {
                    $$ = (int) rint($1) % (int) rint($3);
                }
            }
        ;

nexpr:	    iexpr {
                if ($1 < 0) {
                    yyerror("negative value supplied for non-negative");
                    YYABORT;
                } else {
                    $$ = $1;
                }
	    }
        ;

list:	    /* empty */ {
                $$ = NULL;
            }
	|   expr {
                $$ = darray_new(1);
                if ($$) {
                    REGISTER_DARR($$);
                    darray_set_val($$, 0, $1);
                }
            }
	|   list ',' expr {
                darray_append_val($1, $3);
                $$ = $1;
            }
	;

array:	    '{' list '}' { $$ = $2; }
	|   TOK_VAR_ARR {
                DArray *da;
                gvar_get_arr($1, &da);
                $$ = darray_copy(da);
                if ($$) {
                    REGISTER_DARR($$);
                }
            }
	|   object '.' TOK_PROP_ARR {
                $$ = $3;
                REGISTER_DARR($$);
            }
	;


vexpr:      array { $$ = $1; }
	|   array '[' nexpr TOK_RANGE nexpr ']' {
                if ($3 >= $1->size || $5 >= $1->size) {
	            yyerror("index beyond array bounds");
                    YYABORT;
                } else
                if ($3 > $5) {
	            yyerror("non-positive index range");
                    YYABORT;
                } else {
                    $$ = darray_slice($1, $3, $5);
                    REGISTER_DARR($$);
                }
            }
        |   vexpr '@' vexpr {
                $$ = darray_concat($1, $3);
                REGISTER_DARR($$);
            }
        |   vexpr '+' expr {
                $$ = darray_copy($1);
                if ($$) {
                    REGISTER_DARR($$);
                    darray_add_val($$, $3);
                }
            }
        |   expr '+' vexpr {
                $$ = darray_copy($3);
                if ($$) {
                    REGISTER_DARR($$);
                    darray_add_val($$, $1);
                }
            }
        |   vexpr '+' vexpr {
                if ($1->size != $3->size) {
	            yyerror("vector lengths mismatch");
                    YYABORT;
                } else {
                    $$ = darray_copy($1);
                    if ($$) {
                        REGISTER_DARR($$);
                        darray_add($$, $3);
                    }
                }
            }
        |   vexpr '-' expr {
                $$ = darray_copy($1);
                if ($$) {
                    REGISTER_DARR($$);
                    darray_add_val($$, -$3);
                }
            }
        |   expr '-' vexpr {
                $$ = darray_copy($3);
                if ($$) {
                    REGISTER_DARR($$);
                    darray_mul_val($$, -1.0);
                    darray_add_val($$, $1);
                }
            }
        |   vexpr '-' vexpr {
                if ($1->size != $3->size) {
	            yyerror("vector lengths mismatch");
                    YYABORT;
                } else {
                    $$ = darray_copy($1);
                    if ($$) {
                        REGISTER_DARR($$);
                        darray_sub($$, $3);
                    }
                }
            }
        |   vexpr '*' expr {
                $$ = darray_copy($1);
                if ($$) {
                    darray_mul_val($$, $3);
                }
            }
        |   expr '*' vexpr {
                $$ = darray_copy($3);
                if ($$) {
                    REGISTER_DARR($$);
                    darray_mul_val($$, $1);
                }
            }
        |   vexpr '*' vexpr {
                if ($1->size != $3->size) {
	            yyerror("vector lengths mismatch");
                    YYABORT;
                } else {
                    $$ = darray_copy($1);
                    if ($$) {
                        REGISTER_DARR($$);
                        darray_mul($$, $3);
                    }
                }
            }
        |   vexpr '/' expr {
                $$ = darray_copy($1);
                if ($$) {
                    REGISTER_DARR($$);
                    if ($3 == 0.0) {
	                yyerror("divide by zero");
                        YYABORT;
                    } else {
                        darray_mul_val($$, 1.0/$3);
                    }
                }
            }
        |   vexpr '/' vexpr {
                if ($1->size != $3->size) {
	            yyerror("vector lengths mismatch");
                    YYABORT;
                } else {
                    $$ = darray_copy($1);
                    if ($$) {
                        REGISTER_DARR($$);
                        if (darray_div($$, $3) != RETURN_SUCCESS) {
	                    yyerror("divide by zero");
                            YYABORT;
                        }
                    }
                }
            }
        |   vexpr '^' expr {
                double vmin;
                darray_min($1, &vmin);
                if (vmin < 0 && rint($3) != $3) {
                    yyerror("negative value raised to non-integer power");
                    YYABORT;
                } else if (darray_has_zero($1) && $3 <= 0.0) {
                    yyerror("zero raised to non-positive power");
                    YYABORT;
                } else {
                    $$ = darray_copy($1);
                    if ($$) {
                        REGISTER_DARR($$);
                        darray_pow($$, $3);
                    }
                }
            }
	|   '-' vexpr %prec UMINUS {
                $$ = darray_copy($2);
                if ($$) {
                    REGISTER_DARR($$);
                    darray_mul_val($$, -1.0);
                }
            }
	|   '+' vexpr %prec UMINUS { $$ = $2; }
	|   '(' vexpr ')' { $$ = $2; }
        ;

sarray:	    TOK_STRING { $$ = $1; }
	|   TOK_VAR_STR {
                char *s;
                gvar_get_str($1, &s);
                $$ = copy_string(NULL, s);
            }
	;
        
sexpr:      sarray { $$ = $1; }
	|   object '.' TOK_PROP_STR {
                $$ = $3;
            }
	|   sexpr '@' sexpr {
                $$ = concat_strings($1, $3);
                xfree($3);
            }
        |   sarray '[' nexpr TOK_RANGE nexpr ']' {
                if ($3 > $5) {
	            xfree($1);
                    yyerror("non-positive index range");
                    YYABORT;
                } else
                if ($5 >= strlen($1)) {
	            xfree($1);
	            yyerror("index beyond string length");
                    YYABORT;
                } else {
                    unsigned int len = $5 - $3 + 1;
                    $$ = xmalloc(len + 1);
                    if ($$) {
                        memcpy($$, $1 + $3, len);
                        $$[len] = '\0';
                    }
	            xfree($1);
                }
            }
        ;

bexpr:	    TOK_TRUE  { $$ = TRUE; }
	|   TOK_FALSE { $$ = FALSE; }
	|   TOK_VAR_BOOL {
                gvar_get_bool($1, &$$);
            }
	|   object '.' TOK_PROP_BOOL {
                $$ = $3;
            }
        |   expr TOK_EQ expr { $$ = ($1 == $3 ? TRUE:FALSE); }
	|   expr TOK_NE expr { $$ = ($1 != $3 ? TRUE:FALSE); }
	|   expr TOK_LE expr { $$ = ($1 <= $3 ? TRUE:FALSE); }
	|   expr TOK_GE expr { $$ = ($1 >= $3 ? TRUE:FALSE); }
	|   expr '<' expr { $$ = ($1 < $3 ? TRUE:FALSE); }
	|   expr '>' expr { $$ = ($1 > $3 ? TRUE:FALSE); }
	|   bexpr TOK_EQ bexpr { $$ = $1 == $3; }
	|   bexpr TOK_NE bexpr { $$ = $1 != $3; }
	|   bexpr TOK_OR bexpr { $$ = $1 || $3; }
	|   bexpr TOK_AND bexpr { $$ = $1 && $3; }
	|   '!' bexpr %prec UMINUS { $$ = !$2; }
	|   '(' bexpr ')' { $$ = $2; }
	;
        
object:     TOK_OBJECT {
                Graal *g = yyget_extra(scanner);
                $$ = $1;
                g->current_obj = $1;
            }
	|   object ':' { SET_DOTCONTEXT(GContextColumn); } TOK_OBJECT {
                Graal *g = yyget_extra(scanner);
                $$ = $4;
                g->current_obj = $4;
            }
        ;
%%

int graal_parse(Graal *g, const char *s, void *lcontext, void *rcontext)
{
    int retval;
    YY_BUFFER_STATE buffer = yy_scan_string(s, g->scanner);
    
    graal_set_context(g, lcontext, rcontext);
    g->current_obj = lcontext;

    g->dot_context = GContextNone;
    g->RHS         = FALSE;
    
    if (yyparse(g->scanner) == 0) {
        retval = RETURN_SUCCESS;
    } else {
        retval = RETURN_FAILURE;
    }
    
    yy_delete_buffer(buffer, g->scanner);
    
    return retval;
}
