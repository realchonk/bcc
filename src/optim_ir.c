#include <tgmath.h>
#include "target.h"
#include "optim.h"
#include "bcc.h"

// remove NOPs
static bool remove_nops(ir_node_t** n) {
   bool success = false;
   ir_node_t* next;
   while ((*n)->type == IR_NOP) {
      next = (*n)->next;
      free_ir_node(*n);
      *n = next;
      success = true;
   }
   ir_node_t* cur = *n;
   while (cur) {
      next = cur->next;
      if (cur->type == IR_NOP)
         ir_remove(cur);
      cur = next;
   }
   return success;
}

// (load R1, 40; iadd R0, R0, R1) -> (iadd R0, R0, 40) 
static bool direct_val(ir_node_t** n) {
   bool success = false;
   
   for (ir_node_t* cur = *n; cur; cur = cur->next) {
      if (cur->type == IR_LOAD
            && ir_isv(cur->next, IR_IADD, IR_ISUB, IR_IMUL, IR_IDIV, IR_UMUL, IR_UDIV,
               IR_IMOD, IR_UMOD, IR_IAND, IR_IOR, IR_IXOR, IR_ILSL, IR_ILSR, IR_IASR,
               IR_ISTEQ, IR_ISTNE, IR_ISTGR, IR_ISTGE, IR_ISTLT, IR_ISTLE,
               IR_USTGR, IR_USTGE, IR_USTLT, IR_USTLE, NUM_IR_NODES)
            ) {
         if (cur->load.dest == cur->next->binary.b.reg) {
            cur->next->binary.b.type = IRT_UINT;
            cur->next->binary.b.uVal = cur->load.value;
            cur->type = IR_NOP;
            success = true;
         } else if (cur->load.dest == cur->next->binary.a.reg) {
            cur->next->binary.a.type = IRT_UINT;
            cur->next->binary.a.uVal = cur->load.value;
            cur->type = IR_NOP;
            success = true;
         }
      }
   }
   return success;
}

// evaluate constant expresions not evaluated by the optim_expr() function.
static bool fold(ir_node_t** n) {
   bool success = false;
   for (ir_node_t* cur = *n; cur; cur = cur->next) {
      if (ir_isv(cur, IR_IADD, IR_ISUB, IR_IMUL, IR_IDIV, IR_UMUL, IR_UDIV,
            IR_IMOD, IR_UMOD, IR_IAND, IR_IOR, IR_IXOR, IR_ILSL, IR_ILSR, IR_IASR,
            IR_ISTEQ, IR_ISTNE, IR_ISTGR, IR_ISTGE, IR_ISTLT, IR_ISTLE,
            IR_USTGR, IR_USTGE, IR_USTLT, IR_USTLE, NUM_IR_NODES)
         && cur->binary.a.type == cur->binary.b.type
         && cur->binary.a.type == IRT_UINT) {
         const ir_reg_t dest = cur->binary.dest;
         const enum ir_value_size sz = cur->binary.size;
         const uintmax_t a = cur->binary.a.uVal;
         const uintmax_t b = cur->binary.b.uVal;
         uintmax_t res;

         switch (cur->type) {
         case IR_IADD:  res = a + b; break;
         case IR_ISUB:  res = a - b; break;
         case IR_IAND:  res = a & b; break;
         case IR_IOR:   res = a | b; break;
         case IR_IXOR:  res = a ^ b; break;
         case IR_ILSL:  res = a << b; break;
         case IR_ILSR:  res = a >> b; break;
         case IR_IASR:  res = (intmax_t)a >> b; break;
         case IR_UMUL:  res = a * b; break;
         case IR_UDIV:  res = a / b; break;
         case IR_UMOD:  res = a % b; break;
         case IR_IMUL:  res = (intmax_t)a * (intmax_t)b; break;
         case IR_IDIV:  res = (intmax_t)a / (intmax_t)b; break;
         case IR_IMOD:  res = (intmax_t)a % (intmax_t)b; break;
         case IR_ISTEQ: res = a == b; break;
         case IR_ISTNE: res = a != b; break;
         case IR_USTGR: res = a >  b; break;
         case IR_USTGE: res = a >= b; break;
         case IR_USTLT: res = a <  b; break;
         case IR_USTLE: res = a <= b; break;
         case IR_ISTGR: res = (intmax_t)a >  (intmax_t)b; break;
         case IR_ISTGE: res = (intmax_t)a >= (intmax_t)b; break;
         case IR_ISTLT: res = (intmax_t)a <  (intmax_t)b; break;
         case IR_ISTLE: res = (intmax_t)a <= (intmax_t)b; break;
         default: continue;
         }

         cur->type = IR_LOAD;
         cur->load.dest = dest;
         cur->load.size = sz;
         cur->load.value = res;
         success = true;
      } else if (cur->type == IR_LOAD && ir_isv(cur->next, IR_INOT, IR_INEG, NUM_IR_NODES)
            && cur->next->unary.reg == cur->load.dest) {
         uintmax_t a = cur->load.value;
         if (cur->next->type == IR_INOT) a = ~a;
         else a = ~a + 1;
         cur->next->type = IR_NOP;
         cur->load.value = a;// & target_get_umax(cur->load.size);
         success = true;
      }
   }
   return success;
}

