#ifndef SCOPE_H
#define SCOPE_H

#include <stdlib.h>
#include <stdbool.h>

#include "utils/hash_map.h"
#include "utils/dict_tree.h"
#include "rt_value.h"

typedef struct Scope Scope;

struct Scope {
    Scope *parent;
    wchar_t *name;
    DictTree *vars;
};

Scope *new_Scope(Scope *parent, wchar_t *name);
void free_Scope(Scope *scope);
RTExpr *Scope_get_var(Scope *scope, const wchar_t *varname);

typedef struct ValueRegister ValueReg;

ValueReg *new_ValueReg(size_t size);
void free_ValueReg(ValueReg *vr);

#endif
