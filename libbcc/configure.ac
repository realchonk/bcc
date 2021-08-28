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

m4_define([bcc_VERSION], m4_esyscmd_s([../util/version.sh]))
AC_INIT([libbcc], m4_defn([bcc_VERSION]), [benni@stuerz.xyz], [libbcc-]m4_defn([bcc_VERSION]),
        [https://github.com/Benni3D/bcc])

# Initialization stuff
AC_PREREQ([2.69])
AC_CONFIG_AUX_DIR([build-aux])
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([1.16 foreign subdir-objects])
AC_CONFIG_MACRO_DIRS([../util/m4])

AC_SUBST([version], m4_defn([bcc_VERSION]))

AC_PROG_CC
AC_PROG_RANLIB

AX_CHECK_TARGET
AX_SET_COMPILERDIRS

if test $ARCH = x86; then
   AX_FIND_NASM
else
   AX_FIND_AS
fi

# --with-bcc=
AC_ARG_WITH([bcc],
            [AS_HELP_STRING([--with-bcc=PATH], [path to bcc])],
            BCC=$withval,
            BCC=../bcc)

AC_SUBST([BCC])

AM_CONDITIONAL([BITS_32], [test $BITS = 32 ])

# Generate a Makefile
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
