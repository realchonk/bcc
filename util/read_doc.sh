#!/usr/bin/env bash

# Wait for OPTIONS
while read line; do
   [ "${line}" = ".SH OPTIONS" ] && found_options="yes" && break
done

[ -z "${found_options}" ] && echo "Failed to find OPTIONS" && exit 1

echo "const char* help_options ="

while read line; do
   [ -z "${line}" ] && break
   if [[ ${line} =~ \.B ]]; then
      option="$(echo "${line}" | awk '{print $2}') "
      while read line; do
         [ "$line" = ".RS 5" ] && break
      done
      read info
      while read line; do
         [ "$line" = ".RE" ] && break
         info+=" $line"
      done
      len_option=${#option}
      printf "\t\"  %-20s %s\\\\n\"\n" "$option" "$(echo "$info" | sed 's/\.[A-Z]\+\s//g;s/f[IR]//g')"
   fi
done

echo ";"
