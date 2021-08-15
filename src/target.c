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

#include <stdarg.h>
#include <string.h>
#include "target.h"
#include "error.h"

#define ASM_INDENT ' '

struct builtin_type {
   istr_t name;
   struct value_type* vt;
};

static struct builtin_type* btypes = NULL;

static FILE* file = NULL;
unsigned asm_indent = 0;

void emit_init(FILE* f) {
   file = f;
   asm_indent = 0;
   add_builtin_type("__builtin_int8_t", make_int(target_info.size_int8,    false));
   add_builtin_type("__builtin_int16_t", make_int(target_info.size_int16,  false));
   add_builtin_type("__builtin_int32_t", make_int(target_info.size_int32,  false));
   if (target_info.size_int64 != NUM_INTS)
      add_builtin_type("__builtin_int64_t", make_int(target_info.size_int64,  false));
   add_builtin_type("__builtin_uint8_t", make_int(target_info.size_int8,   true));
   add_builtin_type("__builtin_uint16_t", make_int(target_info.size_int16, true));
   add_builtin_type("__builtin_uint32_t", make_int(target_info.size_int32, true));
   if (target_info.size_int64 != NUM_INTS)
      add_builtin_type("__builtin_uint64_t", make_int(target_info.size_int64, true));
}

void emit_free(void) {
   if (file) fclose(file);
   file = NULL;
}

void emitraw(const char* fmt, ...) {
   va_list ap;
   va_start(ap, fmt);

   for (unsigned i = 0; i < asm_indent; ++i)
      fputc(ASM_INDENT, file);
   vfprintf(file, fmt, ap);

   va_end(ap);
}
void emit(const char* fmt, ...) {
   va_list ap;
   va_start(ap, fmt);

   for (unsigned i = 0; i < asm_indent; ++i)
      fputc(ASM_INDENT, file);
   vfprintf(file, fmt, ap);
   fputc('\n', file);

   va_end(ap);
}


uintmax_t target_get_umax(enum ir_value_size sz) {
   switch (sz) {
   case IRS_BYTE:    return target_info.max_ubyte;
   case IRS_CHAR:    return target_info.max_uchar;
   case IRS_SHORT:   return target_info.max_ushort;
   case IRS_INT:     return target_info.max_uint;
   case IRS_LONG:    return target_info.max_ulong;
   default:          panic("unreachable reached");
   }
}

bool is_builtin_func(const char* name) {
   for (size_t i = 0; i < num_builtin_funcs; ++i) {
      if (!strcmp(name, builtin_funcs[i].name))
         return true;
   }
   return false;
}
void reset_builtins(void) {
   for (size_t i = 0; i < num_builtin_funcs; ++i) {
      builtin_funcs[i].requested = false;
   }
}
size_t irs2sz(const enum ir_value_size sz) {
   switch (sz) {
   case IRS_BYTE:
      return target_info.size_byte;
   case IRS_CHAR:
      return target_info.size_char;
   case IRS_SHORT:
      return target_info.size_short;
   case IRS_INT:
      return target_info.size_int;
   case IRS_LONG:
      return target_info.size_long;
   case IRS_PTR:
      return target_info.size_pointer;
   default:
      panic("irs2sz(): unreachable reached");
   }
}
void request_builtin(const char* name) {
   for (size_t i = 0; i < num_builtin_funcs; ++i) {
      if (!strcmp(name, builtin_funcs[i].name))
         builtin_funcs[i].requested = true;
   }
}
const struct value_type* get_builtin_type(istr_t name) {
   for (size_t i = 0; i < buf_len(btypes); ++i) {
      if (name == btypes[i].name)
         return btypes[i].vt;
   }
   return NULL;
}
void add_builtin_type(const char* name, struct value_type* vt) {
   struct builtin_type t;
   t.name = strint(name);
   t.vt = vt;
   buf_push(btypes, t);
}
struct builtin_func* get_builtin_func(const char* name) {
   for (size_t i = 0; i < num_builtin_funcs; ++i) {
      if (!strcmp(name, builtin_funcs[i].name))
         return &builtin_funcs[i];
   }
   return NULL;
}
const struct machine_option* get_mach_opt(const char* name) {
   for (size_t i = 0; i < num_mach_opts; ++i) {
      const struct machine_option* opt = &mach_opts[i];
      if (!strcmp(name, opt->name))
         return opt;
   }
   return NULL;
}
