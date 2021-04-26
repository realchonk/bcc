#ifndef FILE_TOKEN_H
#define FILE_TOKEN_H
#include <stddef.h>
#include <stdint.h>
#include "strint.h"

typedef long double fpmax_t;

enum token_type {
   TK_DUMMY,
   TK_INTEGER, TK_FLOAT, TK_STRING, TK_CHARACTER, TK_NAME,
   
   TK_EOF,
   NUM_TOKENS,
};

extern const char* token_type_str[NUM_TOKENS];

struct source_pos {
   const char* file;
   size_t line;
   size_t column;
};

struct token {
   enum token_type type;
   struct source_pos pos;
   union {
      istr_t str;
      uintmax_t iVal;
      fpmax_t fVal;
      int ch;
   };
};

#endif /* FILE_TOKEN_H */
