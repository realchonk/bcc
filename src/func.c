#include "error.h"
#include "stmt.h"
#include "func.h"
#include "lex.h"

struct function* parse_func(void) {
   struct function* func = malloc(sizeof(struct function));
   if (!func) panic("parse_func(): failed to allocate function");
   func->type = parse_value_type();
   func->name = lexer_expect(TK_NAME).str;
   lexer_expect(TK_LPAREN);

   if (!lexer_matches(TK_RPAREN)) {
      do {
         struct variable var;
         var.type = parse_value_type();
         const struct token name = lexer_expect(TK_NAME);
         
         var.name = name.str;
         var.begin = var.type->begin;
         var.end = name.end;
         for (size_t i = 0; i < buf_len(func->params); ++i) {
            if (var.name == func->params[i].name)
               parse_error(&var.begin, "parameter '%s' is already declared", var.name);
         }
         buf_push(func->params, var);
      } while (lexer_match(TK_SEMICOLON));
   }
   lexer_expect(TK_RPAREN);

   func->begin = func->type->begin;
   func->scope = make_scope(NULL);
   lexer_expect(TK_CLPAREN);
   while (!lexer_matches(TK_CRPAREN)) {
      buf_push(func->scope->body, parse_stmt(func->scope));
   }
   func->end = lexer_expect(TK_CRPAREN).end;
   return func;
}

static void print_param(FILE* file, const struct variable* var) {
   print_value_type(file, var->type);
   fprintf(file, " %s", var->name);
}

void print_func(FILE* file, const struct function* func) {
   print_value_type(file, func->type);
   fprintf(file, " %s(", func->name);
   if (func->params) {
      print_param(file, &func->params[0]);
      for (size_t i = 1; i < buf_len(func->params); ++i) {
         fputs(", ", file);
         print_param(file, &func->params[i]);
      }
   }
   fputs(") ", file);
   print_scope(file, func->scope);
}

void free_func(struct function* func) {
   free_value_type(func->type);
   free_scope(func->scope);
   for (size_t i = 0; i < buf_len(func->params); ++i) {
      free_value_type(func->params[i].type);
   }
   buf_free(func->params);
   free(func);
}
