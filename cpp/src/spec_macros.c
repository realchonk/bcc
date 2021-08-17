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

#include <string.h>
#include <stdio.h>
#include <time.h>
#include "macro.h"
#include "cpp.h"

static char* spec_FILE(size_t linenum) {
   (void)linenum;
   if (!strcmp("-", source_name))
      return strdup("<stdin>");
   else return strdup(source_name);
}
static char* spec_DATE(size_t linenum) {
   (void)linenum;
   const size_t num = 64;
   char* date = malloc(num);
   if (!date)
      panic("failed to allocate date");
   const time_t t = time(NULL);
   struct tm* tm = localtime(&t);
   if (!strftime(date, num, "%b %d %Y", tm))
      panic("failed to format the date");
   return date;
}
static char* spec_TIME(size_t linenum) {
   (void)linenum;
   const size_t num = 64;
   char* date = malloc(num);
   if (!date)
      panic("failed to allocate date");
   const time_t t = time(NULL);
   struct tm* tm = localtime(&t);
   if (!strftime(date, num, "%T", tm))
      panic("failed to format the date");
   return date;
}

static char* spec_LINE(size_t linenum) {
   char* buf = malloc(64);
   snprintf(buf, 64, "%zu", linenum + 1);
   return buf;
}

#define add(n) \
   m.name = strint("__"#n"__"); \
   m.type = MACRO_SPEC; \
   m.linenum = 0; \
   m.text = ""; \
   m.handler = spec_##n; \
   add_macro(&m)

void init_macros(void) {
   struct macro m;
   add(FILE);
   add(DATE);
   add(TIME);
   add(LINE);
}
