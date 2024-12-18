#include <stdlib.h>

#include "ast_nodes.h"
#include "scope.h"
#include "rt_values/number.h"
#include "rt_values/vector.h"/*
#include "rt_values/array.h"
#include "rt_values/string.h"
#include "rt_values/.h"*/

#ifndef RT_VALUES_H
#define RT_VALUES_H

/*
value = (number | vector | char | string | list | set | dict)
object = value | array(of values) | function
*/

enum rt_value_t {
    RT_ERROR=-1,
    RT_NUMBER, RT_VECTOR, RT_ARRAY,
    RT_CHAR, RT_STRING,
    RT_SET, RT_METASET, RT_LIST,
    RT_FUNCTION, RT_USERTYPE
};

enum rt_expr_t {
    RT_VALUE, RT_VAR, RT_UNARY, RT_BINOP, RT_CALL,
    RT_SUM, RT_PROD, RT_INT
};

typedef struct RTValue RTValue;
typedef struct RTExpr RTExpr;

struct RTValue { //TODO: separate error into own struct with line & col
    enum rt_value_t type;
    union {
        wchar_t *errormsg;
        number_t number; vector_t vector; /*array_t array;
        wchar_t character; string_t string;*/
    };
};

struct RTExpr {
    enum rt_expr_t type;
    bool is_defined;
    OperType oper;

    union {
        RTValue *rt_value;
        wchar_t *varname;
        RTExpr *value, *left;
    };
    RTExpr *right;
};

RTValue *new_RTValue(enum rt_value_t type);
void free_RTValue(RTValue *);
RTValue *copy_RTValue(RTValue *);
void print_RTValue(RTValue *);

RTValue *RTValue_binop(OperType, RTValue *, RTValue *);

#define RT_VALUE_UNARY(name) RTValue *RTValue_##name(RTValue *value)

#undef RT_VALUE_UNARY


#define RT_VALUE_OPER(name) RTValue *RTValue_##name(RTValue *, RTValue *);

RT_VALUE_OPER(add)
RT_VALUE_OPER(sub)
RT_VALUE_OPER(mul)
RT_VALUE_OPER(truediv)
RT_VALUE_OPER(matmul)
RT_VALUE_OPER(div)
RT_VALUE_OPER(mod)
RT_VALUE_OPER(pow)
#undef RT_VALUE_OPER


extern RTExpr *alloc_RTExpr(enum rt_expr_t type);
void free_RTExpr(RTExpr *);
void print_RTExpr(RTExpr *);
RTExpr *eval_RTExpr(Scope *, RTExpr *);

#endif
