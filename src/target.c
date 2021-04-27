#include <stdarg.h>
#include "target.h"

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

void emit(const char* fmt, ...) {
   va_list ap;
   va_start(ap, fmt);

   for (unsigned i = 0; i < asm_indent; ++i)
      fputc(ASM_INDENT, file);
   vfprintf(file, fmt, ap);
   fputc('\n', file);

   va_end(ap);
}

