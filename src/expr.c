#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "parser.h"
#include "error.h"
#include "optim.h"
#include "lex.h"

const char* expr_type_str[NUM_EXPRS] = {
   [EXPR_PAREN]      = "sub",
   [EXPR_INT]        = "signed integer",
   [EXPR_UINT]       = "unsigned integer",
#if !DISABLE_FP
   [EXPR_FLOAT]      = "floating-point",
#endif
   [EXPR_CHAR]       = "character-literal",
   [EXPR_STRING]     = "string-literal",
   [EXPR_NAME]       = "identifier",
   [EXPR_UNARY]      = "unary",
   [EXPR_BINARY]     = "binary",
   [EXPR_PREFIX]     = "prefix",
   [EXPR_SUFFIX]     = "suffix",
   [EXPR_ADDROF]     = "address-of",
   [EXPR_INDIRECT]   = "indirection",
   [EXPR_TERNARY]    = "ternary",
   [EXPR_ASSIGN]     = "assignment",
   [EXPR_COMMA]      = "comma",
   [EXPR_CAST]       = "cast",
   [EXPR_FCALL]      = "function call",
   [EXPR_SIZEOF]     = "sizeof",
   [EXPR_ARRAYLEN]   = "arraylen",
   [EXPR_MEMBER]     = "member",
};

struct expression* new_expr(void) {
   struct expression* expr = malloc(sizeof(struct expression));
   if (!expr) panic("failed to allocate expression");
   expr->vtype = NULL;
   return expr;
}

static struct scope* scope;

static struct expression* expr_comma(void);
static struct expression* expr_assign(void);
static struct expression* expr_unary(void);
static struct expression* expr_prim(void) {
   struct token tk = lexer_next();
   struct expression* expr = new_expr();
   expr->begin = tk.begin;
   expr->end = tk.end;
   switch (tk.type) {
   case TK_INTEGER:
      if (tk.iVal > INTMAX_MAX) {
         expr->type = EXPR_UINT;
         expr->uVal = tk.iVal;
      } else {
         expr->type = EXPR_INT;
         expr->iVal = (intmax_t)tk.iVal;
      }
      break;
   case TK_STRING:
      expr->type = EXPR_STRING;
      expr->str = tk.str;
      while (lexer_matches(TK_STRING)) {
         const struct token stk = lexer_next();
         const size_t len_orig = strlen(expr->str), len_new = strlen(stk.str);
         char buffer[len_orig + len_new + 1];
         memcpy(buffer, expr->str, len_orig);
         memcpy(buffer + len_orig, stk.str, len_new);
         buffer[len_orig + len_new] = '\0';
         expr->str = strint(buffer);
      }
      break;
   case TK_CHARACTER:
      expr->type = EXPR_CHAR;
      expr->ch = tk.ch;
      break;
   case TK_NAME:
      expr->type = EXPR_NAME;
      expr->str = tk.str;
      break;
   case TK_LPAREN:
      expr->cast.type = parse_value_type(scope);
      if (expr->cast.type) {
         if (expr->cast.type->type == VAL_VOID)
            parse_error(&expr->begin, "cast to incomplete type void");
         expr->type = EXPR_CAST;
         lexer_expect(TK_RPAREN);
         expr->cast.expr = expr_unary();
         expr->end = expr->cast.expr->end;
      } else {
         expr->type = EXPR_PAREN;
         expr->expr = expr_comma();
         lexer_expect(TK_RPAREN);
      }
      break;
   case KW_SIZEOF:
      expr->type = EXPR_SIZEOF;
      if (lexer_match(TK_LPAREN)) {
         struct value_type* vt = parse_value_type(scope);
         expr->szof.has_expr = vt == NULL;
         if (!vt) expr->szof.expr = expr_unary();
         else expr->szof.type = vt;
         expr->end = lexer_expect(TK_RPAREN).end;
      } else {
         expr->szof.has_expr = true;
         expr->szof.expr = expr_unary();
      }
      break;
   case KW_ARRAYLEN:
      expr->type = EXPR_ARRAYLEN;
      expr->expr = expr_unary();
      expr->end = expr->expr->end;
      break;
   default:
      parse_error(&tk.begin, "expected expression, got %s\n", token_type_str[tk.type]);
      break;
   }
   

