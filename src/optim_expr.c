#include <string.h>
#include "error.h"
#include "value.h"
#include "optim.h"
#include "bcc.h"

struct value {
   struct value_type* type;
   union {
      istr_t sVal;
      intmax_t iVal;
      uintmax_t uVal;
      fpmax_t fVal;
   };
};

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

static bool try_eval(const struct expression* e, struct value* val) {
   switch (e->type) {
   case EXPR_PAREN:
      return try_eval(e->expr, val);
   case EXPR_INT:
      val->type = get_value_type(NULL, e);
      val->iVal = e->iVal;
      return true;
   case EXPR_UINT:
      val->type = get_value_type(NULL, e);
      val->uVal = e->uVal;
      return true;
   case EXPR_FLOAT:
      val->type = get_value_type(NULL, e);
      val->fVal = e->fVal;
      return true;
   case EXPR_BINARY:
   {
      struct value left, right, result;
      if (!try_eval(e->binary.left, &left) || !try_eval(e->binary.right, &right)) return false;
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
      if (!try_eval(e->unary.expr, &right)) return false;
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
         case VAL_FLOAT:
            result.fVal = -right.fVal;
            break;
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
         if (try_eval(sub, &val)) {
            buf_remove(e->comma, i, 1);
            free_expr(sub);
         } else ++i;
      }
      optim_expr(e->comma[buf_len(e->comma) - 1]);
      if (buf_len(e->comma) == 1) {
         struct expression* sub = e->comma[0];
         struct value result;
         if (try_eval(sub, &result)) {
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

struct expression* optim_expr(struct expression* e) {
   if (e->type == EXPR_INT || e->type == EXPR_UINT
    || e->type == EXPR_STRING || e->type == EXPR_FLOAT
    || e->type == EXPR_CHAR || optim_level < 1)
      return e;
   struct value result;
   if (try_eval(e, &result)) {
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
      case VAL_FLOAT:
         e->fVal = result.fVal;
         break;
      default:
         panic("optim_expr(): invaid value type '%d'", result.type->type);
      }
      free_value_type(result.type);
   }
   return e;
}
