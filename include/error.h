#ifndef FILE_ERROR_H
#define FILE_ERROR_H
#include <stdnoreturn.h>
#include "token.h"

#define arraylen(a) (sizeof(a) / sizeof((a)[0]))

noreturn void panic(const char*, ...);
noreturn void lex_error(const char*, ...);
noreturn void parse_error(const struct source_pos*, const char*, ...);
void parse_warn(const struct source_pos*, const char*, ...);

#endif /* FILE_ERROR_H */
