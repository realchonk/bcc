#ifndef FILE_STMT_H
#define FILE_STMT_H
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
   STMT_COMPOUND,

   NUM_STMTS,
};
extern const char* stmt_type_str[NUM_STMTS];

struct statement {
   enum statement_type type;
   struct source_pos begin, end;
   union {
      struct expression* expr;
      struct statement** stmts;
      struct {
         struct expression* cond;
         struct statement* true_case;
         struct statement* false_case;
      } ifstmt;
      struct {
         struct expression* cond;
         struct statement* stmt;
      } whileloop;
      struct {
         struct statement* init;
         struct expression* cond;
         struct expression* end;
         struct statement* stmt;
      } forloop;
   };
};

struct statement* parse_stmt(void);

void print_stmt(FILE*, const struct statement*);
void print_stmt_tree(FILE*, const struct statement*);

void free_stmt(struct statement*);

#endif /* FILE_STMT_H */
