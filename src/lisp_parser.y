
%{
#include "lisp_types.h"
#include "lisp_core.h"
#include "utils.h"
#include <stdio.h>

/* interface to the lexer */
int yylex();

extern int interactive_usage;
%}

%locations

%union {
    struct expr *e;
}


%token	<e>		SYM STR INT_NUM FLOAT_NUM EOL T N
%type	<e>		constant non_qexpr expr exprlist

%start start_exprlist

%%

constant
	:	INT_NUM
	|	FLOAT_NUM
	|       T
	|	N
	;

non_qexpr
	:	STR
	|	SYM
	|	constant
	|	'(' exprlist ')'
		{ $$ = $2; }
	;

expr
	:	'\'' expr
		{ $$ = q_new($2); }
//	|	'\'' '(' expr '.' expr ')'
//		{ $$ = cons_new($3, $5); }
	|	non_qexpr
		{ $$ = $1; }
	;

exprlist
	:	/* ε */
		{ $$ = NULL; }
	|	expr exprlist
		{ $$ = cons_new($1, $2); };
	;

start_exprlist
	:	/* ε */
		{ if (interactive_usage) printf("> "); }
	|	start_exprlist error
		{ /* yyerrok; */ if (interactive_usage) puts("\n> "); }  // yyerrok is creating an infinite loop
	|	start_exprlist expr
		{
		  expr *e = cons_new($2, NULL);
		  expr_printnl(e); /* should be added under interactive_usage*/
		  e = lisp_eval(e, environ);
		  expr_printnl(e);

		  if (interactive_usage)
		    printf("> ");
		}
	;
