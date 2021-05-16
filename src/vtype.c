#include <stdlib.h>
#include <assert.h>
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
const char* value_type_str[NUM_VALS] = {
   "integer",
   "floating-point number",
   "pointer",
   "void",
   "auto",
   "enum",
   "struct",
   "union",
};

static bool ptreq(const struct value_type* a, const struct value_type* b) {
   if (a->type != b->type) return false;
   switch (a->type) {
   case VAL_INT:     return (a->integer.is_unsigned == b->integer.is_unsigned) && (a->integer.size == b->integer.size);
   case VAL_FLOAT:   return a->fp.size == b->fp.size;
   case VAL_POINTER: return ptreq(a->pointer.type, b->pointer.type);
   case VAL_STRUCT:  return a->vstruct->name == b->vstruct->name;
   default:          panic("ptreq(): invalid value type '%u'", a->type);
   }
}
bool value_type_equal(const struct value_type* a, const struct value_type* b) {
   return a->is_const == b->is_const && ptreq(a, b);
}
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
   case VAL_VOID:
      fputs("void", file);
      break;
   case VAL_ENUM:
      fputs("enum", file);
      if (val->venum->name)
         fprintf(file, " %s", val->venum->name);
      if (val->venum->is_definition) {
         fputs(" {\n", file);
         for (size_t i = 0; i < buf_len(val->venum->entries); ++i) {
            const struct enum_entry* e = &val->venum->entries[i];
            fprintf(file, "\t%s = %jd,\n", e->name, e->value);
         }
         fputc('}', file);
      }
      break;
   case VAL_STRUCT:
   case VAL_UNION:
      fputs(val->type == VAL_UNION ? "union" : "struct", file);
      if (val->vstruct->name)
         fprintf(file, " %s", val->vstruct->name);
      if (val->vstruct->is_definition) {
         fputs(" {\n", file);
         for (size_t i = 0; i < buf_len(val->vstruct->entries); ++i) {
            const struct struct_entry* e = &val->vstruct->entries[i];
            fputc('\t', file);
            print_value_type(file, e->type);
            fprintf(file, " %s;\n", e->name);
         }
         fputc('}', file);
      }
      break;
   default: panic("print_value_type(): invalid value type '%d'", val->type);
   }
   if (val->is_const) fputs(" const", file);
}

void free_value_type(struct value_type* val) {
   switch (val->type) {
   case VAL_POINTER:
      free_value_type(val->pointer.type);
      break;
   case VAL_ENUM:
      buf_free(val->venum->entries);
      free(val->venum);
      break;
   case VAL_STRUCT:
      for (size_t i = 0; i < buf_len(val->vstruct->entries); ++i)
         free_value_type(val->vstruct->entries[i].type);
      buf_free(val->vstruct->entries);
      free(val->vstruct);
   default:
      break;
   }
   free(val);
}

static struct value_type* new_vt(void) {
   struct value_type* vt = malloc(sizeof(struct value_type));
   if (!vt) panic("new_vt(): failed to allocate value type");
   else return vt;
}

struct value_type* make_int(enum integer_size sz, bool is_unsigned) {
   struct value_type* vt = new_vt();
   vt->type = VAL_INT;
   vt->integer.size = sz;
   vt->integer.is_unsigned = is_unsigned;
   return vt;
}

// const unsigned int* const
struct value_type* parse_value_type(struct scope* scope) {
   struct value_type* vt = new_vt();
   bool has_begon = false;
   bool has_signedness = false;
   vt->type = NUM_VALS;
   vt->integer.is_unsigned = false;
   vt->is_const = false;
   


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

