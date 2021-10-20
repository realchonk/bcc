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
#include "config.h"
#include "cpu.h"

static const char* fend_asm[]      = { "s", "S", NULL };
static const char* fend_obj[]      = { "o", NULL      };
static const char* fend_archive[]  = { "a", NULL      };
static const char* fend_dll[]      = { "so", NULL     };

const struct target_info target_info = {
   .name = BCC_FULL_ARCH,
   .size_byte     = 1,
   .size_char     = 1,
   .size_short    = 2,
   .size_int      = 4,
   .size_long     = BITS / 8,
   .size_float    = 4,
   .size_double   = 8,
   .size_pointer  = BITS / 8,

   .min_byte   = INT8_MIN,
   .max_byte   = INT8_MAX,
   .max_ubyte  = UINT8_MAX,
   
   .min_char   = INT8_MIN,
   .max_char   = INT8_MAX,
   .max_uchar  = UINT8_MAX,
   
   .min_short  = INT16_MIN,
   .max_short  = INT16_MAX,
   .max_ushort = UINT16_MAX,
   
   .min_int    = INT32_MIN,
   .max_int    = INT32_MAX,
   .max_uint   = UINT32_MAX,
   
#if BITS == 32
   .min_long   = INT32_MIN,
   .max_long   = INT32_MAX,
   .max_ulong  = UINT32_MAX,
#else
   .min_long   = INT64_MIN,
   .max_long   = INT64_MAX,
   .max_ulong  = UINT64_MAX,
#endif

   .unsigned_char = false,

   .fend_asm      = fend_asm,
   .fend_obj      = fend_obj,
   .fend_archive  = fend_archive,
   .fend_dll      = fend_dll,

   .ptrdiff_type  = INT_LONG,
   .size_type     = INT_LONG,
   .has_c99_array = false,

   .size_int8 = INT_BYTE,
   .size_int16 = INT_SHORT,
   .size_int32 = INT_INT,
   .size_int64 = INT_LONG,

   .max_immed =  2047,
   .min_immed = -2048,
   .max_iload =  524287,
   .min_iload = -524288,

   .fuse_fp_rw = false,
   .fuse_gl_rw = false,
   .fuse_lu_rw = false,
};
