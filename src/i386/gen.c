#include <string.h>
#include <ctype.h>
#include <math.h>
#include "target.h"
#include "error.h"
#include "strdb.h"
#include "regs.h"

static const char* nasm_size(enum ir_value_size s) {
   switch (s) {
   case IRS_BYTE:
   case IRS_CHAR:    return "byte";
   case IRS_SHORT:   return "word";

   case IRS_PTR:
#if BCC_x86_64
   case IRS_LONG:    return "qword";
#endif
   case IRS_INT:     return "dword";
   default:          panic("nasm_size(): unsupported operand size '%s'", ir_size_str[s]);
   }
}

static const struct function* cur_func;
static const struct compilation_unit* cunit;
static uintreg_t esp = 0;
static istr_t* unresolved = NULL;
static istr_t* defined = NULL;

#if BCC_x86_64
static bool is_defined(istr_t s) {
   for (size_t i = 0; i < buf_len(cunit->funcs); ++i) {
      const struct function* f = cunit->funcs[i];
      if (s == f->name && f->scope) return true;
   }
   return false;
}
#endif

static ir_node_t* emit_ir(const ir_node_t* n);
static void emit_begin(void) {
   strdb_init();
   if (unresolved) buf_free(unresolved);
   if (defined) buf_free(defined);
   emit("default rel");
   emit("section .text");
}
static void emit_end(void) {
   for (size_t i = 0; i < buf_len(unresolved); ++i) {
      bool is_defined = false;
      for (size_t j = 0; j < buf_len(defined); ++j) {
         if (unresolved[i] == defined[j]) {
            is_defined = true;
            break;
         }
      }
      if (!is_defined) emit("extern %s", unresolved[i]);
   }

   asm_indent = 0;
   if (strdb) {
      emit("\nsection .rodata\n__strings:");
      size_t i = 0;
      while (i < buf_len(strdb)) {
         if (!strdb[i]) {
            emit("db 0");
            ++i;
            continue;
         }
         emitraw("db ");
         while (strdb[i]) {
            if (isprint(strdb[i])) {
               emitraw("\"");
               while (isprint(strdb[i])) {
                  emitraw("%c", strdb[i++]);
               }
               emitraw("\"");
            } else {
               emitraw("%u", strdb[i++]);
            }
            emitraw(", ");
         }
         emit("0");
         ++i;
      }
   }

   buf_free(unresolved);
   buf_free(defined);
}
static void emit_func(const struct function* func, const ir_node_t* n) {
   cur_func = func;
   esp = 0;
   while ((n = emit_ir(n)) != NULL);
}
void emit_unit(const struct compilation_unit* unit) {
   cunit = unit;
   emit_begin();
   for (size_t i = 0; i < buf_len(unit->funcs); ++i) {
      const struct function* f = unit->funcs[i];
      if (f->ir_code)
         emit_func(f, f->ir_code);
      else emit("extern %s", f->name);
   }
   emit_end();
}

