;  Copyright (C) 2021 Benjamin Stürz
;  
;  This program is free software: you can redistribute it and/or modify
;  it under the terms of the GNU General Public License as published by
;  the Free Software Foundation, either version 3 of the License, or
;  (at your option) any later version.
;  
;  This program is distributed in the hope that it will be useful,
;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;  GNU General Public License for more details.
;  
;  You should have received a copy of the GNU General Public License
;  along with this program.  If not, see <https://www.gnu.org/licenses/>.

; unsigned

global __divui64
__divui64:
mov rax, rdi
xor rdx, rdx
div rsi
ret

global __divui32
__divui32:
mov eax, edi
xor rdx, rdx
div esi
ret

global __divui16
__divui16:
movzx rax, di
xor rdx, rdx
div si
ret

global __divui8
__divui8:
movzx rax, dil
xor rdx, rdx
div sil
ret

; signed

global __divsi64
__divsi64:
mov rax, rdi
xor rdx, rdx
idiv rsi
ret

global __divsi32
__divsi32:
mov eax, edi
xor rdx, rdx
idiv esi
ret

global __divsi16
__divsi16:
movzx rax, di
xor rdx, rdx
idiv si
ret

global __divsi8
__divsi8:
movzx rax, di
xor rdx, rdx
idiv sil
ret
