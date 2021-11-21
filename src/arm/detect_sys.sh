#!/bin/sh

case "$3" in
linux|linux-gnu|linux-gnueabi)
   abi="32-sf"
   ;;
linux-gnueabihf)
   abi="32-hf"
   ;;
*)
   abi="32-sf"
   ;;
esac

echo "armv4,${abi}"
