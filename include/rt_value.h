#ifndef RT_VALUES_H
#define RT_VALUES_H

#include <stdlib.h>

#include "ast_nodes.h"
#include "simple_dt/number.h"
//#include "simple_dt/string.h"
#include "complex_dt/vector.h"
//#include "complex_dt/.h"

/*
object = (error | null_type | boolean | number | char | string)
expr = value | (vector | array | list | set | dict | function | exec_func)
*/

enum rt_value_t {
    RT_ERROR=-1, RT_NULL, RT_BOOL, RT_NUMBER,
    RT_CHAR, RT_STRING, RT_RANGE, RT_SET, RT_METASET
};

enum rt_expr_t {
    RT_VALUE, RT_VAR, RT_CALL,
    RT_UNARY, RT_BINOP, RT_SUM, RT_PROD, RT_INT,
    RT_VECTOR, RT_ARRAY, RT_LIST, RT_DICT,
    RT_FUNCTION, RT_EXEC_FUNC, RT_USERTYPE
};

typedef struct RTValue RTValue;
typedef struct RTExpr RTExpr;

// Immutable atomic objects:
struct RTValue { //TODO: separate error into own struct with line & col
    enum rt_value_t type;
    union {
        wchar_t *errormsg;
        number_t number;
        //wchar_t character; string_t string;
    };
};

struct RTExpr {
    enum rt_expr_t type;
    bool is_defined;
    OperType oper;

    union {
        RTValue *rt_value;
        wchar_t *varname;
        RTExpr *value;
        vector_t vector; // array_t array; ...
    };
    RTExpr *left, *right;
};

RTValue *new_RTValue(enum rt_value_t type);
void free_RTValue(RTValue *);
RTValue *copy_RTValue(RTValue *);
void print_RTValue(RTValue *);

RTValue *RTValue_unary(OperType, RTValue *);
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

typedef struct ValueRegister ValueReg;
extern RTExpr *alloc_RTExpr(ValueReg *vr, enum rt_expr_t type);
void free_RTExpr(RTExpr *);
void print_RTExpr(RTExpr *);

#endif
