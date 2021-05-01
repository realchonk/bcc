#ifndef FILE_EXPR_H
#define FILE_EXPR_H
#include <stdbool.h>
#include "token.h"
#include "buf.h"

enum expression_type {
   EXPR_PAREN,
   EXPR_INT,
   EXPR_UINT,
   EXPR_FLOAT,
   EXPR_CHAR,
   EXPR_STRING,
   EXPR_NAME,
   EXPR_UNARY,
   EXPR_BINARY,
   EXPR_PREFIX,
   EXPR_SUFFIX,
   EXPR_ADDROF,
   EXPR_INDIRECT,
   EXPR_TERNARY,
   EXPR_ASSIGN,
   EXPR_COMMA,
   EXPR_CAST,
   EXPR_FCALL,

   NUM_EXPRS,
};
extern const char* expr_type_str[NUM_EXPRS];
struct value_type;

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
   };
};

struct expression* parse_expr(void);

struct expression* new_expr(void);
void print_expr_tree(FILE*, const struct expression*);
void print_expr(FILE*, const struct expression*);

void free_expr(struct expression*);

bool expr_is_lvalue(const struct expression*);

#endif /* FILE_EXPR_H */
