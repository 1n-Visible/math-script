#include "rt_values/number.h"

const number_t number_zero = {
    .real={.integer=0}, .imag={.integer=0},
    .real_t=NUMT_INT, .imag_t=NUMT_INT
};

const number_t number_unit = {
    .real={.integer=1}, .imag={.integer=0},
    .real_t=NUMT_INT, .imag_t=NUMT_INT
};

const number_t number_imagunit = {
    .real={.integer=0}, .imag={.integer=1},
    .real_t=NUMT_INT, .imag_t=NUMT_INT
};

const number_t number_ReInf = {
    .real={.floating=INFINITY}, .imag={.integer=0},
    .real_t=NUMT_FLOAT, .imag_t=NUMT_INT
};

const number_t number_ImInf = {
    .real={.integer=0}, .imag={.floating=INFINITY},
    .real_t=NUMT_INT, .imag_t=NUMT_FLOAT
};

const number_t number_ReNaN = {
    .real={.floating=NAN}, .imag={.integer=0},
    .real_t=NUMT_FLOAT, .imag_t=NUMT_INT
};

const number_t number_ImNaN = {
    .real={.integer=0}, .imag={.floating=NAN},
    .real_t=NUMT_INT, .imag_t=NUMT_FLOAT
};

const number_t number_NaN = {
    .real={.floating=NAN}, .imag={.floating=NAN},
    .real_t=NUMT_FLOAT, .imag_t=NUMT_FLOAT
};


static number_t normalize_number(number_t num) {
    num.real=normalize_Real(num.real, num.real_t, &num.real_t);
    num.imag=normalize_Real(num.imag, num.imag_t, &num.imag_t);
    return num;
}

number_t number_from_int(int64_t integer, bool is_real) {
    union _Real real={.integer=integer};
    number_t num;
    if (is_real)
        num = (number_t){
            .real=real, .real_t=NUMT_INT, .imag=_Real_zero, .imag_t=NUMT_INT
        };
    else
        num = (number_t){
            .real=_Real_zero, .real_t=NUMT_INT, .imag=real, .imag_t=NUMT_INT
        };
    
    return normalize_number(num);
}

number_t number_from_float(double floating, bool is_real) {
    union _Real real={.floating=floating};
    number_t num;
    if (is_real)
        num = (number_t){
            .real=real, .real_t=NUMT_FLOAT, .imag=_Real_zero, .imag_t=NUMT_INT
        };
    else
        num = (number_t){
            .real=_Real_zero, .real_t=NUMT_INT, .imag=real, .imag_t=NUMT_FLOAT
        };
    
    return normalize_number(num);
}


bool number_to_bool(number_t num) {
    return not (_Real_to_bool(num.real, num.real_t) or
                _Real_to_bool(num.imag, num.imag_t));
}

static const char *_sign_table[] = {"+", " + ", "-", " - "};

wchar_t *number_to_str(number_t num, enum num_expr_t expr_t, bool spaces) {
    union _Real imag=num.imag; enum num_type imag_t=num.imag_t;
    bool real_sign=_Real_sign(num.real, num.real_t);
    bool imag_sign=_Real_sign(imag, imag_t);
    
    size_t strsize;
    wchar_t *real_str, *string;
    if (not _Real_to_bool(imag, imag_t)) {
        real_str=_Real_to_str(num.real, num.real_t);
        if (expr_t!=NUMEXPR_RIGHT or !real_sign)
            return real_str;
        
        strsize=wcslen(real_str)+3;
        string=calloc(strsize, sizeof(wchar_t));
        swprintf(string, strsize, L"(%ls)", real_str);
        
        free(real_str);
        return string;
    }
    
    wchar_t *imag_str;
    if (not _Real_to_bool(num.real, num.real_t)) {
        imag_str=_Real_to_str(imag, imag_t);
        
        strsize=wcslen(imag_str)+4;
        string=calloc(strsize, sizeof(wchar_t));
        swprintf(
            string, strsize,
            (expr_t!=NUMEXPR_RIGHT or !imag_sign)? L"%lsi": L"(%lsi)", imag_str
        );
        
        free(imag_str);
        return string;
    }
    
    imag=_Real_abs(imag, imag_t, &imag_t);
    imag_str=_Real_to_str(imag, imag_t);
    real_str=_Real_to_str(num.real, num.real_t);
    
    strsize=wcslen(real_str)+wcslen(imag_str)+8;
    string=calloc(strsize, sizeof(wchar_t));
    swprintf(
        string, strsize, (expr_t==NUMEXPR_NONE)? L"%ls%s%lsi": L"(%ls%s%lsi)",
        real_str, _sign_table[(imag_sign<<1) | spaces], imag_str
    );
    
    free(real_str);
    free(imag_str);
    return string;
}

