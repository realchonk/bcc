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

#ifndef FILE_EMIT_IR_H
#define FILE_EMIT_IR_H
#include <math.h>
#include "target.h"
#include "regs.h"

static size_t sizeof_scope(const struct scope* scope) {
   size_t num = 0;
   for (size_t i = 0; i < buf_len(scope->vars); ++i) {
      num += sizeof_value(scope->vars[i].type, false);
   }
   size_t max_child = 0;
   for (size_t i = 0; i < buf_len(scope->children); ++i) {
      const size_t sz = sizeof_scope(scope->children[i]);
      if (sz > max_child) max_child = sz;
   }
   return num + max_child;
}

static void assign_scope(struct scope* scope, int* sp) {
   for (size_t i = 0; i < buf_len(scope->vars); ++i) {
      *sp -= sizeof_value(scope->vars[i].type, false);
      scope->vars[i].addr = *sp;
   }
   for (size_t i = 0; i < buf_len(scope->children); ++i) {
      assign_scope(scope->children[i], sp);
   }
}
static void emit_iload(ir_reg_t r, intmax_t val) {
   if (val >= 0) {
      emit("mov %s, #%jd", reg(r), val);
   } else {
      emit("mvn %s, #%jd", reg(r), -val - 1);
   }
}
static void emit_lda(ir_reg_t r, int fpoff) {
   if (fpoff >= 0) {
      emit("add %s, fp, #%d", reg(r), fpoff);
   } else {
      emit("sub %s, fp, #%d", reg(r), -fpoff);
   }
}
static void emit_rw(ir_reg_t dest, bool rw, enum ir_value_size irs, bool se, const char* addr, ...) {
   va_list ap;
   va_start(ap, addr);

   const char* instr;

   if (rw) {
      switch (irs) {
      case IRS_BYTE:
      case IRS_CHAR:    instr = "strb"; break;
      case IRS_SHORT:   instr = "strh"; break;
      case IRS_INT:     
      case IRS_LONG:    
      case IRS_PTR:     instr = "str";  break;
      default:          panic("unsupported IR value size '%s'", ir_size_str[irs]);
      }
   } else {
      switch (irs) {
      case IRS_BYTE:
      case IRS_CHAR:    instr = se ? "ldrsb" : "ldrb"; break;
      case IRS_SHORT:   instr = se ? "ldrsh" : "ldrh"; break;
      case IRS_INT:     
      case IRS_LONG:    
      case IRS_PTR:     instr = "ldr";  break;
      default:          panic("unsupported IR value size '%s'", ir_size_str[irs]);
      }
   }

   emitraw("%s %s, ", instr, reg(dest));
   vemitraw(addr, ap);
   emit("");

   va_end(ap);
}
// TODO: MERGE to bcc.h
static size_t snslen(intmax_t v) {
   size_t n = 0;
   if (!v) return 1;
   if (v < 0) {
      v = -v;
      ++n;
   }
   return (size_t)log10(v) + 1 + n;
}
static const char* irv2str(const struct ir_value* v) {
   switch (v->type) {
   case IRT_REG:
      return reg(v->reg);
   case IRT_UINT:
   {
      char buffer[snslen(v->sVal) + 2];
      snprintf(buffer, sizeof(buffer), "#%jd", v->sVal);
      return strint(buffer);
   }
   default:
      panic("invalid IR value type");
   }
}

#endif /* FILE_EMIT_IR_H */
