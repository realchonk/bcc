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

// Common target-specific optimizations

#include "optim.h"

// IR_COPY -> IR_FCALL(__builtin_memcpy)
bool copy_to_memcpy(ir_node_t** n) {
   bool success = false;
   for (ir_node_t* cur = *n; cur; cur = cur->next) {
      if (cur->type != IR_COPY)
         continue;
      ir_node_t fcall;
      fcall.type = IR_FCALL;
      fcall.prev = cur->prev;
      fcall.next = cur->next;
      fcall.func = cur->func;

      fcall.call.name = strint("__builtin_memcpy");
      fcall.call.dest = cur->copy.dest;
      fcall.call.params = NULL;
      
      ir_node_t* param = new_node(IR_MOVE);
      param->move.dest = fcall.call.dest;
      param->move.src  = cur->copy.dest;
      param->move.size = IRS_PTR;
      buf_push(fcall.call.params, param);

      param = new_node(IR_MOVE);
      param->move.dest = fcall.call.dest;
      param->move.src  = cur->copy.src;
      param->move.size = IRS_PTR;
      buf_push(fcall.call.params, param);

      param = new_node(IR_LOAD);
      param->load.dest = fcall.call.dest;
      param->load.value = cur->copy.len;
      param->load.size = IRS_PTR;
      buf_push(fcall.call.params, param);
      *cur = fcall;
      success = true;
   }
   return success;
}
