#include "complex_dt/vector.h"

vector_t new_vector(ushort dim) {
    return (vector_t){
        .components = calloc(dim, sizeof(RTExpr *)),
        .dim = dim
    };
}

vector_t vector_from_expressions(RTExpr **expressions, ushort dim) {
    vector_t vector = new_vector(dim); // TODO: throw error
    memcpy(vector.components, expressions, dim*sizeof(RTExpr *));
    return vector;
}

void free_vector(vector_t *vector) {
    if (vector!=NULL)
        free(vector->components);
}

vector_t copy_vector(vector_t vector) {
    ushort dim = vector.dim;
    vector_t vector_copy = new_vector(dim);

    memcpy(vector_copy.components, vector.components, dim*sizeof(RTExpr *));
    return vector_copy;
}

/*#define BUFF_GROWRATE 1.3f
wchar_t *vector_to_str(vector_t vector) {
    wchar_t *string, *component_str;
    wchar_t **components=calloc(vector.dim, sizeof(wchar_t *));
    size_t strlen=0, strsize=2*vector.dim+4;
    
    ushort i, dim=vector.dim;
    for (i=0; i<dim; i++) {
        real_str=_Real_to_str(vector.components[i], vector.types[i]);
        components[i]=real_str;
        strsize+=wcslen(real_str);
    }
    
    string=calloc(strsize, sizeof(wchar_t));
    real_str=components[0];
    strlen+=swprintf(string, strsize-strlen, L"(%ls", real_str);
    free(real_str);
    
    for (i=1; i<dim; i++) {
        real_str=components[i];
        strlen+=swprintf(string+strlen, strsize-strlen, L", %ls", real_str);
        free(real_str);
    }
    
    swprintf(string+strlen, strsize-strlen, L")");
    free(components);
    return string;
}
#undef BUFF_GROWRATE*/

RTExpr *vector_getindex(vector_t vector, ushort index) {
    if (index>=vector.dim)
        return NULL; // TODO: throw error

    return vector.components[index];
}

void vector_setindex(vector_t vector, ushort index, RTExpr *expr) {
    if (index>=vector.dim)
        return; //TODO: throw error

    vector.components[index]=expr;
}


static short coord_to_index(char coordinate) {
    switch (coordinate) {
        case 'x': return 0;
        case 'y': return 1;
        case 'z': return 2;
        case 'w': return 3;
    }

    return -1;
}

RTExpr *vector_getcoordinate(vector_t vector, char coordinate) {
    short index=coord_to_index(coordinate);
    if (index==-1) return NULL;

    return vector_getindex(vector, index);
}

void vector_setcoordinate(vector_t vector, char coordinate, RTExpr *expr) {
    short index=coord_to_index(coordinate);
    if (index==-1) return;

    vector_setindex(vector, index, expr);
}

/*
#define VECTOR_OPER(name) \
vector_t vector_##name(vector_t vec1, vector_t vec2) { \
    ushort dim=vec1.dim; \
    if (vec2.dim!=dim) return (vector_t){NULL, NULL, 0}; \
    vector_t vec = (vector_t){calloc(dim, sizeof(union _Real)), \
                              calloc(dim, sizeof(enum num_type)), dim}; \
    \
    for (ushort i=0; i<dim; i++) \
        vec.components[i]=_Real_##name( \
            vec1.components[i], vec1.types[i], \
            vec2.components[i], vec2.types[i], vec.types+i \
        ); \
    return vec; \
}

VECTOR_OPER(add)
VECTOR_OPER(sub)
VECTOR_OPER(mul)
VECTOR_OPER(truediv)*/

#undef VECTOR_OPER
