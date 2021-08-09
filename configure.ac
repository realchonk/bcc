AC_INIT([bcc], [0.11], [benni@stuerz.xyz])

# Initialization stuff
AC_PREREQ([2.69])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([build-aux])
AC_CANONICAL_TARGET 
AM_INIT_AUTOMAKE([1.16 foreign subdir-objects -Wall])

# Search for required programs
AC_PROG_AWK
AC_PROG_CC

die() {
   echo "configure error: $1"
   exit 1
}

# set TARGET to the according value
if test "x${target_alias}" != "x"; then
   TARGET="${target_alias}"
elif test "x${target}" != "x"; then
   TARGET="${target}"
else
   TARGET="${host}"
fi

# Determine the architecture
case ${TARGET} in
i386-*)
   FULL_ARCH=i386
   ARCH=x86
   BITS=32
   ;;
x86_64-*)
   FULL_ARCH=x86_64
   ARCH=x86
   BITS=64
   ;;
riscv32-*)
   FULL_ARCH=riscv32
   ARCH=riscv
   BITS=32
   ;;
riscv64-*)
   FULL_ARCH=riscv64
   ARCH=riscv
   BITS=64
   ;;
*)
   die "Invalid architectue '${TARGET}'"
esac

# Set the target architecture
AC_SUBST([TARGET])
AC_SUBST([FULL_ARCH])
AC_SUBST([ARCH])
AC_SUBST([BITS])

# Pass the architecture info to Makefile.am
AM_CONDITIONAL([ARCH_x86], [test x$ARCH = xx86])
AM_CONDITIONAL([ARCH_riscv], [test x$ARCH = xriscv])

# Find the assembler (NASM for x86, GNU as for other)
if test $ARCH = x86; then
   AC_PATH_PROG(NASM, [nasm], [no])
   if test "x$nasm" = "xno"; then
      AC_MSG_ERROR([NASM assembler not found])
   else
      AC_DEFINE_UNQUOTED([NASM], ["$NASM"], [Path to the NASM assembler])
   fi
else
   if test "x${target_alias}" = "x"; then
      if test "x${host}" = "x${target}"; then
         PRE_AS=as
      else
         die "failed to determine target for assembler. Please specify --target="
      fi
   else
      PRE_AS="${target_alias}-as"
   fi
   AC_PATH_PROG(GNU_AS, [${PRE_AS}], [no])
   if test "x$GNU_AS" = "xno"; then
      AC_MSG_ERROR([GNU assembler not found])
   else
      AC_DEFINE_UNQUOTED([GNU_AS], ["$GNU_AS"], [Path to the GNU assembler])
   fi
fi

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

# --with-cpu=
AC_ARG_WITH([cpu],
            [AS_HELP_STRING([--with-cpu], [select the default target processor])],
            [AC_DEFINE_UNQUOTED([DEF_CPU], ["$withval"], [Default processor])])

# --with-abi=
AC_ARG_WITH([abi],
            [AS_HELP_STRING([--with-abi], [select the default target ABI])],
            [AC_DEFINE_UNQUOTED([DEF_ABI], ["$withval"], [Default application binary interface])])

# Make it visible to Makefile.am
AC_SUBST([ENABLE_FP])

# Patch bcc.1
sed "s/VERSION/$VERSION/g" ${srcdir}/src/bcc.1 >bcc.1 || die "failed to patch bcc.1"

# Generate include/help_options.h
${srcdir}/util/read_doc.sh <bcc.1 >help_options.h || die "failed to generate help options"

# Generate a Makefile
AC_CONFIG_FILES([Makefile])
AC_CONFIG_SUBDIRS([cpp])

AC_OUTPUT
