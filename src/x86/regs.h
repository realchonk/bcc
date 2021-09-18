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

#ifndef FILE_X86_REGS_H
#define FILE_X86_REGS_H

#include <stdint.h>
#include "config.h"
#include "error.h"
#include "ir.h"

#define REGSIZE (BITS / 8)

#if BITS == 32

typedef uint32_t uintreg_t;
static const char* regs[] = { "eax", "ecx", "edx" };

static const char* regs8[]  = { "al", "cl", "dl" };
static const char* regs16[] = { "ax", "cx", "dx" };
#define regs32 regs
static const char* gas_sizes[] = { "BYTE", "BYTE", "WORD", "DWORD", "DWORD", "DWORD" };

#define REG_SP "esp"
#define REG_BP "ebp"
#define REG_AX "eax"
#define REG_BX "ebx"

#else

typedef uint64_t uintreg_t;
static const char* regs[] = { "rax", "rdi", "rsi", "rdx", "rcx",  "r8",  "r9",  "r10",  "r11" };

static const char* regs8[]  = {  "al", "dil", "sil",  "dl",  "cl", "r8b", "r9b", "r10b", "r11b" };
static const char* regs16[] = {  "ax",  "di",  "si",  "dx",  "cx", "r8w", "r9w", "r10w", "r11w" };
static const char* regs32[] = { "eax", "edi", "esi", "edx", "ecx", "r8d", "r9d", "r10d", "r11d" };
#define regs64 regs
static const char* gas_sizes[] = { "BYTE", "BYTE", "WORD", "DWORD", "QWORD", "QWORD" };
static size_t param_regs[] = { 1, 2, 3, 4, 5, 6 };

#define REG_SP "rsp"
#define REG_BP "rbp"
#define REG_AX "rax"
#define REG_BX "rbx"

#endif

#define reg(i) (const char*)(((i) < arraylen(regs)) ? regs[(i)] : \
      (panic("register out of bounds, r=%u", (unsigned)(i)), NULL))
#define as_size(irs) (const char*)(((irs) < arraylen(gas_sizes)) ? gas_sizes[(irs)] : \
      (panic("invalid IR value size, irs='%s'", ir_size_str[irs]), NULL))

static const char* reg_wsz(ir_reg_t r, enum ir_value_size sz) {
   if (r >= arraylen(regs))
      panic("register out of bounds, r=%u", (unsigned)r);
   switch (sz) {
   case IRS_BYTE:
   case IRS_CHAR:
      return regs8[r];
   case IRS_SHORT:
      return regs16[r];
   case IRS_LONG:
   case IRS_PTR:
#if BITS == 64
      return regs64[r];
#endif
   case IRS_INT:
      return regs32[r];
   default:
      panic("invalid IR value size '%s'", ir_size_str[sz]);
   }
}

#endif /* FILE_X86_REGS_H */

