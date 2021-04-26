#ifndef FILE_STRINT_H
#define FILE_STRINT_H
#include <stddef.h>

//  Interned strings can be compared for equality with == instead of strcmp()
typedef const char* istr_t;

istr_t strint(const char*);            // Creates an interned string from a zero-terminated string
istr_t strnint(const char*, size_t);   // Creates an interned string

#endif /* FILE_STRINT_H */
