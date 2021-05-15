#include <tgmath.h>
#include <string.h>
#include "target.h"
#include "error.h"
#include "optim.h"
#include "ir.h"

static ir_node_t* new_node(enum ir_node_type t) {
   ir_node_t* n = malloc(sizeof(ir_node_t));
   if (!n) panic("failed to allocate ir_node");
   n->type = t;
   n->prev = n->next = NULL;
   return n;
}

static ir_reg_t creg = 0;
static size_t clbl = 0;
static istr_t begin_loop = NULL, end_loop = NULL;

static istr_t make_label(size_t i) {
   if (!i) return strint(".L0");
   else if (i == 1) return strint(".L1");
   const size_t len = log10(i) + 4;
   char buffer[len + 1];
   snprintf(buffer, len, ".L%zu", i);
   buffer[len] = '\0';
   return strint(buffer);
}

enum ir_value_size vt2irs(const struct value_type* vt) {
   switch (vt->type) {
   case VAL_VOID:
      return IRS_VOID;
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
   case VAL_ENUM:       return IRS_INT;
   default:             panic("vt2irs(): invalid value type '%d'", vt->type);
   }
}

static ir_node_t* ir_expr(struct scope* scope, const struct expression* e);
static ir_node_t* ir_lvalue(struct scope* scope, const struct expression* e, bool* is_lv) {
   ir_node_t* n;
   size_t idx;
   *is_lv = true;
   switch (e->type) {
   case EXPR_NAME:
      n = new_node(IR_LOOKUP);
      idx = scope_find_var_idx(scope, &n->lookup.scope, e->str);
      if (idx == SIZE_MAX) {
         idx = func_find_param_idx(scope->func, e->str);
         if (idx == SIZE_MAX) {
            struct variable* var = unit_get_var(e->str);
            if (!var) {
               intmax_t val;
               if (find_constant(e->str, &val)) {
                  n->type = IR_LOAD;
                  n->load.dest = creg++;
                  n->load.value = val;
                  n->load.size = IRS_INT;
                  *is_lv = false;
                  return n;
               } else parse_error(&e->begin, "undeclared variable '%s'", e->str);
            }
            n->type = IR_GLOOKUP;
            n->lstr.str = e->str;
            n->lstr.reg = creg++;
         } else {
            n->type = IR_FPARAM;
            n->fparam.reg = creg++;
            n->fparam.func = scope->func;
            n->fparam.idx = idx;
         }
      } else {
         n->lookup.reg = creg++;
         n->lookup.var_idx = idx;
      }
      return n;
   case EXPR_INDIRECT:
      n = ir_expr(scope, e->expr);
      return n;
   case EXPR_MEMBER:
   {
      struct value_type* vt = e->member.base->vtype;
      struct structure* st = real_struct(vt->vstruct);
      bool is_lv2;

      n = ir_lvalue(scope, e->member.base, &is_lv2);
      ir_node_t* tmp = new_node(IR_IADD);
      tmp->binary.dest = creg - 1;
      tmp->binary.a.type = IRT_REG;
      tmp->binary.a.reg = creg - 1;
      tmp->binary.b.type = IRT_UINT;
      tmp->binary.b.uVal = addrof_member(st, struct_get_member_idx(st, e->member.name));
      tmp->binary.size = IRS_PTR;
      ir_append(n, tmp);
      return n;
   }

   default: panic("ir_lvalue(): unsupported expression '%s'", expr_type_str[e->type]);
   }
}
static ir_node_t* ir_expr(struct scope* scope, const struct expression* e) {
   struct value_type* vt = e->vtype;
   const enum ir_value_size irs = vt2irs(vt);
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
      break;
   case EXPR_CHAR:
      n = new_node(IR_LOAD);
      n->load.dest = creg++;
      n->load.value = e->uVal;
      n->load.size = IRS_CHAR;
      break;
   case EXPR_BINARY:
   {
      const bool is_unsigned = vt->type == VAL_POINTER || (vt->type == VAL_INT && vt->integer.is_unsigned);
      struct value_type* vl = e->binary.left->vtype;
      struct value_type* vr = e->binary.right->vtype;
      n = ir_expr(scope, e->binary.left);
      if (e->binary.op.type == TK_PIPI || e->binary.op.type == TK_AMPAMP) {
         const istr_t lbl = make_label(clbl++);
         n = ir_expr(scope, e->binary.left);
         tmp = new_node(e->binary.op.type == TK_PIPI ? IR_JMPIF : IR_JMPIFN);
         tmp->cjmp.label = lbl;
         tmp->cjmp.reg = --creg;
         tmp->cjmp.size = vt2irs(vl);
         ir_append(n, tmp);

         ir_append(n, ir_expr(scope, e->binary.right));
         tmp = new_node(IR_LABEL);
         tmp->str = lbl;
         ir_append(n, tmp);
         break;
      }
      
      if (irs != vt2irs(vl)) {
         tmp = new_node(IR_IICAST);
         tmp->iicast.dest = tmp->iicast.src = creg - 1;
         tmp->iicast.ds = irs;
         tmp->iicast.ss = vt2irs(vl);
         tmp->iicast.sign_extend = !is_unsigned;
         ir_append(n, tmp);
      }
      ir_append(n, ir_expr(scope, e->binary.right));
      if (irs != vt2irs(vr)) {
         tmp = new_node(IR_IICAST);
         tmp->iicast.dest = tmp->iicast.src = creg - 1;
         tmp->iicast.ds = irs;
         tmp->iicast.ss = vt2irs(vr);
         tmp->iicast.sign_extend = !is_unsigned;
         ir_append(n, tmp);
      }
      if (((vl->type == VAL_POINTER && vr->type == VAL_INT) || (vl->type == VAL_INT && vr->type == VAL_POINTER))
            && (e->binary.op.type == TK_PLUS || e->binary.op.type == TK_MINUS)) {
         tmp = new_node(IR_IMUL);
         tmp->binary.dest = creg - 1;
         tmp->binary.a.type = IRT_REG;
         tmp->binary.a.reg = creg - 1;
         tmp->binary.b.type = IRT_UINT;
         tmp->binary.b.uVal = sizeof_value((vl->type == VAL_POINTER ? vl : vr)->pointer.type, false);
         tmp->binary.size = irs;
         ir_append(n, tmp);
      }
      
      tmp = new_node(IR_NOP);
      tmp->binary.size = irs;
      tmp->binary.dest = creg - 2;
      tmp->binary.a = irv_reg(creg - 2);
      tmp->binary.b = irv_reg(creg - 1);
      switch (e->binary.op.type) {
      case TK_PLUS:  tmp->type = IR_IADD; break;
      case TK_MINUS: tmp->type = IR_ISUB; break;
      case TK_STAR:  tmp->type = is_unsigned ? IR_UMUL : IR_IMUL; break;
      case TK_SLASH: tmp->type = is_unsigned ? IR_UDIV : IR_IDIV; break;
      case TK_PERC:  tmp->type = is_unsigned ? IR_UMOD : IR_IMOD; break;
      case TK_GRGR:  tmp->type = IR_ILSL; break;
      case TK_LELE:  tmp->type = IR_ILSR; break;
      case TK_EQEQ:  tmp->type = IR_ISTEQ; break;
      case TK_NEQ:   tmp->type = IR_ISTNE; break;
      case TK_GR:    tmp->type = is_unsigned ? IR_USTGR : IR_ISTGR; break;
      case TK_GREQ:  tmp->type = is_unsigned ? IR_USTGE : IR_ISTGE; break;
      case TK_LE:    tmp->type = is_unsigned ? IR_USTLT : IR_ISTLT; break;
      case TK_LEEQ:  tmp->type = is_unsigned ? IR_USTLE : IR_ISTLE; break;
      case TK_AMP:   tmp->type = IR_IAND; break;
      case TK_PIPE:  tmp->type = IR_IOR; break;
      case TK_XOR:   tmp->type = IR_IXOR; break;
      default:       panic("ir_expr(): unsupported binary operator '%s'", token_type_str[e->binary.op.type]);
      }
      ir_append(n, tmp);

      if (vl->type == VAL_POINTER && vr->type == VAL_POINTER && e->binary.op.type == TK_MINUS) {
         tmp = new_node(IR_IDIV);
         tmp->binary.size = irs;
         tmp->binary.dest = creg - 2;
         tmp->binary.a.type = IRT_REG;
         tmp->binary.a.reg = creg - 2;
         tmp->binary.b.type = IRT_UINT;
         tmp->binary.b.uVal = sizeof_value(vl->pointer.type, false);
         ir_append(n, tmp);
      }

      --creg;
      break;
   }
   case EXPR_UNARY:
      n = ir_expr(scope, e->unary.expr);
      if (e->unary.op.type == TK_PLUS) break;
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
      break;
   case EXPR_NAME:
   case EXPR_MEMBER:
   {
      bool is_lv;
      n = ir_lvalue(scope, e, &is_lv);
      if (is_lv) {
         tmp = new_node(IR_READ);
         tmp->move.dest = tmp->move.src = creg - 1;
         tmp->move.size = irs;
         ir_append(n, tmp);
      }
      break;
   }
   case EXPR_INDIRECT:
   {
      bool is_lv;
      n = ir_lvalue(scope, e, &is_lv);
      if (!is_lv) parse_error(&e->begin, "expected lvalue");
      tmp = new_node(IR_READ);
      tmp->move.dest = tmp->move.src = creg - 1;
      tmp->move.size = irs;
      ir_append(n, tmp);
      break;
   }
   case EXPR_ASSIGN:
   {
      bool is_lv;
      n = ir_expr(scope, e->assign.right);
      tmp = ir_lvalue(scope, e->assign.left, &is_lv);
      if (!is_lv) parse_error(&e->binary.op.begin, "expected lvalue");
      ir_append(n, tmp);
      tmp = new_node(IR_WRITE);
      tmp->move.size = irs;
      tmp->move.dest = creg - 1;
      tmp->move.src = creg - 2;
      --creg;
      ir_append(n, tmp);
      break;
   }
   case EXPR_ADDROF:
   {
      bool is_lv;
      n = ir_lvalue(scope, e->expr, &is_lv);
      if (!is_lv) parse_error(&e->begin, "expected lvalue");
      struct value_type* ve = get_value_type(scope, e->expr);  
      if (ve->type == VAL_POINTER && ve->pointer.is_array) {
         tmp = new_node(IR_READ);
         tmp->move.dest = tmp->move.src = creg - 1;
         tmp->move.size = IRS_PTR;
         ir_append(n, tmp);
      }
      break;
   }
   case EXPR_CAST:
   {
      struct value_type* svt = get_value_type(scope, e->cast.expr);
      if (svt->type != VAL_INT && svt->type != VAL_POINTER)
         parse_error(&e->begin, "unsupported cast");
      n = ir_expr(scope, e->cast.expr);
      tmp = new_node(IR_IICAST);
      tmp->iicast.dest = tmp->iicast.src = creg - 1;
      tmp->iicast.ds = irs;
      tmp->iicast.ss = vt2irs(svt);
      tmp->iicast.sign_extend = svt->type != VAL_POINTER && !svt->integer.is_unsigned;
      free_value_type(svt);
      ir_append(n, tmp);
      break;
   }
   case EXPR_FCALL:
   {
      struct function* func = unit_get_func(e->fcall.name);
      if (!func) parse_error(&e->begin, "function '%s' not found", e->fcall.name);
      const bool has_rv = func->type->type != VAL_VOID;
      n = new_node(IR_IFCALL);
      if (!has_rv) n ->type = IR_FCALL;
      n->ifcall.name = func->name;
      if (has_rv) n->ifcall.dest = creg;
      n->ifcall.params = NULL;

      for (size_t i = 0; i < buf_len(e->fcall.params); ++i) {
         struct expression* p = e->fcall.params[i];
         struct value_type* vp = get_value_type(scope, p);
         ir_node_t* ir = ir_expr(scope, p);
         enum ir_value_size irs = i < buf_len(func->params) ? vt2irs(func->params[i].type) : IRS_INT;
         if (vt2irs(vp) != irs) {
            tmp = new_node(IR_IICAST);
            tmp->iicast.dest = tmp->iicast.src = creg - 1;
            tmp->iicast.ds = irs;
            tmp->iicast.ss = vt2irs(vp);
            tmp->iicast.sign_extend = irs == IRS_PTR ? false : !func->type->integer.is_unsigned;
            ir_append(ir, tmp);
         }
         buf_push(n->ifcall.params, optim_ir_nodes(ir));
         --creg;

         free_value_type(vp);
      }
      ++creg;
      break;
   }
   case EXPR_STRING:
      n = new_node(IR_LSTR);
      n->lstr.reg = creg++;
      n->lstr.str = e->str;
      break;
   case EXPR_PREFIX:
   {
      bool is_lv;
      ++creg;
      n = ir_lvalue(scope, e->unary.expr, &is_lv);
      if (!is_lv) parse_error(&e->unary.expr->begin, "expected lvalue");
      --creg;

      tmp = new_node(IR_READ);
      tmp->move.dest = creg - 1;
      tmp->move.src = creg;
      tmp->move.size = irs;
      ir_append(n, tmp);

      tmp = new_node(e->unary.op.type == TK_PLPL ? IR_IADD : IR_ISUB);
      tmp->binary.dest = creg - 1;
      tmp->binary.a.type = IRT_REG;
      tmp->binary.a.reg = creg - 1;
      tmp->binary.b.type = IRT_UINT;
      tmp->binary.b.uVal = vt->type == VAL_POINTER ? sizeof_value(vt->pointer.type, false) : 1;
      tmp->binary.size = irs;
      ir_append(n, tmp);

      tmp = new_node(IR_WRITE);
      tmp->move.dest = creg;
      tmp->move.src = creg - 1;
      tmp->move.size = irs;
      ir_append(n, tmp);
      break;
   }
   case EXPR_SUFFIX:
   {
      bool is_lv;
      ++creg;
      n = ir_lvalue(scope, e->unary.expr, &is_lv);
      if (!is_lv) parse_error(&e->unary.expr->begin, "expected lvalue");
      --creg;

      tmp = new_node(IR_READ);
      tmp->move.dest = creg - 1;
      tmp->move.src = creg;
      tmp->move.size = irs;
      ir_append(n, tmp);

      tmp = new_node(e->unary.op.type == TK_PLPL ? IR_IADD : IR_ISUB);
      tmp->binary.dest = creg - 1;
      tmp->binary.a.type = IRT_REG;
      tmp->binary.a.reg = creg - 1;
      tmp->binary.b.type = IRT_UINT;
      tmp->binary.b.uVal = vt->type == VAL_POINTER ? sizeof_value(vt->pointer.type, false) : 1;
      tmp->binary.size = irs;
      ir_append(n, tmp);

      tmp = new_node(IR_WRITE);
      tmp->move.dest = creg;
      tmp->move.src = creg - 1;
      tmp->move.size = irs;
      ir_append(n, tmp);
      
      tmp = new_node(e->unary.op.type == TK_PLPL ? IR_ISUB : IR_IADD);
      tmp->binary.dest = creg - 1;
      tmp->binary.a.type = IRT_REG;
      tmp->binary.a.reg = creg - 1;
      tmp->binary.b.type = IRT_UINT;
      tmp->binary.b.uVal = vt->type == VAL_POINTER ? sizeof_value(vt->pointer.type, false) : 1;
      tmp->binary.size = irs;
      ir_append(n, tmp);
      break;
   }
   case EXPR_COMMA:
   {
      const size_t nreg = creg;
      n = NULL;
      for (size_t i = 0; i < buf_len(e->comma); ++i) {
         creg = nreg;
         n = ir_append(n, ir_expr(scope, e->comma[i]));
      }
      break;
   }
   case EXPR_TERNARY:
   {
      istr_t l1 = make_label(clbl);
      istr_t l2 = make_label(clbl + 1);
      clbl += 2;
      n = ir_expr(scope, e->ternary.cond);
      
      tmp = new_node(IR_JMPIFN);
      tmp->cjmp.label = l1;
      tmp->cjmp.reg = --creg;
      tmp->cjmp.size = irs;
      ir_append(n, tmp);
      
      ir_append(n, irgen_expr(scope, e->ternary.true_case));

      tmp = new_node(IR_JMP);
      tmp->str = l2;
      ir_append(n, tmp);

      tmp = new_node(IR_LABEL);
      tmp->str = l1;
      ir_append(n, tmp);

      ir_append(n, irgen_expr(scope, e->ternary.false_case));

      tmp = new_node(IR_LABEL);
      tmp->str = l2;
      ir_append(n, tmp);
      break;
   }
   case EXPR_SIZEOF:
   {
      n = new_node(IR_LOAD);
      n->load.dest = creg++;
      if (e->szof.has_expr) {
         struct value_type* nvt = get_value_type(scope, e->szof.expr);
         if (nvt->type == VAL_POINTER && nvt->pointer.is_array && !nvt->pointer.array.has_const_size) {
            struct expression* t = e->szof.expr;
            while (t->type == EXPR_PAREN) t = t->expr;
            if (t->type != EXPR_NAME)
               parse_error(&t->begin, "expected array");
            n = new_node(IR_ARRAYLEN);
            n->lookup.reg = creg - 1;
            if ((n->lookup.var_idx = scope_find_var_idx(scope, &n->lookup.scope, t->str)) == SIZE_MAX)
               parse_error(&t->begin, "array '%s' not found", t->str);
            free_value_type(nvt);
            break;
         } else n->load.value = sizeof_value(nvt, false);
         free_value_type(nvt);
      } else n->load.value = sizeof_value(e->szof.type, false);
      n->load.size = IRS_INT;
      break;
   }
   case EXPR_ARRAYLEN:
   {
      struct value_type* nvt = get_value_type(scope, e->expr);
      if (nvt->type != VAL_POINTER || !nvt->pointer.is_array)
         parse_error(&e->expr->begin, "expected array value");
      
      if (!nvt->pointer.array.has_const_size) {
         struct expression* t = e->expr;
         while (t->type == EXPR_PAREN) t = t->expr;
         if (t->type != EXPR_NAME)
            parse_error(&t->begin, "expected array");
         n = new_node(IR_ARRAYLEN);
         n->lookup.reg = creg++;
         if ((n->lookup.var_idx = scope_find_var_idx(scope, &n->lookup.scope, t->str)) == SIZE_MAX)
            parse_error(&t->begin, "array '%s' not found", t->str);

         tmp = new_node(IR_UDIV);
         tmp->binary.size = IRS_PTR;
         tmp->binary.dest = creg - 1;
         tmp->binary.a.type = IRT_REG;
         tmp->binary.a.reg = creg - 1;
         tmp->binary.b.type = IRT_UINT;
         tmp->binary.b.uVal = sizeof_value(nvt->pointer.type, false);
         ir_append(n, tmp);
      } else {
         n = new_node(IR_LOAD);
         n->load.dest = creg++;
         n->load.value = nvt->pointer.array.size;
         n->load.size = IRS_INT;
      }
      free_value_type(nvt);
      break;
   }
   
   default:
      panic("ir_expr(): unsupported expression '%s'", expr_type_str[e->type]);
   }
   return n;
}

