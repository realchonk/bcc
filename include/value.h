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

enum attribute {
   ATTR_EXTERN    = 0x0001,
   ATTR_STATIC    = 0x0002,
   ATTR_NORETURN  = 0x0004,
   ATTR_INLINE    = 0x0008,
};

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
enum integer_size {
   INT_BYTE,
   INT_CHAR,
   INT_SHORT,
   INT_INT,
   INT_LONG,

   NUM_INTS,
};
#if ENABLE_FP 
enum fp_size {
   FP_FLOAT,
   FP_DOUBLE,

   NUM_FPS,
};
#endif

struct enum_entry {
   istr_t name;
   intmax_t value;
};
struct struct_entry {
   struct value_type* type;
   istr_t name;
};

struct enumeration {
   istr_t name;                  // optional
   struct enum_entry* entries;
   bool is_definition;
};
struct structure {
   istr_t name;
   struct struct_entry* entries;
   bool is_definition;
};

struct value_type {
   enum value_base_type type;
   struct source_pos begin, end;
   bool is_const;
   bool is_volatile;
   union {
      struct {
         enum integer_size size;
         bool is_unsigned;
      } integer;
#if ENABLE_FP
      struct {
         enum fp_size size;
      } fp;
#endif
      struct {
         struct value_type* type;
         bool is_array;
         bool is_restrict;
         struct {
            bool has_const_size;
            union {
               size_t size;              // if  has_const_size
               struct expression* dsize; // if !has_const_size;
            };
         } array;
      } pointer;
      struct {
         istr_t name; // optional
         struct value_type* ret_val;
         struct value_type** params;
         bool variadic;
      } func;
      struct enumeration* venum;
      struct structure* vstruct;
   };
};

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

struct value_type* parse_value_type(struct scope*);
void free_value_type(struct value_type*);
void print_value_type(FILE*, const struct value_type*);
const struct value_type* get_value_type(struct scope*, struct expression*);
struct value_type* get_value_type_impl(struct scope*, struct expression*);
struct value_type* common_value_type_free(struct value_type*, struct value_type*, bool warn);
struct value_type* common_value_type(const struct value_type*, const struct value_type*, bool warn);
struct value_type* copy_value_type(const struct value_type*);
struct value_type* make_array_vt(struct value_type*);
bool try_eval_expr(struct expression*, struct value*);
size_t sizeof_value(const struct value_type*, bool decay);
struct value_type* decay(struct value_type*);
bool var_is_declared(istr_t, struct scope*);
bool is_castable(const struct value_type* old, const struct value_type* type, bool implicit);
struct enumeration* copy_enum(const struct enumeration*);
struct value_type* make_int(enum integer_size sz, bool is_unsigned);
bool value_type_equal(const struct value_type*, const struct value_type*);
struct structure* copy_struct(const struct structure*);
struct struct_entry* struct_get_member(struct structure*, istr_t);
size_t addrof_member(struct structure*, size_t);
struct structure* real_struct(struct structure*, bool is_union);
size_t struct_get_member_idx(struct structure*, istr_t);
struct value_type* func2vt(const struct function*);
bool is_func_vt(const struct value_type*);
const struct value_type* actual_func_vt(const struct value_type*);
void eval_expr(struct expression*, struct value*);
bool vt_is_signed(const struct value_type*);
const char* attr_to_string(enum attribute);
bool try_eval_array(struct expression*, struct value*, const struct value_type*);

#define is_struct(t) ((t) == VAL_STRUCT || (t) == VAL_UNION)
#define vt_is_struct(vt) (is_struct((vt)->type))
#define vt_is_array(vt) (((vt)->type == VAL_POINTER) && ((vt)->pointer.is_array))
#define vt_is_restrict(vt) (((vt)->type == VAL_POINTER) && ((vt)->pointer.is_restrict))
#define vt_is_volatile(vt) ((vt)->is_volatile)
#define vt_is_string(vt)   (((vt)->type == VAL_POINTER) && ((vt)->pointer.type->type == VAL_INT) \
                           && ((vt)->pointer.type->integer.size = INT_CHAR))

#endif /* FILE_VALUE_H */
