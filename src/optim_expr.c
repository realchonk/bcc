#include "value.h"
#include "optim.h"
#include "error.h"
#include "bcc.h"

struct expression* optim_expr(struct expression* e) {
   if (optim_level < 1) return e;
   if (e->type == EXPR_INT || e->type == EXPR_UINT
    || e->type == EXPR_STRING 
#if !DISABLE_FP
    || e->type == EXPR_FLOAT
#endif
    || e->type == EXPR_CHAR || optim_level < 1)
      return e;
   struct value result;
   if (try_eval_expr(e, &result)) {
      free_expr(e);
      e = new_expr();
      switch (result.type->type) {
      case VAL_INT:
         if (result.type->integer.is_unsigned) {
            e->type = EXPR_UINT;
            e->uVal = result.uVal;
         } else {
            e->type = EXPR_INT;
            e->iVal = result.iVal;
         }
         break;
#if !DISABLE_FP
      case VAL_FLOAT:
         e->fVal = result.fVal;
         break;
#endif
      default:
         panic("invaid value type '%d'", result.type->type);
      }
      free_value_type(result.type);
   }
   return e;
}
