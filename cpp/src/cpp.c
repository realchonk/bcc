#include <stdio.h>
#include "cpp.h"

int run_cpp(FILE* in, FILE* out) {
   char** lines = read_lines(in);
   print_lines(out, lines);
   free_lines(lines);
   return 0;
}


