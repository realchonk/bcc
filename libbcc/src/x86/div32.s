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
.section .text
.intel_syntax noprefix

# unsigned

.global __divui32
.type __divui32, @function
__divui32:
mov eax, dword ptr [esp + 8]
xor edx, edx
div eax, dword ptr [esp + 4]
ret
.size __divui32, .-__divui32

.global __divui16
.type __divui16, @function
__divui16:
xor edx, edx
mov ax, word ptr [esp + 8]
xor edx, edx
div ax, word ptr [esp + 4]
ret
.size __divui16, .-__divui16

.global __divui8
.type __divui8, @function
__divui8:
xor edx, edx
mov al, byte ptr [esp + 8]
xor edx, edx
div al, byte ptr [esp + 4]
ret
.size __divui8, .-__divui8

# signed

.global __divsi32
.type __divsi32, @function
__divsi32:
mov eax, dword ptr [esp + 8]
xor edx, edx
idiv eax, dword ptr [esp + 4]
ret
.size __divsi32, .-__divsi32

.global __divsi16
.type __divsi16, @function
__divsi16:
xor edx, edx
mov ax, word ptr [esp + 8]
xor edx, edx
idiv ax, word ptr [esp + 4]
ret
.size __divsi16, .-__divsi16

.global __divsi8
.type __divsi8, @function
__divsi8:
xor edx, edx
mov al, byte ptr [esp + 8]
xor edx, edx
idiv al, byte ptr [esp + 4]
ret
.size __divsi8, .-__divsi8
