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



crtbegin.o: src/x86/crtbegin.asm
	$(CC) -c -o $@ $<

crtend.o: src/x86/crtend.asm
	$(CC) -c -o $@ $<

.asm.o:
	$(CC) -c -o $@ $<

if BITS_32
libbcc_a_SOURCES += src/x86/mul32.asm	\
						  src/x86/div32.asm	\
						  src/x86/mod32.asm
else
libbcc_a_SOURCES += src/x86/mul64.asm	\
						  src/x86/div64.asm	\
						  src/x86/mod64.asm
endif

