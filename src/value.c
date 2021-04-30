#include <stdlib.h>
#include "target.h"
#include "scope.h"
#include "value.h"
#include "error.h"
#include "lex.h"

const char* integer_size_str[NUM_INTS] = {
   "byte",
   "char",
   "short",
   "int",
   "long",
};
const char* fp_size_str[NUM_FPS] = {
   "float",
   "double",
};

void print_value_type(FILE* file, const struct value_type* val) {
   switch (val->type) {
   case VAL_INT:
      if (val->integer.is_unsigned) fputs("unsigned ", file);
      fputs(integer_size_str[val->integer.size], file);
      break;
   case VAL_FLOAT:
      fputs(fp_size_str[val->fp.size], file);
      break;
   case VAL_POINTER:
      print_value_type(file, val->pointer.type);
      fputc('*', file);
      break;
   default: panic("print_value_type(): invalid value type '%d'", val->type);
   }
   if (val->is_const) fputs(" const", file);
}

void free_value_type(struct value_type* val) {
   if (val->type == VAL_POINTER) free_value_type(val->pointer.type);
   free(val);
}

static struct value_type* new_vt(void) {
   struct value_type* vt = malloc(sizeof(struct value_type));
   if (!vt) panic("new_vt(): failed to allocate value type");
   else return vt;
}

// const unsigned int* const
struct value_type* parse_value_type(void) {
   bool has_begon = false;
   bool has_signedness = false;
   struct value_type* vt = new_vt();
   vt->type = NUM_VALS;
   vt->integer.is_unsigned = false;

   while (lexer_matches(KW_CONST) || lexer_matches(KW_SIGNED) || lexer_matches(KW_UNSIGNED)) {
      const struct token tk = lexer_next();
      switch (tk.type) {
      case KW_CONST:    vt->is_const = true; break;
      case KW_SIGNED:
         vt->type = VAL_INT;
         vt->integer.size = INT_INT;
         vt->integer.is_unsigned = false;
         has_signedness = true;
         break;
      case KW_UNSIGNED:
         vt->type = VAL_INT;
         vt->integer.size = INT_INT;
         vt->integer.is_unsigned = true;
         has_signedness = true;
         break;
      default:          panic("parse_value_type(): invalid token type '%s'", token_type_str[tk.type]);
      }
      if (!has_begon) {
         has_begon = true;
         vt->begin = tk.begin;
      }
   }
   struct token tk = lexer_peek();
   switch (tk.type) {
   case KW_BYTE:
      lexer_skip();
      vt->type = VAL_INT;
      vt->integer.size = INT_BYTE;
      lexer_match(KW_INT);
      break;
   case KW_CHAR:
      lexer_skip();
      vt->type = VAL_INT;
      vt->integer.size = INT_CHAR;
      lexer_match(KW_INT);
      if (!has_signedness) vt->integer.is_unsigned = target_info.unsigned_char;
      break;
   case KW_SHORT:
      lexer_skip();
      vt->type = VAL_INT;
      vt->integer.size = INT_SHORT;
      lexer_match(KW_INT);
      break;
   case KW_LONG:
      lexer_skip();
      vt->type = VAL_INT;
      vt->integer.size = INT_LONG;
      lexer_match(KW_INT);
      break;
   case KW_INT:
      lexer_skip();
      vt->type = VAL_INT;
      vt->integer.size = INT_INT;
      break;
   case KW_FLOAT:
      lexer_skip();
      if (vt->type != NUM_VALS) parse_error(&tk.begin, "invalid combination of signed or unsigned and float");
      vt->type = VAL_FLOAT;
      vt->fp.size = FP_FLOAT;
      break;
   case KW_DOUBLE:
      lexer_skip();
      if (vt->type != NUM_VALS) parse_error(&tk.begin, "invalid combination of signed or unsigned and double");
      vt->type = VAL_FLOAT;
      vt->fp.size = FP_DOUBLE;
      break;
   default: break;
   }
   if (vt->type == NUM_VALS) {
      // no error handling
      free(vt);
      return NULL;
   }
   if (!has_begon) {
      vt->begin = tk.begin;
      has_begon = true;
   }
   vt->end = tk.end;
   while (lexer_matches(KW_CONST)) {
      vt->is_const = true;
      vt->end = lexer_next().end;
   }
   while (lexer_matches(TK_STAR)) {
      const struct token tk2 = lexer_next();
      struct value_type* ptr = new_vt();
      ptr->type = VAL_POINTER;
      ptr->pointer.type = vt;
      ptr->begin = tk2.begin;
      ptr->end = tk2.end;
      while (lexer_match(KW_CONST)) ptr->is_const = true;
      vt = ptr;
   }
   return vt;
}

