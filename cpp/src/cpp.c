#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include "strint.h"
#include "token.h"
#include "macro.h"
#include "cpp.h"
#include "buf.h"
#include "dir.h"

static bool do_cpp_stuff(size_t linenum, const char* line, struct token* tokens, FILE* out) {
   const size_t num_tks = buf_len(tokens);
   size_t tki = 0;
   while (tokens[tki].type == TK_WHITESPACE)
      ++tki;
   ++tki;
   while (tokens[tki].type == TK_WHITESPACE)
      ++tki;

   if (tki >= num_tks) {
      warn(linenum, "expected word, got newline");
      return false;
   }

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
   ++tki;
   if (tki < num_tks && tokens[tki].type == TK_WHITESPACE)
      ++tki;

   return dir->handler(linenum, line, tokens + tki, num_tks - tki, out);
}

int run_cpp(FILE* in, FILE* out) {
   char** lines = read_lines(in);
   
   for (size_t i = 0; i < buf_len(lines); ++i) {
      const char* line = lines[i];
      struct token* tokens = tokenize(line);

      if (is_directive(line)) {
         do_cpp_stuff(i, line, tokens, out);
         buf_free(tokens);
         continue;
      }

      for (size_t j = 0; j < buf_len(tokens); ++j) {
         const struct token tk = tokens[j];
         if (tk.type == TK_WORD) {
            const istr_t word = strnint(tk.begin, tk.end - tk.begin);
            const struct macro* m = get_macro(word);
            if (m) {
               fputs(m->text, out);
               continue;
            }
         }
         for (const char* s = tk.begin; s != tk.end; ++s)
            fputc(*s, out);
      }

      fputc('\n', out);
      buf_free(tokens);
   }


   free_lines(lines);
   return 0;
}

void warn(size_t linenum, const char* msg, ...) {
   va_list ap;
   va_start(ap, msg);

   fprintf(stderr, "bcpp: %zu: ", linenum + 1);
   vfprintf(stderr, msg, ap);
   fputc('\n', stderr);

   va_end(ap);
}
