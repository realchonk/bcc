#include "macro.h"
#include "dir.h"
#include "cpp.h"

bool suppress_code = false;

static bool* layers = NULL;

static void update_suppress(void) {
   suppress_code = false;
   for (size_t i = 0; i < buf_len(layers); ++i) {
      if (!layers[i]) {
         suppress_code = true;
         return;
      }
   }
}

static bool ifdef_impl(size_t linenum, struct token* tokens, size_t num_tks, bool negate) {
   if (num_tks < 1) {
   expected_name:
      warn(linenum, "expected macro name");
      return false;
   }
   if (tokens[0].type != TK_WORD)
      goto expected_name;

   istr_t name = strrint(tokens[0].begin, tokens[0].end);
   bool exists = !negate && (get_macro(name) != NULL);
   buf_push(layers, exists);
   update_suppress();
   return true;

}

bool dir_ifdef(size_t linenum, const char* line, struct token* tokens, size_t num_tks, FILE* out) {
   (void)out;
   (void)line;
   return ifdef_impl(linenum, tokens, num_tks, false);
}
bool dir_ifndef(size_t linenum, const char* line, struct token* tokens, size_t num_tks, FILE* out) {
   (void)out;
   (void)line;
   return ifdef_impl(linenum, tokens, num_tks, true);
}
bool dir_endif(size_t linenum, const char* line, struct token* tokens, size_t num_tks, FILE* out) {
   (void)line;
   (void)tokens;
   (void)num_tks;
   (void)out;
   if (buf_len(layers) < 1) {
      warn(linenum, "invalid #endif");
      return false;
   }
   buf_pop(layers);
   update_suppress();
   return true;
}
