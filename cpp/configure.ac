AC_INIT([bcpp], [0.11], [benni@stuerz.xyz])

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

die() {
   echo "configure error: $1"
   exit 1
}


# Patch bcpp.1
sed "s/VERSION/$VERSION/g" ${srcdir}/src/bcpp.1 >bcpp.1 || die "failed to patch bcpp.1"

# Generate include/help_options.h
${srcdir}//../util/read_doc.sh <bcpp.1 >help_options.h || die "failed to generate help options"

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
