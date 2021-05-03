#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include "target.h"
#include "error.h"

const struct target_info target_info = {
   .name = "i386",
   .size_byte = 1,
   .size_char = 1,
   .size_short = 2,
   .size_int = 4,
   .size_long = 8,
   .size_float = 4,
   .size_double = 8,
   .size_pointer = 4,

   
   .min_byte   = INT8_MIN,
   .max_byte   = INT8_MAX,
   .max_ubyte  = UINT8_MAX,
   
   .min_char   = INT8_MIN,
   .max_char   = INT8_MAX,
   .max_uchar  = UINT8_MAX,
   
   .min_short  = INT16_MIN,
   .max_short  = INT16_MAX,
   .max_ushort = UINT16_MAX,
   
   .min_int    = INT32_MIN,
   .max_int    = INT32_MAX,
   .max_uint   = UINT32_MAX,
   
   .min_long   = INT64_MIN,
   .max_long   = INT64_MAX,
   .max_ulong  = UINT64_MAX,

   .unsigned_char = false,

   .fend_asm = "asm",
   .fend_obj = "o",

   .ptrdiff_type = INT_INT,
   .has_c99_array = false,
};

int assemble(const char* source, const char* output) {
   const pid_t pid = fork();
   if (pid < 0) panic("failed to fork");
   if (pid == 0) {
      execlp("nasm", "nasm", "-f", "elf32", "-o", output, source, NULL);
      perror("bcc: failed to invoke nasm");
      _exit(1);
   } else {
      int wstatus;
      waitpid(pid, &wstatus, 0);
      if (WIFEXITED(wstatus)) return WEXITSTATUS(wstatus);
      panic("failed to wait for nasm");
   }
}



