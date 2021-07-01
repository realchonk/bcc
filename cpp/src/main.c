#include <unistd.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
   int option;
   while ((option = getopt(argc, argv, ":D:VE")) != -1) {
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
      default:
         goto print_usage;
      }
   }
   if ((argc - optind) != 1) {
   print_usage:
      fputs("Usage: bcpp [options] input\n", stderr);
      return 1;
   }
   return 0;
}
