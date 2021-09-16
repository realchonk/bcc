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

#include "emit_ir.h"

static const struct function* cur_func;

const ir_node_t* emit_ir(const ir_node_t* n) {
   const char* instr;
   switch (n->type) {
   case IR_NOP:
      emit("nop");
      return n->next;
   case IR_MOVE:
      emit("mov %s, %s", reg(n->move.dest), reg(n->move.src));
      return n->next;
   case IR_LOAD:
      if (n->load.value) {
         emit("mov %s, %jd", reg(n->load.dest), (intmax_t)n->load.value);
      } else {
         emit_clear(reg(n->load.dest));
      }
      return n->next;
   case IR_IADD:
   case IR_ISUB:
   {
      const char* a = irv2str(&n->binary.a);
      const char* b = irv2str(&n->binary.b);
      const char* dest = reg(n->binary.dest);

      if (n->binary.a.type == IRT_REG && n->binary.dest == n->binary.a.reg) {
         if (n->binary.b.type == IRT_UINT && n->binary.b.uVal == 1) {
            emit("%s %s", n->type == IR_IADD ? "inc" : "dec", dest);
         } else {
            emit("%s %s, %s", n->type == IR_IADD ? "add" : "sub", dest, b);
         }
      } else {
         emit("lea %s, [%s %c %s]", dest, a, n->type == IR_IADD ? '+' : '-', b);
      }
      return n->next;
   }
   case IR_IAND:
      instr = "and";
      goto ir_bitwise;
   case IR_IOR:
      instr = "or";
      goto ir_bitwise;
   case IR_IXOR:
      instr = "xor";
      goto ir_bitwise;
   case IR_ILSL:
      instr = "shl";
      goto ir_bitwise;
   case IR_ILSR:
      instr = "shr";
      goto ir_bitwise;
   case IR_IASR:
      instr = "sar";
   {
   ir_bitwise:;
      const char* a = irv2str(&n->binary.a);
      const char* b = irv2str(&n->binary.b);
      const char* dest = reg(n->binary.dest);
      
      if (n->binary.a.type != IRT_REG || n->binary.dest != n->binary.a.reg) {
         emit("mov %s, %s", dest, a);
      }
      emit("%s %s, %s", instr, dest, b);
      return n->next;
   }
   case IR_INOT:
   case IR_INEG:
      emit("%s %s", n->type == IR_INOT ? "not" : "neg", reg(n->unary.reg));
      return n->next;
   case IR_BNOT:
   {
      const char* reg = reg_wsz(n->unary.reg, n->unary.size);
      emit("test %s, %s", reg, reg);
      emit("sete %s", regs8[n->unary.reg]);
      return n->next;
   }

   case IR_BEGIN_SCOPE:
   case IR_END_SCOPE:
      emit("");
      return n->next;
   
   case IR_READ:
#if BTIS == 32
      if (n->read.size < INT_INT) {
#elif BITS == 64
      if (n->read.size < INT_LONG) {
#endif
         emit("mov%s %s, %s PTR [%s]", n->read.sign_extend ? "sx" : "zx", reg(n->read.dest),
               as_size(n->move.size), reg(n->read.src));
      } else {
         emit("mov %s, %s PTR [%s]", reg(n->read.dest), as_size(n->read.size), reg(n->read.src));
      }
      return n->next;
   case IR_WRITE:
      emit("mov %s PTR [%s], %s", as_size(n->move.size), reg(n->move.dest), reg_wsz(n->move.src, n->move.size));
      return n->next;

   case IR_PROLOGUE:
      emit("");
      if (func_is_global(n->func))
         emit(".global %s", n->func->name);
      if (!get_mach_opt("clean-asm")->bVal)
         emit(".type %s, @function", n->func->name);
      emit("%s:", n->func->name);
      emit("push %s", REG_BP);
      emit("mov %s, %s", REG_BP, REG_SP);
      cur_func = n->func;
      return n->next;
   case IR_EPILOGUE:
      emit_clear(REG_AX);
      emit("%s.ret:", n->func->name);
      emit("leave");
      emit("ret");
      if (!get_mach_opt("clean-asm")->bVal)
         emit(".size %s, .-%s", n->func->name, n->func->name);
      emit("");
      cur_func = NULL;
      return n->next;
   case IR_IRET:
      if (n->unary.reg) {
         emit("mov %s, %s", REG_AX, reg(n->unary.reg));
      }
      fallthrough;
   case IR_RET:
      emit("jmp %s.ret", cur_func->name);
      return n->next;

   case IR_LABEL:
      emit("%s.%s:", cur_func->name, n->str);
      return n->next;
   case IR_JMP:
      emit("jmp %s.%s", cur_func->name, n->str);
      return n->next;
   case IR_JMPIF:
      instr = "jnz";
      goto ir_jmpifn;
   case IR_JMPIFN:
      instr = "jz";
   ir_jmpifn:
      emit("test %s, %s", reg(n->cjmp.reg), reg(n->cjmp.reg));
      emit("%s %s.%s", instr, cur_func->name, n->cjmp.label);
      return n->next;

   case IR_GLOOKUP:
      emit("lea %s, [%s]", reg(n->lstr.reg), n->lstr.str);
      return n->next;

   default:
      panic("unimplemented ir_node type '%s'", ir_node_type_str[n->type]);
   }
}
