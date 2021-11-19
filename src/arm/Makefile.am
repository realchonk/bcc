#  Copyright (C) 2021 Benjamin Stürz
#  
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#  
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#  
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <https://www.gnu.org/licenses/>.



bcc_SOURCES += src/riscv/builtins.c		\
				 	src/riscv/common.c		\
				 	src/riscv/cpu.c			\
				 	src/riscv/emit_ir.c		\
				 	src/riscv/gen.c			\
				 	src/riscv/optim.c			\
				 	src/riscv/regs.c			\
				 	src/riscv/target.c		\
					src/riscv/config.c		\
					src/binutils_helpers.c

bcc_CPPFLAGS += -DUSE_BINUTILS=1
