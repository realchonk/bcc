#ifndef FILE_TARGET_H
#define FILE_TARGET_H
#include <stdio.h>
#include "value.h"
#include "ir.h"

struct target_info {
   const char* const name;
   const size_t size_byte;
   const size_t size_char;
   const size_t size_short;
   const size_t size_int;
   const size_t size_long;
   const size_t size_float;
   const size_t size_double;
   const size_t size_pointer;

   const intmax_t min_byte;
   const intmax_t max_byte;
   const uintmax_t max_ubyte;

   const intmax_t min_char;
   const intmax_t max_char;
   const uintmax_t max_uchar;

   const intmax_t min_short;
   const intmax_t max_short;
   const uintmax_t max_ushort;

   const intmax_t min_int;
   const intmax_t max_int;
   const uintmax_t max_uint;

   const intmax_t min_long;
   const intmax_t max_long;
   const uintmax_t max_ulong;
   
   const bool unsigned_char;

   const char* const fend_asm;
   const char* const fend_obj;

   const enum integer_size ptrdiff_type;
   const bool has_c99_array; // variable-length array support?

   enum integer_size size_int8;
   enum integer_size size_int16;
   enum integer_size size_int32;
   enum integer_size size_int64;
};

struct builtin_func {
   const char* name;
   const char* code;
   bool requested;
};

extern const struct target_info target_info;
extern unsigned asm_indent;
extern struct builtin_func builtin_funcs[];
extern const size_t num_builtin_funcs;
struct function;

void emit_init(FILE*);
void emit_free(void);
void emit(const char*, ...);
void emitraw(const char*, ...);
void emit_unit(const char**);
int assemble(const char* source, const char* output);
uintmax_t target_get_umax(enum ir_value_size);
bool is_builtin_func(const char*);
void reset_buitins(void);
size_t irs2sz(enum ir_value_size);
void request_builtin(const char* name);
const struct value_type* get_builtin_type(istr_t name);
void add_builtin_type(const char* name, struct value_type*);

#endif /* FILE_TARGET_H */
