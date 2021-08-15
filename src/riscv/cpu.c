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

#include <stdio.h>
#include <ctype.h>
#include "riscv/cpu.h"
#include "target.h"

struct riscv_cpu riscv_cpu;

bool parse_cpu(const char* s, struct riscv_cpu* cpu) {
   if (!s || !cpu) return false;
   size_t i = 0;
   if (tolower(s[0]) != 'r' || tolower(s[1]) != 'v' || s[2] != (BITS / 10 + '0') || s[3] != (BITS % 10 + '0')) {
      fprintf(stderr, "bcc: invalid CPU '%s', expected rv%d.\n", s, BITS);
      return false;
   }
   i += 4;
   if (s[i] == 'i') ++s;
   else if (s[i] == 'e') ++s, cpu->is_embedded = true;
   else if (s[i] != 'g') {
      fprintf(stderr, "bcc: invalid CPU '%s', expected i, g or e after rv%d.\n", s, BITS);
      return false;
   }

   // parse extensions
   for (; s[i]; ++i) {
      const char ch = toupper(s[i]);
      switch (ch) {
      case 'M':
         cpu->has_mult = true;
         break;
      case 'A':
         cpu->has_atomic = true;
         break;
      case 'F':
         cpu->has_float = true;
         break;
      case 'D':
         cpu->has_double = true;
         break;
      case 'C':
         cpu->has_compressed = true;
         break;
      case 'Q':
         cpu->has_quad = true;
         break;
      case 'G':
         cpu->has_mult = true;
         cpu->has_atomic = true;
         cpu->has_float = true;
         cpu->has_double = true;
         break;
      case 'Z':
      case 'X':
         fprintf(stderr, "bcc: %c extensions are not supported for RISC-V backend.\n", ch);
         return false;
      default:
         fprintf(stderr, "bcc: invalid character '%c' for -mcpu.\n", ch);
      }
   }
   return true;
}

bool emit_prepare(void) {
   const struct machine_option* opt = get_mach_opt("cpu");
   assert(opt != NULL);
   return parse_cpu(opt->sVal, &riscv_cpu);
}
