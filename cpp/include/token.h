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

#ifndef FILE_CPP_TOKEN_H
#define FILE_CPP_TOKEN_H
#include <stdbool.h>
#include <ctype.h>

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

#define isname1(ch) (isalpha(ch) || (ch) == '_')
#define isname(ch) (isname1(ch) || isdigit(ch))

int isodigit(int);

#endif /* FILE_CPP_TOKEN_H */
