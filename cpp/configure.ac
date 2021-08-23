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

AC_INIT([bcpp], m4_esyscmd_s([../util/version.sh]), [benni@stuerz.xyz])

AC_PREREQ([2.69])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([build-aux])
AC_CANONICAL_TARGET 
AM_INIT_AUTOMAKE([1.16 foreign subdir-objects -Wall])

AC_PROG_CC

AC_FUNC_STRNLEN
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([realpath strerror])

AC_CHECK_HEADERS([stddef.h stdint.h stdlib.h string.h unistd.h stdnoreturn.h])
AC_CHECK_HEADER_STDBOOL

AC_TYPE_SIZE_T
AC_C_INLINE

# TODO: --with-includes=
SYS_INCLUDES="/usr/include:/usr/local/include"

AC_DEFINE_UNQUOTED([SYS_INCLUDES], ["$SYS_INCLUDES"], [The default system-wide incldues])

# Patch bcpp.1
sed "s/VERSION/$VERSION/g" ${srcdir}/src/bcpp.1 >bcpp.1 || AC_MSG_ERROR([failed to patch bcpp.1])

# Generate include/help_options.h
${srcdir}//../util/read_doc.sh <bcpp.1 >help_options.h || AC_MSG_ERROR([failed to generate help options])

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
