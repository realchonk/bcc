#!/bin/sh

case "$3" in
*eabihf)
   abi="32-hf"
   ;;
*)
   abi="32-sf"
   ;;
esac

echo "armv4,${abi}"
