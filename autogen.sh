#!/bin/sh
autoreconf -i || exit 1

# cpp is auto-initialized by autoconf

# Initialize libbcc
cd libbcc
./autogen.sh
cd ..
