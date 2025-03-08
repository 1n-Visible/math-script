#include "interpreter.h"

DictTree *datatypes = NULL;

#define D(dt) add_word(datatypes, dt)
__attribute__((constructor)) static void setup_interpreter() {
    datatypes=new_DictTree(); {
        D(L"bool"), D(L"number"), D(L"vector"), D(L"char"), D(L"str"),
        D(L"list"), D(L"set"), D(L"dict"), D(L"metaset");
    };
}

__attribute__((destructor)) static void teardown_interpreter() {
    free_DictTree(datatypes);
}
#undef D

const wchar_t *interpreter_prompt=L"\n\nms> ";

struct Interpreter {
    FILE *file; char *filename;
    wchar_t *prompt;
    Parser *parser;
    Scope *glob_scope;
    ValueReg *vr;
    bool own_file;
};

Interpreter *new_Interpreter(FILE *file, char *filename) {
    bool own_file = false;
    if (file==NULL) {
        own_file = true;
        file = fopen(filename, "r");
        if (file==NULL)
            return NULL;
    }

    Interpreter *inter = malloc(sizeof(Interpreter));
    inter->file=file; inter->filename=filename;
    inter->own_file=own_file;
    inter->prompt=interpreter_prompt;

    inter->parser=new_Parser(file);
    inter->glob_scope=new_Scope(NULL, str_to_wcs(filename, 0));
    inter->vr=new_ValueReg(64);
    return inter;
}

void free_Interpreter(Interpreter *inter) {
    free_Parser(inter->parser);
    free_Scope(inter->glob_scope);
    free_ValueReg(inter->vr);

    if (inter->own_file)
        fclose(inter->file);
    free(inter);
}


static RTExpr *eval_Atom(ValueReg *vr, Atom atom) { // frees Atom
    RTExpr *rt_expr;
    RTValue *rt_value;

    switch (atom.type) {
        case AT_IDENTIFIER:
            rt_expr=alloc_RTExpr(vr, RT_VAR);
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

    rt_expr=alloc_RTExpr(vr, RT_VALUE);
    rt_expr->rt_value=rt_value;
    return rt_expr;
}

static RTExpr *eval_Expression(ValueReg *vr, Expression *expr) { // frees Expression
    if (expr==NULL)
        return NULL;

    RTExpr *rt_expr=NULL;
    switch (expr->type) {
        case NT_ATOM:
            rt_expr=eval_Atom(vr, expr->atom);
            break;

        case NT_UNARY_PREFIX:
        case NT_UNARY_POSTFIX:
            rt_expr=alloc_RTExpr(vr, RT_UNARY);
            rt_expr->oper=expr->oper;
            rt_expr->value=eval_Expression(vr, expr->value);
            break;

        case NT_BINOP:
            rt_expr=alloc_RTExpr(vr, RT_BINOP);
            rt_expr->oper=expr->oper;
            rt_expr->left=eval_Expression(vr, expr->left);
            rt_expr->right=eval_Expression(vr, expr->right);
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

RTExpr *eval_ASTNode(ValueReg *vr, ASTNode node) { // frees ASTNode
    RTExpr *rt_expr;
    RTValue *rt_value;

    switch (node.type) {
        case NT_EOF:
        case NT_EMPTY:
            return NULL;
        case NT_ERROR:
            rt_value=new_RTValue(RT_ERROR);
            rt_value->errormsg=node.error_code;
            rt_expr=alloc_RTExpr(vr, RT_VALUE);
            rt_expr->rt_value=rt_value;
            return rt_expr;
        case NT_EXPR:
            return eval_Expression(vr, node.expr);
        case NT_COMMAND: // implement
            return NULL;
    }

    return NULL;
}

RTExpr *eval_RTExpr(RTExpr *rt_expr, Scope *scope, ValueReg *vr) {
    RTExpr *return_rt_expr, *rt_expr_left, *rt_expr_right;
    RTValue *rt_value;

    switch (rt_expr->type) {
        case RT_VALUE:
            return rt_expr;

        case RT_VAR:
            return_rt_expr=Scope_get_var(scope, rt_expr->varname);
            if (return_rt_expr==NULL)
                return rt_expr;

            return eval_RTExpr(return_rt_expr, scope, vr);

        case RT_CALL: return NULL;
        case RT_UNARY:
            return_rt_expr=eval_RTExpr(rt_expr->value, scope, vr);
            if (return_rt_expr->type!=RT_VALUE)
                return rt_expr;

            rt_value=RTValue_unary(rt_expr->oper, return_rt_expr->rt_value);
            return_rt_expr=alloc_RTExpr(vr, RT_VALUE);
            return_rt_expr->rt_value=rt_value;
            return return_rt_expr;

        case RT_BINOP:
            rt_expr_left=eval_RTExpr(rt_expr->left, scope, vr);
            if (rt_expr_left->type!=RT_VALUE)
                return rt_expr;

            rt_expr_right=eval_RTExpr(rt_expr->right, scope, vr);
            if (rt_expr_right->type!=RT_VALUE)
                return rt_expr;

            rt_value=RTValue_binop(rt_expr->oper, rt_expr_left->rt_value,
                                                  rt_expr_right->rt_value);
            return_rt_expr=alloc_RTExpr(vr, RT_VALUE);
            return_rt_expr->rt_value=rt_value;
            return return_rt_expr;

        case RT_SUM:
        case RT_PROD:
            return_rt_expr=eval_RTExpr(rt_expr->value, scope, vr);
        case RT_INT:
            return NULL;
        case RT_VECTOR: break;
        case RT_ARRAY: break;
        case RT_LIST: break;
        case RT_DICT: break;
        case RT_FUNCTION: break;
        case RT_EXEC_FUNC: break;
        case RT_USERTYPE: break;
    }

    return NULL;
}

void eval_expr(Interpreter *inter) {
    wprintf(inter->prompt);
    ASTNode node=parse_line(inter->parser);
    if (node.type==NT_EOF)
        return;

    print_ASTNode(node);
    RTExpr *rt_expr=eval_ASTNode(inter->vr, node);
    if (rt_expr==NULL)
        return;

    rt_expr=eval_RTExpr(rt_expr, inter->glob_scope, inter->vr);
    print_RTExpr(rt_expr);
}
