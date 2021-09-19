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
#include <errno.h>
#include "target.h"
#include "error.h"
#include "cpp.h"
#include "lex.h"
#include "bcc.h"

bool enable_warnings = true;
bool console_colors = true;
unsigned optim_level = 1;
bool save_temps = false;

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
   if (!strcmp(s, "-")) {
      char new_str[8 + len_end + 2];
      snprintf(new_str, sizeof(new_str), "bcc_temp.%s", end);
      return strint(new_str);
   }
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
bool ends_with_one(const char* str, const char** ends) {
   while (*ends) {
      if (ends_with(str, *ends))
         return true;
      ++ends;
   }
   return false;
}

const char* create_output_name(const char* source, enum compilation_level level) {
   switch (level) {
   case LEVEL_PREPROCESS:  return "-";
   case LEVEL_PARSE:       return "-";
   case LEVEL_IRGEN:       return replace_ending(source, "ir");
   case LEVEL_GEN:         return replace_ending(source, target_info.fend_asm[0]);
   case LEVEL_ASSEMBLE:    return replace_ending(source, target_info.fend_obj[0]);
   case LEVEL_LINK:        return "a.out";
   default:                panic("unreachable reached");
   }
}
static FILE* open_file_write(const char* name) {
   FILE* file = !strcmp(name, "-") ? stdout : fopen(name, "w");
   if (!file) {
      fprintf(stderr, "bcc: failed to open file '%s': %s", name, strerror(errno));
   }
   return file;
}
static void close_file(FILE* file) {
   if (file == stdout || file == stdin)
      return;
   fclose(file);
}

int process_file(const char* source_name, const char* output_name, enum compilation_level level) {
   if (ends_with_one(source_name, target_info.fend_asm)) {
      return level >= LEVEL_ASSEMBLE ? assemble(source_name, output_name) : 0;
   }
   FILE* source = run_cpp(source_name);
   if (!source)
      return 1;
   if (level == LEVEL_PREPROCESS) {
      FILE* output = open_file_write(output_name);
      if (!output)
         return 1;
      int ch;
      while ((ch = fgetc(source)) != EOF)
         fputc(ch, output);
      close_file(output);
      close_file(source);
      return 0;
   }
   lexer_init(source, source_name);

   target_init();

   parse_unit(level >= LEVEL_IRGEN);
   lexer_free();

   if (level == LEVEL_PARSE || level == LEVEL_IRGEN) {
      FILE* output = open_file_write(output_name);
      if (!output)
         return 1;
      if (level == LEVEL_PARSE)
         print_unit(output);
      else print_ir_unit(output);
      free_unit();
      close_file(output);
      return 0;
   }

   const char* asm_name;
   if (level >= LEVEL_ASSEMBLE) {
      asm_name = create_output_name(source_name, LEVEL_GEN);
   } else asm_name = output_name;
   FILE* asm_file = open_file_write(asm_name);
   if (level >= LEVEL_GEN)
      emit_init(asm_file);
   emit_unit();
   free_unit();
   emit_free();
   if (level <= LEVEL_GEN)
      return 0;
   const int ec = assemble(asm_name, output_name);  
   remove(asm_name);
   return ec;
}
