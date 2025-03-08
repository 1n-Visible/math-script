#include "rt_values/_real.h"

const union _Real _Real_zero = {.integer=0};
const union _Real _Real_unit = {.integer=1};
const union _Real _Real_Inf = {.floating=INFINITY};
const union _Real _Real_NaN = {.floating=NAN};

union _Real normalize_Real(union _Real real, enum num_type type,
                                             enum num_type *ret_type) {
    int64_t m;
    switch (type) {
        case NUMT_INT:
            *ret_type=NUMT_INT;
            return real;
        case NUMT_FLOAT:
            m=real.floating;
            if (real.floating!=(double)m) {
                *ret_type=NUMT_FLOAT;
                return real;
            }
            
            *ret_type=NUMT_INT;
            return (union _Real){.integer=m};
    }

    Fraction fract=fraction(real.fraction.m, real.fraction.n);
    m=fract.m;
    uint64_t n=fract.n;
    if (n==0) {
        *ret_type=NUMT_FLOAT;
        return (union _Real){.floating=INFINITY*m};
    }

    if (m==0 or real.fraction.n==1) {
        *ret_type=NUMT_INT;
        return (union _Real){.integer=m};
    }

    *ret_type=NUMT_FRACT;
    return (union _Real){.fraction=fract};
}

bool _Real_sign(union _Real real, enum num_type type) {
    switch (type) {
        case NUMT_INT: return real.integer<0;
        case NUMT_FLOAT: return real.floating<0.0;
    }
    return Fraction_sign(real.fraction);
}


bool _Real_to_bool(union _Real real, enum num_type type) {
    switch (type) {
        case NUMT_INT: return real.integer;
        case NUMT_FLOAT: return real.floating!=0.0;
    }
    return real.fraction.m;
}

int64_t _Real_to_int(union _Real real, enum num_type type) {
    switch (type) {
        case NUMT_INT: return real.integer;
        case NUMT_FLOAT: return (int64_t)real.floating; //TODO: floor
    }
    return Fraction_to_int(real.fraction);
}

double _Real_to_double(union _Real real, enum num_type type) {
    if (type==NUMT_FLOAT)
        return real.floating;
    if (type==NUMT_FRACT)
        return Fraction_to_double(real.fraction);
    return (double)real.integer;
}

Fraction _Real_to_Fraction(union _Real real, enum num_type type) {
    if (type==NUMT_FRACT)
        return real.fraction; //TODO: double
    return (Fraction){real.integer, 1ul};
}

wchar_t *_Real_to_str(union _Real real, enum num_type type) {
    wchar_t *string;
    switch (type) {
        case NUMT_INT:
            string=calloc(25, sizeof(wchar_t));
            swprintf(string, 25, L"%lld", real.integer);
            return string;
        case NUMT_FLOAT: //TODO: remote formatting system
            string=calloc(50, sizeof(wchar_t));
            swprintf(string, 50, L"%lf", real.floating);
            return string;
    }
    
    char *fract_str=Fraction_to_str(real.fraction);
    size_t strsize=strlen(fract_str)+1;
    string=calloc(strsize, sizeof(wchar_t));
    swprintf(string, strsize, L"%s", fract_str);
    free(fract_str);
    return string;
}


#define REAL_UNARY(name) \
union _Real _Real_##name(union _Real real, enum num_type type, \
                         enum num_type *ret_type)

REAL_UNARY(to_floating) {
    real.floating=_Real_to_double(real, type);
    *ret_type=NUMT_FLOAT;
    return real;
}

#define REAL_ROUNDING(name) \
REAL_UNARY(name) { \
    *ret_type=NUMT_INT; \
    switch (type) { \
        case NUMT_FLOAT: real.integer=name(real.floating); \
        case NUMT_INT: return real; \
    } \
    real.integer=Fraction_##name(real.fraction); \
    return real; \
}

REAL_ROUNDING(floor)
REAL_ROUNDING(ceil)
REAL_ROUNDING(round)
#undef REAL_ROUNDING

REAL_UNARY(pos) {
    *ret_type=type;
    return real;
}

REAL_UNARY(neg) {
    *ret_type=type;
    switch (type) {
        case NUMT_INT:
            real.integer*=-1;
            return real;
        case NUMT_FLOAT:
            real.floating*=-1;
            return real;
    }
    real.fraction=Fraction_neg(real.fraction);
    return real;
}

