#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>
#include <iso646.h>

#include "fraction.h"

#ifndef _REAL_H
#define _REAL_H

enum num_type {NUMT_INT, NUMT_FRACT, NUMT_FLOAT};

union _Real {
    int64_t integer;
    double floating;
    Fraction fraction;
};

extern const union _Real _Real_zero, _Real_unit;
extern const union _Real _Real_Inf, _Real_NaN;

bool _Real_sign(union _Real, enum num_type);

bool _Real_to_bool(union _Real, enum num_type);
int64_t _Real_to_int(union _Real, enum num_type);
double _Real_to_double(union _Real, enum num_type);
Fraction _Real_to_Fraction(union _Real, enum num_type);
wchar_t *_Real_to_str(union _Real, enum num_type);


#define REAL_UNARY(name) \
union _Real _Real_##name(union _Real, enum num_type, \
                         enum num_type *ret_type);
REAL_UNARY(to_floating)
REAL_UNARY(floor)
REAL_UNARY(ceil)
REAL_UNARY(round)

REAL_UNARY(pos)
REAL_UNARY(neg)
REAL_UNARY(abs)
REAL_UNARY(sqr)
REAL_UNARY(cube)
REAL_UNARY(sqrt)
#undef REAL_UNARY


#define REAL_OPER(name) union _Real _Real_##name( \
    union _Real, enum num_type, \
    union _Real, enum num_type, enum num_type *ret_type);

REAL_OPER(add)
REAL_OPER(sub)
REAL_OPER(mul)
REAL_OPER(truediv)
REAL_OPER(div)
REAL_OPER(mod)
REAL_OPER(pow)
#undef REAL_OPER

uint8_t _Real_comp(union _Real, enum num_type,
                   union _Real, enum num_type);

#endif
