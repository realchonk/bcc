#include "target.h"
#include "error.h"

#define gen_div(bits, sign, a, d, sz, instr) \
{ \
   .name = "__div" #sign "i" #bits, \
   .code = \
      "push rdx\n" \
      "mov " #a ", " #sz " [rsp + 16]\n" \
      "xor " #d ", " #d "\n" \
      #instr " " #sz "[rsp + 24]\n" \
      "pop rdx\n" \
      "ret" \
}
#define gen_mod(bits, sign, a, d, sz, instr) \
{ \
   .name = "__mod" #sign "i" #bits, \
   .code = \
      "push rdx\n" \
      "mov " #a ", " #sz " [rsp + 16]\n" \
      "xor " #d ", " #d "\n" \
      #instr " " #sz "[rsp + 24]\n" \
      "mov rax, rdx\n" \
      "pop rdx\n" \
      "ret" \
}

struct builtin_func builtin_funcs[] = {
   gen_div(8,  s,  al,  dl, byte, idiv),
   gen_div(16, s,  ax,  dx, word, idiv),
   gen_div(32, s, eax, edx, dword, idiv),
   gen_div(64, s, rax, rdx, qword, idiv),
   gen_div(8,  u,  al,  dl, byte, div),
   gen_div(16, u,  ax,  dx, word, div),
   gen_div(32, u, eax, edx, dword, div),
   gen_div(64, u, rax, rdx, qword, div),

   gen_mod(8,  s,  al,  dl, byte, idiv),
   gen_mod(16, s,  ax,  dx, word, idiv),
   gen_mod(32, s, eax, edx, dword, idiv),
   gen_mod(64, s, rax, rdx, qword, idiv),
   gen_mod(8,  u,  al,  dl, byte, div),
   gen_mod(16, u,  ax,  dx, word, div),
   gen_mod(32, u, eax, edx, dword, div),
   gen_mod(64, u, rax, rdx, qword, div),
   {
      .name = "__check_sp",
      .code =
         "check_sp:\n"
         "test spl, 7\n"
         "jnz .major\n"
         "test spl, 15\n"
         "jnz .minor\n"
         "ret\n"
         ".major:\n"
         "lea rdi, [rel .major_err]\n"
         "call [rel puts wrt ..got]\n"
         "jmp [rel abort wrt ..got]\n"
         ".minor:\n"
         "lea rdi, [rel .minor_err]\n"
         "call [rel puts wrt ..got]\n"
         "jmp [rel abort wrt ..got]\n"
         "section .rodata\n"
         ".major_err: db \"Stack is not 4-byte aligned.\", 10, 0\n"
         ".minor_err: db \"Stack is not 16-byte aligned.\", 10, 0\n"
         "section .text\n"

   },
};

const size_t num_builtin_funcs = arraylen(builtin_funcs);



