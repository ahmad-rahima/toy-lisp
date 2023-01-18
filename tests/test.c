#include "lisp_types.h"
#include "symtable.h"

#include <stdio.h>

extern symtable *vartable;
extern symtable *fntable;


int main() {
    //expr *five = int_num_new(5);
    //expr *four = int_num_new(4);
    //expr *three = int_num_new(3);

    //expr *nums = cons_new(three, cons_new(four, cons_new(five, NULL)));
    //expr_printnl(nums);

    //expr *expr_copy(expr *);
    //expr *nums_dup = expr_copy(nums);

    //destroy(nums);

    //expr_printnl(nums_dup);
    //destroy(nums_dup);

    // symtable_put(vartable, "abc", int_num_new(5));


    //symtable *vartable1 = symtable_new(13, vartable);
    //symtable_put(vartable1, "abc", int_num_new(6));



    //init_tables();

    // expr_printnl(sexpr);

    // eval_args(sexpr);
    // expr_printnl(sexpr);

    return 0;
}
