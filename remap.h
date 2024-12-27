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
