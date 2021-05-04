#include <stdlib.h>
#include "parser.h"
#include "optim.h"
#include "lex.h"
#include "ir.h"

struct compilation_unit* parse_unit(void) {
   struct compilation_unit* u = malloc(sizeof(struct compilation_unit));
   if (!u) panic("failed to allocate compilation unit");
   u->funcs = NULL;
   while (!lexer_match(TK_EOF)) {
      struct function* func = parse_func(u);
      if (func->scope) func->ir_code = optim_ir_nodes(irgen_func(func));
      buf_push(u->funcs, func);
   }
   return u;
}

void print_unit(FILE* file, const struct compilation_unit* u) {
   for (size_t i = 0; i < buf_len(u->funcs); ++i) {
      print_func(file, u->funcs[i]);
      fputc('\n', file);
   }
}
void print_ir_unit(FILE* file, const struct compilation_unit* u) {
   for (size_t i = 0; i < buf_len(u->funcs); ++i) {
      fprintf(file, "\n// function %s", u->funcs[i]->name);
      print_ir_nodes(file, u->funcs[i]->ir_code);
      fputc('\n', file);
   }
}

void free_unit(struct compilation_unit* u) {
   for (size_t i = 0; i < buf_len(u->funcs); ++i) {
      free_func(u->funcs[i]);
   }
   buf_free(u->funcs);
   free(u);
}
