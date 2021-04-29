#include "target.h"
#include "error.h"

static const char* regs8[] =  {  "al",  "ah",  "cl",  "ch",  "dl",  "dh",  "bl",  "bh" };
static const char* regs16[] = {  "ax",  "cx",  "dx",  "bx",  "si",  "di" };
static const char* regs32[] = { "eax", "ecx", "edx", "ebx", "esi", "edi" };

#define reg8(i) (i < arraylen(regs8) ? regs8[i] : (panic("emit_ir(): register out of range"), NULL))
#define reg16(i) (i < arraylen(regs16) ? regs16[i] : (panic("emit_ir(): register out of range"), NULL))
#define reg32(i) (i < arraylen(regs32) ? regs32[i] : (panic("emit_ir(): register out of range"), NULL))

#define reg_op(dest, src, size) \
   switch (size) { \
   case IRS_BYTE: \
   case IRS_CHAR:    dest = reg8(src); break; \
   case IRS_SHORT:   dest = reg16(src); break; \
   case IRS_PTR: \
   case IRS_INT:     dest = reg32(src); break; \
   default:          panic("emit_ir(): unsupported operand size '%s'", ir_size_str[size]); \
   }

static const char* nasm_size(enum ir_value_size s) {
   switch (s) {
   case IRS_BYTE:
   case IRS_CHAR:    return "byte";
   case IRS_SHORT:   return "word";
   case IRS_INT:
   case IRS_PTR:     return "dword";
   default:          panic("nasm_size(): unsupported operand size '%s'", ir_size_str[s]);
   }
}

ir_node_t* emit_ir(const ir_node_t* n) {
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
      emit("mov %s, %ju", dest, n->load.value);
      return n->next;
   }
   case IR_IADD:
   case IR_ISUB:
   {
      const char* dest;
      const char* a;
      const char* b;
      reg_op(dest, n->binary.dest, n->binary.size);
      reg_op(a, n->binary.a, n->binary.size);
      reg_op(b, n->binary.b, n->binary.size);
      if (n->binary.dest == n->binary.a) {
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
      const char* dest;
      const char* a;
      const char* b;
      reg_op(dest, n->binary.dest, n->binary.size);
      reg_op(a, n->binary.a, n->binary.size);
      reg_op(b, n->binary.b, n->binary.b);

      if (n->binary.dest != n->binary.a) {
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
      const char* eax;
      const char* dest;
      const char* a;
      const char* b;
      reg_op(eax, 0, n->binary.size);
      reg_op(dest, n->binary.dest, n->binary.size);
      reg_op(a, n->binary.a, n->binary.size);
      reg_op(b, n->binary.b, n->binary.size);
      emit("push edx");
      if (n->binary.a != n->binary.dest) {
         emit("mov %s, %s", dest, a);
      }
      if (n->binary.dest != 0) {
         emit("push eax");
         emit("mov %s, %s", eax, dest);
      }

      emit("%s %s", instr, b);

      if (n->binary.dest != 0) {
         emit("mov %s, %s", dest, eax);
         emit("pop eax");
      }
      emit("pop edx");
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
      const char* a;
      const char* b;
      reg_op(eax, 0, n->binary.size);
      reg_op(dest, n->binary.dest, n->binary.size);
      reg_op(a, n->binary.a, n->binary.size);
      reg_op(b, n->binary.b, n->binary.size);
      emit("push edx");
      emit("xor edx, edx");

      if (n->binary.dest != n->binary.a) {
         emit("mov %s, %s", dest, a);
      }
      if (n->binary.dest != 0) {
         emit("push eax");
         emit("mov %s, %s", eax, dest);
      }

      emit("%s %s, %s", instr, eax, b);
      if (n->binary.dest != 0) {
         emit("mov %s, %s", dest, eax);
         emit("pop eax");
      }
      emit("pop edx");
      return n->next;
   }
   case IR_BEGIN_SCOPE:
      emit("sub esp, %zu", 4 * buf_len(n->scope->vars));
      return n->next;
   case IR_END_SCOPE:
      emit("add esp, %zu", 4 * buf_len(n->scope->vars));
      return n->next;
   case IR_LOOKUP:
   {
      size_t idx = 4 + 4 * n->lookup.var_idx;
      struct scope* scope = n->lookup.scope->parent;
      while (scope) {
         idx += buf_len(scope->vars);
         scope = scope->parent;
      }
      emit("lea %s, [ebp - %zu]", reg32(n->lookup.reg), idx);
      return n->next;
   }
   case IR_READ:
   {
      const char* dest;
      const char* src;
      reg_op(dest, n->move.dest, n->move.size);
      reg_op(src, n->move.src, INT_INT);
      emit("mov %s, %s [%s]", dest, nasm_size(n->move.size), src);
      return n->next;
   }
   case IR_WRITE:
   {
      const char* dest;
      const char* src;
      reg_op(dest, n->move.dest, INT_INT);
      reg_op(src, n->move.src, n->move.size);
      emit("mov %s [%s], %s", nasm_size(n->move.size), dest, src);
      return n->next;
   }
   case IR_PROLOGUE:
      emit("push ebp");
      emit("mov ebp, esp");
      return n->next;
   case IR_EPILOGUE:
      emit(".ret:");
      emit("leave");
      emit("ret");
      return n->next;
   case IR_IRET:
      if (n->next && n->next->type != IR_EPILOGUE)
         emit("jmp .ret");
      return n->next;
   default: panic("emit_ir(): unsupported ir_node type '%s'", ir_node_type_str[n->type]);
   }
}