// (4 * x) -> (x << 2) 
static bool unmuldiv(ir_node_t** n) {
   bool success = false;
   for (ir_node_t* cur = *n; cur; cur = cur->next) {
      if (ir_isv(cur, IR_IMUL, IR_UMUL, IR_IDIV, IR_UDIV, NUM_IR_NODES)
            && ((cur->binary.a.type == IRT_UINT) ^ (cur->binary.b.type == IRT_UINT))) {
         const enum ir_value_size sz = cur->binary.size;
         const ir_reg_t dest = cur->binary.dest;
         ir_reg_t a;
         uintmax_t u;
         if (cur->binary.a.type == IRT_UINT) {
            u = cur->binary.a.uVal;
            a = cur->binary.b.reg;
         } else {
            u = cur->binary.b.uVal;
            a = cur->binary.a.reg;
         }
         if (u == 1) {
            cur->type = IR_NOP;
            success = true;
            continue;
         } else if (u == 0) {
            cur->type = IR_LOAD;
            cur->load.dest = dest;
            cur->load.size = sz;
            cur->load.value = 0;
            success = true;
            continue;
         }
         if (!is_pow2(u)) continue;
         switch (cur->type) {
         case IR_IMUL:
         case IR_UMUL:
            cur->type = IR_ILSL;
            break;
         case IR_IDIV:
            cur->type = IR_IASR;
            break;
         case IR_UDIV:
            cur->type = IR_ILSR;
            break;
         default:
            continue;
         }
         cur->binary.dest = dest;
         cur->binary.size = sz;
         cur->binary.a.type = IRT_REG;
         cur->binary.a.reg = a;
         cur->binary.b.type = IRT_UINT;
         cur->binary.b.uVal = log2(u);
         success = true;
      }
   }
   return success;
}
// (add R0, 42, R0) -> (add R0, R0, 42)
static bool reorder_params(ir_node_t** n) {
   bool success = false;
   for (ir_node_t* cur = *n; cur; cur = cur->next) {
      if (ir_isv(cur, IR_IADD, IR_IMUL, IR_UMUL, IR_IAND, IR_IOR, IR_IXOR, IR_ISTEQ, IR_ISTNE, NUM_IR_NODES)
         && cur->binary.a.type == IRT_UINT
         && cur->binary.b.type == IRT_REG) {
         const struct ir_value tmp = cur->binary.a;
         cur->binary.a = cur->binary.b;
         cur->binary.b = tmp;
         success = true;
      }
   }
   return success;
}

ir_node_t* optim_ir_nodes(ir_node_t* n) {
   if (optim_level < 1) return n;
   while (remove_nops(&n)
      || direct_val(&n)
      || unmuldiv(&n)
      || fold(&n)
      || reorder_params(&n)
   );
   return n;
}
