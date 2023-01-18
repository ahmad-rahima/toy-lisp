#include "lisp_types.h"
#include "lisp_core.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


/* define the tables */
env *environ;

#define ASSERT(COND, FMT, ...)			\
    if (!(COND)) {				\
	fprintf(stderr, FMT "\n", ##__VA_ARGS__);	\
	goto error;				\
    }

#define ASSERT_ARGS_NO(E, FN, NO)				       \
    { int l = expr_len(E);                                             \
	ASSERT(l == NO, "function '%s': invalid number of arguments. " \
	   "expected: %d, got: %d.", FN, NO, l); }

env *env_new(int valsz, symtable *valpar, int fnsz, symtable *fnpar) {
    env *en = malloc(sizeof(*en));
    en->val = symtable_new(valsz, valpar);
    en->fn = symtable_new(fnsz, fnpar);

    return en;
}

void env_free(env *en) {
    /* cleanup the tables */
    symtable_destroy(en->fn);
    symtable_destroy(en->val);
}

void init_lisp() {
    /* create env tables */
    environ = env_new(0, NULL, 0, NULL);

    /* fill in the env vars */
    symtable_put(environ->val, "PI", float_num_new(3.141592653589793));

    /* fill in the env functions */
    symtable_put(environ->fn, "QUOTE",       subr_new(lisp_quote,       EXPR_SPECIAL_FORM));
    symtable_put(environ->fn, "PROGN",       subr_new(lisp_progn,       EXPR_SPECIAL_FORM));
    symtable_put(environ->fn, "RETURN-FROM", subr_new(lisp_return_from, EXPR_SPECIAL_FORM));
    symtable_put(environ->fn, "BLOCK",       subr_new(lisp_block,       EXPR_SPECIAL_FORM));
    symtable_put(environ->fn, "LAMBDA",      subr_new(lisp_lambda,      EXPR_SPECIAL_FORM));
    symtable_put(environ->fn, "DEFUN",       subr_new(lisp_defun,       EXPR_SPECIAL_FORM));
    symtable_put(environ->fn, "DEFMACRO",    subr_new(lisp_defmacro,    EXPR_SPECIAL_FORM));
    symtable_put(environ->fn, "IF",          subr_new(lisp_if,          EXPR_SPECIAL_FORM));

    symtable_put(environ->fn, "EVAL",   subr_new(lisp_eval,     EXPR_FUNC));
    symtable_put(environ->fn, "LENGTH", subr_new(lisp_length,	EXPR_FUNC));
    symtable_put(environ->fn, "CAR",    subr_new(lisp_car,	EXPR_FUNC));
    symtable_put(environ->fn, "CDR",    subr_new(lisp_cdr,	EXPR_FUNC));
    symtable_put(environ->fn, "LIST",   subr_new(lisp_list,	EXPR_FUNC));
    symtable_put(environ->fn, "CONS",   subr_new(lisp_cons,	EXPR_FUNC));
    symtable_put(environ->fn, "SET",    subr_new(lisp_set,	EXPR_FUNC));
    symtable_put(environ->fn, "AND",    subr_new(lisp_and,	EXPR_FUNC));
    symtable_put(environ->fn, "OR",     subr_new(lisp_or,	EXPR_FUNC));
    symtable_put(environ->fn, "EQUAL",  subr_new(lisp_equal,	EXPR_FUNC));
    symtable_put(environ->fn, "PRIN1",  subr_new(lisp_prin1,	EXPR_FUNC));
    symtable_put(environ->fn, "NOT",    subr_new(lisp_not,	EXPR_FUNC));

    symtable_put(environ->fn, "+",      subr_new(lisp_plus_operator,      EXPR_FUNC));
    symtable_put(environ->fn, "-",      subr_new(lisp_minus_operator,     EXPR_FUNC));
    symtable_put(environ->fn, "*",      subr_new(lisp_asterisk_operator,  EXPR_FUNC));
    symtable_put(environ->fn, "/",      subr_new(lisp_backslash_operator, EXPR_FUNC));
}

void finalize_lisp() {
    env_free(environ);
}

static expr *cons_eval(expr *, env *);
static expr *get_function(expr *e, env *en) {
    /* <LFN>: (params) body */
    if (expr_len(e) > 1)	return e;

    switch (expr_car(e)->type) {
    case EXPR_CONS:	return cons_eval(expr_copy(e), en); /* <expresssion>: (lambda (params) body) */
    case EXPR_SYM:	return lisp_symbol_function(e, en); /* <FN-NAME>: SYM */
    default:		return e; /* error */
    }
}

/* quote expr */
expr *lisp_quote(expr *e, env *en) {
    /* unwrap one quote */
    expr *res = expr_take(e, 0);

    if (res && res->type == EXPR_CONS) {
	/* check for nested quotes */
	CONS_FOREACH_EXPR(eptr, res) {
	    expr *subexpr = expr_car(eptr);
	    if (subexpr->type == EXPR_CONS
		&& expr_car(subexpr)->type == EXPR_SYM
		&& !strcmp(((sym*)expr_car(subexpr))->value, "QUOTE")) {

		/* NOTE: car subexpr is a "Quote".. thus the following works */
		*expr_car_ref(eptr) = lisp_quote(expr_wrap(subexpr), en);
	    }
	}
    }

    return res;
}

/* prin1 expr */
expr *lisp_prin1(expr *e, env *en) {
    expr *res = expr_unwrap(e);
    expr_printnl(res);

    return res;
}

expr *lisp_symbol_value(expr *e, env *en) {
    expr *target = expr_take(e, 0);
    expr *res = symtable_get(en->val, ((sym*)target)->value);

    /* cleanup */
    expr_free(target);
    return res;
}

expr *lisp_symbol_function(expr *e, env *en) {
    expr *target = expr_take(e, 0);
    expr *res = symtable_get(en->fn, ((sym*)target)->value);

    /* cleanup */
    expr_free(target);
    return res;

}

expr *lisp_function(expr *e, env *en) {
    expr *target = expr_take(e, 0);
    expr *res = target->type != EXPR_CONS
	? symtable_get(en->fn, ((sym*)target)->value)
	/* should not make copy */
	: lisp_eval(expr_wrap(expr_copy(target)), en);

    /* cleanup */
    expr_free(target);
    return res;
}

expr *lisp_set(expr *e, env *en) {
    expr *symbol = NIL, *val = NIL;
    ASSERT_ARGS_NO(e, "set", 2);

    symbol = expr_take(expr_release(&e), 0);
    val    = expr_take(expr_release(&e), 0);

    /* `symtable_put` consumes val */
    symtable_put(en->val, ((sym*)symbol)->value, val);

 error:
    free(symbol);
    return val;			/* MUST not be freed. stored in table! */
}

expr *lisp_car(expr *e, env *en) { return expr_take(expr_take(e, 0), 0); }
expr *lisp_cdr(expr *e, env *en) { return expr_pop(expr_take(e, 0)); }

expr *lisp_length(expr *e, env *en) {
    expr *res = NIL;
    if (!e)		return res;
    ASSERT_ARGS_NO(e, "length", 1);

    e = expr_take(e, 0);
    res = int_num_new(expr_len(e));

    /* cleanup */
error:
    expr_free(e);
    return res;
}

expr *lisp_list(expr *e, env *en) {
    /* return arguments as are */
    return e;
}

/* (cons car cdr) */
expr *lisp_cons(expr *e, env *en) {
    expr *res = NIL;
    ASSERT_ARGS_NO(e, "cons", 2);

    /* first argument */
    expr *car = expr_car(e);
    *expr_car_ref(e) = NIL;

    /* second argument */
    expr *cdr = expr_car(expr_cdr(e));
    *expr_car_ref(*expr_cdr_ref(e)) = NIL;
    *expr_cdr_ref(*expr_cdr_ref(e)) = NIL;

    res = cons_new(car, cdr);

 error:
    expr_free(e);
    return res;
}

/* symbolp s */
expr *lisp_symbolp(expr *e, env *en) {
    expr *car = expr_take(expr_take(e, 0), 0);
    expr *res = NIL;

    if (car->type == EXPR_SYM) res = t_new();

    expr_free(car);
    return res;
}

expr *lisp_lambda(expr *e, env *en) {
    expr *params = expr_unwrap(expr_release(&e));
    expr *body   = e;

    expr *fn = lfn_new(params, body, EXPR_FUNC);

    return fn;
}

#define GENSYM_PREFIX "g"
static int gensym_counter = 0;

expr *lisp_gensym(expr *e, env *en) {
    char s[32];
    snprintf(s, 32, "%s%d", GENSYM_PREFIX, gensym_counter++);

    /* TODO: if not in table */
    return sym_new(s);
}

static inline int is_return_from(expr *e) {
    return e &&
	e->type == EXPR_CONS &&
	expr_car(e)->type == EXPR_SYM &&
	!strcmp(as_sym(expr_car(e))->value, "RETURN-FROM");
}

static inline int is_my_return_from(expr *sym, expr *e) {
    expr *othersym = expr_car(expr_cdr(e));
    if (!sym && !othersym)		return 1;

    return !strcmp(as_sym(sym)->value, as_sym(othersym)->value);
}

/* block sym expr &rest exprs */
expr *lisp_block(expr *e, env *en) {
    expr *sym    = expr_unwrap(expr_release(&e)),
	 *res    = NIL,
	 *prvres = NIL;

    while (e) {
	res = expr_release(&e);

	res = lisp_eval(res, en);
	if (is_return_from(res)) {
	    if (is_my_return_from(sym, res)) {

		if (expr_cdr(expr_cdr(res))) {
		    res = expr_take(res, 2);
		    expr_free(prvres);
		} else {
		    expr_free(res);	/* unwrap value stuffed in `return-from` */
		    res = prvres;
		}
		prvres = NIL;
		break;
	    } else {
		expr_free(prvres);
		break;
	    }
	}

	expr_free(prvres);
	prvres = res;
    }

    return res;
}

static int rising_p(expr *e) {
    return e &&
	(is_return_from(e) || e->type == EXPR_ERR);
}

/* TODO: change this to be (return-from sym [value]) */
/* return-from sym value */
expr *lisp_return_from(expr *e, env *en) {
    return cons_new(sym_new("return-from"), e);
}

expr *lisp_progn(expr *e, env *en) {
    expr *res = NIL;

    while (e) {
	/* free the previous result */
	expr_free(res);

	expr *cur = expr_release(&e);

	/* `lisp_eval` consumes cur */
	res = lisp_eval(cur, en);
    }

    return res;
}

expr *lisp_defun(expr *e, env *en) {
    expr *symbol = NIL, *val = NIL;

    symbol       = expr_unwrap(expr_release(&e));
    expr *params = expr_release(&e);
    expr *body   = cons_new(sym_new("block"), cons_new(expr_copy(symbol), e));
    val          = cons_new(expr_unwrap(params), expr_wrap(body));
    val          = lisp_lambda(val, en);

    /* `symtable_put` consumes val */
    symtable_put(en->fn, ((sym*)symbol)->value, val);

 error:
    expr_free(symbol);
    return val;			/* MUST not be freed. stored in table! */
}


/* equal expr expr */
expr *lisp_equal(expr *e, env *en) {
    expr *fst = expr_unwrap(expr_release(&e));
    expr *snd = expr_unwrap(expr_release(&e));
    expr *res = expr_t_ize(expr_equal(fst, snd));

 error:
    expr_free(fst);
    expr_free(snd);
    return res;
}

/* not expr */
expr *lisp_not(expr *e, env *en) {
    expr *fst = expr_unwrap(expr_release(&e));
    expr *res = expr_t_ize(!fst);

 error:
    expr_free(fst);
    return res;
}


/* TODO: make _eval expression lispfn! e must be a list!! */
static inline expr *sym_eval(expr *e, env *en) { return lisp_symbol_value(expr_wrap(e), en); }
static inline expr *self_evaluating_eval(expr *e, env *en) { return e; }

static expr *cons_eval(expr *e, env *en) { /* 1 arg */
    expr *res = NIL;
    expr *car = expr_take(e, 0);
    /* TODO: handle if s is cons as lambda ! */
    expr *s = expr_take(expr_release(&car), 0);

    /* BUG: we should not copy s memleak! */
    expr *fn = get_function(expr_wrap(expr_copy(s)), en);

    /* BUG */
    //if (!fn)	return NIL;

    switch (fn->type) {
    case EXPR_FUNC:
	/* eval the arguments */
	CONS_FOREACH_EXPR(eptr, car) {
	    /* release is better here */
	    expr *arg = expr_wrap(expr_car(eptr));
	    *expr_car_ref(eptr) = lisp_eval(arg, en);
	    if (rising_p(expr_car(eptr))) {
		res = expr_car(eptr);
		*expr_car_ref(eptr) = NIL;
		goto error;
	    }
	}
    case EXPR_MACRO:		/* fall through */
    case EXPR_SPECIAL_FORM:
	/* then eval the function */
	res = lisp_apply(cons_new(fn, expr_wrap(car)), en);

	/* if macro then evaluate twice */
	if (fn->type == EXPR_MACRO)	res = cons_eval(expr_wrap(res), en);
    }
    return res;

 error:
    expr_free(fn);
    expr_free(car);
    return res ? res : err_new("eval: unbound symbol!");
}

static inline expr *abstract_func_eval(expr *e, env *en) {
    return err_new("eval: should not be able to eval function..");
}

typedef expr*(*eval_fn_t)(expr*, env*);
const eval_fn_t lisp_eval_fn[] = {
    self_evaluating_eval,
    sym_eval,
    self_evaluating_eval,
    self_evaluating_eval,
    self_evaluating_eval,
    cons_eval,
    self_evaluating_eval,
    abstract_func_eval,
    abstract_func_eval,
    abstract_func_eval
};

expr *lisp_eval(expr *e, env *en) {
    if (!e || !expr_car(e))	return NIL;
    if (expr_car(e)->type != EXPR_CONS)	e = expr_take(e, 0);

    return (e->type < EXPR_END)
	? lisp_eval_fn[e->type](e, en)
	: err_new("eval: unknown type!");
}

/* apply fn args */
expr *lisp_apply(expr *e, env *en) { /* 2 args */
    expr *res = NIL;
    ASSERT_ARGS_NO(e, "apply", 2);

    expr *symcons = expr_unwrap(expr_release(&e));
    e = expr_take(e, 0);

    /* car(symcons) could be CONS /lambda expression/, SYM /fn name/, subr/func */
    expr *fn = get_function(symcons, en);

    //ASSERT(fn, "unbound symbol!");
    if (!fn)	goto error;

    if (expr_subr_p(fn)) {
	subr *s = (subr*)fn;
	res = s->fn(e, en);
    } else {
	lfn* f = (lfn*)expr_copy(fn);

	/* make a function local env */
	env *fen = env_new(23, en->val, 23, en->fn);

	/* assign args */
	while (e) {
	    expr *param = expr_release(&f->params);

	    /* if next param is &rest */
	    if (!strcmp(as_sym(expr_car(param))->value, "&REST")) {
		param = expr_release(&f->params);
		symtable_put(fen->val, as_sym(expr_car(param))->value, e);
		expr_free(param);
		break;
	    }

	    expr *arg   = expr_unwrap(expr_release(&e));
	    symtable_put(fen->val, as_sym(expr_car(param))->value, arg);

	    expr_free(param);
	    /* we still need arg */
	}

	/* now call the function */
	res = lisp_progn(f->body, fen);
	res = expr_copy(res);	/* free res out of fen! */

	env_free(fen);
    }

error:
    expr_free(fn);
    return res;
}

/* numeric operations */
/* f n0 &rest ns */
expr *lisp_f_operator(expr *e, env *en, expr*(*op)(expr *n0, expr*n1)) {
    expr *res = expr_unwrap(expr_release(&e));

    while (e)		res = op(res, expr_unwrap(expr_release(&e)));

    return res;
}

/* binary numeric operations; +, -, *, / */
/* TODO: those functions should be `static` */
#define DEFINE_EXPR_BINARY_NUM_OP(OP_NAME, OP_SYM) \
    static expr *expr_binary_ ##OP_NAME(expr *n0, expr *n1) {       \
    union { int i; double d; } res;                                             \
    if (n0->type == EXPR_INT_NUM && n1->type == EXPR_FLOAT_NUM) {               \
	res.d = ((int_num*)n0)->value OP_SYM ((float_num*)n1)->value;           \
    } else if (n0->type == EXPR_FLOAT_NUM && n1->type == EXPR_INT_NUM) {        \
	res.d = ((float_num*)n0)->value OP_SYM ((int_num*)n1)->value;           \
    } else if (n0->type == EXPR_FLOAT_NUM && n1->type == EXPR_FLOAT_NUM) {      \
	res.d = ((float_num*)n0)->value OP_SYM ((float_num*)n1)->value;         \
    } else {                                                                    \
	res.i = ((int_num*)n0)->value OP_SYM ((int_num*)n1)->value;		\
    }                                                                           \
    if (n0->type == EXPR_INT_NUM && n1->type == EXPR_INT_NUM)                   \
        { expr_free(n0); expr_free(n1); return int_num_new(res.i); }		\
    else                                                                        \
        { expr_free(n0); expr_free(n1); return float_num_new(res.d); } }


