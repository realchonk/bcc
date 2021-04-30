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
