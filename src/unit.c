#include <stdlib.h>
#include "parser.h"
#include "optim.h"
#include "lex.h"
#include "ir.h"

struct cunit cunit = { NULL };

void parse_unit(void) {
   buf_free(cunit.funcs);
   while (!lexer_match(TK_EOF)) {
      struct function* func = parse_func();
      if (func->scope) func->ir_code = optim_ir_nodes(irgen_func(func));
      buf_push(cunit.funcs, func);
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
