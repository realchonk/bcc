#ifndef FILE_UNIT_H
#define FILE_UNIT_H
#include "func.h"

enum symbol_type {
   SYM_VAR,
   SYM_FUNC,
   SYM_ALIAS,
   SYM_CONST,
};
struct symbol {
   enum symbol_type type;
   size_t idx;
};

struct typerename {
   struct source_pos begin, end;
   struct value_type* type;
   istr_t name;
};

struct cunit {
   struct function** funcs;
   struct variable* vars;
   struct typerename* aliases;
   struct enumeration** enums;
   struct enum_entry* constants;
   struct structure** structs;
   struct structure** unions;
};

extern struct cunit cunit;

void parse_unit(bool gen_ir);
void print_unit(FILE*);
void print_ir_unit(FILE*);
struct function* unit_get_func(const char*);
struct variable* unit_get_var(istr_t);
struct typerename* unit_get_typedef(istr_t);
void free_unit(void);
bool find_constant(istr_t, intmax_t*);
struct enumeration* unit_get_enum(istr_t);
struct structure* unit_get_struct(istr_t);
struct structure* unit_get_union(istr_t);
size_t unit_get_var_idx(istr_t);
size_t unit_get_func_idx(istr_t);
size_t unit_get_typedef_idx(istr_t);
size_t unit_get_const_idx(istr_t);
bool unit_find(istr_t, struct symbol*);
void unit_add_enum(const struct value_type*);
void unit_add_union(const struct value_type*);
void unit_add_struct(const struct value_type*);

#endif /* FILE_UNIT_H */
