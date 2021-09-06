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

global __modui32
__modui32:
mov eax, dword [esp + 8]
xor edx, edx
div eax, dword [esp + 4]
mov eax, edx
ret

global __modui16
__modui16:
xor eax, eax
mov ax, word [esp + 6]
xor edx, edx
div ax, word [esp + 4]
mov eax, edx
ret

global __modui8
__modui8:
xor eax, eax
mov al, byte [esp + 5]
xor edx, edx
div al, byte [esp + 4]
mov eax, edx
ret

; signed

global __modsi32
__modsi32:
mov eax, dword [esp + 8]
xor edx, edx
idiv eax, dword [esp + 4]
mov eax, edx
ret

global __modsi16
__modsi16:
xor eax, eax
mov ax, word [esp + 6]
xor edx, edx
idiv ax, word [esp + 4]
mov eax, edx
ret

global __modsi8
__modsi8:
xor eax, eax
mov al, byte [esp + 5]
xor edx, edx
idiv al, byte [esp + 4]
mov eax, edx
ret
