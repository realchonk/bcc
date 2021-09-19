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

#ifndef FILE_BCC_H
#define FILE_BCC_H
#include <stdbool.h>
#include <stdint.h>
#include "cmdline.h"
#include "strint.h"

extern bool enable_warnings;
extern unsigned optim_level;
extern bool console_colors;
extern bool save_temps;

enum compilation_level {
   LEVEL_NONE,
   LEVEL_PREPROCESS,
   LEVEL_PARSE,
   LEVEL_IRGEN,
   LEVEL_GEN,
   LEVEL_ASSEMBLE,
   LEVEL_LINK,          // TODO: implement linking
};

unsigned popcnt(uintmax_t);
#define is_pow2(n) (popcnt(n) == 1)

istr_t replace_ending(const char* s, const char* end);
bool ends_with(const char* s, const char* end);
bool ends_with_one(const char* s, const char** end);
enum flag_option_type get_mach_opt_type(const char*);
const char* create_output_name(const char*, enum compilation_level);
int process_file(const char* source, const char* output, enum compilation_level);

#endif /* FILE_BCC_H */

