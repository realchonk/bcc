#!/bin/bash


if [ $# -eq 0 ]; then
   arch="$(uname -m)"
else
   arch="$1"
fi

shopt -s extglob

case $arch in
x86_64)
	echo "x86_64"
	;;
i[3456]86)
	echo "i386"
	;;
riscv32|rv32i*([mafdgc]))
   echo "riscv32"
   ;;
*)
	exit 1
	;;
esac

