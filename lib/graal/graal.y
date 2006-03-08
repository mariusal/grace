%{
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
    GVar   *gvar;
}

%token <dval> TOK_NUMBER
%token <sval> TOK_STRING
%token <gvar> TOK_VAR_NIL
%token <gvar> TOK_VAR_NUM
%token <gvar> TOK_VAR_ARR
%left '-' '+'
%left '*' '/'
%nonassoc UMINUS

%type <dval> expr
%type <ival> iexpr
%type <ival> nexpr
%type <darr> list
%type <darr> array
%type <darr> vexpr

%%
input:	/* empty */
	|   input line { graal_free_darrs(yyget_extra(scanner)); }
	;

line:	'\n'
	|   TOK_VAR_NIL '=' expr '\n' {
                gvar_set_num($1, $3);
            }
	|   TOK_VAR_NUM '=' expr '\n' {
                gvar_set_num($1, $3);
            }
	|   TOK_VAR_NIL '=' vexpr '\n' {
                gvar_set_arr($1, $3);
            }
	|   TOK_VAR_ARR '=' vexpr '\n' {
                gvar_set_arr($1, $3);
            }
	|   TOK_VAR_NUM '\n' {
                double value;
                gvar_get_num($1, &value);
                printf("\t%g\n", value);
            }
	|   TOK_VAR_ARR '\n' {
                unsigned int i;
                DArray *da;
                gvar_get_arr($1, &da);
                printf("\t{");
                for (i = 0; da && i < da->size; i++) {
                    printf(" %g ", da->x[i]);
                }
                printf("}\n");
            }
	;

expr:	    TOK_NUMBER { $$ = $1; }
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
	|   '-' expr %prec UMINUS   { $$ = -$2; }
	|   '(' expr ')' { $$ = $2; }
	|   TOK_VAR_NUM {
                gvar_get_num($1, &$$);
            }
	|   TOK_VAR_ARR '[' nexpr ']' {
                DArray *da;
                gvar_get_arr($1, &da);
                if (darray_get_val(da, $3, &$$) != RETURN_SUCCESS) {
	            yyerror("index beyond array length");
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
	|   TOK_VAR_ARR {
                DArray *da;
                gvar_get_arr($1, &da);
                $$ = darray_copy(da);
                if ($$) {
                    REGISTER_DARR($$);
                }
            }
        ;
%%

int graal_parse(Graal *g, const char *s)
{
    int retval;
    YY_BUFFER_STATE buffer = yy_scan_string(s, g->scanner);
    
    if (yyparse(g->scanner) == 0) {
        retval = RETURN_SUCCESS;
    } else {
        retval = RETURN_FAILURE;
    }
    
    yy_delete_buffer(buffer, g->scanner);
    
    return retval;
}
