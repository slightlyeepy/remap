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
