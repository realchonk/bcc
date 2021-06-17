#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "error.h"
#include "bcc.h"

noreturn void panic_impl(const char* func, const char* fmt, ...) {
   va_list ap;
   va_start(ap, fmt);

   const int errno_saved = errno;

   fprintf(stderr, "bcc: %s(): ", func);
   vfprintf(stderr, fmt, ap);
   if (errno) fprintf(stderr, ": %s\n", strerror(errno_saved));
   else fputc('\n', stderr);

   va_end(ap);

   abort();
}

noreturn void parse_error(const struct source_pos* pos, const char* fmt, ...) {
   va_list ap;
   va_start(ap, fmt);

   print_source_pos(stderr, pos);
   fputs(": ", stderr);
   vfprintf(stderr, fmt, ap);
   fputc('\n', stderr);

   va_end(ap);

   exit(1);
}

void parse_warn(const struct source_pos* pos, const char* fmt, ...) {
   if (!enable_warnings) return;
   va_list ap;
   va_start(ap, fmt);

   print_source_pos(stderr, pos);
   fputs(": ", stderr);
   vfprintf(stderr, fmt, ap);
   fputc('\n', stderr);

   va_end(ap);
}

