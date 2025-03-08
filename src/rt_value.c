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
            free(rt_value->errormsg); break;
        case RT_STRING:
        case RT_RANGE:
        case RT_SET:
        case RT_METASET:
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
            rt_value_copy->errormsg=wcsdup(rt_value->errormsg);
            break;
        case RT_NULL: break;
        case RT_BOOL: break;
        case RT_NUMBER:
            rt_value_copy->number=rt_value->number;
            break;
        case RT_CHAR: break;
        case RT_STRING: break;
        case RT_RANGE: break;
        case RT_SET: break;
        case RT_METASET: break;
    }

    return rt_value_copy;
}

void print_RTValue(RTValue *rt_value) {
    wchar_t *string;
    switch (rt_value->type) {
        case RT_ERROR:
            wprintf(L"Runtime Error: %ls\n", rt_value->errormsg);
            break;
        case RT_NULL: break;
        case RT_BOOL: break;
        case RT_NUMBER:
            string=number_to_str(rt_value->number, NUMEXPR_NONE, false);
            wprintf(string);
            free(string);
            break;
        case RT_CHAR: break;
        case RT_STRING: break;
        case RT_RANGE: break;
        case RT_SET: break;
        case RT_METASET: break;
    }
}

RTValue *RTValue_unary(OperType oper, RTValue *value) {
    if (value->type==RT_ERROR) return copy_RTValue(value);

    RTValue *ret_value;
    switch (oper) {
        case UNARY_LENGTH: //TODO
            return NULL;
        case UNARY_ABS:
            ret_value=new_RTValue(RT_NUMBER);
            ret_value->number=number_abs(value->number);
            return ret_value;
        case UNARY_SQR:
            ret_value=new_RTValue(RT_NUMBER);
            ret_value->number=number_sqr(value->number);
            return ret_value;
        case UNARY_CUBE:
            ret_value=new_RTValue(RT_NUMBER);
            ret_value->number=number_cube(value->number);
            return ret_value;
        case UNARY_FACTORIAL:
            return NULL;
        case UNARY_SQRT:
            ret_value=new_RTValue(RT_NUMBER);
            ret_value->number=number_sqrt(value->number);
            return ret_value;
        case UNARY_POS:
            return copy_RTValue(value);
        case UNARY_NEG:
            ret_value=new_RTValue(RT_NUMBER);
            ret_value->number=number_neg(value->number);
            return ret_value;
        case UNARY_PM:
            return NULL;
    }

    return NULL;
}

RTValue *RTValue_binop(OperType oper, RTValue *value1, RTValue *value2) {
    if (value1->type==RT_ERROR) return copy_RTValue(value1);
    if (value2->type==RT_ERROR) return copy_RTValue(value2);

    RTValue *value;
    switch (oper) {
        case BINOP_ADD:
        case BINOP_SUB:
            value=new_RTValue(RT_NUMBER);
            value->number=((oper==BINOP_ADD)? number_add: number_sub)(value1->number, value2->number);
            return value;
        case BINOP_MUL:
        case BINOP_TRUEDIV:
            value=new_RTValue(RT_NUMBER);
            value->number=((oper==BINOP_MUL)? number_mul: number_truediv)(value1->number, value2->number);
            return value;
        case BINOP_DIV:
        case BINOP_MOD:
            value=new_RTValue(RT_NUMBER);
            value->number=((oper==BINOP_DIV)? number_div: number_mod)(value1->number, value2->number);
            return value;
    }

    return NULL;
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
    }

    free(rt_expr);
}

void print_RTExpr(RTExpr *rt_expr) {
    if (rt_expr==NULL)
        return;

    OperType oper = rt_expr->oper;
    const char *operstr = (oper<COMP_SUPERSET)? operators_string[oper]: NULL;
    bool is_postfix = (oper<=UNARY_FACTORIAL);

    switch (rt_expr->type) {
        case RT_VALUE:
            print_RTValue(rt_expr->rt_value); break;
        case RT_VAR:
            wprintf(rt_expr->varname); break;
        case RT_UNARY:
            if (is_postfix) wprintf(L"(");
            else wprintf(L"%s(", operstr);

            print_RTExpr(rt_expr->value);

            if (is_postfix) wprintf(L")%s", operstr);
            else wprintf(L")");
            break;
        case RT_BINOP:
            putwchar(L'(');
            print_RTExpr(rt_expr->left);
            wprintf(L" %s ", operstr);
            print_RTExpr(rt_expr->right);
            putwchar(L')');
            break;
        case RT_CALL: //TODO
            return;
    }
}
