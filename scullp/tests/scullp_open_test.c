/* SPDX-License-Identifier: GPL-2.0 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "kselftest.h"

static int open_test(int *i)
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

	for (t = tests; t->name != NULL; t++) {
		printf("%2d) %-70s", (*i)++, t->name);
		fd = open(t->dev_name, t->flags, t->mode);
		if (fd == -1) {
			perror("open");
			goto fail;
		}
		if (close(fd)) {
			perror("close");
			goto fail;
		}
		ksft_inc_pass_cnt();
		puts("PASS");
	}
	return 0;
fail:
	ksft_inc_fail_cnt();
	puts("FAIL");
	return 1;
}

int main(void)
{
	int fail = 0;
	int i = 1;

	if (open_test(&i))
		fail++;

	puts("");
	if (fail)
		ksft_exit_fail();
	else
		ksft_exit_pass();
}
