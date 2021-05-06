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

struct builtin_func builtin_funcs[] = {
   gen_div(8,  s,  al,  dl, byte, idiv),
   gen_div(16, s,  ax,  dx, word, idiv),
   gen_div(32, s, eax, edx, dword, idiv),
   gen_div(64, s, rax, rdx, qword, idiv),
   gen_div(8,  u,  al,  dl, byte, div),
   gen_div(16, u,  ax,  dx, word, div),
   gen_div(32, u, eax, edx, dword, div),
   gen_div(64, u, rax, rdx, qword, div),
};

const size_t num_builtin_funcs = arraylen(builtin_funcs);



