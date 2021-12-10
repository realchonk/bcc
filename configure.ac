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

m4_define([bcc_VERSION], m4_esyscmd_s([./util/version.sh]))
AC_INIT([bcc], m4_defn([bcc_VERSION]), [benni@stuerz.xyz], [bcc-]m4_defn([bcc_VERSION]),
        [https://github.com/Benni3D/bcc])

# Initialization stuff
AC_PREREQ([2.69])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([build-aux])
AC_CANONICAL_TARGET 
AM_INIT_AUTOMAKE([1.16 foreign subdir-objects -Wall])
AC_CONFIG_MACRO_DIRS([util/m4])

AC_SUBST([version], m4_defn([bcc_VERSION]))

# Search for required programs
AC_PROG_AWK
AC_PROG_CC

# Determine the architecture
AX_CHECK_TARGET
AX_SET_PREDEF_MACROS
AX_SET_COMPILERDIRS

# Find the GNU binutils
AX_FIND_AS
AX_FIND_LD

# Suggested by autoscan
AC_FUNC_STRNLEN
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_FORK
AC_SEARCH_LIBS([powl], [m])
AC_CHECK_FUNCS([atexit strchr strdup strerror strrchr strtoul])

# Suggested by autoscan
AC_CHECK_HEADERS([fcntl.h stddef.h stdint.h stdlib.h string.h unistd.h stdnoreturn.h])
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_PID_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

# Option to disable preliminary floating-point support
AC_ARG_ENABLE(fp,
              [AS_HELP_STRING([--disable-fp], [disable preliminary floating-point support])],
              [if test x$enableval = xyes; then ENABLE_FP=1; else ENABLE_FP=0; fi],
              ENABLE_FP=1)
AC_DEFINE_UNQUOTED([ENABLE_FP], [$ENABLE_FP], [Enable/Disable floating-point support])
AC_SUBST([ENABLE_FP])

# Option to disable the installation of bcl
AC_ARG_ENABLE(bcl,
              [AS_HELP_STRING([--enable-bcl], [install the deprecated bcl wrapper])],
              [if test x$enableval = xyes; then ENABLE_BCL=1; else ENABLE_BCL=0; fi],
              ENABLE_BCL=0)
AC_SUBST([ENABLE_BCL])

# Option to disable the installation of libbcc
AC_ARG_ENABLE(target-libbcc,
              [AS_HELP_STRING([--disable-target-libbcc], [disable the installation of the compiler-specific library libbcc])],
              [if test x$enableval = xyes; then ENABLE_LIBBCC=1; else ENABLE_LIBBCC=0; fi],
              ENABLE_LIBBCC=1)
AM_CONDITIONAL([ENABLE_LIBBCC], [test x$ENABLE_LIBBCC = x1])
AC_DEFINE_UNQUOTED([HAS_LIBBCC], [${ENABLE_LIBBCC}], [Will the compiler-support library, libbcc.a, be installed?])

# Option to disable the installation of bash-completions
AC_ARG_ENABLE(bash-completions,
              [AS_HELP_STRING([--disable-bash-completions], [disable the installation of bash-completions])],
              [if test x$enableval = xyes; then ENABLE_BASHCOMP=1; else ENABLE_BASHCOMP=0; fi],
              ENABLE_BASHCOMP=1)
AC_SUBST([ENABLE_BASHCOMP])

def_cpu_abi=$(sh ./src/${ARCH}/detect_sys.sh ${FULL_ARCH} ${target_cpu} ${target_os})
if test x${def_cpu_abi} = x; then
   AC_MSG_ERROR([failed to determine default CPU and ABI])
fi

# --with-cpu=
AC_ARG_WITH([cpu],
            [AS_HELP_STRING([--with-cpu=CPU], [select the default target processor])],
            [DEF_CPU=$withval],
            [DEF_CPU=$(echo ${def_cpu_abi} | cut -d',' -f1)])

AC_DEFINE_UNQUOTED([DEF_CPU], ["${DEF_CPU}"], [Default processor])

# --with-abi=
AC_ARG_WITH([abi],
            [AS_HELP_STRING([--with-abi=ABI], [select the default target ABI])],
            [DEF_ABI=$withval],
            [DEF_ABI=$(echo ${def_cpu_abi} | cut -d',' -f2)])

AC_DEFINE_UNQUOTED([DEF_ABI], ["${DEF_ABI}"], [Default ABI])

# --with-dl=
AC_ARG_WITH([dl],
            [AS_HELP_STRING([--with-dl=PATH], [overwrite the default path to the dynamic linker])],
            [AC_DEFINE_UNQUOTED([PATH_DL], ["${withval}"], [path to the dynamic linker])])

# --with-includes=
AC_ARG_WITH([includes],
            [AS_HELP_STRING([--with-includes=STRING], [overwrite the default include paths])],
            [SYS_INCLUDES="$withval"])

# Patch bcc.1
sed "s/VERSION/$VERSION/g" ${srcdir}/src/bcc.1 >bcc.1 || AC_MSG_ERROR([failed to patch bcc.1])

# Generate include/help_options.h
${srcdir}/util/read_doc.sh <bcc.1 >help_options.h || AC_MSG_ERROR([failed to generate help options])


# Generate a Makefile
AC_CONFIG_FILES([Makefile])

AC_OUTPUT
