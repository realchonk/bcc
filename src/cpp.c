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

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "error.h"
#include "cpp.h"
#include "bcc.h"
#include "buf.h"

const char* cpp_path = BCPP_PATH;
char** predef_macros = NULL;
char** includes = NULL;

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
      for (size_t i = 0; i < buf_len(includes); ++i) {
         buf_push(args, strdup("-I"));
         buf_push(args, includes[i]);
      }
      buf_push(args, strdup("-D__bcc__=1"));
      buf_push(args, strdup("-D__" BCC_FULL_ARCH "__=1"));
      for (size_t i = 0; i < buf_len(predef_macros); ++i) {
         buf_push(args, strdup("-D"));
         buf_push(args, predef_macros[i]);
      }
      buf_push(args, strdup("-o"));
      buf_push(args, strdup("-"));
      buf_push(args, strdup(source_name));
      buf_push(args, NULL);

#if 0
      fprintf(stderr, "bcpp");
      for (size_t i = 0; args[i]; ++i) {
         fprintf(stderr, " %s", args[i]);
      }
      fputc('\n', stderr);
#endif

      execvp(cpp_path, args);
      panic("failed to exec %s", cpp_path);
   } else {
      close(pipes[1]);
      FILE* file = fdopen(pipes[0], "r");
      if (!file) panic("failed to open pipes[0]");
      else return file;
   }

}
