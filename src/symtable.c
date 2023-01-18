#include "symtable.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lisp_types.h"


symcell *symcell_new(const char *key, expr *value) {
    symcell *cell = malloc(sizeof(cell));
    size_t l = strlen(key);
    cell->key = malloc(l+1);
    strcpy(cell->key, key);
    cell->value = value;	/* shallow copy */

    return cell;
}

static inline void symcell_destroy(symcell *cell) {
    cell->value->refcount--;
    free(cell->key);
    expr_free(cell->value);
    free(cell);
}

symtable *symtable_new(int sz, symtable *parent) {
    symtable *table = malloc(sizeof(*table));
    table->sz = sz ? sz : SYMTABLE_INIT_SIZE;
    table->table = malloc(sizeof(symcell*) * table->sz);
    memset(table->table, 0, sizeof(symcell*) * table->sz);
    table->parent = parent;

    return table;
}

void symtable_destroy(symtable *table) {
    for (int i = 0; i < table->sz; ++i) {
	if (table->table[i]) {
	    symcell_destroy(table->table[i]);
	}
    }

    free(table->table);
    free(table);
}

static inline
unsigned hash(const char *key) {
    unsigned hash = 0;
    unsigned c;
    while ((c = *key++))	hash = hash*9 ^ c;

    return hash;
}

//int symtable_contains(symtable *table, const char *key) {
//    symcell **cell = &table->table[hash(key)%table->sz];
//    int count = table->sz;
//
//    while (count--) {
//	if (*cell && !strcmp((*cell)->key, key))	return 1;
//
//	/* hash table impl is circular */
//	if (++cell >= (table->table + table->sz))	cell = table->table;
//    }
//
//    return 0;
//}
//
//void symtable_put_on_root(symtable *table, const char *key, expr *value) {
//    return table->parent
//	? symtable_put_on_root(table->parent, key, value)
//	: symtable_put(table, key, value);
//}

void symtable_put(symtable *table, const char *key, expr *value) {
    /* increment value's reference count */
    expr_incr_refcount(value);

    symcell **cell = &table->table[hash(key)%table->sz];
    int count = table->sz;

    while (count--) {
	if (*cell && !strcmp((*cell)->key, key))	{
	    expr_decr_refcount((*cell)->value);
	    (*cell)->value = value;
	    return;
	}

	if (!*cell) {
	    *cell = symcell_new(key, value);
	    return;
	}

	/* hash table impl is circular */
	if (++cell >= (table->table + table->sz))	cell = table->table;
    }

    fprintf(stderr, "error: symtable overflow\n");
    return;
}

/* TODO: swapping should decrease refcount */
expr *symtable_get(symtable *table, const char *key) {
    symcell **cell = &table->table[hash(key)%table->sz];
    int count = table->sz;

    while (count--) {
	if (*cell && !strcmp((*cell)->key, key))	return (*cell)->value;

	/* hash table impl is circular */
	if (++cell >= (table->table + table->sz))	cell = table->table;
    }

    return table->parent ?
	symtable_get(table->parent, key) : err_new("unbound symbol %s!", key);
}
