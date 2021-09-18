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

#ifndef FILE_RISCV_CPU_H
#define FILE_RISCV_CPU_H
#include <stdbool.h>
#include "config.h"

struct riscv_cpu {
   bool is_embedded;
   bool has_mult;
   bool has_atomic;
   bool has_compressed;
   bool has_float;
   bool has_double;
   bool has_quad;
};

extern struct riscv_cpu riscv_cpu;

bool parse_cpu(const char*, struct riscv_cpu*);

#if BITS == 32

#ifndef DEF_CPU
#define DEF_CPU "rv32gc"
#endif

#ifndef DEF_ABI
#define DEF_ABI   "ilp32d"
#endif

#else // BITS

#ifndef DEF_CPU
#define DEF_CPU  "rv64gc"
#endif

#ifndef DEF_ABI
#define DEF_ABI   "lp64d"
#endif

#endif

#endif /* FILE_RISCV_CPU_H */
