#!/bin/bash

print_asses() {
   for dir in $(echo "${PATH}" | tr ':' '\n'); do
      ls "$dir" 2>/dev/null | grep "${ARCH}-.*-${PROG}" | awk "{print \"${dir}/\" \$0}"
   done
}

if [ $# -eq 1 ]; then
   ARCH="$1"
   PROG="as"
elif [ $# -eq 2 ]; then
   ARCH="$1"
   PROG="$2"
else
   echo "Usage: $(basename "$0") <arch> [prog]" >&2
   exit 1
fi

list="$(print_asses)"
[ -z "${list}" ] && exit 1
linux="$(echo "${list}" | grep linux)"
[ -n "${linux}" ] && echo "${linux}" | head -n1 && exit 0
echo "${list}" | head -n1