DEFINE_EXPR_BINARY_NUM_OP(plus, +)
DEFINE_EXPR_BINARY_NUM_OP(sub, -)
DEFINE_EXPR_BINARY_NUM_OP(mul, *)
DEFINE_EXPR_BINARY_NUM_OP(div, /)

expr *lisp_plus_operator(expr *e, env *en) {
    return lisp_f_operator(e, en, expr_binary_plus);
}

expr *lisp_minus_operator(expr *e, env *en) {
    return lisp_f_operator(e, en, expr_binary_sub);
}

expr *lisp_asterisk_operator(expr *e, env *en) {
    return lisp_f_operator(e, en, expr_binary_mul);
}

expr *lisp_backslash_operator(expr *e, env *en) {
    return lisp_f_operator(e, en, expr_binary_div);
}


/* Logical Operations */
/* and expr &rest exprs */
expr *lisp_and(expr *e, env *en) {
    expr *res = NIL;
    while (e) {
	res = expr_unwrap(expr_release(&e));

	if (!res)	goto final;
    }

 final:
    expr_free(e);		/* if e consumed, then its NIL */
    return res;
}

/* or expr &rest exprs */
expr *lisp_or(expr *e, env *en) {
    expr *res = NIL;
    while (e) {
	res = expr_unwrap(expr_release(&e));

	if (res)	goto final;
    }

 final:
    expr_free(e);
    return res;
}

/* if cond then-part &rest else-part */
expr *lisp_if(expr *e, env *en) {
    // ASSERT_ARGS_MIN_NO(e, "if", 2);
    // ASSERT_ARGS_MAX_NO(e, "if", 2);

    expr *res      = NIL;
    expr *cond     = expr_release(&e);
    expr *thenpart = expr_release(&e);
    expr *elsepart = e;

    cond = lisp_eval(cond, en);
    if (cond) {
	res = lisp_eval(thenpart, en);
	thenpart = NIL;
    } else if (elsepart) {	/* else-part is *optional* */
	res = lisp_progn(elsepart, en);
	elsepart = NIL;
    }

 error:
    expr_free(thenpart);
    expr_free(elsepart);
    expr_free(cond);
    return res;
}

/* defmacro symbol params body */
expr *lisp_defmacro(expr *e, env *en) {
    expr *symbol = expr_unwrap(expr_release(&e));
    expr *params = expr_unwrap(expr_release(&e));
    expr *body   = e;

    expr *fn = lfn_new(params, body, EXPR_MACRO);
    /* `symtable_put` consumes val */
    symtable_put(en->fn, ((sym*)symbol)->value, fn);

 error:
    expr_free(symbol);
    return fn;
}
