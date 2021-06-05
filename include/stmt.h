#ifndef FILE_STMT_H
#define FILE_STMT_H
#include <stdbool.h>
#include "scope.h"
#include "expr.h"
#include "buf.h"

enum statement_type {
   STMT_NOP,         //             | no operation
   STMT_EXPR,        // .expr       | simple expression-statement
   STMT_RETURN,      // .expr?      | return with optional value
   STMT_IF,          // .ifstmt     | if with optional else
   STMT_WHILE,       // .whileloop  | while-loop (also for-loop)
   STMT_DO_WHILE,    // .whileloop  | do-while loop
   STMT_VARDECL,     // .var_decl   | variable declaration
   STMT_SCOPE,       // .scope      | compount statement
   STMT_BREAK,       //             | exit a loop
   STMT_CONTINUE,    //             | skip the current iteration in a loop
   STMT_SWITCH,      // .sw         | switch/case

   NUM_STMTS,
};
extern const char* stmt_type_str[NUM_STMTS];
struct function;
struct switch_entry;

struct statement {
   enum statement_type type;
   struct source_pos begin, end;
   struct scope* parent;
   struct function* func;
   union {
      struct expression* expr;
      struct scope* scope;
      struct {
         struct expression* cond;
         struct statement* true_case;
         struct statement* false_case; // optional
      } ifstmt;
      struct {
         struct expression* cond;
         struct statement* stmt;
         struct expression* end; // optional, unused for do-while
      } whileloop;
      struct {
         struct value_type* type;
         size_t idx, num;
      } var_decl;
      struct {
         struct expression* expr;
         struct switch_entry* body;
      } sw;
   };
};

enum switch_entry_type {
   SWITCH_STMT,
   SWITCH_CASE,
   SWITCH_DEFAULT,
};
struct switch_entry {
   struct source_pos begin, end;
   enum switch_entry_type type; // 0=stmt, 1=case, 2=default
   union {
      struct statement* stmt;
      struct {
         struct expression* expr;
         struct value value;
      } cs;
   };
};

struct statement* parse_stmt(struct scope*);

void print_stmt(FILE*, const struct statement*);
void print_stmt_tree(FILE*, const struct statement*);
void free_stmt(struct statement*);
struct statement* new_stmt(void);
bool stmt_is_pure(const struct statement*);

#endif /* FILE_STMT_H */
