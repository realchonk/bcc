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

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "target.h"
#include "bcc.h"

bool enable_warnings = true;
bool console_colors = true;
unsigned optim_level = 1;

unsigned popcnt(const uintmax_t val) {
   const unsigned len = sizeof(val) * 8;
   unsigned num = 0;
   for (unsigned i = 0; i < len; ++i) {
      num += (val >> i) & 1;
   }
   return num;
}

istr_t replace_ending(const char* s, const char* end) {
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
bool ends_with(const char* str, const char* end) {
   const char* ending = strrchr(str, '.');
   return ending && !strcmp(ending + 1, end);
}
int get_mach_opt_vtype(const char* value) {
   if (!value) return 0;
   while (isdigit(*value)) ++value;
   return *value ? 2 : 1;
}
bool parse_mach_opt(char* arg) {
   if (!strcmp(arg, "help")) {
      puts("Machine Options for " BCC_FULL_ARCH);
      const size_t max_len = 24;
      for (size_t i = 0; i < num_mach_opts; ++i) {
         const struct machine_option* opt = &mach_opts[i];
         size_t n = (size_t)printf("-m%s", opt->name);
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
