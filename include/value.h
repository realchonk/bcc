#ifndef FILE_VALUE_H
#define FILE_VALUE_H
#include <stdbool.h>
#include <stdio.h>
#include "token.h"


enum attribute {
   ATTR_EXTERN    = 0x0001,
   ATTR_STATIC    = 0x0002,
};

enum value_base_type {
   VAL_INT,
   VAL_FLOAT,
   VAL_POINTER,
   VAL_VOID,
   VAL_AUTO,
   VAL_ENUM,

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

struct enum_entry {
   istr_t name;
   intmax_t value;
};
struct enumeration {
   istr_t name;                  // optional
   struct enum_entry* entries;
   bool is_definition;
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
         bool is_array;
         struct {
            bool has_const_size;
            union {
               size_t size;              // if  has_const_size
               struct expression* dsize; // if !has_const_size;
            };
         } array;
      } pointer;
      struct enumeration* venum;
   };
};

struct value {
   struct source_pos begin, end;
   struct value_type* type;
   union {
      istr_t sVal;
      intmax_t iVal;
      uintmax_t uVal;
      fpmax_t fVal;
   };
};

extern const char* integer_size_str[NUM_INTS];
extern const char* fp_size_str[NUM_FPS];
extern const char* value_type_str[NUM_VALS];
struct expression;
struct scope;

struct value_type* parse_value_type(struct scope*);
void free_value_type(struct value_type*);
void print_value_type(FILE*, const struct value_type*);
struct value_type* get_value_type(struct scope*, const struct expression*);
struct value_type* common_value_type_free(struct value_type*, struct value_type*, bool warn);
struct value_type* common_value_type(const struct value_type*, const struct value_type*, bool warn);
struct value_type* copy_value_type(const struct value_type*);
struct value_type* make_array_vt(struct value_type*);
bool try_eval_expr(const struct expression*, struct value*);
size_t sizeof_value(const struct value_type*, bool decay);
struct value_type* decay(struct value_type*);
bool var_is_declared(istr_t, struct scope*);
bool is_castable(const struct value_type* old, const struct value_type* type, bool implicit);
struct enumeration* copy_enum(const struct enumeration*);
struct value_type* make_int(enum integer_size sz, bool is_unsigned);

#endif /* FILE_VALUE_H */
