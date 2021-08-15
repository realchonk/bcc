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

#include "error.h"

#define reg8(i) ((i) < arraylen(regs8) ? regs8[i] : (panic("register out of range"), NULL))
#define reg16(i) ((i) < arraylen(regs16) ? regs16[i] : (panic("register out of range"), NULL))
#define reg32(i) ((i) < arraylen(regs32) ? regs32[i] : (panic("register out of range"), NULL))

#if BITS == 32

typedef uint32_t uintreg_t;
static const char* regs8[] =  {  "al",  "ch",  "dl" };
static const char* regs16[] = {  "ax",  "cx",  "dx" };
static const char* regs32[] = { "eax", "ecx", "edx" };

#define REGSIZE 4
#define reg_sp "esp"
#define reg_bp "ebp"
#define reg_ax "eax"
#define reg_bx "ebx"
#define reg_dx "edx"
#define reg_dxi 2

#define mreg(i) reg32(i)

#define reg_op(dest, src, size) \
   switch (size) { \
   case IRS_BYTE: \
   case IRS_CHAR:    dest = reg8(src); break; \
   case IRS_SHORT:   dest = reg16(src); break; \
   case IRS_PTR: \
   case IRS_LONG: \
   case IRS_INT:     dest = reg32(src); break; \
   default:          panic("unsupported operand size '%s'", ir_size_str[size]); \
   }

#else

typedef uint64_t uintreg_t;
static const char* regs8[] =  {  "al", "dil", "sil",  "dl",  "cl", "r8b", "r9b", "r10b", "r11b" };
static const char* regs16[] = {  "ax",  "di",  "si",  "dx",  "cx", "r8w", "r9w", "r10w", "r11w" };
static const char* regs32[] = { "eax", "edi", "esi", "edx", "ecx", "r8d"  "r9d", "r10d", "r11d" };
static const char* regs64[] = { "rax", "rdi", "rsi", "rdx", "rcx",  "r8",  "r9",  "r10",  "r11" };
static size_t param_regs[] = { 1, 2, 3, 4, 5, 6 };

#define REGSIZE 8
#define reg_sp "rsp"
#define reg_bp "rbp"
#define reg_ax "rax"
#define reg_bx "rbx"
#define reg_dx "rdx"
#define reg_dxi 3

#define reg64(i) ((i) < arraylen(regs64) ? regs64[i] : (panic("register out of range"), NULL))
#define mreg(i) reg64(i)

#define reg_op(dest, src, size) \
   switch (size) { \
   case IRS_BYTE: \
   case IRS_CHAR:    dest = reg8(src); break; \
   case IRS_SHORT:   dest = reg16(src); break; \
   case IRS_INT:     dest = reg32(src); break; \
   case IRS_PTR: \
   case IRS_LONG:    dest = reg64(src); break; \
   default:          panic("unsupported operand size '%s'", ir_size_str[size]); \
   }

#endif




