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

#include "target.h"
#include "regs.h"

static size_t sizeof_scope(const struct scope* scope) {
   size_t num = 0;
   for (size_t i = 0; i < buf_len(scope->vars); ++i) {
      num += sizeof_value(scope->vars[i].type, false);
   }
   size_t max_child = 0;
   for (size_t i = 0; i < buf_len(scope->children); ++i) {
      const size_t sz = sizeof_scope(scope->children[i]);
      if (sz > max_child) max_child = sz;
   }
   return num + max_child;
}

static void assign_scope(struct scope* scope, uintreg_t* sp) {
   for (size_t i = 0; i < buf_len(scope->vars); ++i) {
      *sp += sizeof_value(scope->vars[i].type, false);
      scope->vars[i].addr = *sp;
   }
   for (size_t i = 0; i < buf_len(scope->children); ++i) {
      assign_scope(scope->children[i], sp);
   }
}
