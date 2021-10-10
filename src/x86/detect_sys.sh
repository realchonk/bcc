#!/bin/sh

arch="$1"
cpu="$2"

case "${arch}" in
i386)
   echo "${cpu},32"
   ;;
x86_64)
   echo "x86_64,64"
   ;;
esac

