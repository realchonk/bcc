#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "error.h"
#include "cpp.h"

const char* cpp_path = "bcpp";

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

      execlp(cpp_path, "bcpp", "-E", "-o", "-", source_name, NULL);
      panic("failed to exec bcpp");
   } else {
      close(pipes[1]);
      FILE* file = fdopen(pipes[0], "r");
      if (!file) panic("failed to open pipes[0]");
      else return file;
   }

}