   while (lexer_matches(TK_PLPL) || lexer_matches(TK_MIMI)
         || lexer_matches(TK_LBRACK) || lexer_matches(TK_DOT)
         || lexer_matches(TK_ARROW)
         || lexer_matches(TK_LPAREN)) {
      if (!expr_is_lvalue(expr))
         parse_error(&expr->begin, "expected lvalue");
      struct expression* tmp = new_expr();
      tk = lexer_next();
      tmp->begin = expr->begin;
      switch (tk.type) {
      case TK_LPAREN:
      {
         tmp->type = EXPR_FCALL;
         tmp->fcall.func = expr;
         tmp->fcall.params = NULL;
         if (!lexer_matches(TK_RPAREN)) {
            do {
               buf_push(tmp->fcall.params, optim_expr(expr_assign()));
            } while (lexer_match(TK_COMMA));
         }
         tmp->end = lexer_expect(TK_RPAREN).end;
         break;
      }
      case TK_LBRACK:
      {
         struct expression* add = new_expr();
         add->type = EXPR_BINARY;
         add->binary.op.type = TK_PLUS;
         add->binary.left = expr;
         add->binary.right = optim_expr(expr_comma());
         add->begin = expr->begin;
         add->end = lexer_expect(TK_RBRACK).end;
         
         tmp->type = EXPR_INDIRECT;
         tmp->expr = add;
         tmp->end = add->end;
         break;
      }
      case TK_PLPL:
      case TK_MIMI:
         tmp->type = EXPR_SUFFIX;
         tmp->unary.op = tk;
         tmp->unary.expr = expr;
         tmp->end = tmp->unary.op.begin;
         break;
      case TK_DOT:
      {
         const struct token name = lexer_expect(TK_NAME);
         tmp->type = EXPR_MEMBER;
         tmp->member.base = expr;
         tmp->member.name = name.str;
         tmp->end = name.end;
         const struct value_type* bt = get_value_type(scope, tmp->member.base);
         if (bt->type != VAL_STRUCT && bt->type != VAL_UNION)
            parse_error(&tmp->begin, "member expressions only apply to structs");
         struct structure* st = real_struct(bt->vstruct, bt->type == VAL_UNION);
         if (!st) parse_error(&tmp->begin, "%s '%s' not defined", value_type_str[bt->type], bt->vstruct->name);
         if (!struct_get_member(st, name.str))
            parse_error(&name.begin, "'struct %s' has no member '%s'", st->name, name.str);
         break;
      }
      case TK_ARROW:
      {
         const struct token name = lexer_expect(TK_NAME);
         struct expression* sub = new_expr();
         sub->type = EXPR_INDIRECT;
         sub->begin = expr->begin;
         sub->end = name.end;
         sub->expr = expr;

         tmp->type = EXPR_MEMBER;
         tmp->member.base = sub;
         tmp->member.name = name.str;
         
         const struct value_type* bt = get_value_type(scope, tmp->member.base);
         if (bt->type != VAL_STRUCT && bt->type != VAL_UNION)
            parse_error(&tmp->begin, "arrow expressions only apply to pointer to struct");
      
         struct structure* st = real_struct(bt->vstruct, bt->type == VAL_UNION);
         if (!struct_get_member(st, name.str))
            parse_error(&name.begin, "'struct %s' has no member '%s'", st->name, name.str);
         break;
      }
      default:
         panic("invalid token '%s'", token_type_str[tk.type]);
      }
      expr = tmp;
   }

   return optim_expr(expr);
}

