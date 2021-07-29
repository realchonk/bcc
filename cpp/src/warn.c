#include <stdarg.h>
#include <stdio.h>
#include "cpp.h"

bool failed = false;

static void do_warn(size_t linenum, const char* msg, va_list ap) {
   fflush(stdout);

   if (console_color) {
      fputs("\033[31;1m", stderr);
   }

   fprintf(stderr, "bcpp: %s:%zu: ", source_name ? source_name : "<source>", linenum + 1);
   if (console_color) {
      fputs("\033[0m", stderr);
   }
   vfprintf(stderr, msg, ap);
   fputc('\n', stderr);
}

void warn(size_t linenum, const char* msg, ...) {
   va_list ap;
   va_start(ap, msg);

   do_warn(linenum, msg, ap);

   va_end(ap);
}

void fail(size_t linenum, const char* msg, ...) {
   va_list ap;
   va_start(ap, msg);

   do_warn(linenum, msg, ap);
   
   va_end(ap);

   failed = true;
}
