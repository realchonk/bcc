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

// !!! IMPORTANT !!!
// Shared code for targets using the binutils.
// Must not be included by targets that don't use the binutils.

#ifndef FILE_BINUTILS_H
#define FILE_BINUTILS_H
#include <stdbool.h>
#include <stdint.h>
#include "ir.h"

#if !USE_BINUTILS
#error "This target doesn't use binutils."
#endif

struct binutils_info {
   const char* const comment;          // Begin of a one-line comment

   const char* const section_text;     // Name of the code/text section          (typically: .text)
   const char* const section_data;     // Name of the data section               (typically: .data)
   const char* const section_rodata;   // Name of the readonly-data section      (typically: .rodata)
   const char* const section_bss;      // Name of the uninitialized data section (typically: .bss)

   const char* const init_byte;        // Initialization sequence for bytes      (eg.: .byte)
   const char* const init_char;        // Initialization sequence for chars
   const char* const init_short;       // Initialization sequence for shorts
   const char* const init_int;         // Initialization sequence for ints
   const char* const init_long;        // Initialization sequence for longs
   const char* const init_ptr;         // Initialization sequence for pointers
   const char* const init_float;       // Initialization sequence for floats
   const char* const init_double;      // Initialization sequence for doubles
   const char* const init_bool;        // Initialization sequence for booleans
   const char* const init_string;      // Initialization sequence for strings    (typically: .string or .asciz)
   const char* const init_zero;        // Allocate zero-initialized memory       (typically: .zero)

   const bool init_string_has_null;    // Does .init_string append a NUL-byte?
   const char type_prefix;             // '@' for @function
};

// Variables that must be defined by the target
extern const struct binutils_info binutils_info;


// Functions that must be implemented by the target.

// Generate code for an IR node.
ir_node_t* emit_ir(ir_node_t* n);

// Gets called at the begin of the assembly generation.
void emit_begin_hook(void);

// Gets called before the generation of the builtin functions.
void emit_builtin_funcs_hook(void);

// Gets called before the generation of global variables and the string database.
void emit_global_vars_hook(void);

// Gets called at the very end of the assembly generation
void emit_end_hook(void);

// Place `BINUTILS_MACH_OPTS` into `mach_opts`
#define BINUTILS_MACH_OPTS \
{ "clean-asm",    "Generate cleaned assembly",  0, .bVal = false }

#endif /* FILE_BINUTILS_H */
