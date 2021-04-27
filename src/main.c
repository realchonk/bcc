#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "parser.h"
#include "target.h"
#include "lex.h"
#include "ir.h"

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

   struct expression* e = parse_expr();

   puts("expr:");
   print_expr(stdout, e);
   
   puts("\nIR:");

   ir_node_t* nodes = irgen_expr(e);

   print_ir_nodes(stdout, nodes);

   puts("\nASM:");

   emit_init(stdout);
   
   ir_node_t* n = nodes;
   while (n) n = emit_ir(n);

   free_ir_nodes(nodes);

   free_expr(e);

   lexer_free();
   return 0;
}
