%{
#include "grace/graalP.h"
#include "parser.h"
%}

/* joint work with bison */
%option bison-bridge
%option bison-locations

/* build reentrant scanner */
%option reentrant

%option always-interactive

/* eliminate unused boilerplate and stop compiler diagnostics */
%option noyyget_lineno
%option noyyset_lineno
%option noyy_top_state
%option nounput
%option noyywrap

/* simple definitions */
DIGIT    [0-9]

%%
(({DIGIT}+|({DIGIT}*\.{DIGIT}+))([eE][-+]?{DIGIT}+)?) {
	    yylval->dval = atof(yytext);
	    return TOK_NUMBER;
	}

[ \t]	; /* ignore white space */

[A-Za-z$][A-Za-z0-9_]* {
            yylval->gvar = graal_get_var(yyextra, yytext);
	    switch (yylval->gvar->type) {
            case GVarNum:
                return TOK_VAR_NUM;
                break;
            case GVarArr:
                return TOK_VAR_ARR;
                break;
            default:
                return TOK_VAR_NIL;
                break;
            }
	}

\n	|
.	return yytext[0];
%%

void graal_scanner_init(Graal *g)
{
    yyscan_t scanner;
    
    yylex_init(&scanner);
    g->scanner = scanner;
    
    yyset_extra(g, scanner);
}

void graal_scanner_delete(Graal *g)
{
    yylex_destroy(g->scanner);
}
