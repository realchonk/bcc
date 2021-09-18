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
.intel_syntax noprefix
.section .text

# unsigned

.global __modui32
.type __modui32, @function
__modui32:
mov eax, dword [esp + 8]
xor edx, edx
div eax, dword [esp + 4]
mov eax, edx
ret
.size __modui32, .-__modui32

.global __modui16
.type __modui16, @function
__modui16:
xor eax, eax
mov ax, word [esp + 6]
xor edx, edx
div ax, word [esp + 4]
mov eax, edx
ret
.size __modui16, .-__modui16

.global __modui8
.type __modui8, @function
__modui8:
xor eax, eax
mov al, byte [esp + 5]
xor edx, edx
div al, byte [esp + 4]
mov eax, edx
ret
.size __modui8, .-__modui8

# signed

.global __modsi32
.type __modsi32, @function
__modsi32:
mov eax, dword [esp + 8]
xor edx, edx
idiv eax, dword [esp + 4]
mov eax, edx
ret
.size __modsi32, .-__modsi32

.global __modsi16
.type __modsi16, @function
__modsi16:
xor eax, eax
mov ax, word [esp + 6]
xor edx, edx
idiv ax, word [esp + 4]
mov eax, edx
ret
.size __modsi16, .-__modsi16

.global __modsi8
.type __modsi8, @function
__modsi8:
xor eax, eax
mov al, byte [esp + 5]
xor edx, edx
idiv al, byte [esp + 4]
mov eax, edx
ret
.size __modsi8, .-__modsi8
