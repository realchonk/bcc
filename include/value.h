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

#ifndef FILE_VALUE_H
#define FILE_VALUE_H
#include <stdbool.h>
#include <stdio.h>
#include "token.h"

struct value_type;
struct function;

// attributes for variables and functions
enum attribute {
   ATTR_EXTERN    = 0x0001,
   ATTR_STATIC    = 0x0002,
   ATTR_NORETURN  = 0x0004,
   ATTR_INLINE    = 0x0008,
};

// the basic value types
enum value_base_type {
   VAL_INT,
#if ENABLE_FP
   VAL_FLOAT,
#endif
   VAL_POINTER,
   VAL_VOID,
   VAL_AUTO,
   VAL_ENUM,
   VAL_STRUCT,
   VAL_UNION,
   VAL_FUNC,
   VAL_BOOL,

   NUM_VALS,
};

// the specific integer sizes
enum integer_size {
   INT_BYTE,
   INT_CHAR,
   INT_SHORT,
   INT_INT,
   INT_LONG,

   NUM_INTS,
};

#if ENABLE_FP 
// the specific floating-point sizes
enum fp_size {
   FP_FLOAT,
   FP_DOUBLE,

   NUM_FPS,
};
#endif

// enum values
struct enum_entry {
   istr_t name;
   intmax_t value;
};

// struct & union members
struct struct_entry {
   struct value_type* type;
   istr_t name;
};

// representation of an enum
struct enumeration {
   istr_t name;                  // optional
   struct enum_entry* entries;
   bool is_definition;
};

// representation of a struct or union
struct structure {
   istr_t name;
   struct struct_entry* entries;
   bool is_definition;
};

// representation of a data type
struct value_type {
   enum value_base_type type;
   struct source_pos begin, end;
   bool is_const;
   bool is_volatile;
   union {

      // integer-specific properties
      struct {
         enum integer_size size;
         bool is_unsigned;
      } integer;


#if ENABLE_FP
      // floating-point-specific properties
      struct {
         enum fp_size size;
      } fp;
#endif

      // properties of arrays and pointers
      struct {
         struct value_type* type;
         bool is_array;
         bool is_restrict;

         // array-specific properties
         struct {
            bool has_const_size;
            union {
               size_t size;              // if  has_const_size
               struct expression* dsize; // if !has_const_size;
            };
         } array;
      } pointer;

      // function-specific properties
      struct {
         istr_t name; // optional
         struct value_type* ret_val;
         struct value_type** params;
         bool variadic;
      } func;

      // enum-specific properties
      struct enumeration* venum;

      // struct-specific properties
      struct structure* vstruct;
   };
};

// representation of a compile-time known value
struct value {
   struct source_pos begin, end;
   struct value_type* type;
   union {
      istr_t sVal;
      intmax_t iVal;
      uintmax_t uVal;
#if ENABLE_FP
      fpmax_t fVal;
#endif
      struct value* array;
   };
};

extern const char* integer_size_str[NUM_INTS];
#if ENABLE_FP
extern const char* fp_size_str[NUM_FPS];
#endif
extern const char* value_type_str[NUM_VALS];
struct expression;
struct scope;

//// functions

/// parsing stuff

// parse a value_type
struct value_type* parse_value_type(struct scope*);


/// miscellaneous stuff

// free the memory of a value_type
void free_value_type(struct value_type*);

// dump the contents of a value_type
void print_value_type(FILE*, const struct value_type*);

// create a copy of a value_type
struct value_type* copy_value_type(const struct value_type*);

// turn a single attribute into a string
const char* attr_to_string(enum attribute);


/// creators/constructors

// create an value_type of array with the base-type of `vt`
struct value_type* make_array_vt(struct value_type* vt);

// create a pointer to `base`
struct value_type* vt_pointer_from(struct value_type* base);

// constructs a value_type of type integer from the argument `sz` and `is_unsigned`
struct value_type* make_int(enum integer_size sz, bool is_unsigned);



/// conversions

// if `vt` is an array, turn `vt` into a pointer;
// or if `vt` is an enum, turn `vt` into an int
// Note: this does not copy
struct value_type* decay(struct value_type* vt);

// if vt is a function, return vt;
// if vt is a pointer to a function, return the function;
// othwerise panic
const struct value_type* actual_func_vt(const struct value_type* vt);

// converts the representation of a function to a value_type
struct value_type* func2vt(const struct function*);


