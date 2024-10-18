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
            rt_value_copy->errormsg=string_copy(rt_value->errormsg);
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

void print_RTValue(RTValue *rt_value) {
    wchar_t *string;
    switch (rt_value->type) {
        case RT_ERROR:
            wprintf(L"Runtime Error: %ls\n", rt_value->errormsg);
            break;
        case RT_NUMBER:
            string=number_to_str(rt_value->number, NUMEXPR_NONE, false);
            wprintf(string);
            free(string);
            break;
        case RT_VECTOR:
            string=vector_to_str(rt_value->vector);
            wprintf(string);
            free(string);
            break;
    }
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
        case RT_UNARY:
            free_RTExpr(rt_expr->value); break;
        case RT_BINOP:
            free_RTExpr(rt_expr->left);
            free_RTExpr(rt_expr->right);
            break;
        case RT_CALL: //TODO
            return;
    }

    free(rt_expr);
}

void print_RTExpr(RTExpr *rt_expr) {
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
            print_RTExpr(rt_expr->left);
            wprintf(L" %s ", operstr);
            print_RTExpr(rt_expr->right);
            break;
        case RT_CALL: //TODO
            return;
    }
}

RTExpr *eval_RTExpr(Scope *scope, RTExpr *rt_expr) {
    RTExpr *rt_expr2;
    switch (rt_expr->type) {
        case RT_VALUE:
            rt_expr->is_defined=true;
            return rt_expr;

        case RT_VAR:
            rt_expr2=Scope_get_var(scope, rt_expr->varname);
            if (rt_expr2==NULL) {
                rt_expr->is_defined=false;
                return rt_expr;
            }

            return eval_RTExpr(scope, rt_expr2);

        case RT_UNARY:
            break; //TODO
    }

    return NULL;
}
