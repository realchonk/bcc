#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "strint.h"
#include "buf.h"

struct entry {
	char* str;
	size_t len;
};

static struct entry* table[1024] = { NULL };

// source: (djb2) https://www.cse.yorku.ca/~oz/hash.html
static size_t do_hash(const char* s, size_t len) {
   size_t hash = 5381;
   for (size_t i = 0; i < len; ++i) {
      hash += (hash * 33) + s[i];
   }
   return hash % arraylen(table);
}

static istr_t do_strint(const char* str, size_t len) {
   const size_t hash = do_hash(str, len);
   struct entry* line = table[hash];
   for (size_t i = 0; i < buf_len(line); ++i) {
      if (len == line[i].len && !memcmp(str, line[i].str, len))
         return line[i].str;
   }
   char* new_str = malloc(len + 1);
   assert(new_str != NULL);
   memcpy(new_str, str, len);
   new_str[len] = '\0';
   struct entry e;
   e.str = new_str;
   e.len = len;
   buf_push(line, e);
   table[hash] = line;
   return new_str;
}

istr_t strint(const char* s) {
   return do_strint(s, strlen(s));
}
istr_t strnint(const char* s, size_t len) {
   return do_strint(s, strnlen(s, len));
}
