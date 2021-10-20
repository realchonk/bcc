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

#include <tgmath.h>
#include "target.h"
#include "error.h"
#include "optim.h"
#include "bcc.h"

// remove NOPs
static bool remove_nops(ir_node_t** n) {
   bool success = false;
   if (!*n)
      return false;
   ir_node_t* next;
   while (*n && (*n)->type == IR_NOP) {
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

static bool is_immed(const intmax_t v) {
   return v >= target_info.min_immed && v <= target_info.max_immed;
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
            && is_immed(cur->load.value) ) {
         ir_node_t* next = cur->next;
         if (next->binary.b.type == IRT_REG && cur->load.dest == next->binary.b.reg) {
            next->binary.b.type = IRT_UINT;
            next->binary.b.uVal = cur->load.value;
            cur->type = IR_NOP;
            success = true;
         } else if (next->binary.a.type == IRT_REG && cur->load.dest == next->binary.a.reg) {
            next->binary.a.type = IRT_UINT;
            next->binary.a.uVal = cur->load.value;
            cur->type = IR_NOP;
            success = true;
         }
      }
   }
   return success;
}

// evaluate constant expressions not evaluated by the optim_expr() function.
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

// (add R0, R0, 0) -> (nop)
// (imul R0, R0, 1) -> (nop)
// (imul R0, R0, 0) -> (load R0, 0)
// (idiv R0, R0, 0) -> warning
static bool add_zero(ir_node_t** n) {
   bool success = false;
   for (ir_node_t* cur = *n; cur; cur = cur->next) {
      if (ir_isv(cur, IR_IADD, IR_ISUB, IR_ILSL, IR_ILSR, IR_IASR, IR_IOR, IR_IXOR, NUM_IR_NODES)
         && cur->binary.b.type == IRT_UINT
         && cur->binary.b.uVal == 0) {
         cur->type = IR_NOP;
         success = true;
      } else if (ir_isv(cur, IR_IMUL, IR_UMUL, IR_IDIV, IR_UDIV, IR_IAND, NUM_IR_NODES)
         && cur->binary.b.type == IRT_UINT
         && cur->binary.b.uVal == 1) {
         cur->type = IR_NOP;
         success = true;
      } else if (ir_isv(cur, IR_IMUL, IR_UMUL, NUM_IR_NODES)
         && cur->binary.b.type == IRT_UINT
         && cur->binary.b.uVal == 0) {
         const ir_reg_t dest = cur->binary.dest;
         const enum ir_value_size sz = cur->binary.size;
         cur->type = IR_LOAD;
         cur->load.dest = dest;
         cur->load.value = 0;
         cur->load.size = sz;
         success = true;
      } else if (ir_isv(cur, IR_IDIV, IR_UDIV, NUM_IR_NODES)
         && cur->binary.b.type == IRT_UINT
         && cur->binary.b.uVal == 0) {
         fprintf(stderr, "integer division by zero in IR code\n");
      }
   }
   return success;
}

// (load R0, 42; load R0, 39; ... R0) -> (load R0, 39)
static bool remove_unreferenced(ir_node_t** n) {
   if (optim_level < 3) return false; // XXX: experimental
   bool success = false;
   for (ir_node_t* cur = *n; cur && cur->next; cur = cur->next) {
      if (cur->type == IR_READ && !ir_is_used(cur->next, cur->rw.dest)) {
         cur->type = IR_NOP;
         success = true;
      }
   }
   return success;
}

static enum ir_node_type rcall_to_fcall(enum ir_node_type t) {
   switch (t) {
   case IR_RCALL:    return IR_FCALL;
   case IR_IRCALL:   return IR_IFCALL;
   default:          panic("invalid IR node type %s", ir_node_type_str[t]);
   }
}

