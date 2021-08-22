//  Copyright (C) 2021 Benjamin Stürz
//  
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//  
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//  
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include "help_options.h"
#include "parser.h"
#include "target.h"
#include "config.h"
#include "lex.h"
#include "bcc.h"
#include "cpp.h"
#include "ir.h"

static const char* remove_asm_filename;
static void remove_asm_file(void) {
   remove(remove_asm_filename);
}

int main(int argc, char* argv[]) {
   bool dumpmacros = false;
   struct cpp_arg cpp_arg;
   const char* output_file = NULL;
   int level = 'c';
   int option;
   while ((option = getopt(argc, argv, ":d:hm:VO:wciSAo:Ee:I:CD:U:")) != -1) {
      switch (option) {
      case 'h':
         printf("Usage: bcc [options] file...\nOptions:\n%s", help_options);
         return 0;
      case 'o': output_file = optarg; break;
      case 'c':
      case 'i':
      case 'S':
      case 'A':
      case 'E':
         level = option;
         break;
      case 'w':
         enable_warnings = false;
         break;
      case 'd':
         if (!strcmp(optarg, "umpversion")) {
            puts(VERSION);
         } else if (!strcmp(optarg, "umpmachine")) {
            puts(BCC_TARGET);
         } else if (!strcmp(optarg, "umparch")) {
            puts(BCC_FULL_ARCH);
         } else if (!strcmp(optarg, "umpmacros") || !strcmp(optarg, "M")) {
            dumpmacros = true;
            break;
         } else {
            fprintf(stderr, "bcc: invalid option '-d%s'\n", optarg);
            return 1;
         }
         return 0;
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
      case 'e':
         cpp_path = optarg;
         break;
      case 'V':
         printf("bcc %s\nCopyleft Benjamin Stürz.\n"
               "This software is distributed under the terms of the GPLv3\n", VERSION);
#if ENABLE_FP
         puts("Has floating-point: yes");
#else
         puts("Has floating-point: no");
#endif
         printf("Compiled on %s for %s\n", __DATE__, BCC_TARGET);
         return 0;
      case 'm':
         if (!parse_mach_opt(optarg)) return false;
         break;
      case 'I':
      case 'D':
      case 'U':
         cpp_arg.option = option;
         cpp_arg.arg = optarg;
         buf_push(cpp_args, cpp_arg);
         break;
      case 'C':
         console_colors = false;
         break;
      case ':':
         if (optopt != 'd') {
            fprintf(stderr, "bcc: missing argument for '-%c'\n", optopt);
            return 1;
         }
         fallthrough;
      case '?':
         fprintf(stderr, "bcc: invalid option '-%c'\n", optopt);
         return 1;
      default:
      print_usage:;
         fputs("Usage: bcc [options] input\n", stderr);
         return 1;
      }
   }
   if (!emit_prepare()) return 1;
   if ((argc - optind) != 1) {
      fputs("bcc: no input file\n", stderr);
      return 1;
   } else if (dumpmacros) {
      if (level == 'E') {
         cpp_arg.option = 'd';
         cpp_arg.arg = "M";
         buf_push(cpp_args, cpp_arg);
      }
   }
   const char* source_file = argv[optind];
   if (ends_with(source_file, target_info.fend_asm)) {
      if (level != 'c') {
         fprintf(stderr, "bcc: invalid option -'%c' for assembly file\n", level);
         return 1;
      } else return assemble(source_file, replace_ending(source_file, target_info.fend_obj));
   }

   // define macros
   define_macros();

   FILE* source = run_cpp(source_file);
   if (!source) {
      return 1;
   }

   if (!output_file) {
      if (source == stdin) output_file = "-";
      else switch (level) {
      case 'c':   output_file = replace_ending(source_file, target_info.fend_obj); break;
      case 'S':   output_file = replace_ending(source_file, target_info.fend_asm); break;
      case 'i':   output_file = replace_ending(source_file, "ir"); break;
      case 'E':
      case 'A':   output_file = "-"; break;
      }
   }
   FILE* output = NULL;
   const char* asm_filename = NULL;
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
   
   if (level == 'E') {
      int ch;
      while ((ch = fgetc(source)) != EOF) {
         fputc(ch, output);
      }
      fclose(source);
      fclose(output);
      return 0;
   }

   lexer_init(source, source_file);
   if (level == 'S' || level == 'c') emit_init(asm_file);
   parse_unit(level != 'A');
   if (level == 'A') print_unit(output);
   else if (level == 'i') print_ir_unit(output);
   else emit_unit();
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
