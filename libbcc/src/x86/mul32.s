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

.global __mului32
.type __mului32, @function
__mului32:
mov eax, dword [esp + 8]
mul dword ptr [esp + 4]
ret
.size __mului32, .-__mului32

.global __mului16
.type __mului16, @function
__mului16:
xor eax, eax
mov ax, word [esp + 6]
mul word ptr [esp + 4]
ret
.size __mului16, .-__mului16

.global __mului8
.type __mului8, @function
__mului8:
xor eax, eax
mov al, byte [esp + 5]
mul byte ptr [esp + 4]
ret
.size __mului8, .-__mului8

# signed

.global __mulsi32
.type __mulsi32, @function
__mulsi32:
mov eax, dword [esp + 8]
imul eax, dword [esp + 4]
ret
.size __mulsi32, .-__mulsi32

.global __mulsi16
.type __mulsi16, @function
__mulsi16:
xor eax, eax
mov ax, word [esp + 6]
imul ax, word [esp + 4]
ret
.size __mulsi16, .-__mulsi16

.global __mulsi8
.type __mulsi8, @function
__mulsi8:
xor eax, eax
mov al, byte [esp + 5]
imul byte ptr [esp + 4]
ret
.size __mulsi8, .-__mulsi8
