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
#
#  Searches for the GNU assembler for the target

AC_DEFUN([AX_FIND_AS], [
if test x${target_alias} = x; then
   if test x${host} = x${target}; then
      ac_pre_as=as
   else
      echo host: ${host}
      echo target: ${target}
      AC_MSG_ERROR([failed to determine target for assembler. Please specify --target=])
   fi
else
   ac_pre_as=${target_alias}-as
fi

AC_PATH_PROG([ac_path_as], [${ac_pre_as}], [no])

if test x$ac_path_as = xno; then
   AC_MSG_ERROR([GNU assembler not found])
else
   AC_DEFINE_UNQUOTED([GNU_AS], ["${ac_path_as}"], [Path to the GNU assembler])
fi

])
