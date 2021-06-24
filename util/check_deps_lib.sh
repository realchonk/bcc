#!/bin/sh

[ "$(basename "$0")" = "check_deps_lib.sh" ] && echo "This is not a program!" >&2 && exit 1

[ -z "${ARCH}" ] && echo "ARCH is not defined" >&2 && exit 1
[ -z "${TOP}" ] && echo "TOP is not defined" >&2 && exit 1

find_prog() {
   whereis "$1" | awk '{print $2}'
}

missing_dep() {
   echo "Missing dependency: $1" >&2
   exit 1
}

check_dep() {
   [ -z "$(find_prog "$1")" ] && missing_dep "$1"
}
