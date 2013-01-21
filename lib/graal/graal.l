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

#include "grace/graalP.h"
#include "parser.h"

/* FIXME!!! */
char string_buf[512];
char *string_buf_ptr;
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
WS      [ \t]
DIGIT   [0-9]
NAME    [A-Za-z$][A-Za-z0-9_]*

/* exclusive state for quoted strings */
%x QSTR

%%
(({DIGIT}+|({DIGIT}*\.{DIGIT}+))([eE][-+]?{DIGIT}+)?) {
	    graal_set_dotcontext(yyextra, GContextNone);
            yylval->dval = atof(yytext);
	    return TOK_NUMBER;
	}

"this"	{
            Graal *g = (Graal *) yyextra;
            void *default_obj = graal_get_context(g);
            if (default_obj) {
                g->current_obj = default_obj;
                yylval->gobj = default_obj;
                return TOK_OBJECT;
            } else {
                REJECT;
            }
	}

"true"	{ return TOK_TRUE; }
"false"	{ return TOK_FALSE; }

"length" {
            if (graal_get_dotcontext(yyextra) != GContextNone) {
                REJECT;
            } else {
                return TOK_LENGTH;
            }
	}

"if"	{ return TOK_IF; }
"else"	{ return TOK_ELSE; }

{NAME}	{
            void *uobj;
            void *defobject;
            GVar *var;
            Graal *g = (Graal *) yyextra;
	    
            switch (graal_get_dotcontext(g)) {
            case GContextDot:
                graal_set_dotcontext(g, GContextNone);
            
                if (graal_get_RHS(g)) {
                    GVarData prop;
                    GVarType type;
                    type = graal_get_user_obj_prop(g, g->current_obj, yytext, &prop);
                    switch (type) {
                    case GVarNum:
                        yylval->dval = prop.num;
                        return TOK_PROP_NUM;
                        break;
                    case GVarBool:
                        yylval->ival = prop.bool;
                        return TOK_PROP_BOOL;
                        break;
                    case GVarArr:
                        yylval->darr = prop.arr;
                        return TOK_PROP_ARR;
                        break;
                    case GVarStr:
                        yylval->sval = prop.str;
                        return TOK_PROP_STR;
                        break;
                    default:
                        return TOK_PROP_NIL;
                        break;
                    }
                } else {
                    yylval->sval = copy_string(NULL, yytext);
                    return TOK_NAME;
                }
                break;

            case GContextColumn:
                graal_set_dotcontext(g, GContextNone);

                uobj = graal_get_user_obj(g, g->current_obj, yytext);
                if (uobj) {
                    yylval->gobj = uobj;
                    return TOK_OBJECT;
                } else {
                    yylval->sval = copy_string(NULL, yytext);
                    return TOK_NAME;
                }
                break;
            
            default:
                defobject = graal_get_context(g);
                uobj = graal_get_user_obj(g, defobject, yytext);
	        if (uobj) {
                    yylval->gobj = uobj;
                    return TOK_OBJECT;
                }
                
                var = graal_get_var(g, yytext, FALSE);
	        if (var) {
                    yylval->gvar = var;
                    switch (var->type) {
                    case GVarNum:
                        return TOK_VAR_NUM;
                        break;
                    case GVarBool:
                        return TOK_VAR_BOOL;
                        break;
                    case GVarStr:
                        return TOK_VAR_STR;
                        break;
                    case GVarArr:
                        return TOK_VAR_ARR;
                        break;
                    default:
                        return TOK_VAR_NIL;
                        break;
                    }
                } else {
                    yylval->sval = copy_string(NULL, yytext);
                    return TOK_NAME;
                }
                break;
            }
	}

{WS}	; /* ignore white space */

".."	{ return TOK_RANGE; }

"=="	{ return TOK_EQ; }
"!="	{ return TOK_NE; }
">="	{ return TOK_GE; }
"<="	{ return TOK_LE; }

"||"	{ return TOK_OR; }
"&&"	{ return TOK_AND; }

\.	{
	    graal_set_dotcontext(yyextra, GContextDot);
            return yytext[0];
	}
=	{
	    graal_set_RHS(yyextra, TRUE);
            return yytext[0];
	}
;	{
	    graal_set_RHS(yyextra, FALSE);
            return yytext[0];
	}
\"	string_buf_ptr = string_buf; BEGIN(QSTR);

\n	; /* ignore EOL */
.	{
	    graal_set_dotcontext(yyextra, GContextNone);
            return yytext[0];
	}

<QSTR>{
\"	{ /* saw closing quote - all done */
            BEGIN(INITIAL);
            *string_buf_ptr = '\0';
            yylval->sval = copy_string(NULL, string_buf);
            return TOK_STRING;
        }

\n	{
            /* error - unterminated string constant */
            BEGIN(INITIAL);
            /* generate error message */
	}
\\n	*string_buf_ptr++ = '\n';
\\t	*string_buf_ptr++ = '\t';
\\r	*string_buf_ptr++ = '\r';
\\b	*string_buf_ptr++ = '\b';
\\f	*string_buf_ptr++ = '\f';

\\(.|\n)	*string_buf_ptr++ = yytext[1];

[^\\\n\"]+	{
        char *yptr = yytext;

        while (*yptr)
            *string_buf_ptr++ = *yptr++;
        }
}

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
