//  Copyright (C) 2021 Benjamin Stürz
//  
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//  
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//  
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef FILE_LINKER_H
#define FILE_LINKER_H
#include "cmdline.h"

extern const char* linker_path;
extern struct cmdline_arg* linker_args;
extern bool nostartfiles, nolibc, nobccobjs;

int run_linker(const char* output, const char** objects);

#endif /* FILE_LINKER_H */
