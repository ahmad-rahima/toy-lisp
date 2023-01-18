//#include <stdarg.h>
//#include <stdio.h>
//#include <stdlib.h>
//
//#include "symtable.h"
//#include "lisp_types.h"
//#include "lisp_core.h"
//#include "lisp_parser.h"
//
//extern symtable *vartable;
//extern symtable *fntable;
//
//extern FILE *yyin;
//extern YYLTYPE yylloc;
//
//int interactive_usage = 1;
//
//void yyerror(const char *fmt, ...) {
//    va_list ap;
//    va_start(ap, fmt);
//
//    if (yylloc.first_line)
//	fprintf(stderr, "%d.%d-%d.%d: error: ", yylloc.first_line, yylloc.first_column,
//		yylloc.last_line, yylloc.last_column);
//
//    vfprintf(stderr, fmt, ap);
//    fprintf(stderr, "\n");
//    /* cleanup */
//    va_end(ap);
//}
//
//int yyparse();
//int main(int argc, char *argv[]) {
//    init_lisp();
//
//    if (argc > 1) {
//	const char *filename = argv[1];
//	FILE *infile = fopen(filename, "r");
//	if (!infile) {
//	    yyerror("file not found.");
//	    exit(1);
//	}
//
//	interactive_usage = 0;
//	yyin = infile;
//    }
//
//    yyparse();
//
//    // finalize_lisp();
//    return 0;
//}