bool number_is_real(number_t num) {
    return not _Real_to_bool(num.imag, num.imag_t);
}


#define NUMBER_UNARY(name) number_t number_##name(number_t num)

#define NUMBER_CONV(name) \
NUMBER_UNARY(name) { \
    num.real=_Real_##name(num.real, num.real_t, &num.real_t); \
    num.imag=_Real_##name(num.imag, num.imag_t, &num.imag_t); \
    return num; \
}

NUMBER_CONV(to_floating)
NUMBER_CONV(floor)
NUMBER_CONV(ceil)
NUMBER_CONV(round)
#undef NUMBER_CONV

NUMBER_UNARY(pos) {
    return num;
}

NUMBER_UNARY(neg) {
    num.real=_Real_neg(num.real, num.real_t, &num.real_t);
    num.imag=_Real_neg(num.imag, num.imag_t, &num.imag_t);
    return num;
}

NUMBER_UNARY(conj) {
    num.imag=_Real_neg(num.imag, num.imag_t, &num.imag_t);
    return num;
}

NUMBER_UNARY(perp) {
    enum num_type t;
    union _Real r=_Real_neg(num.real, num.real_t, &t);
    
    num.real=num.imag; num.real_t=num.imag_t;
    num.imag=r; num.imag_t=t;
    return num;
}

static union _Real number_abssqr(number_t num, enum num_type *ret_type) {
    num.real=_Real_sqr(num.real, num.real_t, &num.real_t);
    num.imag=_Real_sqr(num.imag, num.imag_t, &num.imag_t);
    return _Real_add(num.real, num.real_t, num.imag, num.imag_t, ret_type);
}

static union _Real number_optabs(number_t num, enum num_type *ret_type) {
    if (not _Real_to_bool(num.imag, num.imag_t))
        return _Real_abs(num.real, num.real_t, ret_type);
    if (not _Real_to_bool(num.real, num.real_t))
        return _Real_abs(num.imag, num.imag_t, ret_type);
    
    num.real=number_abssqr(num, &num.real_t);
    return _Real_sqrt(num.real, num.real_t, ret_type);
}

static double number_abs_dbl(number_t num) {
    enum num_type type;
    union _Real real=number_optabs(num, &type);
    return _Real_to_double(real, type);
}

NUMBER_UNARY(abs_sqr) {
    num.real=number_abssqr(num, &num.real_t);
    num.imag=_Real_zero; num.imag_t=NUMT_INT;
    return normalize_number(num);
}

NUMBER_UNARY(abs) {
    num.real=number_optabs(num, &num.real_t);
    num.imag=_Real_zero; num.imag_t=NUMT_INT;
    return normalize_number(num);
}

double number_arg_cos(number_t num) {
    return _Real_to_double(num.real, num.real_t)/number_abs_dbl(num);
}

double number_arg_sin(number_t num) {
    return _Real_to_double(num.imag, num.imag_t)/number_abs_dbl(num);
}

NUMBER_UNARY(arg_tan);

NUMBER_UNARY(sqr) {
    return normalize_number(number_mul(num, num));
}

NUMBER_UNARY(cube) {
    return normalize_number(number_mul(number_mul(num, num), num));
}

NUMBER_UNARY(sqrt) {
    double r=number_abs_dbl(num), a=_Real_to_double(num.real, num.real_t);

    if (not _Real_to_bool(num.imag, num.imag_t)) {
        if (not _Real_sign(num.real, num.real_t)) {
            num.real.floating=sqrt(r); num.real_t=NUMT_FLOAT;
            num.imag=_Real_zero; num.imag_t=NUMT_INT;
        } else {
            num.imag.floating=sqrt(r); num.imag_t=NUMT_FLOAT;
            num.real=_Real_zero; num.real_t=NUMT_INT;
        }
        return normalize_number(num);
    }

    num.real.floating=sqrt((r+a)/2);
    num.imag.floating=sqrt((r-a)/2)*(_Real_sign(num.imag, num.imag_t)? -1: 1);
    num.real_t=NUMT_FLOAT;
    num.imag_t=NUMT_FLOAT;
    return normalize_number(num);
}

#undef NUMBER_UNARY


#define NUMBER_OPER(name) \
number_t number_##name(number_t num1, number_t num2)

