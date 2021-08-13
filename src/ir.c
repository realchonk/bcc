#include <stdarg.h>
#include "error.h"
#include "ir.h"

const char* ir_size_str[NUM_IR_SIZES] = {
   [IRS_BYTE]  = "byte",
   [IRS_CHAR]  = "char",
   [IRS_SHORT] = "short",
   [IRS_INT]   = "int",
   [IRS_LONG]  = "long",
   [IRS_PTR]   = "ptr",
   [IRS_VOID]  = "void",
};

const char* ir_node_type_str[NUM_IR_NODES] = {
   [IR_NOP]          = "nop",
   [IR_MOVE]         = "move",
   [IR_LOAD]         = "load",
   [IR_IADD]         = "iadd",
   [IR_ISUB]         = "isub",
   [IR_IAND]         = "iand",
   [IR_IOR]          = "ior",
   [IR_IXOR]         = "ixor",
   [IR_ILSL]         = "ilsl",
   [IR_ILSR]         = "ilsr",
   [IR_IASR]         = "iasr",
   [IR_IMUL]         = "imul",
   [IR_IDIV]         = "idiv",
   [IR_IMOD]         = "imod",
   [IR_UMUL]         = "umul",
   [IR_UDIV]         = "udiv",
   [IR_UMOD]         = "umod",
   [IR_INEG]         = "ineg",
   [IR_INOT]         = "inot",
   [IR_BNOT]         = "bnot",
   [IR_RET]          = "ret",
   [IR_IRET]         = "iret",
   [IR_LOOKUP]       = "lookup",
   [IR_BEGIN_SCOPE]  = "begin_scope",
   [IR_END_SCOPE]    = "end_scope",
   [IR_READ]         = "read",
   [IR_WRITE]        = "write",
   [IR_PROLOGUE]     = "enter",
   [IR_EPILOGUE]     = "leave",
   [IR_IICAST]       = "iicast",
   [IR_IFCALL]       = "ifcall",
   [IR_FPARAM]       = "fparam",
   [IR_LSTR]         = "lstr",
   [IR_ISTEQ]        = "isteq",
   [IR_ISTNE]        = "istne",
   [IR_ISTGR]        = "istgt",
   [IR_ISTGE]        = "istge",
   [IR_ISTLT]        = "istlt",
   [IR_ISTLE]        = "istle",
   [IR_USTGR]        = "ustgt",
   [IR_USTGE]        = "ustge",
   [IR_USTLT]        = "ustlt",
   [IR_USTLE]        = "ustle",
   [IR_JMP]          = "jmp",
   [IR_JMPIF]        = "jmpif",
   [IR_JMPIFN]       = "jmpifn",
   [IR_LABEL]        = "",
   [IR_ALLOCA]       = "alloca",
   [IR_COPY]         = "copy",
   [IR_ARRAYLEN]     = "arraylen",
   [IR_GLOOKUP]      = "glookup",
   [IR_FCALL]        = "fcall",
   [IR_IRCALL]       = "ircall",
   [IR_RCALL]        = "rcall",
   [IR_FLOOKUP]      = "flookup",
};
const char* ir_value_type_str[NUM_IR_VALUES] = {
   [IRT_REG]         = "register",
   [IRT_UINT]        = "immediate value",
};

ir_node_t* ir_end(ir_node_t* n) {
   while (n->next) n = n->next;
   return n;
}

ir_node_t* ir_append(ir_node_t* a, ir_node_t* b) {
   if (!a) return b;
   else if (!b) return a;
   ir_node_t* e = ir_end(a);
   e->next = b;
   b->prev = e;
   return a;
}

ir_node_t* ir_insert(ir_node_t* a, ir_node_t* b) {
   ir_node_t* n = a->next;
   a->next = b;
   b->next = n;
   b->prev = a;
   n->prev = b;
   return n;
}

size_t ir_length(const ir_node_t* n) {
   size_t i = 0;
   while (n) {
      ++i;
      n = n->next;
   }
   return i;
}

