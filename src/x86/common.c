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
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "cmdline.h"
#include "target.h"
#include "config.h"
#include "error.h"

struct machine_option mach_opts[] = {
   { "clean-asm", "Generate cleaned assembly", 0, .bVal = false },
};
const size_t num_mach_opts = arraylen(mach_opts);

int assemble(const char* source, const char* output) {
   const pid_t pid = fork();
   assert(pid >= 0);
   if (pid == 0) {
      verbose_execl(GNU_AS, GNU_AS, "-msyntax=intel", "-o", output, source, NULL);
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

char* get_ld_abi(void) {
#if BITS == 32
   return strdup("-melf_i386");
#else
   return strdup("-melf_x86_64");
#endif
}

bool emit_prepare(void) {
   return true;
}
