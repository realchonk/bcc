#!/bin/sh
autoreconf -i || exit 1

# cpp is auto-initialized by autoconf

# TODO: finish x86
exit

# Initialize libbcc
cd libbcc
./autogen.sh
cd ..
