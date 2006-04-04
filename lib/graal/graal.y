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

#include <string.h>

#include "grace/baseP.h"
#include "grace/graalP.h"

#include "parser.h"
#include "scanner.h"

#define YYPARSE_PARAM scanner
#define YYLEX_PARAM   scanner

#define REGISTER_DARR(da)   graal_register_darr(yyget_extra(scanner), da)

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

%token        TOK_PROP_NIL
%token <dval> TOK_PROP_NUM
%token <sval> TOK_PROP_STR
%token <darr> TOK_PROP_ARR

%token <gvar> TOK_VAR_NIL
%token <gvar> TOK_VAR_NUM
%token <gvar> TOK_VAR_STR
%token <gvar> TOK_VAR_ARR

%type <dval> expr
%type <ival> iexpr
%type <ival> nexpr
%type <darr> list
%type <darr> array
%type <darr> vexpr
%type <gobj> object
%type <sval> sexpr

/* Precedence */
%left '-' '+' '@'
%left '*' '/' '%'
%nonassoc UMINUS

%%
input:	/* empty */
	|   input line { graal_free_darrs(yyget_extra(scanner)); }
	;

line:	'\n'
	|   TOK_NAME '=' expr '\n' {
                GVar *var = graal_get_var(yyget_extra(scanner), $1, TRUE);
                xfree($1);
                if (gvar_set_num(var, $3) != RETURN_SUCCESS) {
	            yyerror("assignment failed");
                    return 1;
                }
            }
	|   TOK_VAR_NUM '=' expr '\n' {
                gvar_set_num($1, $3);
            }
	|   TOK_VAR_NUM '+' '=' expr '\n' {
                double val;
                gvar_get_num($1, &val);
                gvar_set_num($1, val + $4);
            }
	|   TOK_VAR_NUM '-' '=' expr '\n' {
                double val;
                gvar_get_num($1, &val);
                gvar_set_num($1, val - $4);
            }
	|   TOK_VAR_NUM '*' '=' expr '\n' {
                double val;
                gvar_get_num($1, &val);
                gvar_set_num($1, val*$4);
            }
	|   TOK_VAR_NUM '/' '=' expr '\n' {
                if ($4 == 0) {
	            yyerror("divide by zero");
                    return 1;
                } else {
                    double val;
                    gvar_get_num($1, &val);
                    gvar_set_num($1, val/$4);
                }
            }
	|   TOK_NAME '=' vexpr '\n' {
                GVar *var = graal_get_var(yyget_extra(scanner), $1, TRUE);
                xfree($1);
                if (gvar_set_arr(var, $3) != RETURN_SUCCESS) {
	            yyerror("assignment failed");
                    return 1;
                }
            }
	|   TOK_VAR_ARR '=' vexpr '\n' {
                gvar_set_arr($1, $3);
            }
	|   TOK_VAR_ARR '=' expr '\n' {
                gvar_set_num($1, $3);
            }
	|   TOK_VAR_ARR '[' nexpr ']' '=' expr '\n' {
                DArray *da;
                gvar_get_arr($1, &da);
                if ($3 < da->size) {
                    da->x[$3] = $6;
                } else {
	            yyerror("index beyond array bounds");
                    return 1;
                }
            }
	|   object '.' TOK_NAME '=' expr '\n' {
                Graal *g = yyget_extra(scanner);
                GVarData prop;
                prop.num = $5;
                if (graal_set_user_obj_prop(g, $1, $3, GVarNum, prop) !=
                    RETURN_SUCCESS) {
	            yyerror("assignment failed");
                }
                xfree($3);
            }
	|   object '.' TOK_NAME '=' vexpr '\n' {
                Graal *g = yyget_extra(scanner);
                GVarData prop;
                prop.arr = $5;
                if (graal_set_user_obj_prop(g, $1, $3, GVarArr, prop) !=
                    RETURN_SUCCESS) {
	            yyerror("assignment failed");
                }
                xfree($3);
            }
	|   TOK_NAME '=' sexpr '\n' {
                GVar *var = graal_get_var(yyget_extra(scanner), $1, TRUE);
                xfree($1);
                if (gvar_set_str(var, $3) != RETURN_SUCCESS) {
	            yyerror("assignment failed");
                    return 1;
                }
                xfree($3);
            }
	|   TOK_VAR_STR '=' sexpr '\n' {
                if (gvar_set_str($1, $3) != RETURN_SUCCESS) {
                    xfree($3);
	            yyerror("assignment failed");
                    return 1;
                } else {
                    xfree($3);
                }
            }
	|   eval_anon expr '\n' {
                GVarData vardata;
                vardata.num = $2;
                graal_call_eval_proc(yyget_extra(scanner), GVarNum, vardata);
            }
	|   eval_anon vexpr '\n' {
                GVarData vardata;
                vardata.arr = $2;
                graal_call_eval_proc(yyget_extra(scanner), GVarArr, vardata);
            }
	|   eval_anon sexpr '\n' {
                GVarData vardata;
                vardata.str = $2;
                graal_call_eval_proc(yyget_extra(scanner), GVarStr, vardata);
                xfree($2);
            }
	;

eval_anon:  '?' { graal_set_RHS(yyget_extra(scanner), TRUE); }
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
                    return 1;
                } else {
	            $$ = $1 / $3;
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
                    return 1;
                }
            }
	;

iexpr:	    expr {
                int itmp = rint($1);
                if (fabs(itmp - $1) > 1.e-6) {
                    yyerror("Non-integer value supplied for integer");
                    return 1;
                } else {
                    $$ = itmp;
                }
	    }
	|   iexpr '%' iexpr {
                if ($3 == 0) {
	            yyerror("divide by zero");
                    return 1;
                } else {
                    $$ = (int) rint($1) % (int) rint($3);
                }
            }
        ;

nexpr:	    iexpr {
                if ($1 < 0) {
                    yyerror("Negative value supplied for non-negative");
                    return 1;
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
        |   vexpr '+' expr {
                $$ = darray_copy($1);
                if ($$) {
                    REGISTER_DARR($$);
                    darray_add_val($$, $3);
                }
            }
        |   vexpr '-' expr {
                $$ = darray_copy($1);
                if ($$) {
                    REGISTER_DARR($$);
                    darray_add_val($$, -$3);
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
        |   vexpr '/' expr {
                $$ = darray_copy($1);
                if ($$) {
                    REGISTER_DARR($$);
                    if ($3 == 0.0) {
	                yyerror("divide by zero");
                        return 1;
                    } else {
                        darray_mul_val($$, 1.0/$3);
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

sexpr:      TOK_STRING { $$ = $1; }
	|   TOK_VAR_STR {
                char *s;
                gvar_get_str($1, &s);
                $$ = copy_string(NULL, s);
            }
	|   object '.' TOK_PROP_STR {
                $$ = $3;
            }
	|   sexpr '@' sexpr {
                $$ = concat_strings($1, $3);
                xfree($3);
            }
        ;

object:     TOK_OBJECT {
                Graal *g = yyget_extra(scanner);
                $$ = $1;
                g->current_obj = $1;
            }
	|   object ':' TOK_OBJECT {
                Graal *g = yyget_extra(scanner);
                $$ = $3;
                g->current_obj = $3;
            }
        ;
%%

int graal_parse(Graal *g, const char *s, void *context)
{
    int retval;
    YY_BUFFER_STATE buffer = yy_scan_string(s, g->scanner);
    
    g->default_obj = context;
    g->current_obj = context;

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
