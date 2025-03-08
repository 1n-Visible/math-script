#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <stdlib.h>
#include <stdio.h>

#include "parser.h"
#include "ast_nodes.h"
#include "rt_value.h"
#include "scope.h"

extern DictTree *datatypes;

typedef struct Interpreter Interpreter;

Interpreter *new_Interpreter(FILE *file, char *filename);
void free_Interpreter(Interpreter *);
void eval_expr(Interpreter *);

RTExpr *eval_ASTNode(ValueReg *, ASTNode); // frees ASTNode

#endif
