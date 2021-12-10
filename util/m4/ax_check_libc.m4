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
#  Sets the predefined macros

AC_DEFUN([AX_CHECK_LIBC], [
AC_MSG_CHECKING([for C library])

case ${target_os} in
linux|linux-gnu*)
   LIBC="glibc"
   ;;
linux-musl*)
   LIBC="musl"
   ;;
haiku)
   LIBC="libroot"
   ;;
*)
   AC_MSG_ERROR([unsupported combination of OS and C library, please look into util/m4/ax_check_libc.m4])
   ;;
esac

AC_MSG_RESULT([${LIBC}])

AC_SUBST([LIBC])

AC_DEFINE_UNQUOTED([LIBC_NAME], ["${LIBC}"], [Name of the C library])

])
