#include <ctype.h>
#include <stdio.h>
#include <unistd.h>

int
main(void)
{
	char buf[16];
	ssize_t nread;

	for (int i = 0; i < 15; ++i) {
		nread = read(STDIN_FILENO, buf, sizeof(buf));
		for (ssize_t j = 0; j < nread; ++j) {
			if (isprint(buf[j]))
				putchar(buf[j]);
			else
				printf("(%#x)", buf[j]);
		}
		putchar('\n');
	}

	return 0;
}
