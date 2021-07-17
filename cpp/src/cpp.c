#include <stdio.h>
#include "token.h"
#include "cpp.h"
#include "buf.h"

int run_cpp(FILE* in, FILE* out) {
   char** lines = read_lines(in);
   
   for (size_t i = 0; i < buf_len(lines); ++i) {

      if (*lines[i] == '#') {
         // TODO: implement actual pre-processing
      }

      struct token* tokens = tokenize(lines[i]);
      for (size_t j = 0; j < buf_len(tokens); ++j) {
         const struct token tk = tokens[j];
         for (const char* str = tk.begin; str != tk.end; ++str)
            putchar(*str);
      }
      buf_free(tokens);
      putchar('\n');
   }


   free_lines(lines);
   return 0;
}


