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
   istr_t* undef_macros = NULL;
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
         buf_push(undef_macros, strint(optarg));
         break;
      default:
      print_help:;
         fputs("Usage: bcpp [options] [source [output]]\n", stderr);
         return 1;
      }
   }

   // check the additional arguments
   const int argn = argc - optind;
   if (argn == 0) {
      source_name = "-";
   } else if (argn == 1) {
      source_name = argv[optind];
   } else if (argn == 2) {
      source_name = argv[optind];
      if (output_name) {
         fputs("bcpp: output filename specfied multiple times\n", stderr);
         return 1;
      } else output_name = argv[optind + 1];
   } else {
      fputs("bcpp: too many input files\n", stderr);
      return 1;
   }

   // remove the macros specified by -U
   for (size_t i = 0; i < buf_len(undef_macros); ++i) {
      remove_macro(undef_macros[i]);
   }
   buf_push(cmdline_includes, NULL);

   // initialization stuff
   init_macros();
   init_includes();

   // open the source file
   FILE* source = !strcmp(source_name, "-") ? stdin : fopen(source_name, "r");
   if (!source) {
      fprintf(stderr, "bcpp: failed to open '%s': %s\n", source_name, strerror(errno));
      return 1;
   }

   // open the output file
   FILE* output = !strcmp(output_name, "-") ? stdout : fopen(output_name, "w");
   if (!output) {
      fprintf(stderr, "bcpp: failed to open '%s': %s\n", output_name, strerror(errno));
      fclose(source);
      return 1;
   }

   // run the pre-processor
   const int status = run_cpp(source, output);

   // close the files
   fclose(source);
   fclose(output);
   return status;
}
