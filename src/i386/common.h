struct stack_alloc_entry {
   bool is_const;
   size_t sz; // if !is_const it is [ebp - sz]
};

static const struct function* cur_func;
static uintreg_t esp = 0;
static istr_t* unresolved = NULL;
static istr_t* defined = NULL;
static struct stack_alloc_entry** stack_alloc;
static struct stack_alloc_entry* stack_cur_alloc;


static const char* reg_bx(const enum ir_value_size irs) {
   switch (irs) {
   case IRS_BYTE:
   case IRS_CHAR:
      return "bl";
   case IRS_SHORT:
      return "bx";
   case IRS_PTR:
#if BCC_x86_64
   case IRS_LONG:
      return "rbx";
#endif
   case IRS_INT:
      return "ebx";
   default:
      panic("reg_bx(): invalid value size '%s'", ir_size_str[irs]);
   }
}
static const char* nasm_size(enum ir_value_size s) {
   switch (s) {
   case IRS_BYTE:
   case IRS_CHAR:    return "byte";
   case IRS_SHORT:   return "word";

   case IRS_PTR:
#if BCC_x86_64
   case IRS_LONG:    return "qword";
#endif
   case IRS_INT:     return "dword";
   default:          panic("nasm_size(): unsupported operand size '%s'", ir_size_str[s]);
   }
}
static size_t x86_sizeof_value(const struct value_type* vt) {
   if (vt->type == VAL_POINTER && !vt->pointer.array.has_const_size)
      return REGSIZE * 2;
   else return sizeof_value(vt, true);
}
static size_t sizeof_scope(const struct scope* scope) {
   size_t num = 0;
   for (size_t i = 0; i < buf_len(scope->vars); ++i) {
      const size_t sz = x86_sizeof_value(scope->vars[i].type);
      if (sz > num) num = sz;
   }
   size_t max_child = 0;
   for (size_t i = 0; i < buf_len(scope->children); ++i) {
      const size_t sz = sizeof_scope(scope->children[i]);
      if (sz > max_child) max_child = sz;
   }
   return num + max_child;
}
static void assign_scope(struct scope* scope, size_t* addr) {
   for (size_t i = 0; i < buf_len(scope->vars); ++i) {
      scope->vars[i].addr = *addr;
      *addr += x86_sizeof_value(scope->vars[i].type);
   }
   for (size_t i = 0; i < buf_len(scope->children); ++i) {
      assign_scope(scope->children[i], addr);
   }
}
static ir_node_t* emit_ir(const ir_node_t* n);
static void emit_begin(void) {
   strdb_init();
   if (unresolved) buf_free(unresolved);
   if (defined) buf_free(defined);
   emit("default rel");
   emit("section .text");
   emit("extern memcpy");
}
static void emit_end(void) {
   for (size_t i = 0; i < buf_len(unresolved); ++i) {
      bool is_defined = false;
      for (size_t j = 0; j < buf_len(defined); ++j) {
         if (unresolved[i] == defined[j]) {
            is_defined = true;
            break;
         }
      }
      if (!is_defined) emit("extern %s", unresolved[i]);
   }

   asm_indent = 0;
   if (strdb) {
      emit("\nsection .rodata\n__strings:");
      size_t i = 0;
      while (i < buf_len(strdb)) {
         if (!strdb[i]) {
            emit("db 0");
            ++i;
            continue;
         }
         emitraw("db ");
         while (strdb[i]) {
            if (isprint(strdb[i])) {
               emitraw("\"");
               while (isprint(strdb[i])) {
                  emitraw("%c", strdb[i++]);
               }
               emitraw("\"");
            } else {
               emitraw("%u", strdb[i++]);
            }
            emitraw(", ");
         }
         emit("0");
         ++i;
      }
   }

   buf_free(unresolved);
   buf_free(defined);
}
static void emit_func(const struct function* func, const ir_node_t* n) {
   cur_func = func;
   esp = 0;
   while ((n = emit_ir(n)) != NULL);
}
void emit_unit(void) {
   emit_begin();
   for (size_t i = 0; i < buf_len(cunit.funcs); ++i) {
      const struct function* f = cunit.funcs[i];
      if (f->ir_code)
         emit_func(f, f->ir_code);
      else emit("extern %s", f->name);
   }
   emit_end();
}

static size_t uslen(uintmax_t v) {
   return v == 0 ? 1 : (size_t)log10(v) + 1;
}
static const char* irv2str(const struct ir_value* v, const enum ir_value_size s) {
   if (v->type == IRT_REG) {
      const char* str;
      reg_op(str, v->reg, s);
      return str;
   } else if (v->type == IRT_UINT) {
      char buffer[uslen(v->uVal) + 1];
      snprintf(buffer, sizeof(buffer), "%ju", v->uVal);
      return strint(buffer);
   } else panic("irv2str(): invalid IR value type '%u'", v->type);
}
static void emit_clear(const char* reg) {
   if (optim_level < 1) emit("mov %s, 0", reg);
   else emit("xor %s, %s", reg, reg);
}

static size_t align_stack_size(size_t n) {
   return n % REGSIZE ? ((n / REGSIZE + 1) * REGSIZE) + REGSIZE : n;
}

static void free_stack(void) {
   struct stack_alloc_entry* e = stack_cur_alloc;
   stack_cur_alloc = buf_pop(stack_alloc);

   size_t const_sz = 0;
   for (size_t i = 0; i < buf_len(e); ++i) {
      if (e[i].is_const)
         const_sz += e[i].sz;
      else {
         emit("add %s, %s [%s - %zu]", reg_sp, nasm_size(IRS_PTR), reg_bp, e[i].sz);
      }
   }
   if (const_sz) emit("add %s, %zu", reg_sp, const_sz);

   buf_free(e);
}
