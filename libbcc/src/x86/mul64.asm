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

global __mului64
__mului64:
mov rax, rdi
mul rsi
ret

global __mului32
__mului32:
mov eax, edi
mul esi
ret

global __mului16
__mului16:
movzx rax, di
mul si
ret

global __mului8
__mului8:
movzx rax, dil
mul sil
ret

; signed

global __mulsi64
__mulsi64:
mov rax, rdi
imul rsi
ret

global __mulsi32
__mulsi32:
mov eax, edi
imul esi
ret

global __mulsi16
__mulsi16:
movzx rax, di
imul si
ret

global __mulsi8
__mulsi8:
movzx rax, di
imul sil
ret
