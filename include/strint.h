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

#ifndef FILE_STRINT_H
#define FILE_STRINT_H
#include <stddef.h>

//  Interned strings can be compared for equality with == instead of strcmp()
typedef const char* istr_t;

istr_t strint(const char*);            // Creates an interned string from a zero-terminated string
istr_t strnint(const char*, size_t);   // Creates an interned string

#define strrint(begin, end) (strnint((begin), ((end) - (begin))))

#endif /* FILE_STRINT_H */
