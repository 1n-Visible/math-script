#include "scope.h"

Scope *new_Scope(Scope *parent, wchar_t *name) {
    Scope *scope=malloc(sizeof(Scope));
    scope->parent=parent;
    scope->name=wcsdup(name);
    scope->vars=new_DictTree();
    return scope;
}

void free_Scope(Scope *scope) {
    if (scope==NULL)
        return;

    free_DictTree(scope->vars);
    free(scope->name);
    free(scope);
}

void print_Scope(Scope *scope) {
    wprintf(L"Scope(name=%ls, vars={...(impl)}", scope->name);
    //iterDictTree *iter_dt=new_iterDictTree(scope->vars);
    if (scope->parent!=NULL) {
        wprintf(L", parent=");
        print_Scope(scope->parent);
    }
    putwchar(L')');
}

RTExpr *Scope_get_var(Scope *scope, const wchar_t *varname) {
    if (scope==NULL)
        return NULL;

    RTExpr *rt_expr=get_word(scope->vars, varname);
    if (rt_expr!=NULL)
        return rt_expr;

    return Scope_get_var(scope->parent, varname);
}


struct ValueRegister {
    RTExpr **objects;
    size_t length, size;
};

ValueReg *new_ValueReg(size_t size) {
    ValueReg *vr = malloc(sizeof(ValueReg));

    vr->objects=calloc(size, sizeof(RTExpr *));
    vr->length=0; vr->size=size;
    return vr;
}

void free_ValueReg(ValueReg *vr) {
    for (size_t i=0; i<vr->size; i++)
        free_RTExpr(vr->objects[i]);
    free(vr->objects);
    free(vr);
}

RTExpr *alloc_RTExpr(ValueReg *vr, enum rt_expr_t type) {
    RTExpr *rt_expr = malloc(sizeof(RTExpr));
    rt_expr->type=type;

    if (vr->length>=vr->size) {
        vr->size*=BUFF_GROWRATE;
        vr->objects = realloc(vr->objects, vr->size*sizeof(RTExpr *));
    }

    vr->objects[vr->length++] = rt_expr;
    return rt_expr;
}

void collect_RTExpr(ValueReg *vr, RTExpr *rt_expr) {
    if (rt_expr==NULL)
        return;

    for (size_t i=0; i<vr->length; i++) {
        if (vr->objects[i]==rt_expr) {
            vr->objects[i]=vr->objects[--vr->length];
            free_RTExpr(rt_expr);
            return;
        }
    }

    perror("collect_RTExpr() rt_expr not in value register");
    free_RTExpr(rt_expr);
}

size_t garbage_collect(ValueReg *vr, ushort level) {
    return 0;
}
