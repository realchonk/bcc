#include "target.h"
#include "error.h"
#include "regs.h"

// 16 - REGSIZE
#if BITS == 32
#define SP_REGSIZE "12"
#else
#define SP_REGSIZE "8"
#endif

struct builtin_func builtin_funcs[] = {
   {
      .name = "__check_sp",
      .code =
         "addi sp, sp, -16\n"
         SW " a0, " SP_REGSIZE "(sp)\n"
         "andi a0, sp, 15\n"
         "bne a0, x0, __check_sp.failed\n"
         LW " a0, " SP_REGSIZE "(sp)\n"
         "addi sp, sp, 16\n"
         "ret\n"
         "__check_sp.failed:\n"
         "la a0, __check_sp.msg\n"
         "call puts\n"
         "call abort\n"
         ".section .rodata\n"
         "__check_sp.msg: .string \"Stack is not aligned.\\012\"\n"
         ".section .text\n"
   }

};
const size_t num_builtin_funcs = arraylen(builtin_funcs);
