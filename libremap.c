/*
 * ============================================================================
 * includes
 */
#define _GNU_SOURCE
#include <asm/termbits.h>
#include <sys/ioctl.h>

#include <dlfcn.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "remap.h"

/*
 * ============================================================================
 * macros
 */
#define MIN(x, y) ((x) < (y) ? (x) : (y))

/*
 * ============================================================================
 * function declarations
 */

/* errors */
/* noreturn */ static void die(const char *fmt, ...);

/* atexit() cleanup */
static void cleanup(void);

/* reading map data */
static enum bind_type parse_bind_type(char c);
static enum special_map_type parse_special_map_type(char c);
/* (modifies global state) */ static void read_maps(void);

/* utility functions for binds */
static int binds_equal(const struct bind *bind1, const struct bind *bind2);

/* remap map handling */
static int try_remap(const struct bind *bind, /* (modifies) */ char *buf,
		size_t bufsz, /* (modifies) */ size_t *byteswritten);

/* special map handling */
/* (modifies global state) */ static int try_special(const struct bind *bind);

/* ASCII / escape sequence from/to bind conversion */
static size_t convert_from_bind(const struct bind *bind,
		/* (modifies) */ char *buf, size_t bufsz);
static size_t convert_to_bind(const char *s, size_t sl,
		/* (modifies) */ struct bind *bind);

/* read() override */
static ssize_t read_and_remap(int fd, /* (modifies) */ void *buf, size_t count);
/* (modifies global state) */ ssize_t read(int fd, /* (modifies) */ void *buf,
		size_t count);

/*
 * ============================================================================
 * library includes (listed here so that die() is visible)
 */
#define DYNBUF_IMPLEMENTATION
#define DYNBUF_OOM() die("libremap: error: out of memory");
#include "util/dynbuf.h"

/*
 * ============================================================================
 * global variables
 */
static ssize_t (*libc_read)(int, void *, size_t) = NULL;

static struct map *maps = NULL;
static size_t mapcnt;

static struct dynbuf pending;

static int enabled = 1;

/*
 * ============================================================================
 * errors
 */
/* noreturn */
static void
die(const char *fmt, ...)
{
	if (fmt) {
		va_list ap;
		va_start(ap, fmt);
		fputs("\033[31m", stderr);
		vfprintf(stderr, fmt, ap);
		fputs("\033[0m\n", stderr);
		va_end(ap);
	}
	exit(1);
}

/*
 * ============================================================================
 * atexit() cleanup
 */
static void
cleanup(void)
{
	free(pending.buf);
	free(maps);
}

/*
 * ============================================================================
 * reading map data
 */
static enum bind_type
parse_bind_type(char c)
{
	switch (c) {
	case 'L':
		return BIND_TYPE_ARROW_LEFT;
	case 'D':
		return BIND_TYPE_ARROW_DOWN;
	case 'U':
		return BIND_TYPE_ARROW_UP;
	case 'R':
		return BIND_TYPE_ARROW_RIGHT;
	case 'X':
		return BIND_TYPE_DELETE;
	case 'I':
		return BIND_TYPE_INSERT;
	case 'H':
		return BIND_TYPE_HOME;
	case 'E':
		return BIND_TYPE_END;
	case 'P':
		return BIND_TYPE_PAGE_UP;
	case 'V':
		return BIND_TYPE_PAGE_DOWN;
	default:
		return BIND_TYPE_CHAR;
	}
}

static enum special_map_type
parse_special_map_type(char c)
{
	switch (c) {
	case 'T':
		return SPECIAL_MAP_TYPE_TOGGLE;
	case 'D':
		return SPECIAL_MAP_TYPE_DISABLE;
	case 'E':
		return SPECIAL_MAP_TYPE_ENABLE;
	case 'Q':
		return SPECIAL_MAP_TYPE_FORCE_QUIT;
	}
}

/* (modifies global state) */
static void
read_maps(void)
{
	const char *mapstr = getenv("_LIBREMAP__MAPS");
	size_t mapstrlen;
	size_t i;

	if (!mapstr)
		die("libremap: error: _LIBREMAP__MAPS is not set");

	mapstrlen = strlen(mapstr);
	if (mapstrlen % 5 != 0)
		die("libremap: error: _LIBREMAP__MAPS contains invalid data");

	mapcnt = mapstrlen / 5;
	maps = malloc(sizeof(struct map) * mapcnt);
	if (!maps)
		die("libremap: error: out of memory");

	for (i = 0; i < mapcnt; ++i) {
		switch (mapstr[i * 5]) {
		case 'R':
			maps[i].map_type = MAP_TYPE_REMAP;
			maps[i].bind.type = parse_bind_type(mapstr[i * 5 + 1]);
			maps[i].bind.ch = mapstr[i * 5 + 2];
			maps[i].bindto.type = parse_bind_type(mapstr[i * 5 + 3]);
			maps[i].bindto.ch = mapstr[i * 5 + 4];
			break;
		case 'S':
			maps[i].map_type = MAP_TYPE_SPECIAL;
			maps[i].special_map_type = parse_special_map_type(
					mapstr[i * 5 + 1]);
			maps[i].bind.type = parse_bind_type(mapstr[i * 5 + 2]);
			maps[i].bind.ch = mapstr[i * 5 + 3];
			break;
		}
	}
}

