/* SPDX-License-Identifier: GPL-2.0 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

static int open_test(void)
{
	const struct test {
		const char	*name;
		const char	*dev_name;
		int		flags;
		mode_t		mode;
	} tests[] = {
		{
			.name =		"read only",
			.dev_name =	"/dev/scullp0",
			.flags =	O_RDONLY,
			.mode =		S_IRUSR,
		},
		{
			.name =		"write only",
			.dev_name =	"/dev/scullp1",
			.flags =	O_WRONLY,
			.mode =		S_IWUSR,
		},
		{
			/* sentry */
		},
	};
	const struct test *t;
	int fd;
	int i;

	i = 1;
	for (t = tests; t->name != NULL; t++) {
		printf("%d: %s: ", i++, t->name);
		fd = open(t->dev_name, t->flags, t->mode);
		if (fd == -1) {
			perror("open");
			return 1;
		}
		if (close(fd)) {
			perror("close");
			return 1;
		}
		puts("[OK]");
	}
	return 0;
}

int main(void)
{
	if (open_test())
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}
