#include "rt_value.h"

RTValue *new_RTValue(enum rt_value_t type) {
    RTValue *rt_value = malloc(sizeof(RTValue));
    rt_value->type=type;
    return rt_value;
}

void free_RTValue(RTValue *rt_value) {
    if (rt_value==NULL)
        return;

    switch (rt_value->type) {
        case RT_ERROR:
            free(rt_value->errcode); break;
        case RT_VECTOR:
            free_vector(&rt_value->vector); break;
        case RT_ARRAY:
        case RT_STRING:
        	break;
    }

    free(rt_value);
}

RTValue *copy_RTValue(RTValue *rt_value) {
    if (rt_value==NULL)
        return NULL;

    enum rt_value_t type = rt_value->type;
    RTValue *rt_value_copy = new_RTValue(type);
    switch (type) {
        case RT_ERROR:
            rt_value_copy->errcode=string_copy(rt_value->errcode);
            break;
        case RT_VECTOR:
            rt_value_copy->vector=copy_vector(rt_value->vector);
            break;
        case RT_ARRAY:
        case RT_STRING:
        	break;
    }

    return rt_value_copy;
}


#define RT_VALUE_UNARY(name) RTValue *RTValue_##name(RTValue *value)

#undef RT_VALUE_UNARY


#define RT_VALUE_OPER(name) \
RTValue *RTValue_##name(RTValue *value1, RTValue *value2)
#define ERR_CHECK(name) \
    if (value1->type==RT_ERROR) return copy_RTValue(value1); \
    if (value2->type==RT_ERROR) return copy_RTValue(value2); \
    RTValue *value = new_RTValue(RT_NUMBER); \
    value->number=number_##name(value1->number, value2->number); \
    return value;

RT_VALUE_OPER(add) {
    ERR_CHECK(add)
}

RT_VALUE_OPER(sub) {
    ERR_CHECK(sub)
}

RT_VALUE_OPER(mul) {
    ERR_CHECK(mul)
}

RT_VALUE_OPER(truediv) {
    ERR_CHECK(truediv)
}

RT_VALUE_OPER(div) {
    ERR_CHECK(div)
}

RT_VALUE_OPER(mod) {
    ERR_CHECK(mod)
}

RT_VALUE_OPER(pow) {
    ERR_CHECK(pow)
}

#undef RT_VALUE_UNARY
#undef ERR_CHECK


void free_RTExpr(RTExpr *rt_expr) {
    if (rt_expr==NULL)
        return;

    switch (rt_expr->type) {
        case RT_VALUE:
            free_RTValue(rt_expr->rt_value); break;
        case RT_VAR:
            free(rt_expr->varname); break;
        case RT_UNARY_FUNC:
        case RT_UNARY_POS:
        case RT_UNARY_NEG:
        case RT_UNARY_PM:
            free_RTExpr(rt_expr->value); break;
        case RT_BINOP_POW:
        case RT_BINOP_MUL:
        case RT_BINOP_TRUEDIV:
        case RT_BINOP_ADD:
        case RT_BINOP_SUB:
        case RT_BINOP_PM:
            free_RTExpr(rt_expr->left);
            free_RTExpr(rt_expr->right);
    }

    free(rt_expr);
}

RTExpr *eval_RTExpr(RTExpr *rt_expr) { //TODO
    return NULL;
}
