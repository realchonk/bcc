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
#include "value.h"
#include "error.h"
#include "optim.h"
#include "scope.h"
#include "expr.h"
#include "unit.h"

#if DISABLE_FP
#define do_binary(t, op) \
   switch ((t)->type) { \
   case VAL_INT: \
      if ((t)->integer.is_unsigned) \
         result.uVal = left.uVal op right.uVal; \
      else result.iVal = left.iVal op right.iVal; \
      break; \
   default: \
      free_value_type(result.type); \
      return false; \
   }

#else
#define do_binary(t, op) \
   switch ((t)->type) { \
   case VAL_INT: \
      if ((t)->integer.is_unsigned) \
         result.uVal = left.uVal op right.uVal; \
      else result.iVal = left.iVal op right.iVal; \
      break; \
   case VAL_FLOAT: \
      result.fVal = left.fVal op right.fVal; \
      break; \
   default: \
      free_value_type(result.type); \
      return false; \
   }
#endif

#define do_binary_int(t, op) \
   switch ((t)->type) { \
   case VAL_INT: \
      if ((t)->integer.is_unsigned) \
         result.uVal = left.uVal op right.uVal; \
      else result.iVal = left.iVal op right.iVal; \
      break; \
   default: \
      free_value_type(result.type); \
      return false; \
   }

bool try_eval_expr(struct expression* e, struct value* val) {
   if (val) {
      val->begin = e->begin;
      val->end = e->end;
   }
   switch (e->type) {
   case EXPR_PAREN:
      return try_eval_expr(e->expr, val);
   case EXPR_INT:
      val->type = copy_value_type(get_value_type(NULL, e));
      val->iVal = e->iVal;
      return true;
   case EXPR_UINT:
      val->type = copy_value_type(get_value_type(NULL, e));
      val->uVal = e->uVal;
      return true;
#if !DISABLE_FP
   case EXPR_FLOAT:
      val->type = copy_value_type(get_value_type(NULL, e));
      val->fVal = e->fVal;
      return true;
#endif
   case EXPR_BINARY:
   {
      struct value left, right, result;
      if (!try_eval_expr(e->binary.left, &left) || !try_eval_expr(e->binary.right, &right)) return false;
      result.type = common_value_type_free(left.type, right.type, false); // TODO: convert to common

      switch (e->binary.op.type) {
      case TK_PLUS:
         do_binary(result.type, +);
         break;
      case TK_MINUS:
         do_binary(result.type, -);
         break;
      case TK_STAR:
         do_binary(result.type, *);
         break;
      case TK_SLASH:
         do_binary(result.type, /);
         break;
      case TK_AMP:
         do_binary_int(result.type, &);
         break;
      case TK_PIPE:
         do_binary_int(result.type, |);
         break;
      case TK_XOR:
         do_binary_int(result.type, ^);
         break;
      case TK_EQEQ:
         do_binary(result.type, ==);
         break;
      case TK_NEQ:
         do_binary(result.type, !=);
         break;
      case TK_GR:
         do_binary(result.type, >);
         break;
      case TK_GREQ:
         do_binary(result.type, >=);
         break;
      case TK_LE:
         do_binary(result.type, <);
         break;
      case TK_LEEQ:
         do_binary(result.type, <=);
         break;
      case TK_GRGR:
         do_binary_int(result.type, >>);
         break;
      case TK_LELE:
         do_binary_int(result.type, <<);
         break;
      default:
         return false;
      }
      *val = result;
      return true;
   }
   case EXPR_UNARY:
   {
      struct value right, result;
      if (!try_eval_expr(e->unary.expr, &right)) return false;
      result.type = right.type;
      right.type = NULL;

      switch (e->unary.op.type) {
      case TK_PLUS:
         result = right;
         break;
      case TK_MINUS:
         switch (result.type->type) {
         case VAL_INT:
            if (result.type->integer.is_unsigned)
               parse_error(&e->unary.op.begin, "negation of an unsigned value");
            result.iVal = -right.iVal;
            break;
#if !DISABLE_FP
         case VAL_FLOAT:
            result.fVal = -right.fVal;
            break;
#endif
         default:
            free_value_type(result.type);
            return false;
         }
         break;
      case TK_WAVE:
         if (result.type->type != VAL_INT)
            parse_error(&e->unary.op.begin, "bitwise NOT is only applicable to integers");
         result.uVal = ~right.uVal;
         break;
      default:
         return false;
      }
      *val = result;
      return true;
   }
   case EXPR_COMMA:
   {
      size_t i = 0;
      while (i < (buf_len(e->comma) - 1)) {
         struct expression* sub = e->comma[i];
         struct value val;
         if (try_eval_expr(sub, &val)) {
            buf_remove(e->comma, i, 1);
            free_expr(sub);
         } else ++i;
      }
      optim_expr(e->comma[buf_len(e->comma) - 1]);
      if (buf_len(e->comma) == 1) {
         struct expression* sub = e->comma[0];
         struct value result;
         if (try_eval_expr(sub, &result)) {
            *val = result;
            return true;
         }
      }
      return false;
   }
   default:
      return false;
   }
}
bool var_is_declared(istr_t name, struct scope* scope) {
   return (scope && scope_find_var(scope, name)) || unit_get_var(name) || unit_get_func(name);
}
void eval_expr(struct expression* e, struct value* v) {
   if (!try_eval_expr(e, v))
      parse_error(&e->begin, "expected constant-expression");
}
