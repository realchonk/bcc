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
#include "target.h"
#include "config.h"

char* get_ld_abi(void) {
#if BITS == 32
   return strdup("-melf_i386");
#else
   return strdup("-melf_x86_64");
#endif
}

char* get_interpreter(void) {
#if !HAS_LIBC
   panic("trying to get interpreter from a target that does not support dynamic linking");
#endif

   if (is_libc("glibc")) {
#if BITS == 32
      return strdup("/lib/ld-linux.so.2");
#else
      return strdup("/lib64/ld-linux-x86-64.so.2");
#endif
   } else if (is_libc("musl")) {
      return strdup("/lib/ld-musl-" BCC_FULL_ARCH ".so.1");
   } else {
      panic("unsupported C library");
   }
}
