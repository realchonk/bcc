#ifndef FILE_VALUE_H
#define FILE_VALUE_H
#include <stdbool.h>
#include <stdio.h>
#include "token.h"

enum value_base_type {
   VAL_INT,
   VAL_FLOAT,
   VAL_POINTER,
   VAL_VOID,

   NUM_VALS,
};
enum integer_size {
   INT_BYTE,
   INT_CHAR,
   INT_SHORT,
   INT_INT,
   INT_LONG,

   NUM_INTS,
};
enum fp_size {
   FP_FLOAT,
   FP_DOUBLE,

   NUM_FPS,
};

struct value_type {
   enum value_base_type type;
   struct source_pos begin, end;
   bool is_const;
   union {
      struct {
         enum integer_size size;
         bool is_unsigned;
      } integer;
      struct {
         enum fp_size size;
      } fp;
      struct {
         struct value_type* type;
      } pointer;
   };
};

extern const char* integer_size_str[NUM_INTS];
extern const char* fp_size_str[NUM_FPS];
struct expression;
struct scope;

struct value_type* parse_value_type(void);
void free_value_type(struct value_type*);
void print_value_type(FILE*, const struct value_type*);
struct value_type* get_value_type(struct scope*, const struct expression*);
struct value_type* common_value_type_free(struct value_type*, struct value_type*, bool);
struct value_type* common_value_type(const struct value_type*, const struct value_type*, bool);
struct value_type* copy_value_type(const struct value_type*);

bool is_castable(const struct value_type* old, const struct value_type* type, bool implicit);

#endif /* FILE_VALUE_H */
