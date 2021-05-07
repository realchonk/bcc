#!/bin/sh

path=$(whereis nasm | awk '{print $2}')
[ -z "$path" ] && echo "Missing dependency: nasm" >&2 && exit 1
echo Success 