/*
 * ============================================================================
 * utility functions for binds
 */
static int
binds_equal(const struct bind *bind1, const struct bind *bind2)
{
	if (bind1->type != bind2->type)
		return 0;
	if (bind1->type == BIND_TYPE_CHAR) {
		if (bind1->ch == bind2->ch)
			return 1;
		return 0;
	}
	return 1;
}

/*
 * ============================================================================
 * remap map handling
 */
static int
try_remap(const struct bind *bind, /* (modifies) */ char *buf, size_t bufsz,
		/* (modifies) */ size_t *byteswritten)
{
	size_t i;

	char buf2[4]; /* NOT null terminated */
	size_t l;
	for (i = 0; i < mapcnt; ++i) {
		if (maps[i].map_type == MAP_TYPE_REMAP && binds_equal(
					&maps[i].bind, bind)) {
			l = convert_from_bind(&maps[i].bindto, buf2, sizeof(buf2));
			if (l == 1) {
				*buf = buf2[0];
				*byteswritten = 1;
			} else {
				if (bufsz >= l) {
					memcpy(buf, buf2, l);
					*byteswritten = l;
				} else {
					memcpy(buf, buf2, bufsz);
					*byteswritten = bufsz;
					dynbuf_append(&pending, buf2 + bufsz,
							l - bufsz);
				}
			}
			return 1;
		}
	}
	return 0;
}

/*
 * ============================================================================
 * special map handling
 */
/* (modifies global state) */
static int
try_special(const struct bind *bind)
{
	size_t i;
	for (i = 0; i < mapcnt; ++i) {
		if (maps[i].map_type == MAP_TYPE_SPECIAL &&
				binds_equal(&maps[i].bind, bind)) {
			switch (maps[i].special_map_type) {
			case SPECIAL_MAP_TYPE_TOGGLE:
				enabled = !enabled;
				return 1;
			case SPECIAL_MAP_TYPE_DISABLE:
				enabled = 0;
				return 1;
			case SPECIAL_MAP_TYPE_ENABLE:
				enabled = 1;
				return 1;
			case SPECIAL_MAP_TYPE_FORCE_QUIT:
				if (enabled)
					exit(0);
				break;
			}
		}
	}
	return 0;
}

/*
 * ============================================================================
 * ASCII / escape sequence to bind conversion
 */
static size_t
convert_from_bind(const struct bind *bind, /* (modifies) */ char *buf, size_t bufsz)
{
	size_t ret;

	if (bufsz == 0)
		return 0;

	/* bufsz is now confirmed to be at least 1 */
	switch (bind->type) {
	case BIND_TYPE_CHAR:
		*buf = bind->ch;
		return 1;
	case BIND_TYPE_ARROW_UP:
		ret = MIN(sizeof(ARROW_UP) - 1, bufsz);
		memcpy(buf, ARROW_UP, ret);
		return ret;
	case BIND_TYPE_ARROW_DOWN:
		ret = MIN(sizeof(ARROW_DOWN) - 1, bufsz);
		memcpy(buf, ARROW_DOWN, ret);
		return ret;
	case BIND_TYPE_ARROW_RIGHT:
		ret = MIN(sizeof(ARROW_RIGHT) - 1, bufsz);
		memcpy(buf, ARROW_RIGHT, ret);
		return ret;
	case BIND_TYPE_ARROW_LEFT:
		ret = MIN(sizeof(ARROW_LEFT) - 1, bufsz);
		memcpy(buf, ARROW_LEFT, ret);
		return ret;
	case BIND_TYPE_DELETE:
		ret = MIN(sizeof(DELETE) - 1, bufsz);
		memcpy(buf, DELETE, ret);
		return ret;
	case BIND_TYPE_INSERT:
		ret = MIN(sizeof(INSERT) - 1, bufsz);
		memcpy(buf, INSERT, ret);
		return ret;
	case BIND_TYPE_HOME:
		ret = MIN(sizeof(HOME) - 1, bufsz);
		memcpy(buf, HOME, ret);
		return ret;
	case BIND_TYPE_END:
		ret = MIN(sizeof(END) - 1, bufsz);
		memcpy(buf, END, ret);
		return ret;
	case BIND_TYPE_PAGE_UP:
		ret = MIN(sizeof(PAGE_UP) - 1, bufsz);
		memcpy(buf, PAGE_UP, ret);
		return ret;
	case BIND_TYPE_PAGE_DOWN:
		ret = MIN(sizeof(PAGE_DOWN) - 1, bufsz);
		memcpy(buf, PAGE_DOWN, ret);
		return ret;
	}
}

