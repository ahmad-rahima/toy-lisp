#include "lisp_types.h"
#include "lisp_core.h"
#include "symtable.h"
#include "utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>


const char *expr_type_str(int8_t type) {
    switch (type) {
    case EXPR_ERR:		return "Error";
    case EXPR_SYM:		return "Sym";
    case EXPR_INT_NUM:		return "Integer";
    case EXPR_FLOAT_NUM:	return "Floating-Point";
    case EXPR_STR:		return "String";
    case EXPR_CONS:		return "Cons";
    case EXPR_SPECIAL_FORM:	return "Special Form";
    case EXPR_FUNC:		return "Function";
    case EXPR_MACRO:		return "Macro";

    default:
	return "Unknown type.";
    }
}

static inline char *upperstr(const char *s) {
    /* returns new allocated string that is uppercased */
    char *res = malloc(strlen(s)+1);
    char *resptr = res;

    while (*s)	*resptr++ = toupper(*s++);
    *resptr = '\0';

    return res;
}

expr *sym_new(const char *s)  {
    sym *e = malloc(sizeof(*e));
    e->refcount = 0;
    e->type = EXPR_SYM;
    e->value = upperstr(s);

    return (expr*)e;
}

expr *int_num_new(int n) {
    int_num *e = malloc(sizeof(*e));
    e->refcount = 0;
    e->type = EXPR_INT_NUM;
    e->value = n;

    return (expr*)e;
}

expr *float_num_new(double d) {
    float_num *e = malloc(sizeof(*e));
    e->refcount = 0;
    e->type = EXPR_FLOAT_NUM;
    e->value = d;

    return (expr*)e;
}

expr *str_new(const char *s) {
    str *e = malloc(sizeof(*e));
    e->refcount = 0;
    e->type = EXPR_STR;
    e->value = malloc(strlen(s)+1);
    strcpy(e->value, s);

    return (expr*)e;
}

expr *err_new(const char *fmt , ...)  {
    err *e = malloc(sizeof(*e));
    e->refcount = 0;
    e->type = EXPR_ERR;
    e->value = malloc(ERR_MSG_MAX_LEN);

    va_list ap;
    va_start(ap, fmt);
    vsprintf(e->value, fmt, ap);

    /* shrink to fit */
    e->value = realloc(e->value, strlen(e->value)+1);

    /* cleanup */
    va_end(ap);
    return (expr*)e;
}

expr *cons_new(expr *car, expr *cdr) {
    cons *e = malloc(sizeof(*e));
    e->refcount = 0;
    e->type = EXPR_CONS;
    e->car = car;
    e->cdr = cdr;

    return (expr*)e;
}

expr *q_new(expr *value) {
    cons *e = malloc(sizeof(*e));
    e->refcount = 0;
    e->type = EXPR_CONS;
    e->car = sym_new("quote");
    e->cdr = cons_new(value, NIL);

    return (expr*)e;
}

expr *t_new() {
    expr *e = malloc(sizeof(*e));
    e->refcount = 0;
    e->type = EXPR_T;

    return (expr*)e;
}

expr *subr_new(expr *(*fn)(expr*, env*), int type) {
    subr *e = malloc(sizeof(*e));
    e->refcount = 0;
    e->type = type;
    e->subr_p = 1;
    e->fn = fn;

    return (expr*)e;
}

expr *lfn_new(expr *params, expr *body, int type) {
    lfn *e = malloc(sizeof(*e));
    e->refcount = 0;
    e->type = type;		/* MACRO or FUNC */
    e->subr_p = 0;
    e->params = params;
    e->body = body;

    return (expr*)e;
}

int expr_len(expr *e) {
    /* TODO: expect one argument */
    int i = 0;
    CONS_FOREACH_EXPR(e1, e)	++i;

    return i;
}

