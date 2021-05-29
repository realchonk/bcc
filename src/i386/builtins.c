#include "target.h"
#include "error.h"

#define gen_div(bits, sign, a, d, sz, instr) \
{ \
   .name = "__div" #sign "i" #bits, \
   .code = \
      "push edx\n" \
      "mov " #a ", " #sz " [esp + 8]\n" \
      "xor " #d ", " #d "\n" \
      #instr " " #sz "[esp + 12]\n" \
      "pop edx\n" \
      "ret" \
}
#define gen_mod(bits, sign, a, d, sz, instr) \
{ \
   .name = "__mod" #sign "i" #bits, \
   .code = \
      "push edx\n" \
      "mov " #a ", " #sz " [esp + 8]\n" \
      "xor " #d ", " #d "\n" \
      #instr " " #sz "[esp + 12]\n" \
      "mov eax, edx\n" \
      "pop edx\n" \
      "ret" \
}

struct builtin_func builtin_funcs[] = {
   gen_div(8,  s,  al,  dl, byte, idiv),
   gen_div(16, s,  ax,  dx, word, idiv),
   gen_div(32, s, eax, edx, dword, idiv),
   gen_div(8,  u,  al,  dl, byte, div),
   gen_div(16, u,  ax,  dx, word, div),
   gen_div(32, u, eax, edx, dword, div),
   
   gen_mod(8,  s,  al,  dl, byte, idiv),
   gen_mod(16, s,  ax,  dx, word, idiv),
   gen_mod(32, s, eax, edx, dword, idiv),
   gen_mod(8,  u,  al,  dl, byte, div),
   gen_mod(16, u,  ax,  dx, word, div),
   gen_mod(32, u, eax, edx, dword, div),
   {
      .name = "__check_sp",
      .code =
         "check_sp:\n"
         "test sp, 7\n"
         "jnz .major\n"
         "test sp, 8\n"
         "jz .minor\n"
         "ret\n"
         ".major:\n"
         "and esp, ~15\n"
         "add esp, 8\n"
         "push .major_err\n"
         "call puts\n"
         "add esp, 4\n"
         "call abort\n"
         ".minor:\n"
         "and esp, ~15\n"
         "add esp, 8\n"
         "push .minor_err\n"
         "call puts\n"
         "add esp, 4\n"
         "call abort\n"
         "section .rodata\n"
         ".major_err: db \"Stack is not 4-byte aligned.\", 10, 0\n"
         ".minor_err: db \"Stack is not 16-byte aligned.\", 10, 0\n"
         "section .text\n"
   },
};

const size_t num_builtin_funcs = arraylen(builtin_funcs);



