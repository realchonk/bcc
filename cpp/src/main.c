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

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "help_options.h"
#include "config.h"
#include "macro.h"
#include "cpp.h"
#include "buf.h"

extern const char** cmdline_includes;
bool console_color = true;

int main(int argc, char* argv[]) {
   const char* output_name = "-";
   int option;
   while ((option = getopt(argc, argv, ":D:VEo:I:ChU:")) != -1) {
      switch (option) {
      case 'h':
         printf("Usage: bcpp [options] file\nOptions:\n%s", help_options);
         return 0;
      case 'V':
         puts("bcpp " VERSION);
         puts("Copyleft Benjamin Stürz.");
         puts("This software is distributed under the terms of the GPLv3");
         puts("Compiled on " __DATE__);
         return 0;
      case 'E':
         // skip since this is the pre-processor
         break;
      case ':':
         fprintf(stderr, "bcpp: missing argument for '-%c'\n", optopt);
         return 1;
      case '?':
         fprintf(stderr, "bcpp: invalid option '-%c'\n", optopt);
         return 1;
      case 'o':
         output_name = optarg;
         break;
      case 'I':
         buf_push(cmdline_includes, optarg);
         break;
      case 'C':
         console_color = false;
         break;
      case 'D':
         add_cmdline_macro(optarg);
         break;
      case 'U':
         remove_macro(strint(optarg));
         break;
      default:
         goto print_usage;
      }
   }
   if ((argc - optind) != 1) {
   print_usage:
      fputs("Usage: bcpp [options] input\n", stderr);
      return 1;
   }
   buf_push(cmdline_includes, NULL);

   source_name = argv[optind];
   FILE* source = !strcmp(source_name, "-") ? stdin : fopen(source_name, "r");
   if (!source) {
      fprintf(stderr, "bcpp: failed to open '%s': %s\n", source_name, strerror(errno));
      return 1;
   }

   FILE* output = !strcmp(output_name, "-") ? stdout : fopen(output_name, "w");
   if (!output) {
      fprintf(stderr, "bcpp: failed to open '%s': %s\n", output_name, strerror(errno));
      fclose(source);
      return 1;
   }

   const int status = run_cpp(source, output);

   fclose(source);
   fclose(output);
   return status;
}
