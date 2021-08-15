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

#include <sys/stat.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include "strint.h"
#include "dir.h"
#include "cpp.h"

// TODO: make configurable through something like includes.conf

static const char* system_includes[] = {
   "/usr/include",
   "/usr/local/include",
   NULL,
};
static const char* user_includes[] = {
   ".",
   NULL,
};

const char** cmdline_includes = NULL;

static char* full_search_include(const char* name, const char** includes);

bool dir_include(size_t linenum, const char* line, struct token* tokens, size_t num_tks, FILE* out) {
   (void)line;
   if (num_tks < 1) {
   expected_ip:
      warn(linenum, "expected include path");
      return false;
   }
   istr_t name;
   char* path;
   if (tokens->type == TK_STRING) {
      name = strrint(tokens->begin + 1, tokens->end - 1);
      path = full_search_include(name, user_includes);
      if (!path) {
         warn(linenum, "failed to find include \"%s\"", name);
         return false;
      }
   } else {
      const char* s = tokens[0].begin;
      while (isspace(*s)) ++s;
      if (*s != '<')
         goto expected_ip;
      const char* begin = ++s;
      while (*s && *s != '>') ++s;
      if (*s != '>')
         goto expected_ip;
      name = strrint(begin, s);
      path = full_search_include(name, system_includes);
      if (!path) {
         warn(linenum, "failed to find include <%s>", name);
         return false;
      }
   }

   FILE* file = fopen(path, "r");
   if (!file) {
      warn(linenum, "failed to open '%s': %s", path, strerror(errno));
      return false;
   }
   const char* saved_name = source_name;
   source_name = path;
   const int ec = run_cpp(file, out);
   source_name = saved_name;
   fclose(file);
   free(path);
   return ec == 0;
}

static bool file_exists(const char* path) {
   struct stat st;
   return stat(path, &st) == 0;
}

static char* search_include(const char* name, const char** includes) {
   if (!includes) return NULL;
   const size_t len_name = strlen(name);
   while (*includes) {
      char* new_path = malloc(strlen(*includes) + len_name + 2);
      assert(new_path != NULL);

      strcpy(new_path, *includes);
      strcat(new_path, "/");
      strcat(new_path, name);

      if (file_exists(new_path)) {
         char* path = realpath(new_path, NULL);
         free(new_path);
         return path;
      }

      ++includes;
   }
   return NULL;
}

static char* full_search_include(const char* name, const char** includes) {
   char* path = search_include(name, cmdline_includes);
   if (path) return path;
   return search_include(name, includes);
}
