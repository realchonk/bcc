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

#ifndef FILE_CMDLINE_H
#define FILE_CMDLINE_H
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

struct cmdline_arg {
   char option;
   const char* arg;
};

// options specified with -f and -m
enum flag_option_type {
   FLAG_BOOL,
   FLAG_INT,
   FLAG_STRING,
};

struct flag_option {
   const char* name;
   const char* description;
   enum flag_option_type type;
   union {
      bool bVal;
      int32_t iVal;
      const char* sVal;
   };
};

// print debug information to standard error
extern bool verbose;

// '-f' options
extern struct flag_option flag_opts[];

// arraylen(flag_opts)
extern const size_t num_flag_opts;

// verbose version of execl()
#define verbose_execl(prog, arg0, ...) \
   vexecl_print(prog, __VA_ARGS__); \
   execlp(prog, arg0, __VA_ARGS__)

void vexecl_print(const char* name, ...);

// returns an existing (-m) machine-option; or panic
struct flag_option* get_mach_opt(const char* name);

// returns an existing (-f) flag-option; or panic
struct flag_option* get_flag_opt(const char* name);

// parse a machine options
bool parse_mach_opt(char*);

// parse a flag option
bool parse_flag_opt(char*);

#endif /* FILE_CMDLINE_H */
