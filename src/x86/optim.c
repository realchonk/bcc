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
#include "optim.h"
#include "error.h"

static ir_node_t* val_to_node(const struct ir_value* val, const ir_reg_t dest, const enum ir_value_size irs) {
   ir_node_t* n;
   switch (val->type) {
   case IRT_REG:
      n = new_node(IR_MOVE);
      n->move.dest = dest;
      n->move.src = val->reg;
      n->move.size = irs;
      break;
   case IRT_UINT:
      n = new_node(IR_LOAD);
      n->load.dest = dest;
      n->load.value = val->uVal;
      n->load.size = irs;
      break;
   default:
      panic("unreachable reached");
   }
   return n;
}
static bool mul_to_func(ir_node_t** n) {
   bool success = false;
   for (ir_node_t* cur = *n; cur; cur = cur->next) {
      const char* type;
      char sign;
      switch (cur->type) {
      case IR_IMUL:
         type = "mul";
         sign = 's';
         break;
      case IR_UMUL:
         type = "mul";
         sign = 'u';
         break;
      case IR_IDIV:
         type = "div";
         sign = 's';
         break;
      case IR_UDIV:
         type = "div";
         sign = 'u';
         break;
      case IR_IMOD:
         type = "mod";
         sign = 's';
         break;
      case IR_UMOD:
         type = "mod";
         sign = 'u';
         break;
      default:
         continue;
      }
      const ir_reg_t dest = cur->binary.dest;
      char name[] = "__mulxixx";
      snprintf(name, sizeof(name), "__%s%ci%zu", type, sign, irs2sz(cur->binary.size) * 8);
      ir_node_t func;
      func.type = IR_IFCALL;
      func.ifcall.name = strint(name);
      func.ifcall.dest = dest;
      func.ifcall.params = NULL;
      buf_push(func.ifcall.params, val_to_node(&cur->binary.a, dest, cur->binary.size));
      buf_push(func.ifcall.params, val_to_node(&cur->binary.b, dest, cur->binary.size));
      *cur = func;
      success = true;
   }
   return success;
}

// TODO: implement target-specific IR optimizations
bool target_optim_ir(struct ir_node** n) {
   return false;
}

bool target_post_optim_ir(struct ir_node** n) {
   bool success = false;
   while (mul_to_func(n))
      success = true;
   return success;
}