   if (lexer_matches(KW_ENUM)) {
      vt->type = VAL_ENUM;
      if (has_begon) lexer_next();
      else vt->begin = lexer_next().begin;
      vt->venum = malloc(sizeof(struct enumeration));
      if (!vt->venum) panic("parse_value_type(): failed to allocate enum");
      if (lexer_matches(TK_NAME)) {
         const struct token tk = lexer_next();
         vt->venum->name = tk.str;
         vt->end = tk.end;
      } else vt->venum->name = NULL;
      vt->venum->entries = NULL;
      vt->venum->is_definition = lexer_match(TK_CLPAREN);
      if (vt->venum->is_definition) {
         intmax_t counter = 0;
         do {
            if (lexer_matches(TK_CRPAREN)) break;
            struct enum_entry e;
            e.name = lexer_expect(TK_NAME).str;
            if (lexer_match(TK_EQ)) {
               struct value val = parse_const_expr();
               if (val.type->type != VAL_INT)
                  parse_error(&val.begin, "expected integer value");
               e.value = val.iVal;
               counter = e.value + 1;
            } else e.value = counter++;
            buf_push(vt->venum->entries, e);
         } while (lexer_match(TK_COMMA));
         vt->end = lexer_expect(TK_CRPAREN).end;
      } else {
         if (!vt->venum->name)
            parse_error(&vt->end, "expected name");
      }
   } else if (lexer_matches(KW_STRUCT) || lexer_matches(KW_UNION)) {
      const struct token tk_struct = lexer_next();
      vt->type = tk_struct.type == KW_UNION ? VAL_UNION : VAL_STRUCT;
      if (!has_begon) vt->begin = tk_struct.begin;
      vt->vstruct = malloc(sizeof(struct structure));
      if (!vt->vstruct) panic("parse_value_type(): failed to allocate struct");
      if (lexer_matches(TK_NAME)) {
         const struct token tk = lexer_next();
         vt->vstruct->name = tk.str;
         vt->end = tk.end;
      } else vt->vstruct->name = NULL;
      vt->vstruct->entries = NULL;
      vt->vstruct->is_definition = lexer_match(TK_CLPAREN);
      if (vt->vstruct->is_definition && !lexer_match(TK_CRPAREN)) {
         do {
            struct value_type* tt = parse_value_type(scope);
            do {
               struct struct_entry e;
               e.type = copy_value_type(tt);
               e.name = lexer_expect(TK_NAME).str;
               buf_push(vt->vstruct->entries, e);
            } while (lexer_match(TK_COMMA));
            lexer_expect(TK_SEMICOLON);
            free_value_type(tt);
         } while (!lexer_match(TK_CRPAREN));
      } else if (!vt->vstruct->name)
         parse_error(&vt->end, "expected name");
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
   case KW_VOID:
      lexer_skip();
      if (vt->type != NUM_VALS) parse_error(&tk.begin, "invalid combination of signed or unsigned and void");
      vt->type = VAL_VOID;
      break;
   case KW_AUTO:
      lexer_skip();
      if (vt->type != NUM_VALS) parse_error(&tk.begin, "invalid combination of signed or unsigned and void");
      vt->type = VAL_AUTO;
      break;
   case TK_NAME:
   {
      if (vt->type != NUM_VALS) return vt;
      else if (!has_begon && scope && var_is_declared(tk.str, scope)) return NULL;
      struct typerename* td = unit_get_typedef(tk.str);
      if (!td) return NULL;
      lexer_skip();
      free_value_type(vt);
      vt = copy_value_type(td->type);
      break;
   }
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
      ptr->pointer.is_array = false;
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
      // TODO: improve warnings
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

#define check1or(a, b, c) (((a)->type == (c)) || ((b)->type == (c)))
#define check1and(a, b, c) (((a)->type == (c)) && ((b)->type == (c)))
#define check2(a, b, c, d) ((((a)->type == (c)) && ((b)->type == (d))) || (((a)->type == (d)) && ((b)->type == (c))))

struct value_type* decay(struct value_type* vt) {
   if (vt->type == VAL_POINTER && vt->pointer.is_array) {
      vt->pointer.is_array = false;
   } else if (vt->type == VAL_ENUM) {
      vt->type = VAL_INT;
      vt->integer.is_unsigned = false;
      vt->integer.size = INT_INT;
   }
   return vt;
}

struct value_type* get_value_type_impl(struct scope* scope, struct expression* e) {
   struct value_type* type;
   switch (e->type) {
   case EXPR_PAREN:
      return copy_value_type(get_value_type(scope, e->expr));
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
   case EXPR_ARRAYLEN:
   case EXPR_SIZEOF:
      type = new_vt();
      type->type = VAL_INT;
      type->is_const = true;
      type->integer.is_unsigned = true;
      type->integer.size = INT_INT;
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
      if (!var) var = func_find_param(scope->func, e->str);
      if (!var) var = unit_get_var(e->str);
      if (find_constant(e->str, NULL)) return make_int(INT_INT, false);
      if (!var) parse_error(&e->begin, "undeclared variable '%s'", e->str);
      return copy_value_type(var->type);
   }
   case EXPR_ADDROF:
   {
      const struct value_type* ve = get_value_type(scope, e->expr);
      if (ve->type == VAL_POINTER && ve->pointer.is_array)
         return decay(copy_value_type(ve));
      type = new_vt();
      type->type = VAL_POINTER;
      type->is_const = true;
      type->pointer.type = copy_value_type(ve);
      return type;
   }
   case EXPR_INDIRECT: {
      const struct value_type* tmp = get_value_type(scope, e->expr);
      if (tmp->type != VAL_POINTER) parse_error(&e->expr->begin, "cannot dereference a non-pointer");
      type = copy_value_type(tmp->pointer.type);
      return type;
   }
   case EXPR_STRING:
      type = new_vt();
      type->type = VAL_POINTER;
      type->is_const = true;
      type->pointer.type = make_int(INT_CHAR, target_info.unsigned_char);
      return type;
   case EXPR_COMMA:
      return copy_value_type(get_value_type(scope, e->comma[buf_len(e->comma) - 1]));
   case EXPR_SUFFIX:
      type = copy_value_type(get_value_type(scope, e->unary.expr));
      if (type->is_const)
         parse_error(&e->begin, "assignment to const");
      type->is_const = true;
      return type;
   case EXPR_ASSIGN:
   {
      const struct value_type* vl = get_value_type(scope, e->assign.left);
      const struct value_type* vr = get_value_type(scope, e->assign.right);
      if (vl->is_const)
         parse_error(&e->begin, "assignment to const");
      else if (!is_castable(vr, vl, true))
         parse_error(&e->begin, "incompatible types");
      return copy_value_type(vl);
   }
   case EXPR_BINARY:
   {
      const struct value_type* vl = get_value_type(scope, e->binary.left);
      const struct value_type* vr = get_value_type(scope, e->binary.right);
      if (check1or(vl, vr, VAL_VOID))
         parse_error(&e->binary.op.begin, "binary operation on void");
      switch (e->binary.op.type) {
      case TK_EQEQ:
      case TK_NEQ:
      case TK_GR:
      case TK_GREQ:
      case TK_LE:
      case TK_LEEQ:
         if (vl->type != vr->type) {
            if (check2(vl, vr, VAL_POINTER, VAL_FLOAT))
               parse_error(&e->binary.op.begin, "comparisson between pointer and floating-point");
            else if (check2(vl, vr, VAL_POINTER, VAL_INT))
               parse_warn(&e->binary.op.begin, "comparisson between pointer and integer");
         } else if (check1and(vl, vr, VAL_POINTER)) {
            if (!ptreq(vl, vr))
               parse_error(&e->binary.op.begin, "comparisson between pointer of different types");
         }
         return make_int(INT_INT, false);
      case TK_AMPAMP:
      case TK_PIPI:
         return make_int(INT_INT, false);
      case TK_PLUS:
         if (check1and(vl, vr, VAL_POINTER))
            parse_error(&e->binary.op.begin, "addition of pointers");
         switch (vl->type) {
         case VAL_INT:
            if (vr->type == VAL_POINTER) return copy_value_type(vr);
            else return common_value_type(vl, vr, true);
         case VAL_FLOAT:
            if (vr->type == VAL_POINTER)
               parse_error(&e->binary.op.begin, "addition of pointer and floating-point number");
            else return common_value_type(vl, vr, true);
         case VAL_POINTER:
            if (vr->type == VAL_INT) return decay(copy_value_type(vl));
            else if (vr->type == VAL_FLOAT)
               parse_error(&e->binary.op.begin, "addition of pointer and floating-point number");
            panic("get_value_type(): addition of pointer and '%s'", value_type_str[vr->type]);
         default:
            panic("get_value_type(): unsupported value type '%s'", value_type_str[vl->type]);
         }
      case TK_MINUS:
         switch (vl->type) {
         case VAL_INT:
            if (vr->type == VAL_POINTER)
               parse_error(&e->binary.op.begin, "invalid types");
            else return common_value_type(vl, vr, true);
         case VAL_FLOAT:
            if (vr->type == VAL_POINTER)
               parse_error(&e->binary.op.begin, "invalid types");
            else return common_value_type(vl, vr, true);
         case VAL_POINTER:
            if (vr->type == VAL_FLOAT)
               parse_error(&e->binary.op.begin, "invalid types");
            else if (vr->type == VAL_INT) return decay(copy_value_type(vl));
            else if (vr->type == VAL_POINTER) {
               if (!ptreq(vl->pointer.type, vr->pointer.type))
                  parse_error(&e->binary.op.begin, "incompatible pointer types");
               return make_int(target_info.ptrdiff_type, false);
            }
            else panic("get_value_type(): unsupported value type '%s'", value_type_str[vr->type]);
         default:
            panic("get_value_type(): unsupported value type '%s'", value_type_str[vl->type]);
         }
      case TK_STAR:
      case TK_SLASH:
      case TK_PERC:
         if (check1or(vl, vr, VAL_POINTER))
            parse_error(&e->binary.op.begin, "invalid use of pointer");
         else return common_value_type(vl, vr, true);
      case TK_AMP:
      case TK_PIPE:
      case TK_XOR:
         if (check1or(vl, vr, VAL_FLOAT))
            parse_error(&e->binary.op.begin, "bitwise operation on floating-point number");
         else if (check1or(vl, vr, VAL_POINTER))
            parse_error(&e->binary.op.begin, "bitwise operation on pointer");
         else return common_value_type(vl, vr, true);

      default:
         panic("get_value_type(): binary operator '%s' not implemented", token_type_str[e->binary.op.type]);
      }
      panic("get_value_type(): reached unreachable");
   }
      return common_value_type(get_value_type(scope, e->binary.left), get_value_type(scope, e->binary.right), true); // TODO
   case EXPR_TERNARY:
      return common_value_type(get_value_type(scope, e->ternary.true_case), get_value_type(scope, e->ternary.false_case), true);
   case EXPR_PREFIX:
   case EXPR_UNARY:
      type = copy_value_type(get_value_type(scope, e->unary.expr));
      if (e->type == EXPR_PREFIX && type->is_const)
         parse_error(&e->begin, "assignment to const");
      if (e->unary.op.type == TK_MINUS && type->type == VAL_INT && type->integer.is_unsigned)
         parse_warn(&e->begin, "negating an unsigned integer");
      type->is_const = true;
      return type;
   case EXPR_CAST:
   {
      const struct value_type* old = get_value_type(scope, e->cast.expr);
      if (!is_castable(old, e->cast.type, false))
         parse_error(&e->begin, "invalid cast");
      return copy_value_type(e->cast.type);
   }
   case EXPR_FCALL:
   {
      if (e->fcall.name == scope->func->name)
         return copy_value_type(scope->func->type);
      if (is_builtin_func(e->fcall.name))
         parse_warn(&e->begin, "'%s' is a compiler-specific builtin-function.", e->fcall.name);
      struct function* callee = unit_get_func(e->fcall.name);
      if (!callee) {
         parse_warn(&e->begin, "function '%s' is not declared.", e->fcall.name);
         return make_int(INT_INT, false);
      }
      const size_t num_callee_params = buf_len(callee->params);
      const size_t num_params = buf_len(e->fcall.params);
      if (num_callee_params != num_params) {
         if (callee->variadic && num_params < num_callee_params)
            parse_warn(&e->begin, "not enough parameters for %s()", callee->name);
         else if (!callee->variadic) parse_warn(&e->begin, "invalid number of parameters");
      }
      for (size_t i = 0; i < my_min(num_params, num_callee_params); ++i) {
         const struct value_type* vp = get_value_type(scope, e->fcall.params[i]);
         if (!is_castable(vp, callee->params[i].type, true))
            parse_error(&e->begin, "invalid type of parameter %zu", i);
      }
      return copy_value_type(callee->type);
   }
   case EXPR_MEMBER:
   {
      const struct value_type* base = get_value_type(scope, e->member.base);
      if (base->type != VAL_STRUCT && base->type != VAL_UNION)
         parse_error(&base->begin, "is not a struct");
      struct structure* st = real_struct(base->vstruct, base->type == VAL_UNION);
      struct struct_entry* m = struct_get_member(st, e->member.name);
      if (m) return copy_value_type(m->type);
      else parse_error(&e->begin, "'struct %s' has no member '%s'", st->name, e->member.name);
   }
   default: panic("get_value_type(): unsupported expression '%s'", expr_type_str[e->type]);
   }
}
const struct value_type* get_value_type(struct scope* scope, struct expression* e) {
   if (!e->vtype) e->vtype = get_value_type_impl(scope, e);
   return e->vtype;
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
      copy->pointer.is_array = vt->pointer.is_array;
      if (vt->pointer.is_array) {
         copy->pointer.array.has_const_size = vt->pointer.array.has_const_size;
         if (copy->pointer.array.has_const_size)
            copy->pointer.array.size = vt->pointer.array.size;
         else copy->pointer.array.dsize = vt->pointer.array.dsize;
      }
      break;
   case VAL_ENUM:
      copy->venum = copy_enum(vt->venum);
      break;
   case VAL_STRUCT:
   case VAL_UNION:
      copy->vstruct = copy_struct(vt->vstruct);
      break;
   case VAL_VOID:
   case VAL_AUTO:
      break;
   default: panic("copy_value_type(): unsupported value type '%d'", vt->type);
   }
   return copy;
}

