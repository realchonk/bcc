#ifndef FILE_FUNC_H
#define FILE_FUNC_H
#include "strint.h"
#include "value.h"
#include "scope.h"

struct function {
   istr_t name;
   struct value_type* type;
   struct variable* params;
   struct scope* scope;
   struct source_pos begin, end;
};

struct function* parse_func(void);

void print_func(FILE*, const struct function*);
void free_func(struct function*);

size_t func_find_var_idx(const struct function*, const char*);
const struct variable* func_find_var(const struct function*, const char*);

#endif /* FILE_FUNC_H */
