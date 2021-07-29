AC_INIT([bcpp], [0.11], [benni@stuerz.xyz])

AC_PREREQ([2.69])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([1.16 foreign subdir-objects -Wall])

AC_PROG_CC

AC_FUNC_STRNLEN
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([realpath strerror])

AC_CHECK_HEADERS([stddef.h stdint.h stdlib.h string.h unistd.h])
AC_CHECK_HEADER_STDBOOL

AC_TYPE_SIZE_T
AC_C_INLINE

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
