#include "target.h"
#include "error.h"
#include "regs.h"

// 16 - REGSIZE
#if BITS == 32
#define SP_REGSIZE "12"
#else
#define SP_REGSIZE "8"
#endif

#define gen_mului(nm) \
   { \
      .name = nm, \
      .code = \
         "mv a5, a0\n" \
         "mv a0, x0\n" \
         "beq a1, x0, " nm "_L5\n" \
         nm "_L4:\n" \
         "andi a4, a1, 1\n" \
         "srli a1, a1, 1\n" \
         "beq a4, x0, " nm "_L3\n" \
         "add a0, a0, a5\n" \
         nm "_L3:\n" \
         "slli a5, a5, 1\n" \
         "bne a1, x0, " nm "_L4\n" \
         "ret\n" \
         nm "_L5:\n" \
         "ret\n" \
   }
#define gen_mulsi(nm) \
   { \
      .name = nm, \
      .code = \
         "mv a5, a0\n" \
         "blt a0, x0, " nm "_L22\n" \
         "blt a1, x0, " nm "_L11\n" \
         "mv a3, x0\n" \
         "beq a1, x0, " nm "_L20\n" \
         nm "_L4:\n" \
         "mv a0, x0\n" \
         nm "_L8:\n" \
         "andi a4, a1, 1\n" \
         "srai a1, a1, 1\n" \
         "beq a4, x0, " nm "_L7\n" \
         "add a0, a0, a5\n" \
         nm "_L7:\n" \
         "slli a5, a5, 1\n" \
         "bne a1, x0, " nm "_L8\n" \
         "beq a3, x0, " nm "_L1\n" \
         "neg a0, a0\n" \
         nm "_L1:\n" \
         "ret\n" \
         nm "_L11:\n" \
         "li a3, 1\n" \
         "neg a1, a1\n" \
         "j " nm "_L4\n" \
         nm "_L22:\n" \
         "neg a5, a0\n" \
         "blt a1, x0, " nm "_L9\n" \
         "li a3, 1\n" \
         "bne a1, x0, " nm "_L4\n" \
         nm "_L20:\n" \
         "mv a0, x0\n" \
         "ret\n" \
         nm "_L9:\n" \
         "mv a3, x0\n" \
         "neg a1, a1\n" \
         "j " nm "_L4\n" \
   }

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
   },
   gen_mului("__mului32"),
   gen_mului("__mului64"),
   gen_mulsi("__mulsi32"),
   gen_mulsi("__mulsi64"),
};
const size_t num_builtin_funcs = arraylen(builtin_funcs);
