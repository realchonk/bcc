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

#ifndef FILE_SCOPE_H
#define FILE_SCOPE_H
#include "value.h"
#include "buf.h"

struct expression;
struct statement;

struct variable {
   struct value_type* type;
   istr_t name;
   struct expression* init;
   struct source_pos begin, end;
   uintmax_t addr; // used by backend
   unsigned attrs;
   bool has_const_value;
   struct value const_init;
};

struct scope {
   struct scope* parent;
   struct scope** children;
   struct variable* vars;
   struct statement** body;
   struct function* func;
};

struct scope* make_scope(struct scope* parent, struct function* func);
void print_scope(FILE*, const struct scope*);
void free_scope(struct scope*);

const struct variable* scope_find_var(struct scope*, const char*);
size_t scope_find_var_idx(struct scope*, struct scope**, const char*);
size_t scope_add_var(struct scope*, const struct variable*);

#endif /* FILE_SCOPE_H */
