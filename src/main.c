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
   memcpy(new_str, s, len_s);
   new_str[len_s] = '.';
   memcpy(new_str + len_s + 1, end, len_end);
   new_str[len_s + len_end + 1] = '\0';
   return strint(new_str);
}

static const char* remove_asm_filename;
static void remove_asm_file(void) {
   remove(remove_asm_filename);
}

int main(int argc, char* argv[]) {
   const char* output_file = NULL;
   int level = 'c';
   enable_warnings = true;
   optim_level = 1;
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
         printf("bcc %s\nCopyleft Benjamin Stürz.\n"
               "This software is distributed under the terms of the GPLv2\n", BCC_VER);
#if DISABLE_FP
         puts("Has floating-point: no");
#else
         puts("Has floating-point: yes");
#endif
         printf("Compiled on %s for %s\n", __DATE__, BCC_ARCH);
         return 0;
      case 'm':
         buf_push(target_opts, optarg);
         break;
      case ':':
         fprintf(stderr, "bcc: missing argument for '-%c'\n", optopt);
         return 1;
      case '?':
         fprintf(stderr, "bcc: invalid option '-%c'\n", optopt);
         return 1;
      default: goto print_usage;
      }
   }
   if ((argc - optind) != 1) {
   print_usage:
      fputs("Usage: bcc [options] input\n", stderr);
      return 1;
   }
   const char* source_file = argv[optind];
   FILE* source;
   if (!strcmp(source_file, "-")) source = stdin;
   else source = fopen(source_file, "r");
   if (!source) {
      fprintf(stderr, "bcc: failed to open '%s': %s\n", source_file, strerror(errno));
      return 1;
   }
   
   if (!output_file) {
      if (source == stdin) output_file = "-";
      else switch (level) {
      case 'c':   output_file = replace_ending(source_file, target_info.fend_obj); break;
      case 'S':   output_file = replace_ending(source_file, target_info.fend_asm); break;
      case 'i':   output_file = replace_ending(source_file, "ir"); break;
      case 'A':   output_file = "-"; break;
      }
   }
   FILE* output = NULL;
   const char* asm_filename;
   FILE* asm_file;
   if (level == 'c') {
      asm_filename = tmpnam(NULL);
      asm_file = fopen(asm_filename, "w");
      if (!asm_file)
         panic("failed to open '%s'", asm_filename);
      remove_asm_filename = asm_filename;
      atexit(remove_asm_file);
   } else {
      if (!strcmp(output_file, "-")) output = stdout;
      else output = fopen(output_file, "w");
      if (!output)
         panic("failed to open '%s'", output_file);
      asm_file = output;
   }

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
      if (ec != 0) panic("assembler returned: %d", ec);
   }
   return ec;
}
