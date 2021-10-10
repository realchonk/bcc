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
#include "config.h"
#include "target.h"
#include "error.h"
#include "strdb.h"
#include "cpu.h"

ir_node_t* emit_ir(ir_node_t*);

static void emit_global_init(const struct value_type* vt, const struct value* val);
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
         emit_global_init(var->type, var->has_const_value ? &var->const_init : NULL);
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
void emit_init_int(enum ir_value_size irs, intmax_t val, bool is_unsigned) {
   switch (irs) {
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
      panic("invalid IR integer size");
   }
   if (is_unsigned) {
      emit("%ju", val);
   } else {
      emit("%jd", val);
   }
}
static void emit_global_init(const struct value_type* vt, const struct value* val) {
   switch (vt->type) {
   case VAL_INT:
      emit_init_int(vt2irs(vt), val ? val->iVal : 0, vt->integer.is_unsigned);
      break;
#if ENABLE_FP
      case VAL_FLOAT:
         switch (vt->fp.size) {
         case FP_FLOAT:
            emitraw(".word ");
            break;
         case FP_DOUBLE:
            emitraw("dword ");
            break;
         default:
            panic("invalid IR fp size");
         }
         if (val) {
            panic("initialization fo global floating-point variables is not supported");
         } else emit("0");
         break;
#endif
      case VAL_POINTER:
         if (vt->pointer.is_array) {
            if (val) {
               print_value_type(stderr, vt->pointer.type);
               if (!vt_is_array(val->type))
                  panic("val is not an array, val is %s", value_type_str[val->type->type]);
               for (size_t i = 0; i < buf_len(val->array); ++i) {
                  emit_global_init(vt->pointer.type, &val->array[i]);
               }
            } else {
               emit(".zero %zu", sizeof_value(vt, false));
            }
            break;
         } else {
#if BITS == 32
            emitraw(".word ");
#else
            emitraw(".dword ");
#endif
            if (val) {
               emit("%ju", val->uVal);
            } else emit("0");
         }
         break;
      case VAL_BOOL:
         emitraw(".byte ");
         if (val) {
            emit("%u", val->uVal ? 1 : 0);
         } else emit("0");
         break;
      default:
         panic("invalid variable type '%s'", value_type_str[vt->type]);
   }
}
