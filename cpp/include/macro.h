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

#ifndef FILE_MACRO_H
#define FILE_MACRO_H
#include <stdbool.h>
#include <stdio.h>
#include "strint.h"
#include "buf.h"

enum macro_type {
   MACRO_VAR,     // just your normal #define X 42
   MACRO_FUNC,    // macro function (eg. #define f(x) x)
   MACRO_SPEC,    // macro backed by a C function (__FILE__, __TIME__ etc.)
};

typedef char*(*special_macro_handler_t)(size_t linenum);

struct macro {
   enum macro_type type;
   istr_t name;
   size_t linenum;
   const char* text;
   union {
      special_macro_handler_t handler;
      istr_t* params;
   };
};

void add_macro(const struct macro*);
bool remove_macro(istr_t);
const struct macro* get_macro(istr_t);
void add_cmdline_macro(const char* arg);
void dump_macros(FILE*);

#endif /* FILE_MACRO_H */