ir_node_t* irgen_expr(struct scope* scope, const struct expression* expr) {
   creg = 0;
   return ir_expr(scope, expr);
}

ir_node_t* irgen_stmt(const struct statement* s) {
   ir_node_t* n;
   ir_node_t* tmp;
   creg = 0;
   switch (s->type) {
   case STMT_NOP: return new_node(IR_NOP);
   case STMT_EXPR:return irgen_expr(s->parent, s->expr);
   case STMT_RETURN:
      tmp = new_node(IR_RET);
      if (s->expr) {
         struct value_type* vt = get_value_type(s->parent, s->expr);
         if (vt->type == VAL_FLOAT) parse_error(&s->expr->begin, "floating-point numbers are not supported");
         enum ir_value_size irs = vt2irs(vt);
         n = ir_expr(s->parent, s->expr);
         if (vt2irs(s->parent->func->type) != irs) {
            tmp->type = IR_IICAST;
            tmp->iicast.dest = tmp->iicast.src = creg - 1;
            tmp->iicast.ds = vt2irs(s->parent->func->type);
            tmp->iicast.ss = irs;
            tmp->iicast.sign_extend = !s->parent->func->type->integer.is_unsigned;
            ir_append(n, tmp);
            tmp = new_node(IR_IRET);
         }
         tmp->type = IR_IRET;
         tmp->unary.size = irs; // TODO
         tmp->unary.reg = creg - 1;
         ir_append(n, tmp);
         
      } else n = tmp;
      return n;
   case STMT_SCOPE:
      n = new_node(IR_BEGIN_SCOPE);
      n->scope = s->scope;
      for (size_t i = 0; i < buf_len(s->scope->body); ++i)
         n = ir_append(n, irgen_stmt(s->scope->body[i]));
      tmp = new_node(IR_END_SCOPE);
      tmp->scope = s->scope;
      return ir_append(n, tmp);
   case STMT_VARDECL:
   {
      n = new_node(IR_NOP);
      for (size_t i = 0; i < s->var_decl.num; ++i) {
         const struct variable* var = &s->parent->vars[s->var_decl.idx + i];
         ir_node_t* cur;
         if (var->type->type == VAL_POINTER && var->type->pointer.is_array) {
            cur = new_node(IR_ALLOCA);
            cur->alloca.dest = creg;
            cur->alloca.var = var;
            if (var->type->pointer.array.has_const_size) {
               cur->alloca.size.type = IRT_UINT;
               cur->alloca.size.uVal = sizeof_value(var->type, false);
            } else {
               tmp = irgen_expr(s->parent, var->type->pointer.array.dsize);
               ir_node_t* mul = new_node(IR_UMUL);
               mul->binary.size = IRS_PTR;
               mul->binary.dest = creg - 1;
               mul->binary.a.type = IRT_REG;
               mul->binary.a.reg = creg - 1;
               mul->binary.b.type = IRT_UINT;
               mul->binary.b.uVal = sizeof_value(var->type->pointer.type, false);
               ir_append(tmp, mul);

               cur->alloca.size.type = IRT_REG;
               cur->alloca.size.reg = --creg;
               ir_append(tmp, cur);
               cur = tmp;
            }
            // creg is ptr to array
            tmp = new_node(IR_LOOKUP);
            tmp->lookup.reg = creg + 1;
            tmp->lookup.scope = s->parent;
            tmp->lookup.var_idx = s->var_decl.idx + i;
            ir_append(cur, tmp);

            tmp = new_node(IR_WRITE);
            tmp->move.dest = creg + 1;
            tmp->move.src = creg;
            tmp->move.size = IRS_PTR;
            ir_append(cur, tmp);

            if (var->init) {
               tmp = new_node(IR_LSTR);
               tmp->lstr.str = var->init->str;
               tmp->lstr.reg = creg + 1;
               ir_append(cur, tmp);

               tmp = new_node(IR_COPY);
               tmp->copy.dest = creg;
               tmp->copy.src = creg + 1;
               tmp->copy.len = my_min(var->type->pointer.array.size, strlen(var->init->str) + 1);
               ir_append(cur, tmp);
            }
         }
         else if (var->init) {
            struct expression* init = var->init;
            cur = ir_expr(s->parent, init);
            tmp = new_node(IR_LOOKUP);
            tmp->lookup.reg = creg;
            tmp->lookup.scope = s->parent;
            tmp->lookup.var_idx = s->var_decl.idx + i;
            ir_append(cur, tmp);

            tmp = new_node(IR_WRITE);
            tmp->move.size = vt2irs(var->type);
            tmp->move.dest = creg;
            tmp->move.src = creg - 1;
            ir_append(cur, tmp);
            --creg;
         } else cur = new_node(IR_NOP);
         ir_append(n, cur);
      }
      return n;
   }
   case STMT_IF:
   {
      struct value_type* vt = get_value_type(s->parent, s->ifstmt.cond);
      const enum ir_value_size irs = vt2irs(vt);
      free_value_type(vt);

      const bool has_false = s->ifstmt.false_case != NULL;
      istr_t lbl1 = make_label(clbl);
      istr_t lbl2 = has_false ? make_label(clbl + 1) : NULL;
      clbl += has_false ? 2 : 1;
     
      n = ir_expr(s->parent, s->ifstmt.cond);
      tmp = new_node(IR_JMPIFN);
      tmp->cjmp.label = has_false ? lbl2 : lbl1;
      tmp->cjmp.reg = --creg;
      tmp->cjmp.size = irs;
      ir_append(n, tmp);
      ir_append(n, irgen_stmt(s->ifstmt.true_case));

      if (has_false) {
         tmp = new_node(IR_JMP);
         tmp->str = lbl1;
         ir_append(n, tmp);

         tmp = new_node(IR_LABEL);
         tmp->str = lbl2;
         ir_append(n, tmp);

         ir_append(n, irgen_stmt(s->ifstmt.false_case));
      }

      tmp = new_node(IR_LABEL);
      tmp->str = lbl1;
      ir_append(n, tmp);

      return n;
   }
   case STMT_WHILE:
   {
      const istr_t old_begin = begin_loop, old_end = end_loop;
      struct value_type* vt = get_value_type(s->parent, s->whileloop.cond);
      const enum ir_value_size irs = vt2irs(vt);
      free_value_type(vt);

      const istr_t begin = make_label(clbl);
      begin_loop = make_label(clbl + 1);
      end_loop = make_label(clbl + 2);
      clbl += 3;
      
      n = new_node(IR_LABEL);
      n->str = begin;
      ir_append(n, irgen_expr(s->parent, s->whileloop.cond));

      tmp = new_node(IR_JMPIFN);
      tmp->cjmp.label = end_loop;
      tmp->cjmp.reg = --creg;
      tmp->cjmp.size = irs;
      ir_append(n, tmp);

      ir_append(n, irgen_stmt(s->whileloop.stmt));

      tmp = new_node(IR_LABEL);
      tmp->str = begin_loop;
      ir_append(n, tmp);

      if (s->whileloop.end) ir_append(n, irgen_expr(s->parent, s->whileloop.end));

      tmp = new_node(IR_JMP);
      tmp->str = begin;
      ir_append(n, tmp);

      tmp = new_node(IR_LABEL);
      tmp->str = end_loop;
      ir_append(n, tmp);

      begin_loop = old_begin;
      end_loop = old_end;
      return n;
   }
   case STMT_DO_WHILE:
   {
      const istr_t old_begin = begin_loop, old_end = end_loop;
      struct value_type* vt = get_value_type(s->parent, s->whileloop.cond);
      const enum ir_value_size irs = vt2irs(vt);
      free_value_type(vt);
      begin_loop = make_label(clbl);
      end_loop = make_label(clbl + 1);
      clbl += 2;

      n = new_node(IR_LABEL);
      n->str = begin_loop;

      ir_append(n, irgen_stmt(s->whileloop.stmt));

      ir_append(n, irgen_expr(s->parent, s->whileloop.cond));
      tmp = new_node(IR_JMPIF);
      tmp->cjmp.label = begin_loop;
      tmp->cjmp.reg = --creg;
      tmp->cjmp.size = irs;
      ir_append(n, tmp);

      tmp = new_node(IR_LABEL);
      tmp->str = end_loop;
      ir_append(n, tmp);

      begin_loop = old_begin;
      end_loop = old_end;
      return n;
   }
   case STMT_BREAK:
      if (!end_loop) parse_error(&s->begin, "break statement outside a loop");
      n = new_node(IR_JMP);
      n->str = end_loop;
      return n;
   case STMT_CONTINUE:
      if (!begin_loop) parse_error(&s->begin, "break statement outside a loop");
      n = new_node(IR_JMP);
      n->str = begin_loop;
      return n;

   default: panic("irgen_stmt(): unsupported statement '%s'", stmt_type_str[s->type]);
   }
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

