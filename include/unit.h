#ifndef FILE_UNIT_H
#define FILE_UNIT_H
#include "func.h"

struct cunit {
   struct function** funcs;
};

extern struct cunit cunit;

void parse_unit(void);
void print_unit(FILE*);
void print_ir_unit(FILE*);
struct function* unit_get_func(const char*);
void free_unit(void);

#endif /* FILE_UNIT_H */
