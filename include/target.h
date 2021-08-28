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

#ifndef FILE_TARGET_H
#define FILE_TARGET_H
#include <stdio.h>
#include "value.h"
#include "ir.h"

struct target_info {
   const char* const name;
   const size_t size_byte;
   const size_t size_char;
   const size_t size_short;
   const size_t size_int;
   const size_t size_long;
   const size_t size_float;
   const size_t size_double;
   const size_t size_pointer;

   const intmax_t min_byte;
   const intmax_t max_byte;
   const uintmax_t max_ubyte;

   const intmax_t min_char;
   const intmax_t max_char;
   const uintmax_t max_uchar;

   const intmax_t min_short;
   const intmax_t max_short;
   const uintmax_t max_ushort;

   const intmax_t min_int;
   const intmax_t max_int;
   const uintmax_t max_uint;

   const intmax_t min_long;
   const intmax_t max_long;
   const uintmax_t max_ulong;
   
   const bool unsigned_char;

                                    // file extension of ...
   const char* const fend_asm;      // assembly file
   const char* const fend_obj;      // object file
   const char* const fend_archive;  // archive
   const char* const fend_dll;      // dynamically-linked library

   const enum integer_size ptrdiff_type;
   const enum integer_size size_type;
   const bool has_c99_array; // variable-length array support?

   enum integer_size size_int8;
   enum integer_size size_int16;
   enum integer_size size_int32;
   enum integer_size size_int64;

   intmax_t max_immed;
   intmax_t min_immed;
};

struct builtin_func {
   const char* name;
   const char* code;
   bool requested;
};

struct machine_option {
   const char* name;
   const char* description;
   int type; // 0=bool, 1=int, 2=string
   union {
      bool bVal;
      int iVal;
      const char* sVal;
   };
};

extern const struct target_info target_info;
extern unsigned asm_indent;
extern struct builtin_func builtin_funcs[];
extern const size_t num_builtin_funcs;
extern struct machine_option mach_opts[];
extern const size_t num_mach_opts;
struct function;

void emit_init(FILE*);
void emit_free(void);
void emit(const char*, ...);
void emitraw(const char*, ...);
void emit_unit(void);
int assemble(const char* source, const char* output);
uintmax_t target_get_umax(enum ir_value_size);
bool is_builtin_func(const char*);
void reset_buitins(void);
size_t irs2sz(enum ir_value_size);
void request_builtin(const char* name);
const struct value_type* get_builtin_type(istr_t name);
void add_builtin_type(const char* name, struct value_type*);
struct builtin_func* get_builtin_func(const char* name);
bool emit_prepare(void);
const struct machine_option* get_mach_opt(const char* name);
void define_ctarget_macros(void);
void target_init(void);

#endif /* FILE_TARGET_H */
