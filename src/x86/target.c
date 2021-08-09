#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include "target.h"
#include "config.h"
#include "error.h"

const struct target_info target_info = {
   .name = BCC_FULL_ARCH,
   .size_byte = 1,
   .size_char = 1,
   .size_short = 2,
   .size_int = 4,
   .size_long = BITS * 8,
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
   
#if BITS == 32
   .min_long   = INT32_MIN,
   .max_long   = INT32_MAX,
   .max_ulong  = UINT32_MAX,
#else
   .min_long   = INT64_MIN,
   .max_long   = INT64_MAX,
   .max_ulong  = UINT64_MAX,
#endif

   .unsigned_char = false,

   .fend_asm = "asm",
   .fend_obj = "o",

   .ptrdiff_type = INT_INT,
   .has_c99_array = true,

   .size_int8 = INT_BYTE,
   .size_int16 = INT_SHORT,
   .size_int32 = INT_INT,
#if BITS == 32
   .size_int64 = NUM_INTS,
#else
   .size_int64 = INT_LONG,
#endif

#if BITS == 32
   .max_immed = INT32_MAX,
   .min_immed = INT32_MIN,
#else
   .max_immed = INT64_MAX,
   .min_immed = INT64_MIN,
#endif

};

#if BITS == 32
#define ELFxx "elf32"
#else
#define ELFxx "elf64"
#endif

int assemble(const char* source, const char* output) {
   const pid_t pid = fork();
   if (pid < 0) panic("failed to fork");
   if (pid == 0) {
      execlp(NASM, NASM, "-f", ELFxx, "-o", output, source, NULL);
      perror("bcc: failed to invoke nasm");
      _exit(1);
   } else {
      int wstatus;
      waitpid(pid, &wstatus, 0);
      if (WIFEXITED(wstatus)) return WEXITSTATUS(wstatus);
      panic("failed to wait for nasm");
   }
}
