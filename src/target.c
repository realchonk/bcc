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
#include <ctype.h>
#include "target.h"
#include "error.h"
#include "strdb.h"
#include "cpp.h"

#define ASM_INDENT ' '

struct builtin_type {
   istr_t name;
   struct value_type* vt;
};

static struct builtin_type* btypes = NULL;

static FILE* file = NULL;
unsigned asm_indent = 0;

void target_init(void) {
   static bool initialized = false;
   if (initialized)
      return;
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

   add_builtin_type("__builtin_ptrdiff_t", make_int(target_info.ptrdiff_type, false));
   add_builtin_type("__builtin_size_t", make_int(target_info.size_type, true));
   initialized = true;
}

void emit_init(FILE* f) {
   file = f;
   asm_indent = 0;
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
   if (!name) return false;
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
void define_ctarget_macros(void) {
   define_macro2("__bcc_char_signed", target_info.unsigned_char ? "0" : "1");
   define_macro2i("__bcc_min_schar", target_info.min_char);
   define_macro2i("__bcc_max_schar", target_info.max_char);
   define_macro2i("__bcc_max_uchar", target_info.max_uchar);
   define_macro2i("__bcc_min_short", target_info.min_short);
   define_macro2i("__bcc_max_short", target_info.max_short);
   define_macro2i("__bcc_max_ushort", target_info.max_ushort);
   define_macro2i("__bcc_min_int", target_info.min_int);
   define_macro2i("__bcc_max_int", target_info.max_int);
   define_macro2i("__bcc_max_uint", target_info.max_uint);
   define_macro2i("__bcc_min_long", target_info.min_long);
   define_macro2i("__bcc_max_long", target_info.max_long);
   define_macro2i("__bcc_max_ulong", target_info.max_ulong);
}
void emit_strdb(void) {
   if (!strdb)
      return;
   emit(".section .rodata");
   emit("__strings:");
   emitraw(".string \"");
   for (size_t i = 0; i < buf_len(strdb); ++i) {
      const char ch = strdb[i];
      if (isprint(ch)) {
         emitraw("%c", ch);
      } else {
         emitraw("\\%03o", ch);
      }
   }
   emit("\"");
}
