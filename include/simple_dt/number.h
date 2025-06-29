#ifndef NUMBER_H
#define NUMBER_H

#include <stdint.h>
#include <stdbool.h>
#include <wchar.h>
#include <iso646.h>

#include "utils/intmath.h"
#include "_real.h"

typedef struct number {
    union _Real real, imag;
    enum num_type real_t, imag_t;
} number_t;

extern const number_t number_zero, number_unit, number_imagunit;
extern const number_t number_ReInf, number_ImInf;
extern const number_t number_ReNaN, number_ImNaN, number_NaN;

number_t number_from_int(int64_t integer, bool is_real);
number_t number_from_float(double floating, bool is_real);

enum num_expr_t {NUMEXPR_NONE, NUMEXPR_LEFT, NUMEXPR_RIGHT};

bool number_to_bool(number_t);
wchar_t *number_to_str(number_t, enum num_expr_t, bool spaces);
bool number_is_real(number_t);


#define NUMBER_UNARY(name) number_t number_##name(number_t);

NUMBER_UNARY(to_floating)
NUMBER_UNARY(floor)
NUMBER_UNARY(ceil)
NUMBER_UNARY(round)

NUMBER_UNARY(pos)
NUMBER_UNARY(neg)
NUMBER_UNARY(conj)
NUMBER_UNARY(perp)

NUMBER_UNARY(abs_sqr)
NUMBER_UNARY(abs)
NUMBER_UNARY(arg)
double number_arg_cos(number_t);
double number_arg_sin(number_t);
NUMBER_UNARY(arg_tan)

NUMBER_UNARY(sqr)
NUMBER_UNARY(cube)
NUMBER_UNARY(sqrt)

#undef NUMBER_UNARY


#define NUMBER_OPER(name) number_t number_##name(number_t, number_t);

NUMBER_OPER(add)
NUMBER_OPER(sub)
NUMBER_OPER(mul)
NUMBER_OPER(div)
NUMBER_OPER(remdiv)
NUMBER_OPER(mod)
NUMBER_OPER(pow)
#undef NUMBER_OPER

uint8_t number_comp(number_t, number_t);

#endif
