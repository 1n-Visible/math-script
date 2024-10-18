#include <stdlib.h>
#include <stdbool.h>

#include "hash_map.h"
#include "dict_tree.h"

#ifndef SCOPE_H
#define SCOPE_H

typedef struct Scope Scope;

struct Scope {
    Scope *parent;
    wchar_t *name;
    DictTree *vars;
};

Scope *new_Scope(Scope *parent, wchar_t *name);
void free_Scope(Scope *scope);

typedef struct RTExpr RTExpr;
RTExpr *Scope_get_var(Scope *scope, const wchar_t *varname);

#endif
