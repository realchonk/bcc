#include "ir.h"

const char* ir_node_type_str[NUM_IR_NODES] = {
   [IR_MOVE]   = "move",
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
   case IR_MOVE:
      fprintf(file, " R%u, R%u", n->move.dest, n->move.src);
      break;
   }
   fputc('\n', file);
}
