#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "parser.h"
#include "error.h"
#include "lex.h"

const char* expr_type_str[NUM_EXPRS] = {
   [EXPR_PAREN]   = "sub",
   [EXPR_INT]     = "signed integer",
   [EXPR_UINT]    = "unsigned integer",
   [EXPR_FLOAT]   = "floating-point",
   [EXPR_CHAR]    = "character-literal",
   [EXPR_STRING]  = "string-literal",
   [EXPR_NAME]    = "identifier",
   [EXPR_UNARY]   = "unary",
   [EXPR_BINARY]  = "binary",
   [EXPR_PREFIX]  = "prefix",
   [EXPR_SUFFIX]  = "suffix",
   [EXPR_ADDROF]  = "address-of",
   [EXPR_INDIRECT]= "indirection",
   [EXPR_TERNARY] = "ternary",
   [EXPR_ASSIGN]  = "assignment",
   [EXPR_COMMA]   = "comma",
   [EXPR_CAST]    = "cast",
   [EXPR_FCALL]   = "function call",
};

static struct expression* new_expr(void) {
   struct expression* expr = malloc(sizeof(struct expression));
   if (!expr) panic("failed to allocate expression");
   else return expr;
}

static struct expression* expr_assign(void);
static struct expression* expr_prim(void) {
   const struct token tk = lexer_next();
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
      break;
   case TK_CHARACTER:
      expr->type = EXPR_CHAR;
      expr->ch = tk.ch;
      break;
   case TK_NAME:
      if (lexer_match(TK_LPAREN)) {
         expr->type = EXPR_FCALL;
         expr->fcall.name = tk.str;
         expr->fcall.params = NULL;
         if (!lexer_matches(TK_RPAREN)) {
            do {
               buf_push(expr->fcall.params, expr_assign());
            } while (lexer_match(TK_COMMA));
         }
         lexer_expect(TK_RPAREN);
      } else {
         expr->type = EXPR_NAME;
         expr->str = tk.str;
      }
      break;
   case TK_LPAREN:
      expr->cast.type = parse_value_type();
      if (expr->cast.type) {
         expr->type = EXPR_CAST;
         lexer_expect(TK_RPAREN);
         expr->cast.expr = expr_prim();
         expr->end = expr->cast.expr->end;
      } else {
         expr->type = EXPR_PAREN;
         expr->expr = parse_expr();
         lexer_expect(TK_RPAREN);
      }
      break;
   default:
      parse_error(&tk.begin, "expected expression, got %s\n", token_type_str[tk.type]);
      break;
   }

   return expr;
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
      if (!expr_is_lvalue(expr->expr))
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
   if (lexer_matches(TK_PLPL) || lexer_matches(TK_MIMI)) {
      struct expression* tmp = new_expr();
      tmp->type = EXPR_SUFFIX;
      tmp->unary.op = lexer_next();
      tmp->unary.expr = expr;
      tmp->begin = tmp->unary.op.begin;
      tmp->end = expr->end;
      expr = tmp;
   }
   return expr;
}

static struct expression* expr_mul(void) {
   struct expression* left = expr_unary();
   while (lexer_matches(TK_STAR) || lexer_matches(TK_SLASH) || lexer_match(TK_PERC)) {
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
      struct expression* expr = new_expr();
      expr->type = EXPR_ASSIGN;
      expr->begin = left->begin;
      expr->binary.op = lexer_next();
      expr->binary.left = left;
      expr->binary.right = expr_assign();
      expr->end = expr->binary.right->end;
      left = expr;
   }
   return left;
}

struct expression* parse_expr(void) {
   struct expression* expr = expr_assign();
   if (!lexer_match(TK_COMMA)) return expr;
   struct expression* comma = new_expr();
   comma->type = EXPR_COMMA;
   comma->begin = expr->begin;
   comma->comma = NULL;
   buf_push(comma->comma, expr);
   do {
      buf_push(comma->comma, expr_assign());
   } while (lexer_match(TK_COMMA));
   return comma;
}


void free_expr(struct expression* e) {
   switch (e->type) {
   case EXPR_ADDROF:
   case EXPR_INDIRECT:
   case EXPR_PAREN:  free_expr(e->expr); break;
   case EXPR_PREFIX:
   case EXPR_SUFFIX:
   case EXPR_UNARY:  free_expr(e->unary.expr); break;
   case EXPR_ASSIGN:
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
   case EXPR_INT:
   case EXPR_UINT:
   case EXPR_STRING:
   case EXPR_CHAR:
   case EXPR_FLOAT:
   case EXPR_NAME:
   case NUM_EXPRS:   break;
   }
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
   case EXPR_FLOAT:
      fprintf(file, "%Lf", e->fVal);
      break;
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
      fprintf(file, "%s(", e->fcall.name);
      if (e->fcall.params) {
         print_expr(file, e->fcall.params[0]);
         for (size_t i = 1; i < buf_len(e->fcall.params); ++i) {
            fputs(", ", file);
            print_expr(file, e->fcall.params[i]);
         }
      }
      fputc(')', file);
      break;


   case NUM_EXPRS: break;
   }
}

bool expr_is_lvalue(const struct expression* e) {
   switch (e->type) {
   case EXPR_PAREN:  return expr_is_lvalue(e->expr);
   case EXPR_INDIRECT:
   case EXPR_NAME:   return true;
   default:          return false;
   }
}

