#ifndef FILE_SCOPE_H
#define FILE_SCOPE_H
#include "value.h"
#include "buf.h"

struct expression;
struct statement;

struct variable {
   struct value_type* type;
   istr_t name;
   struct expression* init;
   struct source_pos begin, end;
   uintmax_t addr; // used by backend
   unsigned attrs;
   bool has_const_value;
   struct value const_init;
};

struct scope {
   struct scope* parent;
   struct scope** children;
   struct variable* vars;
   struct statement** body;
   struct function* func;
};

struct scope* make_scope(struct scope* parent, struct function* func);
void print_scope(FILE*, const struct scope*);
void free_scope(struct scope*);

const struct variable* scope_find_var(struct scope*, const char*);
size_t scope_find_var_idx(struct scope*, struct scope**, const char*);
size_t scope_add_var(struct scope*, const struct variable*);

#endif /* FILE_SCOPE_H */
