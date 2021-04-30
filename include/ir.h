#ifndef FILE_IR_H
#define FILE_IR_H
#include <stdint.h>
#include <stdio.h>
#include "value.h"
#include "expr.h"
#include "stmt.h"
#include "func.h"

enum ir_node_type {
   IR_NOP,
   IR_MOVE,
   IR_LOAD,
   IR_IADD,
   IR_ISUB,
   IR_IAND,
   IR_IOR,
   IR_IXOR,
   IR_ILSL,
   IR_ILSR,
   IR_IASR,
   IR_IMUL,
   IR_IDIV,
   IR_IMOD,
   IR_UMUL,
   IR_UDIV,
   IR_UMOD,
   IR_INEG,
   IR_INOT,
   IR_BNOT,
   IR_RET,
   IR_IRET,
   IR_LOOKUP,
   IR_BEGIN_SCOPE,
   IR_END_SCOPE,
   IR_READ,
   IR_WRITE,
   IR_PROLOGUE,
   IR_EPILOGUE,
   IR_IICAST,
   IR_IFCALL,
   IR_FPARAM,

   NUM_IR_NODES,
};
enum ir_value_size {
   IRS_BYTE,
   IRS_CHAR,
   IRS_SHORT,
   IRS_INT,
   IRS_LONG,
   IRS_PTR,

   NUM_IR_SIZES,
};
extern const char* ir_size_str[NUM_IR_SIZES];
extern const char* ir_node_type_str[NUM_IR_NODES];
typedef uint16_t ir_reg_t;

typedef struct ir_node {
   enum ir_node_type type;
   struct ir_node* prev;
   struct ir_node* next;
   union {
      const struct function* func;
      struct scope* scope;
      struct {
         ir_reg_t dest, src;
         enum ir_value_size size;
      } move;
      struct {
         ir_reg_t dest;
         uintmax_t value;
         enum ir_value_size size;
      } load;
      struct {
         ir_reg_t dest, a, b;
         enum ir_value_size size;
      } binary;
      struct {
         ir_reg_t reg;
         enum ir_value_size size;
      } unary;
      struct {
         ir_reg_t reg;
         struct scope* scope;
         size_t var_idx;
      } lookup;
      struct {
         ir_reg_t dest, src;
         enum ir_value_size ds, ss;
      } iicast;
      struct {
         istr_t name;
         ir_reg_t dest;
         struct ir_node** params;
      } ifcall;
      struct {
         ir_reg_t reg;
         struct function* func;
         size_t idx;
      } fparam;
   };
} ir_node_t;

ir_node_t* irgen_expr(struct scope*, const struct expression*);
ir_node_t* irgen_stmt(const struct statement*);
ir_node_t* irgen_func(const struct function*);

ir_node_t* ir_append(ir_node_t*, ir_node_t*);
ir_node_t* ir_insert(ir_node_t*, ir_node_t*);
ir_node_t* ir_end(ir_node_t*);
size_t ir_length(const ir_node_t*);

void print_ir_node(FILE*, const ir_node_t*);
void print_ir_nodes(FILE*, const ir_node_t*);

void free_ir_node(ir_node_t*);
void free_ir_nodes(ir_node_t*);

enum ir_value_size vt2irs(const struct value_type*);

#endif /* FILE_IR_H */
