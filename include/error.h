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

#ifndef FILE_ERROR_H
#define FILE_ERROR_H
#include "token.h"
#include "config.h"
#if HAVE_STDNORETURN_H
#include <stdnoreturn.h>
#else
#define noreturn
#endif

#if defined(__GNUC__)
#define fallthrough __attribute__((fallthrough))
#else
#define fallthough
#endif

#if defined(__clang)
#define PRINTF_FMT_WARN(fmt_idx, va_idx) \
   __attribute__((__format__(__printf__, fmt_idx, va_idx)))
#elif defined(__GNUC__)
#define PRINTF_FMT_WARN(fmt_idx, va_idx) \
   __attribute__((format(printf, fmt_idx, va_idx)))
#else
#define PRINTF_FMT_WARN(fmt_idx, va_idx)
#endif

noreturn void panic_impl(const char*, const char*, ...) PRINTF_FMT_WARN(2, 3);
noreturn void parse_error(const struct source_pos*, const char*, ...) PRINTF_FMT_WARN(2, 3);
void parse_warn(const struct source_pos*, const char*, ...) PRINTF_FMT_WARN(2, 3);

#define panic(...) panic_impl(__func__, __VA_ARGS__)

#endif /* FILE_ERROR_H */

