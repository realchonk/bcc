#include <string.h>
#include <ctype.h>
#include <math.h>
#include "target.h"
#include "error.h"
#include "strdb.h"
#include "regs.h"
#include "bcc.h"
#include "common.h"


static ir_node_t* emit_ir(const ir_node_t* n) {
   const char* instr;
   switch (n->type) {
   case IR_NOP:
      emit("nop");
      return n->next;
   case IR_MOVE:
   {
      const char* dest;
      const char* src;
      reg_op(dest, n->move.dest, n->move.size);
      reg_op(src, n->move.src, n->move.size);
      
      emit("mov %s, %s", dest, src);
      return n->next;
   }
   case IR_LOAD:
   {
      const char* dest;
      reg_op(dest, n->load.dest, n->load.size);
      if (!n->load.value) emit_clear(dest);
      else emit("mov %s, %jd", dest, (intmax_t)n->load.value);
      return n->next;
   }
   case IR_IADD:
   case IR_ISUB:
   {
      const char* a = irv2str(&n->binary.a, n->binary.size);
      const char* b = irv2str(&n->binary.b, n->binary.size);
      const char* dest;
      reg_op(dest, n->binary.dest, n->binary.size);

      if (n->binary.a.type == IRT_REG && n->binary.dest == n->binary.a.reg) {
         if (n->binary.b.type == IRT_UINT && n->binary.b.uVal == 1) {
            emit("%s %s", (n->type == IR_IADD ? "inc" : "dec"), dest);
         } else emit("%s %s, %s", (n->type == IR_IADD ? "add" : "sub"), dest, b);
      } else {
         emit("lea %s, [%s %c %s]", dest, a, (n->type == IR_IADD ? '+' : '-'), b);
      }
      return n->next;
   }
   case IR_IAND:
      instr = "and";
      goto ir_binary;
   case IR_IOR:
      instr = "or";
      goto ir_binary;
   case IR_IXOR:
      instr = "xor";
      goto ir_binary;
   case IR_ILSL:
      instr = "shl";
      goto ir_binary;
   case IR_ILSR:
      instr = "shr";
      goto ir_binary;
   case IR_IASR:
      instr = "sar";
   {
   ir_binary:;
      const char* a = irv2str(&n->binary.a, n->binary.size);
      const char* b = irv2str(&n->binary.b, n->binary.size);
      const char* dest;
      reg_op(dest, n->binary.dest, n->binary.size);

      if (n->binary.a.type != IRT_REG || n->binary.dest != n->binary.a.reg) {
         emit("mov %s, %s", dest, a);
      }
      emit("%s %s, %s", instr, dest, b);
      return n->next;
   }
   case IR_INOT:
      instr = "not";
      goto ir_neg;
   case IR_INEG:
      instr = "neg";
   {
   ir_neg:;
      const char* reg;
      reg_op(reg, n->unary.reg, n->unary.size);
      emit("%s %s", instr, reg);
      return n->next;
   }
   case IR_IMUL:
      instr = "imul";
      goto ir_mul;
   case IR_UMUL:
      instr = "mul";
   {
   ir_mul:;
      const char* a = irv2str(&n->binary.a, n->binary.size);
      const char* b = irv2str(&n->binary.b, n->binary.size);
      const char* eax;
      const char* dest;
      reg_op(eax, 0, n->binary.size);
      reg_op(dest, n->binary.dest, n->binary.size);
      if (n->binary.dest != reg_dxi) emit("push %s", reg_dx);
      if (n->binary.a.type != IRT_REG || n->binary.dest != n->binary.a.reg) {
         emit("mov %s, %s", dest, a);
      }
      if (n->binary.dest != 0) {
         emit("push %s", reg_ax);
         emit("mov %s, %s", eax, dest);
      }

      if (n->binary.b.type == IRT_UINT) {
         emit("mov %s, %ju", reg_dx, n->binary.b.uVal);
         b = reg_dx;
      }
      emit("%s %s", instr, b);

      if (n->binary.dest != 0) {
         emit("mov %s, %s", dest, eax);
         emit("pop %s", reg_ax);
      }
      if (n->binary.dest != reg_dxi) emit("pop %s", reg_dx);
      return n->next;
   }
   case IR_IDIV:
      instr = "sdiv";
      goto ir_div;
   case IR_UDIV:
      instr = "udiv";
      goto ir_div;
   case IR_IMOD:
      instr = "smod";
      goto ir_div;
   case IR_UMOD:
      instr = "umod";
   {
   ir_div:;
      const char* a = irv2str(&n->binary.a, IRS_PTR);
      const char* b = irv2str(&n->binary.b, IRS_PTR);
      if (n->binary.dest != 0) emit("push %s", reg_ax);
      char f[] = "__divxixx";
      snprintf(f, sizeof(f), "__%s%ci%zu", instr + 1, *instr, irs2sz(n->binary.size) * 8);

      emit("push %s", b);
      emit("push %s", a);

      emit("call %s", f);
      emit("add %s, %zu", reg_sp, REGSIZE * 2);

      if (n->binary.dest != 0) {
         emit("mov %s, %s", mreg(n->binary.dest), reg_ax);
         emit("pop %s", reg_ax);
      }

      request_builtin(f);

      return n->next;
   }
   case IR_BEGIN_SCOPE:
      emit("");
      buf_push(stack_alloc, stack_cur_alloc);
      stack_cur_alloc = NULL;
      return n->next;
   case IR_END_SCOPE:
      emit("");
      free_stack();
      return n->next;
   case IR_READ:
   {
      const char* dest;
      const char* src =  mreg(n->move.src);
      reg_op(dest, n->move.dest, n->move.size);
      emit("mov %s, %s [%s]", dest, nasm_size(n->move.size), src);
      return n->next;
   }
   case IR_WRITE:
   {
      const char* dest = mreg(n->move.dest);;
      const char* src;
      reg_op(src, n->move.src, n->move.size);
      emit("mov %s [%s], %s", nasm_size(n->move.size), dest, src);
      return n->next;
   }
   case IR_PROLOGUE:
   {
      buf_push(defined, n->func->name);
      if (!(n->func->attrs & ATTR_STATIC))
         emit("global %s", n->func->name);
      emit("%s:", n->func->name);
      if (has_mach_opt("stack-check")) {
         add_unresolved(strint("puts"));
         add_unresolved(strint("abort"));
         request_builtin("__check_sp");
         emit("call __check_sp");
         buf_push(defined, strint("__check_sp"));
      }
      emit("push %s", reg_bp);
      emit("mov %s, %s", reg_bp, reg_sp);
      esp = REGSIZE * 2;
      size_t addr = REGSIZE;
#if BCC_x86_64
      for (size_t i = 0; i < my_min(buf_len(n->func->params), arraylen(param_regs)); ++i) {
         emit("push %s", mreg(param_regs[i]));
         esp += REGSIZE;
         addr += REGSIZE;
      }
#endif
      const size_t sz = align_stack_size(sizeof_scope(n->func->scope));
      if (sz) emit("sub %s, %zu", reg_sp, sz);
      assign_scope(n->func->scope, &addr);
      esp += sz;
      stack_cur_alloc = NULL;
      return n->next;
   }
   case IR_EPILOGUE:
      if (strcmp(n->func->name, "main") == 0 && n->prev && n->prev->prev && n->prev->prev->type != IR_IRET)
         emit_clear(reg_ax);
      emit(".ret:");
      emit("leave");
      emit("ret\n\n");
      return n->next;
   case IR_IRET:
      if (n->unary.reg != 0) {
         const char* reg;
         const char* eax;
         reg_op(reg, n->unary.reg, n->unary.size);
         reg_op(eax, 0, n->unary.size);
         emit("mov %s, %s", eax, reg);
      }
      fallthrough;
   case IR_RET:
      if (n->next && n->next->type != IR_END_SCOPE && n->next->next && n->next->next->type == IR_EPILOGUE)
         emit("jmp .ret");
      return n->next;
   case IR_IICAST:
   {
      if (n->iicast.ds < n->iicast.ss) {
         if (n->iicast.dest != n->iicast.src) {
            const char* dest;
            const char* src;
            reg_op(dest, n->iicast.dest, n->iicast.ds);
            reg_op(src, n->iicast.src, n->iicast.ds);
            emit("mov %s, %s", dest, src);
         } else {
            uintreg_t mask;
            switch (n->iicast.ds) {
            case IRS_BYTE:
            case IRS_CHAR:    mask = 0x000000ff; break;
            case IRS_SHORT:   mask = 0x0000ffff; break;
#if BCC_x86_64
            case IRS_INT:
               emit("mov %s, %s", reg32(n->iicast.dest), reg32(n->iicast.dest));
               fallthrough;
#endif
            default:          mask = 0;
            }
            if (mask) emit("and %s, 0x%jx", mreg(n->iicast.dest), (uintmax_t)mask);
         }
      } else if (n->iicast.ds > n->iicast.ss) {
         const char* dest;
         const char* src;
         if (n->iicast.ds != IRS_LONG && n->iicast.ds != IRS_PTR) {
            reg_op(dest, n->iicast.dest, n->iicast.ds);
            reg_op(src, n->iicast.src, n->iicast.ss);
            emit("%s %s, %s", n->iicast.sign_extend ? "movsx" : "movzx", dest, src);
         }
      } else {
         if (n->iicast.dest != n->iicast.src) {
            const char* dest;
            const char* src;
            reg_op(dest, n->iicast.dest, n->iicast.ds);
            reg_op(src, n->iicast.src, n->iicast.ds);
            emit("mov %s, %s", dest, src);
         }
      }
      return n->next;
   }
   case IR_IFCALL:
   case IR_FCALL:
   {
      const size_t np = buf_len(n->ifcall.params);
      size_t i;
      uintreg_t padding;
      for (i = 0; i < n->ifcall.dest; ++i) {
         emit("push %s", mreg(i));
         esp += REGSIZE;
      }
#if BCC_x86_64
      if (np < arraylen(param_regs)) padding = 16 - (esp % 16);
      else padding = 16 - (esp + (np - arraylen(param_regs) * REGSIZE) % 16);
#else
      padding = 16 - ((esp + np * REGSIZE) % 16);
#endif
      if (padding == 16) padding = 0;
      if (padding) emit("sub %s, %u", reg_sp, padding);
#if BCC_x86_64
      if (np > arraylen(param_regs)) {
         for (i = np; i != arraylen(param_regs); --i) {
            ir_node_t* tmp = n->ifcall.params[i - 1];
            while ((tmp = emit_ir(tmp)) != NULL);
            emit("push %s", mreg(n->ifcall.dest));
         }
      }
      for (i = 0; i < my_min(np, arraylen(param_regs)); ++i) {
         ir_node_t* tmp = n->ifcall.params[i];
         while ((tmp = emit_ir(tmp)) != NULL);
         if (i >= my_min(np, arraylen(param_regs))) {
            emit("mov %s, %s", mreg(param_regs[i]), mreg(n->ifcall.dest));
            break;
         }
         else emit("push %s", mreg(n->ifcall.dest));
      }
      for (; i > 0; --i) {
         emit("pop %s", mreg(param_regs[i - 1]));
      }
      bool variadic = false;
      for (size_t i = 0; i < buf_len(cunit.funcs); ++i) {
         if (n->ifcall.name == cunit.funcs[i]->name) {
            variadic = cunit.funcs[i]->variadic;
            break;
         }
      }
      if (variadic) emit_clear(reg_ax);
      if (is_defined(n->ifcall.name)) emit("call %s", n->ifcall.name);
      else emit("call [rel %s wrt ..got]", n->ifcall.name);
      size_t add_rsp = padding;
      if (np > arraylen(param_regs)) add_rsp += (np - arraylen(param_regs)) * REGSIZE;
      if (add_rsp) emit("add %s, %u", reg_sp, add_rsp);

#else
      for (size_t i = np; i != 0; --i) {
         ir_node_t* tmp = n->ifcall.params[i - 1];
         while ((tmp = emit_ir(tmp)) != NULL);
         emit("push %s", mreg(n->ifcall.dest));
      }
      emit("call %s", n->ifcall.name);
      emit("add %s, %u", reg_sp, padding + REGSIZE * np);
#endif
      if (n->ifcall.dest != 0 && n->type != IR_FCALL) {
         emit("mov %s, %s", mreg(n->ifcall.dest), reg_ax);
      }
      for (size_t i = n->ifcall.dest; i != 0; --i) {
         emit("pop %s", mreg(i - 1));
         esp -= REGSIZE;
      }
      return n->next;
   }
   case IR_LOOKUP:
   {
      size_t idx = n->lookup.scope->vars[n->lookup.var_idx].addr;
#if BCC_x86_64
      idx += my_min(arraylen(param_regs), buf_len(cur_func->params)) * REGSIZE;
#endif
      // TODO: experimental
      if (optim_level >= 2) {
         if (ir_is(n->next, IR_READ) && n->next->move.src == n->lookup.reg) {
            ir_node_t* read = n->next;
            if (ir_isv(read->next, IR_IADD, IR_ISUB, NUM_IR_NODES) && read->next->binary.dest == read->move.dest) {
               ir_node_t* inc = read->next;
               if (inc->binary.a.type != IRT_REG || inc->binary.a.reg != inc->binary.dest) goto lookup_lea;
               if (ir_is(inc->next, IR_WRITE) && inc->next->move.src == inc->binary.dest && inc->next->move.dest == n->lookup.reg) {
                  ir_node_t* write = inc->next;
                  if (inc->binary.b.type == IRT_UINT) {
                     if (inc->binary.b.uVal == 1)
                        emit("%s %s [%s - %zu]", (inc->type == IR_IADD ? "inc" : "dec"), nasm_size(read->move.size), reg_bp, idx);
                     else emit("%s %s [%s - %zu], %s", (inc->type == IR_IADD ? "add" : "sub"),
                           nasm_size(read->move.size), reg_bp, idx, inc->binary.b.uVal);
                  } else if (inc->binary.b.type == IRT_REG) {
                     const char* reg;
                     reg_op(reg, inc->binary.b.reg, write->move.size);
                     emit("%s %s [%s - %zu], %s", (inc->type == IR_IADD ? "add" : "sub"),
                           nasm_size(read->move.size), reg_bp, idx, reg);
                  } else goto lookup_lea;
                  return write->next;
               }
               else goto lookup_lea;
            }
            const char* dest;
            reg_op(dest, n->next->move.dest, read->move.size);
            emit("mov %s, %s [%s - %zu]", dest, nasm_size(read->move.size), reg_bp, idx);
            return read->next;
         } else if (ir_is(n->next, IR_WRITE) && n->next->move.dest == n->lookup.reg) {
            ir_node_t* write = n->next;
            const char* src;
            reg_op(src, write->move.src, write->move.size);
            emit("mov %s [%s - %zu], %s", nasm_size(write->move.size), reg_bp, idx, src);
            return write->next;
         }
      }
   lookup_lea:
      emit("lea %s, [%s - %zu]", mreg(n->lookup.reg), reg_bp, idx);
      return n->next;
   }
   case IR_FPARAM:
   {
#if BCC_x86_64
      if (n->fparam.reg < arraylen(param_regs)) {
         emit("lea %s, [%s - %u]", mreg(n->fparam.reg), reg_bp, REGSIZE * (n->fparam.idx + 1));
      } else {
         emit("lea %s, [%s + %u]", mreg(n->fparam.reg), reg_bp, REGSIZE * (n->fparam.idx + 2 - arraylen(param_regs)));
      }
#else
      emit("lea %s, [%s + %u]", mreg(n->fparam.reg), reg_bp, REGSIZE * (n->fparam.idx + 2));
#endif
      return n->next;
   }
   case IR_LSTR:
   {
      const struct strdb_ptr* ptr;
      strdb_add(n->lstr.str, &ptr);
#if BCC_x86_64
      emit("lea %s, [rel __strings + %u]", mreg(n->lstr.reg), ptr->idx);
#else
      emit("lea %s, [__strings + %u]", mreg(n->lstr.reg), ptr->idx);
#endif
      return n->next;
   }
   case IR_ISTEQ:
      instr = "sete";
      goto setcc;
   case IR_ISTNE:
      instr = "setne";
      goto setcc;
   case IR_ISTGR:
      instr = "setg";
      goto setcc;
   case IR_ISTGE:
      instr = "setge";
      goto setcc;
   case IR_ISTLT:
      instr = "setl";
      goto setcc;
   case IR_ISTLE:
      instr = "setle";
      goto setcc;
   case IR_USTGR:
      instr = "seta";
      goto setcc;
   case IR_USTGE:
      instr = "setae";
      goto setcc;
   case IR_USTLT:
      instr = "setb";
      goto setcc;
   case IR_USTLE:
      instr = "setbe";
   {
   setcc:;
      const char* dest;
      const char* a = irv2str(&n->binary.a, n->binary.size);
      const char* b = irv2str(&n->binary.b, n->binary.size);
      reg_op(dest, n->binary.dest, n->binary.size);

      emit("cmp %s, %s", a, b);
      emit("%s %s", instr, reg8(n->binary.dest));
      if (n->binary.size > IRS_CHAR) emit("movzx %s, %s", dest, reg8(n->binary.dest));
      return n->next;
   }
   case IR_LABEL:
      emit("%s:", n->str);
      return n->next;
   case IR_JMP:
      emit("jmp %s", n->str);
      return n->next;
   case IR_JMPIF:
      instr = "jnz";
      goto jmpif;
   case IR_JMPIFN:
      instr = "jz";
   {
   jmpif:;
      const char* reg;
      reg_op(reg, n->cjmp.reg, n->cjmp.size);
      emit("test %s, %s", reg, reg);
      emit("%s %s", instr, n->cjmp.label);
      return n->next;
   }
   case IR_ALLOCA:
   {
      struct stack_alloc_entry e;
      const char* dest;
      const char* num = irv2str(&n->alloca.size, IRS_PTR);
      reg_op(dest, n->alloca.dest, IRS_PTR);
      
      e.is_const = n->alloca.size.type == IRT_UINT;
      
      emit("sub %s, %s", reg_sp, num);
      if (e.is_const) e.sz = n->alloca.size.uVal;
      else {
         e.sz = n->alloca.var->addr + REGSIZE;
         emit("mov %s [%s - %zu], %s", nasm_size(IRS_PTR), reg_bp, e.sz, num);
      }
      emit("mov %s, %s", dest, reg_sp);

      buf_push(stack_cur_alloc, e);
      return n->next;
   }
   case IR_ARRAYLEN:
   {
      emit("mov %s, %s [%s - %zu]", mreg(n->lookup.reg), nasm_size(IRS_PTR),
            reg_bp, n->lookup.scope->vars[n->lookup.var_idx].addr + REGSIZE);
      return n->next;
   }
   case IR_COPY:
   {
#if BCC_x86_64
      const size_t align = esp & 15 ? 16 - (esp & 15) : 0;
      if (align) emit("sub %s, %zu", reg_sp, align);
      emit("mov %s, %s", mreg(param_regs[1]), mreg(n->copy.src));
      emit("mov %s, %s", mreg(param_regs[0]), mreg(n->copy.dest));
      emit("mov %s, %ju", mreg(param_regs[2]), n->copy.len);
      emit("call [rel memcpy wrt ..got]");
      if (align) emit("add %s, %zu", reg_sp, align);
#else
      esp += 12;
      const size_t align = esp & 15 ? 16 - (esp & 15) : 0;
      if (align) emit("sub %s, %zu", reg_sp, align);
      emit("push %ju", n->copy.len);
      emit("push %s", mreg(n->copy.src));
      emit("push %s", mreg(n->copy.dest));
      emit("call memcpy");
      emit("add esp, %zu", 12 + align);
#endif
      return n->next;
   }
   case IR_GLOOKUP:
   case IR_FLOOKUP:
      emit("lea %s, [%s]", mreg(n->lstr.reg), n->lstr.str);
      return n->next;
   case IR_BNOT:
   {
      const char* reg;
      const char* lower;
      reg_op(reg, n->unary.reg, n->unary.size);
      reg_op(lower, n->unary.reg, IRS_BYTE);
      emit("test %s, %s", reg, reg);
      emit("setz %s", lower);
      if (n->unary.size > IRS_CHAR)
         emit("movzx %s, %s", reg, lower);
      return n->next;
   }

   default: panic("unsupported ir_node type '%s'", ir_node_type_str[n->type]);
   }
}