// (flookup R0, add; read.ptr R0, R0; rcall R0) -> (fcall add)
static bool direct_call(ir_node_t** n) {
   bool success = false;
   for (ir_node_t* cur = *n; cur; cur = cur->next) {
      if (ir_isv(cur, IR_RCALL, IR_IRCALL, NUM_IR_NODES)
         && ir_is(cur->call.addr, IR_FLOOKUP) && !cur->call.addr->next) {
         const istr_t name = cur->call.addr->lstr.str;
         cur->type = rcall_to_fcall(cur->type);
         cur->call.name = name;
         success = true;
      }
   }
   return success;
}

// (imod R0, R0, 16) -> (iand R0, R0, 15)
// (imod R0, R0, 1) -> (load R0, 0)
// (imod R0, R0, 0) -> (warning)
static bool mod_to_and(ir_node_t** n) {
   bool success = false;
   for (ir_node_t* cur = *n; cur; cur = cur->next) {
      if ((cur->type == IR_IMOD || cur->type == IR_UMOD)
         && cur->binary.b.type == IRT_UINT
         && cur->binary.a.type == IRT_REG) {
         const unsigned pc = popcnt(cur->binary.b.uVal);
         if (pc == 1) {
            const uintmax_t mask = cur->binary.b.uVal - 1;
            if (mask) {
               cur->type = IR_IAND;
               cur->binary.b.uVal = mask;
            } else {
               const ir_reg_t dest = cur->binary.dest;
               const enum ir_value_size sz = cur->binary.size;
               cur->type = IR_LOAD;
               cur->load.dest = dest;
               cur->load.size = sz;
               cur->load.value = 0;
            }
            success = true;
         } else if (!pc) fprintf(stderr, "integer modulo by zero in IR code\n");
      }
   }
   return success;
}

// (load.char R0, 42; iicast.int R0, char R0) -> (load.int R0, 42)
static bool fuse_load_iicast(ir_node_t** n) {
   bool success = false;
   for (ir_node_t* cur = *n; cur; cur = cur->next) {
      if (cur->type == IR_LOAD && ir_is(cur->next, IR_IICAST)
         && cur->load.dest == cur->next->iicast.src) {
         cur->load.dest = cur->next->iicast.dest;
         const enum ir_value_size ds = cur->next->iicast.ds;
         const enum ir_value_size ss = cur->next->iicast.ss;
         uintmax_t value = cur->load.value;
         if (cur->next->iicast.sign_extend && ds > ss) {
            const uintmax_t dm = target_get_umax(ds);
            const uintmax_t sm = target_get_umax(ss);
            if (value & ((sm >> 1) + 1))
               value |= dm & ~sm;
         } else if (ds < ss) {
            const uintmax_t dm = target_get_umax(ds);
            value &= dm;
         }
         cur->load.value = value;
         cur->next->type = IR_NOP;
         success = true;
      }
   }
   return success;
}

// (move R0, R0) -> (nop)
static bool useless_move(ir_node_t** n) {
   bool success = false;
   for (ir_node_t* cur = *n; cur; cur = cur->next) {
      if (cur->type == IR_MOVE && cur->move.dest == cur->move.src) {
         cur->type = IR_NOP;
         success = true;
      }
   }
   return success;
}

#define is_rw(t) (((t) == IR_READ) || ((t) == IR_WRITE))
#define get_memreg(n) (((n)->type == IR_READ) ? (n)->rw.src : (n)->rw.dest)
#define get_datreg(n) (((n)->type == IR_READ) ? (n)->rw.dest : (n)->rw.src)

static bool check_if_used(ir_node_t* cur, ir_reg_t r) {
   ir_node_t* next = cur->next;
   if (next->type == IR_READ && next->rw.dest == next->rw.src)
      return false;

   return ir_is_used(next->next, r);
}

