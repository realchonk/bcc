#!/bin/sh

die() {
   echo "$1" >&2
   exit 1
}

[ "$(basename "$PWD")" != "bcc" ] && die "This script must be run in the project root."

[ -d pkg ] && rm -rf pkg

./autogen.sh      || exit 1
./configure --program-prefix=test-      || exit 1
make -j$(nproc)   || exit 1
mkdir pkg         || exit 1
make DESTDIR="$PWD/pkg" install || exit 1
make full-clean
find pkg
