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

#include <stdarg.h>
#include <string.h>
#include "emit_ir.h"
#include "target.h"
#include "regs.h"
#include "ir.h"

static size_t stack_size;

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

ir_node_t* emit_ir(const ir_node_t* n) {
   const char* instr;
   switch (n->type) {
   case IR_ASM:
      emit("%s", n->str);
      return n->next;
   case IR_PROLOGUE:
   {
      const size_t nrp = my_min(4, buf_len(n->func->params));
      emit("push {fp, lr}");
      emit("mov fp, sp");

      stack_size  = sizeof_scope(n->func->scope);
      stack_size += nrp * REGSIZE;
      stack_size  = align_stack_size(stack_size);
      emit("sub sp, sp, #%zu", stack_size);

      int fp = -(stack_size - (nrp * REGSIZE));
      for (size_t i = 0; i < nrp; ++i) {
         fp -= REGSIZE;
         emit("str r%u, [fp, #%d]", i, fp);
         n->func->params[i].addr = fp;
      }
   
      assign_scope(n->func->scope, &fp);

      for (size_t i = nrp; i < buf_len(n->func->params); ++i) {
         n->func->params[i].addr = 8 + (i * REGSIZE);
      }
      return n->next;
   }

   case IR_EPILOGUE:
   {
      if (!strcmp(n->func->name, "main"))
         emit("mov r0, #0");
      emit("%s.ret:", n->func->name);
      emit("mov sp, fp");
      emit("pop {fp, pc}");
      return n->next;
   }

   case IR_BEGIN_SCOPE:
   case IR_END_SCOPE:
      emit("");
      return n->next;


   case IR_LOAD:
      emit_iload(n->load.dest, n->load.value);
      return n->next;
   case IR_MOVE:
      emit("mov %s, %s", reg(n->move.dest), reg(n->move.src));
      return n->next;
   case IR_FPARAM:
      emit_lda(n->fparam.reg, n->func->params[n->fparam.idx].addr);
      return n->next;
   case IR_LOOKUP:
      emit_lda(n->lookup.reg, n->lookup.scope->vars[n->lookup.var_idx].addr);
      return n->next;
   
   case IR_IRET:
      if (n->unary.reg != 0) {
         emit("mov r0, %s", reg(n->unary.reg));
      }
      fallthrough;
   case IR_RET:
      emit("b %s.ret", n->func->name);
      return n->next;

   case IR_IADD:
   {
      const char* dest = reg(n->binary.dest);
      struct ir_value av = n->binary.a;
      struct ir_value bv = n->binary.b;

      const char* a;
      if (av.type == IRT_REG) {
         a = reg(av.reg);
      } else {
         a = reg(n->binary.dest);
         emit_iload(n->binary.dest, av.sVal);
      }
      if (bv.type == IRT_REG) {
         emit("add %s, %s, %s", dest, a, reg(bv.reg));
      } else {
         emit("add %s, %s, #%jd", dest, a, bv.sVal);
      }
      return n->next;
   }

   case IR_READ:
   {
      emit_rw(n->rw.dest, false, n->rw.size, n->rw.sign_extend, "[%s]", reg(n->rw.src));
      return n->next;
   }
   case IR_WRITE:
   {
      emit_rw(n->rw.src, true, n->rw.size, false, "[%s]", reg(n->rw.dest));
      return n->next;
   }

   default:
      panic("unsupported ir_node type '%s'", ir_node_type_str[n->type]);
   }
}
