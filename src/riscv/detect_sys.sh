#!/bin/sh

arch="$1"
cpu="$2"

# determine the base ABI
case "${arch}" in
riscv32)
   base_abi="ilp32"
   ;;
riscv64)
   base_abi="lp64"
   ;;
esac

if [ "${arch}" = "$(uname -m)" ]; then
   # If not cross-compiling
   # determine the CPU by looking at /proc/cpuinfo
   out_cpu="$(grep 'isa\s*:\s*rv' /proc/cpuinfo    \
            | head -n1                             \
            | sed 's/^.*:\s*\(.*\)$/\1/'           \
            | sed 's/s[a-z]//')"
   
   
   # determine the floating-pont ABI by looking at the Flags entry in the libc.so.6 file
   tmp_abi="$(readelf -h "$(gcc -print-file-name=libc.so.6)"   \
            | grep 'Flags'                                     \
            | grep -o '\(soft\|single\|double\|\)-float')"
   
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
else
   # If cross-compiling...
   case "${arch}" in
   riscv32)
      # Default values for 32bit
      out_cpu="rv32gc"
      abi_suffix=""
      ;;
   riscv64)
      # Default values for 64bit
      out_cpu="rv64gc"
      abi_suffix=""
      ;;
   esac
fi

out_abi="${base_abi}${abi_suffix}"

echo "${out_cpu},${out_abi}"
