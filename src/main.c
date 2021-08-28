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
#include "linker.h"
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
   struct cmdline_arg cmd_arg;
   const char* output_name = NULL;
   enum compilation_level level = LEVEL_LINK;
   int option;
   while ((option = getopt(argc, argv, ":d:hm:VO:wciSAo:Ee:I:CD:U:L:l:sn:")) != -1) {
      switch (option) {
      case 'h':
         printf("Usage: bcc [options] file...\nOptions:\n%s", help_options);
         return 0;
      case 'o': output_name = optarg;     break;
      case 'E': level = LEVEL_PREPROCESS; break;
      case 'A': level = LEVEL_PARSE;      break;
      case 'i': level = LEVEL_IRGEN;      break;
      case 'S': level = LEVEL_GEN;        break;
      case 'c': level = LEVEL_ASSEMBLE;   break;
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
      case 'n':
         if (!strcmp(optarg, "ostdinc")) {
            nostdinc = true;
         } else if (!strcmp(optarg, "ostdlib")) {
            nostartfiles = nolibc = true;
         } else if (!strcmp(optarg, "ostartfiles")) {
            nostartfiles = true;
         } else if (!strcmp(optarg, "olibc")) {
            nolibc = true;
         } else {
            fprintf(stderr, "bcc: invalid option '-n%s'\n", optarg);
            return 1;
         }
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
         cmd_arg.option = option;
         cmd_arg.arg = optarg;
         buf_push(cpp_args, cmd_arg);
         break;
      case 'C':
         console_colors = false;
         break;
      case 'L':
      case 'l':
      case 's':
         cmd_arg.option = option;
         cmd_arg.arg = optarg;
         buf_push(linker_args, cmd_arg);
         break;
      case ':':
         if (optopt != 'd' && optopt != 'n') {
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
   const int nfiles = argc - optind;
   if (nfiles < 1) {
      fputs("bcc: no input file\n", stderr);
      return 1;
   } else if (nfiles > 1 && level != LEVEL_LINK) {
      fputs("bcc: '-o' cannot be specified with '-E', '-A', '-i', '-S' or '-c' with multiple input files\n", stderr);
      return 1;
   }
   if (dumpmacros) {
      if (level == 'E') {
         cmd_arg.option = 'd';
         cmd_arg.arg = "M";
         buf_push(cpp_args, cmd_arg);
      }
   }
   if (!emit_prepare()) return 1;
   define_macros();

   const char** objects = NULL;

   for (; optind < argc; ++optind) {
      const char* source_name = argv[optind];
      const char* output_name2;
      if (level == LEVEL_LINK) {
         output_name2 = create_output_name(source_name, LEVEL_ASSEMBLE);
         buf_push(objects, output_name2);
      } else if (!output_name)
         output_name2 = create_output_name(source_name, level);
      else output_name2 = output_name;
      if (ends_with(source_name, target_info.fend_obj))
         continue;
      const int ec = process_file(source_name, output_name2, level);
      if (ec != 0) return ec;
   }

   if (level == LEVEL_LINK) {
      run_linker(output_name, objects);
   }

   return 0;
}
