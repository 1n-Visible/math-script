#include <stdlib.h>

#include "parser.h"
#include "ast_nodes.h"
#include "rt_value.h"
#include "dict_tree.h"
#include "charset.h"

#ifndef INTERPRETER_H
#define INTERPRETER_H

extern DictTree *datatypes;

RTExpr *alloc_RTExpr(enum rt_expr_t type);
void collect_RTExpr(RTExpr *);
size_t garbage_collect(ushort level);

typedef struct Scope Scope;

struct Scope {
    Scope *parent;
    wchar_t *name;
    DictTree *vars;
};

Scope *new_Scope(Scope *parent, wchar_t *name);
void free_Scope(Scope *scope);
RTExpr *Scope_get_var(Scope *scope, const wchar_t *varname);

RTExpr *eval_Atom(Scope *scope, Atom atom);
RTExpr *eval_Expression(Scope *scope, Expression *expr);

#endif
