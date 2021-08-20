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

#ifndef __BCC_CONFIG_H__
#define __BCC_CONFIG_H__

#undef __BCC_BITS

#if defined(__i386__)
#define __BCC_BITS 32
#elif defined(__x86_64__)
#define __BCC_BITS 64
#elif defined(__riscv)

#define __BCC_BITS __riscv_xlen

#else
#error Unsupported processor architecture
#endif

#endif /* __BCC_CONFIG_H__ */
