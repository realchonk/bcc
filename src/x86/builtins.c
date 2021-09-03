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

#include "config.h"
#include "target.h"
#include "error.h"

#if BITS == 32
#define gen_div(bits, sign, a, d, sz, instr) \
{ \
   .name = "__div" #sign "i" #bits, \
   .code = \
      "push edx\n" \
      "mov " #a ", " #sz " [esp + 8]\n" \
      "xor " #d ", " #d "\n" \
      #instr " " #sz "[esp + 12]\n" \
      "pop edx\n" \
      "ret\n" \
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
      "ret\n" \
}
#define gen_mul(bits, sign, a, d, sz, instr) \
{ \
   .name = "__mul" #sign "i" #bits, \
   .code = \
      "push edx\n" \
      "mov " #a ", " #sz " [esp + 8]\n" \
      #instr " " #sz "[esp + 12]\n" \
      "pop edx\n" \
      "ret\n" \
}

#else // BITS

#define gen_div(bits, sign, a, d, sz, instr) \
{ \
   .name = "__div" #sign "i" #bits, \
   .code = \
      "push rdx\n" \
      "mov " #a ", " #sz " [rsp + 16]\n" \
      "xor " #d ", " #d "\n" \
      #instr " " #sz "[rsp + 24]\n" \
      "pop rdx\n" \
      "ret\n" \
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
      "ret\n" \
}
#define gen_mul(bits, sign, a, d, sz, instr) \
{ \
   .name = "__mul" #sign "i" #bits, \
   .code = \
      "push rdx\n" \
      "mov " #a ", " #sz " [rsp + 16]\n" \
      #instr " " #sz "[rsp + 24]\n" \
      "pop rdx\n" \
      "ret\n" \
}


#endif

struct builtin_func builtin_funcs[] = {
   gen_div(8,  s,  al,  dl, byte,  idiv),
   gen_div(16, s,  ax,  dx, word,  idiv),
   gen_div(32, s, eax, edx, dword, idiv),
   gen_div(8,  u,  al,  dl, byte,   div),
   gen_div(16, u,  ax,  dx, word,   div),
   gen_div(32, u, eax, edx, dword,  div),
#if BITS == 64
   gen_div(64, s, rax, rdx, qword, idiv),
   gen_div(64, u, rax, rdx, qword,  div),
#endif
   
   gen_mod(8,  s,  al,  dl, byte,  idiv),
   gen_mod(16, s,  ax,  dx, word,  idiv),
   gen_mod(32, s, eax, edx, dword, idiv),
   gen_mod(8,  u,  al,  dl, byte,   div),
   gen_mod(16, u,  ax,  dx, word,   div),
   gen_mod(32, u, eax, edx, dword,  div),
#if BITS == 64
   gen_mod(64, s, rax, rdx, qword, idiv),
   gen_mod(64, u, rax, rdx, qword,  div),
#endif

   gen_mul(8,  s,  al,  dl, byte,  imul),
   gen_mul(16, s,  ax,  dx, word,  imul),
   gen_mul(32, s, eax, edx, dword, imul),
   gen_mul(8,  u,  al,  dl, byte,   mul),
   gen_mul(16, u,  ax,  dx, word,   mul),
   gen_mul(32, u, eax, edx, dword,  mul),
#if BITS == 64
   gen_mul(64, s, rax, rdx, qword, imul),
   gen_mul(64, u, rax, rdx, qword,  mul),
#endif

#if BITS == 32
   {
      .name = "__check_sp",
      .code =
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
   {
      .name = "__builtin_return_address",
      .code =
         "mov eax, dword [ebp + 4]\n"
         "ret\n"
   },
#else
   {
      .name = "__check_sp",
      .code =
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
   {
      .name = "__builtin_return_address",
      .code =
         "mov rax, qword [rbp + 8]\n"
         "ret\n"
   },
#endif

};

const size_t num_builtin_funcs = arraylen(builtin_funcs);



