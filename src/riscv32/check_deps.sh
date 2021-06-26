#!/bin/sh

. "$TOP/util/check_deps_lib.sh"

as=$("$TOP/util/find_as.sh" riscv32) || missing_dep "GNU assembler for ${ARCH}"

echo "const char* gnu_as = \"${as}\";" > "${TOP}/src/${ARCH}/as.c"

echo "Success"
