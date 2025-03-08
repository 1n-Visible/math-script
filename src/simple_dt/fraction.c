#include "rt_values/fraction.h"

Fraction fraction(int64_t m, int64_t n) {
    if (n==0)
        return (Fraction){m, 0};
    if (m==0)
        return (Fraction){0, 1};

    if (n<0) {m=-m, n=-n;}
    uint64_t d=gcd((uint64_t)labs(m), (uint64_t)n);
    return (Fraction){m/(int64_t)d, (uint64_t)n/d};
}

bool Fraction_sign(Fraction f) {
    return f.m<0;
}


bool Fraction_to_bool(Fraction f) {
    return f.m!=0;
}

int64_t Fraction_to_int(Fraction f) {
    return f.m/f.n; //TODO: floor
}

double Fraction_to_double(Fraction f) {
    return (double)f.m/f.n;
}

char *Fraction_to_str(Fraction f) {
    char *string=calloc(45, sizeof(char));
    sprintf(string, "%ld/%lu", f.m, f.n);
    return string;
}


int64_t Fraction_floor(Fraction f) {
    return floor((double)f.m/f.n);
}

int64_t Fraction_ceil(Fraction f) {
    return ceil((double)f.m/f.n);
}

int64_t Fraction_round(Fraction f) {
    return round((double)f.m/f.n);
}


Fraction Fraction_pos(Fraction f) {
    return f;
}

Fraction Fraction_neg(Fraction f) {
    f.m=-f.m;
    return f;
}

Fraction Fraction_abs(Fraction f) {
    return fraction(labs(f.m), f.n);
}

Fraction Fraction_sqr(Fraction f) {
    return (Fraction){f.m*f.m, f.n*f.n};
}

Fraction Fraction_cube(Fraction f) {
    return (Fraction){f.m*f.m*f.m, f.n*f.n*f.n};
}

double Fraction_sqrt(Fraction f) {
    return sqrt((double)f.m/f.n);
}

#define FRACT_OPER(name, m, n) \
Fraction Fraction_##name(Fraction f1, Fraction f2) {return fraction(m, n);}

FRACT_OPER(add, f1.m*(int64_t)f2.n + f2.m*(int64_t)f1.n, f1.n*f2.n)

FRACT_OPER(sub, f1.m*(int64_t)f2.n - f2.m*(int64_t)f1.n, f1.n*f2.n)

FRACT_OPER(mul, f1.m*f2.m, f1.n*f2.n)

FRACT_OPER(truediv, f1.m*(int64_t)f2.n, f2.m*(int64_t)f1.n)

int64_t Fraction_div(Fraction f1, Fraction f2) {
    return idiv(f1.m*(int64_t)f2.n, f2.m*(int64_t)f1.n);
}

FRACT_OPER(mod,
    f1.m*(int64_t)f2.n - f2.m*(int64_t)f1.n*Fraction_div(f1, f2),
    f1.n*f2.n
)

Fraction Fraction_pow(Fraction f, int8_t power) {
    if (power<0) {
        power=-power;
        f=fraction(f.n, f.m);
    } else {
        f=fraction(f.m, f.n);
    }
    
    int64_t m=1; uint64_t n=1;
    for (int8_t i=0; i<abs(power); i++) {
        m*=f.m; n*=f.n;
    }
    return (Fraction){m, n};
}

#undef FRACT_OPER


uint8_t Fraction_comp(Fraction f1, Fraction f2) {
    int64_t a = f1.m*(int64_t)f2.n, b = f2.m*(int64_t)f1.n;
    return (a<b) | (a>b)<<1;
}
