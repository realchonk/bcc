#include <string.h>
#include "buf.h"
#include "dir.h"
#include "cpp.h"

static struct directive dirs[] = {
   { .name = "define",  .handler = dir_define,  true },
   { .name = "undef",   .handler = dir_undef,   true },
   { .name = "include", .handler = dir_include, true },
   { .name = "ifdef",   .handler = dir_ifdef,   false },
   { .name = "ifndef",  .handler = dir_ifndef,  false },
   { .name = "endif",   .handler = dir_endif,   false },
   { .name = "else",    .handler = dir_else,    false },
   { .name = "error",   .handler = dir_error,   false },
};

struct directive* get_dir(const char* name, size_t len) {
   for (size_t i = 0; i < arraylen(dirs); ++i) {
      if (!strncmp(name, dirs[i].name, len))
         return &dirs[i];
   }
   return NULL;
}
bool dir_error(size_t linenum, const char* line, struct token* tokens, size_t num_tks, FILE* out) {
   (void)line;
   (void)out;
   if (num_tks < 1) {
      warn(linenum, "#error expects a message");
      return false;
   }
   warn(linenum, "#error: %s", tokens[0].begin);
   return false;
}
