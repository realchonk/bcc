#ifndef FILE_PARSER_H
#define FILE_PARSER_H
#include <stdnoreturn.h>
#include "expr.h"
#include "stmt.h"
#include "scope.h"

noreturn void parse_error(const struct source_pos*, const char*, ...);

#endif /* FILE_PARSER_H */
