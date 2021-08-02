AC_INIT([bcc], [0.11], [benni@stuerz.xyz])

AC_PREREQ([2.69])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([build-aux])
AC_CANONICAL_TARGET 
AM_INIT_AUTOMAKE([1.16 foreign subdir-objects -Wall])

# Required programs
AC_PROG_AWK
AC_PROG_CC

die() {
   echo "configure error: $1"
   exit 1
}

# Set the target architecture
TARGET=$(./util/getarch.sh $(echo ${target} | cut -d'-' -f1)) || die "invalid target architecture ${target}"
AC_SUBST([TARGET])

# Check for target-dependent dependencies
TOP=$PWD ARCH=$TARGET ./src/$TARGET/check_deps.sh || die "please check all the dependencies"

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

# Do stupid stuff because every simpler solution does not work -_-
AM_CONDITIONAL([TARGET_x86], [test $TARGET = x86_64 -o $TARGET = i386])
AM_CONDITIONAL([TARGET_riscv], [test $TARGET = riscv32 -o $TARGET = riscv64])

# Patch bcc.1
sed "s/VERSION/$VERSION/g" src/bcc.1 >bcc.1 || die "failed to patch bcc.1"

# Generate include/help_options.h
./util/read_doc.sh <bcc.1 >include/help_options.h || die "failed to generate help options"

# Generate a Makefile
AC_CONFIG_FILES([Makefile])
AC_CONFIG_SUBDIRS([cpp])

AC_OUTPUT
