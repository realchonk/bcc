#ifndef FILE_UNIT_H
#define FILE_UNIT_H
#include "func.h"

struct compilation_unit {
   struct function** funcs;
};

struct compilation_unit* parse_unit(void);
void print_unit(FILE*, const struct compilation_unit*);
void print_ir_unit(FILE*, const struct compilation_unit*);
void free_unit(struct compilation_unit*);

#endif /* FILE_UNIT_H */
