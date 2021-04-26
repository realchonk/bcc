#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "error.h"

noreturn void panic(const char* fmt, ...) {
   va_list ap;
   va_start(ap, fmt);

   fputs("bcc: ", stderr);
   vfprintf(stderr, fmt, ap);
   fprintf(stderr, ": %s\n", strerror(errno));
   
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


