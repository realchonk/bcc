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

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "target.h"
#include "linker.h"
#include "config.h"
#include "error.h"
#include "buf.h"

const char* linker_path = GNU_LD;
struct cmdline_arg* linker_args = NULL;
bool nostartfiles = false, nolibc = false;

// compiler-specific libraries/startfiles
#define bcc_crt(c)   COMPILERDIR "/crt" c ".o"
#define libbcc       COMPILERDIR "/libbcc.a"

// system libraries/startfiles
#define SYSLIBSDIR   TARGETDIR "/lib"
#define libc_crt(c)  SYSLIBSDIR  "/crt" c ".o"
#define libc         SYSLIBSDIR  "/libc.a"

// TODO: figure out which option disables -lbcc

// crt1.o      : libc
// crti.o      : libc
// crtbegin.o  : libbcc
// ...
// libbcc.a    : libbcc
// libc.a      : libc
// crtend.o    : libbcc
// crtn.o      : libc

int run_linker(const char* output_name, const char** objects) {
   if (!output_name) {
      output_name = "a.out";
   }

   // ACTUAL LINKING PART
   char** args = NULL;
   buf_push(args, strdup(linker_path));
   // TODO: fixme
   //buf_push(args, "--no-warn-search-mismatch");
   //buf_push(args, "--no-warn-mismatch");
   buf_push(args, get_ld_abi());
   buf_push(args, "-L" COMPILERDIR);
   buf_push(args, "-L" SYSLIBSDIR);
   buf_push(args, "-o");
   buf_push(args, strdup(output_name));
   if (!nostartfiles) {
      buf_push(args, libc_crt("1"));
      buf_push(args, libc_crt("i"));
      buf_push(args, bcc_crt("begin"));
   }

   for (size_t i = 0; i < buf_len(objects); ++i) {
      buf_push(args, strdup(objects[i]));
   }
 
#if HAS_LIBC
   if (!nolibc) {
      buf_push(args, libc);
   }
#endif

   if (!nostartfiles) {
      buf_push(args, bcc_crt("end"));
      buf_push(args, libc_crt("n"));
#if HAS_LIBBCC
      buf_push(args, "-lbcc");
#endif
   }

   for (size_t i = 0; i < buf_len(linker_args); ++i) {
      const struct cmdline_arg arg = linker_args[i];
      const size_t len_buffer = (arg.arg ? strlen(arg.arg) : 0) + 3;
      char* buffer = malloc(len_buffer);
      if (!buffer)
         panic("failed to allocate buffer");
      snprintf(buffer, len_buffer, "-%c%s", arg.option, arg.arg);
      buf_push(args, buffer);
   }
   buf_push(args, NULL);
   
   if (verbose) {
      fprintf(stderr, "Calling %s with:", linker_path);
      for (size_t i = 0; args[i]; ++i) {
         fprintf(stderr, " %s", args[i]);
      }
      fputc('\n', stderr);
   }

   const pid_t pid = fork();
   if (pid < 0)
      panic("failed to fork()");
   else if (pid == 0) {
      execv(linker_path, args);
      perror("bcc: failed to invoke linker");
      _exit(1);
   } else {
      int wstatus;
      waitpid(pid, &wstatus, 0);
      if (WIFEXITED(wstatus))
         return WEXITSTATUS(wstatus);
      else panic("failed to wait for linker");
   }
   return 1;
}
