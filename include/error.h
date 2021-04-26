#ifndef FILE_ERROR_H
#define FILE_ERROR_H
#include <stdnoreturn.h>
#include "token.h"

noreturn void panic(const char*, ...);
noreturn void lex_error(const char*, ...);
noreturn void parse_error(const struct source_pos*, const char*, ...);

#endif /* FILE_ERROR_H */
