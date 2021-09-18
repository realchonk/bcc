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

#include <math.h>
#include "target.h"
#include "value.h"
#include "scope.h"
#include "regs.h"
#include "bcc.h"
#include "ir.h"

static void emit_clear(const char* r) {
   if (optim_level > 0) {
      emit("xor %s, %s", r, r);
   } else {
      emit("mov %s, 0", r);
   }
}
static size_t snslen(intmax_t v) {
   size_t n = 0;
   if (!v) return 1;
   if (v < 0) {
      v = -v;
      ++n;
   }
   return (size_t)log10(v) + 1 + n;
}
static const char* irv2str(const struct ir_value* v) {
   switch (v->type) {
   case IRT_REG:
      return reg(v->reg);
   case IRT_UINT:
   {
      char buffer[snslen(v->sVal) + 1];
      snprintf(buffer, sizeof(buffer), "%jd", v->sVal);
      return strint(buffer);
   }
   default:
      panic("invalid IR value type");
   }
}

// TODO: merge with include/riscv/emit_ir.h
static size_t sizeof_scope(const struct scope* scope) {
   size_t num = 0;
   for (size_t i = 0; i < buf_len(scope->vars); ++i) {
      num += sizeof_value(scope->vars[i].type, false);
   }
   size_t max_child = 0;
   for (size_t i = 0; i < buf_len(scope->children); ++i) {
      const size_t sz = sizeof_scope(scope->children[i]);
      if (sz > max_child)
         max_child = sz;
   }
   return num + max_child;
}
static size_t align_stack_size(size_t n) {
   return n & 15 ? (n & ~15) + 16 : n;
}

static void assign_scope(struct scope* scope, size_t* sp) {
   for (size_t i = 0; i < buf_len(scope->vars); ++i) {
      *sp += sizeof_value(scope->vars[i].type, false);
      scope->vars[i].addr = *sp;
   }
   for (size_t i = 0; i < buf_len(scope->children); ++i) {
      size_t tmp_sp = *sp;
      assign_scope(scope->children[i], &tmp_sp);
      if (tmp_sp > *sp)
         *sp = tmp_sp;
   }
}
