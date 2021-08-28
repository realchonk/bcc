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

#include <stdio.h>
#include "linker.h"
#include "config.h"
#include "buf.h"

const char* linker_path = GNU_LD;
struct cmdline_arg* linker_args = NULL;

int run_linker(const char* output_name, const char** objects) {
   if (!output_name)
      output_name = "a.out";
   fputs("bcc: linking is currently not supported!\n\n", stderr);
   printf("output file: %s\nobject files:", output_name);
   for (size_t i = 0; i < buf_len(objects); ++i) {
      printf(" %s", objects[i]);
   }
   putchar('\n');
   printf("arguments:");
   for (size_t i = 0; i < buf_len(linker_args); ++i) {
      const struct cmdline_arg arg = linker_args[i];
      printf(" -%c%s", arg.option, arg.arg ? arg.arg : "");
   }
   putchar('\n');

   return 1;
}
