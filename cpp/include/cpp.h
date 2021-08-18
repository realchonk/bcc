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

#ifndef FILE_CPP_H
#define FILE_CPP_H
#include <stdbool.h>
#include <stdio.h>
#include "config.h"
#if HAVE_STDNORETURN_H
#include <stdnoreturn.h>
#else
#define noreturn
#endif
#include "buf.h"

extern const char* source_name;
extern bool console_color;
extern bool failed;

struct line_pair {
   char* line;
   size_t linenum;
};

int run_cpp(FILE* in, FILE* out);
struct line_pair* read_lines(FILE* file);
void print_lines(FILE* out, const struct line_pair*);
void free_lines(struct line_pair*);

void warn(size_t linenum, const char*, ...);
void fail(size_t linenum, const char*, ...);

int eval(size_t linenum, const char* s);

noreturn void panic_impl(const char*, const char*, ...);
#define panic(...) panic_impl(__func__, __VA_ARGS__)

// Initialization stuff
void init_macros(void);
void init_includes(void);


#endif /* FILE_CPP_H */
