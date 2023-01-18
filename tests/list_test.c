
#include <stdio.h>
#include <inttypes.h>
#include "lisp_types.h"
#include "symtable.h"
#include "lisp_core.h"

int main() {
    init_lisp();

    expr *list = cons_new(q_new(cons_new(int_num_new(1), cons_new(q_new(int_num_new(2)), NIL))), cons_new(int_num_new(2), cons_new(int_num_new(3), NIL)));
    //expr *cons = cons_new(sym_new("cons"), cons_new(int_num_new(2), cons_new(int_num_new(3), NIL)));
    //expr *list = cons_new(sym_new("cons"), cons_new(int_num_new(1), expr_wrap(cons)));
    //list = cons_new(q_new(list), expr_wrap(cons_new(sym_new("cons"), cons_new(int_num_new(5), cons_new(NIL, NIL)))));
    list = cons_new(sym_new("length"), list);

    expr_printnl(list);
    expr *ll = lisp_eval(expr_wrap(list), environ);
    //expr_printnl(l);
    //expr_free(l);
    expr *plus_expr = cons_new(sym_new("+"), cons_new(int_num_new(100),
						      cons_new(int_num_new(2),
							       expr_wrap(cons_new(sym_new("*"),
										  cons_new(int_num_new(250),
											   cons_new(float_num_new(3), NIL)))))));

    expr_printnl(plus_expr);
    expr *l = lisp_eval(expr_wrap(plus_expr), environ);
    expr_printnl(l);

    finalize_lisp();
}
