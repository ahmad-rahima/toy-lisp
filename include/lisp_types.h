#ifndef __LISP_TYPES_H__
#define __LISP_TYPES_H__

#include "symtable.h"
#include <inttypes.h>
#include <stddef.h>

typedef struct env {
    symtable *val;
    symtable *fn;
} env;


typedef struct expr {
    uint8_t refcount;
    uint8_t type : 6;
} expr;

typedef struct sym {
    uint8_t refcount;
    uint8_t type : 6;
    char *value;
} sym;

typedef struct int_num {
    uint8_t refcount;
    uint8_t type : 6;
    uint8_t value;
} int_num;

typedef struct float_num {
    uint8_t refcount;
    uint8_t type : 6;
    double value;
} float_num;

typedef struct str {
    uint8_t refcount;
    uint8_t type : 6;
    char *value;
} str;

typedef struct cons {
    uint8_t refcount;
    uint8_t type : 6;
    struct expr *car;
    struct expr *cdr;
} cons ;

typedef struct err {
    uint8_t refcount;
    uint8_t type : 6;
    char *value;
} err;

typedef struct subr {
    uint8_t refcount;
    uint8_t type : 6;			/* FUNC, or MACRO */
    uint8_t subr_p : 2;
    expr*(*fn)(expr*, env*);
} subr;

typedef struct lfn {		/* lisp function */
    uint8_t refcount;
    uint8_t type : 6;			/* FUNC, or MACRO */
    uint8_t subr_p : 2;
    expr *params;
    expr *body;
} lfn;

enum {
    EXPR_ERR,
    EXPR_SYM,
    EXPR_INT_NUM,
    EXPR_FLOAT_NUM,
    EXPR_STR,
    EXPR_CONS,
    EXPR_T,
    EXPR_SPECIAL_FORM,
    EXPR_FUNC,
    EXPR_MACRO,
    EXPR_END
    //EXPR_SUBR = 0100		/* bit 64 to identify builtin functions*/
};

/* expression constructors */
expr *sym_new(const char *s);
expr *int_num_new(int n);
expr *float_num_new(double d);
expr *str_new(const char *s);
expr *err_new(const char *fmt , ...);
expr *cons_new(expr *car, expr *cdr);
expr *q_new(expr *e);
expr *t_new();
expr *subr_new(expr *(*fn)(expr*, env*), int type);
expr *lfn_new(expr *params, expr *body, int type);

#define NIL NULL

/* casting functions */
static inline sym       *as_sym      (expr *e) { return  (sym*)e;       }
static inline int_num   *as_int_num  (expr *e) { return  (int_num*)e;   }
static inline float_num *as_float_num(expr *e) { return  (float_num*)e; }
static inline str       *as_str      (expr *e) { return  (str*)e;       }
static inline err       *as_err      (expr *e) { return  (err*)e;       }
static inline cons      *as_cons     (expr *e) { return  (cons*)e;      }
//static inline q         *as_q        (expr *e) { return  (q*)e;         }
//static inline t         *as_t        (expr *e) { return  (t*)e;         }
static inline subr      *as_subr     (expr *e) { return  (subr*)e;      }
static inline lfn       *as_lfn      (expr *e) { return  (lfn*)e;       }

static inline int refcount(expr *e) { return e->refcount; }

static inline expr *expr_t_ize(int boolean) {return boolean ? t_new() : NIL; }

void expr_print(expr *e);
void expr_printnl(expr *e);
expr *expr_copy(expr*);

int expr_equal(expr *e, expr *other);

/* cons operations */
/* pop (free) the first cons out and return the next */
expr *expr_pop(expr*);
/* release the first cons out of e and return it. (redirect e to the next) */
expr *expr_release(expr **e);
/* free all cons except the idx-th and return it */
expr *expr_take(expr *e, int idx);
/* return new cons as (car e NIL) */
expr *expr_wrap(expr *e);
/* return car from the cons and (delete) the cons cell */
expr *expr_unwrap(expr *e);

void expr_incr_refcount(expr *e);
void expr_decr_refcount(expr *e);

int expr_len(expr *e);

void expr_free(expr *e);

static inline expr *expr_car(expr *e)      { return ((cons*)e)->car; }
static inline expr *expr_cdr(expr *e)      { return ((cons*)e)->cdr; }
static inline expr **expr_car_ref(expr *e) { return &((cons*)e)->car; }
static inline expr **expr_cdr_ref(expr *e) { return &((cons*)e)->cdr; }
static inline int expr_subr_p(expr *e)     { return ((subr*)e)->subr_p; }

const char *expr_type_str(int8_t type);

#define ERR_MSG_MAX_LEN 512


/* assertions */
//#define ASSERT(COND, FMT, ...)			\
//    if (!(COND))  return err_new(FMT, ##__VA_ARGS__);

#define EXPR_ASSERT_TYPE(E, T, FN, ...)					\
    { if (((E)->type) != (T)) {			\
	return err_new("function '%s': got incorrect argument type.",	\
		       FN, ##__VA_ARGS__); } }

#define CONS_ASSERT_TYPE(E, T, N, FN, ...)				\
    { expr *tmp=E; for (int i = 0; i < N; ++i) tmp = cdr(tmp);		\
	if ((car(tmp)->type) != (T)) {			\
	return err_new("function '%s': got incorrect argument "		\
		       "type.", FN, ##__VA_ARGS__); } }


#define CONS_ASSERT_LEN(E, N, FN, ...)					\
    { if (len(E) != (N)) {						\
	    return err_new("function '%s': got incorrect number "	\
			   "of arguments.", FN, ##__VA_ARGS__); } }


#define CONS_FOREACH_EXPR(E, CONS) \
    for (expr *E = (CONS); E; E = expr_cdr(E))

#endif // __LISP_TYPES_H__
