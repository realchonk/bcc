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

.global __mului2
__mului2:
mv a5, a0
mv a0, x0
beq a1, x0, .L5
.L4:
andi a4, a1, 1
srli a1, a1, 1
beq a4, x0, .L3
add a0, a0, a5
.L3:
slli a5, a5, 1
bne a1, x0, .L4
ret
.L5:
ret
