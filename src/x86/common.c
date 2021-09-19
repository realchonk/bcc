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
//
//  SEE ALSO:
//  - https://github.com/gcc-mirror/gcc/blob/16e2427f50c208dfe07d07f18009969502c25dc8/gcc/config/i386/linux64.h

#include <sys/wait.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "cmdline.h"
#include "target.h"
#include "config.h"
#include "error.h"
#include "bcc.h"

struct flag_option mach_opts[] = {
   { "clean-asm", "Generate cleaned assembly", 0, .bVal = false },
   { "stack-check", "Check the stack on every function entry", 0, .bVal = false }, // stub
};
const size_t num_mach_opts = arraylen(mach_opts);

int assemble(const char* source, const char* output) {
   const pid_t pid = fork();
   assert(pid >= 0);
   if (pid == 0) {
      char* path_as = strdup(get_flag_opt("path-as")->sVal);
      verbose_execl(path_as, path_as, "-msyntax=intel", "-o", output, source, NULL);
      perror("bcc: failed to invoke assembler");
      _exit(1);
   } else {
      int wstatus;
      waitpid(pid, &wstatus, 0);
      if (WIFEXITED(wstatus))
         return WEXITSTATUS(wstatus);
      else panic("failed to wait for assembler");
   }
}

bool emit_prepare(void) {
   return true;
}