struct value_type* common_value_type(const struct value_type* a, const struct value_type* b, bool warn) {
   struct value_type* c = new_vt();
   c->is_const = false;
   if (a->type == b->type) {
      c->type = a->type;
      switch (a->type) {
      case VAL_INT:
         if (a->integer.is_unsigned != b->integer.is_unsigned) {
            if (warn) parse_warn(&a->begin, "performing operation between signed and unsigned");
            c->integer.is_unsigned = true;
         } else c->integer.is_unsigned = a->integer.is_unsigned;
         c->integer.size = my_max(a->integer.size, b->integer.size);
         break;
      case VAL_FLOAT:
         c->fp.size = my_max(a->fp.size, b->fp.size);
         break;
      case VAL_POINTER: parse_error(&a->end, "performing binary operation on pointers");
      default:          panic("common_value_type(): unsupported value type '%d'", a->type);
      }
   } else {
      if (a->type == VAL_INT && b->type == VAL_FLOAT) {
         c->type = VAL_FLOAT;
         c->fp.size = b->fp.size;
      } else if (a->type == VAL_FLOAT && b->type == VAL_INT) {
         c->type = VAL_FLOAT;
         c->fp.size = a->fp.size;
      } else if (a->type == VAL_POINTER && b->type == VAL_INT) {
         c->type = VAL_POINTER;
         c->pointer.type = copy_value_type(a->pointer.type);
      } else if (a->type == VAL_INT && b->type == VAL_POINTER) {
         c->type = VAL_POINTER;
         c->pointer.type = copy_value_type(b->pointer.type);
      } else parse_error(&a->end, "invalid combination of types");
   }
   return c;
}

struct value_type* common_value_type_free(struct value_type* a, struct value_type* b, bool warn) {
   struct value_type* c = common_value_type(a, b, warn);
   free_value_type(a);
   free_value_type(b);
   return c;
}

struct value_type* get_value_type(struct scope* scope, const struct expression* e) {
   struct value_type* type;
   switch (e->type) {
   case EXPR_PREFIX:
   case EXPR_PAREN:
      return get_value_type(scope, e->expr);
   case EXPR_INT:
   case EXPR_UINT:
      type = new_vt();
      type->type = VAL_INT;
      type->is_const = false;
      type->integer.is_unsigned = e->type == EXPR_UINT;
      if (type->integer.is_unsigned) {
         type->integer.size = e->uVal > target_info.max_uint ? INT_LONG : INT_INT;
      } else {
         type->integer.size = e->iVal > target_info.max_int ? INT_LONG : INT_INT;
      }
      return type;
   case EXPR_FLOAT:
      type = new_vt();
      type->type = VAL_FLOAT;
      type->is_const = true;
      type->fp.size = FP_DOUBLE;
      return type;
   case EXPR_CHAR:
      type = new_vt();
      type->type = VAL_INT;
      type->is_const = true;
      type->begin = e->begin;
      type->end = e->end;
      type->integer.is_unsigned = target_info.unsigned_char;
      type->integer.size = INT_CHAR;
      return type;
   case EXPR_NAME: {
      const struct variable* var = scope_find_var(scope, e->str);
      if (!var) parse_error(&e->begin, "undeclared variable '%s'", e->str);
      return copy_value_type(var->type);
   }
   case EXPR_ADDROF:
      type = new_vt();
      type->type = VAL_POINTER;
      type->is_const = true;
      type->pointer.type = get_value_type(scope, e->expr);
      return type;
   case EXPR_INDIRECT: {
      struct value_type* tmp = get_value_type(scope, e->expr);
      if (tmp->type != VAL_POINTER) parse_error(&e->expr->begin, "cannot dereference a non-pointer");
      type = copy_value_type(tmp->pointer.type);
      free_value_type(tmp);
      return type;
   }
   case EXPR_STRING:
      type = new_vt();
      type->type = VAL_POINTER;
      type->is_const = true;
      type->pointer.type = new_vt();
      type->pointer.type->type = VAL_INT;
      type->pointer.type->integer.size = INT_CHAR;
      type->pointer.type->integer.is_unsigned = target_info.unsigned_char;
      return type;
   case EXPR_COMMA:
      return get_value_type(scope, e->comma[buf_len(e->comma) - 1]);
   case EXPR_SUFFIX:
      type = get_value_type(scope, e->expr);
      type->is_const = true;
      return type;
   case EXPR_ASSIGN:
      return get_value_type(scope, e->binary.left);
   case EXPR_BINARY:
      return common_value_type_free(get_value_type(scope, e->binary.left), get_value_type(scope, e->binary.right), true); // TODO
   case EXPR_TERNARY:
      return common_value_type_free(get_value_type(scope, e->ternary.true_case), get_value_type(scope, e->ternary.false_case), true);
   case EXPR_UNARY:
      type = get_value_type(scope, e->unary.expr);
      if (e->unary.op.type == TK_MINUS) parse_warn(&e->begin, "negating an unsigned integer");
      type->is_const = true;
      return type;
   case EXPR_CAST:
      return copy_value_type(e->cast.type);
   default: panic("get_value_type(): unsupported expression '%s'", expr_type_str[e->type]);
   }
}

struct value_type* copy_value_type(const struct value_type* vt) {
   struct value_type* copy = new_vt();
   copy->type = vt->type;
   copy->begin = vt->begin;
   copy->end = vt->end;
   copy->is_const = vt->is_const;
   switch (vt->type) {
   case VAL_INT:
      copy->integer.size = vt->integer.size;
      copy->integer.is_unsigned = vt->integer.is_unsigned;
      break;
   case VAL_FLOAT:
      copy->fp.size = vt->fp.size;
      break;
   case VAL_POINTER:
      copy->pointer.type = copy_value_type(vt->pointer.type);
      break;
   default: panic("copy_value_type(): unsupported value type '%d'", vt->type);
   }
   return copy;
}
