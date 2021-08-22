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
#
#  Searches for the NASM assembler (only x86)

AC_DEFUN([AX_FIND_NASM], [
AC_PATH_PROG([ac_NASM], [nasm], [no])
if test x${ac_NASM} = xno; then
   AC_MSG_ERROR([NASM assembler not found])
else
   AC_DEFINE_UNQUOTED([NASM], ["${ac_NASM}"], [Path to the NASM assembler])
fi
])
