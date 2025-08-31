#ifndef SCOPE_H
#define SCOPE_H

#include <stdlib.h>
#include <stdbool.h>

#include "utils/hash_map.h"
#include "utils/dict_tree.h"
#include "rt_value.h"

typedef struct RTAllocator RTAlloc;

RTAlloc *new_RTAlloc(size_t size);
void free_RTAlloc(RTAlloc *);

RTExpr *alloc_RTExpr(RTAlloc *, enum rt_expr_t type);
void collect_RTExpr(RTAlloc *, RTExpr *);

void print_RTAlloc(RTAlloc *);

typedef struct Scope Scope;

struct Scope {
	Scope *parent;
	wchar_t *name;
	HashMap *vars;
	RTAlloc *alloc;
};

Scope *new_Scope(Scope *parent, wchar_t *name);
void free_Scope(Scope *);
RTExpr *Scope_get_var(Scope *, const wchar_t *varname);

size_t garbage_collect(Scope *, ushort level);

#endif
