#include "interpreter.h"

DictTree *datatypes = NULL;

static volatile RTExpr **global_value_register;
static volatile size_t register_len = 0, register_size = 16;

#define D(dt) add_word(datatypes, dt)
__attribute__((constructor)) static void setup_interpreter() {
    datatypes=new_DictTree(); {
        D(L"bool"), D(L"number"), D(L"vector"), D(L"char"), D(L"str"),
        D(L"list"), D(L"set"), D(L"dict"), D(L"metaset");
    };

    global_value_register=calloc(register_size, sizeof(RTExpr *));
}

__attribute__((destructor)) static void teardown_interpreter() {
    free_DictTree(datatypes);

    for (size_t i=0; i<register_len; i++)
        free_RTExpr(global_value_register[i]);
    free(global_value_register);
}
#undef D

RTExpr *alloc_RTExpr(enum rt_expr_t type) {
    RTExpr *rt_expr = malloc(sizeof(RTExpr));
    rt_expr->type=type;

    if (register_len>=register_size) {
        register_size*=BUFF_GROWRATE;
        global_value_register = realloc(
            global_value_register, register_size*sizeof(RTExpr *)
        );
    }

    global_value_register[register_len++] = rt_expr;
    return rt_expr;
}

void collect_RTExpr(RTExpr *rt_expr) {
    if (rt_expr==NULL)
        return;

    for (size_t i=0; i<register_len; i++) {
        if (global_value_register[i]==rt_expr) {
            global_value_register[i]=global_value_register[--register_len];
            free_RTExpr(rt_expr);
            return;
        }
    }

    perror("collect_RTExpr() rt_expr not in global_value_register");
    free_RTExpr(rt_expr);
}

size_t garbage_collect(ushort level) { //TODO:
    return 0;
}


static RTExpr *eval_Atom(Atom atom) { // frees Atom
    RTExpr *rt_expr;
    RTValue *rt_value;

    switch (atom.type) {
        case AT_IDENTIFIER:
            rt_expr=alloc_RTExpr(RT_VAR);
            rt_expr->varname=atom.identifier;
            return rt_expr;

        case AT_CHAR:
        case AT_STRING: //TODO
        case AT_FSTRING:
            rt_value=NULL;
            break;

        case AT_INT:
            rt_value=new_RTValue(RT_NUMBER);
            rt_value->number=number_from_int(atom.integer, atom.is_real);
            break;
        case AT_FLOAT:
        	rt_value=new_RTValue(RT_NUMBER);
            rt_value->number=number_from_float(atom.floating, atom.is_real);
            break;
        default:
            return NULL;
    }

    rt_expr=alloc_RTExpr(RT_VALUE);
    rt_expr->rt_value=rt_value;
    return rt_expr;
}

static RTExpr *eval_Expression(Expression *expr) { // frees Expression
    if (expr==NULL)
        return NULL;

    RTExpr *rt_expr=NULL;
    switch (expr->type) {
        case NT_ATOM:
            rt_expr=eval_Atom(expr->atom);
            break;

        case NT_UNARY_PREFIX:
        case NT_UNARY_POSTFIX:
            rt_expr=alloc_RTExpr(RT_UNARY);
            rt_expr->oper=expr->oper;
            rt_expr->value=eval_Expression(expr->value);
            break;

        case NT_BINOP:
            rt_expr=alloc_RTExpr(RT_BINOP);
            rt_expr->oper=expr->oper;
            rt_expr->left=eval_Expression(expr->left);
            rt_expr->right=eval_Expression(expr->right);
            break;
        case NT_COMP:
            // TODO: Evaluate == != < > <=...
        case NT_RANGE:
        case NT_SUM:
        case NT_PROD:
        case NT_INT:
            return NULL;
    }

    free(expr);
    return rt_expr;
}

RTExpr *eval_ASTNode(ASTNode node) { // frees ASTNode
    RTExpr *rt_expr;
    RTValue *rt_value;

    switch (node.type) {
        case NT_EOF:
        case NT_EMPTY:
            return NULL;
        case NT_ERROR:
            rt_value=new_RTValue(RT_ERROR);
            rt_value->errormsg=node.error_code;
            rt_expr=alloc_RTExpr(RT_VALUE);
            rt_expr->rt_value=rt_value;
            return rt_expr;
        case NT_EXPR:
            return eval_Expression(node.expr);
        case NT_COMMAND: // implement
            return NULL;
    }

    return NULL;
}
