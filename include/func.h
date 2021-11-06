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

#ifndef FILE_FUNC_H
#define FILE_FUNC_H
#include "strint.h"
#include "value.h"
#include "scope.h"

struct ir_node;

struct function {
   istr_t name;
   struct value_type* type;
   struct variable* params;
   struct scope* scope;             // optional
   struct source_pos begin, end;
   struct ir_node* ir_code;         // optional
   bool variadic;
   unsigned attrs;
   unsigned max_reg;
   struct ir_big_iload* big_iloads;
   istr_t* labels;
};

void parse_func_part(struct function*);
void print_func(FILE*, const struct function*);
void free_func(struct function*);

size_t func_find_param_idx(const struct function*, const char*);
const struct variable* func_find_param(const struct function*, const char*);
bool func_has_label(const struct function*, istr_t);

#endif /* FILE_FUNC_H */
