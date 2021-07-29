#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "help_options.h"
#include "macro.h"
#include "cpp.h"
#include "buf.h"

extern const char** cmdline_includes;
bool console_color = true;

int main(int argc, char* argv[]) {
   const char* output_name = "-";
   int option;
   while ((option = getopt(argc, argv, ":D:VEo:I:Ch")) != -1) {
      switch (option) {
      case 'h':
         printf("Usage: bcpp [options] file\nOptions:\n%s", help_options);
         return 0;
      case 'V':
         puts("bcpp " BCC_VER);
         puts("Copyleft Benjamin Stürz.");
         puts("This software is distributed under the terms of the GPLv2");
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
