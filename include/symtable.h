#ifndef __SYM_TABLE_H__
#define __SYM_TABLE_H__

//#include "lisp_types.h"

/* interface to expr */
typedef struct expr expr;

typedef struct symcell {
    char *key;
    expr *value;
} symcell;

#define SYMTABLE_INIT_SIZE 9997

typedef struct symtable {
    // TODO: change this to symcell** and apply it to the contstructors
    symcell **table;
    int sz;
    struct symtable *parent;
} symtable;

symcell *symcell_new(const char *key, expr *value);
static void symcell_destroy(symcell *cell);

symtable *symtable_new();
void symtable_destroy();

void symtable_put(symtable *table, const char *key, expr *value);
//void symtable_put_on_root(symtable *table, const char *key, expr *value);
//int symtable_contains(symtable *table, const char *key) {
expr *symtable_get(symtable *table, const char *key);
//void symtable_erase(const char *key);

#endif // __SYM_TABLE_H__
