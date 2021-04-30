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

enum ir_value_size vt2irs(const struct value_type* vt) {
   switch (vt->type) {
   case VAL_INT:
      switch (vt->integer.size) {
      case INT_CHAR:    return IRS_CHAR;
      case INT_BYTE:    return IRS_BYTE;
      case INT_SHORT:   return IRS_SHORT;
      case INT_INT:     return IRS_INT;
      case INT_LONG:    return IRS_LONG;
      default:          panic("vt2irs(): invalid integer value '%d'", vt->integer.size);
      }
   case VAL_FLOAT:      panic("vt2irs(): floating-point values are currently not supported by the IR backend.");
   case VAL_POINTER:    return IRS_PTR;
   default:             panic("vt2irs(): invalid value type '%d'", vt->type);
   }
}

static ir_node_t* ir_expr(struct scope* scope, const struct expression* e);
static ir_node_t* ir_lvalue(struct scope* scope, const struct expression* e) {
   ir_node_t* n;
   size_t idx;
   switch (e->type) {
   case EXPR_NAME:
      n = new_node(IR_LOOKUP);
      idx = scope_find_var_idx(scope, &n->lookup.scope, e->str);
      if (idx == SIZE_MAX) {
         idx = func_find_var_idx(scope->func, e->str);
         if (idx == SIZE_MAX)
            parse_error(&e->begin, "undeclared variable '%s'", e->str);
         n->type = IR_FPARAM;
         n->fparam.reg = creg++;
         n->fparam.func = scope->func;
         n->fparam.idx = idx;
      } else {
         n->lookup.reg = creg++;
         n->lookup.var_idx = idx;
      }
      return n;
   case EXPR_INDIRECT:
      n = ir_expr(scope, e->expr);
      return n;

   default: panic("ir_lvalue(): unsupported expression '%s'", expr_type_str[e->type]);
   }
}
static ir_node_t* ir_expr(struct scope* scope, const struct expression* e) {
   struct value_type* vt = get_value_type(scope, e);
   const enum ir_value_size irs = vt2irs(vt);
   free_value_type(vt);
   ir_node_t* n;
   ir_node_t* tmp;
   switch (e->type) {
   case EXPR_PAREN:  return ir_expr(scope, e->expr);
   case EXPR_UINT:
   case EXPR_INT:
      n = new_node(IR_LOAD);
      n->load.dest = creg++;
      n->load.value = e->uVal;
      n->load.size = irs;
      return n;
   case EXPR_BINARY:
      n = ir_expr(scope, e->binary.left);
      ir_append(n, ir_expr(scope, e->binary.right));
      tmp = new_node(IR_NOP);
      tmp->binary.size = irs;
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
      n = ir_expr(scope, e->unary.expr);
      if (e->unary.op.type == TK_PLUS) return n;
      tmp = new_node(IR_NOP);
      tmp->unary.size = irs;
      tmp->unary.reg = creg - 1;
      switch (e->unary.op.type) {
      case TK_MINUS: tmp->type = IR_INEG; break;
      case TK_NOT:   tmp->type = IR_BNOT; break;
      case TK_WAVE:  tmp->type = IR_INOT; break;
      default:       panic("ir_expr(): unsupported unary operator '%s'", token_type_str[e->unary.op.type]);
      }
      ir_append(n, tmp);
      return n;
   case EXPR_NAME:
   case EXPR_INDIRECT:
      n = ir_lvalue(scope, e);
      tmp = new_node(IR_READ);
      tmp->move.dest = tmp->move.src = creg - 1;
      tmp->move.size = irs;
      return ir_append(n, tmp);
   case EXPR_ASSIGN:
      n = ir_expr(scope, e->binary.right);
      ir_append(n, ir_lvalue(scope, e->binary.left));
      if (e->binary.op.type != TK_EQ) {
         tmp = new_node(IR_READ);
         tmp->move.size = irs;
         tmp->move.dest = creg;
         tmp->move.src = creg - 1;
         ir_append(n, tmp);
         tmp = new_node(IR_NOP);
         tmp->binary.dest = creg - 2;
         tmp->binary.a = creg;
         tmp->binary.b = creg - 2;
         tmp->binary.size = irs;
         switch (e->binary.op.type) {
         case TK_PLEQ:     tmp->type = IR_IADD; break;
         case TK_MIEQ:     tmp->type = IR_ISUB; break;
         case TK_STEQ:     tmp->type = IR_IMUL; break;
         case TK_SLEQ:     tmp->type = IR_IDIV; break;
         case TK_PERCEQ:   tmp->type = IR_IMOD; break;
         case TK_AMPEQ:    tmp->type = IR_IAND; break;
         case TK_PIPEEQ:   tmp->type = IR_IOR; break;
         case TK_XOREQ:    tmp->type = IR_IXOR; break;
         case TK_GRGREQ:   tmp->type = IR_ILSL; break;
         case TK_LELEEQ:   tmp->type = IR_ILSR; break;
         default:          parse_error(&e->binary.op.begin, "invalid assignment operator '%s'", token_type_str[e->binary.op.type]);
         }
         ir_append(n, tmp);
      }
      tmp = new_node(IR_WRITE);
      tmp->move.size = irs;
      tmp->move.dest = creg - 1;
      tmp->move.src = creg - 2;
      --creg;
      return ir_append(n, tmp);
   case EXPR_ADDROF:
      return ir_lvalue(scope, e->expr);
   case EXPR_CAST:
   {
      struct value_type* svt = get_value_type(scope, e->cast.expr);
      n = ir_expr(scope, e->cast.expr);
      tmp = new_node(IR_IICAST);
      tmp->iicast.dest = tmp->iicast.src = creg - 1;
      tmp->iicast.ds = irs;
      tmp->iicast.ss = vt2irs(svt);
      free_value_type(svt);
      return ir_append(n, tmp);
   }
   case EXPR_FCALL:
      n = new_node(IR_IFCALL);
      n->ifcall.name = e->fcall.name;
      n->ifcall.dest = creg++;
      n->ifcall.params = NULL;

      for (size_t i = 0; i < buf_len(e->fcall.params); ++i) {
         buf_push(n->ifcall.params, ir_expr(scope, e->fcall.params[i]));
         --creg;
      }
      return n;
   default:
      panic("ir_expr(): unsupported expression '%s'", expr_type_str[e->type]);
   }
}

