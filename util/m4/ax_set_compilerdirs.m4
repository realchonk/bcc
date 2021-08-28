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
#  Sets the installation directories of the compiler and libraries.

AC_DEFUN([AX_SET_COMPILERDIRS], [

AC_SUBST([compilerdir], ['${prefix}/lib/bcc/${TARGET}/${version}'])
AC_SUBST([includedir],  ['${compilerdir}/include'])
AC_SUBST([targetdir],   ['${prefix}/${TARGET}'])
AC_SUBST([tbindir],     ['${targetdir}/bcc-bin/${version}'])
AC_SUBST([clibdir],     ['${compilerdir}'])

AC_DEFINE_UNQUOTED([TARGETDIR], [PREFIX "/lib/bcc/${TARGET}/${version}"],
                   [Path to the target directory])
AC_DEFINE_UNQUOTED([COMPILERDIR], [PREFIX "/lib/bcc/${TARGET}/${version}"],
                   [Path to the compiler directory])
])
