#include <stdlib.h>

#include "parser.h"
#include "ast_nodes.h"
#include "rt_value.h"
#include "scope.h"

#ifndef INTERPRETER_H
#define INTERPRETER_H

extern DictTree *datatypes;

RTExpr *alloc_RTExpr(enum rt_expr_t type);
void collect_RTExpr(RTExpr *);
size_t garbage_collect(ushort level);

RTExpr *eval_Expression(Scope *scope, Expression *expr);

#endif
