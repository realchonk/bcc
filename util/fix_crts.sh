#!/usr/bin/env bash

# Run this script, if bcc can't find crt{1,i,n}.o and/or libc.a

print_usage() {
   echo "Usage: $(basename "$0") [-ahf] [-t target] [-g GCC] prefix"
}
error() {
   echo "$(basename "$0"): $1" >&2
   exit 1
}
gcc_find() {
   file="$("${GCC}" -print-file-name="$1")"
   [ "${file}" = "$1" ] && error "failed to find '$1'"
   echo "${file}"
}
fix() {
   if [ ! -e "${libdir}/$1" -o "${force}" = "1" ]; then
      ln -sfv "$(gcc_find "$1")" "${libdir}/$1" || exit 1
      fixed="${fixed} $1"
   fi
}
fix_clib() {
   if [ ! -e "${clibdir}/$1" -o "${force}" = "1" ]; then
      ln -sfv "$(gcc_find "$1")" "${clibdir}/$1" || exit 1
      fixed="${fixed} $1"
   fi
}


args=$(getopt -o "ahg:t:fF:G" -- "$@") || exit 1
eval set -- "${args}"
unset args

target="${MACHTYPE}"
GCC="${MACHTYPE}-gcc"
BCC="bcc"
force=0
fix_all=0
fix_gcc=0
search_files=""

while true; do
   case "$1" in
   -h)
      echo "Usage: $(basename "$0") [options] prefix"
      echo
      echo "This script fixes missing crtX.o and libc.a files."
      echo
      echo "Options:"
      echo "  -h           Show this page."
      echo "  -a           Fix also crtbegin.o & crtend.o"
      echo "               use this only if linking against glibc fails."
      echo "  -f           Overwrite already existing files."
      echo "  -F file      Only print the path to file."
      echo "               (may be specified multiple times)"
      echo "  -t target    Specify the target of bcc."
      echo "  -g GCC       Specify the name of gcc."
      echo "  -G           Copy the libgcc.a"
      echo
      echo "Operands:"
      echo "  prefix       The installation prefix of bcc"
      echo "  target       The target of bcc (default: ${MACHTYPE})"
      echo "  GCC          The name of gcc (default: ${MACHTYPE}-gcc)"
      echo "  file         A file or files to be searched by gcc"
      exit 0
      ;;
   -a)
      fix_all=1
      shift
      ;;
   -G)
      fix_lgcc=1
      shift
      ;;
   -f)
      force=1
      shift
      ;;
   -F)
      search_files="${search_files} $2"
      shift 2
      ;;
   -t)
      target="$2"
      BCC="${target}-bcc"
      shift 2
      ;;
   -g)
      GCC="$2"
      shift 2
      ;;
   --)
      shift
      break
      ;;
   *)
      print_usage >&2
      exit 1
      ;;
   esac
done

if [ -n "${search_files}" ]; then
   for f in ${search_files}; do
      gcc_find "${f}"
   done
   exit 0
fi

[ -z "$1" ] && error "operand prefix is missing"
prefix="$1"
libdir="${prefix}/${target}/lib"
bcc="${prefix}/bin/${BCC}"
[ ! -x "${bcc}" ] && error "couldn't to find bcc"
version="$("${bcc}" -dumpversion)" || error "failed to get bcc's version"
clibdir="${prefix}/lib/bcc/${target}/${version}"
fixed=""

[ ! -d "${clibdir}" ] && error "directory not found: ${clibdir}"

mkdir -p "${libdir}" 
fix "crt1.o"
fix "crti.o"
fix "crtn.o"
fix "libc.a"
fix "libc.so"
if [ "${fix_all}" = 1 ]; then
   fix_clib "crtbegin.o" 
   fix_clib "crtend.o"   
fi
if [ "${fix_lgcc}" = 1 ]; then
   fix_clib "libgcc.a"
   fix_clib "libgcc_eh.a"
fi
if [ -z "${fixed}" ]; then
   echo "No files were fixed. Maybe try again with -f"
else
   echo "Successfully fixed${fixed}"
fi

