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

.global __divsi2
__divsi2:
mv a5, a0
blt a0, x0, .L20
blt a1, x0, .L6
blt a0, a1, .L18
mv a3, x0
.L4:
mv a0, x0
.L8:
sub a5, a5, a1
mv a4, a0
addi a0, a0, 1
bge a5, a1, .L8
beq a3, x0, .L1
not a0, a4
ret
.L6:
neg a1, a1
li a3, 1
bge a0, a1, .L4
.L18:
mv a0, x0
.L1:
ret
.L20:
neg a4, a0
blt a1, x0, .L3
mv a5, a4
li a3, 1
bge a4, a1, .L4
mv a0, x0
j .L1
.L3:
neg a3, a1
bgt a0, a1, .L18
mv a1, a3
mv a5, a4
mv a3, x0
j .L4
