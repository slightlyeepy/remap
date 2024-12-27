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

#include <err.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

const char *ascii_nonprintable[] = {
	"<nul>", "<soh>", "<stx>", "<etx>", "<eot>", "<enq>", "<ack>", "<bel>", "<bs>", "<ht>", "<lf>", "<vt>", "<ff>", "<cr>", "<so>", "<si>", "<dle>", "<dc1>", "<dc2>", "<dc3>", "<dc4>", "<nak>", "<syn>", "<etb>", "<can>", "<em>", "<sub>", "<esc>", "<fs>", "<gs>", "<rs>", "<us>"
};

static void
print_char(char c)
{
	if (c > 0x1f && c < 0x7f)
		write(STDOUT_FILENO, &c, 1);
	else if (c < 0x20)
		write(STDOUT_FILENO, ascii_nonprintable[c], strlen(ascii_nonprintable[c]));
	else if (c == 0x7f)
		write(STDOUT_FILENO, "<del>", 5);
}

int
main(void)
{
	struct termios tio, oldtio;
	char c;
	if (!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO))
		errx(1, "stdin and stdout must be a terminal");
	tcgetattr(STDIN_FILENO, &tio);
	memcpy(&oldtio, &tio, sizeof(struct termios));
	cfmakeraw(&tio);
	tcsetattr(STDIN_FILENO, TCSANOW, &tio);
	write(STDOUT_FILENO, "\033[2J\033[;H", 8);
	while (read(STDIN_FILENO, &c, 1) > 0)
		print_char(c);
	tcsetattr(STDIN_FILENO, TCSANOW, &oldtio);
	return 0;
}
