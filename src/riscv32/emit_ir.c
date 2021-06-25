#include <string.h>
#include "emit_ir.h"
#include "strdb.h"
#include "error.h"
#include "regs.h"
#include "bcc.h"

static size_t size_stack;

ir_node_t* emit_ir(const ir_node_t* n) {
   const char* instr;
   switch (n->type) {
   case IR_NOP:
      emit("nop");
      return n->next;
   case IR_MOVE:
      emit("mv   %s, %s", reg_op(n->move.dest), reg_op(n->move.src));
      return n->next;
   case IR_LOAD:
      emit("li   %s, %jd", reg_op(n->load.dest), (intmax_t)n->load.value);
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
      instr = "or ";
      goto ir_binary;
   case IR_IXOR:
      instr = "xor";
      goto ir_binary;
   case IR_ILSL:
      instr = "sll";
      goto ir_binary;
   case IR_ILSR:
      instr = "srl";
      goto ir_binary;
   case IR_IASR:
      instr = "sra";
   {
   ir_binary:;
      if (n->binary.b.type == IRT_REG) {
         emit("%s  %s, %s, %s", instr,
            reg_op(n->binary.dest),
            reg_op(n->binary.a.reg),
            reg_op(n->binary.b.reg));
      } else {
         emit("%si %s, %s, %d", instr,
            reg_op(n->binary.dest),
            reg_op(n->binary.a.reg),
            (int)n->binary.b.uVal);
      }
      return n->next;
   }
   case IR_BEGIN_SCOPE:
      emit("");
      return n->next;
   case IR_END_SCOPE:
      emit("");
      return n->next;
   case IR_READ:
   {
      ir_node_t* next = n->next;
      ir_reg_t dest = n->move.dest;
      bool sign_extend = false;
      if (optim_level >= 1
         && ir_is(next, IR_IICAST)
         && n->move.dest == next->iicast.src) {
         dest = next->iicast.dest;
         sign_extend = next->iicast.sign_extend;
         next = next->next;
      }
      switch (n->move.size) {
      case IRS_BYTE:
      case IRS_CHAR:    instr = sign_extend ? "lb  " : "lbu "; break;
      case IRS_SHORT:   instr = sign_extend ? "lh  " : "lhu "; break;
      case IRS_PTR:
      case IRS_INT:
      case IRS_LONG:    instr = "lw  ";  break;
      default:          panic("unsupported operand size '%s'", ir_size_str[n->move.size]);
      }
      emit("%s %s, 0(%s)", instr, reg_op(dest), reg_op(n->move.src));
      return next;
   }
   case IR_WRITE:
      switch (n->move.size) {
      case IRS_BYTE:
      case IRS_CHAR:    instr = "sb  "; break;
      case IRS_SHORT:   instr = "sh  "; break;
      case IRS_PTR:
      case IRS_INT:
      case IRS_LONG:    instr = "sw  "; break;
      default:          panic("unsupported operand size '%s'", ir_size_str[n->move.size]);
      }
      emit("%s %s, 0(%s)", instr, reg_op(n->move.src), reg_op(n->move.dest));
      return n->next;
   case IR_PROLOGUE:
   {
      if (!(n->func->attrs & ATTR_STATIC))
         emit(".global %s", n->func->name);
      emit("%s:", n->func->name);

      // stack allocation
      const size_t num_reg_params = my_min(8, buf_len(n->func->params));
      size_stack = sizeof_scope(n->func->scope);
      size_stack += 2 * REGSIZE;
      size_stack += num_reg_params * REGSIZE;
      if (size_stack & 15)
         size_stack = (size_stack & ~15) + 16;
      emit("addi sp, sp, -%zu", size_stack);
      size_t sp = size_stack;
      emit("sw   fp, %zu(sp)", sp -= REGSIZE);
      emit("sw   ra, %zu(sp)", sp -= REGSIZE);
      for (size_t i = 0; i < num_reg_params; ++i) {
         emit("sw   a%zu, %zu(sp)", i, sp -= REGSIZE);
      }
      emit("addi fp, sp, %zu", size_stack);

      sp = 20;
      assign_scope(n->func->scope, &sp);

      return n->next;
   }
   case IR_EPILOGUE:
      if (!strcmp(n->func->name, "main"))
         emit("mv   a0, x0");
      emit(".ret:");
      emit("lw   fp, %zu(sp)", size_stack - 1*REGSIZE);
      emit("lw   ra, %zu(sp)", size_stack - 2*REGSIZE);
      emit("addi sp, sp, %zu",     size_stack);
      emit("ret");
      return n->next;

   case IR_IRET:
      if (n->unary.reg != 0) {
         emit("mv   a0, %s", reg_op(n->unary.reg));
      }
      fallthrough;
   case IR_RET:
      emit("j   .ret");
      return n->next;
   case IR_LABEL:
      emit("%s:", n->str);
      return n->next;
   case IR_JMP:
      emit("j    %s", n->str);
      return n->next;
   case IR_JMPIF:
      instr = "bne ";
      goto ir_branch;
   case IR_JMPIFN:
      instr = "beq ";
   ir_branch:;
      emit("%s %s, x0, %s", instr, n->cjmp.reg, n->cjmp.label);
      return n->next;
   case IR_FLOOKUP:
   case IR_GLOOKUP:
      emit("la   %s, %s", reg_op(n->lstr.reg), n->lstr.str);
      return n->next;
   case IR_LOOKUP:
   {
      size_t addr = n->lookup.scope->vars[n->lookup.var_idx].addr;
      emit("addi %s, fp, -%zu", reg_op(n->lookup.reg), addr);
      return n->next;
   }
   case IR_ALLOCA:
      return n->next;
   case IR_LSTR:
   {
      const struct strdb_ptr* ptr;
      strdb_add(n->lstr.str, &ptr);
      emit("la   %s, __strings + %zu", reg_op(n->lstr.reg), ptr->idx);
      return n->next;
   }
   case IR_FPARAM:
      if (n->fparam.idx < 8) {
         emit("addi %s, sp, %zu", reg_op(n->fparam.reg), size_stack - (n->fparam.idx + 3) * REGSIZE);
      } else {

      }
      return n->next;
   
   default:
      panic("unsupported ir_node type '%s'", ir_node_type_str[n->type]);
   }
   panic("unreachable reached, n->type='%s'", ir_node_type_str[n->type]);
}