void expr_incr_refcount(expr *e) {
    if (!e)	return;
    if (e->type == EXPR_CONS) {
	expr_incr_refcount(expr_car(e));
	expr_incr_refcount(expr_cdr(e));
    }

    e->refcount++;
}

void expr_decr_refcount(expr *e) {
    if (!e)	return;
    if (e->type == EXPR_CONS) {
	expr_decr_refcount(expr_car(e));
	expr_decr_refcount(expr_cdr(e));
    }

    if (e->refcount)	e->refcount--;
}

expr *expr_take(expr *e, int idx) {
    if (refcount(e))	{
	CONS_FOREACH_EXPR(eptr, e)	{
	    if (!idx--)		return expr_car(eptr);
	}
    }

    expr *res = NIL;
    expr *prv = cons_new(NIL, NIL); /* dummmy value to avoid segfault */
    //idx++;


    CONS_FOREACH_EXPR(eptr, e) {
	/* preserve the target */
	if (!idx--) {
	    res = expr_car(eptr);
	    /* release current value from the cons */
	    *expr_car_ref(eptr) = NIL;
	}

	*expr_cdr_ref(prv) = eptr;
    }
    expr_free(prv);

    return res;
}

expr *expr_pop(expr *e) {
    if (refcount(e))	return expr_cdr(e);

    expr *nxt = expr_cdr(e);
    *expr_cdr_ref(e) = NIL;

    expr_free(e);
    return nxt;
}

expr *expr_release(expr **e) {
    expr *res = *e;
    *e = expr_cdr(*e);
    *expr_cdr_ref(res) = NIL;

    return res;
}

expr *expr_wrap(expr *e) {
    return cons_new(e, NIL);
}

expr *expr_unwrap(expr *e) {
    if (refcount(e))	return expr_car(e);

    expr *stuffed = expr_release(&e);
    expr *res = expr_car(stuffed);
    *expr_car_ref(stuffed) = NIL;

    expr_free(stuffed);
    return res;
}

static inline void no_action_free(expr*) {}
static inline void str_free(expr *e) { free(((str*)e)->value); }
static inline void sym_free(expr *e) {free(((sym*)e)->value); }
static inline void err_free(expr* e) { free(((err*)e)->value); }
static inline void cons_free(expr *e) {
	expr_free(expr_car(e)); /* free car(e) is recursive*/
	expr_free(expr_cdr(e)); /* free cdr(e) is recursive*/
}
static inline void abstract_func_free(expr *e) {
    if (!expr_subr_p(e)) {
	expr_free(((lfn*)e)->params);
	expr_free(((lfn*)e)->body);
    }
}

typedef void(*free_fn_t)(expr*);
const free_fn_t expr_free_fn[] = {
    err_free,
    sym_free,
    no_action_free,
    no_action_free,
    str_free,
    cons_free,
    no_action_free,
    abstract_func_free,
    abstract_func_free,
    abstract_func_free
};

void expr_free(expr *e) {
    if (!e || refcount(e))		return;
    expr_free_fn[e->type](e);

    free(e);
}

static inline int int_num_equal(expr *e, expr *other) {
    return as_int_num(e)->value == as_int_num(other)->value;
}

static inline int float_num_equal(expr *e, expr *other) {
    return as_float_num(e)->value == as_float_num(other)->value;
}

static inline int sym_equal(expr *e, expr *other) {
    return !strcmp(as_sym(e)->value, as_sym(other)->value);
}

static inline int str_equal(expr *e, expr *other) {
    return !strcmp(as_str(e)->value, as_str(other)->value);
}

static inline int err_equal(expr *e, expr *other) {
    return !strcmp(as_err(e)->value, as_err(other)->value);
}

static inline int t_equal(expr*, expr*) { return 1; }

int expr_equal(expr *e, expr *other);
static int cons_equal(expr *e, expr *other) {
    return expr_equal(expr_car(e), expr_car(other)) &&
	expr_equal(expr_cdr(e), expr_cdr(other));
}

static inline int abstract_func_equal(expr *e, expr *other) {return e == other;}

