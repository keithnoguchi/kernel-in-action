/* SPDX-License-Identifier: GPL-2.0 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "kselftest.h"

static int sysfs_test(int *i)
{
	const struct test {
		const char	*name;
		const char	*sysfs_name;
		int		flags;
	} tests[] = {
		{
			.name =		"/sys/bus/ldd/drivers_autoprobe sysfs attribute",
			.sysfs_name =	"/sys/bus/ldd/drivers_autoprobe",
			.flags =	O_RDONLY,
		},
		{
			.name =		"/sys/devices/ldd0/uevent sysfs attribute",
			.sysfs_name =	"/sys/devices/ldd0/uevent",
			.flags =	O_RDONLY,
		},
		{ /* sentry */ },
	};
	const struct test *t;

	for (t = tests; t->name; t++) {
		int fd;

		printf("%2d) %-16s: ", (*i)++, t->name);
		fd = open(t->sysfs_name, t->flags);
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
	int i = 1;

	if (sysfs_test(&i))
		ksft_exit_fail();
	ksft_exit_pass();
}
