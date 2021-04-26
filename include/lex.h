#ifndef FILE_LEX_H
#define FILE_LEX_H
#include <stdbool.h>
#include <stdio.h>
#include "token.h"

void lexer_init(FILE*, const char*);
void lexer_free(void);

struct token lexer_peek(void);
struct token lexer_next(void);

bool lexer_eof(void);
bool lexer_matches(enum token_type);
bool lexer_match(enum token_type);
struct token lexer_expect(enum token_type);
void lexer_skip(void);

#endif /* FILE_LEX_H */
