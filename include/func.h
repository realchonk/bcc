#ifndef FILE_FUNC_H
#define FILE_FUNC_H
#include "strint.h"
#include "value.h"
#include "scope.h"

struct ir_node;

struct function {
   istr_t name;
   struct value_type* type;
   struct variable* params;
   struct scope* scope;             // optional
   struct source_pos begin, end;
   struct ir_node* ir_code;         // optional
   bool variadic;
   unsigned attrs;
};

void parse_func_part(struct function*);
void print_func(FILE*, const struct function*);
void free_func(struct function*);

size_t func_find_param_idx(const struct function*, const char*);
const struct variable* func_find_param(const struct function*, const char*);

#endif /* FILE_FUNC_H */