#define warn_implicit(f, t) parse_warn(&old->begin, "implicit conversion from %s to %s", f, t)


bool is_castable(const struct value_type* old, const struct value_type* type, bool implicit) {
   if (old->type == VAL_VOID || type->type == VAL_VOID) return false;
   switch (old->type) {
   case VAL_INT:
      switch (type->type) {
      case VAL_INT:
      case VAL_FLOAT:
      case VAL_ENUM:
         return true;
      case VAL_POINTER:
         return !implicit;
      default: panic("is_castable(): invalid value type '%u'", type->type);
      }
   case VAL_FLOAT:
      switch (type->type) {
      case VAL_INT:
         if (implicit) warn_implicit(fp_size_str[old->fp.size], integer_size_str[type->integer.size]);
         fallthrough;
      case VAL_FLOAT:
         return true;
      case VAL_ENUM:
         return !implicit;
      case VAL_POINTER:
         return false;
      default: panic("is_castable(): invalid value type '%u'", type->type);
      }
   case VAL_POINTER:
      switch (type->type) {
      case VAL_INT:
      case VAL_ENUM:
         return !implicit;
      case VAL_POINTER:
         if (old->pointer.type->is_const && !type->pointer.type->is_const && implicit)
            warn_implicit("const pointer", "non-const pointer");
         if (old->pointer.type->type == VAL_VOID || type->pointer.type->type == VAL_VOID) return true;
         if (!ptreq(old->pointer.type, type->pointer.type) && implicit)
            parse_warn(&old->begin, "implicit pointer conversion");
         return true;
      default: panic("is_castable(): invalid value type '%u'", type->type);
      }
   case VAL_ENUM:
      switch (type->type) {
      case VAL_ENUM:
      case VAL_INT:
      case VAL_FLOAT:
         return true;
      case VAL_POINTER:
         return !implicit;
      default: panic("is_castable(): invalid value type '%u'", type->type);
      }
   default: panic("is_castable(): invalid value type '%u'", type->type);
   }
}
struct value_type* make_array_vt(struct value_type* vt) {
   struct value_type* arr = new_vt();
   arr->type = VAL_POINTER;
   arr->begin = vt->begin;
   arr->pointer.type = vt;
   arr->pointer.is_array = true;
   arr->is_const = true;
   return arr;
}
size_t sizeof_value(const struct value_type* vt, bool decay) {
   switch (vt->type) {
   case VAL_POINTER:
      if (vt->pointer.is_array && vt->pointer.array.has_const_size && !decay)
         return sizeof_value(vt->pointer.type, false) * vt->pointer.array.size;
      else return target_info.size_pointer;
   case VAL_FLOAT:
      switch (vt->fp.size) {
      case FP_FLOAT:
         return target_info.size_float;
      case FP_DOUBLE:
         return target_info.size_double;
      default:
         panic("sizeof_value(): invalid floating-point size '%s'", fp_size_str[vt->fp.size]);
      }
   case VAL_INT:
      switch (vt->integer.size) {
      case INT_BYTE:
         return target_info.size_byte;
      case INT_CHAR:
         return target_info.size_char;
      case INT_SHORT:
         return target_info.size_short;
      case INT_INT:
         return target_info.size_int;
      case INT_LONG:
         return target_info.size_long;
      default:
         panic("sizeof_value(): invalid integer size '%s'", integer_size_str[vt->integer.size]);
      }
   case VAL_ENUM:
      return target_info.size_int;
   case VAL_STRUCT:
   {
      struct structure* s;
      if (vt->vstruct->is_definition)
         s = vt->vstruct;
      else s = unit_get_struct(vt->vstruct->name);
      if (!s)
         parse_error(&vt->begin, "failed to find 'struct %s'", vt->vstruct->name);
      size_t sz = 0;
      for (size_t i = 0; i < buf_len(s->entries); ++i) {
         sz += sizeof_value(s->entries[i].type, false);
      }
      return sz;
   }
   case VAL_UNION:
   {
      struct structure* s;
      if (vt->vstruct->is_definition)
         s = vt->vstruct;
      else s = unit_get_union(vt->vstruct->name);
      if (!s)
         parse_error(&vt->begin, "failed to find 'struct %s'", vt->vstruct->name);
      size_t sz = 0;
      for (size_t i = 0; i < buf_len(s->entries); ++i) {
         const size_t tmp = sizeof_value(s->entries[i].type, false);
         if (tmp > sz) sz = tmp;
      }
      return sz;
   }
   default:
      panic("sizeof_value(): invalid value type '%s'", value_type_str[vt->type]);
   }
}
struct enumeration* copy_enum(const struct enumeration* e) {
   struct enumeration* ne = malloc(sizeof(struct enumeration));
   if (!ne) panic("copy_enum(): failed to allocate enum");
   ne->name = e->name;
   ne->entries = NULL;
   if ((ne->is_definition = e->is_definition)) {
      for (size_t i = 0; i < buf_len(e->entries); ++i)
         buf_push(ne->entries, e->entries[i]);
   }
   return ne;
}
struct structure* copy_struct(const struct structure* s) {
   struct structure* ns = malloc(sizeof(struct structure));
   if (!ns) panic("copy_struct(): failed to allocate struct");
   ns->name = s->name;
   ns->entries = NULL;
   if ((ns->is_definition = s->is_definition)) {
      for (size_t i = 0; i < buf_len(s->entries); ++i) {
         struct struct_entry e;
         e.type = copy_value_type(s->entries[i].type);
         e.name = s->entries[i].name;
         buf_push(ns->entries, e);
       }
   }
   return ns;
}
struct struct_entry* struct_get_member(struct structure* st, istr_t name) {
   for (size_t i = 0; i < buf_len(st->entries); ++i) {
      if (name == st->entries[i].name)
         return &st->entries[i];
   }
   return NULL;
}
size_t struct_get_member_idx(struct structure* st, istr_t name) {
   for (size_t i = 0; i < buf_len(st->entries); ++i) {
      if (name == st->entries[i].name)
         return i;
   }
   return SIZE_MAX;
}
size_t addrof_member(struct structure* st, size_t idx) {
   size_t sz = 0;
   for (size_t i = 0; i < idx; ++i) {
      sz += sizeof_value(st->entries[i].type, false);
   }
   return sz;
}
struct structure* real_struct(struct structure* st, bool is_union) {
   return st->is_definition ? st : (is_union ? unit_get_union(st->name) : unit_get_struct(st->name));
}