NUMBER_OPER(add) {
    number_t num;
    num.real=_Real_add(
        num1.real, num1.real_t,
        num2.real, num2.real_t, &num.real_t
    );
    num.imag=_Real_add(
        num1.imag, num1.imag_t,
        num2.imag, num2.imag_t, &num.imag_t
    );
    return normalize_number(num);
}

NUMBER_OPER(sub) {
    number_t num;
    num.real=_Real_sub(
        num1.real, num1.real_t,
        num2.real, num2.real_t, &num.real_t
    );
    num.imag=_Real_sub(
        num1.imag, num1.imag_t,
        num2.imag, num2.imag_t, &num.imag_t
    );
    return normalize_number(num);
}

// (a+bi)*(c+di) = (ac-bd) + (bc+ad)i
NUMBER_OPER(mul) {
    number_t num;
    union _Real r1, r2; 
    enum num_type t1, t2;

    r1=_Real_mul(num1.real, num1.real_t, num2.real, num2.real_t, &t1);
    r2=_Real_mul(num1.imag, num1.imag_t, num2.imag, num2.imag_t, &t2);
    num.real=_Real_sub(r1, t1, r2, t2, &num.real_t); // ac-bd
    
    r1=_Real_mul(num1.imag, num1.imag_t, num2.real, num2.real_t, &t1);
    r2=_Real_mul(num1.real, num1.real_t, num2.imag, num2.imag_t, &t2);
    num.imag=_Real_add(r1, t1, r2, t2, &num.imag_t); // bc+ad
    return normalize_number(num);
}

// (a+bi)/(c+di) = ((ac+bd) + (bc-ad)i)/(c²+d²)
NUMBER_OPER(truediv) {
    number_t num;
    enum num_type t1, t2, td;
    union _Real r1, r2, d=number_abssqr(num2, &td);
    
    r1=_Real_mul(num1.real, num1.real_t, num2.real, num2.real_t, &t1);
    r2=_Real_mul(num1.imag, num1.imag_t, num2.imag, num2.imag_t, &t2);
    num.real=_Real_add(r1, t1, r2, t2, &num.real_t); // ac+bd
    
    r1=_Real_mul(num1.imag, num1.imag_t, num2.real, num2.real_t, &t1);
    r2=_Real_mul(num1.real, num1.real_t, num2.imag, num2.imag_t, &t2);
    num.imag=_Real_sub(r1, t1, r2, t2, &num.imag_t); // bc-ad
    
    num.real=_Real_truediv(num.real, num.real_t, d, td, &num.real_t);
    num.imag=_Real_truediv(num.imag, num.imag_t, d, td, &num.imag_t);
    return normalize_number(num);
}

NUMBER_OPER(div) {
    return number_floor(number_truediv(num1, num2));
}

NUMBER_OPER(mod) {
    return number_sub(num1, number_mul(number_div(num1, num2), num2));
}

static number_t number_optpow(number_t num, int64_t power) {
    bool sign=(power<0);
    if (sign) power=-power;
    
    number_t ret_num = number_unit;
    while (power) { //square-multiply algorithm
        if (power & 1) ret_num=number_mul(ret_num, num);
        num=number_sqr(num); power>>=1;
    }
    
    if (not sign) return ret_num;
    return number_truediv(number_unit, ret_num);
}

NUMBER_OPER(pow) {
    if (number_is_real(num2) and num2.real_t==NUMT_INT)
        return normalize_number(number_optpow(num1, num2.real.integer));
    
    // (a+bi)^(c+di) = e^(cln(r) - dθ) (cos(dln(r) + cθ) + isin(dln(r) + cθ))
    enum num_type t;
    double c = _Real_to_double(num2.real, num2.real_t);
    double d = _Real_to_double(num2.imag, num2.imag_t);
    double ln_r = log(_Real_to_double(number_abssqr(num1, &t), t))/2;
    double theta = atan2(
        _Real_to_double(num1.imag, num1.imag_t),
        _Real_to_double(num1.real, num1.real_t)
    );

    double r = exp(c*ln_r - d*theta);
    double phi = d*ln_r + c*theta;
    return (number_t){
        .real={.floating=r*cos(phi)}, .imag={.floating=r*sin(phi)},
        .real_t=NUMT_FLOAT, .imag_t=NUMT_FLOAT
    };
}

#undef NUMBER_OPER

uint8_t number_comp(number_t num1, number_t num2) {
    return _Real_comp(num1.real, num1.real_t, num2.real, num2.real_t) |
           _Real_comp(num1.imag, num1.imag_t, num2.imag, num2.imag_t);
}
