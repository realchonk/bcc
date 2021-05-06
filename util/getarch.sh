#!/bin/sh

if [ $# -eq 0 ]; then
   arch="$(uname -m)"
else
   arch="$1"
fi

case $arch in
x86_64)
	echo "x86_64"
	;;
i*86)
	echo "i386"
	;;
*)
	exit 1
	;;
esac

