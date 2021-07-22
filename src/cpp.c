#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "error.h"
#include "cpp.h"
#include "bcc.h"
#include "buf.h"

const char* cpp_path = "bcpp";
char** includes;

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
      buf_push(args, strdup("bcpp"));
      if (!console_colors)
         buf_push(args, strdup("-C"));
      buf_push(args, strdup("-E"));
      buf_push(args, strdup("-o"));
      buf_push(args, strdup("-"));
      buf_push(args, strdup(source_name));
      for (size_t i = 0; i < buf_len(includes); ++i) {
         buf_push(args, strdup("-I"));
         buf_push(args, includes[i]);
      }
      buf_push(args, NULL);

      execvp(cpp_path, args);
      panic("failed to exec bcpp");
   } else {
      close(pipes[1]);
      FILE* file = fdopen(pipes[0], "r");
      if (!file) panic("failed to open pipes[0]");
      else return file;
   }

}
