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

#endif /* FILE_FUNC_H */
