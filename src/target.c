#include <stdarg.h>
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
   default:          panic("target_get_umax(): unreachable reached");
   }
}
