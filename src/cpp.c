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

#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "target.h"
#include "config.h"
#include "error.h"
#include "cpp.h"
#include "bcc.h"
#include "buf.h"

const char* cpp_path = BCPP_PATH;
struct cpp_arg* cpp_args = NULL;

FILE* run_cpp(const char* source_name) {
   int pipes[2];
   if (pipe(pipes) != 0)
      panic("failed to create pipe");

   const pid_t pid = fork();
   if (pid < 0) panic("failed to fork()");
   else if (pid == 0) {
      close(pipes[0]);
      close(1);
      if (dup(pipes[1]) != 1)
         panic("failed to duplicate file descriptor");

      char** args = NULL;
      buf_push(args, strdup(cpp_path));
      if (!console_colors)
         buf_push(args, strdup("-C"));
      buf_push(args, strdup("-E"));
      for (size_t i = 0; i < buf_len(cpp_args); ++i) {
         char* s = malloc(3);
         if (!s) panic("failed to allocate argument");
         s[0] = '-';
         s[1] = cpp_args[i].option;
         s[2] = '\0';
         buf_push(args, s);
         if (cpp_args[i].arg)
            buf_push(args, strdup(cpp_args[i].arg));
      }
      buf_push(args, strdup("-o"));
      buf_push(args, strdup("-"));
      buf_push(args, strdup("-I" TARGET_INCLUDE_DIR));
      buf_push(args, strdup(source_name));
      buf_push(args, NULL);

      execvp(cpp_path, args);
      panic("failed to exec %s", cpp_path);
   } else {
      close(pipes[1]);
      FILE* file = fdopen(pipes[0], "r");
      
      int wstatus;
      waitpid(pid, &wstatus, 0);
      if (WIFEXITED(pid) || WEXITSTATUS(wstatus) != 0) {
         if (WEXITSTATUS(wstatus) == 139)
            fputs("bcc: bcpp crashed.\n", stderr);
         return fclose(file), NULL;
      }

      if (!file) panic("failed to open pipes[0]");
      else return file;
   }

}

void define_macro2(const char* n, const char* v) {
   const size_t len = strlen(n) + strlen(v) + 2;
   char* str = malloc(len);
   if (!str)
      panic("failed to allocate string");
   snprintf(str, len, "%s=%s", n, v);

   struct cpp_arg arg;
   arg.option = 'D';
   arg.arg = str;
   buf_push(cpp_args, arg); 
}
void define_macro2i(const char* n, intmax_t x) {
   char str[100];
   snprintf(str, sizeof(str), "%jd", x);
   define_macro2(n, str);
}
void define_macro2u(const char* n, uintmax_t x) {
   char str[100];
   snprintf(str, sizeof(str), "%ju", x);
   define_macro2(n, str);
}
void define_macro(const char* n) {
   struct cpp_arg arg;
   arg.option = 'D';
   arg.arg = n;
   buf_push(cpp_args, arg); 
}
void define_macros(void) {
   if (!target_info.has_c99_array) {
      define_macro("__STDC_NO_VLA__");
   }
   define_ctarget_macros();

   char* predef_macros = strdup(CPP_MACROS);
   char* macro = strtok(predef_macros, " ");
   while (macro != NULL) {
      define_macro(strdup(macro));
      macro = strtok(NULL, " ");
   }
}
