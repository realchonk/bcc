#ifndef FILE_DIR_H
#define FILE_DIR_H
#include <stdbool.h>
#include <stdio.h> 
#include "token.h"

struct directive {
   const char* name;
   bool(*handler)(size_t linenum, const char* line, struct token* tokens, size_t num_tks, FILE* out);
   bool suppressable;
};

struct directive* get_dir(const char* name, size_t len);

// pre-processor directives

#define define_dir(name) bool dir_##name(size_t, const char*, struct token*, size_t, FILE*)

define_dir(define);
define_dir(undef);
define_dir(include);
define_dir(ifdef);
define_dir(ifndef);
define_dir(endif);
define_dir(else);
define_dir(error);
define_dir(if);
define_dir(elif);

#endif /* FILE_DIR_H */
