#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "parser.h"
#include "lex.h"

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
   
   const char* filename = argv[optind++];
   FILE* file = fopen(filename, "r");
   if (!file) {
      fprintf(stderr, "bcc: failed to access '%s': %s\n", filename, strerror(errno));
      return 1;
   }

   lexer_init(file, filename);

   struct statement* stmt = parse_stmt();
   print_stmt(stdout, stmt);
   
   free_stmt(stmt);

   lexer_free();
   return 0;
}
