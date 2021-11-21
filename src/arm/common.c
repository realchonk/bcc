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

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include "binutils.h"
#include "cmdline.h"
#include "target.h"
#include "config.h"
#include "error.h"

struct flag_option mach_opts[] = {
   BINUTILS_MACH_OPTS,
   { "cpu", "The target CPU (default: " DEF_CPU ")", 2, .sVal = DEF_CPU },
   { "abi", "The target ABI (default: " DEF_ABI ")", 2, .sVal = DEF_ABI },
};

const size_t num_mach_opts = arraylen(mach_opts);

int assemble(const char* source, const char* output) {
   const pid_t pid = fork();
   if (pid < 0) panic("failed to fork()");
   if (pid == 0) {
      char* path_as = strdup(get_flag_opt("path-as")->sVal);
      verbose_execl(path_as, path_as, "-o", output, source, NULL);
      perror("bcc: failed to invoke assembler");
      _exit(1);
   } else {
      int wstatus;
      waitpid(pid, &wstatus, 0);
      if (WIFEXITED(wstatus))
         return WEXITSTATUS(wstatus);
      panic("failed to wait for assembler");
   }
}


