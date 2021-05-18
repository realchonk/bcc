#include <stdlib.h>
#include "optim.h"
#include "error.h"
#include "bcc.h"

static struct statement* make_nop(struct statement* old) {
   struct statement* s = malloc(sizeof(struct statement));
   if (!s) panic("make_nop(): failed to allocate statement");
   s->type = STMT_NOP;
   s->begin = old->begin;
   s->end = old->end;
   s->parent = old->parent;
   s->func = old->func;
   free_stmt(old);
   return s;
}

struct statement* optim_stmt(struct statement* s) {
   if (optim_level < 1) return s;
   if (stmt_is_pure(s)) return make_nop(s);
   switch (s->type) {
   case STMT_EXPR:
   case STMT_RETURN:
      if (s->expr)
         s->expr = optim_expr(s->expr);
      break;
   case STMT_IF:
   {
      struct value val;
      if (try_eval_expr(s->ifstmt.cond, &val)) {
         bool cond;
         switch (val.type->type) {
         case VAL_INT:     cond = val.iVal != 0; break;
#if !DISABLE_FP
         case VAL_FLOAT:   cond = val.fVal != 0.0; break;
#endif
         default:          goto end_if;
         }
         struct statement* branch;
         if (cond) {
            free_stmt(s->ifstmt.false_case);
            branch = s->ifstmt.true_case;
         } else {
            free_stmt(s->ifstmt.true_case);
            branch = s->ifstmt.false_case;
         }
         free_expr(s->ifstmt.cond);
         return branch;
      }
end_if:
      break;
   }
   case STMT_WHILE:
   case STMT_DO_WHILE:
      s->whileloop.cond = optim_expr(s->whileloop.cond);
      s->whileloop.stmt = optim_stmt(s->whileloop.stmt);
      if (s->whileloop.end)
         s->whileloop.end = optim_expr(s->whileloop.end);
   default:
      break;
   }
   return s;
}
