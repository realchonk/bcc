#ifndef FILE_OPTIM_H
#define FILE_OPTIM_H
#include "expr.h"
#include "stmt.h"
#include "ir.h"

struct expression* optim_expr(struct expression*);
struct statement* optim_stmt(struct statement*);
struct ir_node* optim_ir_nodes(struct ir_node*);

#endif /* FILE_OPTIM_H */
