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

#ifndef __LIMITS_H__
#define __LIMITS_H__
#include "bcc-config.h"

#define CHAR_BIT     8
#define SCHAR_MIN    (__bcc_min_schar)
#define SCHAR_MAX    (__bcc_max_schar)
#define UCHAR_MAX    (__bcc_max_uchar)
#define SHRT_MIN     (__bcc_min_short)
#define SHRT_MAX     (__bcc_max_short)
#define USHRT_MAX    (__bcc_max_ushort)
#define INT_MIN      (__bcc_min_int)
#define INT_MAX      (__bcc_max_int)
#define UINT_MAX     (__bcc_max_uint)
#define LONG_MIN     (__bcc_min_long)
#define LONG_MAX     (__bcc_max_long)
#define ULONG_MAX    (__bcc_max_ulong)

#define MB_LEN_MAX   1

#if __bcc_char_signed
#define CHAR_MIN     SCHAR_MIN
#define CHAR_MAX     SCHAR_MAX
#else
#define CHAR_MIN     0
#define CHAR_MAX     UCHAR_MAX
#endif

#endif /* __LIMITS_H__ */
