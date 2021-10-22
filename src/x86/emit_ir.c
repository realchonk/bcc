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
#include "strdb.h"

static const struct function* cur_func;

#if BITS == 32
#define only_on_x86_64() panic("this should not be reached in i386")
#else
#define only_on_x86_64()
#endif

#define alloc_stack(n) ((n) && optim_level >= 1 ? emit("sub %s, %zu", REG_SP, (n)) : 0)
#define free_stack(n) ((n) && optim_level >= 1 ? emit("add %s, %zu", REG_SP, (n)) : 0)

void emit_init_int(enum ir_value_size irs, intmax_t val, bool is_unsigned);

static void emit_read(ir_reg_t d, enum ir_value_size irs, bool se, const char* addr, ...) {
   va_list ap;
   va_start(ap, addr);

   const char* dest = reg(d);
   const char* suffix = "";

#if BITS == 32
   if (irs < INT_INT) {
#else
   if (irs < INT_LONG) {
#endif
      if (se) {
         suffix = "sx";
      } else if (irs == INT_INT) {
         // only in 64bit
         dest = regs32[d];
      } else {
         suffix = "zx";
      }
   }

   emitraw("mov%s %s, %s PTR [", suffix, dest, as_size(irs));
   vemitraw(addr, ap);
   emit("]");

   va_end(ap);
}

ir_node_t* emit_ir(ir_node_t* n) {
   const char* instr;
   bool flag = false, flag2 = false;
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
      emit_read(n->rw.dest, n->rw.size, n->rw.sign_extend, "%s", reg(n->rw.src));
      return n->next;
   case IR_WRITE:
      emit("mov %s PTR [%s], %s", as_size(n->move.size), reg(n->move.dest), reg_wsz(n->move.src, n->move.size));
      return n->next;

   case IR_PROLOGUE:
   {
      const ir_reg_t max_reg = n->func->max_reg;
      //emit("# ir_max_reg(%s)=%u\n", n->func->name, (unsigned)max_reg);
      emit("push %s", REG_BP);
      emit("mov %s, %s", REG_BP, REG_SP);
      cur_func = n->func;


      // stack allocation
      size_t nrp;
#if BITS == 32
      if (max_reg >= 3) {
         nrp = max_reg - 2;
      } else {
         nrp = 0;
      }
#else
      nrp = my_min(arraylen(param_regs), buf_len(n->func->params));
#endif
      size_t size_stack = 0;
      size_stack += nrp * REGSIZE;
      size_stack += sizeof_scope(n->func->scope);
      size_stack = align_stack_size(size_stack);
      alloc_stack(size_stack);

      size_t sp = 0;
#if BITS == 32
      for (size_t i = 0; i < nrp; ++i) {
         sp += REGSIZE;
         emit("mov DWORD PTR [ebp - %zu], %s", sp, reg(i + 3));
      }
#else
      for (size_t i = 0; i < nrp; ++i) {
         sp += REGSIZE;
         emit("mov QWORD PTR [rbp - %zu], %s", sp, reg(param_regs[i]));
      }
#endif

      assign_scope(n->func->scope, &sp);

      return n->next;
   }
   case IR_FPARAM:
      emit("lea %s, [%s %+d]", reg(n->fparam.reg), REG_BP, calc_fp_addr(n->fparam.idx));
      return n->next;

   case IR_LOOKUP:
      emit("lea %s, [%s - %zu]", reg(n->lookup.reg), REG_BP, n->lookup.scope->vars[n->lookup.var_idx].addr);
      return n->next;

   case IR_EPILOGUE:
      emit_clear(REG_AX);
      emit("%s.ret:", n->func->name);
#if BITS == 32
      {
         size_t sp = 0;
         if (n->func->max_reg >= 3) {
            for (size_t i = 3; i <= n->func->max_reg; ++i) {
               sp += REGSIZE;
               emit("mov %s, DWORD PTR [ebp - %zu]", reg(i), sp);
            }
         }
      }
#endif
      emit("leave");
      emit("ret");
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
      emit("%s:", n->str);
      return n->next;
   case IR_JMP:
      emit("jmp %s", n->str);
      return n->next;
   case IR_JMPIF:
      instr = "jnz";
      goto ir_jmpifn;
   case IR_JMPIFN:
      instr = "jz";
   ir_jmpifn:
      emit("test %s, %s", reg(n->cjmp.reg), reg(n->cjmp.reg));
      emit("%s %s", instr, n->cjmp.label);
      return n->next;
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
         const char* set;
         const char* jmp;
         enum ir_node_type negation;
      };
      const struct entry es[] = {
         [IR_ISTEQ] = { "sete",  "je",  IR_ISTNE },
         [IR_ISTNE] = { "setne", "jne", IR_ISTEQ },
         [IR_ISTGR] = { "setg",  "jg",  IR_ISTLE },
         [IR_ISTGE] = { "setge", "jge", IR_ISTLT },
         [IR_ISTLT] = { "setl",  "jl",  IR_ISTGE },
         [IR_ISTLE] = { "setle", "jle", IR_ISTGR },
         [IR_USTGR] = { "seta",  "ja",  IR_USTLE },
         [IR_USTGE] = { "setae", "jae", IR_USTLT },
         [IR_USTLT] = { "setb",  "jb",  IR_USTGE },
         [IR_USTLE] = { "setbe", "jbe", IR_USTGR },
      };
      const char* dest = reg(n->binary.dest);
      const char* a = irv2str(&n->binary.a);
      const char* b = irv2str(&n->binary.b);

      emit("cmp %s, %s", a, b);
      if (optim_level >= 1 && n->next && (n->next->type == IR_JMPIF || n->next->type == IR_JMPIFN)
            && n->binary.dest == n->next->cjmp.reg) {
         const char* instr;
         if (n->next->type == IR_JMPIF)
            instr = es[n->type].jmp;
         else instr = es[es[n->type].negation].jmp;
         emit("%s %s", instr, n->next->str);
         return n->next->next;
      } else {
         emit("%s %s", es[n->type].set, regs8[n->binary.dest]);
         if (n->binary.size > IRS_CHAR)
            emit("movzx %s, %s", dest, regs8[n->binary.dest]);
         return n->next;
      }
   }

   case IR_GLOOKUP:
   case IR_FLOOKUP:
      emit("lea %s, [%s]", reg(n->lstr.reg), n->lstr.str);
      return n->next;
   
   case IR_IICAST:
   {
      const ir_reg_t dest = n->iicast.dest;
      const ir_reg_t src = n->iicast.src;
      const enum ir_value_size ds = n->iicast.ds;
      const enum ir_value_size ss = n->iicast.ss;
      const size_t size_ds = sizeof_irs(ds);
      const size_t size_ss = sizeof_irs(ss);
      if (size_ds < size_ss) {
         if (dest == src) {
            if (size_ds == 4 && size_ss == 8) {
               only_on_x86_64();
               reg(dest), reg(src); // verify registers
               emit("mov %s, %s", regs32[dest], regs32[src]);
            } else {
               emit("and %s, 0x%08jx", reg(dest), target_get_umax(ds));
            }
         } else {
            emit("mov %s, %s", reg_wsz(dest, ds), reg_wsz(src, ds));
         }
      } else if (size_ds > size_ss) {
         const char* suffix;
         if (n->iicast.sign_extend) {
            suffix = "sx";
         } else if (size_ds == 8 && size_ss == 4) {
            only_on_x86_64();
            suffix = "";
            emit("mov %s, %s", regs32[dest], regs32[src]);
            return n->next;
         } else {
            suffix = "zx";
         }
         emit("mov%s %s, %s", suffix, reg_wsz(dest, ds), reg_wsz(src, ss));
      } else if (n->iicast.dest != n->iicast.src) {
         emit("mov %s, %s", reg(dest), reg(src));
      }
      return n->next;
   }
   case IR_LSTR:
   {
      const struct strdb_ptr* ptr;
      strdb_add(n->lstr.str, &ptr);
      emit("lea %s, [__strings + %zu]", reg(n->lstr.reg), ptr->idx);
      return n->next;
   }
   
   // flag: relative
   // flag2: has return value
   case IR_RCALL:
      flag = true;
      goto ir_fcall;
   case IR_IRCALL:
      flag = flag2 = true;
      goto ir_fcall;
   case IR_IFCALL:
      flag2 = true;
      fallthrough;
   case IR_FCALL:
   {
   ir_fcall:;
      const ir_reg_t dest = n->call.dest;
      struct ir_node** params = n->call.params;
      const size_t np = buf_len(params);
      
      if (!flag && is_builtin_func(n->call.name))
         request_builtin(n->call.name);

      size_t n_stack = 1;
      n_stack += dest;
      n_stack += np;
      n_stack = align_stack_size(n_stack * REGSIZE);
      alloc_stack(n_stack);

      uintreg_t sp = n_stack - REGSIZE;

      for (size_t i = 0; i < dest; ++i) {
         emit("mov %s PTR [%s + %ju], %s", as_size(IRS_PTR), REG_SP, (uintmax_t)sp, reg(i));
         sp -= REGSIZE;
      }

      const size_t saved_sp = sp;

      sp = REGSIZE * (np - 1);
#if BITS == 32
      for (size_t i = np; i != 0; --i) {
         fcall_helper(&params[i - 1], dest, &sp);
      }
#else
      for (size_t i = 0; i < my_min(np, arraylen(param_regs)); ++i) {
         fcall_helper(&params[i], dest, &sp);
      }
      for (size_t i = np; i > arraylen(param_regs); --i) {
         fcall_helper(&params[i - 1], dest, &sp);
      }
#endif

      if (flag) {
         ir_node_t* tmp = n->call.addr;
         while ((tmp = emit_ir(tmp)) != NULL);
         emit("mov %s, %s", REG_AX, reg(n->call.dest));
      }

#if BITS == 64
      sp = REGSIZE * (np - 1);

      for (size_t i = 0; i < my_min(np, arraylen(param_regs)); ++i) {
         emit("mov %s, %s PTR [%s + %ju]", reg(param_regs[i]), as_size(IRS_PTR), REG_SP, (uintmax_t)sp);
         sp -= REGSIZE;
      }
#endif

      if (flag) {
         emit("call %s", reg(0));
      } else {
         emit("call %s", n->call.name);
      }

      if (dest != 0)
         emit("mov %s, %s", reg(dest), reg(0));

      sp = n_stack - REGSIZE;
      for (size_t i = 0; i < dest; ++i) {
         emit("mov %s, %s PTR [%s + %ju]", reg(i), as_size(IRS_PTR), REG_SP, (uintmax_t)sp);
         sp -= REGSIZE;
      }


      free_stack(n_stack);

      return n->next;
   }
   case IR_ASM:
      emit("%s", n->str);
      return n->next;

   case IR_FFPWR:
      emit("mov %s PTR [%s %+d], %s", as_size(n->ffprw.size), REG_BP, calc_fp_addr(n->ffprw.idx), reg_wsz(n->ffprw.reg, n->ffprw.size));
      return n->next;

   case IR_FFPRD:
      emit_read(n->ffprw.reg, n->ffprw.size, n->ffprw.sign_extend, "%s %+d", REG_BP, calc_fp_addr(n->ffprw.idx));
      return n->next;

   case IR_FGLWR:
      emit("mov %s PTR [%s], %s", as_size(n->fglrw.size), n->fglrw.name, reg_wsz(n->fglrw.reg, n->fglrw.size));
      return n->next;

   case IR_FGLRD:
      emit_read(n->fglrw.reg, n->fglrw.size, n->fglrw.sign_extend, "%s", n->fglrw.name);
      return n->next;

   case IR_FLUWR:
      emit("mov %s PTR [%s - %zu], %s", as_size(n->flurw.size), REG_BP,
            n->lookup.scope->vars[n->lookup.var_idx].addr, reg_wsz(n->flurw.reg, n->flurw.size));
      return n->next;

   case IR_FLURD:
      emit_read(n->flurw.reg, n->flurw.size, n->flurw.sign_extend, "%s - %zu", REG_BP,
            n->lookup.scope->vars[n->lookup.var_idx].addr);
      return n->next;

   default:
      panic("unimplemented ir_node type '%s'", ir_node_type_str[n->type]);
   }
}
