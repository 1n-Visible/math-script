#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <iso646.h>
#include <math.h>

#ifndef INTMATH_H
#define INTMATH_H

int64_t imin(int64_t, int64_t);
int64_t imax(int64_t, int64_t);

int64_t idiv(int64_t, int64_t);
int64_t imod(int64_t, int64_t);

uint64_t isqrt(uint64_t);
short ilog2(uint64_t);

bool isprime(uint64_t n);
uint64_t *primes_length(size_t length);
uint64_t *prime_factors(uint64_t n, short *length);
uint64_t factors_to_number(uint64_t *factors, short length);
uint64_t gcd(uint64_t, uint64_t);

#endif
