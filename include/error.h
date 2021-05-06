#ifndef FILE_ERROR_H
#define FILE_ERROR_H
#include <stdnoreturn.h>
#include "token.h"

#define arraylen(a) (sizeof(a) / sizeof((a)[0]))

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

noreturn void panic(const char*, ...) PRINTF_FMT_WARN(1, 2);
noreturn void lex_error(const char*, ...) PRINTF_FMT_WARN(1, 2);
noreturn void parse_error(const struct source_pos*, const char*, ...) PRINTF_FMT_WARN(2, 3);
void parse_warn(const struct source_pos*, const char*, ...) PRINTF_FMT_WARN(2, 3);

#endif /* FILE_ERROR_H */

