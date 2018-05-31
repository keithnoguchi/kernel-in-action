/* SPDX-License-Identifier: GPL-2.0 */
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "kselftest.h"

static int test_devfs(int *i)
{
	const struct test {
		const char	*name;
		const char	*devname;
	} tests[] = {
		{
			.name		= "/dev/scullmc0 simple open",
			.devname	= "/dev/scullmc0",
		},
		{
			.name		= "/dev/scullmc1 simple open",
			.devname	= "/dev/scullmc1",
		},
		{
			.name		= "/dev/scullmc2 simple open",
			.devname	= "/dev/scullmc2",
		},
		{
			.name		= "/dev/scullmc3 simple open",
			.devname	= "/dev/scullmc3",
		},
		{ /* sentry */ },
	};
	const struct test *t;
	int fail = 0;

	for (t = tests; t->name; t++) {
		int err;
		int fd;

		printf("%3d) %-12s %-55s", ++(*i), __FUNCTION__, t->name);

		fd = open(t->devname, O_RDONLY);
		if (fd == -1) {
			printf("FAIL: open(%s): %s\n", t->devname, strerror(errno));
			goto fail;
		}
		close(fd);
		puts("PASS");
		ksft_inc_pass_cnt();
		continue;
fail:
		ksft_inc_fail_cnt();
		fail++;
	}
	return fail;
}

int main(void)
{
	int fail = 0;
	int i = 0;

	if (test_devfs(&i))
		fail++;

	if (fail)
		ksft_exit_fail();
	ksft_exit_pass();
}
