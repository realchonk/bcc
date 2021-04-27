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
};

extern unsigned asm_indent;

void emit_init(FILE*);
void emit_free(void);

void emit(const char*, ...);

ir_node_t* emit_ir(const ir_node_t*);

#endif /* FILE_TARGET_H */
