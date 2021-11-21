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

crtbegin.o: src/arm/crtbegin.s
	$(CC) -c -o $@ $<

crtend.o: src/arm/crtend.s
	$(CC) -c -o $@ $<

libbcc_a_SOURCES += 	src/arm/divui.s	\
							src/arm/divsi.s	\
							src/arm/modui.s	\
							src/arm/modsi.s
						  	
