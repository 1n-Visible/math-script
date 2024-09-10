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
    for (size_t i=0; i<register_len; i++) {
        if (global_value_register[i]==rt_expr) {
            global_value_register[i]=global_value_register[--register_len];
            return free_RTExpr(rt_expr);
        }
    }

    perror("collect_RTValue() rt_expr not in global_value_register");
    free_RTExpr(rt_expr);
}

size_t garbage_collect(ushort level) { //TODO:
    return 0;
}


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

RTExpr *eval_Atom(Scope *scope, Atom atom) {
    RTExpr *rt_expr;
    switch (atom.type) {
        case AT_IDENTIFIER:
            rt_expr=Scope_get_var(scope, atom.identifier);
            if (rt_expr!=NULL) //TODO: cycle detection
                return rt_expr;
            return NULL; //TODO: rt_var
        case AT_CHAR:
        case AT_STRING:
        case AT_FSTRING:
            break;
        case AT_INT:
            return NULL;
        case AT_FLOAT:
        	break;
    }
    return NULL;
}
