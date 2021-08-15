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

#ifndef FILE_STRDB_H
#define FILE_STRDB_H
#include <stdbool.h>
#include "strint.h"
#include "buf.h"

struct strdb_ptr {
   istr_t str;
   size_t idx, len;
};

extern char* strdb;

void strdb_init(void);
void strdb_free(void);

bool strdb_find(const char*, const struct strdb_ptr**);
bool strdb_add(const char*, const struct strdb_ptr**);

#endif /* FILE_STRDB_H */
