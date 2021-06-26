#!/bin/bash

print_asses() {
   for dir in $(echo "${PATH}" | tr ':' '\n'); do
      ls "$dir" 2>/dev/null | grep "${ARCH}-.*-as" | awk "{print \"${dir}/\" \$0}"
   done
}

[ $# -ne 1 ] && echo "Usage: $(basename "$0") <arch>" && exit 1
ARCH="$1"

list="$(print_asses)"
[ -z "${list}" ] && exit 1
linux="$(echo "${list}" | grep linux)"
[ -n "${linux}" ] && echo "${linux}" | head -n1 && exit 0
echo "${list}" | head -n1
