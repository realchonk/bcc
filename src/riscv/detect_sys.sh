#!/bin/sh

arch="$1"
cpu="$2"

# determine the CPU by looking at /proc/cpuinfo
out_cpu="$(cat /proc/cpuinfo | grep 'isa\s*:\s*rv' | head -n1 | sed 's/^.*\(rv\(32|64\).\+\)$/\1/')"

# determine the ABI by looking at the Flags entry in the libc.so.6 file
out_abi=$(readelf -h "$(gcc -print-file-name=libc.so.6)" \
         | grep 'Flags'                                  \
         | sed 's/^.*\([a-z]\+\)-float ABI.*$/\1/')

echo "${out_cpu},${out_abi}"
