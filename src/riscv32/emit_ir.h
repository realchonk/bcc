#include "target.h"

static size_t sizeof_scope(const struct scope* scope) {
   size_t num = 0;
   for (size_t i = 0; i < buf_len(scope->vars); ++i) {
      num += sizeof_value(scope->vars[i].type, false);
   }
   size_t max_child = 0;
   for (size_t i = 0; i < buf_len(scope->children); ++i) {
      const size_t sz = sizeof_scope(scope->children[i]);
      if (sz > max_child) max_child = sz;
   }
   return num + max_child;
}

static void assign_scope(struct scope* scope, size_t* sp) {
   for (size_t i = 0; i < buf_len(scope->vars); ++i) {
      scope->vars[i].addr = *sp;
      *sp += sizeof_value(scope->vars[i].type, false);
   }
   for (size_t i = 0; i < buf_len(scope->children); ++i) {
      assign_scope(scope->children[i], sp);
   }
}
