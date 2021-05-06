#!/bin/sh

case $(uname -m) in
x86_64)
	echo "x86_64"
	;;
i*86)
	echo "i386"
	;;
*)
	echo "Unsupported architecture" >&2
	exit 1
	;;
esac