static size_t
convert_to_bind(const char *s, size_t sl, /* (modifies) */ struct bind *bind)
{
	fprintf(stderr, "DEBUG: s[0] = %c, s[0] == '\\033' = %d, s[1] = %c, s[1] == '[' == %d\n",
			s[0], s[0] == '\033', s[1], s[1] == '[');
	fprintf(stderr, "DEBUG: s = %.*s, sl = %d\n", sl, s, sl);
	if (sl >= 2 && s[0] == '\033' && s[1] == '[') {
		if (sl >= 3 && s[3] == '~') {
			switch (s[2]) {
			case '3':
				bind->type = BIND_TYPE_DELETE;
				return 4;
			case '2':
				bind->type = BIND_TYPE_INSERT;
				return 4;
			case '5':
				bind->type = BIND_TYPE_PAGE_UP;
				return 4;
			case '6':
				bind->type = BIND_TYPE_PAGE_DOWN;
				return 4;
			}
		} else {
			switch (s[2]) {
			case 'A':
				bind->type = BIND_TYPE_ARROW_UP;
				return 3;
			case 'B':
				bind->type = BIND_TYPE_ARROW_DOWN;
				return 3;
			case 'C':
				bind->type = BIND_TYPE_ARROW_RIGHT;
				return 3;
			case 'D':
				bind->type = BIND_TYPE_ARROW_LEFT;
				return 3;
			case 'H':
				bind->type = BIND_TYPE_HOME;
				return 3;
			case 'F':
				bind->type = BIND_TYPE_END;
				return 3;
			}
		}
	}

	bind->type = BIND_TYPE_CHAR;
	bind->ch = s[0];
	return 1;
}

/*
 * ============================================================================
 * read() override
 */
static ssize_t
read_and_remap(int fd, /* (modifies) */ void *buf, size_t count)
{
	char *cbuf = (char *)buf;
	char *newbuf = malloc(count);

	ssize_t nread;
	struct bind bind;
	size_t bindlen;
	size_t readidx = 0;

	size_t byteswritten;
	size_t writeidx = 0;

	if (!newbuf)
		die("libremap: error: out of memory");

	nread = libc_read(fd, buf, count);
	if (nread < 0) {
		free(newbuf);
		return nread;
	}

	while (readidx < (size_t)nread) {
		bindlen = convert_to_bind(cbuf + readidx, count - readidx, &bind);

		fprintf(stderr, "DEBUG: %d\n", (int)bind.type);

		if (enabled && try_remap(&bind, newbuf + writeidx,
					count - writeidx, &byteswritten)) {
			writeidx += byteswritten;
		} else if (!try_special(&bind)) {
			if (writeidx < count)
				newbuf[writeidx++] = cbuf[readidx];
			else
				dynbuf_append(&pending, &cbuf[readidx], 1);
		}
		readidx += bindlen;
	}
	memcpy(buf, newbuf, writeidx);
	free(newbuf);
	return (ssize_t)writeidx;
}

/* (modifies global state) */
ssize_t
read(int fd, /* (modifies) */ void *buf, size_t count)
{
	static int pending_initialized = 0;
	static int cleanup_atexit_registered = 0;

	if (!libc_read) {
		libc_read = (ssize_t (*)(int, void *, size_t))dlsym(RTLD_NEXT, "read");
		if (!libc_read)
			die("libremap: error: dlsym: %s", dlerror());
	}

	if (isatty(fd)) {
		if (!maps)
			read_maps();

		if (!pending_initialized) {
			dynbuf_init(&pending);
			pending_initialized = 1;
		}

		if (!cleanup_atexit_registered) {
			atexit(cleanup);
			cleanup_atexit_registered = 1;
		}

		if (pending.len > 0) {
			char *cbuf = (char *)buf;

			size_t npending = dynbuf_pop_from_start(&pending,
					cbuf, count);
			ssize_t nread;
			int unread;

			/*
			 * if there's any pending data on the file descriptor
			 * that we can read without blocking, read it
			 */
			if (npending < count && ioctl(fd, FIONREAD, &unread) == 0 &&
					unread > 0) {
				nread = read_and_remap(fd, cbuf + npending,
						MIN(count - npending, unread));
				if (nread < 0)
					return (ssize_t)npending;
				return (ssize_t)npending + nread;
			}
			return (ssize_t)npending;
		}
		return read_and_remap(fd, buf, count);
	} else {
		return libc_read(fd, buf, count);
	}
}
