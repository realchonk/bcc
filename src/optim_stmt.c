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

#include <stdlib.h>
#include "optim.h"
#include "error.h"
#include "bcc.h"

static struct statement* make_nop(struct statement* old) {
   struct statement* s = malloc(sizeof(struct statement));
   if (!s) panic("failed to allocate statement");
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
   case STMT_SWITCH:
   {
      if (optim_level < 2) break;
      struct value val;
      if (!try_eval_expr(s->sw.expr, &val)) break;
      size_t def_lbl = SIZE_MAX;
      struct statement* new_st = new_stmt();
      new_st->type = STMT_SCOPE;
      new_st->parent = s->parent;
      new_st->func = s->func;
      for (size_t i = 0; i < buf_len(s->sw.body); ++i) {
         const struct switch_entry* e = &s->sw.body[i];
         if (e->type == SWITCH_CASE && e->cs.value.uVal == val.uVal) {
            new_st->begin = e->begin;
            new_st->scope = make_scope(s->parent, s->func);
            for (; i < buf_len(s->sw.body); ++i) {
               e = &s->sw.body[i];
               if (e->type == SWITCH_STMT) {
                  if (e->stmt->type == STMT_BREAK) break;
                  buf_push(new_st->scope->body, e->stmt);
               }
            }
            new_st->end = new_st->scope->body[buf_len(new_st->scope->body) - 1]->end;
            goto end_switch;
         } else if (e->type == SWITCH_DEFAULT) def_lbl = i;
      }
      if (def_lbl == SIZE_MAX) {
         new_st->type = STMT_NOP;
         new_st->begin = s->begin;
         new_st->end = s->end;
      } else {
         const struct switch_entry* e = &s->sw.body[def_lbl];
         new_st->scope = make_scope(s->parent, s->func);
         new_st->begin = e->begin;
         for (size_t i = def_lbl + 1; i < buf_len(s->sw.body); ++i) {
            e = &s->sw.body[i];
            if (e->type == SWITCH_STMT) {
               if (e->stmt->type == STMT_BREAK) break;
               buf_push(new_st->scope->body, e->stmt);
            }
         }
      }
end_switch:
      buf_free(s->sw.body);
      free_expr(s->sw.expr);
      free(s);
      return new_st;
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