typedef int (*equal_fn_t)(expr*, expr*);
const equal_fn_t expr_equal_fn[] = {
    err_equal,
    sym_equal,
    int_num_equal,
    float_num_equal,
    str_equal,
    cons_equal,
    t_equal,
    abstract_func_equal,
    abstract_func_equal,
    abstract_func_equal
};

int expr_equal(expr *e, expr *other) {
    if (!e)	return !other;	/* NIL & _   */
    if (!other)  return 0;	/* T   & NIL */

    if (e->type != other->type)	return 0;
    return expr_equal_fn[e->type](e, other);
}

static inline void int_num_print(expr *e) {printf("%d", ((int_num*)e)->value);}
static inline void float_num_print(expr *e) {printf("%.2f",
						    ((float_num*)e)->value);}
static inline void str_print(expr *e) { printf("\"%s\"", ((str*)e)->value); }
static inline void sym_print(expr *e) { printf("%s", ((sym*)e)->value); }
static inline void err_print(expr *e) { fprintf(stderr, "error: %s", ((sym*)e)->value); }
static void cons_print(expr *e) {
    /* TODO: change to sym equality */
    if (expr_car(e) && expr_car(e)->type == EXPR_SYM &&
	!strcmp(((sym*)expr_car(e))->value, "QUOTE")) {
	putchar('\'');
	expr_print(expr_car(expr_cdr(e)));
	return;
    }

    putchar('(');
    while (e) {
	expr *car = expr_car(e);
	expr *cdr = expr_cdr(e);

	/* TODO: fix this */
	/*if (car)*/ expr_print(car);
	if (cdr)	{
	    putchar(' ');

	    if (cdr->type == EXPR_CONS)	e = (expr*)cdr;
	    else {
		printf(". ");
		expr_print(cdr);
		e = (expr*)cdr;
	    }
	} else
	    break;
    }

    putchar(')');
}

static inline void abstract_func_print(expr *e) {
    if (expr_subr_p(e))	printf("<SUBR>");
    else {
	putchar('(');
	expr_print(((lfn*)e)->params); putchar(' ');
	//expr_print();
	expr_print(((lfn*)e)->body);
	putchar(')');
    }
}
static inline void t_print(expr *e) { putchar('T'); }

typedef void (*print_fn_t)(expr*);
const print_fn_t expr_print_fn[] = {
    err_print,
    sym_print,
    int_num_print,
    float_num_print,
    str_print,
    cons_print,
    t_print,
    abstract_func_print,
    abstract_func_print,
    abstract_func_print
};


void expr_print(expr *e) {
    if (!e) { printf("NIL"); return; }
    expr_print_fn[e->type](e);
}

void expr_printnl(expr *e) {
    expr_print(e); putchar('\n');
}


/* expr *take_car(expr *e, int n) { */
/*     expr *eptr = e; */
/*     expr *res = NULL; */

/*     for (;;) { */
/* 	if (n--) { */
/* 	    res = expr_car(eptr); */
/* 	    ((cons*)eptr)->car = NULL; */
/* 	} */

/* 	eptr = expr_cdr(eptr); */
/*     } */

/*     /\* cleanup *\/ */
/*     expr_free(e); */
/*     return res; */
/* } */

static inline expr *err_copy(expr *e) { return err_new(((err*)e)->value); }
static inline expr *sym_copy(expr *e) { return sym_new(((sym*)e)->value); }
static inline expr *int_num_copy(expr *e) { return int_num_new(((int_num*)e)->value); }
static inline expr *float_num_copy(expr *e) { return float_num_new(((float_num*)e)->value); }
static inline expr *str_copy(expr *e) { return str_new(((str*)e)->value); }
static inline expr *t_copy(expr *) { return t_new(); }
static expr *cons_copy(expr *e) {
    expr *next = NULL;
    if (expr_cdr(e)) {
	next = expr_copy(expr_cdr(e));
    }
    return cons_new(expr_copy(expr_car(e)), next);
}
static inline expr *abstract_func_copy(expr *e) {
    return (expr_subr_p(e))
	? subr_new(((subr*)e)->fn, e->type)
	: lfn_new(expr_copy(((lfn*)e)->params),
		  expr_copy(((lfn*)e)->body),
		  e->type);
}

