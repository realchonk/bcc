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

#include <string.h>
#include "cmdline.h"
#include "error.h"

#define GLIBC_SF_DL "/lib/ld-linux.so.3"
#define GLIBC_HF_DL "/lib/ld-linux-armhf.so.3"

#define MUSL_SF_DL "/lib/ld-musl-arm.so.1"
#define MUSL_HF_DL "/lib/ld-musl-armhf.so.1"

static bool has_hard_float(void) {
   // 32-sf or 32-hf
   const char* abi = get_mach_opt("abi")->sVal;
   char c = abi[3];
   return c == 'h';
}

char* get_ld_abi(void) {
   return strdup("-marmelf_linux_eabi");
}

char* get_interpreter(void) {
#if !HAS_LIBC
   panic("trying to get interpreter from a target that does not support dynamic linking");
#endif


#if LIBC_glibc
   return strdup(has_hard_float() ? GLIBC_HF_DL : GLIBC_SF_DL);
#elif LIBC_musl
   return strdup(has_hard_float() ? MUSL_HF_DL : MUSL_SF_DL);
#else
#error "Unsupported C library"
#endif
}
