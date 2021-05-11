#include <stdlib.h>
#include <string.h>
#include "target.h"
#include "error.h"
#include "parser.h"
#include "optim.h"
#include "lex.h"

const char* stmt_type_str[NUM_STMTS] = {
   [STMT_NOP]     = "empty",
   [STMT_EXPR]    = "expression",
   [STMT_RETURN]  = "return",
   [STMT_IF]      = "if",
   [STMT_WHILE]   = "while",
   [STMT_DO_WHILE]= "do-while",
   [STMT_VARDECL] = "variable declaration",
   [STMT_SCOPE]   = "scope/compound",
   [STMT_BREAK]   = "break",
   [STMT_CONTINUE]= "continue",
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
      if (s->whileloop.end)
         free_expr(s->whileloop.end);
      break;
   case STMT_SCOPE:
      free_scope(s->scope);
      break;
   case STMT_VARDECL:
      free_value_type(s->var_decl.type);
      break;
   default: break;
   }
   free(s);
}

void print_stmt(FILE* file, const struct statement* s) {
   switch (s->type) {
   case STMT_RETURN:
      fputs("return", file);
      if (!s->expr) { fputc(';', file); break; }
      else fputc(' ', file);
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
   case STMT_VARDECL: {
      print_value_type(file, s->var_decl.type);
      for (size_t i = 0; i < s->var_decl.num; ++i) {
         const struct variable* var = &s->parent->vars[i + s->var_decl.idx];
         if (!var) panic("print_stmt(): var == NULL");
         if (i != 0) fputc(',', file);
         if (var->type->type == VAL_POINTER && var->type->pointer.is_array) {
            print_value_type(file, var->type->pointer.type);
            fprintf(file, " %s[", var->name);
            if (var->type->pointer.array.has_const_size) {
               fprintf(file, "%zu", var->type->pointer.array.size);
            } else {
               print_expr(file, var->type->pointer.array.dsize);
            }
            fputc(']', file);
         } else {
            fprintf(file, " %s", var->name);
            if (var->init) {
               fputs(" = ", file);
               print_expr(file, var->init);
            }
         }
      }
      fputc(';', file);
      break;
   }
   case STMT_SCOPE:
      print_scope(file, s->scope);
      break;
   case STMT_BREAK:
      fputs("break;", file);
      break;
   case STMT_CONTINUE:
      fputs("continue;", file);
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
      buf_push(stmt->parent->children, stmt->scope);
      break;
   case KW_RETURN:
      lexer_skip();
      stmt->type = STMT_RETURN;
      stmt->expr = lexer_matches(TK_SEMICOLON) ? NULL : parse_expr(scope);
      struct function* f = stmt->parent->func;
      if (stmt->expr) {
         if (f->type->type == VAL_VOID) parse_error(&stmt->expr->begin, "return a value in a void-function");
         struct value_type* old = get_value_type(stmt->parent, stmt->expr);
         if (!is_castable(old, f->type, true))
            parse_error(&stmt->expr->begin, "invalid implicit conversion from '%s' to '%s'",
                  value_type_str[old->type], value_type_str[f->type->type]);
         free_value_type(old);
      } else if (f->type->type != VAL_VOID) parse_error(&stmt->end, "expected return value");
      stmt->end = lexer_expect(TK_SEMICOLON).end;
      break;
   case KW_IF:
      lexer_skip();
      lexer_expect(TK_LPAREN);
      stmt->type = STMT_IF;
      stmt->ifstmt.cond = parse_expr(scope);
      lexer_expect(TK_RPAREN);
      stmt->ifstmt.true_case = parse_stmt(scope);
      stmt->ifstmt.false_case = lexer_match(KW_ELSE) ? parse_stmt(scope) : NULL;
      stmt->end = stmt->ifstmt.false_case ? stmt->ifstmt.false_case->end : stmt->ifstmt.true_case->end;
      break;
   case KW_WHILE:
      lexer_skip();
      lexer_expect(TK_LPAREN);
      stmt->type = STMT_WHILE;
      stmt->whileloop.cond = parse_expr(scope);
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
      stmt->whileloop.cond = parse_expr(scope);
      lexer_expect(TK_RPAREN);
      stmt->end = lexer_expect(TK_SEMICOLON).end;
      break;
   case KW_BREAK:
      lexer_skip();
      stmt->type = STMT_BREAK;
      stmt->end = lexer_expect(TK_SEMICOLON).end;
      break;
   case KW_CONTINUE:
      lexer_skip();
      stmt->type = STMT_CONTINUE;
      stmt->end = lexer_expect(TK_SEMICOLON).end;
      break;
   case KW_FOR:
   {
      struct statement* outer = new_stmt();
      outer->type = STMT_SCOPE;
      outer->scope = make_scope(scope, scope->func);
      outer->parent = scope;
      outer->func = scope->func;
      outer->begin = lexer_next().begin;
      buf_push(scope->children, outer->scope);
      
      lexer_expect(TK_LPAREN);
     
      buf_push(outer->scope->body, parse_stmt(outer->scope));

      stmt->type = STMT_WHILE;
      stmt->parent = outer->scope;
      struct expression* cond;
      if (lexer_matches(TK_SEMICOLON)) {
         cond = malloc(sizeof(struct expression));
         if (!cond) panic("parse_stmt(): failed to allocate expression");
         cond->type = EXPR_UINT;
         cond->begin = cond->end = outer->begin;
         cond->uVal = 1;
      } else cond = parse_expr(scope);
      stmt->whileloop.cond = cond;
      lexer_expect(TK_SEMICOLON);
      
      stmt->whileloop.end = lexer_matches(TK_RPAREN) ? NULL : parse_expr(scope);
      lexer_expect(TK_RPAREN);
      stmt->whileloop.stmt = parse_stmt(outer->scope);
      
      buf_push(outer->scope->body, stmt);
      outer->end = stmt->end;

      stmt = outer;
      break;
   }
   default: {
      struct value_type* base_type = parse_value_type(scope);
      if (base_type) {
         if (base_type->type == VAL_VOID)
            parse_error(&base_type->begin, "invalid use of incomplete type void");

         stmt->var_decl.num = 0;
         stmt->var_decl.idx = buf_len(scope->vars);
         stmt->var_decl.type = base_type;
         stmt->type = STMT_VARDECL;
         stmt->begin = base_type->begin;
         do {
            struct value_type* vtype = copy_value_type(base_type);
            struct variable var;
            const struct token name_tk = lexer_expect(TK_NAME);
            var.name = name_tk.str;
            var.begin = base_type->begin;
            
            if (!scope->parent && func_find_param(scope->func, var.name))
               parse_error(&var.begin, "redefinition of parameter as variable");
    
            // TODO: add multi-dimensional arrays
            if (lexer_match(TK_LBRACK)) {
               vtype = make_array_vt(vtype);
               if (lexer_match(TK_RBRACK)) {
                  //parse_error(&vtype->end, "expected array size");
                  vtype->pointer.array.has_const_size = false;
                  vtype->pointer.array.dsize = NULL;
                  goto skip_asize;
               }
               struct expression* expr = parse_expr(scope);
               struct value val;
               if (try_eval_expr(expr, &val)) {
                  if (val.type->type != VAL_INT)
                     parse_error(&expr->begin, "expected integer size");
                  else if (!val.type->integer.is_unsigned && val.iVal < 0)
                     parse_error(&expr->begin, "negative array size");
                  else if (val.uVal == 0)
                     parse_error(&expr->begin, "zero length array");
                  vtype->pointer.array.has_const_size = true;
                  vtype->pointer.array.size = val.uVal;
               } else {
                  if (!target_info.has_c99_array)
                     parse_error(&expr->begin, BCC_ARCH " does not support C99 arrays.");
                  struct value_type* st = get_value_type(scope, expr);
                  if (st->type != VAL_INT)
                     parse_error(&expr->begin, "expected integer size");
                  vtype->pointer.array.has_const_size = false;
                  vtype->pointer.array.dsize = expr;
               }
               vtype->end = lexer_expect(TK_RBRACK).end;
            }
         skip_asize:
    
            var.init = lexer_match(TK_EQ) ? parse_expr_no_comma(scope) : NULL;
            var.end = var.init ? var.init->end : name_tk.end;
            var.type = vtype;
    
            if (vtype->type == VAL_POINTER && vtype->pointer.is_array && var.init) {
               if (!vtype->pointer.array.has_const_size && vtype->pointer.array.dsize)
                  parse_error(&var.init->begin, "initializing a variable-length array is not supported");
               if (vtype->pointer.type->type == VAL_INT && vtype->pointer.type->integer.size == INT_CHAR) {
                  if (var.init->type == EXPR_STRING) {
                     const char* str = var.init->str;
                     const size_t len = strlen(str) + 1;
                     if (vtype->pointer.array.has_const_size) {
                        if (len < vtype->pointer.array.size)
                           parse_warn(&var.init->begin, "array is too small to fit string, cutting off");
                     } else {
                        vtype->pointer.array.has_const_size = true;
                        vtype->pointer.array.size = len;
                     }
                  } else parse_error(&var.init->begin, "only string literal initialization supported");
               }
               else parse_error(&var.init->begin, "array initialization is only supported for char []");
            }
    
            if (vtype->type == VAL_AUTO) {
               if (!var.init) parse_error(&var.end, "auto variable expects initializer");
               var.type = decay(get_value_type(scope, var.init));
               var.type->is_const = vtype->is_const;
               free_value_type(vtype);
               vtype = var.type;
            }
    
            if (var.init) {
               struct value_type* old = get_value_type(scope, var.init);
               if (!is_castable(old, var.type, true))
                  parse_error(&var.init->begin, "incompatible init value type");
               free_value_type(old);
            } else if (vtype->is_const && (vtype->type != VAL_POINTER || !vtype->pointer.is_array))
               parse_error(&var.end, "expected init value for const variable");
            
            if (scope_add_var(scope, &var) == SIZE_MAX)
               parse_error(&var.begin, "variable '%s' is already declared.", var.name);
            ++stmt->var_decl.num;
         } while (lexer_match(TK_COMMA));
         stmt->end = lexer_expect(TK_SEMICOLON).end;
      } else {
         stmt->type = STMT_EXPR;
         stmt->expr = parse_expr(scope);
         stmt->end = lexer_expect(TK_SEMICOLON).end;
      }
      break;
   }
   }
   return optim_stmt(stmt);
}
bool stmt_is_pure(const struct statement* s) {
   switch (s->type) {
   case STMT_NOP:
      return true;

   case STMT_EXPR:
      return expr_is_pure(s->expr);
   case STMT_IF:
      return expr_is_pure(s->ifstmt.cond)
         && stmt_is_pure(s->ifstmt.true_case)
         && stmt_is_pure(s->ifstmt.false_case);
   case STMT_SCOPE:
      for (size_t i = 0; i < buf_len(s->scope->body); ++i) {
         if (!stmt_is_pure(s->scope->body[i]))
            return false;
      }
      return true;

   case STMT_WHILE:
   case STMT_DO_WHILE:
      // TODO: implement check if this is a infinite-loop
      return false;

   case STMT_RETURN:
   case STMT_BREAK:
   case STMT_CONTINUE:
   case STMT_VARDECL:
      return false;
   case NUM_STMTS:
      break;
   }
   panic("stmt_is_pure(): unreachable reached");
}