typedef expr*(*copy_fn_t)(expr*);
const copy_fn_t expr_copy_fn[] = {
    err_copy,
    sym_copy,
    int_num_copy,
    float_num_copy,
    str_copy,
    cons_copy,
    t_copy,
    abstract_func_copy,
    abstract_func_copy,
    abstract_func_copy
};

expr *expr_copy(expr *e) {
    if (!e)	return NIL;

    return expr_copy_fn[e->type](e);
    //return err_new("function 'copy': unknown expression type.");
}

/* void init_tables() { */
/*     /\* not working with 9997 AND `(eval (list '+ 1 (+ 1 2)))` *\/ */
/*     vartable = symtable_new(997, NULL); */
/*     fntable = symtable_new(997, NULL); */

/*     /\* list stuff *\/ */
/*     symtable_put(fntable, "head", subr_new(expr_head, EXPR_FUNC)); */
/*     symtable_put(fntable, "tail", subr_new(expr_tail, EXPR_FUNC)); */
/*     symtable_put(fntable, "car", subr_new(expr_car, EXPR_FUNC)); */
/*     symtable_put(fntable, "cdr", subr_new(expr_cdr, EXPR_FUNC)); */
/*     symtable_put(fntable, "len", subr_new(expr_len, EXPR_FUNC)); */
/*     symtable_put(fntable, "cons", subr_new(expr_cons, EXPR_FUNC)); */
/*     symtable_put(fntable, "list", subr_new(expr_list, EXPR_FUNC)); */

/*     /\* variable stuff *\/ */
/*     symtable_put(fntable, "set", subr_new(expr_set, EXPR_FUNC)); */

/*     /\* arithmetic operations *\/ */
/*     symtable_put(fntable, "+", subr_new(expr_plus, EXPR_FUNC)); */
/*     symtable_put(fntable, "-", subr_new(expr_sub, EXPR_FUNC)); */
/*     symtable_put(fntable, "*", subr_new(expr_mul, EXPR_FUNC)); */
/*     symtable_put(fntable, "/", subr_new(expr_div, EXPR_FUNC)); */

/*     /\* logic operations *\/ */
/*     symtable_put(fntable, "and", subr_new(expr_and, EXPR_MACRO)); */
/*     symtable_put(fntable, "or", subr_new(expr_or, EXPR_MACRO)); */
/*     symtable_put(fntable, "not", subr_new(expr_not, EXPR_FUNC)); */

/*     /\* comparison operations *\/ */
/*     symtable_put(fntable, "equal", subr_new(expr_equal, EXPR_FUNC)); */


/*     /\* misc *\/ */
/*     symtable_put(fntable, "eval",   subr_new(expr_eval, EXPR_FUNC)); */
/*     symtable_put(fntable, "print",   subr_new(expr_print_, EXPR_FUNC)); */
/*     symtable_put(fntable, "call",   subr_new(expr_call, EXPR_FUNC)); */
/*     symtable_put(fntable, "progn",  subr_new(expr_progn, EXPR_MACRO)); */
/*     symtable_put(fntable, "lambda", subr_new(expr_lambda, EXPR_MACRO)); */
/*     symtable_put(fntable, "defun",  subr_new(expr_defun, EXPR_MACRO)); */
/*     symtable_put(fntable, "sym-function",  subr_new(expr_sym_function, EXPR_FUNC)); */

/* } */

/* void finalize_tables() { */
/*     /\* cleanup *\/ */
/*     symtable_destroy(fntable); */
/*     symtable_destroy(vartable); */
/* } */
