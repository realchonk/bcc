#ifndef FILE_STMT_H
#define FILE_STMT_H
#include <stdbool.h>
#include "scope.h"
#include "expr.h"
#include "buf.h"

enum statement_type {
   STMT_NOP,
   STMT_EXPR,
   STMT_RETURN,
   STMT_IF,
   STMT_WHILE,
   STMT_DO_WHILE,
   STMT_FOR,
   STMT_VARDECL,
   STMT_SCOPE,

   NUM_STMTS,
};
extern const char* stmt_type_str[NUM_STMTS];
struct function;

struct statement {
   enum statement_type type;
   struct source_pos begin, end;
   struct scope* parent;
   struct function* func;
   union {
      struct expression* expr;
      struct scope* scope;
      size_t var_idx;
      struct {
         struct expression* cond;
         struct statement* true_case;
         struct statement* false_case; // optional
      } ifstmt;
      struct {
         struct expression* cond;
         struct statement* stmt;
      } whileloop;
      struct {
         struct statement* init; // optional
         struct expression* cond; // optional
         struct expression* end; // optional
         struct statement* stmt;
      } forloop;
   };
};

struct statement* parse_stmt(struct scope*);

void print_stmt(FILE*, const struct statement*);
void print_stmt_tree(FILE*, const struct statement*);

void free_stmt(struct statement*);

#endif /* FILE_STMT_H */
