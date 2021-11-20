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
#include "bcc.h"
#include "ir.h"

static size_t stack_size;


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
      instr = "add";
      goto ir_binary;
   case IR_ISUB:
      instr = "sub";
      goto ir_binary;
   case IR_IAND:
      instr = "and";
      goto ir_binary;
   case IR_IOR:
      instr = "orr";
      goto ir_binary;
   case IR_IXOR:
      instr = "eor";
      goto ir_binary;
   case IR_ILSL:
      instr = "lsl";
      goto ir_binary;
   case IR_ILSR:
      instr = "lsr";
      goto ir_binary;
   case IR_IASR:
      instr = "asr";
      goto ir_binary;
   case IR_IMUL:
   case IR_UMUL:
      instr = "mul";
   {
   ir_binary:;
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
         emit("%s %s, %s, %s", instr, dest, a, reg(bv.reg));
      } else {
         emit("%s %s, %s, #%jd", instr, dest, a, bv.sVal);
      }
      return n->next;
   }

   case IR_INOT:
      emit("mnv %s, %s", reg(n->unary.reg), reg(n->unary.reg));
      return n->next;
   case IR_INEG:
      emit("rsb %s, %s, #0", reg(n->unary.reg), reg(n->unary.reg));
      return n->next;
   case IR_BNOT:
   {
      const char* r = reg(n->unary.reg);
      emit("rsbs %s, %s, #1", r, r);
      emit("movcc %s, #0", r);
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

   case IR_LABEL:
      emit("%s:", n->str);
      return n->next;
   case IR_JMP:
      emit("b %s", n->str);
      return n->next;

   case IR_JMPIF:
      instr = "bne";
      goto ir_cjmp;
   case IR_JMPIFN:
      instr = "beq";
   {
   ir_cjmp:;
      emit("cmp %s, #0", reg(n->cjmp.reg));
      emit("%s %s", instr, n->cjmp.label);
      return n->next;
   }

   case IR_ISTEQ:
   case IR_ISTNE:
   case IR_ISTGR:
   case IR_ISTGE:
   case IR_ISTLT:
   case IR_ISTLE:
   case IR_USTGR:
   case IR_USTGE:
   case IR_USTLT:
   case IR_USTLE:
   {
      struct entry {
         const char* cc;
         enum ir_node_type negation;
      };
      const struct entry es[] = {
         [IR_ISTEQ] = { "eq", IR_ISTNE },
         [IR_ISTNE] = { "ne", IR_ISTEQ },
         [IR_ISTGR] = { "gt", IR_ISTLE },
         [IR_ISTGE] = { "ge", IR_ISTLT },
         [IR_ISTLT] = { "lt", IR_ISTGE },
         [IR_ISTLE] = { "le", IR_ISTGR },
         [IR_USTGR] = { "hi", IR_USTLE },
         [IR_USTGE] = { "hs", IR_USTLT },
         [IR_USTLT] = { "lo", IR_USTGE },
         [IR_USTLE] = { "ls", IR_USTGR },
      };
      const char* dest = reg(n->binary.dest);
      const char* a = irv2str(&n->binary.a);
      const char* b = irv2str(&n->binary.b);
      ir_node_t* next = n->next;

      emit("cmp %s, %s", a, b);
      if (optim_level >= 1 && next
         && (next->type == IR_JMPIF || next->type == IR_JMPIFN)) {
         const char* cc;
         if (next->type == IR_JMPIF) {
            cc = es[n->type].cc;
         } else {
            cc = es[es[n->type].negation].cc;
         }
         emit("b%s %s", cc, next->str);
         return next->next;
      } else {
         emit("mov%s %s, #0", es[es[n->type].negation].cc, dest);
         emit("mov%s %s, #1", es[n->type].cc, dest);
         return next;
      }
   }


   default:
      panic("unsupported ir_node type '%s'", ir_node_type_str[n->type]);
   }
}
