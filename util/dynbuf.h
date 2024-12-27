/*
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * For more information, please refer to <http://unlicense.org/>
 */

#ifndef DYNBUF_H
#define DYNBUF_H

#include <stddef.h>

struct dynbuf {
	char *buf;
	size_t size, len;
};

void dynbuf_init(/* (modifies) */ struct dynbuf *db);

void dynbuf_append(/* (modifies) */ struct dynbuf *db, const char *s, size_t l);
size_t dynbuf_pop_from_start(/* (modifies) */ struct dynbuf *db, /* (modifies) */ char *buf, size_t l);

#endif /* DYNBUF_H */

#ifdef DYNBUF_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>

#ifndef DYNBUF_INITIAL_SIZE
#define DYNBUF_INITIAL_SIZE 16
#endif /* DYNBUF_INITIAL_SIZE */

#ifndef DYNBUF_SIZE_INCREMENT
#define DYNBUF_SIZE_INCREMENT 32
#endif /* DYNBUF_SIZE_INCREMENT */

#ifndef DYNBUF_OOM
#include <stdio.h>

#define DYNBUF_OOM() { \
	fputs("malloc: out of memory\n", stderr); \
	exit(1); \
}
#endif /* DYNBUF_OOM */

void
dynbuf_init(/* (modifies) */ struct dynbuf *db)
{
	db->buf = malloc(DYNBUF_INITIAL_SIZE);
	if (!db->buf)
		DYNBUF_OOM()

	db->buf[0] = '\0';
	db->size = DYNBUF_INITIAL_SIZE;
	db->len = 0;
}

void
dynbuf_append(/* (modifies) */ struct dynbuf *db, const char *s, size_t l)
{
	if (db->len + l >= db->size) {
		char *olddbbuf = db->buf;
		db->size += l + DYNBUF_SIZE_INCREMENT;
		db->buf = realloc(db->buf, db->size);
		if (!db->buf) {
			free(olddbbuf);
			DYNBUF_OOM()
		}
	}
	memcpy(db->buf + db->len, s, l);
	db->len += l;
	db->buf[db->len] = '\0';
}

size_t
dynbuf_pop_from_start(/* (modifies) */ struct dynbuf *db, /* (modifies) */ char *buf, size_t l)
{
	size_t tocopy = (l < db->len) ? l : db->len;
	memcpy(buf, db->buf, tocopy);
	db->len -= tocopy;
	memmove(db->buf, db->buf + tocopy, db->len);
	db->buf[db->len] = '\0';
	return tocopy;
}

#endif /* DYNBUF_IMPLEMENTATION */
