#ifndef __LISP_CORE_H__
#define __LISP_CORE_H__

#include "lisp_types.h"
#include "symtable.h"

/* TODO: add env parameter to each declaration */
extern env *environ;

void init_lisp();
void finalize_lisp();

typedef expr*(lispfn)(expr*, env*);

/* (symbol-value sym) */
lispfn lisp_symbol_value;

/* (symbol-function sym) */
lispfn lisp_symbol_function;

/* (eval expr) */
lispfn lisp_eval;

/* (type-of expr) */
lispfn lisp_type_of;

/* (documentation fn t) */
lispfn lisp_documentation;

/* (car cons) */
lispfn lisp_car;

/* (cdr cons) */
lispfn lisp_cdr;

/* (length cons) */
lispfn lisp_length;

/* (apply fn list) */
lispfn lisp_apply;

/* (quote expr) */
lispfn lisp_quote;

/* (list &rest elt) */
lispfn lisp_list;

/* (cons car cdr) */
lispfn lisp_cons;

lispfn lisp_progn;

lispfn lisp_lambda;

lispfn lisp_defun;

lispfn lisp_defmacro;

lispfn lisp_set;

lispfn lisp_gensym;

lispfn lisp_return_from;

lispfn lisp_block;

lispfn lisp_prin1;

lispfn lisp_equal;

lispfn lisp_not;

lispfn lisp_and;
lispfn lisp_or;
lispfn lisp_if;

/* (fn n &rest ns) */
lispfn lisp_plus_operator;
lispfn lisp_minus_operator;
lispfn lisp_asterisk_operator;
lispfn lisp_backslash_operator;

#endif // __LISP_CORE_H__