static struct expression* expr_unary(void) {
   struct expression* expr = NULL;
   if (lexer_matches(TK_PLUS) || lexer_matches(TK_MINUS)
    || lexer_matches(TK_NOT) || lexer_matches(TK_WAVE)) {
      expr = new_expr();
      expr->type = EXPR_UNARY;
      expr->unary.op = lexer_next();
      expr->unary.expr = expr_unary();
      expr->begin = expr->unary.op.begin;
      expr->end = expr->unary.expr->end;
   } else if (lexer_matches(TK_PLPL) || lexer_matches(TK_MIMI)) {
      expr = new_expr();
      expr->type = EXPR_PREFIX;
      expr->unary.op = lexer_next();
      expr->unary.expr = expr_unary();
      expr->begin = expr->unary.op.begin;
      expr->end = expr->unary.expr->end;
      if (!expr_is_lvalue(expr->unary.expr))
         parse_error(&expr->begin, "expected lvalue, got %s", expr_type_str[expr->expr->type]);
   } else if (lexer_matches(TK_AMP)) {
      expr = new_expr();
      expr->type = EXPR_ADDROF;
      expr->begin = lexer_next().begin;
      expr->expr = expr_unary();
      expr->end = expr->expr->end;
      if (!expr_is_lvalue(expr->expr))
         parse_error(&expr->begin, "expected lvalue, got %s", expr_type_str[expr->expr->type]);
   } else if (lexer_matches(TK_STAR)) {
      expr = new_expr();
      expr->type = EXPR_INDIRECT;
      expr->begin = lexer_next().begin;
      expr->expr = expr_unary();
      expr->end = expr->expr->end;
   }


   if (!expr) expr = expr_prim();

   return expr;
}

static struct expression* expr_mul(void) {
   struct expression* left = expr_unary();
   while (lexer_matches(TK_STAR) || lexer_matches(TK_SLASH) || lexer_matches(TK_PERC)) {
      struct expression* expr = new_expr();
      expr->type = EXPR_BINARY;
      expr->begin = left->begin;
      expr->binary.left = left;
      expr->binary.op = lexer_next();
      expr->binary.right = expr_unary();
      expr->end = expr->binary.right->end;
      left = expr;
   }
   return left;
}
static struct expression* expr_add(void) {
   struct expression* left = expr_mul();
   while (lexer_matches(TK_PLUS) || lexer_matches(TK_MINUS)) {
      struct expression* expr = new_expr();
      expr->type = EXPR_BINARY;
      expr->begin = left->begin;
      expr->binary.left = left;
      expr->binary.op = lexer_next();
      expr->binary.right = expr_mul();
      expr->end = expr->binary.right->end;
      left = expr;
   }
   return left;
}
static struct expression* expr_shift(void) {
   struct expression* left = expr_add();
   while (lexer_matches(TK_GRGR) || lexer_matches(TK_LELE)) {
      struct expression* expr = new_expr();
      expr->type = EXPR_BINARY;
      expr->begin = left->begin;
      expr->binary.left = left;
      expr->binary.op = lexer_next();
      expr->binary.right = expr_add();
      expr->end = expr->binary.right->end;
      left = expr;
   }
   return left;
}
static struct expression* expr_cmp(void) {
   struct expression* left = expr_shift();
   while (lexer_matches(TK_GR) || lexer_matches(TK_GREQ)
       || lexer_matches(TK_LE) || lexer_matches(TK_LEEQ)) {
      struct expression* expr = new_expr();
      expr->type = EXPR_BINARY;
      expr->begin = left->begin;
      expr->binary.left = left;
      expr->binary.op = lexer_next();
      expr->binary.right = expr_shift();
      expr->end = expr->binary.right->end;
      left = expr;
   }
   return left;
}
static struct expression* expr_eq(void) {
   struct expression* left = expr_cmp();
   while (lexer_matches(TK_EQEQ) || lexer_matches(TK_NEQ)) {
      struct expression* expr = new_expr();
      expr->type = EXPR_BINARY;
      expr->begin = left->begin;
      expr->binary.left = left;
      expr->binary.op = lexer_next();
      expr->binary.right = expr_cmp();
      expr->end = expr->binary.right->end;
      left = expr;
   }
   return left;
}
static struct expression* expr_bitand(void) {
   struct expression* left = expr_eq();
   while (lexer_matches(TK_AMP)) {
      struct expression* expr = new_expr();
      expr->type = EXPR_BINARY;
      expr->begin = left->begin;
      expr->binary.left = left;
      expr->binary.op = lexer_next();
      expr->binary.right = expr_eq();
      expr->end = expr->binary.right->end;
      left = expr;
   }
   return left;
}
static struct expression* expr_bitor(void) {
   struct expression* left = expr_bitand();
   while (lexer_matches(TK_PIPE)) {
      struct expression* expr = new_expr();
      expr->type = EXPR_BINARY;
      expr->begin = left->begin;
      expr->binary.left = left;
      expr->binary.op = lexer_next();
      expr->binary.right = expr_bitand();
      expr->end = expr->binary.right->end;
      left = expr;
   }
   return left;
}
static struct expression* expr_bitxor(void) {
   struct expression* left = expr_bitor();
   while (lexer_matches(TK_XOR)) {
      struct expression* expr = new_expr();
      expr->type = EXPR_BINARY;
      expr->begin = left->begin;
      expr->binary.left = left;
      expr->binary.op = lexer_next();
      expr->binary.right = expr_bitor();
      expr->end = expr->binary.right->end;
      left = expr;
   }
   return left;
}
static struct expression* expr_and(void) {
   struct expression* left = expr_bitxor();
   while (lexer_matches(TK_AMPAMP)) {
      struct expression* expr = new_expr();
      expr->type = EXPR_BINARY;
      expr->begin = left->begin;
      expr->binary.left = left;
      expr->binary.op = lexer_next();
      expr->binary.right = expr_bitxor();
      expr->end = expr->binary.right->end;
      left = expr;
   }
   return left;
}
static struct expression* expr_or(void) {
   struct expression* left = expr_and();
   while (lexer_matches(TK_PIPI)) {
      struct expression* expr = new_expr();
      expr->type = EXPR_BINARY;
      expr->begin = left->begin;
      expr->binary.left = left;
      expr->binary.op = lexer_next();
      expr->binary.right = expr_or();
      expr->end = expr->binary.right->end;
      left = expr;
   }
   return left;
}
static struct expression* expr_ternary(void) {
   struct expression* expr = expr_or();
   if (lexer_match(TK_QMARK)) {
      struct expression* tmp = new_expr();
      tmp->type = EXPR_TERNARY;
      tmp->begin = expr->begin;
      tmp->ternary.cond = expr;
      get_value_type(scope, tmp->ternary.cond);
      tmp->ternary.true_case = expr_or();
      lexer_expect(TK_COLON);
      tmp->ternary.false_case = expr_ternary();
      tmp->end = tmp->ternary.false_case->end;
      expr = tmp;
   }

