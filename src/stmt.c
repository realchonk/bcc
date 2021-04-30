#include <stdlib.h>
#include "error.h"
#include "parser.h"
#include "lex.h"

const char* stmt_type_str[NUM_STMTS] = {
   [STMT_NOP]     = "empty",
   [STMT_EXPR]    = "expression",
   [STMT_RETURN]  = "return",
   [STMT_IF]      = "if",
   [STMT_WHILE]   = "while",
   [STMT_DO_WHILE]= "do-while",
   [STMT_FOR]     = "for",
   [STMT_VARDECL] = "variable declaration",
   [STMT_SCOPE]   = "scope/compound",
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
   case STMT_SCOPE:
      free_scope(s->scope);
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
   case STMT_VARDECL: {
      const struct variable* var = &s->parent->vars[s->var_idx];
      if (!var) panic("print_stmt(): var == NULL");
      print_value_type(file, var->type);
      fprintf(file, " %s", var->name);
      if (var->init) {
         fputs(" = ", file);
         print_expr(file, var->init);
      }
      fputc(';', file);
      break;
   }
   case STMT_SCOPE:
      print_scope(file, s->scope);
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

struct statement* parse_stmt(struct scope* scope) {
   struct statement* stmt = new_stmt();
   const struct token tk = lexer_peek();
   stmt->begin = tk.begin;
   stmt->parent = scope;
   stmt->func = scope->func;
   switch (tk.type) {
   case TK_SEMICOLON:
      lexer_skip();
      stmt->type = STMT_NOP;
      stmt->end = tk.end;
      break;
   case TK_CLPAREN:
      lexer_skip();
      stmt->type = STMT_SCOPE;
      stmt->scope = make_scope(scope, scope->func);
      while (!lexer_matches(TK_CRPAREN)) {
         buf_push(stmt->scope->body, parse_stmt(stmt->scope));
      }
      stmt->end = lexer_expect(TK_CRPAREN).end;
      break;
   case KW_RETURN:
      lexer_skip();
      stmt->type = STMT_RETURN;
      stmt->expr = lexer_matches(TK_SEMICOLON) ? NULL : parse_expr();
      struct function* f = stmt->parent->func;
      if (stmt->expr) {
         if (f->type->type == VAL_VOID) parse_error(&stmt->expr->begin, "return a value in a void-function");
         struct value_type* old = get_value_type(stmt->parent, stmt->expr);
         if (!is_castable(old, f->type, true))
            parse_error(&stmt->expr->begin, "invalid return type");
         free_value_type(old);
      } else if (f->type->type != VAL_VOID) parse_error(&stmt->end, "expected return value");
      stmt->end = lexer_expect(TK_SEMICOLON).end;
      break;
   case KW_IF:
      lexer_skip();
      lexer_expect(TK_LPAREN);
      stmt->type = STMT_IF;
      stmt->ifstmt.cond = parse_expr();
      lexer_expect(TK_RPAREN);
      stmt->ifstmt.true_case = parse_stmt(scope);
      stmt->ifstmt.false_case = lexer_match(KW_ELSE) ? parse_stmt(scope) : NULL;
      stmt->end = stmt->ifstmt.false_case ? stmt->ifstmt.false_case->end : stmt->ifstmt.true_case->end;
      break;
   case KW_WHILE:
      lexer_skip();
      lexer_expect(TK_LPAREN);
      stmt->type = STMT_WHILE;
      stmt->whileloop.cond = parse_expr();
      lexer_expect(TK_RPAREN);
      stmt->whileloop.stmt = parse_stmt(scope);
      stmt->end = stmt->whileloop.stmt->end;
      break;
   case KW_DO:
      lexer_skip();
      stmt->type = STMT_DO_WHILE;
      stmt->whileloop.stmt = parse_stmt(scope);
      lexer_expect(KW_WHILE);
      lexer_expect(TK_LPAREN);
      stmt->whileloop.cond = parse_expr();
      lexer_expect(TK_RPAREN);
      stmt->end = lexer_expect(TK_SEMICOLON).end;
      break;
   default: {
      struct value_type* vtype = parse_value_type();
      if (vtype) {
         if (vtype->type == VAL_VOID)
            parse_error(&vtype->begin, "invalid use of incomplete type void");
         struct variable var;
         var.type = vtype;
         var.name = lexer_expect(TK_NAME).str;
         var.begin = vtype->begin;
         var.init = lexer_match(TK_EQ) ? parse_expr() : NULL;
         var.end = lexer_expect(TK_SEMICOLON).end;
        
         stmt->var_idx = scope_add_var(scope, &var);
         if (stmt->var_idx == SIZE_MAX) parse_error(&var.begin, "variable '%s' is already declared.", var.name);

         if (var.init) {
            struct value_type* old = get_value_type(scope, var.init);
            if (!is_castable(old, var.type, true))
               parse_error(&var.init->begin, "incompatible init value type");
            free_value_type(old);
         }

         stmt->type = STMT_VARDECL;
         stmt->begin = var.begin;
         stmt->end = var.end;
      } else {
         stmt->type = STMT_EXPR;
         stmt->expr = parse_expr();
         stmt->end = lexer_expect(TK_SEMICOLON).end;
      }
      break;
   }
   }
   return stmt;
}
