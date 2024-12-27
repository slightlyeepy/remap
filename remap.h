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

#ifndef REMAP_H
#define REMAP_H

#define ARROW_UP "\033[A"
#define ARROW_DOWN "\033[B"
#define ARROW_RIGHT "\033[C"
#define ARROW_LEFT "\033[D"
#define DELETE "\033[3~"
#define INSERT "\033[2~"
#define HOME "\033[H"
#define END "\033[F"
#define PAGE_UP "\033[5~"
#define PAGE_DOWN "\033[6~"

enum map_type {
	MAP_TYPE_REMAP,
	MAP_TYPE_SPECIAL
};

enum bind_type {
	BIND_TYPE_CHAR,
	BIND_TYPE_ARROW_UP,
	BIND_TYPE_ARROW_DOWN,
	BIND_TYPE_ARROW_RIGHT,
	BIND_TYPE_ARROW_LEFT,
	BIND_TYPE_DELETE,
	BIND_TYPE_INSERT,
	BIND_TYPE_HOME,
	BIND_TYPE_END,
	BIND_TYPE_PAGE_UP,
	BIND_TYPE_PAGE_DOWN
};

enum special_map_type {
	SPECIAL_MAP_TYPE_TOGGLE,
	SPECIAL_MAP_TYPE_DISABLE,
	SPECIAL_MAP_TYPE_ENABLE,
	SPECIAL_MAP_TYPE_FORCE_QUIT
};

struct bind {
	enum bind_type type;
	char ch;
};

struct map {
	enum map_type map_type;
	enum special_map_type special_map_type;
	struct bind bind;
	struct bind bindto;
};

#endif /* REMAP_H */