REAL_UNARY(abs) {
    *ret_type=type;
    switch (type) {
        case NUMT_INT:
            real.integer=labs(real.integer);
            return real;
        case NUMT_FLOAT:
            real.floating=fabs(real.floating);
            return real;
    }
    real.fraction=Fraction_abs(real.fraction);
    return real;
}

REAL_UNARY(sqr) {
	int64_t i;
	double f;
    switch (type) {
        case NUMT_INT:
            i=real.integer;
            real.integer=i*i; break;
        case NUMT_FLOAT:
            f=real.floating;
            real.floating=f*f; break;
        case NUMT_FRACT:
            real.fraction=Fraction_sqr(real.fraction);
    }
    
    *ret_type=type;
    return real;
}

REAL_UNARY(cube) {
	int64_t i;
	double f;
    switch (type) {
        case NUMT_INT:
            i=real.integer;
            real.integer=i*i*i; break;
        case NUMT_FLOAT:
            f=real.floating;
            real.floating=f*f*f; break;
        case NUMT_FRACT:
            real.fraction=Fraction_cube(real.fraction);
    }
    
    *ret_type=type;
    return real;
}

REAL_UNARY(sqrt) {
    *ret_type=NUMT_FLOAT;
    real.floating=sqrt(_Real_to_double(real, type));
    return real;
}

#undef REAL_UNARY


#define REAL_OPER(name) \
union _Real _Real_##name(union _Real r1, enum num_type t1, \
                         union _Real r2, enum num_type t2, \
                                         enum num_type *ret_type)

#define HANDLE_FLOAT(value) if (t1==NUMT_FLOAT or t2==NUMT_FLOAT) { \
    *ret_type=NUMT_FLOAT; \
    return (union _Real){.floating=value}; \
}

#define HANDLE_INT(value) if (t1==NUMT_INT and t2==NUMT_INT) { \
    *ret_type=NUMT_INT; \
    return (union _Real){.integer=value}; \
}

#define HANDLE_FRACT(name) { \
    *ret_type=NUMT_FRACT; \
    return (union _Real){.fraction = Fraction_##name( \
        _Real_to_Fraction(r1, t1), _Real_to_Fraction(r2, t2) \
    )}; \
};

REAL_OPER(add) {
    HANDLE_FLOAT(_Real_to_double(r1, t1) + _Real_to_double(r2, t2));
    HANDLE_INT(r1.integer + r2.integer);
    HANDLE_FRACT(add);
}

REAL_OPER(sub) {
    HANDLE_FLOAT(_Real_to_double(r1, t1) - _Real_to_double(r2, t2));
    HANDLE_INT(r1.integer - r2.integer);
    HANDLE_FRACT(sub);
}

REAL_OPER(mul) {
    HANDLE_FLOAT(_Real_to_double(r1, t1) * _Real_to_double(r2, t2));
    HANDLE_INT(r1.integer * r2.integer);
    HANDLE_FRACT(mul);
}

REAL_OPER(truediv) {
    HANDLE_FLOAT(_Real_to_double(r1, t1) / _Real_to_double(r2, t2));
    HANDLE_FRACT(truediv);
}

REAL_OPER(div) {
    *ret_type=NUMT_INT;
    if (t1==NUMT_FLOAT or t2==NUMT_FLOAT or (t1==NUMT_INT and t2==NUMT_INT))
        return (union _Real){
            .integer=floor(_Real_to_double(r1, t1)/_Real_to_double(r2, t2))
        };
    HANDLE_FRACT(div);
}

REAL_OPER(mod) {
    if (t1!=NUMT_FLOAT and t2!=NUMT_FLOAT)
        HANDLE_FRACT(mod);

    union _Real div_int=_Real_div(r1, t1, r2, t2, ret_type);
    *ret_type=NUMT_FLOAT;
    return (union _Real){.floating =
        _Real_to_double(r1, t1) - _Real_to_double(r2, t2)*div_int.integer
    };
}

//NotImplemented
REAL_OPER(pow); //TODO

#undef REAL_OPER
#undef HANDLE_FLOAT
#undef HANDLE_INT
#undef HANDLE_FRACT

uint8_t _Real_comp(union _Real r1, enum num_type t1,
                   union _Real r2, enum num_type t2) {
    if (t1!=NUMT_FLOAT and t2!=NUMT_FLOAT)
        return Fraction_comp(_Real_to_Fraction(r1, t1),
                             _Real_to_Fraction(r2, t2));
    
    double f1=_Real_to_double(r1, t1), f2=_Real_to_double(r2, t2);
    return (f1<f2) | (f1>f2)<<1;
}
