#define _GNU_SOURCE 1
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include "riscv/cpu.h"
#include "target.h"
#include "error.h"
#include "regs.h"
#include "as.h"

struct machine_option mach_opts[] = {
   { "cpu", "The target CPU", 2, .sVal = DEF_MACH },
   { "abi", "The target ABI", 2, .sVal = DEF_ABI  },
};

const size_t num_mach_opts = arraylen(mach_opts);
extern const char* gnu_as;

// TODO: add -mas=... option
int assemble(const char* source, const char* output) {
   const pid_t pid = fork();
   if (pid < 0) panic("failed to fork()");
   if (pid == 0) {
      char* mabi = NULL;
      if (asprintf(&mabi, "-mabi=%s", get_mach_opt("abi")->sVal) < 0) {
         perror("bcc: failed to invoke asprintf");
         _exit(1);
      }
      execlp(gnu_as, gnu_as, mabi, "-o", output, source, NULL);
      perror("bcc: failed to invoke assembler");
      _exit(1);
   } else {
      int wstatus;
      waitpid(pid, &wstatus, 0);
      if (WIFEXITED(wstatus)) return WEXITSTATUS(wstatus);
      panic("failed to wait for assembler");
   }
}
