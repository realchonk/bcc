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

#include "target.h"
#include "error.h"
#include "strdb.h"
#include "regs.h"
#include "unit.h"

static void emit_global_init(const struct value_type*, const struct value*);

ir_node_t* emit_ir(ir_node_t*);

void emit_unit(void) {
   strdb_init();
   //emit(".file \"%s\"", source_name);
   emit(".intel_syntax noprefix");
   emit(".section .text");

   for (size_t i = 0; i < buf_len(cunit.funcs); ++i) {
      struct function* f = cunit.funcs[i];
      if (f->ir_code) {
         ir_node_t* n = f->ir_code;
         while ((n = emit_ir(n)) != NULL);
      }
   }


   asm_indent = 0;
   for (size_t i = 0; i < num_builtin_funcs; ++i) {
      const struct builtin_func* f = &builtin_funcs[i];
      if (f->requested)
         emit("%s:\n%s", f->name, f->code);
   }

   if (cunit.vars) {
      for (size_t i = 0; i < buf_len(cunit.vars); ++i) {
         const struct variable* var = &cunit.vars[i];
         const struct value_type* vt = var->type;
         if (var->attrs & ATTR_EXTERN)
            continue;

         if (!(var->attrs & ATTR_STATIC))
            emit(".global %s", var->name);

         if (var->has_const_value) {
            emit(".section .data");
            emit("%s:", var->name);
            emit_global_init(var->type, &var->const_init);
         } else {
            emit(".section .bss");
            emit("%s:", var->name);
            emit(".zero %zu", sizeof_value(vt, false));
         }

         if (!get_mach_opt("clean-asm")->bVal) {
            emit(".type %s, @object", var->name);
            emit(".size %s, %zu", var->name, sizeof_value(vt, false));
         }
      }
   }

   emit_strdb();

   if (!get_mach_opt("clean-asm")->bVal) {
      // emit identification
      emit(".ident \"brainlet C compiler v%s\"", PACKAGE_VERSION);
   }
}
static void emit_global_init(const struct value_type* vt, const struct value* val) {
   switch (vt->type) {
   case VAL_INT:
      switch (vt->integer.size) {
      case INT_BYTE:
      case INT_CHAR:
         emitraw(".byte ");
         break;
      case INT_SHORT:
         emitraw(".short ");
         break;
      case INT_LONG:
#if BITS == 64
         emitraw(".quad ");
         break;
#endif
      case INT_INT:
         emitraw(".long ");
         break;
      default:
         panic("invalid IR integer size");
      }
      if (vt->integer.is_unsigned)
         emit("%ju", val->uVal);
      else emit("%jd", val->iVal);
      break;
#if ENABLE_FP
      case VAL_FLOAT:
         switch (vt->fp.size) {
         case FP_FLOAT:
            emitraw(".long ");
            break;
         case FP_DOUBLE:
            emitraw(".quad ");
            break;
         default:
            panic("invalid IR fp size");
         }
         panic("initialization of global floating-point variables is not supported");
         break;
#endif
      case VAL_POINTER:
         if (vt->pointer.is_array) {
            print_value_type(stderr, vt->pointer.type);
            if (!vt_is_array(val->type))
               panic("val is not an array, val is %s", value_type_str[val->type->type]);
            for (size_t i = 0; i < buf_len(val->array); ++i) {
               emit_global_init(vt->pointer.type, &val->array[i]);
            }
            break;
         } else {
#if BITS == 32
            emitraw(".long ");
#else
            emitraw(".quad ");
#endif
            emit("%ju", val->uVal);
         }
         break;
      case VAL_BOOL:
         emitraw(".byte ");
         emit("%u", val->uVal ? 1 : 0);
         break;
      default:
         panic("invalid variable type '%s'", value_type_str[vt->type]);
   }
}
