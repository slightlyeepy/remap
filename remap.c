#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int
main(int argc, char *argv[])
{
	if (argc < 3) {
		fprintf(stderr, "usage: %s [map string] [program] [program arguments ...]\n", argv[0]);
		return 1;
	}
	setenv("LD_PRELOAD", "libremap.so", 1);
	setenv("_LIBREMAP__MAPS", argv[1], 1);
	execvp(argv[2], argv + 2);
	fprintf(stderr, "%s: ", argv[0]);
	perror("execvp");
	return 1;
}
