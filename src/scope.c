#include "scope.h"

struct RTAllocator {
    RTExpr **objects;
    size_t length, size;
};

RTAlloc *new_RTAlloc(size_t size) {
    RTAlloc *alloc = malloc(sizeof(RTAlloc));

    alloc->objects=calloc(size, sizeof(RTExpr *));
    alloc->length=0; alloc->size=size;
    return alloc;
}

void free_RTAlloc(RTAlloc *alloc) {
    for (size_t i=0; i<alloc->size; i++)
        free_RTExpr(alloc->objects[i]);
    free(alloc->objects);
    free(alloc);
}

RTExpr *alloc_RTExpr(RTAlloc *alloc, enum rt_expr_t type) {
    RTExpr *rt_expr = malloc(sizeof(RTExpr));
    rt_expr->type=type;

    if (alloc->length>=alloc->size) {
        alloc->size*=BUFF_GROWRATE;
        alloc->objects = realloc(alloc->objects, alloc->size*sizeof(RTExpr *));
    }

    alloc->objects[alloc->length++] = rt_expr;
    return rt_expr;
}

void collect_RTExpr(RTAlloc *alloc, RTExpr *rt_expr) {
    if (rt_expr==NULL)
        return;

    for (size_t i=0; i<alloc->length; i++) {
        if (alloc->objects[i]==rt_expr) {
            alloc->objects[i]=alloc->objects[--alloc->length];
            free_RTExpr(rt_expr);
            return;
        }
    }

    perror("collect_RTExpr() rt_expr not in value register");
    free_RTExpr(rt_expr);
}

size_t garbage_collect(RTAlloc *alloc, ushort level) {
    return 0;
}

void print_RTAlloc(RTAlloc *alloc) {
    size_t length=alloc->length;
    wprintf(
        L"RTAlloc(length=%ld, size=%ld, objects=[",
        length, alloc->size
    );

    if (length==0) {
        wprintf(L"])\n");
        return;
    } else putwchar(L'\n');

    print_RTExpr(alloc->objects[0]);
    for (size_t i=1; i<length; i++) {
        wprintf(L", ");
        print_RTExpr(alloc->objects[i]);
    }
    wprintf(L"\n])\n");
}


Scope *new_Scope(Scope *parent, wchar_t *name) {
    Scope *scope=malloc(sizeof(Scope));
    scope->parent=parent;
    if (name!=NULL)
        scope->name=wcsdup(name);
    else
        scope->name=NULL;

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
