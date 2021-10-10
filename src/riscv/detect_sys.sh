#!/bin/sh

arch="$1"
cpu="$2"

# determine the CPU by looking at /proc/cpuinfo
out_cpu="$(grep 'isa\s*:\s*rv' /proc/cpuinfo    \
         | head -n1                             \
         | sed 's/^.*:\s*\(.*\)$/\1/'           \
         | sed 's/s[a-z]//')"


# determine the floating-pont ABI by looking at the Flags entry in the libc.so.6 file
tmp_abi="$(readelf -h "$(gcc -print-file-name=libc.so.6)"   \
         | grep 'Flags'                                     \
         | grep -o '\(soft\|single\|double\|\)-float')"

# determine the base ABI
case "${arch}" in
riscv32)
   base_abi="ilp32"
   ;;
riscv64)
   base_abi="lp64"
   ;;
esac

# determine the suffix for the ABI
case "${tmp_abi}" in
soft-float)
   abi_suffix=""
   ;;
single-float)
   abi_suffix="f"
   ;;
double-float)
   abi_suffix="d"
   ;;
esac

out_abi="${base_abi}${abi_suffix}"

echo "${out_cpu},${out_abi}"
