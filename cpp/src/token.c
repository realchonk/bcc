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

#include <ctype.h>
#include "token.h"
#include "buf.h"

const char* token_type_str[] = {
   "word",
   "string",
   "number",
   "whitespace",
   "newline",
   "punct",
   "eof",
   "unknown",
};

static int iscpunct(int ch) {
   return ch != '"' && ch != '\'' && ispunct(ch);
}
int isodigit(int ch) {
   return ch >= '0' && ch <= '7';
}

static const char* escape_seq(const char* str) {
   switch (*str) {
   case 'a':
   case 'b':
   case 'f':
   case 'n':
   case 'r':
   case 't':
   case 'v':
   case '"':
   case '\'':
   case '\\':
      ++str;
      break;
   case 'x':
      ++str;
      for (size_t i = 0; i < 2 && isxdigit(*str); ++i, ++str);
      break;
   case '0':
      ++str;
      for (size_t i = 0; i < 3 && isodigit(*str); ++i, ++str);
      break;
   default:
      ++str;
      break;
   }
   return str;
}

#define ret(type, begin) { *tk = (struct token){ type, begin, str }; return str; }

static const char* next(const char* str, struct token* tk) {
   const char* begin = str;
   char ch = *str;
   if (!ch) {
      ret(TK_NEWLINE, str);
   } else if (ch == '"' || ch == '\'') {
      ++str;
      while (*str && *str != ch) {
         if (*str == '\\')
            str = escape_seq(++str);
         else ++str;
      }
      ++str;
      ret(TK_STRING, begin);
   } else if (isdigit(ch)) {
      while (isdigit(*str)) ++str;
      if (*str == '.') {
         ++str;
         while (isdigit(*str)) ++str;
      }
      if (*str == 'e' || *str == 'E') {
         ++str;
         while (isdigit(*str)) ++str;
      }
      ret(TK_NUMBER, begin);
   } else if (isspace(ch)) {
      while (isspace(*str)) ++str;
      ret(TK_WHITESPACE, begin);
   } else if (isalpha(ch) || ch =='_') {
      while (isalnum(*str) || *str == '_') ++str;
      ret(TK_WORD, begin);
   } else if (ispunct(ch)) {
      while (iscpunct(*str)) ++str;
      ret(TK_PUNCT, begin);
   } else {
      ++str;
      ret(TK_UNKNOWN, str - 1);
   }
}

struct token* tokenize(const char* str) {
   struct token* tokens = NULL;
   struct token tk;
   do {
      str = next(str, &tk);
      buf_push(tokens, tk);
   } while (tk.type != TK_NEWLINE);
   return tokens;
}
bool is_directive(const char* line) {
   while (isspace(*line)) ++line;
   return *line == '#';
}
