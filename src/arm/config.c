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
#include "error.h"

#define ARM_SF_DL "/lib/linux.so.3"
#define ARM_HF_DL "/lib/linux-armhf.so.3"

char* get_ld_abi(void) {
   return strdup("-marmelf_linux_eabi");
}

char* get_interpreter(void) {
#if !HAS_LIBC
   panic("trying to get interpreter from a target that does not support dynamic linking");
#endif


#if LIBC_glibc
   return strdup(ARM_SF_DL);
#else
#error "Unsupported C library"
#endif
}
