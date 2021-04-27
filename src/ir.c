#include "ir.h"

const char* ir_size_str[NUM_IR_SIZES] = {
   [IRS_BYTE]  = "byte",
   [IRS_CHAR]  = "char",
   [IRS_SHORT] = "short",
   [IRS_INT]   = "int",
   [IRS_LONG]  = "long",
};
const char* ir_node_type_str[NUM_IR_NODES] = {
   [IR_NOP]    = "nop",
   [IR_MOVE]   = "move",
   [IR_LOAD]   = "load",
   [IR_IADD]   = "iadd",
   [IR_ISUB]   = "isub",
   [IR_IAND]   = "iand",
   [IR_IOR]    = "ior",
   [IR_IXOR]   = "ixor",
   [IR_ILSL]   = "ilsl",
   [IR_ILSR]   = "ilsr",
   [IR_IASR]   = "iasr",
   [IR_IMUL]   = "imul",
   [IR_IDIV]   = "idiv",
   [IR_IMOD]   = "imod",
   [IR_UMUL]   = "umul",
   [IR_UDIV]   = "udiv",
   [IR_UMOD]   = "umod",
   [IR_INEG]   = "ineg",
   [IR_INOT]   = "inot",
   [IR_BNOT]   = "bnot",
   [IR_RETURN] = "return",
};

ir_node_t* ir_end(ir_node_t* n) {
   while (n->next) n = n->next;
   return n;
}

ir_node_t* ir_append(ir_node_t* a, ir_node_t* b) {
   ir_node_t* e = ir_end(a);
   e->next = b;
   b->prev = e;
   return ir_end(b);
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
   case NUM_IR_NODES:
      break;
   case IR_MOVE:
      fprintf(file, ".%s R%u, R%u", ir_size_str[n->move.size], n->move.dest, n->move.src);
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
      fprintf(file, ".%s R%u, R%u, R%u", ir_size_str[n->binary.size], n->binary.dest, n->binary.a, n->binary.b);
      break;
   case IR_INEG:
   case IR_INOT:
   case IR_BNOT:
   case IR_RETURN:
      fprintf(file, ".%s R%u", ir_size_str[n->unary.size], n->unary.reg);
      break;
   }
   fputc('\n', file);
}
void free_ir_node(ir_node_t* n) {
   //switch (n->type) {}
   free(n);
}
void free_ir_nodes(ir_node_t* n) {
   while (n) {
      ir_node_t* next = n->next;
      free_ir_node(n);
      n = next;
   }
}
