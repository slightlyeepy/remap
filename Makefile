.POSIX:

CC               = cc
REMAP_SRC        = remap.c
LIBREMAP_SRC     = libremap.c
LIBREMAP_HEADERS = remap.h util/dynbuf.h

CFLAGS = -std=gnu99 -Os -Wall -Wextra

PREFIX  = /usr/local

remap: ${REMAP_SRC} libremap.so
	${CC} -L. -lremap ${CFLAGS} -o remap ${REMAP_SRC}
libremap.so: ${LIBREMAP_SRC} ${LIBREMAP_HEADERS}
	${CC} -fPIC -shared ${CFLAGS} -o libremap.so ${LIBREMAP_SRC}
install: remap libremap.so
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f remap ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/remap
	mkdir -p ${DESTDIR}${PREFIX}/lib
	cp -f libremap.so ${DESTDIR}${PREFIX}/lib
	chmod 755 ${DESTDIR}${PREFIX}/lib/libremap.so
uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/remap
	rm -f ${DESTDIR}${PREFIX}/lib/libremap.so
clean:
	rm -f remap libremap.so
