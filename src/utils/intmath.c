#include "utils/intmath.h"

int64_t imin(int64_t value1, int64_t value2) {
    return (value1>value2)? value2: value1;
}

int64_t imax(int64_t value1, int64_t value2) {
    return (value1<value2)? value2: value1;
}

int64_t idiv(int64_t m, int64_t n) {
    return floor((double)m/n);
}

int64_t imod(int64_t m, int64_t n) {
    return m-n*floor((double)m/n);
}

uint64_t isqrt(uint64_t n) {
    if (!n) return 0;
    if (n<=3) return 1;
    
    uint64_t rem=0, root=0;
    for (short i = 64>>1; i>0; i--) {
        rem = (rem<<2) | (n>>(64-2));
        n<<=2; root<<=1;
        if (root<rem) {
            rem-=(root | 1); root+=2;
        }
    }
    return root>>1;
}

short ilog2(uint64_t n) {
    if (!n) return -1;
    
    short i = 0;
    while (n>>=1) i++;
    return i;
}

bool isprime(uint64_t n) {
    if (n<2) return false;
    if (n<4) return true;
    if (!(n&1)) return false;
    
    uint64_t sqrt_n=isqrt(n);
    for (uint64_t p=3; p<=sqrt_n; p+=2) {
        if (!(n%p)) return false;
    }
    return true;
}

uint64_t *primes_length(size_t length) {
    uint64_t *primes = calloc(length, sizeof(uint64_t));
    primes[0]=2; primes[1]=3;
    
    uint64_t k=3, p, q;
    bool is_prime;
    
    size_t i, j;
    for (i=2; i<length; i++) {
        do {
            k+=2; q=isqrt(k); is_prime=true; //TODO: p>sqrt(k) -> p*p>k
            for (j=0; j<length; j++) {
                p=primes[j];
                if (p>q) break;
                if (!(k%p)) {is_prime=false; break;}
            }
        } while (!is_prime);
        primes[i]=k;
    }
    return primes;
}

uint64_t *prime_factors(uint64_t n, short *length) {
    *length = ilog2(n);
    size_t primes_len = imin(isqrt(n), 1<<16), index = 1;
    uint64_t *factors = calloc(*length, sizeof(uint64_t)),
        p=1, *primes = calloc(primes_len, sizeof(uint64_t));
    
    primes[0] = 2; *length = 0;
    while (!(n&1)) {
        n>>=1;
        factors[(*length)++]=2;
    }
    
    size_t i; bool is_prime;
    while (n!=1) {
        do {
            p+=2; is_prime=true;
            for (i=0; i<index; i++)
                if (!(p%primes[i])) {is_prime=false; break;}
        } while (!is_prime);
        if (index<primes_len) primes[index++]=p;
        
        if (p*p>n) {
            factors[(*length)++]=n;
            break;
        }
        
        while (!(n%p)) {
            n/=p;
            factors[(*length)++]=p;
        }
    }
    
    free(primes);
    return factors;
}

uint64_t factors_to_number(uint64_t *factors, short length) {
    uint64_t n=1;
    for (short i=0; i<length; i++)
        n*=factors[i];
    
    return n;
}

uint64_t gcd(uint64_t n1, uint64_t n2) {
    if (n1==n2)
        return n1;
    
    uint64_t p, res=1;
    while (not (n1&1 or n2&1)) {
        n1>>=1; n2>>=1;
        res<<=1;
    }
    
    if (n2<n1) {
        p=n1; n1=n2; n2=p;
    }
    for (p=3; p<=n1 and n1!=1 and n2!=1; p+=2) {
        while (not (n1%p or n2%p)) {
            n1/=p; n2/=p;
            res*=p;
        }
    }
    
    return res;
}
