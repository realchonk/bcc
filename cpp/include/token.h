#ifndef FILE_CPP_TOKEN_H
#define FILE_CPP_TOKEN_H
#include <stdbool.h>
#include <stdio.h>

enum token_type {
   TK_WORD,
   TK_STRING,
   TK_NUMBER,
   TK_WHITESPACE,
   TK_NEWLINE,
   TK_PUNCT,
   TK_EOF,
   TK_UNKNOWN,
};

struct token {
   enum token_type type;
   const char* begin;
   const char* end;
};

extern const char* token_type_str[];
struct token* tokenize(const char* lines);
bool is_directive(const char* line);

#endif /* FILE_CPP_TOKEN_H */
