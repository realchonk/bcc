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
struct cmdline_arg* cpp_args = NULL;
bool nostdinc = false;

#define TARGET_INCLUDE_DIR COMPILERDIR "/include"

FILE* run_cpp(const char* source_name) {
   int pipes[2];
   if (pipe(pipes) != 0)
      panic("failed to create pipes");

   char** args = NULL;
   buf_push(args, strdup(cpp_path));
   if (!console_colors)
      buf_push(args, strdup("-C"));
   buf_push(args, strdup("-E"));
   for (size_t i = 0; i < buf_len(cpp_args); ++i) {
      const struct cmdline_arg arg = cpp_args[i];
      const size_t len_buffer = (arg.arg ? strlen(arg.arg) : 0) + 3;
      char* buffer = malloc(len_buffer);
      if (!buffer)
         panic("failed to allocate argument");
      snprintf(buffer, len_buffer, "-%c%s", arg.option, arg.arg);
      buf_push(args, buffer);
   }
   buf_push(args, strdup("-o"));
   buf_push(args, strdup("-"));
   if (!nostdinc)
      buf_push(args, strdup("-I" TARGET_INCLUDE_DIR));
   buf_push(args, strdup(source_name));
   buf_push(args, NULL);

   if (verbose) {
      fprintf(stderr, "Calling %s with:", cpp_path);
      for (size_t i = 0; args[i]; ++i) {
         fprintf(stderr, " %s", args[i]);
      }
      fputc('\n', stderr);
   }

   const pid_t pid = fork();
   if (pid < 0) panic("failed to fork()");
   else if (pid == 0) {
      close(pipes[0]);
      close(1);
      if (dup(pipes[1]) != 1)
         panic("failed to duplicate file descriptor");

      execvp(cpp_path, args);
      panic("failed to exec %s", cpp_path);
   } else {
      close(pipes[1]);
      FILE* file = fdopen(pipes[0], "r");

      if (!file)
         panic("failed to open pipes[0]");
      
      int wstatus;
      waitpid(pid, &wstatus, 0);
      if (WIFEXITED(wstatus)) {
         const int ec = WEXITSTATUS(wstatus);
         if (ec == 0) {
            return file;
         } else {
            fprintf(stderr, "bcc: bcpp exited with code %d\n", ec);
            return NULL;
         }
      } else if (WIFSIGNALED(wstatus)) {
         fprintf(stderr, "bcc: bcpp killed by signal %d\n", WTERMSIG(wstatus));
      } else if (WIFSTOPPED(wstatus)) {
         fprintf(stderr, "bcc: bcpp stopped by signal %d\n", WSTOPSIG(wstatus));
      } else if (WIFCONTINUED(wstatus)) {
         fprintf(stderr, "bcc: bcpp continued\n");
      } else {
         fprintf(stderr, "bcc: bcpp failed by undetermined cause\n");
      }
      exit(254);
   }

}

void define_macro2(const char* n, const char* v) {
   const size_t len = strlen(n) + strlen(v) + 2;
   char* str = malloc(len);
   if (!str)
      panic("failed to allocate string");
   snprintf(str, len, "%s=%s", n, v);

   struct cmdline_arg arg;
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
   struct cmdline_arg arg;
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
