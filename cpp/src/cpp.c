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

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "expand.h"
#include "strint.h"
#include "token.h"
#include "macro.h"
#include "cpp.h"
#include "buf.h"
#include "dir.h"

extern bool suppress_code;

static bool do_cpp_stuff(size_t linenum, const char* line, struct token* tokens, FILE* out) {
   const size_t num_tks = buf_len(tokens);
   size_t tki = 0;
   while (tokens[tki].type == TK_WHITESPACE)
      ++tki;
   ++tki;
   while (tokens[tki].type == TK_WHITESPACE)
      ++tki;

   if (tki >= num_tks || tokens[tki].type == TK_NEWLINE)
      return true;

   const struct token tk_dir = tokens[tki];
   if (tk_dir.type != TK_WORD) {
      warn(linenum, "expected word, got %s", token_type_str[tk_dir.type]);
      return false;
   }
   const struct directive* dir = get_dir(tk_dir.begin, tk_dir.end - tk_dir.begin);
   if (!dir) {
      warn(linenum, "invalid pre-processor directive '%s'", strrint(tk_dir.begin, tk_dir.end));
      return false;
   }
   if (dir->suppressable && suppress_code) return true;
   ++tki;
   if (tki < num_tks && tokens[tki].type == TK_WHITESPACE)
      ++tki;

   return dir->handler(linenum, line, tokens + tki, num_tks - tki, out);
}

int run_cpp(FILE* in, FILE* out) {
   struct line_pair* lines = read_lines(in);

   if (failed)
      return 1;

   char* buf;
   size_t len_buf;
   FILE* tmp = open_memstream(&buf, &len_buf);
   
   for (size_t i = 0; i < buf_len(lines); ++i) {
      const struct line_pair pair = lines[i];
      struct token* tokens = tokenize(pair.line);

      if (is_directive(pair.line)) {
         failed |= !do_cpp_stuff(pair.linenum, pair.line, tokens, tmp);
         buf_free(tokens);
         continue;
      }

      buf_free(tokens);
      if (suppress_code) continue;

      char* e = expand(lines[i].linenum, lines[i].line, NULL, NULL, false);
      if (!e) {
         warn(i, "failed to expand");
         failed = true;
         continue;
      }
      fputs(e, tmp);
      fputc('\n', tmp);
      buf_free(e);
   }
   free_lines(lines);

   fclose(tmp);

   if (!failed) {
      fwrite(buf, 1, len_buf, out);
   }
   free(buf);

   return failed;
}
noreturn void panic_impl(const char* func, const char* fmt, ...) {
   va_list ap;
   va_start(ap, fmt);

   const int errno_saved = errno;

   if (console_color)
      fputs("\033[31;1m", stderr);
   fprintf(stderr, "bcpp: %s(): ", func);
   if (console_color)
      fputs("\033[0m", stderr);
   vfprintf(stderr, fmt, ap);
   if (errno_saved) fprintf(stderr, ": %s\n", strerror(errno_saved));
   else fputc('\n', stderr);

   va_end(ap);
   abort();
}

