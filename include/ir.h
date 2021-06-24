#ifndef FILE_IR_H
#define FILE_IR_H
#include <stdint.h>
#include <stdio.h>
#include "value.h"
#include "expr.h"
#include "stmt.h"
#include "unit.h"

enum ir_node_type {
   IR_NOP,           //             | no operation
   IR_MOVE,          // .move       | copy registers
   IR_LOAD,          // .load       | load constants
   IR_IADD,          // .binary     | integer addition
   IR_ISUB,          // .binary     | integer subtraction
   IR_IAND,          // .binary     | bitwise and
   IR_IOR,           // .binary     | bitwise or
   IR_IXOR,          // .binary     | bitwise xor
   IR_ILSL,          // .binary     | logical shift left
   IR_ILSR,          // .binary     | logical shift right
   IR_IASR,          // .binary     | arithmetic shift right
   IR_IMUL,          // .binary     | signed integer multiplication
   IR_IDIV,          // .binary     | signed integer division
   IR_IMOD,          // .binary     | signed integer modulo
   IR_UMUL,          // .binary     | unsigned integer multiplication
   IR_UDIV,          // .binary     | unsigned integer division
   IR_UMOD,          // .binary     | unsigned integer modulo
   IR_INEG,          // .unary      | integer 2s-complement
   IR_INOT,          // .unary      | integer 1s-complement (integer negation)
   IR_BNOT,          // .unary      | boolean negation      (bitwise negation)
   IR_RET,           //             | return from function 
   IR_IRET,          // .unary      | return from function w/ integer
   IR_LOOKUP,        // .lookup     | load address of variable
   IR_BEGIN_SCOPE,   // .scope      | beginning of a scope
   IR_END_SCOPE,     // .scope      | ending of a scope
   // TODO: change to .read
   IR_READ,          // .move       | read data from memory
   IR_WRITE,         // .move       | write data to memory
   IR_PROLOGUE,      // .func       | beginning of a function
   IR_EPILOGUE,      // .func       | ending of a function
   IR_IICAST,        // .iicast     | integer-to-integer cast
   IR_IFCALL,        // .ifcall     | function call w/ integer return value
   IR_FPARAM,        // .fparam     | load address of function parameter
   IR_LSTR,          // .lstr       | load address of string
   IR_ISTEQ,         // .binary     | set if integer equal
   IR_ISTNE,         // .binary     | set if integer not equal
   IR_ISTGR,         // .binary     | set if signed integer greater than
   IR_ISTGE,         // .binary     | set if signed integer greater than or equal
   IR_ISTLT,         // .binary     | set if signed integer less than
   IR_ISTLE,         // .binary     | set if signed integer less than or equal
   IR_USTGR,         // .binary     | set if unsigned integer greater than
   IR_USTGE,         // .binary     | set if unsigned integer greater than or equal
   IR_USTLT,         // .binary     | set if unsigned integer less than
   IR_USTLE,         // .binary     | set if unsigned integer less than or equal
   IR_JMP,           // .str        | unconditional jump to label
   IR_JMPIF,         // .cjmp       | jump to label if true
   IR_JMPIFN,        // .cjmp       | jump to label if false
   IR_LABEL,         // .str        | define label
   IR_ALLOCA,        // .alloca     | allocate data on the stack
   IR_COPY,          // .copy       | copy array
   IR_ARRAYLEN,      // .lookup     | get length of variable-length array
   IR_GLOOKUP,       // .lstr       | get address of global variable
   IR_FCALL,         // .ifcall     | function call w/ return-type void
   IR_IRCALL,        // .rcall      | indirect function call w/ integer return value
   IR_RCALL,         // .rcall      | indirect function call w/o return value
   IR_FLOOKUP,       // .lstr       | function lookup

   NUM_IR_NODES,
};
enum ir_value_size {
   IRS_BYTE,
   IRS_CHAR,
   IRS_SHORT,
   IRS_INT,
   IRS_LONG,
   IRS_PTR,
   IRS_VOID,

   NUM_IR_SIZES,
};
enum ir_value_type {
   IRT_REG,
   IRT_UINT,

   NUM_IR_VALUES,
};
extern const char* ir_size_str[NUM_IR_SIZES];
extern const char* ir_node_type_str[NUM_IR_NODES];
extern const char* ir_value_type_str[NUM_IR_VALUES];
typedef unsigned ir_reg_t;

struct ir_value {
   enum ir_value_type type;
   union {
      ir_reg_t reg;
      uintmax_t uVal;
   };
};

#define irv_reg(r) ((struct ir_value){ .type = IRT_REG, .reg = (r) })
#define irv_uint(v) ((struct ir_value){ .type = IRT_UINT, .uVal = (v) })

typedef struct ir_node {
   enum ir_node_type type;
   struct ir_node* prev;
   struct ir_node* next;
   union {
      const struct function* func;
      struct scope* scope;
      istr_t str;
      struct {
         ir_reg_t dest, src;
         enum ir_value_size size;
         bool sign_extend; // only for READ, WRITE
      } move;
      struct {
         ir_reg_t dest;
         uintmax_t value;
         enum ir_value_size size;
      } load;
      struct {
         ir_reg_t dest;
         struct ir_value a;
         struct ir_value b;
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
         bool sign_extend;
      } iicast;
      struct {
         istr_t name;
         ir_reg_t dest;             // only used with IR_IFCALL
         struct ir_node** params;
      } ifcall;
      struct {
         ir_reg_t reg;
         struct function* func;
         size_t idx;
      } fparam;
      struct {
         ir_reg_t reg;
         istr_t str;
      } lstr;
      struct {
         istr_t label;
         ir_reg_t reg;
         enum ir_value_size size;
      } cjmp;
      struct {
         ir_reg_t dest;
         struct ir_value size;
         const struct variable* var;
      } alloca;
      struct {
         ir_reg_t dest, src;
         uintmax_t len;
      } copy;
      struct {
         ir_reg_t dest;
         bool variadic;
         struct ir_node* addr;
         struct ir_node** params;
      } rcall;
   };
} ir_node_t;

ir_node_t* irgen_expr(struct scope*, const struct expression*);
ir_node_t* irgen_stmt(const struct statement*);
ir_node_t* irgen_func(const struct function*);

ir_node_t* ir_append(ir_node_t*, ir_node_t*);
ir_node_t* ir_insert(ir_node_t*, ir_node_t*);
ir_node_t* ir_end(ir_node_t*);
size_t ir_length(const ir_node_t*);
void ir_remove(ir_node_t*);

void print_ir_node(FILE*, const ir_node_t*);
void print_ir_nodes(FILE*, const ir_node_t*);
void print_ir_value(FILE*, const struct ir_value*);

void free_ir_node(ir_node_t*);
void free_ir_nodes(ir_node_t*);

enum ir_value_size vt2irs(const struct value_type*);

bool ir_is(ir_node_t*, enum ir_node_type);
bool ir_isv(ir_node_t*, ...); // must be terminated with NUM_IR_NODES

#endif /* FILE_IR_H */
