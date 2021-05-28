#include "parser.h"
#include "error.h"
#include "lex.h"
#include "ir.h"

void parse_func_part(struct function* func) {
   func->params = NULL;
   func->ir_code = NULL;
   lexer_expect(TK_LPAREN);

   if (lexer_matches(TK_RPAREN)) {
      func->variadic = true;
      func->params = NULL;
   } else {
      do {
         if (lexer_match(TK_DDD)) {
            func->variadic = true;
            break;
         }
         struct variable var;
         var.type = parse_value_type(NULL);
         if (var.type->type == VAL_VOID) {
            if (func->params)
               parse_error(&var.type->begin, "incomplete type void");
            break;
         }
         var.begin = var.type->begin;
         
         if (lexer_matches(TK_NAME)) {
            const struct token name = lexer_next();
         
            var.name = name.str;
            var.end = name.end;
            for (size_t i = 0; i < buf_len(func->params); ++i) {
               if (var.name == func->params[i].name)
                  parse_error(&var.begin, "parameter '%s' is already declared", var.name);
            }
         } else {
            var.name = NULL;
            var.end = var.type->end;
         }
         buf_push(func->params, var);
      } while (lexer_match(TK_COMMA));
   }
   lexer_expect(TK_RPAREN);

   func->begin = func->type->begin;
   if (lexer_match(TK_SEMICOLON)) {
      func->scope = NULL;
   } else {
      func->scope = make_scope(NULL, func);
      lexer_expect(TK_CLPAREN);
      while (!lexer_matches(TK_CRPAREN)) {
         buf_push(func->scope->body, parse_stmt(func->scope));
      }
      func->end = lexer_expect(TK_CRPAREN).end;
   }
}

static void print_param(FILE* file, const struct variable* var) {
   print_value_type(file, var->type);
   if (var->name) fprintf(file, " %s", var->name);
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
      if (func->variadic) fputs(", ...", file);
   } else if (func->variadic) fputs("...", file);
   if (func->scope) {
      fputs(") ", file);
      print_scope(file, func->scope);
   } else {
      fputs(");", file);
   }
}

void free_func(struct function* func) {
   free_value_type(func->type);
   if (func->scope) free_scope(func->scope);
   for (size_t i = 0; i < buf_len(func->params); ++i) {
      free_value_type(func->params[i].type);
   }
   if (func->ir_code) free_ir_nodes(func->ir_code);
   buf_free(func->params);
   free(func);
}
size_t func_find_param_idx(const struct function* func, const char* name) {
   name = strint(name);
   for (size_t i = 0; i < buf_len(func->params); ++i) {
      if (name == func->params[i].name) return i;
   }
   return SIZE_MAX;
}
const struct variable* func_find_param(const struct function* func, const char* name) {
   const size_t i = func_find_param_idx(func, name);
   return i == SIZE_MAX ? NULL : &func->params[i];
}