   return expr;
}
static struct expression* expr_assign(void) {
   struct expression* left = expr_ternary();
   if (lexer_matches(TK_EQ)
    || lexer_matches(TK_PLEQ) || lexer_matches(TK_MIEQ)
    || lexer_matches(TK_STEQ) || lexer_matches(TK_SLEQ) || lexer_matches(TK_PERCEQ)
    || lexer_matches(TK_GRGREQ) || lexer_matches(TK_LELEEQ)
    || lexer_matches(TK_AMPEQ) || lexer_matches(TK_PIPEEQ) || lexer_matches(TK_XOREQ)) {
      if (!expr_is_lvalue(left)) parse_error(&left->begin, "expected lvalue, got %s", expr_type_str[left->type]);
      const struct token op = lexer_next();
      struct expression* expr = new_expr();
      expr->type = EXPR_ASSIGN;
      expr->begin = left->begin;
      expr->assign.left = left;
      struct expression* right = expr_assign();
      if (op.type == TK_EQ) {
         expr->assign.right = right;
      } else {
         struct expression* tmp = new_expr();
         tmp->type = EXPR_BINARY;
         tmp->begin = left->begin;
         tmp->end = right->end;
         enum token_type type;
         switch (op.type) {
         case TK_PLEQ:     type = TK_PLUS;   break;
         case TK_MIEQ:     type = TK_MINUS;  break;
         case TK_STEQ:     type = TK_STAR;   break;
         case TK_SLEQ:     type = TK_SLASH;  break;
         case TK_PERCEQ:   type = TK_PERC;   break;
         case TK_AMPEQ:    type = TK_AMP;    break;
         case TK_PIPEEQ:   type = TK_PIPE;   break;
         case TK_XOREQ:    type = TK_XOR;    break;
         case TK_GRGREQ:   type = TK_GRGR;   break;
         case TK_LELEEQ:   type = TK_LELE;   break;
         default:          parse_error(&op.begin, "invalid operator");
         }
         tmp->binary.op.type = type;
         tmp->binary.left = left;
         tmp->binary.right = right;
         expr->assign.right = tmp;
      }
      expr->end = right->end;
      left = expr;
   }
   return left;
}

