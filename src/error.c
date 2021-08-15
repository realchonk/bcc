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

   if (console_colors)
      fputs("\033[31;1m", stderr);
   fprintf(stderr, "bcc: %s(): ", func);
   if (console_colors)
      fputs("\033[0m", stderr);
   vfprintf(stderr, fmt, ap);
   if (errno_saved) fprintf(stderr, ": %s\n", strerror(errno_saved));
   else fputc('\n', stderr);

   va_end(ap);

   abort();
}

noreturn void parse_error(const struct source_pos* pos, const char* fmt, ...) {
   va_list ap;
   va_start(ap, fmt);

   if (console_colors)
      fputs("\033[31m", stderr);
   print_source_pos(stderr, pos);
   fputs(": ", stderr);
   if (console_colors)
      fputs("\033[0m", stderr);
   vfprintf(stderr, fmt, ap);
   fputc('\n', stderr);

   va_end(ap);

   exit(1);
}

void parse_warn(const struct source_pos* pos, const char* fmt, ...) {
   if (!enable_warnings) return;
   va_list ap;
   va_start(ap, fmt);

   if (console_colors)
      fputs("\033[33m", stderr);
   print_source_pos(stderr, pos);
   fputs(": ", stderr);
   if (console_colors)
      fputs("\033[0m", stderr);
   vfprintf(stderr, fmt, ap);
   fputc('\n', stderr);

   va_end(ap);
}

