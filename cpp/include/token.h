#ifndef FILE_CPP_TOKEN_H
#define FILE_CPP_TOKEN_H
#include <stdio.h>

enum token_type {
   TOKEN_NAME = 256,
};
struct token {
   int type;
   union {
      const char* str;
      int ch;
   };
};

void print_token(FILE*, const struct token*);

#endif /* FILE_CPP_TOKEN_H */
