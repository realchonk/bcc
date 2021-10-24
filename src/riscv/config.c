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
#include <stdlib.h>
#include <stdio.h>
#include "config/base.h"
#include "target.h"
#include "config.h"

char* get_ld_abi(void) {
   char* abi = malloc(100);
   if (!abi)
      panic("failed to malloc()");
   snprintf(abi, 100, "-melf%dlriscv_%s", BITS, get_mach_opt("abi")->sVal);
   return abi;
}

#if LIBC_musl
static const char* get_musl_suffix(void) {
   if (is_abi("ilp32") || is_abi("lp64")) {
      return "-sf";
   } else if (is_abi("ilp32f") || is_abi("lp64f")) {
      return "-sp";
   } else if (is_abi("ilp32d") || is_abi("lp64d")) {
      return "";
   } else {
      panic("unsupported ABI for musl");
   }
}
#endif

char* get_interpreter(void) {
#if !HAS_LIBC
   panic("trying to get interpreter from a target that does not support dynamic linking");
#endif

#if LIBC_glibc
      char interp[] = "/lib/ld-linux-riscv__-______.so.1";
      snprintf(interp, sizeof(interp), "/lib/ld-linux-riscv%d-%s.so.1", BITS, get_mach_opt("abi")->sVal);
      return strdup(interp);
#elif LIBC_musl
      char interp[] = "/lib/ld-musl-riscv__-___.so.1";
      snprintf(interp, sizeof(interp), "/lib/ld-musl-riscv%d%s.so.1", BITS, get_musl_suffix());
      return strdup(interp);
#else
      panic("unsupported C library");
#endif
}