static struct expression* expr_comma(void) {
   struct expression* expr = expr_assign();
   if (!lexer_match(TK_COMMA)) return expr;
   struct expression* comma = new_expr();
   comma->type = EXPR_COMMA;
   comma->begin = expr->begin;
   comma->comma = NULL;
   get_value_type(scope, expr);
   buf_push(comma->comma, expr);
   do {
      struct expression* e = expr_assign();
      get_value_type(scope, e);
      buf_push(comma->comma, e);
   } while (lexer_match(TK_COMMA));
   return comma;
}

struct expression* parse_expr(struct scope* s) {
   scope = s;
   struct expression* e = expr_comma();
   get_value_type(s, e);
   return optim_expr(e);
}
struct expression* parse_expr_no_comma(struct scope* s) {
   scope = s;
   struct expression* e = expr_assign();
   get_value_type(s, e);
   return optim_expr(e);
}


void free_expr(struct expression* e) {
   switch (e->type) {
   case EXPR_ADDROF:
   case EXPR_INDIRECT:
   case EXPR_ARRAYLEN:
   case EXPR_PAREN:  free_expr(e->expr); break;
   case EXPR_PREFIX:
   case EXPR_SUFFIX:
   case EXPR_UNARY:  free_expr(e->unary.expr); break;
   case EXPR_ASSIGN:
      if (e->assign.right->type != EXPR_BINARY || e->assign.right->binary.left != e->assign.left)
         free_expr(e->assign.left);
      free_expr(e->assign.right);
      break;
   case EXPR_BINARY:
      free_expr(e->binary.left);
      free_expr(e->binary.right);
      break;
   case EXPR_TERNARY:
      free_expr(e->ternary.cond);
      free_expr(e->ternary.true_case);
      free_expr(e->ternary.false_case);
      break;
   case EXPR_COMMA:
      for (size_t i = 0; i < buf_len(e->comma); ++i)
         free_expr(e->comma[i]);
      buf_free(e->comma);
      break;
   case EXPR_CAST:
      free_value_type(e->cast.type);
      free_expr(e->cast.expr);
      break;
   case EXPR_FCALL:
      for (size_t i = 0; i < buf_len(e->fcall.params); ++i)
         free_expr(e->fcall.params[i]);
      buf_free(e->fcall.params);
      break;
   case EXPR_SIZEOF:
      if (e->szof.has_expr) free_expr(e->szof.expr);
      else free_value_type(e->szof.type);
   case EXPR_MEMBER:
   case EXPR_INT:
   case EXPR_UINT:
   case EXPR_STRING:
   case EXPR_CHAR:
#if !DISABLE_FP
   case EXPR_FLOAT:
#endif
   case EXPR_NAME:
   case NUM_EXPRS:   break;
   }
   if (e->vtype) free_value_type(e->vtype);
   free(e);
}

