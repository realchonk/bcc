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

#include <string.h>
#include "target.h"
#include "regs.h"
#include "ir.h"

ir_node_t* emit_ir(const ir_node_t* n) {
   const char* instr;
   switch (n->type) {
   case IR_ASM:
      emit("%s", n->str);
      return n->next;
   case IR_PROLOGUE:
   {
      const size_t num_reg_params = my_min(4, buf_len(n->func->params));
      emit("push {r4-r11,lr}");
      return n->next;
   }

   case IR_EPILOGUE:
   {
      if (!strcmp(n->func->name, "main"))
         emit("mov r0, #0");
      emit("%s.ret:", n->func->name);
      emit("pop {r4-r11,pc}");
      return n->next;
   }

   case IR_BEGIN_SCOPE:
   case IR_END_SCOPE:
      return n->next;


   case IR_LOAD:
   {
      const intmax_t val = n->load.value;
      if (val >= 0) {
         emit("mov %s, #%jd", reg(n->load.dest), val);
      } else {
         emit("mvn %s, #%jd", -val - 1);
      }
      return n->next;
   }
   
   case IR_IRET:
      if (n->unary.reg != 0) {
         emit("mov r0, %s", reg(n->unary.reg));
      }
      fallthrough;
   case IR_RET:
      emit("b %s.ret", n->func->name);
      return n->next;

   default:
      panic("unsupported ir_node type '%s'", ir_node_type_str[n->type]);
   }
}
