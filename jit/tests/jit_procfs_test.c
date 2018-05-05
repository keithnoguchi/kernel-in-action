/* SPDX-License-Identifier: GPL-2.0 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "kselftest.h"

static int procfs_test(int *i)
{
	const struct test {
		const char	*name;
		const char	*file_name;
		int		flags;
	} tests[] = {
		{
			.name = "/proc/currenttime file",
			.file_name = "/proc/currenttime",
			.flags = O_RDONLY,
		},
		{ /* sentory */ },
	};
	const struct test *t;
	int fail = 0;

	for (t = tests; t->name; t++) {
		printf("%2d) %-70s", (*i)++, t->name);
		int fd;

		fd = open(t->file_name, t->flags);
		if (fd == -1) {
			perror("open");
			goto fail;
		}
		if (close(fd)) {
			perror("close");
			fd = -1;
			goto fail;
		}
		ksft_inc_pass_cnt();
		puts("PASS");
		continue;
fail:
		ksft_inc_fail_cnt();
		puts("FAIL");
		if (fd != -1)
			close(fd);
		fail++;
	}
	if (fail)
		return 1;
	return 0;
}

int main(void)
{
	int i = 1;

	if (procfs_test(&i))
		ksft_exit_fail();
	ksft_exit_pass();
}
