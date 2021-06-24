#!/bin/sh

. "$TOP/util/check_deps_lib.sh"

find_gnu() {
   local as
   as=$(find_prog "${ARCH}-linux-$1")
   [ -z "${as}" ] && as=$(find_prog "${ARCH}-linux-gnu-$1")
   [ -z "${as}" ] && as=$(find_prog "${ARCH}-linux-musl-$1")
   [ -z "${as}" ] && as=$(find_prog "${ARCH}-elf-$1")
   echo "${as}"
}

as=$(find_gnu as)
[ -z "${as}" ] && missing_dep "GNU as for ${ARCH}"
echo "const char* gnu_as = \"${as}\";" > "${TOP}/src/${ARCH}/as.c"

echo "Success"
