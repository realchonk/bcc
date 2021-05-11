#ifndef FILE_EXPR_H
#define FILE_EXPR_H
#include <stdbool.h>
#include "token.h"
#include "buf.h"

enum expression_type {
   EXPR_PAREN,       // .expr       | sub-expression
   EXPR_INT,         // .iVal       | signed integer literal
   EXPR_UINT,        // .uVal       | unsigned integer literal
   EXPR_FLOAT,       // .fVal       | floating-point literal
   EXPR_CHAR,        // .ch         | character literal
   EXPR_STRING,      // .str        | string literal
   EXPR_NAME,        // .str        | identifier
   EXPR_UNARY,       // .unary      | unary expression
   EXPR_BINARY,      // .binary     | binary expression
   EXPR_PREFIX,      // .unary      | prefix increment/decrement
   EXPR_SUFFIX,      // .unary      | suffix increment/decrement
   EXPR_ADDROF,      // .expr       | address-of
   EXPR_INDIRECT,    // .expr       | dereference/indirection
   EXPR_TERNARY,     // .ternary    | ternary/if-else-expression
   EXPR_ASSIGN,      // .binary     | assignment
   EXPR_COMMA,       // .comma      | comma/multi-expression
   EXPR_CAST,        // .cast       | type conversion
   EXPR_FCALL,       // .fcall      | function call
   EXPR_SIZEOF,      // .szof       | sizeof operator
   EXPR_ARRAYLEN,    // .expr       | arraylen operator

   NUM_EXPRS,
};
extern const char* expr_type_str[NUM_EXPRS];
struct value_type;
struct scope;

struct expression {
   enum expression_type type;
   struct source_pos begin, end;
   union {
      struct expression* expr;
      struct expression** comma;
      uintmax_t uVal;
      intmax_t iVal;
      fpmax_t fVal;
      istr_t str;
      int ch;
      struct {
         struct token op;
         struct expression* expr;
      } unary;
      struct {
         struct token op;
         struct expression* left;
         struct expression* right;
      } binary;
      struct {
         struct expression* cond;
         struct expression* true_case;
         struct expression* false_case;
      } ternary;
      struct {
         struct value_type* type;
         struct expression* expr;
      } cast;
      struct {
         const char* name;
         struct expression** params;
      } fcall;
      struct {
         struct expression* base;
         struct expression* index;
      } subs;
      struct {
         struct expression* left;
         struct expression* right;
      } assign;
      struct {
         bool has_expr;
         union {
            struct expression* expr;
            struct value_type* type;
         };
      } szof;
   };
};

struct expression* parse_expr(struct scope*);
struct expression* parse_expr_no_comma(struct scope*);

struct expression* new_expr(void);
void print_expr_tree(FILE*, const struct expression*);
void print_expr(FILE*, const struct expression*);

void free_expr(struct expression*);

bool expr_is_lvalue(const struct expression*);
bool expr_is_pure(const struct expression*);

struct value parse_const_expr(void);

#endif /* FILE_EXPR_H */
