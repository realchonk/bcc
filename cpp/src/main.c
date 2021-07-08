#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

int main(int argc, char* argv[]) {
   const char* output_name = "-";
   int option;
   while ((option = getopt(argc, argv, ":D:VEo:")) != -1) {
      switch (option) {
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
      default:
         goto print_usage;
      }
   }
   if ((argc - optind) != 1) {
   print_usage:
      fputs("Usage: bcpp [options] input\n", stderr);
      return 1;
   }

   const char* source_name = argv[optind];
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

   // TODO: implement

   fclose(source);
   fclose(output);
   return 0;
}
