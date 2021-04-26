#ifndef FILE_IR_H
#define FILE_IR_H
#include <stdint.h>
#include <stdio.h>
#include "expr.h"
#include "stmt.h"

enum ir_node_type {
   IR_MOVE,

   NUM_IR_NODES,
};
extern const char* ir_node_type_str[NUM_IR_NODES];
typedef uint16_t ireg_t;

typedef struct ir_node {
   enum ir_node_type type;
   struct ir_node* prev;
   struct ir_node* next;
   union {
      struct {
         ireg_t dest, src;
      } move;
   };
} ir_node_t;

ir_node_t* irgen_expr(const struct expression*);
ir_node_t* irgen_stmt(const struct statement*);

ir_node_t* ir_append(ir_node_t*, ir_node_t*);
ir_node_t* ir_insert(ir_node_t*, ir_node_t*);
ir_node_t* ir_end(ir_node_t*);
size_t ir_length(const ir_node_t*);

void print_ir_node(FILE*, const ir_node_t*);
void print_ir_nodes(FILE*, const ir_node_t*);

#endif /* FILE_IR_H */
