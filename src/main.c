#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "parser.h"
#include "target.h"
#include "lex.h"
#include "ir.h"

static char* replace_ending(const char* s, const char* end) {
   const size_t len_end = strlen(end);
   const char* dot = strrchr(s, '.');
   char* new_str;
   const size_t len_s = dot ? (size_t)(dot - s) : strlen(s);
   new_str = malloc(len_s + len_end + 2);
   if (!new_str) panic("failed to allocate buffer");
   strncpy(new_str, s, len_s);
   strncat(new_str, ".", 1);
   strncat(new_str, end, len_end);
   return new_str;
}

int main(int argc, char* argv[]) {
   const char* output_file = NULL;
   int level = 'c';
   int option;
   while ((option = getopt(argc, argv, ":ciSAo:")) != -1) {
      switch (option) {
      case 'o': output_file = optarg; break;
      case 'c':
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
      fputs("Usage: bcc [-ciAS] [-o output] input\n", stderr);
      return 1;
   }
   const char* source_file = argv[optind];
   FILE* source = fopen(source_file, "r");
   if (!source) panic("failed to access '%s'", source_file);
   
   if (!output_file) {
      switch (level) {
      case 'c':   output_file = replace_ending(source_file, target_info.fend_obj); break;
      case 'S':   output_file = replace_ending(source_file, target_info.fend_asm); break;
      case 'i':   output_file = replace_ending(source_file, "ir"); break;
      case 'A':   output_file = "-"; break;
      }
   }
   FILE* output;
   if (level != 'c') {
      if (strcmp(output_file, "-") == 0) output = stdout;
      else output = fopen(output_file, "w");
   }

   const char* asm_filename;
   FILE* asm_file;
   if (level == 'c') {
      asm_filename = tmpnam(NULL);
      asm_file = fopen(asm_filename, "w");
   } else asm_file = output;

   lexer_init(source, source_file);
   if (level == 'S' || level == 'c') emit_init(asm_file);
   while (!lexer_match(TK_EOF)) {
      struct function* func = parse_func();
      if (level == 'A') print_func(output, func);
      else {
         ir_node_t* ir = irgen_func(func);
         if (level == 'i') print_ir_nodes(output, ir);
         else {
            emit_func(func, ir);
         }
         free_ir_nodes(ir);
      }
      free_func(func);
   }
   lexer_free();
   int ec = 0;
   if (level == 'c') {
      fclose(asm_file);
      ec = assemble(asm_filename, output_file);
      if (ec != 0) panic("assembler returned: %d");
   }
   return ec;
}
