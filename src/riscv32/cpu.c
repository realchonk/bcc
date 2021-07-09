#include <stdio.h>
#include <ctype.h>
#include "riscv32/cpu.h"
#include "target.h"

struct riscv32_cpu riscv32_cpu;



bool parse_cpu(const char* s, struct riscv32_cpu* cpu) {
   if (!s || !cpu) return false;
   size_t i = 0;
   if (tolower(s[0]) != 'r' || tolower(s[1]) != 'v' || s[2] != '3' || s[3] != '2') {
      fprintf(stderr, "bcc: invalid CPU '%s', expected rv32.\n", s);
      return false;
   }
   i += 4;
   if (s[i] == 'i') ++s;
   else if (s[i] == 'e') ++s, cpu->is_embedded = true;
   else if (s[i] != 'g') {
      fprintf(stderr, "bcc: invalid CPU '%s', expected i, g or e after rv32.\n", s);
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
   return parse_cpu(opt->sVal, &riscv32_cpu);
}
