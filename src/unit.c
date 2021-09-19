//  Copyright (C) 2021 Benjamin Stürz
//  
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//  
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//  
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <stdlib.h>
#include "parser.h"
#include "optim.h"
#include "lex.h"
#include "ir.h"

struct cunit cunit = { NULL};

void unit_add_enum(const struct value_type* type) {
   if (type->venum->name && type->venum->is_definition) {
      if (unit_get_enum(type->venum->name))
         parse_error(&type->begin, "'enum %s' already defined", type->venum->name);
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
void unit_add_struct(const struct value_type* type) {
   if (type->vstruct->name && type->vstruct->is_definition) {
      if (unit_get_struct(type->vstruct->name))
         parse_error(&type->begin, "'struct %s' already defined", type->vstruct->name);
      buf_push(cunit.structs, copy_struct(type->vstruct));
   }
}
void unit_add_union(const struct value_type* type) {
   if (type->vstruct->name && type->vstruct->is_definition) {
      if (unit_get_struct(type->vstruct->name))
         parse_error(&type->begin, "'union %s' already defined", type->vstruct->name);
      buf_push(cunit.unions, copy_struct(type->vstruct));
   }
}

void parse_unit(bool gen_ir) {
   buf_free(cunit.funcs);
   while (!lexer_match(TK_EOF)) {
      if (lexer_matches(KW_TYPEDEF)) {
         struct typerename alias;
         alias.begin = lexer_next().begin;
         alias.type = parse_value_type(NULL);
         if (!alias.type)
            parse_error(&alias.begin, "failed to parse type");
         switch (alias.type->type) {
         case VAL_ENUM:
            unit_add_enum(alias.type);
            break;
         case VAL_STRUCT:
            unit_add_struct(alias.type);
            break;
         case VAL_UNION:
            unit_add_union(alias.type);
            break;
         default:
            break;
         }
         alias.name = lexer_expect(TK_NAME).str;
         alias.end = lexer_expect(TK_SEMICOLON).end;
         buf_push(cunit.aliases, alias);
         continue;
      }

      bool has_begin = false;
      unsigned attrs = 0;
      struct source_pos begin;

      while (lexer_matches(KW_EXTERN) || lexer_matches(KW_STATIC)
            || lexer_matches(KW_NORETURN) || lexer_matches(KW_INLINE)) {
         const struct token tk = lexer_next();
         switch (tk.type) {
         case KW_EXTERN:
            attrs |= ATTR_EXTERN;
            break;
         case KW_STATIC:
            attrs |= ATTR_STATIC;
            break;
         case KW_NORETURN:
            attrs |= ATTR_NORETURN;
            break;
         case KW_INLINE:
            attrs |= ATTR_INLINE;
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
      if (!has_begin) begin = lexer_peek().begin;
      if (!type)
         parse_error(&begin, "failed to parse type");

      if ((attrs & ATTR_NORETURN) && type->type != VAL_VOID) {
         parse_warn(&begin, "'noreturn' function has return type");
      }

      if (type->type == VAL_ENUM) {
         unit_add_enum(type);
         if (lexer_match(TK_SEMICOLON)) continue;
      } else if (type->type == VAL_STRUCT) {
         unit_add_struct(type);
         if (lexer_match(TK_SEMICOLON)) continue;
      } else if (type->type == VAL_UNION) {
         unit_add_union(type);
         if (lexer_match(TK_SEMICOLON)) continue;
      }

      const struct token name_tk = lexer_expect(TK_NAME);
      istr_t name = name_tk.str;

      if (!has_begin) begin = type->begin;
      
      if (lexer_matches(TK_LPAREN)) {
         struct function* func = malloc(sizeof(struct function));
         if (!func) panic("failed to allocate function");
         func->name = name;
         func->type = type;
         if (func->scope) {
            struct function* f = unit_get_func_def(name);
            if (f && f->scope)
               parse_error(&begin, "function '%s' already defined", name);
         }
         if (attrs & ATTR_EXTERN || attrs & ATTR_STATIC) {
            const enum attribute a = attrs & ATTR_EXTERN ? ATTR_STATIC : ATTR_EXTERN;
            if (find_func_with_attr(func->name, a)) {
               parse_error(&begin, "function '%s' already declared as %s\n", func->name, attr_to_string(a));
            }
         }
         
         buf_push(cunit.funcs, func);
         parse_func_part(func);
         func->begin = begin;
         func->attrs = attrs;
         if (func->scope) {
            if (func->attrs & ATTR_EXTERN)
               parse_warn(&begin, "function definition shall not be extern");
            if (gen_ir) {
               func->ir_code = optim_ir_nodes(irgen_func(func));
               func->max_reg = ir_max_reg(func->ir_code);
            }
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
               var.type = make_array_vt(vt);
               if (lexer_match(TK_RBRACK)) {
                  var.type->pointer.array.has_const_size = false;
                  var.type->pointer.array.dsize = NULL;
               } else {
                  struct expression* expr = parse_expr(NULL);
                  struct value val;
                  if (!try_eval_expr(expr, &val, NULL))
                     parse_error(&expr->begin, "global VLAs are not allowed.");
                  else if (val.type->type != VAL_INT)
                     parse_error(&expr->begin, "size of array must be an integer.");
                  else if (!val.type->integer.is_unsigned && val.iVal < 0)
                     parse_error(&expr->begin, "size of array must be positive.");
                  var.type->pointer.array.has_const_size = true;
                  var.type->pointer.array.size = val.uVal;
                  lexer_expect(TK_RBRACK);
               }
               vt = var.type;
            }
            var.has_const_value = lexer_match(TK_EQ);
            if (var.has_const_value) {
               var.init = parse_var_init(NULL, &var.type);
               if (var.attrs & ATTR_EXTERN)
                  parse_error(&var.init->begin, "cannot initialize extern variable");
               bool s;
               if (vt_is_array(var.type))
                  s = try_eval_array(var.init, &var.const_init, var.type, NULL);
               else s = try_eval_expr(var.init, &var.const_init, NULL);
               if (!s)
                  parse_error(&var.init->begin, "global variables may only be initialized with a constant value");
               if (!is_castable(var.type, var.type, true))
                  parse_error(&var.init->begin, "incompatible initialization type");
                   
            } else {
               if (var.type->type == VAL_AUTO)
                  parse_error(&var.begin, "auto variable expects initializer");
               var.init = NULL;
            }
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

size_t unit_get_func_idx(istr_t name) {
   for (size_t i = 0; i < buf_len(cunit.funcs); ++i) {
      if (name == cunit.funcs[i]->name) return i;
   }
   return SIZE_MAX;
}
struct function* unit_get_func(istr_t name) {
   const size_t idx = unit_get_func_idx(name);
   return idx == SIZE_MAX ? NULL : cunit.funcs[idx];
}

#define free_structs(structs) \
   for (size_t i = 0; i < buf_len(structs); ++i) { \
      for (size_t j = 0; j < buf_len(structs[i]->entries); ++j) \
         free_value_type(structs[i]->entries[j].type); \
      buf_free(structs[i]->entries); \
   } \
   buf_free(structs)
void free_unit(void) {
   // free functions
   for (size_t i = 0; i < buf_len(cunit.funcs); ++i) {
      free_func(cunit.funcs[i]);
   }
   buf_free(cunit.funcs);

   // free variables
   for (size_t i = 0; i < buf_len(cunit.vars); ++i) {
      struct variable* var = &cunit.vars[i];
      free_value_type(var->type);
      if (var->init)
         free_expr(var->init);
   }
   buf_free(cunit.vars);

   // free typedes
   for (size_t i = 0; i < buf_len(cunit.aliases); ++i)
      free_value_type(cunit.aliases[i].type);
   buf_free(cunit.aliases);

   // free enums
   for (size_t i = 0; i < buf_len(cunit.enums); ++i)
      buf_free(cunit.enums[i]->entries);
   buf_free(cunit.enums);
   buf_free(cunit.constants);

   // free structs & unions
   free_structs(cunit.structs);
   free_structs(cunit.unions);
}
size_t unit_get_var_idx(istr_t name) {
   for (size_t i = 0; i < buf_len(cunit.vars); ++i) {
      if (name == cunit.vars[i].name)
         return i;
   }
   return SIZE_MAX;
}
struct variable* unit_get_var(istr_t name) {
   const size_t idx = unit_get_var_idx(name);
   return idx == SIZE_MAX ? NULL : &cunit.vars[idx];
}

size_t unit_get_typedef_idx(istr_t name) {
   for (size_t i = 0; i < buf_len(cunit.aliases); ++i) {
      if (name == cunit.aliases[i].name)
         return i;
   }
   return SIZE_MAX;
}
struct typerename* unit_get_typedef(istr_t name) {
   const size_t idx = unit_get_typedef_idx(name);
   return idx == SIZE_MAX ? NULL : &cunit.aliases[idx];
}
size_t unit_get_const_idx(istr_t name) {
   for (size_t i = 0; i < buf_len(cunit.constants); ++i) {
      if (name == cunit.constants[i].name)
         return i;
   }
   return SIZE_MAX;

}
bool find_constant(istr_t name, intmax_t* value) {
   const size_t idx = unit_get_const_idx(name);
   if (idx == SIZE_MAX) return false;
   if (value) *value = cunit.constants[idx].value;
   return true;
}
struct enumeration* unit_get_enum(istr_t name) {
   for (size_t i = 0; i < buf_len(cunit.enums); ++i) {
      if (name == cunit.enums[i]->name)
         return cunit.enums[i];
   }
   return NULL;
}
struct structure* unit_get_struct(istr_t name) {
   for (size_t i = 0; i < buf_len(cunit.structs); ++i) {
      if (name == cunit.structs[i]->name)
         return cunit.structs[i];
   }
   return NULL;
}
struct structure* unit_get_union(istr_t name) {
   for (size_t i = 0; i < buf_len(cunit.unions); ++i) {
      if (name == cunit.unions[i]->name)
         return cunit.unions[i];
   }
   return NULL;
}

bool unit_find(istr_t name, struct symbol* sym) {
   size_t idx;
   if ((idx = unit_get_var_idx(name)) != SIZE_MAX) {
      if (sym) {
         sym->type = SYM_VAR;
         sym->idx = idx;
      }
      return true;
   }
   if ((idx = unit_get_func_idx(name)) != SIZE_MAX) {
      if (sym) {
         sym->type = SYM_FUNC;
         sym->idx = idx;
      }
      return true;
   }
   if ((idx = unit_get_typedef_idx(name)) != SIZE_MAX) {
      if (sym) {
         sym->type = SYM_ALIAS;
         sym->idx = idx;
      }
      return true;
   }
   if ((idx = unit_get_const_idx(name)) != SIZE_MAX) {
      if (sym) {
         sym->type = SYM_CONST;
         sym->idx = idx;
      }
      return true;
   }
   return false;
}
bool unit_func_is_extern(istr_t n) {
   for (size_t i = 0; i < buf_len(cunit.funcs); ++i) {
      const struct function* f = cunit.funcs[i];
      if (n == f->name && f->attrs & ATTR_EXTERN)
         return true;
   }
   return false;
}
bool func_is_global(const struct function* f) {
   if (f->attrs & ATTR_STATIC)
      return false;
   else if (f->attrs & ATTR_INLINE) {
      return unit_func_is_extern(f->name);
   } else return true;
}
struct function* unit_get_func_def(istr_t name) {
   for (size_t i = 0; i < buf_len(cunit.funcs); ++i) {
      struct function* f = cunit.funcs[i];
      if (name == f->name && f->scope)
         return f;
   }
   return NULL;
}
struct function* find_func_with_attr(istr_t name, enum attribute a) {
   for (size_t i = 0; i < buf_len(cunit.funcs); ++i) {
      struct function* f = cunit.funcs[i];
      if (name == f->name && (f->attrs & a) == a)
         return f;
   }
   return NULL;
}
