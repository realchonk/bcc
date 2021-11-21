#!/bin/sh

cpusfile="$(dirname "$0")/cpus.h"

has_cpu() {
   local line
   
   while read -r line; do
      echo "${line}" | grep -q "^\\s*\\.name\\s*=\\s*\"$1\"\\s*\\,\\s*$" && return 0
   done
   return 1
}

# Detect Local CPU
cpu="$(uname -m)"
case "${cpu}" in
armv*l)
   cpu="$(echo "${cpu}" | sed 's/l$//')"
   ;;
armv*h)
   cpu="$(echo "${cpu}" | sed 's/h$//')"
   #has_hard_float=1
   ;;
armv*)
   ;;
*)
   cpu="armv4"
   ;;
esac

# Check if CPU is supported
if ! has_cpu "${cpu}" <"${cpusfile}"; then
   cpu="armv4"
fi

# Check for the floating-point ABI
case "$3" in
*eabihf)
   abi="32-hf"
   ;;
*)
   abi="32-sf"
   ;;
esac


echo "${cpu},${abi}"
