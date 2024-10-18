#include "scope.h"

Scope *new_Scope(Scope *parent, wchar_t *name) {
    Scope *scope=malloc(sizeof(Scope));
    scope->parent=parent;
    scope->name=string_copy(name);
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
