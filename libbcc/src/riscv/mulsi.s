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

.global __mulsi2
__mulsi2:
mv a5, a0
blt a0, x0, .L22
blt a1, x0, .L11
mv a3, x0
beq a1, x0, .L20
.L4:
mv a0, x0
.L8:
andi a4, a1, 1
srai a1, a1, 1
add a0, a0, a5
.L7:
slli a5, a5, 1
bne a1, x0, .L8
beq a3, x0, .L1
neg a0, a0
.L1:
ret
.L11:
li a3, 1
neg a1, a1
j .L4
.L22:
neg a5, a0
blt a1, x0, .L9
li a3, 1
bne a1, x0, .L4
.L20:
mv a0, x0
ret
.L9:
mv a3, x0
neg a1, a1
j .L4
