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

.global __modui64
.type __modui64, @function
__modui64:
mov rax, rdi
xor rdx, rdx
div rsi
mov rax, rdx
ret
.size __modui64, .-__modui64

.global __modui32
.type __modui32, @function
__modui32:
mov eax, edi
xor rdx, rdx
div esi
mov rax, rdx
ret
.size __modui32, .-__modui32

.global __modui16
.type __modui16, @function
__modui16:
movzx rax, di
xor rdx, rdx
div si
mov rax, rdx
ret
.size __modui16, .-__modui16

.global __modui8
.type __modui8, @function
__modui8:
movzx rax, dil
xor rdx, rdx
div sil
mov rax, rdx
ret
.size __modui8, .-__modui8

# signed

.global __modsi64
.type __modsi64, @function
__modsi64:
mov rax, rdi
xor rdx, rdx
idiv rsi
mov rax, rdx
ret
.size __modsi64, .-__modsi64

.global __modsi32
.type __modsi32, @function
__modsi32:
mov eax, edi
xor rdx, rdx
idiv esi
mov rax, rdx
ret
.size __modsi32, .-__modsi32

.global __modsi16
.type __modsi16, @function
__modsi16:
movzx rax, di
xor rdx, rdx
idiv si
mov rax, rdx
ret
.size __modsi16, .-__modsi16

.global __modsi8
.type __modsi8, @function
__modsi8:
movzx rax, di
xor rdx, rdx
idiv sil
mov rax, rdx
ret
.size __modsi8, .-__modsi8
