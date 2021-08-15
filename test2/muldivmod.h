//  Copyright (C) 2021 Benjamin Stürz
//  
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//  
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//  
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

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


