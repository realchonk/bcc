//  Copyright (C) 2021 Benjamin Stürz
//  
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//  
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//  
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <ctype.h>
#include "riscv/cpu.h"
#include "config.h"
#include "target.h"
#include "error.h"
#include "strdb.h"

ir_node_t* emit_ir(ir_node_t*);

static void emit_begin(void) {
   strdb_init();
   emit(".section .text");
}
static void emit_end(void) {
   for (size_t i = 0; i < num_builtin_funcs; ++i) {
      if (builtin_funcs[i].requested)
         emit("%s:\n%s", builtin_funcs[i].name, builtin_funcs[i].code);
   }
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

   if (cunit.vars) {
      emit(".section .data");
      for (size_t i = 0; i < buf_len(cunit.vars); ++i) {
         const struct variable* var = &cunit.vars[i];
         const struct value_type* vt = var->type;
         if (var->attrs & ATTR_EXTERN)
            continue;
         if (!(var->attrs & ATTR_STATIC))
            emit(".global %s", var->name);
         emit("%s:", var->name);
         switch (vt->type) {
         case VAL_INT:
            switch (vt->integer.size) {
            case INT_BYTE:
            case INT_CHAR:
               emitraw(".byte ");
               break;
            case INT_SHORT:
               emitraw(".half ");
               break;
            case INT_LONG:
#if BITS == 64
               emitraw(".dword ");
               break;
#endif   
            case INT_INT:
               emitraw(".word ");
               break;
            default:
               panic("unreachable reached");
            }
            if (var->init) {
               if (vt->integer.is_unsigned)
                  emit("%ju", var->const_init.uVal);
               else emit("%jd", var->const_init.iVal);
            } else emit("0");
            break;
#if ENABLE_FP
         case VAL_FLOAT:
            switch (vt->fp.size) {
            case FP_FLOAT: {
               if (var->init) {
                  const float f = (float)var->const_init.fVal;
                  emit(".word %jd", (intmax_t)*(int32_t*)&f);
               } else emit(".word 0");
               break;
            }
            case FP_DOUBLE:
               if (var->init) {
                  const double f = (double)var->const_init.fVal;
                  emit(".dword %jd", (intmax_t)*(int64_t*)&f);
               } else emit(".dword 0");
               break;
            default:
               panic("unreachable reached");
            }
            break;
#endif
         case VAL_POINTER:
            if (vt->pointer.is_array) {
               emit(".zero %zu", sizeof_value(vt, false));
            } else {
#if BITS == 32
               emit(".word 0");
#else
               emit(".dword 0");
#endif
            }
            break;
         default:
            panic("invalid variable type '%s'", value_type_str[vt->type]);
         }
      }
   }

}

static void emit_func(const struct function* func) {
   ir_node_t* n = func->ir_code;
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
