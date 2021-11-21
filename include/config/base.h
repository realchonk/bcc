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

#ifndef FILE_CONFIG_BASE_H
#define FILE_CONFIG_BASE_H
#include "config.h"

#if ARCH_x86
# include "config/x86/x86.h"
#elif ARCH_riscv
# include "config/riscv/riscv.h"
#elif ARCH_arm
# include "config/arm/arm.h"
#else
# error "invalid processor architecture"
#endif

#endif /* FILE_CONFIG_BASE_H */
