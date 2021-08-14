#include "target.h"
#include "error.h"
#include "regs.h"

// 16 - REGSIZE
#if BITS == 32
#define SP_REGSIZE "12"
#else
#define SP_REGSIZE "8"
#endif


// Generated assembly for mul/div/mod was derived from output by the GCC compiler

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

#define gen_divui(nm) \
   { \
      .name = nm, \
      .code = \
         "mv a5, a0\n" \
         "mv a0, x0\n" \
         "bltu a5, a1, " nm "_L4\n" \
         nm "_L3:\n" \
         "sub a5, a5, a1\n" \
         "addi a0, a0, 1\n" \
         "bleu a1, a5, " nm "_L3\n" \
         "ret\n" \
         nm "_L4:\n" \
         "ret\n" \
   }
#define gen_divsi(nm) \
   { \
      .name = nm, \
      .code = \
         "mv a5, a0\n" \
         "blt a0, x0, " nm "_L20\n" \
         "blt a1, x0, " nm "_L6\n" \
         "blt a0, a1, " nm "_L18\n" \
         "mv a3, x0\n" \
         nm "_L4:\n" \
         "mv a0, x0\n" \
         nm "_L8:\n" \
         "sub a5, a5, a1\n" \
         "mv a4, a0\n" \
         "addi a0, a0, 1\n" \
         "bge a5, a1, " nm "_L8\n" \
         "beq a3, x0, " nm "_L1\n" \
         "not a0, a4\n" \
         "ret\n" \
         nm "_L6:\n" \
         "neg a1, a1\n" \
         "li a3, 1\n" \
         "bge a0, a1, " nm "_L4\n" \
         nm "_L18:\n" \
         "mv a0, x0\n" \
         nm "_L1:\n" \
         "ret\n" \
         nm "_L20:\n" \
         "neg a4, a0\n" \
         "blt a1, x0, " nm "_L3\n" \
         "mv a5, a4\n" \
         "li a3, 1\n" \
         "bge a4, a1, " nm "_L4\n" \
         "mv a0, x0\n" \
         "j " nm "_L1\n" \
         nm "_L3:\n" \
         "neg a3, a1\n" \
         "bgt a0, a1, " nm "_L18\n" \
         "mv a1, a3\n" \
         "mv a5, a4\n" \
         "mv a3, x0\n" \
         "j " nm "_L4\n" \
   }
#define gen_modui(nm) \
   { \
      .name = nm, \
      .code = \
         "bgtu a1, a0, " nm "_L7\n" \
         nm "_L3:\n" \
         "sub a0, a0, a1\n" \
         "bleu a1, a0, " nm "_L3\n" \
         nm "_L7:\n" \
         "ret\n" \
   }
#define gen_modsi(nm) \
   { \
      .name = nm, \
      .code = \
         "mv a5, a0\n" \
         "blt a0, x0, " nm "_L18\n" \
         "blt a1, x0, " nm "_L6\n" \
         "blt a0, a1, " nm "_L10\n" \
         nm "_L16:\n" \
         "mv a3, x0\n" \
         nm "_L8:\n" \
         "mv a4, a5\n" \
         "sub a5, a5, a1\n" \
         "bge a5, a1, " nm "_L8\n" \
         "mv a0, a5\n" \
         "beq a3, x0, " nm "_L1\n" \
         "sub a0, a1, a4\n" \
         "ret\n" \
         nm "_L6:\n" \
         "neg a1, a1\n" \
         "li a3, 1\n" \
         "bge a0, a1, " nm "_L8\n" \
         "neg a0, a0\n" \
         nm "_L1:\n" \
         "ret\n" \
         nm "_L18:\n" \
         "neg a0, a0\n" \
         "blt a1, x0, " nm "_L3\n" \
         "blt a0, a1, " nm "_L10\n" \
         "mv a5, a0\n" \
         "li a3, 1\n" \
         "j " nm "_L8\n" \
         nm "_L10:\n" \
         "mv a0, a5\n" \
         "ret\n" \
         nm "_L3:\n" \
         "neg a4, a1\n" \
         "bgt a5, a1, " nm "_L19\n" \
         "mv a1, a4\n" \
         "mv a5, a0\n" \
         "j " nm "_L16\n" \
         nm "_L19:\n" \
         "ret\n" \
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
   gen_divui("__divui32"),
   gen_divui("__divui64"),
   gen_divsi("__divsi32"),
   gen_divsi("__divsi64"),
   gen_modui("__modui32"),
   gen_modui("__modui64"),
   gen_modsi("__modsi32"),
   gen_modsi("__modsi64"),
};
const size_t num_builtin_funcs = arraylen(builtin_funcs);
