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
subs    r3, r0, #0
blt     .L2
cmp     r1, #0
blt     .L3
cmp     r3, r1
blt     .L18
.L17:
mov     ip, #0
.L4:
mov     r0, #0
.L8:
sub     r3, r3, r1
cmp     r3, r1
mov     r2, r0
add     r0, r0, #1
bge     .L8
cmp     ip, #0
mvnne   r0, r2
mov     pc, lr
.L3:
rsb     r1, r1, #0
cmp     r3, r1
movge   ip, #1
bge     .L4
.L18:
mov     r0, #0
mov     pc, lr
.L2:
cmp     r1, #0
rsb     r2, r3, #0
blt     .L6
cmp     r2, r1
blt     .L18
mov     r3, r2
mov     ip, #1
b       .L4
.L6:
cmp     r3, r1
movle   r3, r2
rsb     r1, r1, #0
ble     .L17
b       .L18
