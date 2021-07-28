#include <string.h>
#include "riscv/cpu.h"
#include "emit_ir.h"
#include "strdb.h"
#include "error.h"
#include "regs.h"
#include "bcc.h"

static uintreg_t size_stack;

ir_node_t* emit_ir(const ir_node_t* n) {
   const char* instr;
   bool flag = false, flag2 = false;
   intreg_t off;
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
   // flag2: swap operands
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
      goto ir_binary;
   case IR_ISTGR:
      flag2 = true;
      fallthrough;
   case IR_ISTLT:
      instr = "slt";
   {
   ir_binary:;
      struct ir_value av;
      struct ir_value bv;
         
      if (flag2) {
         av = n->binary.b;
         bv = n->binary.a;
      } else {
         av = n->binary.a;
         bv = n->binary.b;
      }
      const char* a;
      if (av.type == IRT_REG) {
         a = reg_op(av.reg);
      } else {
         a = reg_op(n->binary.dest);
         emit("li %s, %jd", a, av.sVal);
      }
      if (bv.type == IRT_REG) {
         emit("%s  %s, %s, %s", instr,
            reg_op(n->binary.dest),
            a,
            reg_op(bv.reg));
      } else {
         emit("%si %s, %s, %jd", instr,
            reg_op(n->binary.dest),
            a,
            bv.sVal);
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
   ir_read:;
      ir_node_t* next = n->next;
      ir_reg_t dest = n->read.dest;
      if (optim_level >= 1
         && ir_is(next, IR_IICAST)
         && n->read.dest == next->iicast.src) {
         dest = next->iicast.dest;
         next = next->next;
      }
      switch (n->move.size) {
      case IRS_BYTE:
      case IRS_CHAR:    instr = n->read.sign_extend ? "lb  " : "lbu "; break;
      case IRS_SHORT:   instr = n->read.sign_extend ? "lh  " : "lhu "; break;
#if BITS == 32
      case IRS_PTR:
      case IRS_INT:
      case IRS_LONG:    instr = "lw  "; break;
#else
      case IRS_INT:     instr = n->read.sign_extend ? "lw  " : "lwu "; break;
      case IRS_LONG:
      case IRS_PTR:     instr = "ld  "; break;
#endif
      default:          panic("unsupported operand size '%s'", ir_size_str[n->read.size]);
      }
      if (flag) {
         emit("%s %s, %d(fp)", instr, reg_op(dest), off);
      } else {
         emit("%s %s, 0(%s)", instr, reg_op(dest), reg_op(n->read.src));
      }
      return next;
   }
   case IR_WRITE:
   ir_write:;
      switch (n->move.size) {
      case IRS_BYTE:
      case IRS_CHAR:    instr = "sb  "; break;
      case IRS_SHORT:   instr = "sh  "; break;
#if BITS == 32
      case IRS_PTR:
      case IRS_INT:
      case IRS_LONG:    instr = "sw  "; break;
#else
      case IRS_INT:     instr = "sw  "; break;
      case IRS_LONG:
      case IRS_PTR:     instr = "sd  "; break;
#endif
      default:          panic("unsupported operand size '%s'", ir_size_str[n->move.size]);
      }
      if (flag) {
         emit("%s %s, %d(fp)", instr, reg_op(n->move.src), off);
      } else {
         emit("%s %s, 0(%s)", instr, reg_op(n->move.src), reg_op(n->move.dest));
      }
      return n->next;
   case IR_PROLOGUE:
   {
      if (!(n->func->attrs & ATTR_STATIC))
         emit(".global %s", n->func->name);
      emit("%s:", n->func->name);

      if (get_mach_opt("stack-check")->bVal) {
         emit("addi sp, sp, -16");
         emit(SW " ra, %zu(sp)", 16 - REGSIZE);
         emit("call __check_sp");
         emit(LW " ra, %zu(sp)", 16 - REGSIZE);
         emit("addi sp, sp, 16");
         request_builtin("__check_sp");
      }

      // stack allocation
      const size_t num_reg_params = my_min(8, buf_len(n->func->params));
      size_stack = sizeof_scope(n->func->scope);
      size_stack += 3 * REGSIZE;
      size_stack += num_reg_params * REGSIZE;
      size_stack = align_stack_size(size_stack);
      emit("addi sp, sp, -%zu", size_stack);
      uintreg_t sp = size_stack;
      emit(SW "   fp, %zu(sp)", sp -= REGSIZE);
      emit(SW "   ra, %zu(sp)", sp -= REGSIZE);
      for (size_t i = 0; i < num_reg_params; ++i) {
         emit(SW "   a%zu, %zu(sp)", i, sp -= REGSIZE);
      }
      emit("addi fp, sp, %zu", size_stack);

      uintreg_t fp = REGSIZE * (3 + num_reg_params);
      assign_scope(n->func->scope, &fp);

      return n->next;
   }
   case IR_EPILOGUE:
      if (!strcmp(n->func->name, "main"))
         emit("mv   a0, x0");
      emit("%s.ret:", n->func->name);
      emit(LW "   fp, %zu(sp)", size_stack - 1*REGSIZE);
      emit(LW "   ra, %zu(sp)", size_stack - 2*REGSIZE);
      emit("addi sp, sp, %zu",     size_stack);
      emit("ret");
      return n->next;

   case IR_IRET:
      if (n->unary.reg != 0) {
         emit("mv   a0, %s", reg_op(n->unary.reg));
      }
      fallthrough;
   case IR_RET:
      emit("j   %s.ret", n->func->name);
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
      emit("%s %s, x0, %s", instr, reg_op(n->cjmp.reg), n->cjmp.label);
      return n->next;
   case IR_FLOOKUP:
   case IR_GLOOKUP:
      emit("la   %s, %s", reg_op(n->lstr.reg), n->lstr.str);
      return n->next;
   case IR_LOOKUP:
   {
      const char* dest = reg_op(n->lookup.reg);
      const size_t addr = n->lookup.scope->vars[n->lookup.var_idx].addr;
      const ir_reg_t reg = n->lookup.reg;
      if (optim_level >= 2
         && n->next
         && ((n->next->type == IR_READ && n->next->move.src == reg)
         || (n->next->type == IR_WRITE && n->next->move.dest == reg))) {
         flag = true;
         off = -(intreg_t)addr;
         n = n->next;
         if (n->move.dest == reg || !ir_is_used(n->next, reg)) {
            if (n->type == IR_READ) goto ir_read;
            else goto ir_write;
         }
      }
      emit("addi %s, fp, -%zu", dest, addr);
      if (flag) {
         if (n->type == IR_READ) goto ir_read;
         else goto ir_write;
      }
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
         const char* dest = reg_op(n->fparam.reg);
         const size_t addr = (3 + n->fparam.idx) * REGSIZE;
         const ir_reg_t reg = n->lookup.reg;
         if (optim_level >= 2
            && n->next
            && ((n->next->type == IR_READ && n->next->move.src == reg)
            || (n->next->type == IR_WRITE && n->next->move.dest == reg))) {
            flag = true;
            off = -(intreg_t)addr;
            n = n->next;
            if (n->move.dest == reg || !ir_is_used(n->next, reg)) {
               if (n->type == IR_READ) goto ir_read;
               else goto ir_write;
            }
         }
         emit("addi %s, fp, -%zu", dest, addr);
         if (flag) {
            if (n->type == IR_READ) goto ir_read;
            else goto ir_write;
         }
      } else {
         // TODO: fix this
         panic("fparam.idx >= 8");
      }
      return n->next;
   case IR_INEG:
      emit("sub %s, x0, %s", reg_op(n->unary.reg), reg_op(n->unary.reg));
      return n->next;
   case IR_INOT:
      emit("not %s, %s", reg_op(n->unary.reg), reg_op(n->unary.reg));
      return n->next;
   case IR_BNOT:
      emit("seqz %s, %s", reg_op(n->unary.reg), reg_op(n->unary.reg));
      return n->next;
   case IR_ISTEQ:
      instr = "seqz";
      goto ir_seqz;
   case IR_ISTNE:
      instr = "snez";
   {
   ir_seqz:;
      const char* dest = reg_op(n->binary.dest);
      const char* a;
      if (n->binary.a.type == IRT_REG) {
         a = reg_op(n->binary.a.reg);
      } else {
         a = reg_op(n->binary.dest);
         emit("li %s, %jd", a, n->binary.b.sVal);
      }
      if (n->binary.b.type == IRT_REG) {
         emit("sub %s, %s, %s", dest, a, reg_op(n->binary.b.reg));
      } else {
         const char* sign = n->binary.b.sVal < 0 ? "" : "-";
         emit("addi %s, %s, %s%jd", dest, a, sign, n->binary.b.sVal);
      }
      emit("%s %s, %s", instr, dest, dest);
      return n->next;
   }
   // flag: unsigned
   // flag2: swap operands
   case IR_USTLE:
      flag = true;
      fallthrough;
   case IR_ISTLE:
      flag2 = true;
      goto ir_istge;
   case IR_USTGE:
      flag = true;
      fallthrough;
   case IR_ISTGE:
   {
   ir_istge:;
      struct ir_value av, bv;
      if (flag2) {
         av = n->binary.b;
         bv = n->binary.a;
      } else {
         av = n->binary.a;
         bv = n->binary.b;
      }
      const char* dest = reg_op(n->binary.dest);
      const char* a;
      if (av.type == IRT_REG) {
         a = reg_op(av.reg);
      } else {
         a = dest;
         emit("li %s, %jd", a, av.sVal);
      }
      if (bv.type == IRT_REG) {
         emit("%s %s, %s, %s", flag ? "sltu" : "slt", dest, a, reg_op(bv.reg));
      } else {
         emit("%s %s, %s, %jd", flag ? "sltiu" : "slti", dest, a, bv.sVal);
      }
      emit("xori %s, %s, 1", dest, dest);
      return n->next;
   }
   // flag2: swap operands
   case IR_USTGR:
      flag2 = true;
      fallthrough;
   case IR_USTLT:
   {
      const char* dest = reg_op(n->binary.dest);
      struct ir_value av, bv;
      if (flag2) {
         av = n->binary.b;
         bv = n->binary.a;
      } else {
         av = n->binary.a;
         bv = n->binary.b;
      }
      const char* a;
      if (av.type == IRT_REG) {
         a = reg_op(av.reg);
      } else {
         a = dest;
         emit("li %s, %jd", a, av.sVal);
      }
      if (bv.type == IRT_REG) {
         emit("sltu %s, %s, %s", dest, a, reg_op(bv.reg));
      } else {
         emit("sltiu %s, %s, %jd", dest, a, bv.sVal);
      }
      return n->next;
   }
   case IR_IICAST:
   {
      const char* dest = reg_op(n->iicast.dest);
      const char* src = reg_op(n->iicast.src);
      if (n->iicast.ds == n->iicast.ss) {
         if (n->iicast.dest != n->iicast.src)
            emit("mv %s, %s", dest, src);
      } else if (n->iicast.ds < n->iicast.ss) {
         switch (n->iicast.ss) {
         case IRS_BYTE:
         case IRS_CHAR:
            emit("andi %s, %s, 255", dest, src);
            break;
         case IRS_SHORT:
            emit("slli %s, %s, 16", dest, src);
            emit("srai %s, %s, 16", dest, dest);
            break;
#if BITS == 64
         case IRS_INT:
            emit("slli %s, %s, 32", dest, src);
            emit("srai %s, %s, 32", dest, dest);
            break;
#endif
         default:
            panic("unreachable reached, ds=%s, ss=%s", ir_size_str[n->iicast.ds], ir_size_str[n->iicast.ss]);
         }
      } else {
         if (n->iicast.sign_extend) {
            // TODO: implement sign extension
         }
         emit("mv %s, %s", dest, src);
      }
      return n->next;
   }
   // flag: relative
   // flag2: has return value

   case IR_RCALL:
      flag = true;
      goto ir_call;
   case IR_IRCALL:
      flag = flag2 = true;
      goto ir_call;
   case IR_IFCALL:
      flag2 = true;
      goto ir_call;
   case IR_FCALL:
   {
   ir_call:;
      ir_reg_t dest = flag ? n->rcall.dest : n->ifcall.dest;
      struct ir_node** params = flag ? n->rcall.params : n->ifcall.params;
      const size_t np = buf_len(params);
      uintreg_t n_stack = ((flag2 ? dest : 0) + np) * REGSIZE;
      const uintreg_t saved_sp = n_stack;
      uintreg_t sp = saved_sp;
      n_stack = align_stack_size(n_stack);
      
      emit("addi sp, sp, -%zu", n_stack);

      if (flag2) {
         for (size_t i = 0; i < dest; ++i) {
            emit(SW " %s, %zu(sp)", reg_op(i), sp -= REGSIZE);
         }
         sp = saved_sp;
      }

      for (size_t i = 0; i < my_min(8, np); ++i) {
         ir_node_t* tmp = params[i];
         while ((tmp = emit_ir(tmp)) != NULL);
         emit(SW " a0, %zu(sp)", sp -= REGSIZE);
      }

      if (np) {
         for (size_t i = np - 1; i >= 8; --i) {
            ir_node_t* tmp = params[i];
            while ((tmp = emit_ir(tmp)) != NULL);
            emit(SW " a0, %zu", sp -= REGSIZE);
         }
      }

      if (flag) {
         ir_node_t* tmp = n->rcall.addr;
         while ((tmp = emit_ir(tmp)) != NULL);
         emit("mv t0, a0");
      }

      sp = saved_sp;
      for (size_t i = 0; i < my_min(8, np); ++i) {
         emit(LW " a%zu, %zu(sp)", i, sp -= REGSIZE);
      }
      if (flag) {
         emit("jalr t0");
      } else {
         emit("call %s", n->ifcall.name);
      }
      if (flag2 && dest != 0) {
         emit("mv %s, a0", reg_op(dest));
         sp = saved_sp;
         for (size_t i = 0; i < dest; ++i) {
            emit(LW " %s, %zu(sp)", reg_op(i), sp -= REGSIZE);
         }
      }
      emit("addi sp, sp, %zu", n_stack);
      return n->next;
   }

   case IR_IMUL:
   case IR_UMUL:
      instr = "mul";
      goto ir_mul;
   case IR_IDIV:
      instr = "div";
      goto ir_mul;
   case IR_UDIV:
      instr = "divu";
      goto ir_mul;
   case IR_IMOD:
      instr = "rem";
      goto ir_mul;
   case IR_UMOD:
      instr = "remu";
   {
   ir_mul:;
      struct ir_value av, bv;
      const char* dest = reg_op(n->binary.dest);
      av = n->binary.a;
      bv = n->binary.b;

      if (!riscv_cpu.has_mult) {
         panic("soft-multiplication is currently not implemented");
#if BITS == 32
         char f[] = "__mulxi32";
#else
         char f[] = "__mulxi64";
#endif
         memcpy(f + 2, instr, 3);
         f[5] = strlen(instr) == 3 ? 's' : 'u';
         request_builtin(f);
         return n->next;
      }
      
      if (n->binary.b.type == IRT_UINT && n->binary.a.type == IRT_REG) {
         av = n->binary.b;
         bv = n->binary.a;
      }
      const char* a;
      const char* b;


      if (av.type == IRT_REG) {
         a = reg_op(av.reg);
      } else {
         a = dest;
         emit("li %s, %jd", a, av.sVal);
      }

      if (bv.type == IRT_REG) {
         b = reg_op(bv.reg);
      } else {
         b = "s1";
         emit(SW " s1, -4(sp)");
         emit("li %s, %jd", b, bv.sVal);
      }

      emit("%s %s, %s, %s", instr, dest, a, b);


      if (bv.type == IRT_UINT) {
         emit(LW " s1, -4(sp)");
      }

      return n->next;
   }


   default:
      panic("unsupported ir_node type '%s'", ir_node_type_str[n->type]);
   }
   panic("unreachable reached, n->type='%s'", ir_node_type_str[n->type]);
}
