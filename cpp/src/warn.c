#include <stdarg.h>
#include <stdio.h>
#include "cpp.h"

void warn(size_t linenum, const char* msg, ...) {
   va_list ap;
   va_start(ap, msg);

   if (console_color) {
      fputs("\033[31;1m", stderr);
   }

   fprintf(stderr, "bcpp: %zu: ", linenum + 1);
   if (console_color) {
      fputs("\033[0m", stderr);
   }
   vfprintf(stderr, msg, ap);
   fputc('\n', stderr);

   va_end(ap);
}