// (fparam R0, 0; read R0, R0) -> (ffprd R0, 0)
static bool fuse_fp_rw(ir_node_t** n) {
   bool success = false;
   for (ir_node_t* cur = *n; cur && cur->next; cur = cur->next) {
      ir_node_t* next = cur->next;
      if (cur->type == IR_FPARAM && is_rw(next->type) && cur->fparam.reg == get_memreg(next)) {
         if (check_if_used(cur, get_memreg(next)))
            continue;
         ir_node_t tmp;
         tmp.type = next->type == IR_READ ? IR_FFPRD : IR_FFPWR;
         tmp.prev = cur->prev;
         tmp.next = next->next;
         tmp.func = cur->func;
         tmp.ffprw.reg = get_datreg(next);
         tmp.ffprw.idx = cur->fparam.idx;
         tmp.ffprw.size = next->rw.size;
         tmp.ffprw.sign_extend = next->rw.sign_extend;
         tmp.ffprw.is_volatile = next->rw.is_volatile;

         ir_remove(next);
         *cur = tmp;
         success = true;
      }
   }
   return success;
}

// (glookup R0, name; write R0, R1) -> (fglwr name, R1)
static bool fuse_gl_rw(ir_node_t** n) {
   bool success = false;
   for (ir_node_t* cur = *n; cur && cur->next; cur = cur->next) {
      ir_node_t* next = cur->next;
      if (cur->type == IR_GLOOKUP && is_rw(next->type) && cur->lstr.reg == get_memreg(next)) {
         if (check_if_used(cur, get_memreg(next)))
            continue;
         ir_node_t tmp;
         tmp.type = next->type == IR_READ ? IR_FGLRD : IR_FGLWR;
         tmp.prev = cur->prev;
         tmp.next = next->next;
         tmp.func = cur->func;
         tmp.fglrw.reg = get_datreg(next);
         tmp.fglrw.name = cur->lstr.str;
         tmp.fglrw.size = next->rw.size;
         tmp.fglrw.sign_extend = next->rw.sign_extend;
         tmp.fglrw.is_volatile = next->rw.is_volatile;

         ir_remove(next);
         *cur = tmp;
         success = true;
      }
   }
   return success;
}

// (lookup R0, ...; read R1, R0) -> (flurd R1, ...)
static bool fuse_lu_rw(ir_node_t** n) {
   bool success = false;
   for (ir_node_t* cur = *n; cur && cur->next; cur = cur->next) {
      ir_node_t* next = cur->next;
      if (cur->type == IR_LOOKUP && is_rw(next->type) && cur->lookup.reg == get_memreg(next)) {
         if (check_if_used(cur, get_memreg(next)))
            continue;
         ir_node_t tmp;
         tmp.type = next->type == IR_READ ? IR_FLURD : IR_FLUWR;
         tmp.prev = cur->prev;
         tmp.next = next->next;
         tmp.func = cur->func;
         tmp.flurw.reg = get_datreg(next);
         tmp.flurw.scope = cur->lookup.scope;
         tmp.flurw.var_idx = cur->lookup.var_idx;
         tmp.flurw.size = next->rw.size;
         tmp.flurw.sign_extend = next->rw.sign_extend;
         tmp.flurw.is_volatile = next->rw.is_volatile;
         
         ir_remove(next);
         *cur = tmp;
         success = true;
      }
   }
   return success;
}

// Fuse memory-nodes (experimental)
static bool fuse_memops(ir_node_t** n) {
   if (optim_level < 3)
      return false;
   bool success = false;

   if (target_info.fuse_fp_rw)
      success |= fuse_fp_rw(n);

   if (target_info.fuse_gl_rw)
      success |= fuse_gl_rw(n);

   if (target_info.fuse_lu_rw)
      success |= fuse_lu_rw(n);

   return success;
}

ir_node_t* optim_ir_nodes(ir_node_t* n) {
   if (optim_level < 1) {
      while (target_optim_ir(&n));
      while (target_post_optim_ir(&n));
      return n;
   }
   while (remove_nops(&n)
      || direct_val(&n)
      || fuse_memops(&n)
      || unmuldiv(&n)
      || fold(&n)
      || reorder_params(&n)
      || add_zero(&n)
      //|| remove_unreferenced(&n)
      || direct_call(&n)
      || mod_to_and(&n)
      || fuse_load_iicast(&n)
      || target_optim_ir(&n)
      || useless_move(&n)
   );
   while (target_post_optim_ir(&n));
   return n;
}
