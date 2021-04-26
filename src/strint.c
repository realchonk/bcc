#include <stdlib.h>
#include <string.h>
#include "strint.h"
#include "buf.h"

struct entry {
	char* str;
	size_t len;
};

static struct entry* entries = NULL;

/*
static size_t strnlen(const char* str, size_t num) {
	size_t i = 0;
	while (i < num && str[i]) ++i;
	return i;
}
*/

static istr_t do_strint(const char* str, size_t len) {
	for (size_t i = 0; i < buf_len(entries); ++i) {
		if (entries[i].len == len && (entries[i].str == str || !memcmp(entries[i].str, str, len)))
			return entries[i].str;
	}
	char* buf = (char*)malloc(len + 1);
	if (!buf) return NULL;
	memcpy(buf, str, len);
	buf[len] = '\0';
	buf_push(entries, ((struct entry){ buf, len }));
	return buf;
}

istr_t strint(const char* str) {
	const size_t len = strlen(str);
	return do_strint(str, len);
}

istr_t strnint(const char* str, size_t len) {
	return do_strint(str, strnlen(str, len));
}
