#!/bin/sh
autoreconf -i || exit 1

# Initialize bcpp
cd cpp
./autogen.sh
cd ..

# Initialize libbcc
cd libbcc
./autogen.sh
cd ..
