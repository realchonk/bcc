#include <stdlib.h>
#include "error.h"
#include "stmt.h"
#include "lex.h"

#if defined(__GNUC__)
#define fallthrough __attribute__((fallthrough))
#else
#define fallthough
#endif

const char* stmt_type_str[NUM_STMTS] = {
   [STMT_NOP]     = "empty",
   [STMT_EXPR]    = "expression",
   [STMT_RETURN]  = "return",
   [STMT_IF]      = "if",
   [STMT_WHILE]   = "while",
   [STMT_DO_WHILE]= "do-while",
   [STMT_FOR]     = "for",
   [STMT_COMPOUND]= "compound",
};

void free_stmt(struct statement* s) {
   switch (s->type) {
   case STMT_EXPR:
   case STMT_RETURN:
      if (s->expr)
         free_expr(s->expr);
      break;
   case STMT_IF:
      free_expr(s->ifstmt.cond);
      free_stmt(s->ifstmt.true_case);
      if (s->ifstmt.false_case)
         free_stmt(s->ifstmt.false_case);
      break;
   case STMT_WHILE:
   case STMT_DO_WHILE:
      free_expr(s->whileloop.cond);
      free_stmt(s->whileloop.stmt);
      break;
   case STMT_FOR:
      free_stmt(s->forloop.init);
      if (s->forloop.cond)
         free_expr(s->forloop.cond);
      if (s->forloop.end)
         free_expr(s->forloop.end);
      free_stmt(s->forloop.stmt);
      break;
   case STMT_COMPOUND:
      for (size_t i = 0; i < buf_len(s->stmts); ++i) {
         free_stmt(s->stmts[i]);
      }
      buf_free(s->stmts);
      break;
   default: break;
   }
   free(s);
}

void print_stmt(FILE* file, const struct statement* s) {
   switch (s->type) {
   case STMT_RETURN:
      fputs("return ", file);
      fallthrough;
   case STMT_EXPR:
      print_expr(file, s->expr);
      fallthrough;
   case STMT_NOP:
      fputc(';', file);
      break;
   case STMT_IF:
      fputs("if (", file);
      print_expr(file, s->ifstmt.cond);
      fputs(") ", file);
      print_stmt(file, s->ifstmt.true_case);
      if (s->ifstmt.false_case) {
         fputs("else ", file);
         print_stmt(file, s->ifstmt.false_case);
      }
      break;
   case STMT_WHILE:
      fputs("while (", file);
      print_expr(file, s->whileloop.cond);
      fputs(") ", file);
      print_stmt(file, s->whileloop.stmt);
      break;
   case STMT_DO_WHILE:
      fputs("do ", file);
      print_stmt(file, s->whileloop.stmt);
      fputs("while (", file);
      print_expr(file, s->whileloop.cond);
      fputs(");", file);
      break;
   case STMT_FOR:
      fputs("for (", file);
      print_stmt(file, s->forloop.init);
      if (s->forloop.cond) {
         fputc(' ', file);
         print_expr(file, s->forloop.cond);
      }
      fputc(';', file);
      if (s->forloop.end) {
         fputc(' ', file);
         print_expr(file, s->forloop.end);
      }
      fputs(") ", file);
      print_stmt(file, s->forloop.stmt);
      break;
   case STMT_COMPOUND:
      fputs("{\n", file);
      for (size_t i = 0; i < buf_len(s->stmts); ++i) {
         print_stmt(file, s->stmts[i]);
      }
      fputc('}', file);
      break;
   case NUM_STMTS: break;
   }
   fputc('\n', file);
}

static struct statement* new_stmt(void) {
   struct statement* stmt = malloc(sizeof(struct statement));
   if (!stmt) panic("failed to allocate statement");
   else return stmt;
}

struct statement* parse_stmt(void) {
   struct statement* stmt = new_stmt();
   const struct token tk = lexer_peek();
   stmt->begin = tk.begin;
   switch (tk.type) {
   case TK_SEMICOLON:
      lexer_skip();
      stmt->type = STMT_NOP;
      stmt->end = tk.end;
      break;
   case TK_CLPAREN:
      lexer_skip();
      stmt->type = STMT_COMPOUND;
      stmt->stmts = NULL;
      while (!lexer_match(TK_CRPAREN)) {
         buf_push(stmt->stmts, parse_stmt());
      }
      stmt->end = stmt->stmts[buf_len(stmt->stmts) - 1]->end;
      break;
   case KW_RETURN:
      lexer_skip();
      stmt->type = STMT_RETURN;
      stmt->expr = parse_expr();
      stmt->end = lexer_expect(TK_SEMICOLON).end;
      break;
   case KW_IF:
      lexer_skip();
      lexer_expect(TK_LPAREN);
      stmt->type = STMT_IF;
      stmt->ifstmt.cond = parse_expr();
      lexer_expect(TK_RPAREN);
      stmt->ifstmt.true_case = parse_stmt();
      stmt->ifstmt.false_case = lexer_match(KW_ELSE) ? parse_stmt() : NULL;
      stmt->end = stmt->ifstmt.false_case ? stmt->ifstmt.false_case->end : stmt->ifstmt.true_case->end;
      break;
   case KW_WHILE:
      lexer_skip();
      lexer_expect(TK_LPAREN);
      stmt->type = STMT_WHILE;
      stmt->whileloop.cond = parse_expr();
      lexer_expect(TK_RPAREN);
      stmt->whileloop.stmt = parse_stmt();
      stmt->end = stmt->whileloop.stmt->end;
      break;
   case KW_DO:
      lexer_skip();
      stmt->type = STMT_DO_WHILE;
      stmt->whileloop.stmt = parse_stmt();
      lexer_expect(KW_WHILE);
      lexer_expect(TK_LPAREN);
      stmt->whileloop.cond = parse_expr();
      lexer_expect(TK_RPAREN);
      stmt->end = lexer_expect(TK_SEMICOLON).end;
      break;
   default:
      stmt->type = STMT_EXPR;
      stmt->expr = parse_expr();
      stmt->end = lexer_expect(TK_SEMICOLON).end;
      break;
   }
   return stmt;
}
