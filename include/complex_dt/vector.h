#ifndef VECTOR_H
#define VECTOR_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>

typedef struct RTExpr RTExpr;

typedef struct vector {
    RTExpr **components;
    ushort dim;
} vector_t;

vector_t new_vector(ushort dim);
vector_t vector_from_expressions(RTExpr **expressions, ushort dim);
void free_vector(vector_t *);
vector_t copy_vector(vector_t);

RTExpr *vector_getindex(vector_t, ushort index);
void vector_setindex(vector_t, ushort index, RTExpr *);

RTExpr *vector_getcoordinate(vector_t, char coordinate);
void vector_setcoordinate(vector_t, char coordinate, RTExpr *);

/*#define VECTOR_OPER(name) \
vector_t vector_##name(vector_t, vector_t);

VECTOR_OPER(add)
VECTOR_OPER(sub)
VECTOR_OPER(mul)
VECTOR_OPER(truediv)
#undef VECTOR_OPER*/

#endif
