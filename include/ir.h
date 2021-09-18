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
   IR_READ,          // .read       | read data from memory
   IR_WRITE,         // .write      | write data to memory
   IR_PROLOGUE,      // .func       | beginning of a function
   IR_EPILOGUE,      // .func       | ending of a function
   IR_IICAST,        // .iicast     | integer-to-integer cast
   IR_IFCALL,        // .call       | function call w/ integer return value
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
   IR_FCALL,         // .call       | function call w/ return-type void
   IR_IRCALL,        // .call       | indirect function call w/ integer return value
   IR_RCALL,         // .call       | indirect function call w/o return value
   IR_FLOOKUP,       // .lstr       | function lookup
   IR_SRET,          // .sret       | return struct/union

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
   IRS_STRUCT,

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
      intmax_t sVal;
   };
};

#define irv_reg(r) ((struct ir_value){ .type = IRT_REG, .reg = (r) })
#define irv_uint(v) ((struct ir_value){ .type = IRT_UINT, .uVal = (v) })

typedef struct ir_node {
   enum ir_node_type type;
   struct ir_node* prev;
   struct ir_node* next;
   const struct function* func;
   union {
      struct scope* scope;
      istr_t str;
      struct {
         ir_reg_t dest, src;
         enum ir_value_size size;
      } move;
      struct {
         ir_reg_t dest, src;
         enum ir_value_size size;
         bool sign_extend;
         bool is_volatile;
      } read;
      struct {
         ir_reg_t dest, src;
         enum ir_value_size size;
         bool is_volatile;
      } write;
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
         ir_reg_t reg;
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
         struct ir_node** params;
         union {
            struct ir_node* addr;   // IR_*RCALL
            const char* name;       // IR_*FCALL
         };
      } call;
      struct {
         ir_reg_t ptr;
         uintmax_t size;
      } sret;
   };
} ir_node_t;


/// IR-generation stuff

ir_node_t* irgen_expr(struct scope*, const struct expression*);
ir_node_t* irgen_stmt(const struct statement*);
ir_node_t* irgen_func(const struct function*);



// Node-manipulation stuff

// appends `b` to the end of `a`
ir_node_t* ir_append(ir_node_t* a, ir_node_t* b);

// inserts `b` after `a`
ir_node_t* ir_insert(ir_node_t* a, ir_node_t* b);

// returns the last node
ir_node_t* ir_end(ir_node_t*);

// returns the total number of nodes
size_t ir_length(const ir_node_t*);

// removes the node `n`
void ir_remove(ir_node_t* n);



/// Printing stuff

void print_ir_node(FILE*, const ir_node_t*);
void print_ir_nodes(FILE*, const ir_node_t*);
void print_ir_value(FILE*, const struct ir_value*);



/// Memory management
ir_node_t* new_node(enum ir_node_type t);
void free_ir_node(ir_node_t*);
void free_ir_nodes(ir_node_t*);



/// Other stuff

// gets the ir_value_size from `vt` if possible, else panic's
enum ir_value_size vt2irs(const struct value_type* vt);

// is `n` non-NULL and of type `t`
bool ir_is(ir_node_t* n, enum ir_node_type t);

// checks if `n` is one of the given types
// Note: must be terminated with NUM_IR_NODES
bool ir_isv(ir_node_t* n, ...); 

// a nonsensical value, like NULL
#define IRR_NONSENSE ((ir_reg_t)-1)

// returns the target-register of `n` if applicable;
// otherwise IRR_NONSENSE
ir_reg_t ir_get_target(const ir_node_t*);

// checks if `r` is used as a source by `n`
bool ir_is_source(const ir_node_t* n, ir_reg_t r);

// checks if `t` is a binary operation
bool ir_is_binary(const enum ir_node_type t);

// checks if `n` uses the the register `r` (source, or target)
bool ir_is_used(const ir_node_t*, ir_reg_t);

// returns the size in bytes of an irs
size_t sizeof_irs(enum ir_value_size);

#endif /* FILE_IR_H */
