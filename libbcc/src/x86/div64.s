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

.global __divui64
.type __divui64, @function
__divui64:
mov rax, rdi
xor rdx, rdx
div rsi
ret
.size __divui64, .-__divui64

.global __divui32
.type __divui32, @function
__divui32:
mov eax, edi
xor rdx, rdx
div esi
ret
.size __divui32, .-__divui32

.global __divui16
.type __divui16, @function
__divui16:
movzx rax, di
xor rdx, rdx
div si
ret
.size __divui16, .-__divui16

.global __divui8
.type __divui8, @function
__divui8:
movzx rax, dil
xor rdx, rdx
div sil
ret
.size __divui8, .-__divui8

# signed

.global __divsi64
.type __divsi64, @function
__divsi64:
mov rax, rdi
xor rdx, rdx
idiv rsi
ret
.size __divsi64, .-__divsi64

.global __divsi32
.type __divsi32, @function
__divsi32:
mov eax, edi
xor rdx, rdx
idiv esi
ret
.size __divsi32, .-__divsi32

.global __divsi16
.type __divsi16, @function
__divsi16:
movzx rax, di
xor rdx, rdx
idiv si
ret
.size __divsi16, .-__divsi16

.global __divsi8
.type __divsi8, @function
__divsi8:
movzx rax, di
xor rdx, rdx
idiv sil
ret
.size __divsi8, .-__divsi8
