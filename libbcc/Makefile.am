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

clib_DATA = crtbegin.o crtend.o
clib_LIBRARIES = libbcc.a

libbcc_a_SOURCES = src/has_libbcc.c		\
						 src/memcpy.c

if ARCH_x86
include $(srcdir)/src/x86/Makefile.am
endif

if ARCH_riscv
include $(srcdir)/src/riscv/Makefile.am
endif

print-sources:
	@echo $(libbcc_a_SOURCES)

full-clean: clean
	rm -f Makefile Makefile.in configure src/.dirstamp src/$(ARCH)/.dirstamp
	rm -f config.h config.h.in config.h.in~ config.log config.status stamp-h1 aclocal.m4
	rm -rf autom4te.cache build-aux src/.deps .deps src/$(ARCH)/.deps configure~

clean-local:
	rm -f crtbegin.o crtend.o

