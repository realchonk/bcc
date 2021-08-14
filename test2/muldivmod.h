#include <stdbool.h>
#include <stdio.h>

// This file was used to generate the mul/div/mod builtins for RISC-V

#define divsi(name, type) \
type name(type a, type b) { \
    bool negative = false; \
    if (a < 0) negative = !negative, a = -a; \
    if (b < 0) negative = !negative, b = -b; \
    type quo = 0; \
    while (a >= b) { \
        a -= b; \
        ++quo; \
    } \
    if (negative) \
        quo = -quo; \
    return quo; \
}

#define divui(name, type) \
unsigned type name(unsigned type a, unsigned type b) { \
    unsigned type quo = 0; \
    while (a >= b) { \
        a -= b; \
        ++quo; \
    } \
    return quo; \
} 

#define modsi(name, type) \
type name(type a, type b) { \
    bool negative = false; \
    if (a < 0) negative = !negative, a = -a; \
    if (b < 0) negative = !negative, b = -b; \
    while (a >= b) { \
        a -= b; \
    } \
    if (negative) \
        a = -a; \
    return a; \
}

#define modui(name, type) \
unsigned type name(unsigned type a, unsigned type b) { \
    while (a >= b) { \
        a -= b; \
    } \
    return a; \
} 

#define mulsi(name, type) \
type name(type a, type b) { \
    bool negative = false; \
    if (a < 0) a = -a, negative = !negative; \
    if (b < 0) b = -b, negative = !negative; \
    type r = 0; \
    while (b) { \
        if (b & 1) { \
            r += a; \
        } \
        a <<= 1; \
        b >>= 1; \
    } \
    if (negative) \
        r = -r; \
    return r; \
}
#define mului(name, type) \
unsigned type name(unsigned type a, unsigned type b) { \
    unsigned type r = 0; \
    while (b) { \
        if (b & 1) { \
            r += a; \
        } \
        a <<= 1; \
        b >>= 1; \
    } \
    return r; \
}

//mulsi(mulsi32, long)
//mului(mului32, long)