void print_ir_nodes(FILE* file, const ir_node_t* n) {
   while (n) {
      print_ir_node(file, n);
      n = n->next;
   }
}
void print_ir_node(FILE* file, const ir_node_t* n) {
   fputs(ir_node_type_str[n->type], file);
   switch (n->type) {
   case IR_NOP:
   case IR_BEGIN_SCOPE:
   case IR_END_SCOPE:
   case IR_PROLOGUE:
   case IR_EPILOGUE:
   case IR_RET:
   case NUM_IR_NODES:
      break;
   case IR_MOVE:
      fprintf(file, ".%s R%u, R%u", ir_size_str[n->move.size], n->move.dest, n->move.src);
      break;
   case IR_WRITE:
      fprintf(file, ".%s%s R%u, R%u", ir_size_str[n->write.size],
            n->write.is_volatile ? "v" : "", n->write.dest, n->write.src);
      break;
   case IR_READ:
      fprintf(file, "%c.%s%s R%u, R%u", n->read.sign_extend ? 'u' : 's',
            n->read.is_volatile ? "v" : "", ir_size_str[n->read.size],
            n->read.dest, n->read.src);
      break;
   case IR_LOAD:
      fprintf(file, ".%s R%u, %ju", ir_size_str[n->load.size], n->load.dest, n->load.value);
      break;
   case IR_IADD:
   case IR_ISUB:
   case IR_IAND:
   case IR_IOR:
   case IR_IXOR:
   case IR_ILSL:
   case IR_ILSR:
   case IR_IASR:
   case IR_IMUL:
   case IR_IDIV:
   case IR_IMOD:
   case IR_UMUL:
   case IR_UDIV:
   case IR_UMOD:
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
      fprintf(file, ".%s R%u, ", ir_size_str[n->binary.size], n->binary.dest);
      print_ir_value(file, &n->binary.a);
      fputs(", ", file);
      print_ir_value(file, &n->binary.b);
      break;
   case IR_INEG:
   case IR_INOT:
   case IR_BNOT:
   case IR_IRET:
      fprintf(file, ".%s R%u", ir_size_str[n->unary.size], n->unary.reg);
      break;
   case IR_LOOKUP:
   case IR_ARRAYLEN:
      fprintf(file, " R%u, %s", n->lookup.reg, n->lookup.scope->vars[n->lookup.var_idx].name);
      break;
   case IR_IICAST:
      fprintf(file, ".%s R%u, %s R%u", ir_size_str[n->iicast.ds], n->iicast.dest, ir_size_str[n->iicast.ss], n->iicast.src);
      break;
   case IR_IFCALL:
      fprintf(file, " %s, R%u", n->ifcall.name, n->ifcall.dest);
      /*for (size_t i = 0; i < buf_len(n->ifcall.params); ++i)
         fprintf(file, ", R%u", n->ifcall.params[i]);
      */
      break;
   case IR_FCALL:
      fprintf(file, "%s", n->ifcall.name);
      break;

   case IR_FPARAM:
      fprintf(file, " R%u, %s", n->fparam.reg, n->func->params[n->fparam.idx].name);
      break;
   case IR_LSTR:
      fprintf(file, " R%u, '%s'", n->lstr.reg, n->lstr.str);
      break;
   case IR_GLOOKUP:
   case IR_FLOOKUP:
      fprintf(file, " R%u, %s", n->lstr.reg, n->lstr.str);
      break;
   case IR_JMP:
      fprintf(file, " %s", n->str);
      break;
   case IR_JMPIF:
   case IR_JMPIFN:
      fprintf(file, ".%s %s, R%u", ir_size_str[n->cjmp.size], n->cjmp.label, n->cjmp.reg);
      break;
   case IR_LABEL:
      fprintf(file, "%s:", n->str);
      break;
   case IR_ALLOCA:
      fprintf(file, " R%u, ", n->alloca.dest);
      print_ir_value(file, &n->alloca.size);
      break;
   case IR_COPY:
      fprintf(file, " R%u, R%u, %ju", n->copy.dest, n->copy.src, n->copy.len);
      break;
   case IR_IRCALL:
      fprintf(file, " R%u", n->rcall.dest);
      break;
   case IR_RCALL:
      break;
   }
   fputc('\n', file);
}
void print_ir_value(FILE* file, const struct ir_value* v) {
   switch (v->type) {
   case IRT_REG:
      fprintf(file, "R%u", v->reg);
      break;
   case IRT_UINT:
      fprintf(file, "%ju", v->uVal);
      break;
   default: panic("invalid IR value type '%d'", v->type);
   }
}
void free_ir_node(ir_node_t* n) {
   if (n->type == IR_IFCALL || n->type == IR_FCALL) {
      for (size_t i = 0; i < buf_len(n->ifcall.params); ++i)
         free_ir_nodes(n->ifcall.params[i]);
      buf_free(n->ifcall.params);
   } else if (n->type == IR_IRCALL || n->type == IR_RCALL) {
      for (size_t i = 0; i < buf_len(n->rcall.params); ++i)
         free_ir_nodes(n->rcall.params[i]);
      free_ir_nodes(n->rcall.addr);
   }
   free(n);
}
void free_ir_nodes(ir_node_t* n) {
   while (n) {
      ir_node_t* next = n->next;
      free_ir_node(n);
      n = next;
   }
}
void ir_remove(ir_node_t* n) {
   if (n->prev) n->prev->next = n->next;
   if (n->next) n->next->prev = n->prev;
   free_ir_node(n);
}
bool ir_is(ir_node_t* n, enum ir_node_type t) {
   return n && n->type == t;
}
bool ir_isv(ir_node_t* n, ...) {
   if (!n) return false;
   bool success = false;
   va_list ap;
   va_start(ap, n);

   while (1) {
      const enum ir_node_type t = va_arg(ap, enum ir_node_type);
      if (t == NUM_IR_NODES) break;
      else if (t == n->type) {
         success = true;
         break;
      }
   }

   va_end(ap);
   return success;
}

