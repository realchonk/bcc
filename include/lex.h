//  Copyright (C) 2021 Benjamin Stürz
//  
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//  
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//  
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

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
