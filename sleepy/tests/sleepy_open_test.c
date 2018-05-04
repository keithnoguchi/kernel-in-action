/* SPDX-License-Identifier: GPL-2.0 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include "kselftest.h"

static int open_test(int *i)
{
	const struct test {
		const char	*name;
		const char	*file_name;
		int		flags;
	} tests[] = {
		{
			.name = "open(/dev/sleep0, O_RDONLY) test",
			.file_name = "/dev/sleep0",
			.flags = O_RDONLY,
		},
		{
			.name = "open(/dev/sleep1, O_RDONLY) test",
			.file_name = "/dev/sleep1",
			.flags = O_RDONLY,
		},
		{
			.name = "open(/dev/sleep2, O_RDONLY) test",
			.file_name = "/dev/sleep2",
			.flags = O_RDONLY,
		},
		{
			.name = "open(/dev/sleep3, O_RDONLY) test",
			.file_name = "/dev/sleep3",
			.flags = O_RDONLY,
		},
		{ /* sentry */ },
	};
	const struct test *t;
	int fail = 0;

	for (t = tests; t->name; t++) {
		int fd;

		printf("%2d) %-70s", (*i)++, t->name);

		fd = open(t->file_name, t->flags);
		if (fd == -1) {
			perror("open");
			goto fail;
		}
		if (close(fd)) {
			fd = -1;
			perror("close");
			goto fail;
		}
		ksft_inc_pass_cnt();
		puts("PASS");
		continue;
fail:
		if (fd != -1)
			close(fd);
		ksft_inc_fail_cnt();
		puts("FAIL");
		fail++;
	}
	if (fail)
		return 1;
	return 0;
}

int main(void)
{
	int i = 1;

	if (open_test(&i))
		ksft_exit_fail();
	ksft_exit_pass();
}
