#ifndef FILE_TARGET_H
#define FILE_TARGET_H
#include <stdio.h>
#include "ir.h"

struct target_info {
   const char* name;
   size_t size_byte;
   size_t size_char;
   size_t size_short;
   size_t size_int;
   size_t size_long;
   size_t size_float;
   size_t size_double;
   size_t size_pointer;

   intmax_t min_byte;
   intmax_t max_byte;
   uintmax_t max_ubyte;

   intmax_t min_char;
   intmax_t max_char;
   uintmax_t max_uchar;

   intmax_t min_short;
   intmax_t max_short;
   uintmax_t max_ushort;

   intmax_t min_int;
   intmax_t max_int;
   uintmax_t max_uint;

   intmax_t min_long;
   intmax_t max_long;
   uintmax_t max_ulong;
   
   bool unsigned_char;
};

extern const struct target_info target_info;
extern unsigned asm_indent;

void emit_init(FILE*);
void emit_free(void);

void emit(const char*, ...);

ir_node_t* emit_ir(const ir_node_t*);

#endif /* FILE_TARGET_H */
