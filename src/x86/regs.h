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


#if BITS == 32

typedef uint32_t uintreg_t;
static const char* regs[] = { "eax", "ecx", "edx" };

#define REG_SP "esp"
#define REG_BP "ebp"
#define REG_AX "eax"

#else

typedef uint64_t uintreg_t;
static const char* regs[] = { "rax", "rdi", "rsi", "rdx", "rcx",  "r8",  "r9",  "r10",  "r11" };

#define REG_SP "rsp"
#define REG_BP "rbp"
#define REG_AX "rax"

#endif

#define reg(i) (((i) < arraylen(regs)) ? regs[(i)] : (panic("register out of bounds, r=%zu", (size_t)(i)), NULL))


#endif /* FILE_X86_REGS_H */
