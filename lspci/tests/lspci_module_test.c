/* SPDX-License-Identifier: GPL-2.0 */

#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include "kselftest.h"

static int sysfs_test(int *i)
{
	const struct test {
		const char	*name;
		const char	*file_name;
		int		flags;
	} tests[] = {
		{
			.name = "open(/sys/module/lspci/refcnt, O_RDONLY)",
			.file_name = "/sys/module/lspci/refcnt",
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
		if (fd == -1)
			goto fail;
		if (close(fd)) {
			fd = -1;
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
	return fail;
}

int main(void)
{
	int i = 1;

	if (sysfs_test(&i))
		ksft_exit_fail();
	ksft_exit_pass();
}
