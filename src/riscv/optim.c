#define _GNU_SOURCE // asprintf
#include <string.h>
#include <stdio.h>
#include "riscv/cpu.h"
#include "optim.h"
#include "error.h"

#if BITS == 32
#define SBITS "32"
#else
#define SBITS "64"
#endif

// Turn IR_MUL to IR_IFCALL (if no M extension)
static bool mul_to_func(ir_node_t** n) {
   if (riscv_cpu.has_mult)
      return false;
   bool success = false;
   ir_node_t* cur;
   for (cur = *n; cur; cur = cur->next) {
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
      char name[] = "__mulxi" SBITS;
      memcpy(name + 2, type, 3);
      name[5] = sign;

      ir_node_t fc;
      fc.type = IR_IFCALL;
      fc.ifcall.name = strint(name);
      fc.ifcall.dest = cur->binary.dest;
      fc.ifcall.params = NULL;

      ir_node_t* tmp = new_node(IR_NOP);
      if (cur->binary.a.type == IRT_REG) {
         tmp->type = IR_MOVE;
         tmp->move.dest = fc.ifcall.dest;
         tmp->move.src = cur->binary.a.reg;
         tmp->move.size = cur->binary.size;
      } else if (cur->binary.a.type == IRT_UINT) {
         tmp->type = IR_LOAD;
         tmp->load.dest = fc.ifcall.dest;
         tmp->load.size = cur->binary.size;
         tmp->load.value = cur->binary.a.uVal;
      } else panic("unreachable reached");
      buf_push(fc.ifcall.params, tmp);

      tmp = new_node(IR_NOP);
      if (cur->binary.b.type == IRT_REG) {
         tmp->type = IR_MOVE;
         tmp->move.dest = fc.ifcall.dest;
         tmp->move.src = cur->binary.b.reg;
         tmp->move.size = cur->binary.size;
      } else if (cur->binary.b.type == IRT_UINT) {
         tmp->type = IR_LOAD;
         tmp->load.dest = fc.ifcall.dest;
         tmp->load.size = cur->binary.size;
         tmp->load.value = cur->binary.b.uVal;
      } else panic("unreachable reached");
      buf_push(fc.ifcall.params, tmp);

      *cur = fc;
      success = true;
   }
   return success;
}

// TODO: implement target-specific IR optimizations
bool target_optim_ir(ir_node_t** n) {
   bool success = false;
   while (mul_to_func(n))
      success = true;
   return success;
}
