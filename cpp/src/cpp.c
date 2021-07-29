#include <stdio.h>
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

      if (suppress_code) continue;

      for (size_t j = 0; j < buf_len(tokens); ++j) {
         const struct token tk = tokens[j];
         if (tk.type == TK_WORD) {
            const istr_t word = strnint(tk.begin, tk.end - tk.begin);
            const struct macro* m = get_macro(word);
            if (m) {
               fputs(m->text, tmp);
               continue;
            }
         }
         for (const char* s = tk.begin; s != tk.end; ++s)
            fputc(*s, tmp);
      }

      fputc('\n', tmp);
      buf_free(tokens);
   }
   free_lines(lines);

   fclose(tmp);

   if (!failed)
      fwrite(buf, 1, len_buf, out);
   free(buf);

   return failed;
}

