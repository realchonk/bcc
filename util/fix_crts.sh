#!/bin/sh

# Run this script, if bcc can't find crt{1,i,n}.o

print_usage() {
   echo "Usage: $(basename "$0") prefix target"
}

if [ $# -eq 1 ]; then
   [ "$1" = "-h" ] && print_usage && exit 0
   [ -z "${MACHTYPE}" ] && echo "$(basename "$0"): failed to determine machine type. Please run '$0 -h'" && exit 1
   prefix="$1"
   target="${MACHTYPE}"
elif [ $# -eq 2 ]; then
   prefix="$1"
   target="$2"
else
   print_usage >&2
   exit 1
fi

prefix="$1"
target="$2"

fix_crt() {
   ln -sfv "$(gcc -print-file-name="$1")" "${prefix}/${target}/$1"
}

fix_crt "crt1.o" || exit 1
fix_crt "crti.o" || exit 1
fix_crt "crtn.o" || exit 1
echo "Successfully fixed the crt{1,i,n}.o files."