/// copying

// creates a copy of a struct
struct structure* copy_struct(const struct structure*);

// creates a copy of an enum
struct enumeration* copy_enum(const struct enumeration*);

// creates a copy of a value
void copy_value(struct value*, const struct value*);

/// getters

// get the value_type from an expression
const struct value_type* get_value_type(struct scope*, struct expression*);

// actual implementation of get_value_type()
struct value_type* get_value_type_impl(struct scope*, struct expression*);

// call decay() if (`decay` == true); then determine the size of a value_type
size_t sizeof_value(const struct value_type* vt, bool decay);

// gets the address of the `n`th member in a struct `s`
size_t addrof_member(struct structure* s, size_t n);

// returns an existing member of a struct `s` with the name `n`;
// otherwise returns NULL
struct struct_entry* struct_get_member(struct structure* s, istr_t n);

// returns the definition of `s`
struct structure* real_struct(struct structure* s, bool is_union);

// returns the index of a member with the name `n` in a struct or union `s`
size_t struct_get_member_idx(struct structure* s, istr_t n);



/// checkers

// if (`implicit` == true) check if `old` is implicitly convertible to `type`;
// otherwise check if `old` is explicitly convertible to `type`
bool is_castable(const struct value_type* old, const struct value_type* type, bool implicit);

// checks if a value_type is a function
// TODO: turn this into a macro
bool is_func_vt(const struct value_type*);

// check if vt is an integer and signed
// TODO: turn this into a macro
bool vt_is_signed(const struct value_type* vt);

// check if a variable named `name` is declared within `scope`
// TODO: move this to include/scope.h
bool var_is_declared(istr_t name, struct scope* scope);

// checks if two value_type's are equal (internally implemented as ptreq())
bool value_type_equal(const struct value_type*, const struct value_type*);



/// compile-time evaluation

// try to evaluate an expression at compile-time
bool try_eval_expr(struct expression*, struct value*, struct scope*);

// evaluate an expression at compile, or error
void eval_expr(struct expression*, struct value*, struct scope*);

// try to evaluate an array initialization at compile-time
bool try_eval_array(struct expression*, struct value*, const struct value_type*, struct scope*);

/// uncategorized


// TODO: find better solution
// DEPRECATED: get the common value_type of two expressions
struct value_type* common_value_type(const struct value_type*, const struct value_type*, bool warn);

// same as common_value_type(), but calls free_value_type() on the inputs
struct value_type* common_value_type_free(struct value_type*, struct value_type*, bool warn);


/// macros

// checks if `t` is a struct or union (t => enum value_type)
// TODO: remove this
#define is_struct(t) ((t) == VAL_STRUCT || (t) == VAL_UNION)

// checks if `vt` is a struct or union (vt => struct value_type)
#define vt_is_struct(vt) (is_struct((vt)->type))

// checks if `vt` is an array
#define vt_is_array(vt) (((vt)->type == VAL_POINTER) && ((vt)->pointer.is_array))

// checks if `vt` is an array that has compile-time defined size
#define vt_is_const_array(vt) (vt_is_array(vt) && ((vt)->pointer.array.has_const_size))

// checks if `vt` is an array that has runtime defined size
#define vt_is_vla(vt) (vt_is_array(vt) && (!(vt)->pointer.array.has_const_size) && (vt)->pointer.array.dsize)

// checks if `vt` has no size
#define vt_is_unsized_array(vt) (vt_is_array(vt) \
      && (!(vt)->pointer.array.has_const_size) \
      && !(vt)->pointer.array.dsize)

// checks if `vt` is a pointer and is restrict
#define vt_is_restrict(vt) (((vt)->type == VAL_POINTER) && ((vt)->pointer.is_restrict))

// returns the is_volatile value of `vt`
#define vt_is_volatile(vt) ((vt)->is_volatile)

// checks if `vt` is a string (pointer to char)
#define vt_is_string(vt)   (((vt)->type == VAL_POINTER) && ((vt)->pointer.type->type == VAL_INT) \
                           && ((vt)->pointer.type->integer.size = INT_CHAR))

// checks if `vt` is a pointer or an integer and unsigned
#define vt_is_unsigned(vt) (((vt)->type == VAL_POINTER) \
      || (((vt)->type == VAL_INT) && ((vt)->integer.is_unsigned)))

#endif /* FILE_VALUE_H */
