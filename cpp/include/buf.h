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

#ifndef FILE_BUF_H
#define FILE_BUF_H
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef my_min
#define my_min(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef my_max
#define my_max(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef arraylen
#define arraylen(a) (sizeof(a) / sizeof(*(a)))
#endif

struct BufHdr {
	size_t cap, len;
	char buf[0];
};

#define buf__hdr(b) ((struct BufHdr*)((char*)(b) - offsetof(struct BufHdr, buf)))
#define buf_size(buf) ((buf) ? buf__hdr(buf)->len : 0)
#define buf_len(buf) buf_size(buf)
#define buf_capacity(buf) ((buf) ? buf__hdr(buf)->cap : 0)
#define buf__fit(buf, n) ((n) <= buf_capacity(buf) ? 0 : ((buf) = buf__grow((buf), (n), (sizeof(*(buf))))))
#define buf_push(buf, elem) (buf__fit((buf), 1 + buf_size(buf)), (buf)[buf__hdr(buf)->len++] = (elem))
#define buf_pop(buf) (assert(buf_len(buf) != 0), (buf)[--buf__hdr(buf)->len])
#define buf_free(buf) ((buf) ? (free(buf__hdr(buf)), (buf) = NULL) : 0)
#define buf_last(buf) (*((buf) + buf_size(buf) - 1))
#define buf_reserve(buf, len) ((buf_size(buf) < (len)) ? ((buf) = buf__grow((buf), (len), sizeof(*(buf)))) : (buf))
#define buf__insert(buf, pos, elem) (buf__fit((buf), 1 + buf_size(buf)), memmove((buf) + (pos) + 1, (buf) + (pos), sizeof(*(buf)) * (buf_len(buf) - (pos))), (buf)[pos] = (elem), ++(buf__hdr(buf)->len))
#define buf_insert(buf, pos, elem) (buf__insert((buf), my_min((pos), buf_len((buf))), (elem)))
#define buf__remove(buf, pos, num) (memmove((buf) + (pos), (buf) + (pos) + (num), sizeof(*(buf)) * (buf_len(buf) - ((pos) + (num) - 1))), (buf__hdr(buf)->len -= (num)))
#define buf_remove(buf, pos, num) (pos < buf_len(buf) ? buf__remove((buf), (pos), my_min((num), buf_len(buf) - (pos)))  : 0)

inline static void* buf__grow(void* buf, size_t new_length, size_t elem_size) {
	assert(buf_capacity(buf) <= (SIZE_MAX - 1) / 2);
	const size_t new_cap = my_max(64, my_max(1 + 2 * buf_capacity(buf), new_length));
	assert(new_length <= new_cap);
	assert(new_cap <= (SIZE_MAX - offsetof(struct BufHdr, buf)) / elem_size);
	const size_t new_size = offsetof(struct BufHdr, buf) + new_cap * elem_size;

	struct BufHdr* new_hdr;
	if (buf)
		new_hdr = (struct BufHdr*)realloc(buf__hdr(buf), new_size);
	else {
		new_hdr = (struct BufHdr*)malloc(new_size);
		if (!new_hdr) return NULL;
		new_hdr->len = 0;
	}
	if (!new_hdr) return NULL;
	new_hdr->cap = new_cap;
	return (void*)new_hdr->buf;
}

#endif /* FILE_BUF_H */
