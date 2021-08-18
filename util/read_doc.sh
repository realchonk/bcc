#!/bin/sh
#  Copyright (C) 2021 Benjamin Stürz
#  
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#  
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#  
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <https://www.gnu.org/licenses/>.


# Wait for OPTIONS
while read line; do
   [ "${line}" = ".SH OPTIONS" ] && found_options="yes" && break
done

[ -z "${found_options}" ] && echo "Failed to find OPTIONS" && exit 1

echo "const char* help_options ="

while read line; do
   [ -z "${line}" ] && break
   if echo "${line}" | grep -q '^\.B '; then
      option="$(echo "${line}" | awk '{print $2, $3}') "
      while read line; do
         [ "${line}" = ".RS 5" ] && break
      done
      read info
      while read line; do
         [ "${line}" = ".RE" ] && break
         info="${info} ${line}"
      done
      len_option=${#option}
      printf "\t\"  %-20s %s\\\\n\"\n" "$(echo "${option}" | sed 's/f[IR]//g')" "$(echo "${info}" | sed 's/\.[A-Z]\+\s//g;s/f[IR]//g')"
   fi
done

echo ";"
