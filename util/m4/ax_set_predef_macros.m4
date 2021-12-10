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

AC_DEFUN([AX_SET_PREDEF_MACROS], [

ac_macros="__bcc__"

# C language specific
ac_macros="${ac_macros} __STDC_NO_ATOMICS__"
ac_macros="${ac_macros} __STDC_NO_COMPLEX__"
ac_macros="${ac_macros} __STDC_NO_THREADS__"
# __STDC_NO_VLA__ is defined in src/cpp.c

# Support for new operating systems can be added here:
case ${target_os} in
linux*)
   OS="linux"
   ac_has_libc=1
   ac_macros="${ac_macros} linux __linux __linux__"
   ac_macros="${ac_macros} unix __unix __unix__"
   ac_macros="${ac_macros} __ELF__"
   ;;
haiku)
   OS="haiku"
   ac_has_libc=1
   ac_macros="${ac_macros} __HAIKU__ __ELF__"
   ;;
elf*)
   OS="elf"
   ac_has_libc=0
   ac_macros="${ac_macros} __ELF__"
   ;;
*)
   AC_MSG_ERROR([unsupported operating system '${target_os}', please look into util/m4/ax_set_predef_macros.m4])
   ;;
esac


case ${FULL_ARCH} in
i386)
   ac_macros="${ac_macros} i386 __i386 __i386__"
   if test x${target_cpu} != xi386; then
      ac_macros="${ac_macros} __${target_cpu} __${target_cpu}__"
   fi
   ;;
x86_64)
   ac_macros="${ac_macros} __x86_64 __x86_64__ __amd64 __amd64__"
   ;;
riscv32)
   ac_macros="${ac_macros} __riscv __riscv_xlen=32"
   ;;
riscv64)
   ac_macros="${ac_macros} __riscv __riscv_xlen=64"
   ;;
arm)
   ac_macros="${ac_macros} __arm__"
   ;;
*)
   AC_MSG_ERROR([unsupported processor architecture, please look into util/m4/ax_set_predef_macros.m4}])
   ;;
esac

AC_DEFINE_UNQUOTED([CPP_MACROS], ["${ac_macros}"], [Predefined macros for the pre-processor])
AC_DEFINE_UNQUOTED([HAS_LIBC], [${ac_has_libc}], [Does the operating system have a C library?])
AC_SUBST([OS])

SYS_INCLUDES="$(cat "${srcdir}/config/os/${OS}/def_includes" | tr '\n' ':' | sed 's/:$//')"
AC_SUBST([SYS_INCLUDES])

if test x$ac_has_libc = x1; then
   AX_CHECK_LIBC
fi

])

