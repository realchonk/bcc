#include <stdlib.h>
#include "parser.h"
#include "optim.h"
#include "lex.h"
#include "ir.h"

struct cunit cunit = { NULL};

static void add_enum(const struct value_type* type) {
   if (type->venum->name && type->venum->is_definition) {
      if (unit_get_enum(type->venum->name))
         parse_error(&type->begin, "enum '%s' already defined", type->venum->name);
      buf_push(cunit.enums, copy_enum(type->venum));
   }

   // add all entries as constants
   for (size_t i = 0; i < buf_len(type->venum->entries); ++i) {
      const struct enum_entry* e = &type->venum->entries[i];
      if (find_constant(e->name, NULL))
         parse_error(&type->begin, "constant '%s' already defined", e->name);
      else if (unit_get_var(e->name) || unit_get_func(e->name))
         parse_error(&type->begin, "'%s' already declared", e->name);
      buf_push(cunit.constants, *e);
   }

}

void parse_unit(void) {
   buf_free(cunit.funcs);
   while (!lexer_match(TK_EOF)) {
      if (lexer_matches(KW_TYPEDEF)) {
         struct typerename alias;
         alias.begin = lexer_next().begin;
         alias.type = parse_value_type(NULL);
         if (!alias.type)
            parse_error(&alias.end, "failed to parse type");
         if (alias.type->type == VAL_ENUM)
            add_enum(alias.type);
         alias.name = lexer_expect(TK_NAME).str;
         alias.end = lexer_expect(TK_SEMICOLON).end;
         buf_push(cunit.renames, alias);
         continue;
      }

      bool has_begin = false;
      unsigned attrs = 0;
      struct source_pos begin;

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
      if (!type)
         parse_error(&begin, "failed to parse type");

      if (type->type == VAL_ENUM) {
         add_enum(type);
         if (lexer_match(TK_SEMICOLON)) continue;
      }

      const struct token name_tk = lexer_expect(TK_NAME);
      istr_t name = name_tk.str;

      if (!has_begin) begin = type->begin;
      
      if (lexer_matches(TK_LPAREN)) {
         struct function* func = parse_func_part(type, name);
         {
            struct function* f = unit_get_func(name);
            if (f) {
               if (f->scope)
                  parse_error(&begin, "function '%s' already defined", name);
            }
         }
         func->begin = begin;
         func->attrs = attrs;
         buf_push(cunit.funcs, func);
         if (func->scope) {
            if (func->attrs & ATTR_EXTERN)
               parse_warn(&begin, "function definition shall not be extern");
            func->ir_code = optim_ir_nodes(irgen_func(func));
         }
      } else {
         bool first = true;
         do {
            struct value_type* vt = copy_value_type(type);
            struct variable var;
            struct source_pos name_end;
            var.type = vt;
            if (first) {
               var.name = name;
               name_end = name_tk.end;
               first = false;
            } else {
               const struct token tk = lexer_expect(TK_NAME);
               var.begin = tk.begin;
               var.name = tk.str;
               name_end = tk.end;
            }
            var.begin = begin;
            var.attrs = attrs;
            {
               struct variable* v = unit_get_var(var.name);
               if (v) {
                  if (!(v->attrs & ATTR_EXTERN))
                     parse_error(&begin, "global variable '%s' already declared", name);
                  if (!value_type_equal(v->type, var.type))
                     parse_error(&begin, "incompatible types");
               }
            }
            if (unit_get_func(name))
               parse_error(&begin, "function with name '%s' already exists", name);
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
               if (var.attrs & ATTR_EXTERN)
                  parse_error(&var.init->begin, "cannot initialize extern variable");
               if (!try_eval_expr(var.init, &var.const_init))
                  parse_error(&var.init->begin, "global variables may only be initialized with a constant value");
               var.has_const_value = true;
            } else var.init = NULL;
            var.end = var.init ? var.init->end : name_end;
            buf_push(cunit.vars, var);
         } while (lexer_match(TK_COMMA));
         lexer_expect(TK_SEMICOLON);
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
bool find_constant(const char* name, intmax_t* value) {
   name = strint(name);
   for (size_t i = 0; i < buf_len(cunit.constants); ++i) {
      if (name == cunit.constants[i].name) {
         if (value) *value = cunit.constants[i].value;
         return true;
      }
   }
   return false;
}
struct enumeration* unit_get_enum(const char* name) {
   name = strint(name);
   for (size_t i = 0; i < buf_len(cunit.enums); ++i) {
      if (name == cunit.enums[i]->name)
         return cunit.enums[i];
   }
   return NULL;
}
