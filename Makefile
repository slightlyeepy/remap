.POSIX:

CC               = cc
LIBREMAP_SRC     = libremap.c
LIBREMAP_HEADERS = remap.h util/dynbuf.h
REMAP_SRC        = remap.c

CFLAGS = -std=c99 -pedantic -Os

PREFIX  = /usr/local

libremap.so: ${LIBREMAP_SRC} ${LIBREMAP_HEADERS}
	${CC} -fPIC -shared ${CFLAGS} -o libremap.so ${LIBREMAP_SRC}
remap: ${REMAP_SRC} libremap.so
	${CC} -L. -lremap ${CFLAGS} -o remap ${REMAP_SRC}
install: libremap.so remap
	mkdir -p ${DESTDIR}${PREFIX}/lib
	cp -f libremap.so ${DESTDIR}${PREFIX}/lib
	chmod 755 ${DESTDIR}${PREFIX}/lib/libremap.so
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f remap ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/remap
uninstall:
	rm -f ${DESTDIR}${PREFIX}/lib/libremap.so
	rm -f ${DESTDIR}${PREFIX}/bin/remap
clean:
	rm -f libremap.so remap
