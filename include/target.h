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
#include "cmdline.h"
#include "value.h"
#include "error.h"
#include "ir.h"

struct target_info {
   const char* const name;                      // name of the target architecture (eg. i386)
   const size_t size_byte;                      // size of a byte and _Bool
   const size_t size_char;                      // size of a char (typicalls same as size_byte)
   const size_t size_short;                     // size of a short
   const size_t size_int;                       // size of an int
   const size_t size_long;                      // size of a long
   const size_t size_float;                     // size of a float
   const size_t size_double;                    // size of a double
   const size_t size_pointer;                   // size of a pointer

   const intmax_t min_byte;                     // the   signed minimum value of a byte (typically -128)
   const intmax_t max_byte;                     // the   signed maximum value of a byte (typically  127)
   const uintmax_t max_ubyte;                   // the unsigned maximum value of a byte (typicalls  255)

   const intmax_t min_char;                     // the   signed minimum value of a char
   const intmax_t max_char;                     // the   signed maximum value of a char
   const uintmax_t max_uchar;                   // the unsigned maximum value of a char

   const intmax_t min_short;                    // the   signed minimum value of a short
   const intmax_t max_short;                    // the   signed maximum value of a short
   const uintmax_t max_ushort;                  // the unsigned maximum value of a short

   const intmax_t min_int;                      // the   signed minimum value of a int
   const intmax_t max_int;                      // the   signed maximum value of a int
   const uintmax_t max_uint;                    // the unsigned maximum value of a int

   const intmax_t min_long;                     // the   signed minimum value of a long
   const intmax_t max_long;                     // the   signed maximum value of a long
   const uintmax_t max_ulong;                   // the unsigned maximum value of a long
   
   const bool unsigned_char;                    // is the char datatype unsigned

                                                // file extension of ...
   const char** const fend_asm;                 //    an assembly file
   const char** const fend_obj;                 //    an object file
   const char** const fend_archive;             //    an archive
   const char** const fend_dll;                 //    a  dynamically-linked library

   const bool has_c99_array;                    // does the target support C99's variable length arrays?

   const enum integer_size ptrdiff_type;        // the underlying integer type of __builtin_ptrdiff_t
   const enum integer_size size_type;           // the underlying integer type of __builtin_size_t
   enum integer_size size_int8;                 // the underlying integer type of __builtin_[u]int8_t
   enum integer_size size_int16;                // the underlying integer type of __builtin_[u]int16_t
   enum integer_size size_int32;                // the underlying integer type of __builtin_[u]int32_t
   enum integer_size size_int64;                // the underlying integer type of __builtin_[u]int64_t 
                                                //     (or NUM_INTS, if not available)

   intmax_t max_immed;                          // the signed maximum value of an immediate value
   intmax_t min_immed;                          // the signed minimum value of an immediate value
   intmax_t max_iload;                          // the signed maximum value for IR_LOAD
   intmax_t min_iload;                          // the signed minimum value for IR_LOAD
};

struct builtin_func {
   const char* name;
   const char* code;
   bool requested;
};

extern unsigned asm_indent;
struct function;



/// General

// initializes the assembly-generation
void emit_init(FILE*);

// initializes the target
void target_init(void);

// cleans-up the assembly-generation
void emit_free(void);

// like printf, but also prints a newline
void emit(const char*, ...) PRINTF_FMT_WARN(1, 2);

// line emit(), but doesn't print a newline
void emitraw(const char*, ...) PRINTF_FMT_WARN(1, 2);

// defines compiler-specific macros
void define_ctarget_macros(void);



/// builtin functions

// checks if `func` is a builtin-function
bool is_builtin_func(const char* func);

// marks a builtin-function as requested
void request_builtin(const char* name);

// unmarks all requested builtin-functions
void reset_buitins(void);

// returns a builtin-function
struct builtin_func* get_builtin_func(const char* name);



/// builtin data types

// returns the value_type representation of a builtin-type `name`
const struct value_type* get_builtin_type(istr_t name);

// adds a builtin-type
void add_builtin_type(const char* name, struct value_type*);



/// Miscellaneous

// get the unsigned maximum value of `irs`
uintmax_t target_get_umax(enum ir_value_size irs);

// returns the size in bytes of `irs`
size_t irs2sz(enum ir_value_size irs);

// emit the contents of the string database
void emit_strdb(void);

// checks if the specified ABI matches the ABI specified by -mabi=
bool is_abi(const char*);

// checks if the given C library is equal to LIBC_NAME
bool is_libc(const char*);


/// global variables that must be defined by the target

// the core-definitions for the target architecture
extern const struct target_info target_info;

// the available machine options
extern struct flag_option mach_opts[];

// should be defined as `arraylen(mach_opts)`
extern const size_t num_mach_opts;

// the implemented compiler-specific builtin functions
extern struct builtin_func builtin_funcs[];

// should be defined as `arraylen(builtin_funcs)`
extern const size_t num_builtin_funcs;



/// functions that must be implemented by the target


// emit the assembly for the current unit
void emit_unit(void);

// prepare the target architecture (eg. machine-option parsing)
bool emit_prepare(void);

// return the ABI passed to the linker
char* get_ld_abi(void);

// assemble an assembly file
int assemble(const char* source, const char* output);

// return the path to the dynamic linker (eg. /lib/ld-linux.so.2)
char* get_interpreter(void);

#endif /* FILE_TARGET_H */
