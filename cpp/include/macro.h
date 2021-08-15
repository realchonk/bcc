#ifndef FILE_MACRO_H
#define FILE_MACRO_H
#include <stdbool.h>
#include "strint.h"
#include "buf.h"

struct macro {
   istr_t name;
   bool is_func;
   const char* text;
   istr_t* params;
   size_t linenum;
};

void add_macro(const struct macro*);
bool remove_macro(istr_t);
const struct macro* get_macro(istr_t);
void add_cmdline_macro(const char* arg);

#endif /* FILE_MACRO_H */
