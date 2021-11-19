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
#include <stdint.h>
#include "binutils.h"
#include "target.h"
#include "config.h"

#define gen_minmax(name, bits)   \
.min_##name = INT##bits##_MIN,   \
.max_##name = INT##bits##_MAX,   \
.max_u##name = UINT##bits##_MAX

static const char* fend_asm[]      = { "s", "S", "asm", NULL   };
static const char* fend_obj[]      = { "o", NULL               };
static const char* fend_archive[]  = { "a", NULL               };
static const char* fend_dll[]      = { "so", NULL              };

const struct target_info target_info = {
   .name          = BCC_FULL_ARCH,
   .size_byte     = 1,
   .size_char     = 1,
   .size_short    = 2,
   .size_int      = 4,
   .size_long     = BITS / 8,
   .size_float    = 4,
   .size_double   = 8,
   .size_pointer  = BITS / 8,

   gen_minmax(byte,  8),
   gen_minmax(char,  8),
   gen_minmax(short, 16),
   gen_minmax(int,   32),
#if BITS == 32
   gen_minmax(long, 32),
#else
   gen_minmax(long, 64),
#endif

   .unsigned_char = false,

   .fend_asm      = fend_asm,
   .fend_obj      = fend_obj,
   .fend_archive  = fend_archive,
   .fend_dll      = fend_dll,

   .ptrdiff_type  = INT_LONG,
   .size_type     = INT_LONG,
   .has_c99_array = false,

   .size_int8     = INT_BYTE,
   .size_int16    = INT_SHORT,
   .size_int32    = INT_INT,
#if BITS == 32
   .size_int64    = NUM_INTS,
#else
   .size_int64    = INT_LONG,
#endif

   .min_immed     = INT32_MIN,
   .max_immed     = INT32_MAX,
   .min_iload     = INT32_MIN,
   .max_iload     = INT32_MAX,
   //.min_iload     = 0,
   //.max_iload     = 7,
   
   .fuse_fp_rw    = true,
   .fuse_gl_rw    = true,
   .fuse_lu_rw    = true,
};

const struct binutils_info binutils_info = {
   .comment                = "#",

   .section_text           = ".text",
   .section_data           = ".data",
   .section_rodata         = ".rodata",
   .section_bss            = ".bss",
   
   .init_byte              = ".byte",
   .init_char              = ".byte",
   .init_short             = ".short",
   .init_int               = ".long",
#if BITS == 32
   .init_long              = ".long",
   .init_ptr               = ".long",
#else
   .init_long              = ".quad",
   .init_ptr               = ".quad",
#endif
   .init_float             = ".long",
   .init_double            = ".quad",
   .init_bool              = ".byte",
   .init_string            = ".string",
   .init_zero              = ".zero",

   .init_string_has_null   = true,
};
