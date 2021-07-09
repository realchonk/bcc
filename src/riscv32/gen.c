#include <ctype.h>
#include "riscv32/cpu.h"
#include "target.h"
#include "strdb.h"

ir_node_t* emit_ir(ir_node_t*);

static void emit_begin(void) {
   strdb_init();
   emit(".section .text");
}
static void emit_end(void) {
   if (strdb) {
      emitraw(".section .rodata\n__strings:\n.string \"");
      for (size_t i = 0; i < buf_len(strdb) - 1; ++i) {
         const char ch = strdb[i];
         if (isprint(ch)) {
            emitraw("%c", ch);
         } else {
            emitraw("\\%03o", ch);
         }
      }
      emit("\"");
   }
}

static void emit_func(const struct function* func) {
   ir_node_t* n = func->ir_code;
   //cur_func = func;
   while ((n = emit_ir(n)) != NULL);
}
void emit_unit(void) {
   emit_begin();
   for (size_t i = 0; i < buf_len(cunit.funcs); ++i) {
      const struct function* f = cunit.funcs[i];
      if (f->ir_code)
         emit_func(f);
   }
   emit_end();
}
