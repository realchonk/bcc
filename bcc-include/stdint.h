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

#ifndef __STDINT_H__
#define __STDINT_H__
#include "bcc-config.h"

// TYPES

typedef __builtin_int8_t   int8_t;
typedef __builtin_int16_t  int16_t;
typedef __builtin_int32_t  int32_t;
typedef __builtin_uint8_t  uint8_t;
typedef __builtin_uint16_t uint16_t;
typedef __builtin_uint32_t uint32_t;

#if __BCC_BITS >= 64
typedef __builtin_int64_t  int64_t;
typedef __builtin_uint64_t uint64_t;
typedef int64_t            intmax_t;
typedef uint64_t           uintmax_t;
#else
typedef int32_t            intmax_t;
typedef uint32_t           uintmax_t;
#endif

typedef int8_t             int_fast8_t;
typedef int16_t            int_fast16_t;
typedef int32_t            int_fast32_t;
typedef uint8_t            uint_fast8_t;
typedef uint16_t           uint_fast16_t;
typedef uint32_t           uint_fast32_t;

#if __BCC_BITS >= 64
typedef int64_t            int_fast64_t;
typedef uint64_t           uint_fast64_t;
#endif

typedef intmax_t           intptr_t;
typedef uintmax_t          uintptr_t;

// TODO: add [u]int_leastN_t


// LIMITS

#define INT8_MIN           (-128)
#define INT8_MAX           ( 127)
#define UINT8_MAX          ( 255)

#define INT16_MIN           (-32768)
#define INT16_MAX           ( 32767)
#define UINT16_MAX          ( 65535)

#define INT32_MIN           (-2147483648)
#define INT32_MAX           ( 2147483647)
#define UINT32_MAX          ( 4294967295)

#if __BCC_BITS >= 64
#define INT64_MIN           (-9223372036854775808)
#define INT64_MAX           ( 9223372036854775807)
#define UINT64_MAX          ( 18446744073709551615)
#endif

#define INT_FAST8_MIN      INT8_MIN
#define INT_FAST8_MAX      INT8_MAX
#define UINT_FAST8_MAX     UINT8_MAX

#define INT_FAST16_MIN      INT16_MIN
#define INT_FAST16_MAX      INT16_MAX
#define UINT_FAST16_MAX     UINT16_MAX

#define INT_FAST32_MIN      INT32_MIN
#define INT_FAST32_MAX      INT32_MAX
#define UINT_FAST32_MAX     UINT32_MAX

#if __BCC_BITS >= 64
#define INT_FAST64_MIN      INT64_MIN
#define INT_FAST64_MAX      INT64_MAX
#define UINT_FAST64_MAX     UINT64_MAX
#endif


#if __BCC_BITS >= 64
#define INTMAX_MIN         INT64_MIN
#define INTMAX_MAX         INT64_MAX
#define UINTMAX_MAX        UINT64_MAX
#else
#define INTMAX_MIN         INT32_MIN
#define INTMAX_MAX         INT32_MAX
#define UINTMAX_MAX        UINT32_MAX
#endif

#define INTPTR_MIN         INTMAX_MIN
#define INTPTR_MAX         INTMAX_MAX
#define UINTPTR_MAX        UINTPTR_MAX

#define SIZE_MAX           UINTPTR_MAX
#define PTRDIFF_MIN        INTMAX_MIN
#define PTRDIFF_MAX        INTMAX_MAX

#endif /* __STDINT_H__ */