static size_t uslen(uintmax_t v) {
   return v == 0 ? 1 : (size_t)log10(v) + 1;
}
static const char* irv2str(const struct ir_value* v, const enum ir_value_size s) {
   if (v->type == IRT_REG) {
      const char* str;
      reg_op(str, v->reg, s);
      return str;
   } else if (v->type == IRT_UINT) {
      char buffer[uslen(v->uVal) + 1];
      snprintf(buffer, sizeof(buffer), "%ju", v->uVal);
      return strint(buffer);
   } else panic("irv2str(): invalid IR value type '%u'", v->type);
}
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
      if (n->load.value) emit("mov %s, %ju", dest, n->load.value);
      else emit("xor %s, %s", dest, dest);
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
         emit("%s %s, %s", n->type == IR_IADD ? "add" : "sub", dest, b);
      } else {
         emit("lea %s, [%s %c %s]", dest, a, n->type == IR_IADD ? '+' : '-', b);
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
      instr = "asr";
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
      emit("push %s", reg_dx);
      if (n->binary.a.type != IRT_REG || n->binary.dest != n->binary.a.reg) {
         emit("mov %s, %s", dest, a);
      }
      if (n->binary.dest != 0) {
         emit("push %s", reg_ax);
         emit("mov %s, %s", eax, dest);
      }

      emit("%s %s", instr, b);

      if (n->binary.dest != 0) {
         emit("mov %s, %s", dest, eax);
         emit("pop %s", reg_ax);
      }
      emit("pop %s", reg_dx);
      return n->next;
   }
   case IR_IDIV:
      instr = "idiv";
      goto ir_div;
   case IR_UDIV:
      instr = "div";
   {
   ir_div:;
      const char* eax;
      const char* dest;
      const char* a = irv2str(&n->binary.a, n->binary.size);
      const char* b = irv2str(&n->binary.b, n->binary.size);
      reg_op(eax, 0, n->binary.size);
      reg_op(dest, n->binary.dest, n->binary.size);
      emit("push %s", reg_dx);
      emit("xor %s, %s", reg_dx, reg_dx);

      if (n->binary.a.type != IRT_REG || n->binary.dest != n->binary.a.reg) {
         emit("mov %s, %s", dest, a);
      }
      if (n->binary.dest != 0) {
         emit("push %s", reg_ax);
         emit("mov %s, %s", eax, dest);
      }

      emit("%s %s, %s", instr, eax, b);
      if (n->binary.dest != 0) {
         emit("mov %s, %s", dest, eax);
         emit("pop %s", reg_ax);
      }
      emit("pop %s", reg_dx);
      return n->next;
   }
   case IR_BEGIN_SCOPE:
      if (n->scope->vars) emit("sub %s, %zu", reg_sp, REGSIZE * buf_len(n->scope->vars));
      esp += REGSIZE * buf_len(n->scope->vars);
      emit("");
      return n->next;
   case IR_END_SCOPE:
      emit("");
      if (n->scope->vars) emit("add %s, %zu", reg_sp, REGSIZE * buf_len(n->scope->vars));
      esp -= 4 * buf_len(n->scope->vars);
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
      buf_push(defined, n->func->name);
      emit("global %s", n->func->name);
      emit("%s:", n->func->name);
      emit("push %s", reg_bp);
      emit("mov %s, %s", reg_bp, reg_sp);
      esp = REGSIZE * 2;
#if BCC_x86_64
      for (size_t i = 0; i < my_min(buf_len(n->func->params), arraylen(param_regs)); ++i) {
         emit("push %s", mreg(param_regs[i]));
         esp += REGSIZE;
      }
#endif
      return n->next;
   case IR_EPILOGUE:
      if (strcmp(n->func->name, "main") == 0 && n->prev && n->prev->prev && n->prev->prev->type != IR_IRET)
         emit("xor %s, %s", reg_ax, reg_ax);
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
            case IRS_INT:     mask = 0xffffffff; break;
            default:          mask = 0;
            }
            if (mask) emit("and %s, 0x%jx", mreg(n->iicast.dest), (uintmax_t)mask);
         }
      } else if (n->iicast.ds > n->iicast.ss) {
         if (n->iicast.dest != n->iicast.src) {
            const char* tmp_dest;
            const char* dest;
            const char* src;
            reg_op(tmp_dest, n->iicast.dest, n->iicast.ds);
            reg_op(dest, n->iicast.dest, n->iicast.ss);
            reg_op(src, n->iicast.src, n->iicast.ss);
            emit("xor %s, %s", tmp_dest, tmp_dest);
            emit("mov %s, %s", dest, src);
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
      for (size_t i = 0; i < buf_len(cunit->funcs); ++i) {
         if (n->ifcall.name == cunit->funcs[i]->name) {
            variadic = cunit->funcs[i]->variadic;
            break;
         }
      }
      if (variadic) emit("xor rax, rax");
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
      if (n->ifcall.dest != 0) {
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
      size_t idx = REGSIZE * (n->lookup.var_idx + 1);
#if BCC_x86_64
      idx += my_min(arraylen(param_regs), buf_len(cur_func->params)) * REGSIZE;
#endif
      struct scope* scope = n->lookup.scope->parent;
      while (scope) {
         idx += buf_len(scope->vars) * REGSIZE;
         scope = scope->parent;
      }
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
      if (n->binary.size > 1) emit("mov %s, 0", dest); // xor changes eflags
      emit("%s %s", instr, reg8(n->binary.dest));
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
      emit("cmp %s, 0", reg);
      emit("%s %s", instr, n->cjmp.label);
      return n->next;
   }
   case IR_IINC:
      instr = "inc";
      goto iinc;
   case IR_IDEC:
      instr = "dec";
   {
   iinc:;
      const char* reg;
      reg_op(reg, n->unary.reg, n->unary.size);
      emit("%s %s", instr, reg);
      return n->next;
   }

   default: panic("emit_ir(): unsupported ir_node type '%s'", ir_node_type_str[n->type]);
   }
}



