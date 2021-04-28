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

   struct scope* scope = make_scope(NULL);
   struct statement* s = parse_stmt(scope);

   puts("expr:");
   print_stmt(stdout, s);
   
   puts("\nIR:");

   ir_node_t* nodes = irgen_stmt(s);

   print_ir_nodes(stdout, nodes);

   puts("\nASM:");

   emit_init(stdout);
   
   ir_node_t* n = nodes;
   while (n) n = emit_ir(n);




   free_ir_nodes(nodes);

   free_stmt(s);
   free_scope(scope);

   lexer_free();
   return 0;
}
