#include "rt_values/vector.h"

vector_t new_vector(ushort dim) {
    return (vector_t){
        .components = calloc(dim, sizeof(union _Real)),
        .types = calloc(dim, sizeof(enum num_type)),
        .dim = dim
    };
}

vector_t vector_from_numbers(number_t *numbers, ushort dim) {
    vector_t vector = new_vector(dim); // TODO: throw error

    for (ushort i=0; i<dim; i++) {
        number_t number = numbers[i];
        vector.components[i] = number.real;
        vector.types[i] = number.real_t;
    }

    return vector;
}

void free_vector(vector_t *vector) {
    if (vector==NULL)
        return;

    free(vector->components);
    free(vector->types);
}

vector_t copy_vector(vector_t vector) {
    ushort dim = vector.dim;
    vector_t vector_copy = new_vector(dim);

    memcpy(vector_copy.components, vector.components,
           dim*sizeof(union _Real));
    memcpy(vector_copy.types, vector.types,
           dim*sizeof(enum num_type));
    return vector_copy;
}


number_t vector_getindex(vector_t vector, ushort index) {
    if (index>=vector.dim)
        return number_zero;

    return (number_t){
        .real=vector.components[index], .real_t=vector.types[index],
        .imag=_Real_zero, .imag_t=NUMT_INT
    };
}

void vector_setindex(vector_t vector, ushort index, number_t number) {
    if (index>=vector.dim)
        return;

    vector.components[index]=number.real;
    vector.types[index]=number.real_t;
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

number_t vector_getcoordinate(vector_t vector, char coordinate) {
    short index=coord_to_index(coordinate);
    if (index==-1) return number_zero;

    return vector_getindex(vector, index);
}

void vector_setcoordinate(vector_t vector, char coordinate,
                              number_t number) {
    short index=coord_to_index(coordinate);
    if (index==-1) return;

    vector_setindex(vector, index, number);
}


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
VECTOR_OPER(truediv)

#undef VECTOR_OPER
