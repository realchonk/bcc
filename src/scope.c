#include <stdlib.h>
#include "strint.h"
#include "error.h"
#include "scope.h"
#include "stmt.h"
#include "expr.h"

#define INDENT 3

struct scope* make_scope(struct scope* parent) {
   struct scope* s = malloc(sizeof(struct scope));
   if (!s) panic("make_scope(): failed to allocate scope");
   s->parent = parent;
   s->body = NULL;
   s->vars = NULL;
   return s;
}

void free_scope(struct scope* scope) {
   for (size_t i = 0; i < buf_len(scope->body); ++i) {
      free_stmt(scope->body[i]);
   }
   buf_free(scope->body);
   for (size_t i = 0; i < buf_len(scope->vars); ++i) {
      free_value_type(scope->vars[i].type);
      if (scope->vars[i].init)
         free_expr(scope->vars[i].init);
   }
   buf_free(scope->vars);
   free(scope);
}

static void print_indent(FILE* file, unsigned num) {
   for (unsigned i = 0; i < num; ++i) fputc(' ', file);
}
static unsigned calculate_indent(const struct scope* scope) {
   const struct scope* s = scope;
   unsigned indent = 0;
   s = s->parent;
   while (s) {
      s = s->parent;
      indent += INDENT;
   }
   return indent;
}

void print_scope(FILE* file, const struct scope* scope) {
   const unsigned indent = calculate_indent(scope);
   fputs("{\n", file);
   /*for (size_t i = 0; i < buf_len(scope->vars); ++i) {
      print_indent(file, indent);
      print_value_type(file, scope->vars[i].type);
      fprintf(file, " %s", scope->vars[i].name);
      if (scope->vars[i].init) {
         fputs(" = ", file);
         print_expr(file, scope->vars[i].init);
      }
      fputs(";\n", file);
   }*/
   for (size_t i = 0; i < buf_len(scope->body); ++i) {
      print_indent(file, indent);
      print_stmt(file, scope->body[i]);
   }
   print_indent(file, indent - INDENT);
   fputs("}\n", file);
}


size_t scope_find_var_idx(struct scope* scope, const char* name) {
   name = strint(name);
   for (size_t i = 0; i < buf_len(scope->vars); ++i) {
      if (name == scope->vars[i].name) return i;
   }
   return scope->parent ? scope_find_var_idx(scope->parent, name) : SIZE_MAX;
}
const struct variable* scope_find_var(struct scope* scope, const char* name) {
   const size_t idx = scope_find_var_idx(scope, name);
   return idx != SIZE_MAX ? &scope->vars[idx] : NULL;
}
size_t scope_add_var(struct scope* scope, const struct variable* var) {
   for (size_t i = 0; i < buf_len(scope->vars); ++i) {
      if (var->name == scope->vars[i].name) return SIZE_MAX;
   }
   buf_push(scope->vars, *var);
   return buf_len(scope->vars) - 1;
}


