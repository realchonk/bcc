#include <unistd.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
   const char* output_file = NULL;
   int option;
   while ((option = getopt(argc, argv, ":o:")) != -1) {
      switch (option) {
      case 'o': output_file = optarg; break;
      default: goto print_usage;
      }
   }
   if ((argc - optind) != 1) {
   print_usage:
      fputs("Usage: bcc [-o output] input\n", stderr);
      return 1;
   }
   
   


}
