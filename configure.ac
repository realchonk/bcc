AC_INIT([bcc], [0.11], [benni@stuerz.xyz])

AC_PREREQ([2.69])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([1.16 foreign subdir-objects -Wall])

# Required programs
AC_PROG_AWK
AC_PROG_CC

# Suggested by autoscan
AC_FUNC_STRNLEN
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_FORK
AC_SEARCH_LIBS([powl], [m])
AC_CHECK_FUNCS([atexit strchr strdup strerror strrchr strtoul])

# Suggested by autoscan
AC_CHECK_HEADERS([fcntl.h stddef.h stdint.h stdlib.h string.h unistd.h])
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
              [AS_HELP_STRING([--disable-fp], [build with floating-point support])],
              ENABLE_FP=$enableval,
              ENABLE_FP=yes)
# Make it visible to Makefile.am
AC_SUBST([ENABLE_FP])

# Select the target architecture (default: host)
# TODO: change to --target= instead of --with-target=
AC_ARG_WITH(target,
            [AS_HELP_STRING([[--with-target=[ARG]]], [select the target architecture])],
            [TARGET=$(./util/getarch.sh ${withval})],
            [TARGET=$(./util/getarch.sh)])

# Make it visible to Makefile.am
AC_SUBST([TARGET])


# Generate a Makefile
AC_CONFIG_FILES([Makefile])
AC_CONFIG_SUBDIRS([cpp])

AC_OUTPUT
