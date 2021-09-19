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

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "cmdline.h"
#include "target.h"
#include "config.h"

void vexecl_print(const char* prog, ...) {
   if (!verbose)
      return;
   va_list ap;
   va_start(ap, prog);

   const char* s;
   fprintf(stderr, "Calling %s:", prog);
   while ((s = va_arg(ap, const char*)) != NULL) {
      fprintf(stderr, " %s", s);
   }
   fputc('\n', stderr);
}

// machine options

struct flag_option* get_mach_opt(const char* name) {
   for (size_t i = 0; i < num_mach_opts; ++i) {
      struct flag_option* opt = &mach_opts[i];
      if (!strcmp(name, opt->name))
         return opt;
   }
   panic("no such option: -m%s", name);
}
struct flag_option* get_flag_opt(const char* name) {
   for (size_t i = 0; i < num_flag_opts; ++i) {
      struct flag_option* opt = &flag_opts[i];
      if (!strcmp(name, opt->name))
         return opt;
   }
   panic("no such option: -f%s", name);
}
static enum flag_option_type get_opt_value_type(const char* value) {
   if (!value) return 0;
   while (isdigit(*value)) ++value;
   return *value ? 2 : 1;
}
static bool parse_generic_opt(char* arg, struct flag_option* flags, const size_t num_flags, const char* hm, char prefix) {
   if (!strcmp(arg, "help")) {
      puts(hm);
      const size_t max_len = 24;
      for (size_t i = 0; i < num_flags; ++i) {
         const struct flag_option* opt = &flags[i];
         size_t n = (size_t)printf("-%c%s", prefix, opt->name);
         switch (opt->type) {
         case 1:
            n += (size_t)printf("=integer");
            break;
         case 2:
            n += (size_t)printf("=string");
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
   if (value) ++value;
   for (size_t i = 0; i < num_flags; ++i) {
      struct flag_option* opt = &flags[i];
      const size_t len_name = strlen(opt->name);
      if (len_name == len_arg && !memcmp(arg, opt->name, len_arg)) {
         const enum flag_option_type type = get_opt_value_type(value);
         if (type != opt->type) {
            if (value) *value = '\0';
            fprintf(stderr, "bcc: invalid type for option '-%c%s'\n", prefix, arg);
            return false;
         } else if (value) {
            if (type == 1) opt->iVal = atoi(value);
            else if (type == 2) opt->sVal = value;
            else {
               *value = '\0';
               fprintf(stderr, "bcc: unexpected value for '-%c%s'\n", prefix, arg);
               return false;
            }
         } else if (!value) {
            if (type == 0) opt->bVal = true;
            else {
               fprintf(stderr, "bcc: expected value for '-%c%s'\n", prefix, arg);
               return false;
            }
         }
         return true;
      } else if (len_arg == (len_name + 3) && !memcmp("no-", arg, 3) && !memcmp(opt->name, arg + 3, len_arg - 3)) {
         if (value) {
            *value = '\0';
            fprintf(stderr, "bcc: unexpected '=' for '-%c%s'\n", prefix, arg);
            return false;
         } else if (opt->type) break;
         opt->bVal = 0;
         return true;
      }
   }
   if (value) *value = '\0';
   fprintf(stderr, "bcc: invalid option '-%c%s'\n", prefix, arg);
   return false;
}

bool parse_mach_opt(char* arg) {
   return parse_generic_opt(arg, mach_opts, num_mach_opts, "Machine Options for " BCC_FULL_ARCH ":", 'm');
}
bool parse_flag_opt(char* arg) {
   return parse_generic_opt(arg, flag_opts, num_flag_opts, "Flag options:", 'f');
}
