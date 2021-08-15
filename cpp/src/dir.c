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
   { .name = "error",   .handler = dir_error,   true  },
   { .name = "if",      .handler = dir_if,      false },
   { .name = "elif",    .handler = dir_elif,    false },
};

struct directive* get_dir(const char* name, size_t len) {
   for (size_t i = 0; i < arraylen(dirs); ++i) {
      const size_t len_dir = strlen(dirs[i].name);
      if (len_dir != len) continue;
      else if (!memcmp(name, dirs[i].name, len))
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
