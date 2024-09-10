#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "../intmath.h"

#ifndef FRACTION_H
#define FRACTION_H

typedef struct {
    int64_t m;
    uint64_t n;
} Fraction;

Fraction fraction(int64_t m, int64_t n);

bool Fraction_sign(Fraction);
int64_t Fraction_to_int(Fraction);
double Fraction_to_double(Fraction);
int64_t Fraction_floor(Fraction);
int64_t Fraction_ceil(Fraction);
int64_t Fraction_round(Fraction);

Fraction Fraction_pos(Fraction);
Fraction Fraction_neg(Fraction);
Fraction Fraction_abs(Fraction);
Fraction Fraction_sqr(Fraction);
Fraction Fraction_cube(Fraction);
double Fraction_sqrt(Fraction);

Fraction Fraction_add(Fraction, Fraction);
Fraction Fraction_sub(Fraction, Fraction);
Fraction Fraction_mul(Fraction, Fraction);
Fraction Fraction_truediv(Fraction, Fraction);
int64_t Fraction_div(Fraction, Fraction);
Fraction Fraction_mod(Fraction, Fraction);
Fraction Fraction_pow(Fraction, int8_t power);

uint8_t Fraction_comp(Fraction, Fraction);

#endif