void print_expr(FILE* file, const struct expression* e) {
   switch (e->type) {
   case EXPR_PAREN:
      fputc('(', file);
      print_expr(file, e->expr);
      fputc(')', file);
      break;
   case EXPR_INT:
      fprintf(file, "%jd", e->iVal);
      break;
   case EXPR_UINT:
      fprintf(file, "%jd", e->uVal);
      break;
#if !DISABLE_FP
   case EXPR_FLOAT:
      fprintf(file, "%Lf", e->fVal);
      break;
#endif
   case EXPR_CHAR:
      fprintf(file, "'%c'", e->ch);
      break;
   case EXPR_STRING:
      fprintf(file, "\"%s\"", e->str);
      break;
   case EXPR_NAME:
      fputs(e->str, file);
      break;
   case EXPR_PREFIX:
   case EXPR_UNARY:
      print_token(file, &e->unary.op);
      print_expr(file, e->unary.expr);
      break;
   case EXPR_SUFFIX:
      print_expr(file, e->unary.expr);
      print_token(file, &e->unary.op);
      break;
   case EXPR_ASSIGN:
      print_expr(file, e->assign.left);
      fputs(" = ", file);
      print_expr(file, e->assign.right);
      break;
   case EXPR_BINARY:
      print_expr(file, e->binary.left);
      fputc(' ', file);
      print_token(file, &e->binary.op);
      fputc(' ', file);
      print_expr(file, e->binary.right);
      break;
   case EXPR_ADDROF:
      fputc('&', file);
      print_expr(file, e->expr);
      break;
   case EXPR_INDIRECT:
      fputc('*', file);
      print_expr(file, e->expr);
      break;
   case EXPR_COMMA:
      print_expr(file, e->comma[0]);
      for (size_t i = 1; i < buf_len(e->comma); ++i) {
         fputs(", ", file);
         print_expr(file, e->comma[i]);
      }
      break;
   case EXPR_TERNARY:
      print_expr(file, e->ternary.cond);
      fputs(" ? ", file);
      print_expr(file, e->ternary.true_case);
      fputs(" : ", file);
      print_expr(file, e->ternary.false_case);
      break;
   case EXPR_CAST:
      fputc('(', file);
      print_value_type(file, e->cast.type);
      fputc(')', file);
      print_expr(file, e->cast.expr);
      break;
   case EXPR_FCALL:
      print_expr(file, e->fcall.func);
      fputc('(', file);
      if (e->fcall.params) {
         print_expr(file, e->fcall.params[0]);
         for (size_t i = 1; i < buf_len(e->fcall.params); ++i) {
            fputs(", ", file);
            print_expr(file, e->fcall.params[i]);
         }
      }
      fputc(')', file);
      break;
   case EXPR_ARRAYLEN:
      fputs("arraylen ", file);
      print_expr(file, e->expr);
      break;
   case EXPR_SIZEOF:
      fputs("sizeof (", file);
      if (e->szof.has_expr)
         print_expr(file, e->szof.expr);
      else print_value_type(file, e->szof.type);
      fputc(')', file);
      break;
   case EXPR_MEMBER:
      print_expr(file, e->member.base);
      fprintf(file, ".%s", e->member.name);
      break;
   case NUM_EXPRS: break;
   }
}

bool expr_is_lvalue(const struct expression* e) {
   switch (e->type) {
   case EXPR_PAREN:  return expr_is_lvalue(e->expr);
   case EXPR_MEMBER:
   case EXPR_INDIRECT:
   case EXPR_ASSIGN:
   case EXPR_PREFIX:
   case EXPR_NAME:   return true;
   case EXPR_COMMA:  return expr_is_lvalue(e->comma[buf_len(e->comma) - 1]);
   default:          return false;
   }
}
struct value parse_const_expr(void) {
   struct expression* expr = parse_expr(NULL);
   struct value val;
   if (!try_eval_expr(expr, &val))
      parse_error(&expr->begin, "expected constant expression");
   free_expr(expr);
   return val;
}
bool expr_is_pure(const struct expression* e) {
   switch (e->type) {
   case EXPR_INT:
   case EXPR_UINT:
   case EXPR_STRING:
   case EXPR_CHAR:
#if !DISABLE_FP
   case EXPR_FLOAT:
#endif
   case EXPR_NAME:
   case EXPR_ADDROF:
   case EXPR_INDIRECT:
   case EXPR_CAST:
   case EXPR_SIZEOF:
   case EXPR_ARRAYLEN:
   case EXPR_MEMBER:
      return true;

   case EXPR_PAREN:
      return expr_is_pure(e->expr);
   case EXPR_UNARY:
      return expr_is_pure(e->unary.expr);
   case EXPR_BINARY:
      return expr_is_pure(e->binary.left) && expr_is_pure(e->binary.right);
   case EXPR_TERNARY:
      return expr_is_pure(e->ternary.cond)
         && expr_is_pure(e->ternary.true_case)
         && expr_is_pure(e->ternary.false_case);
   case EXPR_COMMA:
      for (size_t i = 0; i < buf_len(e->comma); ++i) {
         if (!expr_is_pure(e->comma[i])) return false;
      }
      return true;

   case EXPR_PREFIX:
   case EXPR_SUFFIX:
   case EXPR_ASSIGN:
      return false;

   case EXPR_FCALL:
      // TODO: check if function is pure
      return false;
   
   case NUM_EXPRS:
      break;
   }
   panic("unreachable reached");
}
