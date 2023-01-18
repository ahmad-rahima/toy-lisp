
%option noyywrap yylineno nodefault case-insensitive

%{
#include "lisp_types.h"
#include "lisp_parser.h"

/* handle locations */
int yycolumn = 1;

#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno;	\
    yylloc.first_column = yycolumn; yylloc.last_column = yycolumn+yyleng-1; \
    yycolumn += yyleng;
%}

%%

"." |
"'" |
"(" |
")"	{ return *yytext; }

;.*				/* ignore comments */

[0-9]+				{ yylval.e = int_num_new(atoi(yytext)); return INT_NUM; }
[0-9]+"."|[0-9]*"."[0-9]+	{ yylval.e = float_num_new(atof(yytext)); return FLOAT_NUM; }

"t"			{ yylval.e = t_new(); return T; }
"nil"			{ yylval.e = (expr*)0; return N; }

\"(\\\"|[^\"])*\"	{ yytext[yyleng-1] = '\0'; yylval.e = str_new(yytext+1); return STR; }
[A-Za-z0-9&+_*/-]+	{ yylval.e = sym_new(yytext); return SYM; }
[ \n\t]			/* ignore */
.			{ printf("Unknown character: %c\n", *yytext); }
