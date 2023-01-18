#include <stdio.h>
#include <inttypes.h>
#include "lisp_types.h"
#include "symtable.h"
#include "lisp_core.h"

int main() {
    init_lisp();

    //expr *e = cons_new(sym_new("progn"), cons_new(cons_new(sym_new("set"), cons_new(cons_new(sym_new("quote"), cons_new(sym_new("x"), NIL)), cons_new(int_num_new(5), NIL))), NIL));
    //expr_printnl(e);
    //e = lisp_eval(expr_wrap(e), environ);
    //expr_printnl(e);

    //expr *s = lisp_gensym(NIL, NULL);
    //expr_printnl(s);
    //s = lisp_gensym(NIL, NULL);

    //expr *final_return = cons_new(sym_new("return-from"), cons_new(sym_new("my-first-block"), cons_new(int_num_new(1), NIL)));
    //expr *s = cons_new(cons_new(sym_new("set"), cons_new(cons_new(sym_new("quote"), cons_new(sym_new("x"), NIL)), cons_new(final_return, NIL))), NIL);
    //expr *blk = cons_new(sym_new("block"), cons_new(sym_new("my-first-block"), cons_new(int_num_new(1), s)));
    //expr_printnl(blk);
    //expr *res = lisp_eval(expr_wrap(blk), environ);
    //expr_printnl(res);

    expr *opr = cons_new(sym_new("*"), cons_new(int_num_new(5), cons_new(int_num_new(3), NIL)));
    expr *lmbd = cons_new(sym_new("lambda"), cons_new(cons_new(sym_new("x"), NIL), expr_wrap(cons_new(sym_new("+"), cons_new(sym_new("x"), cons_new(float_num_new(3.5), expr_wrap(opr)))))));
    expr *full_lmbd = cons_new(lmbd, cons_new(int_num_new(4), NIL));
    expr_printnl(full_lmbd);

    expr *e = lisp_eval(expr_wrap(full_lmbd), environ);
    expr_printnl(e);
    expr_free(e);

    //expr *opr = cons_new(sym_new("*"), cons_new(int_num_new(5), cons_new(int_num_new(3), NIL)));
    //expr *defunexpr = cons_new(sym_new("defun"), cons_new(sym_new("foo"), cons_new(cons_new(sym_new("x"), NIL), expr_wrap(cons_new(sym_new("+"), cons_new(sym_new("x"), cons_new(float_num_new(3.5), expr_wrap(opr))))))));
    //expr_printnl(defunexpr);
    //expr *e = lisp_eval(expr_wrap(defunexpr), environ);
    //expr_printnl(e);


    finalize_lisp();
}
