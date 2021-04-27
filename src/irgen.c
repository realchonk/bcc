#include "error.h"
#include "ir.h"

static ir_node_t* new_node(enum ir_node_type t) {
   ir_node_t* n = malloc(sizeof(ir_node_t));
   if (!n) panic("failed to allocate ir_node");
   n->type = t;
   n->prev = n->next = NULL;
   return n;
}

static ir_reg_t creg = 0;


static ir_node_t* ir_expr(const struct expression* e) {
   ir_node_t* n;
   ir_node_t* tmp;
   switch (e->type) {
   case EXPR_PAREN:  return ir_expr(e->expr);
   case EXPR_UINT:
   case EXPR_INT:
      n = new_node(IR_LOAD);
      n->load.dest = creg++;
      n->load.value = e->uVal;
      n->load.size = IRS_INT; // TODO
      return n;
   case EXPR_BINARY:
      n = ir_expr(e->binary.left);
      ir_append(n, ir_expr(e->binary.right));
      tmp = new_node(IR_NOP);
      tmp->binary.size = IRS_INT; // TODO
      tmp->binary.dest = tmp->binary.a = creg - 2;
      tmp->binary.b = creg - 1;
      switch (e->binary.op.type) {
      case TK_PLUS:  tmp->type = IR_IADD; break;
      case TK_MINUS: tmp->type = IR_ISUB; break;
      case TK_STAR:  tmp->type = IR_IMUL; break;
      case TK_SLASH: tmp->type = IR_IDIV; break;
      case TK_PERC:  tmp->type = IR_IMOD; break;
      case TK_GRGR:  tmp->type = IR_ILSL; break;
      case TK_LELE:  tmp->type = IR_ILSR; break;
      default:       panic("ir_expr(): unsupported binary operator '%s'", token_type_str[e->binary.op.type]);
      }
      ir_append(n, tmp);
      --creg;
      return n;
   case EXPR_UNARY:
      n = ir_expr(e->unary.expr);
      if (e->unary.op.type == TK_PLUS) return n;
      tmp = new_node(IR_NOP);
      tmp->unary.size = IRS_INT; // TODO
      tmp->unary.reg = creg - 1;
      switch (e->unary.op.type) {
      case TK_MINUS: tmp->type = IR_INEG; break;
      case TK_NOT:   tmp->type = IR_BNOT; break;
      case TK_WAVE:  tmp->type = IR_INOT; break;
      default:       panic("ir_expr(): unsupported unary operator '%s'", token_type_str[e->unary.op.type]);
      }
      ir_append(n, tmp);
      return n;
   default:
      panic("ir_expr(): unsupported expression '%s'", expr_type_str[e->type]);
   }
}

ir_node_t* irgen_expr(const struct expression* expr) {
   creg = 0;
   return ir_expr(expr);
}
