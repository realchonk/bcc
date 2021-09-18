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
   },
};
const size_t num_builtin_funcs = arraylen(builtin_funcs);
