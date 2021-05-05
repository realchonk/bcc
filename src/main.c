#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "parser.h"
#include "target.h"
#include "lex.h"
#include "bcc.h"
#include "ir.h"

bool enable_warnings;
unsigned optim_level;

static istr_t replace_ending(const char* s, const char* end) {
   const size_t len_end = strlen(end);
   const char* dot = strrchr(s, '.');
   const size_t len_s = dot ? (size_t)(dot - s) : strlen(s);
   char new_str[len_s + len_end + 2];
   strncpy(new_str, s, len_s);
   strncat(new_str, ".", 1);
   strncat(new_str, end, len_end);
   return strint(new_str);
}

int main(int argc, char* argv[]) {
   const char* output_file = NULL;
   int level = 'c';
   enable_warnings = true;
   optim_level = 0;
   const char** target_opts = NULL;
   int option;
   while ((option = getopt(argc, argv, ":m:VO:wciSAo:")) != -1) {
      switch (option) {
      case 'o': output_file = optarg; break;
      case 'c':
      case 'i':
      case 'S':
      case 'A':
         level = option;
         break;
      case 'w':
         enable_warnings = false;
         break;
      case 'O':
      {
         char* endp;
         optim_level = (unsigned)strtoul(optarg, &endp, 10);
         if (*endp) {
            fprintf(stderr, "bcc: '%s' is not a valid number.\n", optarg);
            return 1;
         }
         break;
      }
      case 'V':
         printf("bcc %s\nCopyright (C) Benjamin Stürz.\n"
               "This software is distributed under the terms of the GPLv2\n"
               "Compiled for %s\n", BCC_VER, BCC_ARCH);
         return 0;
      case 'm':
         buf_push(target_opts, optarg);
         break;
      default: goto print_usage;
      }
   }
   if ((argc - optind) != 1) {
   print_usage:
      fputs("Usage: bcc [options] input\n", stderr);
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
   parse_unit();
   if (level == 'A') print_unit(output);
   else if (level == 'i') print_ir_unit(output);
   else emit_unit(target_opts);
   free_unit();
   lexer_free();
   
   int ec = 0;
   if (level == 'S' || level == 'c') emit_free();
   if (level == 'c') {
      ec = assemble(asm_filename, output_file);
      if (ec != 0) panic("assembler returned: %d");
   }
   return ec;
}
