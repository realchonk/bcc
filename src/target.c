#include <stdarg.h>
#include <string.h>
#include "target.h"
#include "error.h"

#define ASM_INDENT ' '

static FILE* file = NULL;
unsigned asm_indent = 0;

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
