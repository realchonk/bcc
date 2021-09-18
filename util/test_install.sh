#!/bin/sh

die() {
   echo "$1" >&2
   exit 1
}

ask() {
   local y
   printf "%s? " "$1"
   read y
   [ "$y" = "y" ]
   return $?
}

[ "$(basename "$PWD")" != "bcc" ] && die "This script must be run in the project root."

[ -d pkg ] && rm -rf pkg

./autogen.sh      || exit 1
./configure $@    || exit 1
make -j$(nproc)   || exit 1

ask "Check" && make check

if ask "Test install"; then
   mkdir pkg         || exit 1
   make DESTDIR="$PWD/pkg" install || exit 1
   find pkg
fi

ask "Cleaup" && make full-clean
