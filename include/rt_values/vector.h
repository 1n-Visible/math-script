#include <stdlib.h>
#include <stdint.h>
#include <string.h>
/*#include <stdbool.h>
#include <iso646.h>*/

#include "number.h"
#include "_real.h"

#ifndef VECTOR_H
#define VECTOR_H

typedef struct vector {
    union _Real *components;
    enum num_type *types;
    ushort dim;
} vector_t;

vector_t new_vector(ushort dim);
vector_t vector_from_numbers(number_t *numbers, ushort dim);
void free_vector(vector_t *);
vector_t copy_vector(vector_t);
void print_vector(vector_t);

number_t vector_getindex(vector_t, ushort index);
void vector_setindex(vector_t, ushort index, number_t);

number_t vector_getcoordinate(vector_t, char coordinate);
void vector_setcoordinate(vector_t, char coordinate, number_t);

#define VECTOR_OPER(name) \
vector_t vector_##name(vector_t, vector_t);

VECTOR_OPER(add)
VECTOR_OPER(sub)
VECTOR_OPER(mul)
VECTOR_OPER(truediv)
#undef VECTOR_OPER

#endif
