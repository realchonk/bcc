#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
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

static int get_mach_opt_vtype(const char* value) {
   if (!value) return 0;
   while (isdigit(*value)) ++value;
   return *value ? 2 : 1;
}
static bool parse_mach_opt(char* arg) {
   if (!strcmp(arg, "help")) {
      puts("Machine Options for " BCC_ARCH);
      const size_t max_len = 24;
      for (size_t i = 0; i < num_mach_opts; ++i) {
         const struct machine_option* opt = &mach_opts[i];
         size_t n = (size_t)printf("-m%s", opt->name);
         switch (opt->type) {
         case 1:
            n += (size_t)printf("=integer");
            break;
         case 2:
            n += (size_t)printf("=integer");
            break;
         default:
            break;
         }
         for (; n < max_len; ++n)
            putchar(' ');
         puts(opt->description);
      }
      return false;
   }
   char* value = strchr(arg, '=');
   const size_t len_arg = value ? (size_t)(value - arg) : strlen(arg);
   for (size_t i = 0; i < num_mach_opts; ++i) {
      struct machine_option* opt = &mach_opts[i];
      const size_t len_name = strlen(opt->name);
      if (len_name == len_arg && !memcmp(arg, opt->name, len_arg)) {
         const int type = get_mach_opt_vtype(value);
         if (type != opt->type) {
            if (value) *value = '\0';
            fprintf(stderr, "bcc: invalid type for option '-m%s'\n", arg);
            return false;
         } else if (value) {
            if (type == 1) opt->iVal = atoi(value);
            else if (type == 2) opt->sVal = value;
            else {
               *value = '\0';
               fprintf(stderr, "bcc: unexpected value for '-m%s'\n", arg);
               return false;
            }
         } else if (!value) {
            if (type == 0) opt->bVal = true;
            else {
               fprintf(stderr, "bcc: expected value for '-m%s'\n", arg);
               return false;
            }
         }
         return true;
      } else if (len_arg == (len_name + 3) && !memcmp("no-", arg, 3) && !memcmp(opt->name, arg + 3, len_arg - 3)) {
         if (value) {
            *value = '\0';
            fprintf(stderr, "bcc: unexpected '=' for '-m%s'\n", arg);
            return false;
         } else if (opt->type) break;
         opt->bVal = 0;
         return true;
      }
   }
   if (value) *value = '\0';
   fprintf(stderr, "bcc: invalid option '-m%s'\n", arg);
   return false;
}

int main(int argc, char* argv[]) {
   const char* output_file = NULL;
   int level = 'c';
   enable_warnings = true;
   optim_level = 1;
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
         if (!parse_mach_opt(optarg)) return false;
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
