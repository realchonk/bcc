#!/usr/bin/env bash


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
riscv64|rv64i*([mafdgc]))
   echo "riscv64"
   ;;
*)
	exit 1
	;;
esac

