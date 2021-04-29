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
   int level = 'S';
   int option;
   while ((option = getopt(argc, argv, ":iSAo:")) != -1) {
      switch (option) {
      case 'o': output_file = optarg; break;
      case 'i':
      case 'S':
      case 'A':
         level = option;
         break;
      default: goto print_usage;
      }
   }
   if ((argc - optind) != 1) {
   print_usage:
      fputs("Usage: bcc [-iAS] [-o output] input\n", stderr);
      return 1;
   }
   
   const char* filename = argv[optind++];
   FILE* file = fopen(filename, "r");
   if (!file) {
      fprintf(stderr, "bcc: failed to access '%s': %s\n", filename, strerror(errno));
      return 1;
   }
   FILE* output;
   if (output_file && strcmp(output_file, "-") != 0) output = fopen(output_file, "w");
   else output = stdout;
   if (!output) {
      fprintf(stderr, "bcc: failed to access '%s': %s\n", output_file, strerror(errno));
      return 1;
   }

   lexer_init(file, filename);

   while (!lexer_match(TK_EOF)) {
      struct function* func = parse_func();

      if (level == 'A') {
         print_func(output, func);
      } else {
         ir_node_t* ir = irgen_func(func);
         if (level == 'i') {
            print_ir_nodes(output, ir);
         } else {
            emit_init(output);
            while ((ir = emit_ir(ir)) != NULL);
         }
         free_ir_nodes(ir);
      }

      free_func(func);
   }


   lexer_free();
   if (output != stdout) fclose(output);
   return 0;
}
