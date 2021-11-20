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
#  Checks if the target architecture is supported

AC_DEFUN([AX_CHECK_TARGET], [
AC_MSG_CHECKING([for target architecture])

# Alternative names for processor architectures can be added here
case ${target_cpu} in
i386|i486|i586|i686)
   FULL_ARCH=i386
   ARCH=x86
   BITS=32
   ;;
x86_64)
   FULL_ARCH=x86_64
   ARCH=x86
   BITS=64
   ;;
riscv32)
   FULL_ARCH=riscv32
   ARCH=riscv
   BITS=32
   ;;
riscv64)
   FULL_ARCH=riscv64
   ARCH=riscv
   BITS=64
   ;;
arm|armv7l)
   FULL_ARCH=arm
   ARCH=arm
   BITS=32
   ;;
*)
   AC_MSG_ERROR([invalid target architecture '${target_cpu}'])
   ;;
esac
AC_MSG_RESULT([${FULL_ARCH}])

if test x${target_alias} != x; then
   TARGET=${target_alias}
else
   TARGET=${target}
fi

AC_DEFINE_UNQUOTED([BCC_TARGET], ["${TARGET}"], [The full cpu-machine-os target])
AC_DEFINE_UNQUOTED([BCC_FULL_ARCH], ["${FULL_ARCH}"], [The full architecture name])
AC_DEFINE_UNQUOTED([BCC_ARCH], ["${ARCH}"], [The base architecture name])
AC_DEFINE_UNQUOTED([BITS], [${BITS}], [How many bits a register has])
AC_DEFINE_UNQUOTED([SBITS], ["${BITS}"], [How many bits a register has (string variant)])

AC_SUBST([TARGET])
AC_SUBST([FULL_ARCH])
AC_SUBST([ARCH])
AC_SUBST([BITS])

AM_CONDITIONAL([ARCH_x86], [test x${ARCH} = xx86])
AM_CONDITIONAL([ARCH_riscv], [test x${ARCH} = xriscv])
AM_CONDITIONAL([ARCH_arm], [test x${ARCH} = xarm])

])
