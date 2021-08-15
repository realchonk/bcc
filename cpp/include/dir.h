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

#ifndef FILE_DIR_H
#define FILE_DIR_H
#include <stdbool.h>
#include <stdio.h> 
#include "token.h"

struct directive {
   const char* name;
   bool(*handler)(size_t linenum, const char* line, struct token* tokens, size_t num_tks, FILE* out);
   bool suppressable;
};

struct directive* get_dir(const char* name, size_t len);

// pre-processor directives

#define define_dir(name) bool dir_##name(size_t, const char*, struct token*, size_t, FILE*)

define_dir(define);
define_dir(undef);
define_dir(include);
define_dir(ifdef);
define_dir(ifndef);
define_dir(endif);
define_dir(else);
define_dir(error);
define_dir(if);
define_dir(elif);

#endif /* FILE_DIR_H */