ir_node_t* irgen_expr(struct scope* scope, const struct expression* expr) {
   creg = 0;
   return ir_expr(scope, expr);
}

static ir_node_t* ir_stmt(const struct statement* s) {
   ir_node_t* n;
   ir_node_t* tmp;
   switch (s->type) {
   case STMT_NOP: return new_node(IR_NOP);
   case STMT_EXPR:return ir_expr(s->parent, s->expr);
   case STMT_RETURN:
      tmp = new_node(IR_RET);
      if (s->expr) {
         n = ir_expr(s->parent, s->expr);
         tmp->type = IR_IRET;
         tmp->unary.size = IRS_INT; // TODO
         tmp->unary.reg = creg - 1;
         ir_append(n, tmp);
      } else n = tmp;
      return n;
   case STMT_SCOPE:
      n = new_node(IR_BEGIN_SCOPE);
      n->scope = s->scope;
      for (size_t i = 0; i < buf_len(s->scope->body); ++i)
         n = ir_append(n, ir_stmt(s->scope->body[i]));
      tmp = new_node(IR_END_SCOPE);
      tmp->scope = s->scope;
      return ir_append(n, tmp);
   case STMT_VARDECL:
      if (s->parent->vars[s->var_idx].init) {
         n = ir_expr(s->parent, s->parent->vars[s->var_idx].init);
         tmp = new_node(IR_LOOKUP);
         tmp->lookup.reg = creg;
         tmp->lookup.scope = s->parent;
         tmp->lookup.var_idx = s->var_idx;
         ir_append(n, tmp);

         tmp = new_node(IR_WRITE);
         tmp->move.size = vt2irs(s->parent->vars[s->var_idx].type);
         tmp->move.dest = creg;
         tmp->move.src = creg - 1;
         ir_append(n, tmp);
         --creg;
      } else n = new_node(IR_NOP);
      return n;
      

   default: panic("ir_stmt(): unsupported statement '%s'", stmt_type_str[s->type]);
   }
}

ir_node_t* irgen_stmt(const struct statement* s) {
   creg = 0;
   return ir_stmt(s);
}

ir_node_t* irgen_func(const struct function* f) {
   ir_node_t* tmp;
   ir_node_t* n = new_node(IR_PROLOGUE);
   n->func = f;

   tmp = new_node(IR_BEGIN_SCOPE);
   tmp->scope = f->scope;
   ir_append(n, tmp);

   for (size_t i = 0; i < buf_len(f->scope->body); ++i) {
      ir_append(n, irgen_stmt(f->scope->body[i]));
   }
   
   tmp = new_node(IR_END_SCOPE);
   tmp->scope = f->scope;
   ir_append(n, tmp);

   tmp = new_node(IR_EPILOGUE);
   tmp->func = f;
   return ir_append(n, tmp);
}

