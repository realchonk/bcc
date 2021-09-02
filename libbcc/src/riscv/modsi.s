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

.global __modsi2
__modsi2:
mv a5, a0
blt a0, x0, .L18
blt a1, x0, .L6
blt a0, a1, .L10
.L16:
mv a3, x0
.L8:
mv a4, a5
sub a5, a5, a1
bge a5, a1, .L8
mv a0, a5
bge a3, x0, .L1
sub a0, a1, a4
ret
.L6:
neg a1, a1
li a3, 1
bge a0, a1, .L8
neg a0, a0
.L1:
ret
.L18:
neg a0, a0
blt a1, x0, .L3
blt a0, a1, .L10
mv a5, a0
li a3, 1
j .L8
.L10:
mv a0, a5
ret
.L3:
neg a4, a1
bgt a5, a1, .L19
mv a1, a4
mv a5, a0
j .L16
.L19:
ret
