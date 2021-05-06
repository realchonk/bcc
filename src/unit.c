#include <stdlib.h>
#include "parser.h"
#include "optim.h"
#include "lex.h"
#include "ir.h"

struct cunit cunit = { NULL};

void parse_unit(void) {
   buf_free(cunit.funcs);
   while (!lexer_match(TK_EOF)) {
      bool has_begin = false;
      unsigned attrs = 0;
      struct source_pos begin;

      if (lexer_match(KW_TYPEDEF)) {
         struct typerename alias;
         alias.type = parse_value_type(NULL);
         alias.name = lexer_expect(TK_NAME).str;
         alias.begin = alias.type->begin;
         alias.end = lexer_expect(TK_SEMICOLON).end;
         buf_push(cunit.renames, alias);
         continue;
      }

      while (lexer_matches(KW_EXTERN) || lexer_matches(KW_STATIC)) {
         const struct token tk = lexer_next();
         switch (tk.type) {
         case KW_EXTERN:
            attrs |= ATTR_EXTERN;
            break;
         case KW_STATIC:
            attrs |= ATTR_STATIC;
            break;
         default: parse_error(&tk.begin, "invalid storage specifier");
         }
         if (!has_begin) {
            begin = tk.begin;
            has_begin = true;
         }
      }

      if ((attrs & (ATTR_EXTERN | ATTR_STATIC)) == (ATTR_EXTERN | ATTR_STATIC))
         parse_error(&begin, "variable cannot be static and extern at the same time");

      struct value_type* type = parse_value_type(NULL);
      istr_t name = lexer_expect(TK_NAME).str;

      if (!has_begin) begin = type->begin;
      
      if (lexer_matches(TK_LPAREN)) {
         struct function* func = parse_func_part(type, name);
         func->begin = begin;
         func->attrs = attrs;
         if (func->scope) {
            if (func->attrs & ATTR_EXTERN)
               parse_warn(&begin, "function definition shall not be extern");
            func->ir_code = optim_ir_nodes(irgen_func(func));
         }
         buf_push(cunit.funcs, func); 
      } else {
         struct variable var;
         var.type = type;
         var.name = name;
         var.begin = begin;
         var.attrs = attrs;
         if (unit_get_var(name))
            parse_error(&begin, "global variable '%s' already declared", name);
         if (lexer_match(TK_LBRACK)) {
            struct expression* expr = parse_expr(NULL);
            struct value val;
            if (!try_eval_expr(expr, &val))
               parse_error(&expr->begin, "gloal VLAs are not allowed.");
            else if (val.type->type != VAL_INT)
               parse_error(&expr->begin, "size of array must be an integer.");
            else if (!val.type->integer.is_unsigned && val.iVal < 0)
               parse_error(&expr->begin, "size of array must be positive.");
            var.type = make_array_vt(val.type);
            var.type->pointer.array.has_const_size = true;
            var.type->pointer.array.size = val.uVal;
            lexer_expect(TK_RBRACK);
         }
         if (lexer_match(TK_EQ)) {
            var.init = parse_expr_no_comma(NULL);
            if (!try_eval_expr(var.init, &var.const_init))
               parse_error(&var.init->begin, "global variables may only be initialized with a constant value");
            var.has_const_value = true;
         } else var.init = NULL;
         var.end = lexer_expect(TK_SEMICOLON).end;
         buf_push(cunit.vars, var);
      }
   }
}

void print_unit(FILE* file) {
   for (size_t i = 0; i < buf_len(cunit.funcs); ++i) {
      print_func(file, cunit.funcs[i]);
      fputc('\n', file);
   }
}
void print_ir_unit(FILE* file) {
   for (size_t i = 0; i < buf_len(cunit.funcs); ++i) {
      struct function* f = cunit.funcs[i];
      fprintf(file, "\n// function %s\n", f->name);
      print_ir_nodes(file, f->ir_code);
      fputc('\n', file);
   }
}

struct function* unit_get_func(const char* name) {
   name = strint(name);
   for (size_t i = 0; i < buf_len(cunit.funcs); ++i) {
      struct function* f = cunit.funcs[i];
      if (name == f->name) return f;
   }
   return NULL;
}
void free_unit(void) {
   for (size_t i = 0; i < buf_len(cunit.funcs); ++i) {
      free_func(cunit.funcs[i]);
   }
   buf_free(cunit.funcs);
}
struct variable* unit_get_var(const char* name) {
   name = strint(name);
   for (size_t i = 0; i < buf_len(cunit.vars); ++i) {
      if (name == cunit.vars[i].name)
         return &cunit.vars[i];
   }
   return NULL;
}
struct typerename* unit_get_typedef(const char* name) {
   name = strint(name);
   for (size_t i = 0; i < buf_len(cunit.renames); ++i) {
      if (name == cunit.renames[i].name)
         return &cunit.renames[i];
   }
   return NULL;
}
