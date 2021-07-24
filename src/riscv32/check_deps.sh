#!/bin/sh

. "$TOP/util/check_deps_lib.sh"

as=$("$TOP/util/find_as.sh" "${ARCH}") || missing_dep "GNU assembler for ${ARCH}"

echo "static const char* gnu_as = \"${as}\";" > "${TOP}/src/${ARCH}/as.h"

echo "Success"
