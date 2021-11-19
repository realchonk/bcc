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

// !!! IMPORTANT !!!
// Shared code for targets using the binutils.
// Must not be build with targets that don't use the binutils.

#include <ctype.h>
#include "binutils.h"
#include "target.h"
#include "config.h"
#include "strdb.h"

#define check_flag(name) (get_mach_opt(name)->bVal)
#define is_clean_asm() check_flag("clean-asm")


static void emit_init_int(enum ir_value_size irs, intmax_t val, bool is_unsigned);
static void emit_global_init(const struct value_type* vt, const struct value* val);

static void emit_begin(void) {
   strdb_init();

   // TODO: emit .file source_name
   emit_begin_hook();

   emit(".section %s", binutils_info.section_text);
}

static void emit_end(void) {

   emit_builtin_funcs_hook();

   for (size_t i = 0; i < num_builtin_funcs; ++i) {
      if (builtin_funcs[i].requested)
         emit("%s:\n%s", builtin_funcs[i].name, builtin_funcs[i].code);
   }
   
   emit_global_vars_hook();

   if (strdb) {
      emitraw(".section %s\n__strings:\n%s \"",
            binutils_info.section_rodata,
            binutils_info.init_string);
      for (size_t i = 0; i < buf_len(strdb) - 1; ++i) {
         const char ch = strdb[i];
         if (isprint(ch)) {
            emitraw("%c", ch);
         } else {
            emitraw("\\%03o", ch);
         }
      }
      if (!binutils_info.init_string_has_null) {
         emitraw("\\000");
      }
      emit("\"");
   }

   if (cunit.vars) {
      const char* last_section = NULL;
      for (size_t i = 0; i < buf_len(cunit.vars); ++i) {
         const struct variable* var = &cunit.vars[i];
         const struct value_type* vt = var->type;
         if (var->attrs & ATTR_EXTERN)
            continue;

         const char* section;
         if (var->has_const_value) {
            if (vt->is_const) {
               section = binutils_info.section_rodata;
            } else {
               section = binutils_info.section_data;
            }
         } else {
            section = binutils_info.section_bss;
         }

         if (section != last_section)
            emit(".section %s", section);
         last_section = section;

         if (!(var->attrs & ATTR_STATIC))
            emit(".global %s", var->name);

         if (!is_clean_asm()) {
            emit(".type %s, %cobject", var->name, binutils_info.type_prefix);
            emit(".size %s, %zu", var->name, sizeof_value(vt, false));
         }

         emit("%s:", var->name);

         emit_global_init(var->type, var->has_const_value ? &var->const_init : NULL);

         emit("");
      }
   }

   if (!is_clean_asm()) {
      emit(".ident \"brainlet C compiler v%s\"", PACKAGE_VERSION);
   }

   emit_end_hook();
}

static void emit_comment(const char* s, ...) {
   va_list ap;
   va_start(ap, s);

   emitraw("%s ", binutils_info.comment);
   vemitraw(s, ap);
   emit("");

   va_end(ap);
}

static void emit_func(const struct function* func) {
   const char* name = func->name;

   if (func_is_global(func)) {
      emit(".global %s", name);
   }
   
   if (!is_clean_asm()) {
      emit(".type %s, %cfunction", name, binutils_info.type_prefix);
   }

   emit("%s:", name);

   ir_node_t* n = func->ir_code;
   while ((n = emit_ir(n)) != NULL);

   // emit large constants
   if (func->big_iloads) {
      emit("");
      emit_comment("large constants for %s", name);
      for (size_t i = 0; i < buf_len(func->big_iloads); ++i) {
         const struct ir_big_iload* bi = &func->big_iloads[i];
         emit("%s:", bi->label);
         emit_init_int(bi->size, bi->val, bi->is_unsigned);
      }
   }

   if (!is_clean_asm()) {
      emit(".size %s, . - %s", name, name);
   }

   emit("");
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

static void emit_init_int(enum ir_value_size irs, intmax_t val, bool is_unsigned) {
   switch (irs) {
   case IRS_BYTE:    emitraw("%s ", binutils_info.init_byte);    break;
   case IRS_CHAR:    emitraw("%s ", binutils_info.init_char);    break;
   case IRS_SHORT:   emitraw("%s ", binutils_info.init_short);   break;
   case IRS_INT:     emitraw("%s ", binutils_info.init_int);     break;
   case IRS_LONG:    emitraw("%s ", binutils_info.init_long);    break;
   default:          panic("invalid IR value size '%s'", ir_size_str[irs]);
   }
   if (is_unsigned) {
      emit("%ju", val);
   } else {
      emit("%jd", val);
   }
}

static void emit_alloc(size_t sz) {
   emit("%s %zu", binutils_info.init_zero, sz);
}

static void emit_global_init(const struct value_type* vt, const struct value* val) {
   switch (vt->type) {
   case VAL_INT:
      if (val) {
         emit_init_int(vt2irs(vt), val->iVal, vt->integer.is_unsigned);
      } else {
         emit_alloc(sizeof_value(vt, false));
      }
      break;
#if ENABLE_FP
      case VAL_FLOAT:
         switch (vt->fp.size) {
         case FP_FLOAT:
            emitraw("%s ", binutils_info.init_float);
            break;
         case FP_DOUBLE:
            emitraw("%s ", binutils_info.init_double);
            break;
         default:
            panic("invalid IR fp size");
         }
         panic("initialization of global floating-point variables is not supported");
         break;
#endif
      case VAL_POINTER:
         if (vt->pointer.is_array) {
            if (val) {
               if (!vt_is_array(val->type))
                  panic("val is not an array, val is %s", value_type_str[val->type->type]);
               for (size_t i = 0; i < buf_len(val->array); ++i) {
                  emit_global_init(vt->pointer.type, &val->array[i]);
               }
            } else {
               if (!vt_is_const_array(vt))
                  panic("array is not constant-length");
               emit_alloc(sizeof_value(vt, false));
            }
            break;
         } else {
            emit("%s %ju", binutils_info.init_ptr, val->uVal);
         }
         break;
      case VAL_BOOL:
         emit("%s %u", binutils_info.init_bool, (val && val->uVal) ? 1 : 0);
         break;
      default:
         panic("invalid variable type '%s'", value_type_str[vt->type]);
   }
}
