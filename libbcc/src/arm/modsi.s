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
subs    r3, r0, #0
blt     .L2
cmp     r1, #0
blt     .L3
cmp     r3, r1
blt     .L11
.L16:
mov     r2, #0
.L8:
mov     r0, r3
sub     r3, r3, r1
cmp     r3, r1
bge     .L8
cmp     r2, #0
beq     .L11
sub     r0, r1, r0
mov     pc, lr
.L3:
rsb     r1, r1, #0
cmp     r3, r1
movge   r2, #1
bge     .L8
rsb     r0, r3, #0
mov     pc, lr
.L11:
mov     r0, r3
mov     pc, lr
.L2:
cmp     r1, #0
rsb     r0, r3, #0
blt     .L6
cmp     r0, r1
blt     .L11
mov     r3, r0
mov     r2, #1
b       .L8
.L6:
cmp     r3, r1
rsb     r1, r1, #0
movgt   pc, lr
mov     r3, r0
b       .L16
