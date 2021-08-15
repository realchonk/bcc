#ifndef FILE_EXPAND_H
#define FILE_EXPAND_H
#include "macro.h"

struct var {
   istr_t name;
   const char* text;
};

char* expand(size_t linenum, const char* begin, const char* end);
char* expand2(size_t linenum, const char* s, struct var* vars, const char* macro_name);
char* expand_macro(const struct macro*);
char* expand_macro_func(const struct macro*, char** params);

#endif /* FILE_EXPAND_H */
