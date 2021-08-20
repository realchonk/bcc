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

#include <string.h>
#include <ctype.h>
#include "token.h"
#include "cpp.h"

const char* source_name = NULL;

static int read_char2(FILE* file, size_t* linenum) {
   const int ch = fgetc(file);
   if (ch == '\n') {
      ++*linenum;
   }
   return ch;
}

static char str_delim = 0;
static bool escaping = false;
static int escape = 0;
static int escape_n = 0;
static int read_char(FILE* file, size_t* linenum) {
   int ch = read_char2(file, linenum);

   if (str_delim) {
      if (escaping) {
         switch (escape) {
         case 0:
            escape = ch;
            escape_n = 0;
            break;
         case '0':
         case '1':
         case '2':
         case '3':
         case '4':
         case '5':
         case '6':
         case '7':
            if (escape_n == 3 || !isodigit(ch))
               escape = escape_n = 0;
            else ++escape_n;
            break;
         case 'x':
            if (escape_n == 2 || !isxdigit(ch))
               escape = escape_n = 0;
            else ++escape_n;
            break;
         case 'u':
            if (escape_n == 4 || !isxdigit(ch))
               escape = escape_n = 0;
            else ++escape_n;
            break;
         case 'U':
            if (escape_n == 8 || !isxdigit(ch))
               escape = escape_n = 0;
            else ++escape_n;
            break;
         default:
            escaping = false;
            escape = 0;
            break;
         }
      } else if (ch == '\\') {
         escaping = true;
         escape = 0;
      } else if (ch == str_delim) {
         str_delim = 0;
      }
      return ch;
   } else if (ch == '"' || ch == '\'') {
      str_delim = ch;
      return ch;
   }

   if (ch == '/') {
      ch = read_char2(file, linenum);
      if (ch == '/') {
         while (!feof(file) && read_char2(file, linenum) != '\n');
         return read_char(file, linenum);
      } else if (ch == '*') {
         while (!feof(file)) {
            if (read_char2(file, linenum) == '*' && read_char2(file, linenum) == '/')
               return read_char(file, linenum);
         }
         return EOF;
      } else {
         return ungetc(ch, file), '/';
      }
   } else return ch;
}

static char* read_line(FILE* file, bool* eof, size_t* linenum) {
   char* line = NULL;
   int ch;
   while ((ch = read_char(file, linenum)) != EOF) {
   begin:;
      if (ch == '\n')
         break;
      else if (ch == '\\') {
         ch = read_char2(file, linenum);
         if (ch == '\n')
            continue;
         else if (ch == EOF) {
            warn(*linenum, "'\\' at end of file");
            *eof = true;
            break;
         }
         buf_push(line, '\\');
         goto begin;
      } else buf_push(line, ch);
   }
   if (line)
      buf_push(line, '\0');
   *eof |= (ch == EOF);
   return line;
}

// check if #line
static bool check_hline(const char* s, size_t* linenum) {
   while (isspace(*s)) ++s;
   if (*s != '#') return false;
   ++s;
   while (isspace(*s)) ++s;
   if (strncmp("line", s, 4) != 0)
      return false;
   s += 4;
   while (isspace(*s)) ++s;
   size_t nl = 0;
   while (isdigit(*s))
      nl = nl * 10 + (*s++ - '0');
   if (!nl)
      fail(*linenum, "expected valid line number, not '%c'", *s);
   *linenum = nl - 1;
   return true;
}

struct line_pair* read_lines(FILE* file) {
   struct line_pair* pairs = NULL;
   bool eof = false;
   size_t linenum = 0;
   do {
      struct line_pair pair;
      pair.linenum = linenum;
      pair.line = read_line(file, &eof, &linenum);
      if (pair.line) {
         if (check_hline(pair.line, &linenum))
            continue;
         buf_push(pairs, pair);
      }
   } while (!eof);
   return pairs;
}

void free_lines(struct line_pair* lines) {
   for (size_t i = 0; i < buf_len(lines); ++i) {
      buf_free(lines[i].line);
   }
   buf_free(lines);
}
void print_lines(FILE* file, const struct line_pair* lines) {
   for (size_t i = 0; i < buf_len(lines); ++i) {
      fprintf(file, "%s\n", lines[i].line);
   }
}

