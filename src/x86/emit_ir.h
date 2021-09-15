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
   if (!v) return 0;
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
      char buffer[snslen(v->sVal)];
      snprintf(buffer, sizeof(buffer), "%jd", v->sVal);
      return strint(buffer);
   }
   default:
      panic("invalid IR value type");
   }
}