ir_reg_t get_target(const ir_node_t* n) {
   if (ir_is_binary(n->type))
      return n->binary.dest;

   switch (n->type) {
   case IR_MOVE:
   case IR_READ:
      return n->move.dest;
   case IR_LOAD:
      return n->load.dest;
   case IR_IICAST:
      return n->iicast.dest;
   case IR_IFCALL:
      return n->ifcall.dest;
   case IR_IRCALL:
      return n->rcall.dest;
   case IR_LSTR:
   case IR_FLOOKUP:
      return n->lstr.reg;
   case IR_LOOKUP:
   case IR_ARRAYLEN:
      return n->lookup.reg;
   case IR_FPARAM:
      return n->fparam.reg;
   default:
      return IRR_NONSENSE;
   }
}
bool ir_is_source(const ir_node_t* n, ir_reg_t r) {
   if (ir_is_binary(n->type))
      return (n->binary.a.type == IRT_REG && n->binary.a.reg == r)
         ||  (n->binary.b.type == IRT_REG && n->binary.b.reg == r);
   switch (n->type) {
   case IR_READ:
   case IR_MOVE:  return n->move.src == r;
   case IR_WRITE: return n->move.dest == r;
   case IR_INEG:
   case IR_INOT:
   case IR_BNOT:
   case IR_IRET:
      return n->unary.reg == r;
   case IR_IICAST:
      return n->iicast.src == r;
   case IR_ALLOCA:
      return n->alloca.size.type == IRT_REG && n->alloca.size.reg == r;
   case IR_COPY:
      return n->copy.src == r || n->copy.dest == r;
   default:
      return false;
   }
}
bool ir_is_binary(const enum ir_node_type t) {
   switch (t) {
   case IR_IADD:
   case IR_ISUB:
   case IR_IAND:
   case IR_IOR:
   case IR_IXOR:
   case IR_ILSL:
   case IR_ILSR:
   case IR_IASR:
   case IR_IMUL:
   case IR_IDIV:
   case IR_IMOD:
   case IR_UMUL:
   case IR_UDIV:
   case IR_UMOD:
   case IR_INEG:
   case IR_INOT:
   case IR_BNOT:
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
      return true;
   default:
      return false;
   }
}
bool ir_is_used(const ir_node_t* n, ir_reg_t r) {
   while (n) {
      if (ir_is_source(n, r))
         return true;
      else if (get_target(n) == r)
         return false;
      n = n->next;
   }
   return false;
}
